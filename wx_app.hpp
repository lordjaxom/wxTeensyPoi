#ifndef TPC_WX_APP_HPP
#define TPC_WX_APP_HPP

#include <wx/app.h>

#include "poi_accessor.hpp"

class PoiApp : public wxApp
{
public:
	PoiApp() = default;
	PoiApp( PoiApp const& ) = delete;

	virtual bool OnInit() override;

private:
    PoiAccessor accessor_;
};

#endif // TPC_WX_APP_HPP
