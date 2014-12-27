#pragma once

#include <string>
#include <cstdint>
#include <fstream>


class CImageFile
{
protected:

	CImageFile(){}

public:

	struct ImageColor
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};

	struct ImagePixel
	{
		uint32_t x;
		uint32_t y;
		ImageColor color;
	};

	std::fstream& getFile(){ return m_file; }

	virtual bool loadImage(const std::wstring& path){ return false; }
	
	virtual uint32_t getWidth(){ return 0; }
	virtual uint32_t getHeight(){ return 0; }
	virtual uint32_t getNumberOfPixels(){ return 0; }

	virtual size_t getDataFromHeader(unsigned char *data, size_t sizeOfBuffer){ return 0; }
	virtual size_t putDataToHeader(unsigned char *data, size_t sizeOfData){ return 0; }
	
	virtual ImageColor getPixel(uint32_t x, uint32_t y){ return ImageColor(); }
	virtual void setPixel(uint32_t x, uint32_t y, const ImageColor& color){}


protected:
	std::fstream m_file;
};

