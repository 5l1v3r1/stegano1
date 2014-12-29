#include "PngImage.h"



bool CPngImage::loadImage(const std::wstring& path)
{


	return true;
}


size_t CPngImage::getDataFromHeader(unsigned char *data, size_t sizeOfBuffer)
{
	return 0;
}


size_t CPngImage::putDataToHeader(unsigned char *data, size_t sizeOfData)
{
	return 0;
}


CImageFile::ImageColor CPngImage::getPixel(uint32_t x, uint32_t y)
{
	ImageColor col;

	return col;
}


void CPngImage::setPixel(uint32_t x, uint32_t y, const ImageColor& color)
{

}
