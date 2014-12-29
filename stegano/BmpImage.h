#pragma once

#include "ImageFile.h"
#include "BmpImage.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wingdi.h>


#define BMP_ERROR_NO_METADATA_STRUCTURE 0xBADC0DE1
#define BMP_ERROR_INSUFFICIENT_BITNESS	0xBADC0DE2



class CBmpImage : public CImageFile
{
public:
	CBmpImage() : CImageFile() {}
	~CBmpImage(){}

	virtual bool loadImage(const std::wstring& path) override;

	// In case of BMP file format we will be dealing with the ICC profiles
	// header as a container for additional data.
	// ICC Profiles header don't have any particular structure, so we will treat
	// this as a simple appending method. Only certain header fields have to be corrected.
	virtual size_t getDataFromHeader(unsigned char *data, size_t sizeOfBuffer) override;
	virtual size_t putDataToHeader(unsigned char *data, size_t sizeOfData) override;

	virtual CImageFile::ImageColor getPixel(uint32_t x, uint32_t y) override;
	virtual void setPixel(uint32_t x, uint32_t y, const CImageFile::ImageColor& color) override;

protected:

	uint32_t getAbsolutePixelPos(uint32_t x, uint32_t y);


private:
	uint32_t m_posOfPixelsMatrix;

	BITMAPFILEHEADER m_fileHdr;
	BITMAPINFOHEADER m_infoHdr;
	BITMAPV5HEADER m_bmpv5Hdr;
};

