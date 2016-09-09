#include <wx/log.h>

#include "wx_artprovider.hpp"

wxArtID const poiArt_IconGreen    { "icons/green.png"    };
wxArtID const poiArt_IconRed      { "icons/red.png"      };
wxArtID const poiArt_IconYellow   { "icons/yellow.png"   };
wxArtID const poiArt_IconUpload   { "icons/upload.png"   };
wxArtID const poiArt_IconDownload { "icons/download.png" };
wxArtID const poiArt_IconOpen     { "icons/open.png"     };
wxArtID const poiArt_IconRemove   { "icons/remove.png"   };
wxArtID const poiArt_IconReset    { "icons/reset.png"    };
wxArtID const poiArt_IconSettings { "icons/settings.png" };
wxArtID const poiArt_IconUp       { "icons/up.png"       };
wxArtID const poiArt_IconDown     { "icons/down.png"     };

wxBitmap PoiArtProvider::CreateBitmap( wxArtID const& id, wxArtClient const& client, wxSize const& size )
{
	wxLogNull logNull;
	return wxBitmap( id, wxBITMAP_TYPE_PNG );
}
