#pragma once

#include "ImageFile.h"


class CJpegImage : public CImageFile
{
public:
	CJpegImage() : CImageFile() {}
	~CJpegImage(){}

	virtual bool loadImage(const std::wstring& path) override;

	virtual size_t getDataFromHeader(unsigned char *data, size_t sizeOfBuffer) override;
	virtual size_t putDataToHeader(unsigned char *data, size_t sizeOfData) override;

	virtual CImageFile::ImageColor getPixel(uint32_t x, uint32_t y) override;
	virtual void setPixel(uint32_t x, uint32_t y, const CImageFile::ImageColor& color) override;


private:

};

