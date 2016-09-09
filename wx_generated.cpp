///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx_generated.h"

///////////////////////////////////////////////////////////////////////////

PoiFrameBase::PoiFrameBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 450,350 ), wxDefaultSize );
	this->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	
	statusBar_ = this->CreateStatusBar( 3, wxST_SIZEGRIP, wxID_ANY );
	toolBar_ = this->CreateToolBar( wxTB_HORIZONTAL, wxID_ANY ); 
	loadTool_ = toolBar_->AddTool( poiID_LOAD, _("tool"), wxArtProvider::GetBitmap( poiArt_IconDownload, wxART_TOOLBAR ), wxNullBitmap, wxITEM_NORMAL, _("Load"), _("Read existing data from TeensyPoi"), NULL ); 
	
	saveTool_ = toolBar_->AddTool( poiID_SAVE, _("tool"), wxArtProvider::GetBitmap( poiArt_IconUpload, wxART_TOOLBAR ), wxNullBitmap, wxITEM_NORMAL, _("Save"), _("Write pending changes to TeensyPoi"), NULL ); 
	
	toolBar_->AddSeparator(); 
	
	addTool_ = toolBar_->AddTool( poiID_ADD, _("tool"), wxArtProvider::GetBitmap( poiArt_IconOpen, wxART_TOOLBAR ), wxNullBitmap, wxITEM_NORMAL, _("Add image"), _("Loads and adds a new image"), NULL ); 
	
	deleteTool_ = toolBar_->AddTool( poiID_REMOVE, _("tool"), wxArtProvider::GetBitmap( poiArt_IconRemove, wxART_TOOLBAR ), wxNullBitmap, wxITEM_NORMAL, _("Remove image"), _("Remove the selected image"), NULL ); 
	
	toolBar_->AddSeparator(); 
	
	resetTool_ = toolBar_->AddTool( poiID_RESET, _("tool"), wxArtProvider::GetBitmap( poiArt_IconReset, wxART_TOOLBAR ), wxNullBitmap, wxITEM_NORMAL, _("Reset"), _("Reset all content to empty"), NULL ); 
	
	toolBar_->AddSeparator(); 
	
	portNameLabel_ = new wxStaticText( toolBar_, wxID_ANY, _("Port:"), wxDefaultPosition, wxDefaultSize, 0 );
	portNameLabel_->Wrap( -1 );
	toolBar_->AddControl( portNameLabel_ );
	toolBar_->AddSeparator(); 
	
	wxArrayString portNameChoice_Choices;
	portNameChoice_ = new wxChoice( toolBar_, wxID_ANY, wxDefaultPosition, wxSize( 250,-1 ), portNameChoice_Choices, 0 );
	portNameChoice_->SetSelection( 0 );
	toolBar_->AddControl( portNameChoice_ );
	toolBar_->Realize(); 
	
	wxGridBagSizer* gbSizer3;
	gbSizer3 = new wxGridBagSizer( 0, 0 );
	gbSizer3->SetFlexibleDirection( wxBOTH );
	gbSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxGridBagSizer* gbSizer4;
	gbSizer4 = new wxGridBagSizer( 0, 0 );
	gbSizer4->SetFlexibleDirection( wxBOTH );
	gbSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	imageList_ = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );
	imageList_->SetMinSize( wxSize( 200,-1 ) );
	
	gbSizer4->Add( imageList_, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 0 );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );
	
	usageLabel_ = new wxStaticText( this, wxID_ANY, _("Blocks total:"), wxDefaultPosition, wxDefaultSize, 0 );
	usageLabel_->Wrap( -1 );
	bSizer1->Add( usageLabel_, 0, wxALL, 5 );
	
	usageText_ = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	usageText_->Wrap( -1 );
	usageText_->SetMinSize( wxSize( 90,-1 ) );
	
	bSizer1->Add( usageText_, 0, wxALL, 5 );
	
	
	gbSizer4->Add( bSizer1, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_RIGHT, 5 );
	
	wxGridBagSizer* gbSizer41;
	gbSizer41 = new wxGridBagSizer( 0, 0 );
	gbSizer41->SetFlexibleDirection( wxHORIZONTAL );
	gbSizer41->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	imageListLabel_ = new wxStaticText( this, wxID_ANY, _("Available images:"), wxDefaultPosition, wxDefaultSize, 0 );
	imageListLabel_->Wrap( -1 );
	gbSizer41->Add( imageListLabel_, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	imageUpButton_ = new wxBitmapButton( this, wxID_ANY, wxArtProvider::GetBitmap( poiArt_IconUp, wxART_BUTTON ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	imageUpButton_->Enable( false );
	
	gbSizer41->Add( imageUpButton_, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALL, 0 );
	
	imageDownButton_ = new wxBitmapButton( this, wxID_ANY, wxArtProvider::GetBitmap( poiArt_IconDown, wxART_BUTTON ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	imageDownButton_->Enable( false );
	
	gbSizer41->Add( imageDownButton_, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALL, 0 );
	
	
	gbSizer41->AddGrowableCol( 0 );
	
	gbSizer4->Add( gbSizer41, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	
	gbSizer4->AddGrowableCol( 0 );
	gbSizer4->AddGrowableRow( 1 );
	
	gbSizer3->Add( gbSizer4, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 5 );
	
	wxGridBagSizer* gbSizer5;
	gbSizer5 = new wxGridBagSizer( 0, 0 );
	gbSizer5->SetFlexibleDirection( wxBOTH );
	gbSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	widthLabel_ = new wxStaticText( this, wxID_ANY, _("Width (no. of lines):"), wxDefaultPosition, wxDefaultSize, 0 );
	widthLabel_->Wrap( -1 );
	gbSizer5->Add( widthLabel_, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	widthText_ = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	widthText_->Wrap( -1 );
	widthText_->SetMinSize( wxSize( 40,-1 ) );
	
	gbSizer5->Add( widthText_, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );
	
	heightLabel_ = new wxStaticText( this, wxID_ANY, _("Height (fixed):"), wxDefaultPosition, wxDefaultSize, 0 );
	heightLabel_->Wrap( -1 );
	gbSizer5->Add( heightLabel_, wxGBPosition( 0, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	heightText_ = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	heightText_->Wrap( -1 );
	heightText_->SetMinSize( wxSize( 40,-1 ) );
	
	gbSizer5->Add( heightText_, wxGBPosition( 0, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );
	
	linesPerRoundLabel_ = new wxStaticText( this, wxID_ANY, _("Lines per round:"), wxDefaultPosition, wxDefaultSize, 0 );
	linesPerRoundLabel_->Wrap( -1 );
	gbSizer5->Add( linesPerRoundLabel_, wxGBPosition( 1, 2 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	linesPerRoundText_ = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	linesPerRoundText_->Wrap( -1 );
	linesPerRoundText_->SetMinSize( wxSize( 40,-1 ) );
	
	gbSizer5->Add( linesPerRoundText_, wxGBPosition( 1, 3 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	linesPerRoundButton_ = new wxButton( this, wxID_ANY, _("..."), wxDefaultPosition, wxSize( -1,-1 ), wxBU_EXACTFIT );
	linesPerRoundButton_->Enable( false );
	
	gbSizer5->Add( linesPerRoundButton_, wxGBPosition( 1, 4 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	blocksUsedLabel_ = new wxStaticText( this, wxID_ANY, _("Blocks used:"), wxDefaultPosition, wxDefaultSize, 0 );
	blocksUsedLabel_->Wrap( -1 );
	gbSizer5->Add( blocksUsedLabel_, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	blocksUsedText_ = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
	blocksUsedText_->Wrap( -1 );
	blocksUsedText_->SetMinSize( wxSize( 40,-1 ) );
	
	gbSizer5->Add( blocksUsedText_, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );
	
	
	gbSizer5->Add( 0, 0, wxGBPosition( 0, 4 ), wxGBSpan( 1, 1 ), wxEXPAND, 5 );
	
	previewLabel_ = new wxStaticText( this, wxID_ANY, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0 );
	previewLabel_->Wrap( -1 );
	gbSizer5->Add( previewLabel_, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, 5 );
	
	previewBitmap_ = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	gbSizer5->Add( previewBitmap_, wxGBPosition( 3, 0 ), wxGBSpan( 1, 5 ), wxALL|wxEXPAND, 5 );
	
	
	gbSizer5->AddGrowableCol( 4 );
	gbSizer5->AddGrowableRow( 3 );
	
	gbSizer3->Add( gbSizer5, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 5 );
	
	
	gbSizer3->AddGrowableCol( 1 );
	gbSizer3->AddGrowableRow( 0 );
	
	this->SetSizer( gbSizer3 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

PoiFrameBase::~PoiFrameBase()
{
}
