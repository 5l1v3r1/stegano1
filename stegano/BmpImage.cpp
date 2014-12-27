

#include "BmpImage.h"




bool CBmpImage::loadImage(const std::wstring& path)
{
	m_file = std::fstream(path, std::ios_base::binary | std::ios_base::in | std::ios_base::out);

	if (!m_file.good() || !m_file.is_open())
	{
		return false;
	}

	return true;
}


uint32_t CBmpImage::getWidth()
{
	return m_width;
}


uint32_t CBmpImage::getHeight()
{
	return m_height;
}


uint32_t CBmpImage::getNumberOfPixels()
{
	return m_numberOfPixels;
}


size_t CBmpImage::getDataFromHeader(unsigned char *data, size_t sizeOfBuffer)
{
	return 0;
}


size_t CBmpImage::putDataToHeader(unsigned char *data, size_t sizeOfData)
{
	return 0;
}


CImageFile::ImageColor CBmpImage::getPixel(uint32_t x, uint32_t y)
{
	ImageColor col;

	return col;
}


void CBmpImage::setPixel(uint32_t x, uint32_t y, const ImageColor& color)
{

}

