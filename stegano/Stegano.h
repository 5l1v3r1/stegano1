#pragma once

#include "ImageFile.h"
#include "DataEncoder.h"

#include <string>
#include <tuple>

#define BSWAP(x)	(((x & 0xff000000) >> 24) | ((x & 0xff0000) >> 8) | ((x & 0xff00) << 8) | ((x & 0xff) << 24))


class CStegano
{
	static const unsigned int Magic_Data_Start_Marker = BSWAP(0xAABBCCDD);
	
	// This header will be written just before actual data
#pragma pack(1)
	struct EncodedDataHeader
	{
		unsigned int marker;
		unsigned char encodingScheme : 4;
		unsigned int sizeOfOriginalData;
		unsigned int sizeOfEncodedData;
	};
#pragma pack()


	enum RecognizedImageFormat
	{
		Image_Unknown,
		Image_Bmp,
		Image_Png,
		Image_Jpg,
	};

	typedef std::tuple<uint32_t, uint32_t> ImagePosition;

public:

	enum SteganoMethod : unsigned char
	{
		// Use the Least Significant Bit approach in hiding data.
		Stegano_LSB,

		// Extracts the least significant bit out of a modified pixel
		// resulted in some increment/decrement operation.
		// The approach applied in this class will be to increment and
		// decrement stream of pixels alternately 
		// (all by one, skipping 0x00 and 0xFF colored pixels)
		Stegano_LSB_IncDec,

		// Another modification of the LSB method - resulting in 
		// applying this technique only to a subset of pixels. Here,
		// by following picture's edges.
		// This is a Edges based data embedding method (EBE).
		Stegano_LSB_Edges,

		// Altering only one color's component - in this application,
		// the blue one will be chosen. The method will encode a data
		// byte in four consecutive blue color's bytes by adding/subtracting
		// two least significant bits out of them.
		// This is a Pixel Value Differencing method (PVD).
		Stegano_2LSB_Color,

		// Modify image's metadata by adding some Comment/Notes
		// field to the image headers/EXIF.
		Stegano_Metadata,

		// Simple data appending to the image file
		Stegano_Append,

	};

	
	CStegano(){}
	~CStegano(){}

	bool init(const std::wstring& path, SteganoMethod method, EncodingSchemes scheme = EncodingSchemes::Encode_None);

	size_t encode(unsigned char *toWrite, size_t size);
	size_t decode(unsigned char *toRead, size_t size);

	bool getLastOperationHeader(EncodedDataHeader &header);


protected:

	CImageFile *loadImage(const std::wstring& path);

	size_t encodeAppend(unsigned char* toWrite, size_t size);
	size_t encodeMetadata(unsigned char* toWrite, size_t size);
	size_t encodeLSB(unsigned char* toWrite, size_t size);

	void encodeLSBLoop(unsigned char* toWrite, size_t toEncode, uint32_t startPixel);

	size_t encodeLSBIncDec(unsigned char* toWrite, size_t size);
	size_t encodeLSBEdges(unsigned char* toWrite, size_t size);
	size_t encodeLSBColor(unsigned char* toWrite, size_t size);

	size_t decodeAppend(unsigned char* toWrite, size_t size);
	size_t decodeMetadata(unsigned char* toWrite, size_t size);
	size_t decodeLSB(unsigned char* toWrite, size_t size);
	size_t decodeLSBIncDec(unsigned char* toWrite, size_t size);
	size_t decodeLSBEdges(unsigned char* toWrite, size_t size);
	size_t decodeLSBColor(unsigned char* toWrite, size_t size);

	// Some utilities
	inline char getBit(char byte, unsigned char bit)
	{
		return ((byte & (1 << bit)) >> bit) & 1;
	}

	inline size_t calculateNeededSpace(size_t sizeOfDataToEncode)
	{
		return (sizeOfDataToEncode + sizeof(Magic_Data_Start_Marker)) * 8;
	}

	size_t calculateAvailableSpace();

	inline ImagePosition pixelNumberToImagePosition(uint32_t pos)
	{
		const uint32_t w = m_imageFile->getWidth();
		return std::make_tuple<uint32_t, uint32_t>((pos % w), (pos / w));
	}


private:

	size_t decodeLSBLoop(uint8_t* buff, uint32_t pos, uint32_t iterations);

private:

	RecognizedImageFormat m_imageFormat;
	EncodingSchemes m_encodingScheme;
	SteganoMethod m_method;
	EncodedDataHeader m_lastOperationHeader;

	CImageFile *m_imageFile;
	std::fstream *m_file;
	uint32_t m_numOfPixels;
};

