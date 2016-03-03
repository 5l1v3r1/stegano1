#pragma once

#include <cstddef>
#include <vector>


enum EncodingSchemes
{
	Encode_None,
	Encode_Base64,
	Encode_Compress_ByteRun,
	Encode_Compress_RLE,
	Encode_Compress_LZW,
};


void hexdump(const char *data, size_t sizeOfData);
const wchar_t *getCompressionName(EncodingSchemes method);


class CDataEncoder
{
	static const double Encoded_To_Decoded_Worst_Ratio_Base64;
	static const double Encoded_To_Decoded_Worst_Ratio_ByteRun;
	static const double Encoded_To_Decoded_Worst_Ratio_RLE;
	static const double Encoded_To_Decoded_Worst_Ratio_LZW;

	typedef short Dictionary_Element_Type;
	static const Dictionary_Element_Type LZW_Dictionary_Size = 256;

	static const char Base64_Alphabet[64];

public:

	CDataEncoder() = default;
	~CDataEncoder() = default;

	static size_t getBufferSizeForEncoded(EncodingSchemes scheme, size_t dataSize);
	static size_t getBufferSizeForDecoded(EncodingSchemes scheme, size_t encodedSize) ;

	size_t encode(EncodingSchemes scheme, char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);
	size_t decode(EncodingSchemes scheme, char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);

protected:

	static double getSchemeMultiplier(EncodingSchemes scheme);

	size_t encodeBase64(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);
	size_t encodeByteRun(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);
	size_t encodeRLE(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);
	size_t encodeLZW(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);

	size_t decodeBase64(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);
	size_t decodeByteRun(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);
	size_t decodeRLE(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);
	size_t decodeLZW(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);

	size_t internal_compressLZW(char *input, size_t sizeOfInput, std::vector<Dictionary_Element_Type>& output, size_t sizeOfOutput);
	size_t internal_decompressLZW(const Dictionary_Element_Type *input, size_t sizeOfInput, char* output, size_t sizeOfOutput);

};

