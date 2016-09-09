///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __WX_GENERATED_H__
#define __WX_GENERATED_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/statusbr.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/toolbar.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/gbsizer.h>
#include <wx/statbmp.h>
#include <wx/frame.h>

#include "wx_artprovider.hpp"

///////////////////////////////////////////////////////////////////////////

#define poiID_LOAD 1000
#define poiID_SAVE 1001
#define poiID_ADD 1002
#define poiID_REMOVE 1003
#define poiID_RESET 1004

///////////////////////////////////////////////////////////////////////////////
/// Class PoiFrameBase
///////////////////////////////////////////////////////////////////////////////
class PoiFrameBase : public wxFrame 
{
	private:
	
	protected:
		wxStatusBar* statusBar_;
		wxToolBar* toolBar_;
		wxToolBarToolBase* loadTool_; 
		wxToolBarToolBase* saveTool_; 
		wxToolBarToolBase* addTool_; 
		wxToolBarToolBase* deleteTool_; 
		wxToolBarToolBase* resetTool_; 
		wxStaticText* portNameLabel_;
		wxChoice* portNameChoice_;
		wxListCtrl* imageList_;
		wxStaticText* usageLabel_;
		wxStaticText* usageText_;
		wxStaticText* imageListLabel_;
		wxBitmapButton* imageUpButton_;
		wxBitmapButton* imageDownButton_;
		wxStaticText* widthLabel_;
		wxStaticText* widthText_;
		wxStaticText* heightLabel_;
		wxStaticText* heightText_;
		wxStaticText* linesPerRoundLabel_;
		wxStaticText* linesPerRoundText_;
		wxButton* linesPerRoundButton_;
		wxStaticText* blocksUsedLabel_;
		wxStaticText* blocksUsedText_;
		wxStaticText* previewLabel_;
		wxStaticBitmap* previewBitmap_;
	
	public:
		
		PoiFrameBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("TeensyPoi Uploader"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 600,400 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~PoiFrameBase();
	
};

#endif //__WX_GENERATED_H__
