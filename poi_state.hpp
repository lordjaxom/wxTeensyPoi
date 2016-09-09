#ifndef TPC_POI_STATE_HPP
#define TPC_POI_STATE_HPP

#include <ostream>

enum class PoiState
{
    DISCONNECTED,
    CONNECTING,
    READY,
    BUSY
};

std::ostream& operator<<( std::ostream& os, PoiState state );

#endif // TPC_POI_STATE_HPP
