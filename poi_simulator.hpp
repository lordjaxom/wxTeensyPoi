#ifndef TPC_POI_SIMULATOR_HPP
#define TPC_POI_SIMULATOR_HPP

#include <functional>
#include <fstream>
#include <istream>
#include <ostream>
#include <string>

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/thread.hpp>

#include "poi_protocol.hpp"

#include "debug.hpp"

namespace PoiCommunicatorDetail {

	class SerialPortSimulator
	{
		enum class State
		{
			COMMAND,
			BLOCK
		};
		
	public:
		SerialPortSimulator( boost::asio::io_service& service )
			: service_ { service }
			, open_ {}
			, state_ { State::COMMAND }
		{
			std::ifstream is( "memory.bin", std::ios::binary );
			is.read( (char*) &memory_[ 0 ], memory_.size() );
		}
		
		SerialPortSimulator( SerialPortSimulator const& ) = delete;
		
		~SerialPortSimulator()
		{
			std::ofstream os( "memory.bin", std::ios::binary );
			os.write( (char const*) &memory_[ 0 ], memory_.size() );
		}
		
		bool is_open() const { return open_; }
		
		void open( std::string const& port ) { open_ = true; }
		void close() { open_ = false; }
		void cancel() {}
		template< typename Option > void set_option( Option ) {}
		
		void async_read_some( boost::asio::mutable_buffer const& buffer, std::function< void ( boost::system::error_code const&, size_t ) > handler )
		{
			check_open();

			uint8_t* data = boost::asio::buffer_cast< uint8_t* >( buffer );
			size_t length = boost::asio::buffer_size( buffer );
			
			switch ( state_ ) {
				case State::COMMAND: {
					std::copy_n( teensyPoi::magicPacket, teensyPoi::magicPacketLength, data );
					if ( command_ == teensyPoi::commandHello ) {
						data[ 3 ] = teensyPoi::responseOk;
					}
					else {
						data[ 3 ] = teensyPoi::responseAwaitData;
						state_ = State::BLOCK;
					}
					break;
				}
				
				case State::BLOCK: {
					debug( "reading 4096 bytes from block ", blockNo_ );
					std::copy_n( memory_.begin() + blockNo_ * teensyPoi::flashBlockSize, teensyPoi::flashBlockSize, data );
					state_ = State::COMMAND;
					command_ = teensyPoi::commandHello;
					break;
				}
			}
			
			service_.post( [handler, length] { 
				boost::this_thread::sleep( boost::posix_time::milliseconds( 20 ) );
				handler( {}, length ); 
			} );
		}
		
		void async_write_some( boost::asio::const_buffer const& buffer, std::function< void ( boost::system::error_code const&, size_t ) > handler )
		{
			check_open();
			
			uint8_t const* data = boost::asio::buffer_cast< uint8_t const* >( buffer );
			size_t length = boost::asio::buffer_size( buffer );
			
			switch ( state_ ) {
				case State::COMMAND: {
					command_ = data[ 3 ];
					if ( command_ == teensyPoi::commandRead || command_ == teensyPoi::commandWrite ) {
						blockNo_ = data[ 4 ] | ( data[ 5 ] << 8 );
					}
					break;
				}
				
				case State::BLOCK: {
					debug( "writing ", length, " bytes into block ", blockNo_ );
					std::copy_n( data, teensyPoi::flashBlockSize, memory_.begin() + blockNo_ * teensyPoi::flashBlockSize );
					state_ = State::COMMAND;
					command_ = teensyPoi::commandHello;
					break;
				}
			}
			
			service_.post( [handler, length] { 
				boost::this_thread::sleep( boost::posix_time::milliseconds( 50 ) );
				handler( {}, length ); 
			} );
		}
		
	private:
		void check_open()
		{
			if ( !open_ ) {
				throw boost::system::system_error( boost::system::errc::invalid_argument, boost::system::system_category() );
			}
		}
		
		boost::asio::io_service& service_;
		bool open_;
		State state_;
		uint8_t command_;
		uint16_t blockNo_;
		std::array< uint8_t, teensyPoi::flashSize > memory_;
	};

} // namespace PoiCommunicatorDetail

#endif // TPC_POI_SIMULATOR_HPP