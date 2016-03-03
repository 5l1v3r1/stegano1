
#include "Stegano.h"
#include "BmpImage.h"
#include "PngImage.h"
#include "JpegImage.h"

#include <memory>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iomanip>

using std::cout;
using std::wcout;
using std::endl;


const wchar_t *getMethodName(CStegano::SteganoMethod method)
{
	switch (method)
	{
	case CStegano::SteganoMethod::Stegano_LSB: return L"LSB";
	case CStegano::SteganoMethod::Stegano_LSB_IncDec: return L"LSB IncDec";
	case CStegano::SteganoMethod::Stegano_LSB_Edges: return L"LSB Edges";
	case CStegano::SteganoMethod::Stegano_2LSB_Color: return L"2LSB Color";
	case CStegano::SteganoMethod::Stegano_Metadata: return L"Metadata";
	case CStegano::SteganoMethod::Stegano_Append: return L"Append";
	}

	return L"Unknown";
}


bool CStegano::init(const std::wstring& path, SteganoMethod method, EncodingSchemes scheme, bool verbose)
{
	m_method = method;
	m_encodingScheme = scheme;
	m_imageFormat = Image_Unknown;
	memset(&m_lastOperationHeader, 0, sizeof(m_lastOperationHeader));

	m_imageFile = loadImage(path);

	bool ret = m_imageFile->loadImage(path);
	m_file = &m_imageFile->getFile();

	m_numOfPixels = m_imageFile->getNumberOfPixels();
	m_verbose = verbose;

	return ret;
}


CImageFile *CStegano::loadImage(const std::wstring& path)
{
	char firstBytes[5];
	bool isBmp = false;
	bool isPng = false;
	bool isJpg = false;

	std::fstream tmpFile = std::fstream(path, std::ios_base::binary | std::ios_base::in);

	// Firstly - try to recognize image format by parsing first couple of header bytes
	if (!tmpFile.fail() && tmpFile.is_open() )
	{
		tmpFile.seekg(0, std::ios_base::beg);
		tmpFile.read(firstBytes, sizeof(firstBytes));

		if (!tmpFile.fail())
		{
			if (!memcmp(firstBytes, "BM", 2))
			{
				m_imageFormat = Image_Bmp;
				tmpFile.close();
				return new CBmpImage();
			}
			else if (!memcmp(&firstBytes[1], "PNG", 3))
			{
				m_imageFormat = Image_Png;
				tmpFile.close();
				return new CPngImage();
			}
			else if (firstBytes[0] == 0xFF && firstBytes[1] == 0xD8 && firstBytes[2] == 0xFF)
			{
				m_imageFormat = Image_Jpg;
				tmpFile.close();
				return new CJpegImage();
			}
		}

		tmpFile.close();
	}

	// If the header-bytes based recognition has failed, try to interpret file's extension.
	if (path.find(L".png") != std::wstring::npos)
	{
		m_imageFormat = Image_Png;
		return new CPngImage();
	}
	else if (path.find(L".bmp") != std::wstring::npos)
	{
		m_imageFormat = Image_Bmp;
		return new CBmpImage();
	}
	else if (path.find(L".jpg") != std::wstring::npos || path.find(L".jpeg") != std::wstring::npos)
	{
		m_imageFormat = Image_Jpg;
		return new CJpegImage();
	}
	else
	{
		return nullptr;
	}
}


size_t CStegano::encode(char *toWrite, size_t size)
{
	const size_t sizeOfHeader = sizeof(EncodedDataHeader);
	const size_t sizeOfOut = CDataEncoder::getBufferSizeForEncoded(m_encodingScheme, size) + sizeOfHeader;

	std::unique_ptr<char[]> buffer(new char[sizeOfOut]);
	char * const data = buffer.get();

	if (!buffer)
	{
		return 0;
	}

	if (m_verbose)
	{
		wcout << "Using encoding method: " << getMethodName(m_method) << endl;
	}

	CDataEncoder dataEnc;
	EncodedDataHeader packet;
	char * dataAfterHeader = data + sizeOfHeader;
	size_t sizeOfEncoded = dataEnc.encode(m_encodingScheme, toWrite, size, dataAfterHeader, sizeOfOut);

	if (m_verbose)
	{
		size_t s1 = min(BYTES_TO_HEX_DUMP, size);
		size_t s2 = min(BYTES_TO_HEX_DUMP, sizeOfEncoded);

		wcout << L"First " << s1 << L" bytes of data before encoding using method " << getCompressionName(m_encodingScheme) << ":\n";
		hexdump(toWrite, s1);
		wcout << L"\nFirst " << s2 << L" bytes of data encoded using " << getCompressionName(m_encodingScheme) << ":\n";
		hexdump(dataAfterHeader, s2);
		wcout << L"\n";
		wcout << "Bytes to encode: " << size << ", bytes after encoding: " << sizeOfEncoded << ". Ratio: " << float(sizeOfEncoded) / size << endl;
	}

	packet.marker = Magic_Data_Start_Marker;
	packet.encodingScheme = m_encodingScheme;
	packet.sizeOfOriginalData = size;
	packet.sizeOfEncodedData = sizeOfEncoded;

	memcpy(data, &packet, sizeOfHeader);
	memcpy(&m_lastOperationHeader, &packet, sizeOfHeader);

	size_t toWriteBytes = sizeOfEncoded + sizeOfHeader;
	size_t encoded = 0;
	switch (m_method)
	{
		case Stegano_Append: encoded = encodeAppend(data, toWriteBytes); break;
		case Stegano_Metadata: encoded = encodeMetadata(data, toWriteBytes); break;
		case Stegano_LSB: encoded = encodeLSB(data, toWriteBytes); break;
		case Stegano_LSB_Edges: encoded = encodeLSBEdges(data, toWriteBytes); break;
		case Stegano_2LSB_Color: encoded = encodeLSBColor(data, toWriteBytes); break;
		case Stegano_LSB_IncDec: encoded = encodeLSBIncDec(data, toWriteBytes); break;
	}

	m_file->flush();

	return encoded;
}


size_t CStegano::decode(char *toRead, size_t size)
{
	size_t decoded = 0;
	char *data = toRead;

	if (m_verbose)
	{
		wcout << "Using decoding method: " << getMethodName(m_method) << endl;
	}

	switch (m_method)
	{
		case Stegano_Append: decoded = decodeAppend(data, size); break;
		case Stegano_Metadata: decoded = decodeMetadata(data, size); break;
		case Stegano_LSB: decoded = decodeLSB(data, size); break;
		case Stegano_LSB_Edges: decoded = decodeLSBEdges(data, size); break;
		case Stegano_2LSB_Color: decoded = decodeLSBColor(data, size); break;
		case Stegano_LSB_IncDec: decoded = decodeLSBIncDec(data, size); break;
	}

	auto m = static_cast<EncodingSchemes>(m_lastOperationHeader.encodingScheme);
	const size_t sizeOfOut = CDataEncoder::getBufferSizeForDecoded(m, decoded);

	std::unique_ptr<char[]> buffer(new char[sizeOfOut]);
	char * const dataOut = buffer.get();
	
	if (!buffer /* || decoded != m_lastOperationHeader.sizeOfOriginalData */ )
	{
		return 0;
	}

	CDataEncoder dataEnc;
	size_t sizeOfDecoded = dataEnc.decode(m, data, decoded, dataOut, sizeOfOut);

	assert(sizeOfDecoded == m_lastOperationHeader.sizeOfOriginalData);

	if (m_verbose)
	{
		size_t s1 = min(BYTES_TO_HEX_DUMP, sizeOfOut);
		size_t s2 = min(BYTES_TO_HEX_DUMP, sizeOfDecoded);

		wcout << L"First " << s1 << L" bytes of data extracted out from image using " << getMethodName(m_method) << L" method:\n";
		hexdump(data, s1);
		wcout << L"\nFirst " << s2 << L" bytes of data decoded using " << getCompressionName(m) << ":\n";
		hexdump(dataOut, s2);
		wcout << L"\n";
		wcout << "Bytes decoded: " << sizeOfDecoded << endl;
	}

	memcpy(data, dataOut, sizeOfDecoded);

	return sizeOfDecoded;
}


bool CStegano::getLastOperationHeader(EncodedDataHeader &header)
{
	if (m_lastOperationHeader.marker == Magic_Data_Start_Marker)
	{
		header = m_lastOperationHeader;
		return true;
	}
	else
	{
		return false;
	}
}


size_t CStegano::encodeAppend(char* toWrite, size_t size)
{
	std::streampos processed = 0;

	m_file->seekp(0, std::ios_base::end);
	processed = m_file->tellp();
	m_file->write(toWrite, size);
	processed = m_file->tellp() - processed;

	return static_cast<size_t>(processed);
}


size_t CStegano::encodeMetadata(char* toWrite, size_t size)
{
	return m_imageFile->putDataToHeader(toWrite, size);
}


size_t CStegano::encodeLSB(char* toWrite, size_t size)
{
	size_t processed = 0;
	size_t toEncode = 0;
	const size_t avail = calculateAvailableSpace();

	if (size < sizeof(EncodedDataHeader))
	{
		return 0;
	}

	if (calculateNeededSpace(size) > avail)
	{
		// At the end we have to round it up to the byte's boundary
		toEncode = avail - avail % 8;
	}
	else
	{
		toEncode = size;
	}

	m_imageFile->getPixel(0, 0);
	if (m_imageFile->getError())
	{
		return 0;
	}

	encodeLSBLoop(toWrite, toEncode, 0);
	return toEncode;
}


size_t CStegano::encodeLSBIncDec(char* toWrite, size_t size)
{
	size_t processed = 0;

	wcout << L"Not implemented yet." << endl;

	return processed;
}


size_t CStegano::encodeLSBEdges(char* toWrite, size_t size)
{
	size_t processed = 0;

	wcout << L"Not implemented yet." << endl;

	return processed;
}


size_t CStegano::encodeLSBColor(char* toWrite, size_t size)
{
	size_t processed = 0;

	wcout << L"Not implemented yet." << endl;

	return processed;
}



size_t CStegano::decodeAppend(char* toWrite, size_t size)
{
	size_t processed = 0;

	m_file->seekg(-4, std::ios_base::end);
	int pos = 0;
	
	unsigned int readDword = 0;
	while (!m_file->fail() && readDword != Magic_Data_Start_Marker)
	{
		m_file->read(reinterpret_cast<char*>(&readDword), sizeof(readDword));
		m_file->seekg(--pos - 4, std::ios_base::end);
	}

	if (readDword == Magic_Data_Start_Marker)
	{
		m_file->seekg(1, std::ios_base::cur);
		m_file->read(reinterpret_cast<char*>(&m_lastOperationHeader), sizeof(EncodedDataHeader));

		if (m_lastOperationHeader.marker == Magic_Data_Start_Marker)
		{
			size_t sizeToRead = min(size, m_lastOperationHeader.sizeOfEncodedData);
			m_file->read(reinterpret_cast<char*>(toWrite), sizeToRead);
			processed = static_cast<size_t>(m_file->gcount());
			toWrite[processed] = 0;
		}
	}

	return processed;
}


size_t CStegano::decodeMetadata(char* toWrite, size_t size)
{
	size_t processed = m_imageFile->getDataFromHeader(toWrite, size);
	if (!processed && m_imageFile->getError() == BMP_ERROR_NO_METADATA_STRUCTURE)
	{
		// In Bitmap getDataFromHeader - this value have been returned in case,
		// when there is no valid fields in bitmap headers indicating possibility
		// of containing specific metadata structures.
		processed = decodeAppend(toWrite, size);
	}

	return processed;
}


size_t CStegano::decodeLSB(char* toWrite, size_t size)
{
	size_t processed = 0;

	if (size < sizeof(EncodedDataHeader))
	{
		return 0;
	}

	const size_t sizeOfHeader = sizeof(EncodedDataHeader);
	unsigned char header[sizeOfHeader];

	// Check if it is possible to get a pixel
	m_imageFile->getPixel(0, 0);
	if (m_imageFile->getError())
	{
		return 0;
	}

	// Firstly, try to extract the header
	memset(header, 0, sizeOfHeader);
	size_t sizeOfExtractedHeader = decodeLSBLoop(header, 0, sizeOfHeader);
	memcpy(&m_lastOperationHeader, header, sizeof(EncodedDataHeader));

	if (m_verbose)
	{
		auto marker = _byteswap_ulong(m_lastOperationHeader.marker);

		wcout << "\nExtracted header's contents:" << endl;
		wcout << "  struct EncodedDataHeader" << endl;
		wcout << "  {" << endl;
		wcout << "    marker = " << std::hex << std::setprecision(8) << std::setfill(L'0') << marker << ";" << endl;
		wcout << "    encodingScheme = " << std::dec << m_lastOperationHeader.encodingScheme 
				<< ";\t// compression: " << getCompressionName(static_cast<EncodingSchemes>(m_lastOperationHeader.encodingScheme)) << endl;
		wcout << "    sizeOfOriginalData = " << std::dec << m_lastOperationHeader.sizeOfOriginalData << ";" << endl;
		wcout << "    sizeOfEncodedData = " << std::dec << m_lastOperationHeader.sizeOfEncodedData << ";" << endl;
		wcout << "  };" << endl << endl;
	}

	// Validate extracted header
	if (sizeOfExtractedHeader == sizeof(EncodedDataHeader) && m_lastOperationHeader.marker == Magic_Data_Start_Marker)
	{
		// Extract the rest of the data
		processed = decodeLSBLoop(reinterpret_cast<uint8_t*>(toWrite), sizeOfHeader, m_lastOperationHeader.sizeOfEncodedData);
		toWrite[processed] = 0;
	}

	return processed;
}


size_t CStegano::decodeLSBIncDec(char* toWrite, size_t size)
{
	size_t processed = 0;

	wcout << L"Not implemented yet." << endl;

	return processed;
}


size_t CStegano::decodeLSBEdges(char* toWrite, size_t size)
{
	size_t processed = 0;

	wcout << L"Not implemented yet." << endl;

	return processed;
}


size_t CStegano::decodeLSBColor(char* toWrite, size_t size)
{
	size_t processed = 0;

	wcout << L"Not implemented yet." << endl;

	return processed;
}


size_t CStegano::calculateAvailableSpace()
{
	size_t availableSpace = 0;
	uint32_t pixelsForData = m_imageFile->getNumberOfPixels() - 8 * sizeof(Magic_Data_Start_Marker);

	/* If the number of available pixels is not divisible by 8,
	* then we got to round this number down to multiplication of 8.
	*/
	if (pixelsForData % 8 != 0)
		pixelsForData -= pixelsForData % 8;

	if (m_method == Stegano_2LSB_Color)
	{
		availableSpace = pixelsForData / 8;
	}
	else if (m_method == Stegano_LSB || m_method == Stegano_LSB_Edges || m_method == Stegano_LSB_IncDec)
	{
		availableSpace = pixelsForData * 3 / 8;
	}

	return availableSpace;
}


size_t CStegano::decodeLSBLoop(uint8_t* buff, uint32_t pos, uint32_t iterations)
{
	CImageFile::ImageColor col;
	size_t extractedBytes = 0;
	uint32_t byte = 0;

	pos *= 8;
	iterations *= 8;
	iterations += pos;

	for (; pos < iterations; pos++)
	{
		if (m_verbose && iterations > 10000 && pos % (iterations / 10000) == 0)
		{
			const float perc = (float(pos) / iterations) * 100.0;
			wcout << std::setw(2) << std::setprecision(2) << std::setfill(L'0') << perc << "%) Progress: "
				<< std::setw(7) << pos/8 << "/" << iterations/8 << " bytes to decode.\r";
		}

		uint32_t x, y;
		const uint32_t pixelNo = pos / 3;
		std::tie(x, y) = pixelNumberToImagePosition(pixelNo);
		col = m_imageFile->getPixel(x, y);

		uint8_t bit = 0;

		if (pos % 3 == 0)
		{
			bit = col.r & 1;
		}
		else if (pos % 3 == 1)
		{
			bit = col.g & 1;
		}
		else
		{
			bit = col.b & 1;
		}

		byte |= (bit << (pos % 8));

		if (pos % 8 == 7)
		{
			buff[extractedBytes++] = byte;
			byte = 0;
		}
	}

	return extractedBytes;
}


void CStegano::encodeLSBLoop(char* toWrite, size_t toEncode, uint32_t startPixel)
{
	int pos = 0;
	auto lsb = [this](uint8_t &col, uint8_t byte, uint8_t bit) -> void
	{
		col &= 0xfe;
		col |= this->getBit(byte, bit);
	};

	for (uint32_t imageByte = startPixel; imageByte < toEncode; imageByte++)
	{
		if (m_verbose && toEncode > 2000 && imageByte % (toEncode / 2000) == 0)
		{
			const float perc = (float(imageByte) / toEncode) * 100.0;
			wcout << std::setw(2) << std::setprecision(2) << std::setfill(L'0') << perc << "%) Progress: "
				<< std::setw(6) << imageByte << "/" << std::setw(6) << toEncode << " bytes to encode.\r";
		}

		for (uint8_t bit = 0; bit < 8; bit++, pos++)
		{
			uint32_t x, y;
			const uint32_t pixelNo = pos / 3;

			std::tie(x, y) = pixelNumberToImagePosition(pixelNo);
			CImageFile::ImageColor col = m_imageFile->getPixel(x, y);

			if (pos % 3 == 0)
			{
				lsb(col.r, toWrite[imageByte], bit);
			}
			else if (pos % 3 == 1)
			{
				lsb(col.g, toWrite[imageByte], bit);
			}
			else
			{
				lsb(col.b, toWrite[imageByte], bit);
			}

			m_imageFile->setPixel(x, y, col);
		}
	}
}
