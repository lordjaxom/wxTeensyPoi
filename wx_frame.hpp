#ifndef TPC_WX_FRAME_HPP
#define TPC_WX_FRAME_HPP

#include <wx/image.h>

#include <map>
#include <vector>

#include "poi_commport.hpp"
#include "poi_state.hpp"
#include "poi_structure.hpp"
#include "wx_generated.h"

class wxGauge;
class wxStaticBitmap;

class PoiAccessor;

class PoiFrame : public PoiFrameBase
{
public:
	PoiFrame( PoiAccessor& accessor );
	PoiFrame( PoiFrame const& ) = delete;

protected:
	virtual WXLRESULT MSWWindowProc( WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam ) override;

private:
	void UpdateCommPorts();
    void AddImageListItem( int index, PoiImage const& image );
    void DeleteImageListItem( int index );
    void EnumerateImageList();

	void OnStatusBarSize();

	void OnToolBarLoad();
	void OnToolBarSave();
	void OnToolBarAdd();
	void OnToolBarRemove();
	void OnToolBarReset();
	void OnPortNameChoiceChoice();
	void OnImageListSelected();
	void OnImageUpButtonButton();
	void OnImageDownButtonButton();
	void OnLinesPerRoundButtonButton();

	void OnPoiError( std::string const& error, bool reset );
	void OnPoiStateChanged( PoiState state );
	void OnPoiProgress( int progress, int total );
	void OnPoiLoaded( PoiStructure structure );
	void OnPoiSaved();

	std::map< PoiState, wxArtID > readyBitmaps_;
	wxGauge* progressGauge_;
	wxStaticBitmap* readyBitmap_;
	PoiAccessor& accessor_;
	PoiStructure structure_;
	std::vector< PoiCommPort > commPorts_;
	std::vector< wxImage > previews_;
};

#endif // TPC_WX_FRAME_HPP
