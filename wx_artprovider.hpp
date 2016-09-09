#ifndef TPC_WX_ARTPROVIDER_HPP
#define TPC_WX_ARTPROVIDER_HPP

#include <wx/artprov.h>

class PoiArtProvider : public wxArtProvider
{
	typedef char const* const* xpm_type;

public:
	PoiArtProvider() = default;
	PoiArtProvider( PoiArtProvider const& ) = delete;

protected:
	virtual wxBitmap CreateBitmap( wxArtID const& id, wxArtClient const& client, wxSize const& size ) override;
};

extern wxArtID const poiArt_IconGreen;
extern wxArtID const poiArt_IconRed;
extern wxArtID const poiArt_IconYellow;
extern wxArtID const poiArt_IconUpload;
extern wxArtID const poiArt_IconDownload;
extern wxArtID const poiArt_IconOpen;
extern wxArtID const poiArt_IconRemove;
extern wxArtID const poiArt_IconReset;
extern wxArtID const poiArt_IconSettings;
extern wxArtID const poiArt_IconUp;
extern wxArtID const poiArt_IconDown;

#endif // TPC_WX_ARTPROVIDER_HPP
