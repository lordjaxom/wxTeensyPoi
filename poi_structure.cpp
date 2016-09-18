#include <limits>

#include "poi_exception.hpp"
#include "poi_structure.hpp"
#include "poi_protocol.hpp"

#include "debug.hpp"

PoiImage::PoiImage( uint16_t linesPerRound, std::vector< uint8_t > data, bool dirty )
    : linesPerRound_ { linesPerRound }
    , data_ { std::move( data ) }
    , headerDirty_ { dirty }
    , dataDirty_ { dirty }
{
}

size_t PoiImage::GetBlocksUsed() const
{
    size_t firstBlockAvail = teensyPoi::flashBlockSize - sizeof( teensyPoi::FlashImageFileHeader );
    if ( data_.size() < firstBlockAvail ) {
        return 1;
    }
    size_t lengthRemain = data_.size() - firstBlockAvail;
    return 1 + lengthRemain / teensyPoi::flashBlockSize + ( lengthRemain % teensyPoi::flashBlockSize > 0 ? 1 : 0 );
}

PoiImageLoader::PoiImageLoader( PoiStructure& structure )
    : structure_ { structure }
    , used_ {}
{
}

void PoiImageLoader::AddHeader( uint32_t fileSize, uint16_t linesPerRound )
{
	if ( used_ ) {
		throw PoiException(
                "Image incomplete: Expected " + std::to_string( data_.size() ) + " bytes, but got " +
                std::to_string( std::distance( dataIt_, data_.end() ) ) + ". Starting with empty structure." );
	}

	used_ = true;
	linesPerRound_ = linesPerRound;
	data_.resize( fileSize );
	dataIt_ = data_.begin();
}

void PoiImageLoader::AddData( uint8_t const* data, size_t available )
{
    if ( !used_ ) {
        throw PoiException( "Image incomplete: Missing image header. Starting with empty structure." );
    }

	size_t length { std::min( (size_t) std::distance( dataIt_, data_.end() ), available ) };
    dataIt_ = std::copy_n( data, length, dataIt_ );
    if ( dataIt_ == data_.end() ) {
        structure_.AddImage( { linesPerRound_, std::move( data_ ), false } );
        used_ = false;
    }
}

void PoiImageLoader::AddEod()
{
    if ( used_ ) {
        throw PoiException(
                "Image incomplete: Expected " + std::to_string( std::distance( dataIt_, data_.end() ) ) +
                " upon End-Of-Data. Starting with empty structure." );
    }
}

PoiStructure::PoiStructure( bool dirty )
	: dirty_( dirty )
{
}

PoiImage const& PoiStructure::AddImage( size_t index, PoiImage image )
{
    size_t blocksAvailable { teensyPoi::flashBlockCount - blocksUsed_ };
    if ( image.GetBlocksUsed() > blocksAvailable ) {
        throw PoiException(
                "Image too large: Would use " + std::to_string( image.GetBlocksUsed() ) + " blocks, but only " +
                std::to_string( blocksAvailable ) + " available." );
    }
    blocksUsed_ += image.GetBlocksUsed();
    images_.emplace( images_.begin() + index, std::move( image ) );
    if ( image.IsDirty() ) {
		dirty_ = true;
    }
    return images_[ index ];
}

PoiImage const& PoiStructure::AddImage( PoiImage image )
{
	return AddImage( images_.size(), std::move( image ) );
}

PoiImage PoiStructure::DeleteImage( size_t index )
{
    auto it { std::next( images_.begin(), index ) };
    PoiImage image { std::move( *it ) };
	it = images_.erase( it );
    blocksUsed_ -= image.GetBlocksUsed();
	dirty_ = true;
	std::for_each( it, images_.end(), []( PoiImage& img ) { img.Dirty( true ); } );
	return image;
}
