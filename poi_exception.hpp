#ifndef TPC_POI_EXCEPTION_HPP
#define TPC_POI_EXCEPTION_HPP

#include <stdexcept>

class PoiException : public std::runtime_error
{
public:
    PoiException( std::string const& what, bool reset = false )
        : std::runtime_error( what )
        , reset_ { reset }
    {
    }

    bool IsReset() const { return reset_; }

private:
	bool reset_;
};

#endif // TPC_POI_EXCEPTION_HPP
