
#include "Stegano.h"
#include "BmpImage.h"
#include "PngImage.h"
#include "JpegImage.h"

#include <memory>
#include <algorithm>
#include <cassert>


bool CStegano::init(const std::wstring& path, SteganoMethod method, EncodingSchemes scheme /*= EncodingSchemes::Encode_None*/)
{
	m_imageFile = CStegano::loadImage(path);
	m_method = method;
	m_encodingScheme = scheme;

	bool ret = m_imageFile->loadImage(path);
	m_file = &m_imageFile->getFile();

	return ret;
}



CImageFile *CStegano::loadImage(const std::wstring& path)
{
	if (path.find(L".png") != std::wstring::npos)
	{
		return new CPngImage();
	}
	else if (path.find(L".bmp") != std::wstring::npos)
	{
		return new CBmpImage();
	}
	else if (path.find(L".jpg") != std::wstring::npos || path.find(L".jpeg") != std::wstring::npos)
	{
		return new CJpegImage();
	}
	else
	{
		return nullptr;
	}
}


size_t CStegano::encode(unsigned char *toWrite, size_t size)
{
	const size_t sizeOfHeader = sizeof(EncodedDataHeader);
	const size_t sizeOfOut = CDataEncoder::getBufferSizeForEncoded(m_encodingScheme, size) + sizeOfHeader;

	std::unique_ptr<unsigned char[]> buffer(new unsigned char[sizeOfOut]);
	unsigned char * const data = buffer.get();

	if (!buffer)
	{
		return 0;
	}

	CDataEncoder dataEnc;
	EncodedDataHeader packet;
	unsigned char * dataAfterHeader = data + sizeOfHeader;
	unsigned int sizeOfEncoded = dataEnc.encode(m_encodingScheme, toWrite, size, dataAfterHeader, sizeOfOut);

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


size_t CStegano::decode(unsigned char *toRead, size_t size)
{
	size_t decoded = 0;
	unsigned char *data = toRead;

	if (size < sizeof(EncodedDataHeader))
	{
		return 0;
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


	const size_t sizeOfOut = CDataEncoder::getBufferSizeForDecoded(m_encodingScheme, decoded);
	std::unique_ptr<unsigned char[]> buffer(new unsigned char[sizeOfOut]);
	unsigned char * const dataOut = buffer.get();
	
	if (!buffer /* || decoded != m_lastOperationHeader.sizeOfOriginalData */ )
	{
		return 0;
	}

	CDataEncoder dataEnc;
	size = dataEnc.decode(static_cast<EncodingSchemes>(m_lastOperationHeader.encodingScheme), data, decoded, dataOut, sizeOfOut);

	assert(size == m_lastOperationHeader.sizeOfOriginalData);

	memcpy(data, dataOut, size);
	return size;
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


size_t CStegano::encodeAppend(unsigned char* toWrite, size_t size)
{
	std::streampos processed = 0;

	m_file->seekp(0, std::ios_base::end);
	processed = m_file->tellp();
	m_file->write(reinterpret_cast<const char*>(toWrite), size);
	processed = m_file->tellp() - processed;

	return static_cast<size_t>(processed);
}


size_t CStegano::encodeMetadata(unsigned char* toWrite, size_t size)
{
	size_t processed = 0;

	return processed;
}


size_t CStegano::encodeLSB(unsigned char* toWrite, size_t size)
{
	size_t processed = 0;

	return processed;
}


size_t CStegano::encodeLSBIncDec(unsigned char* toWrite, size_t size)
{
	size_t processed = 0;

	return processed;
}


size_t CStegano::encodeLSBEdges(unsigned char* toWrite, size_t size)
{
	size_t processed = 0;

	return processed;
}


size_t CStegano::encodeLSBColor(unsigned char* toWrite, size_t size)
{
	size_t processed = 0;

	return processed;
}



size_t CStegano::decodeAppend(unsigned char* toWrite, size_t size)
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
			size_t sizeToRead = std::min(size, m_lastOperationHeader.sizeOfEncodedData);
			m_file->read(reinterpret_cast<char*>(toWrite), sizeToRead);
			processed = m_file->gcount();
			toWrite[processed] = 0;
		}
	}

	return processed;
}


size_t CStegano::decodeMetadata(unsigned char* toWrite, size_t size)
{
	size_t processed = 0;

	return processed;
}


size_t CStegano::decodeLSB(unsigned char* toWrite, size_t size)
{
	size_t processed = 0;

	return processed;
}


size_t CStegano::decodeLSBIncDec(unsigned char* toWrite, size_t size)
{
	size_t processed = 0;

	return processed;
}


size_t CStegano::decodeLSBEdges(unsigned char* toWrite, size_t size)
{
	size_t processed = 0;

	return processed;
}


size_t CStegano::decodeLSBColor(unsigned char* toWrite, size_t size)
{
	size_t processed = 0;

	return processed;
}

