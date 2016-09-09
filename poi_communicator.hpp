#ifndef TPC_POI_COMMUNICATOR_HPP
#define TPC_POI_COMMUNICATOR_HPP

#include <functional>
#include <string>
#include <utility>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/serial_port.hpp>

#include "poi_state.hpp"
#if defined( TPC_SIMULATE_COMMUNICATOR )
#	include "poi_simulator.hpp"
#endif

namespace PoiCommunicatorDetail {
    class StateGuard;
    class TimeoutGuard;
} // namespace PoiCommunicatorDetail

class PoiCommunicator
{
    friend class PoiCommunicatorDetail::StateGuard;
    friend class PoiCommunicatorDetail::TimeoutGuard;

#ifndef TPC_SIMULATE_COMMUNICATOR
	typedef boost::asio::serial_port SerialPortType;
#else
	typedef PoiCommunicatorDetail::SerialPortSimulator SerialPortType;
#endif

public:
    typedef std::function< void () > HandlerType;

    PoiCommunicator( boost::asio::io_service& service);
    PoiCommunicator( PoiCommunicator const& ) = delete;

    PoiState GetState() const { return state_; }

    void Connect( std::string const& port, HandlerType handler );
	void Disconnect();
    void ReadBlock( uint16_t blockNo, uint8_t* buffer, size_t length, HandlerType handler );
    void WriteBlock( uint16_t blockNo, uint8_t const* buffer, size_t length, HandlerType handler );

private:
    void SendCommand( std::vector< uint8_t > command, uint8_t expectedResponse, HandlerType handler, bool retry = true );
    void ReceiveResponse( uint8_t expectedResponse, HandlerType handler, bool retry = false, std::vector< uint8_t > retryCommand = {} );
    void Close();

    void AsyncRead( uint8_t* buffer, size_t length, HandlerType handler );
    void AsyncWrite( uint8_t const* buffer, size_t length, HandlerType handler );

    void FlushInputBuffers( HandlerType handler );

    boost::asio::io_service& service_;
    SerialPortType serial_;
    PoiState state_;
    std::vector< uint8_t > buffer_;
};

#endif // TPC_POI_COMMUNICATOR_HPP
