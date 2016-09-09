#ifndef TPC_CTR_COMMPORT_HPP
#define TPC_CTR_COMMPORT_HPP

#include <string>
#include <vector>

class PoiCommPort
{
public:
	static std::vector< PoiCommPort > Enumerate();

	PoiCommPort( std::string portName, std::string description )
		: portName_( std::move( portName ) )
		, description_( std::move( description ) )
	{
	}
	
	std::string const& GetPortName() const { return portName_; }
	std::string const& GetDescription() const { return description_; }
	
private:
	std::string portName_;
	std::string description_;
};

#endif // TPC_CTR_COMMPORT_HPP