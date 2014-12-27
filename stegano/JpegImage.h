#pragma once

#include "ImageFile.h"


class CJpegImage : public CImageFile
{
public:
	CJpegImage() : CImageFile() {}
	~CJpegImage(){}

	virtual bool loadImage(const std::wstring& path) override;

	virtual uint32_t getWidth() override;
	virtual uint32_t getHeight() override;
	virtual uint32_t getNumberOfPixels() override;

	virtual size_t getDataFromHeader(unsigned char *data, size_t sizeOfBuffer) override;
	virtual size_t putDataToHeader(unsigned char *data, size_t sizeOfData) override;

	virtual CImageFile::ImageColor getPixel(uint32_t x, uint32_t y) override;
	virtual void setPixel(uint32_t x, uint32_t y, const CImageFile::ImageColor& color) override;


private:
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_numberOfPixels;
};

