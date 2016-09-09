#include <wx/filedlg.h>
#include <wx/gauge.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/statbmp.h>

#include "poi_accessor.hpp"
#include "poi_protocol.hpp"
#include "wx_artprovider.hpp"
#include "wx_frame.hpp"

#include "debug.hpp"

wxImage dataToImage( std::vector< uint8_t > const& data )
{
	wxLogNull logNull;

	size_t width { PoiImage::GetLineCount( data ) };
    wxImage result { (int) width, (int) teensyPoi::pixelCount };
    int x {};
    int y {};
    for ( auto it { data.begin() } ; it != data.end() ; ) {
		++it; // skip global field
        uint8_t b = *it++;
        uint8_t g = *it++;
        uint8_t r = *it++;
        result.SetRGB( x, y, r, g, b );
        y = ( y + 1 ) % (int) teensyPoi::pixelCount;
        if ( y == 0 ) ++x;
    }
    return result;
}

std::vector< uint8_t > imageToData( wxImage const& image )
{
    std::vector< uint8_t > result;
    result.reserve( (size_t) ( image.GetWidth() * image.GetHeight() * 3 ) );
    for ( int x { 0 } ; x < image.GetWidth() ; ++x ) {
        for ( int y { 0 } ; y < image.GetHeight() ; ++y ) {
            bool transparent { image.IsTransparent( x, y ) };
            result.push_back( 0xff ); // global field
            result.push_back( transparent ? (uint8_t) 0x00 : image.GetBlue( x, y ) );
            result.push_back( transparent ? 0x00 : image.GetGreen( x, y ) );
            result.push_back( transparent ? 0x00 : image.GetRed( x, y ) );
        }
    }
    return result;
}

wxBitmap createDisabledBitmap( wxBitmap const& bitmap )
{
	wxLogNull logNull;

	wxImage image = bitmap.ConvertToImage();
    for ( int x { 0 } ; x < image.GetWidth() ; ++x ) {
        for ( int y { 0 } ; y < image.GetHeight() ; ++y ) {
			if ( !image.IsTransparent( x, y ) ) {
				image.SetAlpha( x, y, 0x80 );
			}
        }
    }
    return wxBitmap( image );
}

PoiFrame::PoiFrame( PoiAccessor& accessor )
	: PoiFrameBase( nullptr )
	, readyBitmaps_ { { PoiState::DISCONNECTED, poiArt_IconRed },
                      { PoiState::CONNECTING,   poiArt_IconRed },
                      { PoiState::READY,        poiArt_IconGreen },
                      { PoiState::BUSY,         poiArt_IconYellow } }
	, progressGauge_( new wxGauge( statusBar_, wxID_ANY, 100 ) )
	, readyBitmap_( new wxStaticBitmap( statusBar_, wxID_ANY, wxNullBitmap ) )
	, accessor_( accessor )
{
	for ( auto id : { poiID_LOAD, poiID_SAVE, poiID_ADD, poiID_REMOVE, poiID_RESET } ) {
		toolBar_->SetToolDisabledBitmap( id, createDisabledBitmap( toolBar_->FindById( id )->GetBitmap() ) );
	}

    imageList_->InsertColumn( 0, "Name", wxLIST_FORMAT_RIGHT, 150 );
    imageList_->InsertColumn( 1, "Size", wxLIST_FORMAT_RIGHT, 50 );

	std::array< int, 3 > statusBarWidths { -1, 100, 16 };
	statusBar_->SetFieldsCount( (int) statusBarWidths.size(), &statusBarWidths[ 0 ] );

	UpdateCommPorts();

	statusBar_->Bind( wxEVT_SIZE, [this] ( wxSizeEvent& ) { OnStatusBarSize(); } );
	toolBar_->Bind( wxEVT_TOOL, [this] ( wxCommandEvent& ) { OnToolBarLoad(); }, poiID_LOAD );
	toolBar_->Bind( wxEVT_TOOL, [this] ( wxCommandEvent& ) { OnToolBarSave(); }, poiID_SAVE );
	toolBar_->Bind( wxEVT_TOOL, [this] ( wxCommandEvent& ) { OnToolBarAdd(); }, poiID_ADD );
	toolBar_->Bind( wxEVT_TOOL, [this] ( wxCommandEvent& ) { OnToolBarRemove(); }, poiID_REMOVE );
	toolBar_->Bind( wxEVT_TOOL, [this] ( wxCommandEvent& ) { OnToolBarReset(); }, poiID_RESET );
	portNameChoice_->Bind( wxEVT_CHOICE, [this] ( wxCommandEvent& ) { OnPortNameChoiceChoice(); } );
	imageList_->Bind( wxEVT_LIST_ITEM_SELECTED, [this] ( wxListEvent& ) { OnImageListSelected(); } );
	imageUpButton_->Bind( wxEVT_BUTTON, [this] ( wxCommandEvent& ) { OnImageUpButtonButton(); } );
	imageDownButton_->Bind( wxEVT_BUTTON, [this] ( wxCommandEvent& ) { OnImageDownButtonButton(); } );
	linesPerRoundButton_->Bind( wxEVT_BUTTON, [this] ( wxCommandEvent& ) { OnLinesPerRoundButtonButton(); } );

	accessor_.OnError.connect( [this] ( std::string const& error, bool reset ) {
        CallAfter( [=] { OnPoiError( error, reset ); } );
    } );
    accessor_.OnStateChanged.connect( [this] ( PoiState state ) {
        CallAfter( [=] { OnPoiStateChanged( state ); } );
    } );
    accessor_.OnProgress.connect( [this] ( int progress, int total ) {
        CallAfter( [=] { OnPoiProgress( progress, total ); } );
    } );
    accessor_.OnLoaded.connect( [this] ( PoiStructure structure ) {
        CallAfter( [=] { OnPoiLoaded( std::move( structure ) ); } );
    } );
    accessor_.OnSaved.connect( [this] {
		CallAfter( [=] { OnPoiSaved(); } );
    } );

    OnPoiStateChanged( accessor_.GetState() );
    OnPoiLoaded( {} );
}

WXLRESULT PoiFrame::MSWWindowProc( WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam )
{
	if ( nMsg == WM_DEVICECHANGE ) {
		UpdateCommPorts();
		return MSWDefWindowProc( nMsg, wParam, lParam );
	}
	return PoiFrameBase::MSWWindowProc( nMsg, wParam, lParam );
}

void PoiFrame::UpdateCommPorts()
{
	auto index { portNameChoice_->GetSelection() };
	auto portName { index != wxNOT_FOUND ? commPorts_[ index ].GetPortName() : "" };

	auto newIndex { wxNOT_FOUND };
	portNameChoice_->Clear();
	commPorts_ = PoiCommPort::Enumerate();
	for ( auto&& commPort : commPorts_ ) {
		portNameChoice_->Append(
				commPort.GetDescription().length() > 0
				? commPort.GetDescription() + " (" + commPort.GetPortName() + ")"
				: commPort.GetPortName() );
		if ( commPort.GetPortName() == portName ) {
			newIndex = portNameChoice_->GetCount() - 1;
		}
	}
	if ( newIndex != wxNOT_FOUND ) {
		portNameChoice_->SetSelection( newIndex );
	}
	else {
		OnPortNameChoiceChoice();
	}
}

void PoiFrame::AddImageListItem( int index, PoiImage const& image )
{
    imageList_->InsertItem( index, wxEmptyString );
	imageList_->SetItem( index, 1, wxString::Format( "%dx%d", (int) image.GetLineCount(), (int) teensyPoi::pixelCount ) );
    previews_.emplace_back( dataToImage( image.GetData() ) );
    imageList_->SetItemState( index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    EnumerateImageList();
}

void PoiFrame::DeleteImageListItem( int index )
{
	imageList_->DeleteItem( index );
	previews_.erase( previews_.begin() + index );
	if ( imageList_->GetItemCount() == 0 ) {
		OnImageListSelected();
		return;
	}
	imageList_->SetItemState( std::min( index, imageList_->GetItemCount() - 1 ), wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
	EnumerateImageList();
}

void PoiFrame::EnumerateImageList()
{
	for ( int i = 0 ; i < imageList_->GetItemCount() ; ++i ) {
		imageList_->SetItem( i, 0, wxString::Format( "Image %d", i + 1 ) );
	}
}

void PoiFrame::OnStatusBarSize()
{
	wxRect rect;

	statusBar_->GetFieldRect( 1, rect );
	progressGauge_->SetSize( rect );

	statusBar_->GetFieldRect( 2, rect );
	wxSize size = readyBitmap_->GetSize();
	readyBitmap_->Move( rect.x + ( rect.width - size.x ) / 2, rect.y + ( rect.height - size.y ) / 2 );
}

void PoiFrame::OnToolBarLoad()
{
    statusBar_->SetStatusText( "Downloading flash memory from poi..." );
    accessor_.Load();
}

void PoiFrame::OnToolBarSave()
{
    statusBar_->SetStatusText( "Uploading flash memory to poi..." );
    accessor_.Save( structure_ );
}

void PoiFrame::OnToolBarAdd()
{
    wxFileDialog fileDialog { this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "PNG files (*.png)|*.png" };
    if ( fileDialog.ShowModal() == wxID_CANCEL ) {
        return;
    }

	{
		wxLogNull logNull;
		wxImage preview { fileDialog.GetPath() };
		if ( (size_t) preview.GetHeight() != teensyPoi::pixelCount ) {
			int newWidth { (int) ( preview.GetWidth() * teensyPoi::pixelCount / preview.GetHeight() ) };
			preview.Rescale( newWidth, (int) teensyPoi::pixelCount );
		}
	}

    PoiImage image { teensyPoi::pixelCount * 4, imageToData( preview ) };
    AddImageListItem( imageList_->GetItemCount(), structure_.AddImage( std::move( image ) ) );
    usageText_->SetLabel( wxString::Format( "%d / %d (%d%%)", (int) structure_.GetBlocksUsed(), (int) teensyPoi::flashBlockCount, (int) ( structure_.GetBlocksUsed() * 100 / teensyPoi::flashBlockCount ) ) );
}

void PoiFrame::OnToolBarRemove()
{
	auto index { imageList_->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) };
	if ( index == wxNOT_FOUND ) {
		return;
	}

	if ( wxMessageBox( wxString::Format( "Really delete image %d from Poi?", (int) ( index + 1 ) ), wxEmptyString, wxYES_NO | wxICON_QUESTION | wxCENTRE, this ) == wxYES ) {
		DeleteImageListItem( index );
		structure_.DeleteImage( index );

	    usageText_->SetLabel( wxString::Format( "%d / %d (%d%%)", (int) structure_.GetBlocksUsed(), (int) teensyPoi::flashBlockCount, (int) ( structure_.GetBlocksUsed() * 100 / teensyPoi::flashBlockCount ) ) );

		if ( index == imageList_->GetItemCount() ) {
			--index;
		}
		if ( index != wxNOT_FOUND ) {
			imageList_->SetItemState( index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
		}
		else {
			widthText_->SetLabelText( wxEmptyString );
            heightText_->SetLabelText( wxEmptyString );
            linesPerRoundText_->SetLabelText( wxEmptyString );
            blocksUsedText_->SetLabelText( wxEmptyString );
            previewBitmap_->SetBitmap( wxNullBitmap );
            Layout();
		}
	}
}

void PoiFrame::OnToolBarReset()
{
	if ( wxMessageBox( "Do you really want to reset all content?", "Question", wxYES_NO | wxICON_QUESTION | wxCENTRE, this ) == wxYES ) {
		OnPoiLoaded( {} );
	}
}

void PoiFrame::OnPortNameChoiceChoice()
{
	auto index = portNameChoice_->GetSelection();
	auto selected = index != wxNOT_FOUND;
	toolBar_->EnableTool( poiID_LOAD, selected );
	toolBar_->EnableTool( poiID_SAVE, selected );
	toolBar_->EnableTool( poiID_ADD, selected );
	toolBar_->EnableTool( poiID_REMOVE, selected );
	toolBar_->EnableTool( poiID_RESET, selected );

	if ( selected ) {
		accessor_.SetPortName( commPorts_[ index ].GetPortName() );
		OnToolBarLoad();
	}
	else {
		OnPoiLoaded( {} );
	}
}

void PoiFrame::OnImageListSelected()
{
	auto index { imageList_->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) };
	auto selected { index != wxNOT_FOUND };

	imageUpButton_->Enable( selected && index != 0 );
	imageDownButton_->Enable( selected && index != imageList_->GetItemCount() - 1 );
	linesPerRoundButton_->Enable( selected );

	if ( selected ) {
		PoiImage const& image { structure_.GetImages()[ index ] };
		wxImage const& preview { previews_[ index ] };
		widthText_->SetLabelText( wxString::Format( "%d", (int) image.GetLineCount() ) );
		heightText_->SetLabelText( wxString::Format( "%d", (int) teensyPoi::pixelCount ) );
		linesPerRoundText_->SetLabelText( wxString::Format( "%d", (int) image.GetLinesPerRound() ) );
		blocksUsedText_->SetLabelText( wxString::Format( "%d", (int) image.GetBlocksUsed() ) );
		previewBitmap_->SetBitmap( preview );
	}
	else {
		widthText_->SetLabelText( wxEmptyString );
		heightText_->SetLabelText( wxEmptyString );
		linesPerRoundText_->SetLabelText( wxEmptyString );
		blocksUsedText_->SetLabelText( wxEmptyString );
		previewBitmap_->SetBitmap( wxNullBitmap );
	}
	Layout();
}

void PoiFrame::OnImageUpButtonButton()
{
	auto index { imageList_->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) };

	DeleteImageListItem( index );
	auto image { structure_.DeleteImage( index ) };

	AddImageListItem( index - 1, structure_.AddImage( index - 1, std::move( image ) ) );
}

void PoiFrame::OnImageDownButtonButton()
{
	auto index { imageList_->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) };

	DeleteImageListItem( index );
	auto image { structure_.DeleteImage( index ) };

	AddImageListItem( index + 1, structure_.AddImage( index + 1, std::move( image ) ) );
}

void PoiFrame::OnLinesPerRoundButtonButton()
{
    auto index { imageList_->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED ) };
    auto&& image { structure_.GetImage( index ) };
    auto value {
			wxGetNumberFromUser(
				"The number of lines shown in one round\n"
				"determines how often the image is going\n"
				"to be repeated when spinning.",
				"Lines per round", wxEmptyString, image.GetLinesPerRound(), 0, std::numeric_limits< uint16_t >::max(), this ) };
	if ( value != -1 ) {
		image.SetLinesPerRound( (uint16_t) value );
		linesPerRoundText_->SetLabelText( wxString::Format( "%d", (int) value ) );
	}
}

void PoiFrame::OnPoiError( std::string const& error, bool reset )
{
    wxMessageBox( error, wxEmptyString, wxOK | wxICON_ERROR | wxCENTRE );
    statusBar_->SetStatusText( wxEmptyString );
	progressGauge_->SetValue( 0 );

	if ( reset ) {
		OnPoiLoaded( {} );
	}
}

void PoiFrame::OnPoiStateChanged( PoiState state )
{
    readyBitmap_->SetBitmap( wxArtProvider::GetBitmap( readyBitmaps_[ state ] ) );
    /*
	if ( state != PoiState::CONNECTING && state != PoiState::BUSY ) {
		statusBar_->SetStatusText( wxEmptyString );
		progressGauge_->SetValue( 0 );
	}
	*/
}

void PoiFrame::OnPoiProgress( int progress, int total )
{
	if ( progress == -1 ) {
		progressGauge_->Pulse();
	}
	else {
		progressGauge_->SetRange( total );
		progressGauge_->SetValue( progress );
	}
}

void PoiFrame::OnPoiLoaded( PoiStructure structure )
{
    structure_ = std::move( structure );

	usageText_->SetLabel( wxString::Format( "%d / %d (%d%%)", (int) structure_.GetBlocksUsed(), (int) teensyPoi::flashBlockCount, (int) ( structure_.GetBlocksUsed() * 100 / teensyPoi::flashBlockCount ) ) );
    imageList_->DeleteAllItems();
    OnImageListSelected();
    previews_.clear();
    for ( auto&& image : structure_.GetImages() ) {
        AddImageListItem( imageList_->GetItemCount(), image );
    }
	statusBar_->SetStatusText( wxEmptyString );
	progressGauge_->SetValue( 0 );
}

void PoiFrame::OnPoiSaved()
{
	statusBar_->SetStatusText( wxEmptyString );
	progressGauge_->SetValue( 0 );
}
