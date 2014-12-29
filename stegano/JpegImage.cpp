#include "JpegImage.h"



bool CJpegImage::loadImage(const std::wstring& path)
{


	return true;
}


size_t CJpegImage::getDataFromHeader(unsigned char *data, size_t sizeOfBuffer)
{
	return 0;
}


size_t CJpegImage::putDataToHeader(unsigned char *data, size_t sizeOfData)
{
	return 0;
}


CImageFile::ImageColor CJpegImage::getPixel(uint32_t x, uint32_t y)
{
	ImageColor col;

	return col;
}


void CJpegImage::setPixel(uint32_t x, uint32_t y, const ImageColor& color)
{

}
