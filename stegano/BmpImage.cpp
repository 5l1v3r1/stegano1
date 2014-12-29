
#include "BmpImage.h"


bool CBmpImage::loadImage(const std::wstring& path)
{
	memset(&m_fileHdr, 0, sizeof(m_fileHdr));
	memset(&m_infoHdr, 0, sizeof(m_infoHdr));
	memset(&m_bmpv5Hdr, 0, sizeof(m_bmpv5Hdr));
	
	if (this->CImageFile::loadImage(path))
	{
		m_file.read(reinterpret_cast<char*>(&m_fileHdr), sizeof(m_fileHdr));
		m_file.read(reinterpret_cast<char*>(&m_infoHdr), sizeof(m_infoHdr));

		m_posOfPixelsMatrix = m_fileHdr.bfOffBits;
		m_width = m_infoHdr.biWidth;
		m_numberOfPixels = m_height * m_width;

		// According to the BMP specification - it may be negative value
		m_height = abs(m_infoHdr.biHeight);		


		if (m_infoHdr.biSize == sizeof(BITMAPINFOHEADER))
		{
			// Only BITMAPINFOHEADER accessible
		}
		else if (m_infoHdr.biSize == sizeof(BITMAPV4HEADER))
		{
			// BITMAPV4HEADER accessible
		}
		else if (m_infoHdr.biSize == sizeof(BITMAPV5HEADER))
		{
			// BITMAPV5HEADER accessible
			m_file.seekg(sizeof(m_fileHdr), std::ios_base::beg);
			m_file.read(reinterpret_cast<char*>(&m_bmpv5Hdr), sizeof(m_bmpv5Hdr));
		}

		return true;
	}
	else
	{
		return false;
	}
}


size_t CBmpImage::getDataFromHeader(unsigned char *data, size_t sizeOfBuffer)
{
	if (m_bmpv5Hdr.bV5Size != 0 && m_bmpv5Hdr.bV5CSType == PROFILE_EMBEDDED)
	{
		size_t read = 0;
		size_t pos = m_bmpv5Hdr.bV5ProfileData + sizeof(BITMAPFILEHEADER);
		size_t sizeOfData = m_bmpv5Hdr.bV5ProfileSize;

		size_t toRead = min(sizeOfData, sizeOfBuffer);
		m_file.seekg(pos, std::ios_base::beg);
		m_file.read(reinterpret_cast<char*>(data), toRead);
		read = static_cast<size_t>(m_file.gcount());

		return read;
	}
	else
	{
		// Let it be a marker to indicate that the regular decodeAppend should be invoked.
		m_error = BMP_ERROR_NO_METADATA_STRUCTURE;
		return 0;
	}
}


size_t CBmpImage::putDataToHeader(unsigned char *data, size_t sizeOfData)
{
	m_file.seekp(0, std::ios_base::end);
	size_t pos = static_cast<size_t>(m_file.tellp());

	if (m_bmpv5Hdr.bV5Size != 0)
	{
		m_bmpv5Hdr.bV5CSType = PROFILE_EMBEDDED;
		m_bmpv5Hdr.bV5ProfileData = pos - sizeof(BITMAPFILEHEADER);
		m_bmpv5Hdr.bV5ProfileSize = sizeOfData;
	}

	// Appending at the end of the file (where the actual ICC profiles headers exist)
	std::streampos processed = static_cast<std::streampos>(pos);
	m_file.write(reinterpret_cast<char*>(data), sizeOfData);
	processed = m_file.tellp() - processed;

	return static_cast<size_t>(processed);
}


uint32_t CBmpImage::getAbsolutePixelPos(uint32_t x, uint32_t y)
{
	// Apply row padding (rounded to 4-th boundary) to the absolute byte formula
	const char pad = 4 - (m_width % 4);

	//	Since the image is stored upside-down, we need to convert
	// y-th row number, as it was pointing from the end towards the beginning
	// of the pixels matrix.
	//	Also, because every row must be padded to the boundary of 4, whereas
	// we are dealing with 3-bytes aligned RGB structures, at the end of the row,
	// the to-the-four padding is being applied, so we have to take it into account
	// when computing pixel's absolute position.
	//	At the end, we have to multiple resulted position by 3 as it's the size
	// of the RGB structure per each pixel.
	uint32_t pos = m_posOfPixelsMatrix + 3 * ((m_height - y - 1) * m_width + x + y*pad);
	pos -= m_infoHdr.biBitCount / 8;

	return pos;
}


CImageFile::ImageColor CBmpImage::getPixel(uint32_t x, uint32_t y)
{
	ImageColor col = { 0, 0, 0 };

	size_t pos = getAbsolutePixelPos(x, y);

	const uint16_t bpp = m_infoHdr.biBitCount;

	if (bpp < 8)
	{
		// To low number of bits per pixel to store data without disturbing
		// the image visibly
		m_error = BMP_ERROR_INSUFFICIENT_BITNESS;
		return ImageColor();
	}

	size_t bytesPerPixel = bpp / 8;

	m_file.seekg(pos, std::ios_base::beg);
	m_file.read(reinterpret_cast<char*>(&col), bytesPerPixel);

	// In BMP file format - pixel's RGB structure is written backwards,
	// so the Red component must be swapped with the Blue one
	std::swap(col.r, col.b);

	return col;
}


void CBmpImage::setPixel(uint32_t x, uint32_t y, const ImageColor& col)
{
	size_t pos = getAbsolutePixelPos(x, y);

	const uint16_t bpp = m_infoHdr.biBitCount;

	if (bpp < 8)
	{
		// To low number of bits per pixel to store data without disturbing
		// the image visibly
		m_error = BMP_ERROR_INSUFFICIENT_BITNESS;
		return;
	}

	size_t bytesPerPixel = bpp / 8;

	ImageColor col2 = col;
	std::swap(col2.r, col2.b);

	m_file.seekp(pos, std::ios_base::beg);
	m_file.write(reinterpret_cast<char*>(&col2), bytesPerPixel);
}

