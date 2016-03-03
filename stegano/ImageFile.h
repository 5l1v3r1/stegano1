#pragma once

#include <string>
#include <cstdint>
#include <fstream>


class CImageFile
{
protected:

	// This class shall be used only as a sort of interface for other classes in inheritance.
	// It is not supposed to construct objects.
	CImageFile() : m_error(0) {}

public:

	struct ImageColor
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t alpha;
	};

	struct ImagePixel
	{
		uint32_t x;
		uint32_t y;
		ImageColor color;
	};

	virtual std::fstream& getFile() final { return m_file; }
	virtual uint32_t getError() final { return m_error; }	
	virtual uint32_t getWidth() final { return m_width; }
	virtual uint32_t getHeight() final { return m_height; }
	virtual uint32_t getNumberOfPixels() final { return m_numberOfPixels; }

	uint32_t getFileSize(){ return m_fileSize; }

	//
	// To be overridden by child classes
	//

	// This particular routine has to be called by every child class in their overridden loadImage 
	virtual bool loadImage(const std::wstring& path)
	{  
		m_file = std::fstream(path, std::ios_base::binary | std::ios_base::in | std::ios_base::out);
		setFileSize();
		return (m_file.good() && m_file.is_open());
	}

	virtual size_t getDataFromHeader(char *data, size_t sizeOfBuffer){ return 0; }
	virtual size_t putDataToHeader(char *data, size_t sizeOfData){ return 0; }
	
	virtual ImageColor getPixel(uint32_t x, uint32_t y){ return ImageColor(); }
	virtual void setPixel(uint32_t x, uint32_t y, const ImageColor& color){}

private:

	void setFileSize()
	{
		if (m_file.good() && m_file.is_open())
		{
			size_t prev = m_file.tellg();
			m_file.seekg(0, std::ios_base::end);
			m_fileSize = m_file.tellg();
			m_file.seekg(prev, std::ios_base::beg);
		}
	}

protected:
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_numberOfPixels;

	std::fstream m_file;
	uint32_t m_fileSize;
	uint32_t m_error;
};

