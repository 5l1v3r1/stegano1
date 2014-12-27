#pragma once



enum EncodingSchemes
{
	Encode_None,
	Encode_Base64,
	Encode_Compress_ByteRun,
	Encode_Compress_RLE,
	Encode_Compress_LZW,
};


class CDataEncoder
{
	static const double Encoded_To_Decoded_Worst_Ratio_Base64;
	static const double Encoded_To_Decoded_Worst_Ratio_ByteRun;
	static const double Encoded_To_Decoded_Worst_Ratio_RLE;
	static const double Encoded_To_Decoded_Worst_Ratio_LZW;

	static const char Base64_Alphabet[64];

public:

	CDataEncoder() = default;
	~CDataEncoder() = default;

	static size_t getBufferSizeForEncoded(EncodingSchemes scheme, size_t dataSize);
	static size_t getBufferSizeForDecoded(EncodingSchemes scheme, size_t encodedSize) ;

	size_t encode(EncodingSchemes scheme, unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput);
	size_t decode(EncodingSchemes scheme, unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput);

protected:

	static double getSchemeMultiplier(EncodingSchemes scheme);

	size_t encodeBase64(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput);
	size_t encodeByteRun(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput);
	size_t encodeRLE(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput);
	size_t encodeLZW(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput);

	size_t decodeBase64(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput);
	size_t decodeByteRun(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput);
	size_t decodeRLE(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput);
	size_t decodeLZW(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput);

};

