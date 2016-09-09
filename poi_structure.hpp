#ifndef TPC_POI_STRUCTURE_HPP
#define TPC_POI_STRUCTURE_HPP

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "poi_protocol.hpp"

class PoiStructure;

class PoiImage
{
public:
	static uint16_t GetLineCount( std::vector< uint8_t > const& data )
	{
		return data.size() / ( teensyPoi::pixelCount * 4 );
	}

    PoiImage( uint16_t linesPerRound, std::vector< uint8_t > data, bool dirty = true );

    uint16_t GetLineCount() const { return GetLineCount( data_ ); }
    uint16_t GetLinesPerRound() const { return linesPerRound_; }
    std::vector< uint8_t > const& GetData() const { return data_; }

    bool IsDirty() const { return headerDirty_ || dataDirty_; }
    bool IsDataDirty() const { return dataDirty_; }

    void SetLinesPerRound( uint16_t value ) { linesPerRound_ = value; headerDirty_ = true; }

    void Dirty( bool value ) const { headerDirty_ = dataDirty_ = value; }

    uint32_t GetFileSize() const { return data_.size(); }
    size_t GetBlocksUsed() const;

private:
    uint16_t linesPerRound_;
    std::vector< uint8_t > data_;
    mutable bool headerDirty_;
    mutable bool dataDirty_;
};

class PoiImageLoader
{
public:
    PoiImageLoader( PoiStructure& structure );
    PoiImageLoader( PoiImageLoader const& ) = delete;

	bool IsUsed() const { return used_; }

    void AddHeader( uint32_t fileSize, uint16_t linesPerSecond );
    void AddData( uint8_t const* data, size_t available );
    void AddEod();

private:
    PoiStructure& structure_;
    bool used_;
    uint16_t linesPerRound_;
    std::vector< uint8_t > data_;
    std::vector< uint8_t >::iterator dataIt_;
};

class PoiStructure
{
public:
	PoiStructure( bool dirty = true );

    size_t GetBlocksUsed() const { return blocksUsed_; }
    std::vector< PoiImage > const& GetImages() const { return images_; }
    PoiImage& GetImage( size_t index ) { return images_[ index ]; }

    bool IsDirty() const { return dirty_; }

    PoiImage const& AddImage( size_t index, PoiImage image );
    PoiImage const& AddImage( PoiImage image );
	PoiImage DeleteImage( size_t index );

private:
    size_t blocksUsed_ {};
    std::vector< PoiImage > images_;
    bool dirty_;
};

#endif // TPC_POI_STRUCTURE_HPP
