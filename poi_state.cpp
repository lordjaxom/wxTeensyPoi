#include "poi_state.hpp"

std::ostream& operator<<( std::ostream& os, PoiState state )
{
    switch ( state ) {
        case PoiState::DISCONNECTED:
            return os << "DISCONNECTED";
        case PoiState::CONNECTING:
            return os << "CONNECTING";
        case PoiState::READY:
            return os << "READY";
        case PoiState::BUSY:
            return os << "BUSY";
    }
    return os << "UNKNOWN";
}
