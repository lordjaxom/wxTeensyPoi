#include <wx/textdlg.h>

#include "wx_app.hpp"
#include "wx_artprovider.hpp"
#include "wx_frame.hpp"

bool PoiApp::OnInit()
{
	wxImage::AddHandler( new wxPNGHandler );

	wxArtProvider::Push( new PoiArtProvider );

	auto frame = new PoiFrame( accessor_ );
	frame->Show( true );
	return true;
}

wxIMPLEMENT_APP( PoiApp );
