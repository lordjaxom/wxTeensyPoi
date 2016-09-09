#include <array>
#include <functional>
#include <iterator>
#include <limits>
#include <type_traits>

#include "poi_accessor.hpp"
#include "poi_exception.hpp"
#include "poi_protocol.hpp"

#include "debug.hpp"

namespace PoiAccessorDetail {

    template< typename T, typename It, typename... Args >
    T Extract( It& it, Args&&... args );

    template< typename T >
    struct Extractor;

    template<>
    struct Extractor< uint8_t >
    {
        template< typename It >
        static uint8_t Extract( It& it )
        {
            return *it++;
        }
    };

    template<>
    struct Extractor< uint16_t >
    {
        template< typename It >
        static uint16_t Extract( It& it )
        {
            uint16_t result {};
            result |= (uint16_t) ( *it++ << 0 );
            result |= (uint16_t) ( *it++ << 8 );
            return result;
        }
    };

    template<>
    struct Extractor< uint32_t >
    {
        template< typename It >
        static uint32_t Extract( It& it )
        {
            uint32_t result {};
            result |= (uint32_t) ( *it++ << 0 );
            result |= (uint32_t) ( *it++ << 8 );
            result |= (uint32_t) ( *it++ << 16 );
            result |= (uint32_t) ( *it++ << 24 );
            return result;
        }
    };

    template<>
    struct Extractor< teensyPoi::FlashFileHeader >
    {
        template< typename It >
        static teensyPoi::FlashFileHeader Extract( It& it )
        {
            teensyPoi::FlashFileHeader result {};
            result.version = PoiAccessorDetail::Extract< decltype( result.version ) >( it );
            result.fileType = PoiAccessorDetail::Extract< decltype( result.fileType ) >( it );
            result.fileSize = PoiAccessorDetail::Extract< decltype( result.fileSize ) >( it );
            return result;
        }
    };

    template<>
    struct Extractor< teensyPoi::FlashImageFileHeader >
    {
        template< typename It >
        static teensyPoi::FlashImageFileHeader Extract( It& it, teensyPoi::FlashFileHeader const& fileHeader )
        {
            teensyPoi::FlashImageFileHeader result { fileHeader };
            result.linesPerRound = PoiAccessorDetail::Extract< decltype( result.linesPerRound ) >( it );
            return result;
        }
    };

    template< typename T >
    struct Extractor
    {
        template< typename It >
        static typename std::enable_if< std::is_enum< T >::value, T >::type Extract( It& it )
        {
            return (T) PoiAccessorDetail::Extract< typename std::underlying_type< T >::type >( it );
        }
    };

    template< typename T, typename It, typename... Args >
    T Extract( It& it, Args&&... args )
    {
        return Extractor< T >::Extract( it, std::forward< Args >( args )... );
    }

    template< typename T, typename It >
    void Insert( It& it, T const& value );

    template< typename T >
    struct Inserter;

    template<>
    struct Inserter< uint8_t >
    {
        template< typename It >
        static void Insert( It& it, uint8_t value )
        {
            *it++ = value;
        }
    };

    template<>
    struct Inserter< uint16_t >
    {
        template< typename It >
        static void Insert( It& it, uint16_t value )
        {
            *it++ = (uint8_t) ( ( value >> 0  ) & 0xff );
            *it++ = (uint8_t) ( ( value >> 8  ) & 0xff );
        }
    };

    template<>
    struct Inserter< uint32_t >
    {
        template< typename It >
        static void Insert( It& it, uint32_t value )
        {
            *it++ = (uint8_t) ( ( value >> 0  ) & 0xff );
            *it++ = (uint8_t) ( ( value >> 8  ) & 0xff );
            *it++ = (uint8_t) ( ( value >> 16 ) & 0xff );
            *it++ = (uint8_t) ( ( value >> 24 ) & 0xff );
        }
    };

    template<>
    struct Inserter< teensyPoi::FlashFileHeader >
    {
        template< typename It >
        static void Insert( It& it, teensyPoi::FlashFileHeader const& value )
        {
            PoiAccessorDetail::Insert( it, value.version );
            PoiAccessorDetail::Insert( it, value.fileType );
            PoiAccessorDetail::Insert( it, value.fileSize );
        }
    };

    template<>
    struct Inserter< teensyPoi::FlashImageFileHeader >
    {
        template< typename It >
        static void Insert( It& it, teensyPoi::FlashImageFileHeader const& value )
        {
            PoiAccessorDetail::Insert( it, value.fileHeader );
            PoiAccessorDetail::Insert( it, value.linesPerRound );
        }
    };

    template< typename T >
    struct Inserter
    {
        template< typename It >
        static void Insert( It& it, typename std::enable_if< std::is_enum< T >::value, T >::type value )
        {
            typedef typename std::underlying_type< T >::type underlying_type;
            Inserter< underlying_type >::Insert( it, (underlying_type) value );
        }
    };

    template< typename T, typename It >
    void Insert( It& it, T const& value )
    {
        Inserter< T >::Insert( it, value );
    }

    template< typename Func >
    std::shared_ptr< void > MakeScopeGuard( Func func )
    {
    	return std::shared_ptr< void >( nullptr, [func]( void* ) { func(); } );
    }

    class LoadAction : public std::enable_shared_from_this< LoadAction >
    {
		typedef std::array< uint8_t, teensyPoi::flashBlockSize > BlockType;

    public:
        LoadAction( PoiAccessor& accessor )
            : accessor_( accessor )
            , structure_( false )
			, imageLoader_( structure_ )
        {
        }

        void Run()
        {
            auto self = shared_from_this();
            accessor_.communicator_.ReadBlock( blockNo_++, &block_[ 0 ], block_.size(), [this, self] {
				accessor_.OnProgress( -1, -1 );

                auto it = block_.begin();
                bool eod = false;

                debug( "read block ", blockNo_ - 1, ", expecting: ", imageLoader_.IsUsed() ? " image data" : " next header" );

                if ( imageLoader_.IsUsed() ) {
                    imageLoader_.AddData( &*it, std::distance( it, block_.end() ) );
                }
                else if ( blockNo_ == teensyPoi::flashBlockCount ) {
                    eod = true;
                }
                else {
                    auto header = Extract< teensyPoi::FlashFileHeader >( it );
                    if ( header.version != teensyPoi::flashBlockVersion ) {
                        throw PoiException( "Wrong format on Poi's flash memory: Invalid version constant. Starting with empty structure." );
                    }

                    switch ( header.fileType ) {
                        case teensyPoi::FlashFileType::IMAGE: {
                            auto imageHeader = Extract< teensyPoi::FlashImageFileHeader >( it, header );
                            imageLoader_.AddHeader( header.fileSize, imageHeader.linesPerRound );
                            imageLoader_.AddData( &*it, std::distance( it, block_.end() ) );
                            break;
                        }

                        case teensyPoi::FlashFileType::EOD: {
                            eod = true;
                            break;
                        }

                        default: {
                            throw PoiException( "Wrong Format on Poi's flash memory, starting with empty structure." );
                        }
                    }
                }

                if ( eod ) {
                    imageLoader_.AddEod();
                    accessor_.OnLoaded( structure_ );
                    return;
                }

                Run();
            } );
        }

    private:
        PoiAccessor& accessor_;
        PoiStructure structure_;
        PoiImageLoader imageLoader_;
        BlockType block_;
        uint16_t blockNo_ {};
        uint16_t nextFilePtr_ {};
    };

    class SaveAction : public std::enable_shared_from_this< SaveAction >
    {
		typedef std::array< uint8_t, teensyPoi::flashBlockSize > BlockType;

    public:
        SaveAction( PoiAccessor& accessor, PoiStructure const& structure )
            : accessor_( accessor )
            , structure_( structure )
			, blockNo_()
			, imagesIt_( structure_.GetImages().begin() )
			, imageIt_( imagesIt_->GetData().begin() )
        {
        }

        SaveAction( SaveAction const& ) = delete;

        void Run()
        {
            auto self = shared_from_this();

			auto blockIt( block_.begin() );
			bool dirty = false;
			bool eod = false;

			if ( imagesIt_ == structure_.GetImages().end() ) {
                if ( blockNo_ == teensyPoi::flashBlockCount ) {
                    return;
                }
                if ( structure_.IsDirty() ) {
					teensyPoi::FlashFileHeader header {
						teensyPoi::flashBlockVersion,
						teensyPoi::FlashFileType::EOD,
						0
					};
					Insert( blockIt, header );
					dirty = true;
                }
                eod = true;
			}
			else {
                if ( imageIt_ == imagesIt_->GetData().begin() && imagesIt_->IsDirty() ) {
                    teensyPoi::FlashImageFileHeader header { {
                        teensyPoi::flashBlockVersion,
                        teensyPoi::FlashFileType::IMAGE,
                        imagesIt_->GetFileSize() },
                        imagesIt_->GetLinesPerRound()
                    };
                    Insert( blockIt, header );
					dirty = true;
                }

                size_t payloadLength = (size_t) std::min( std::distance( blockIt, block_.end() ), std::distance( imageIt_, imagesIt_->GetData().end() ) );
                if ( imagesIt_->IsDataDirty() ) {
					std::copy_n( imageIt_, payloadLength, blockIt );
					dirty = true;
                }
                imageIt_ += payloadLength;
                if ( imageIt_ == imagesIt_->GetData().end() ) {
					imagesIt_->Dirty( false );
                    ++imagesIt_;
                    imageIt_ = imagesIt_->GetData().begin();
                }
			}

			auto guard = MakeScopeGuard( [this, eod] { eod ? accessor_.OnSaved() : Run(); } );
			auto blockNo { blockNo_++ };
			if ( dirty ) {
				accessor_.communicator_.WriteBlock( blockNo, &block_[ 0 ], block_.size(), [this, self, eod, guard] {
					accessor_.OnProgress( blockNo_, structure_.GetBlocksUsed() );
				} );
			}
        }

    private:
        PoiAccessor& accessor_;
        PoiStructure const& structure_;
        BlockType block_;
		uint16_t blockNo_;
		std::vector< PoiImage >::const_iterator imagesIt_;
		std::vector< uint8_t >::const_iterator imageIt_;
    };

} // namespace PoiAccessorDetail

using namespace PoiAccessorDetail;

PoiAccessor::PoiAccessor()
    : deadline_( service_ )
    , communicator_( service_ )
    , running_()
	, thread_( [this] { Run(); } )
{
}

PoiAccessor::~PoiAccessor()
{
    Stop();
}

void PoiAccessor::SetPortName( std::string portName )
{
	if ( portName_ != portName ) {
		communicator_.Disconnect();
		portName_ = std::move( portName );
	}
}

void PoiAccessor::Stop()
{
    running_ = false;
    service_.stop();
}

void PoiAccessor::KeepAlive()
{
	deadline_.expires_at( boost::posix_time::pos_infin );
	deadline_.async_wait( [this] ( boost::system::error_code const& ec ) { KeepAlive(); } );
}

template< typename T, typename... Args >
void PoiAccessor::StartAction( Args&&... args )
{
	auto action = std::make_shared< T >( *this, std::forward< Args >( args )... );
	if ( communicator_.GetState() == PoiState::DISCONNECTED ) {
		service_.post( [this, action] { communicator_.Connect( portName_, [action] { action->Run(); } ); } );
	}
	else {
		service_.post( [action] { action->Run(); } );
	}
}

void PoiAccessor::Load()
{
	StartAction< LoadAction >();
}

void PoiAccessor::Save( PoiStructure const& structure )
{
	StartAction< SaveAction >( structure );
}

void PoiAccessor::Run()
{
    debug( "poi accessor thread starting up" );
	running_ = true;
	KeepAlive();
    while ( running_ ) {
		PoiState state = communicator_.GetState();
        try {
            if ( service_.run_one() == 0 ) {
				running_ = false;
			}
        }
        catch ( PoiException const& e ) {
            OnError( e.what(), e.IsReset() );
        }
        if ( communicator_.GetState() != state ) {
            OnStateChanged( communicator_.GetState() );
        }
    }
    running_ = false;
    debug( "poi accessor thread finished" );
}
