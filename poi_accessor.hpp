#ifndef TPC_POI_ACCESSOR_HPP
#define TPC_POI_ACCESSOR_HPP

#include <functional>
#include <memory>

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>
#include <boost/signals2/signal.hpp>

#include "poi_communicator.hpp"
#include "poi_structure.hpp"

#include <winioctl.h>
#include <setupapi.h>

namespace PoiAccessorDetail {
    class LoadAction;
    class SaveAction;
} // namespace PoiAccessorDetail

class PoiAccessor
{
    friend class PoiAccessorDetail::LoadAction;
    friend class PoiAccessorDetail::SaveAction;

public:
    PoiAccessor();
    PoiAccessor( PoiAccessor const& ) = delete;
    ~PoiAccessor();

	std::string const& GetPortName() const { return portName_; }
    PoiState GetState() const { return communicator_.GetState(); }

	void SetPortName( std::string portName );

    void Load();
    void Save( PoiStructure const& structure );

    void Stop();

    boost::signals2::signal< void ( std::string const&, bool ) > OnError;
    boost::signals2::signal< void ( PoiState ) > OnStateChanged;
    boost::signals2::signal< void ( int, int ) > OnProgress;
    boost::signals2::signal< void ( PoiStructure ) > OnLoaded;
    boost::signals2::signal< void () > OnSaved;

private:
	void KeepAlive();

	template< typename T, typename... Args >
	void StartAction( Args&&... args );

    void Run();

    boost::asio::io_service service_;
	boost::asio::deadline_timer deadline_;
    PoiCommunicator communicator_;
	std::string portName_;
    bool running_;
    boost::thread thread_;
};

#endif // TPC_POI_ACCESSOR_HPP
