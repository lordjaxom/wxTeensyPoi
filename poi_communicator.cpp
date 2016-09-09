#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/io/ios_state.hpp>
#include <boost/range/size.hpp>

#include "poi_communicator.hpp"
#include "poi_exception.hpp"
#include "poi_protocol.hpp"

#include "debug.hpp"

namespace PoiCommunicatorDetail {

    template< typename InputIt >
    void dump( InputIt first, InputIt last, bool input )
    {
        boost::io::ios_base_all_saver saver( std::cout );
        std::cout << ( input ? "<<< " : ">>> " );
        for ( ; first != last; ++first ) {
            std::cout << std::hex << std::setw( 2 ) << std::setfill( '0' ) << (int) *first << " ";
        }
        std::cout << "\n";
    }

    void dump( std::vector< uint8_t > const& message, bool input )
    {
        dump( message.begin(), message.end(), input );
    }

    class StateGuard
    {
    public:
        StateGuard( PoiCommunicator& communicator, bool connecting )
            : communicator_( communicator )
            , success_()
        {
            switch ( communicator_.state_ ) {
                case PoiState::DISCONNECTED: {
                    if ( !connecting ) {
						throw PoiException( "Poi is not plugged in or not in programming mode." );
                    }
                    communicator_.state_ = PoiState::CONNECTING;
                    break;
                }

                case PoiState::CONNECTING:
                case PoiState::BUSY: {
                    throw PoiException( "Unable to start an operation while another one is still running." );
                }

                case PoiState::READY: {
                    communicator_.state_ =  PoiState::BUSY;
                }
            }
        }

        ~StateGuard()
        {
            if ( !success_ ) {
                communicator_.Close();
            }
        }

        void Success()
        {
            success_ = true;
            communicator_.state_ = PoiState::READY;
        }

    private:
        PoiCommunicator& communicator_;
        bool success_;
    };

    std::shared_ptr< StateGuard > MakeStateGuard( PoiCommunicator& communicator, bool connecting = false )
    {
        return std::make_shared< StateGuard >( communicator, connecting );
    }

    class TimeoutGuard
    {
    public:
    	using HandlerType = std::function< void () >;

        TimeoutGuard( PoiCommunicator& communicator, std::size_t timeout, HandlerType handler )
            : communicator_( communicator )
            , handler_( handler )
            , deadline_( communicator_.service_ )
        {
            deadline_.expires_from_now( boost::posix_time::milliseconds( timeout ) );
            deadline_.async_wait( [this] ( boost::system::error_code const& ec ) {
                if ( ec == boost::system::errc::operation_canceled ) {
                    return;
                }
                deadline_.expires_at( boost::posix_time::pos_infin );
                communicator_.serial_.cancel();
				handler_();
            } );
        }

        void Success()
        {
            deadline_.expires_at( boost::posix_time::pos_infin );
        }

    private:
        PoiCommunicator& communicator_;
        HandlerType handler_;
        boost::asio::deadline_timer deadline_;
    };

    std::shared_ptr< TimeoutGuard > MakeTimeoutGuard( PoiCommunicator& communicator, std::size_t timeout = 2000, TimeoutGuard::HandlerType handler = [] { throw PoiException( "Timeout!" ); } )
    {
        return std::make_shared< TimeoutGuard >( communicator, timeout, handler );
    }

} // namespace PoiCommunicatorDetail

using namespace PoiCommunicatorDetail;

PoiCommunicator::PoiCommunicator( boost::asio::io_service& service )
    : service_( service )
    , serial_( service_ )
    , state_( PoiState::DISCONNECTED )
{
}

void PoiCommunicator::Connect( std::string const& port, HandlerType handler )
{
    if ( state_ != PoiState::DISCONNECTED ) {
        return;
    }

    auto guard = MakeStateGuard( *this, true );

	try {
		serial_.open( port );
		serial_.set_option( boost::asio::serial_port_base::baud_rate( 115200 ) );
	}
	catch ( boost::system::system_error const& e ) {
		throw PoiException( "Poi is not plugged in or not in programming mode." );
	}

	FlushInputBuffers( [=] {
		SendCommand( { teensyPoi::commandHello }, teensyPoi::responseOk, [=] {
			guard->Success();
			handler();
		} );
	} );
}

void PoiCommunicator::Disconnect()
{
	if ( serial_.is_open() ) {
		serial_.close();
	}
	state_ = PoiState::DISCONNECTED;
}

void PoiCommunicator::ReadBlock( uint16_t blockNo, uint8_t* buffer, size_t length, HandlerType handler )
{
    if ( length < teensyPoi::flashBlockSize ) {
        throw PoiException( "Buffer too small." );
    }

    auto guard = MakeStateGuard( *this );

    SendCommand( { teensyPoi::commandRead, (uint8_t) ( blockNo & 0xff ), (uint8_t) ( blockNo >> 8 ) },
                 teensyPoi::responseAwaitData, [this, buffer, handler, guard] {
        AsyncRead( buffer, teensyPoi::flashBlockSize, [this, handler, guard, buffer ] {
            dump( buffer, buffer + 20, true );
            ReceiveResponse( teensyPoi::responseOk, [this, handler, guard] {
                guard->Success();
                handler();
            } );
        } );
    } );
}

void PoiCommunicator::WriteBlock( uint16_t blockNo, uint8_t const* buffer, size_t length, HandlerType handler )
{
     if ( length > teensyPoi::flashBlockSize ) {
        throw PoiException( "Buffer too big." );
    }

    auto guard = MakeStateGuard( *this );

    if ( length < teensyPoi::flashBlockSize ) {
        buffer_.assign( buffer, buffer + length );
        buffer_.resize( teensyPoi::flashBlockSize );
        buffer = &buffer_[ 0 ];
    }

    SendCommand( { teensyPoi::commandWrite, (uint8_t) ( blockNo & 0xff ), (uint8_t) ( blockNo >> 8 ) },
                 teensyPoi::responseAwaitData, [this, buffer, handler, guard] {
        AsyncWrite( buffer, teensyPoi::flashBlockSize, [this, handler, guard] {
            ReceiveResponse( teensyPoi::responseOk, [this, handler, guard] {
                guard->Success();
                handler();
            } );
        } );
    } );
}

void PoiCommunicator::SendCommand( std::vector< uint8_t > command, uint8_t expectedResponse, HandlerType handler, bool retry )
{
    buffer_.assign( std::begin( teensyPoi::magicPacket ), std::end( teensyPoi::magicPacket ) );
    buffer_.insert( buffer_.end(), command.begin(), command.end() );
    dump( buffer_, false );
    AsyncWrite( &buffer_[ 0 ], buffer_.size(), [=] {
        ReceiveResponse( expectedResponse, handler, retry, command );
    } );
}

void PoiCommunicator::ReceiveResponse( uint8_t expectedResponse, HandlerType handler, bool retry, std::vector< uint8_t > retryCommand )
{
    buffer_.resize( teensyPoi::magicPacketLength + 1 );
    AsyncRead( &buffer_[ 0 ], buffer_.size(), [=] {
        dump( buffer_, true );
        if ( std::equal( std::begin( teensyPoi::magicPacket ), std::end( teensyPoi::magicPacket ), buffer_.begin() ) ) {
			uint8_t response = buffer_[ teensyPoi::magicPacketLength ];
			if ( response == expectedResponse ) {
				handler();
				return;
			}
        }
        if ( retry ) {
			FlushInputBuffers( [=] {
				SendCommand( retryCommand, expectedResponse, handler, false );
			} );
        }
		throw PoiException( "Protocol error: Unexpected response from Poi." );
    } );
}

void PoiCommunicator::Close()
{
    if ( serial_.is_open() ) {
    }

    serial_.close();
    state_ = PoiState::DISCONNECTED;
}

void PoiCommunicator::AsyncRead( uint8_t* buffer, size_t length, HandlerType handler )
{
    auto guard { MakeTimeoutGuard( *this ) };
    boost::asio::async_read( serial_, boost::asio::buffer( buffer, length ), [=] ( boost::system::error_code const& ec, size_t received ) {
        debug( "AsyncRead: requested ", length, ", got ", received, " (ec is ", ec, ")" );
        if ( ec == boost::system::errc::operation_canceled ) {
            return;
        }
        if ( ec ) {
            throw PoiException( "Communication error with Poi: " + ec.message() );
        }
        guard->Success();
        received == length ? handler() : AsyncRead( buffer + received, length - received, handler );
    } );
}

void PoiCommunicator::AsyncWrite( uint8_t const* buffer, size_t length, HandlerType handler )
{
    auto guard { MakeTimeoutGuard( *this ) };
    boost::asio::async_write( serial_, boost::asio::buffer( buffer, length ), [=] ( boost::system::error_code const& ec, size_t sent ) {
        if ( ec == boost::system::errc::operation_canceled ) {
            return;
        }
        if ( ec ) {
            throw PoiException( "Communication error with Poi: " + ec.message() );
        }
        guard->Success();
		sent == length ? handler() : AsyncWrite( buffer + sent, length - sent, handler );
    } );
}

void PoiCommunicator::FlushInputBuffers( HandlerType handler )
{
	debug( "trying to flush buffers" );
	auto guard { MakeTimeoutGuard( *this, 250, [=] {
		debug( "flush buffers completed" );
		handler();
	} ) };
	buffer_.resize( 256 );
	serial_.async_read_some( boost::asio::buffer( buffer_ ), [=] ( boost::system::error_code const& ec, std::size_t received ) {
		if ( ec == boost::system::errc::operation_canceled ) {
			return;
		}
        if ( ec ) {
            throw PoiException( "Communication error with Poi: " + ec.message() );
        }
		debug( "read ", received, " bytes of junk" );
		guard->Success();
		FlushInputBuffers( handler );
	} );
}
