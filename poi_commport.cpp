#include <cctype>
#include <algorithm>
#include <memory>

#include "poi_commport.hpp"

#include <windows.h>
#include <winioctl.h>
#include <setupapi.h>

namespace {

	std::string RegQueryValueString( HKEY key, char const* lpValueName )
	{
		//First query for the type and size of the registry value
		DWORD dwType;
		DWORD dwSize;
		LONG nStatus = RegGetValue( key, nullptr, lpValueName, RRF_RT_ANY, &dwType, nullptr, &dwSize );
		if ( nStatus != ERROR_SUCCESS ) {
			SetLastError( nStatus );
			return {};
		}
		if ( ( dwType != REG_SZ ) && ( dwType != REG_EXPAND_SZ ) ) {
			SetLastError( ERROR_INVALID_DATA );
			return {};
		}

		//Allocate enough bytes for the return value
		std::string szValue( dwSize, '\0' );
		nStatus = RegGetValue( key, nullptr, lpValueName, RRF_RT_ANY, &dwType, &szValue[ 0 ], &dwSize );
		if ( nStatus != ERROR_SUCCESS ) {
			SetLastError(nStatus);
			return {};
		}
		szValue.resize( dwSize - 1 );
		
		return szValue;
	}

	int QueryRegistryPortName( HKEY deviceKey )
	{
		//Read in the name of the port
		std::string szPortName = RegQueryValueString( deviceKey, _T("PortName") );
		//If it looks like "COMX" then
		//add it to the array which will be returned
		return szPortName.length() > 3 && 
				std::equal( szPortName.begin(), szPortName.begin() + 3, "COM" ) && 
				std::all_of( szPortName.begin() + 3, szPortName.end(), [] ( char ch ) { return isdigit( ch ); } )
				? atoi( &szPortName[ 3 ] )
				: -1;
	}
	
	std::string QueryDeviceDescription( HDEVINFO hDevInfoSet, SP_DEVINFO_DATA& devInfo )
	{
		DWORD dwType = 0;
		DWORD dwSize = 0;
		//Query initially to get the buffer size required
		if ( !SetupDiGetDeviceRegistryProperty(hDevInfoSet, &devInfo, SPDRP_DEVICEDESC, &dwType, nullptr, 0, &dwSize ) ) {
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
				return {};
			}
		}

		std::string szFriendlyName( dwSize, '\0' );
		if ( !SetupDiGetDeviceRegistryProperty( hDevInfoSet, &devInfo, SPDRP_DEVICEDESC, &dwType, reinterpret_cast< LPBYTE >( &szFriendlyName[ 0 ] ), dwSize, &dwSize ) || 
				dwType != REG_SZ ) {
			return {};
		}
		szFriendlyName.resize( dwSize - 1 );
		return szFriendlyName;
	}
	
} // namespace

std::vector< PoiCommPort > PoiCommPort::Enumerate()
{
	std::vector< std::shared_ptr< void > > guards;
	
	//Create a "device information set" for the specified GUID
	HDEVINFO hDevInfoSet = SetupDiGetClassDevs( &GUID_DEVINTERFACE_COMPORT, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );
	if ( hDevInfoSet == INVALID_HANDLE_VALUE ) {
		return {};
	}
	guards.emplace_back( hDevInfoSet, [] ( HDEVINFO handle ) { SetupDiDestroyDeviceInfoList( handle ); } );
	
	//Finally do the enumeration
	std::vector< PoiCommPort > result;
	for ( int nIndex = 0 ; ; ++nIndex ) {
		SP_DEVINFO_DATA devInfo;
		devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

		//Enumerate the current device
		if ( !SetupDiEnumDeviceInfo( hDevInfoSet, nIndex, &devInfo ) ) {
			break;
		}
		
		//Get the registry key which stores the ports settings
		HKEY deviceKey = SetupDiOpenDevRegKey( hDevInfoSet, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE );
		if ( deviceKey == INVALID_HANDLE_VALUE ) {
			continue;
		}
		guards.emplace_back( deviceKey, [] ( HKEY handle ) { RegCloseKey( handle ); } );
		
		int nPort = QueryRegistryPortName( deviceKey );
		if ( nPort == -1 ) {
			continue;
		}

		//If the port was a serial port, then also try to get its friendly name
		std::string szFriendlyName = QueryDeviceDescription( hDevInfoSet, devInfo );
		result.emplace_back( "COM" + std::to_string( nPort ), szFriendlyName );
	}
	
	return result;
}
