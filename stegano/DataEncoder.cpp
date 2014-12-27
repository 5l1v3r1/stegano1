#include "DataEncoder.h"

#include <new>
#include <memory>


const double CDataEncoder::Encoded_To_Decoded_Worst_Ratio_Base64 = 0.75;
const double CDataEncoder::Encoded_To_Decoded_Worst_Ratio_ByteRun = 1.0;
const double CDataEncoder::Encoded_To_Decoded_Worst_Ratio_RLE = 1.0;
const double CDataEncoder::Encoded_To_Decoded_Worst_Ratio_LZW = 1.0;

const char CDataEncoder::Base64_Alphabet[64] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};



size_t CDataEncoder::getBufferSizeForDecoded(EncodingSchemes scheme, size_t encodedSize)
{
	double multiplier = CDataEncoder::getSchemeMultiplier(scheme);
	return static_cast<size_t>(multiplier * encodedSize);
}


size_t CDataEncoder::getBufferSizeForEncoded(EncodingSchemes scheme, size_t dataSize)
{
	double multiplier = (1 / CDataEncoder::getSchemeMultiplier(scheme)) * 1.2;
	return static_cast<size_t>(multiplier * dataSize);
}


double CDataEncoder::getSchemeMultiplier(EncodingSchemes scheme)
{
	double multiplier = 1.0;
	switch (scheme)
	{
		case Encode_None: multiplier = 1; break;
		case Encode_Base64: multiplier = Encoded_To_Decoded_Worst_Ratio_Base64; break;
		case Encode_Compress_ByteRun: multiplier = Encoded_To_Decoded_Worst_Ratio_ByteRun; break;
		case Encode_Compress_RLE: multiplier = Encoded_To_Decoded_Worst_Ratio_RLE; break;
		case Encode_Compress_LZW: multiplier = Encoded_To_Decoded_Worst_Ratio_LZW; break;
	}
	
	return multiplier;
}


size_t CDataEncoder::encode(EncodingSchemes scheme, unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput)
{
	size_t encoded = 0;

	switch (scheme)
	{
		case Encode_None: encoded = sizeOfInput; break;
		case Encode_Base64: encoded = encodeBase64(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_ByteRun: encoded = encodeByteRun(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_RLE: encoded = encodeRLE(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_LZW: encoded = encodeLZW(input, sizeOfInput, output, sizeOfOutput); break;
	}

	return encoded;
}


size_t CDataEncoder::decode(EncodingSchemes scheme, unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput)
{
	size_t decoded = 0;

	switch (scheme)
	{
		case Encode_None: decoded = sizeOfInput; break;
		case Encode_Base64: decoded = decodeBase64(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_ByteRun: decoded = decodeByteRun(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_RLE: decoded = decodeRLE(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_LZW: decoded = decodeLZW(input, sizeOfInput, output, sizeOfOutput); break;
	}

	return decoded;
}


size_t CDataEncoder::encodeBase64(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput)
{
	const char *alphabet = Base64_Alphabet;

	int pos = 0;
	size_t processed = 0;
	for (; pos < sizeOfInput && processed < sizeOfOutput; pos += 3)
	{
		unsigned int word =
			(input[pos] << 16) | (input[pos + 1] << 8) | (input[pos + 2]);

		output[processed + 0] = alphabet[((word & (63 << 18)) >> 18)];
		output[processed + 1] = alphabet[((word & (63 << 12)) >> 12)];

		int len = sizeOfInput - pos;
		output[processed + 2] = (len > 1) ? alphabet[((word & (63 << 6)) >> 6)] : '=';
		output[processed + 3] = (len > 2) ? alphabet[((word & (63)))] : '=';
		processed += 4;
	}

	output[processed] = 0x00;
	return processed;
}


size_t CDataEncoder::encodeByteRun(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput)
{
	size_t processed = 0;

	return processed;
}


size_t CDataEncoder::encodeRLE(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput)
{
	size_t processed = 0;

	return processed;
}


size_t CDataEncoder::encodeLZW(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput)
{
	size_t processed = 0;

	return processed;
}



size_t CDataEncoder::decodeBase64(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput)
{
	const char *alphabet = Base64_Alphabet;

	unsigned char decodingTable[256];
	memset(decodingTable, 0, sizeof(decodingTable));

	int i;
	for (i = 0; i < 64; i++)
	{
		decodingTable[alphabet[i]] = i;
	}

	int sizeOfCode = sizeOfInput;
	while (input[sizeOfCode-- - 1] == '=');

	int pos = 0;
	size_t processed = 0;
	for (; pos < sizeOfCode && processed < sizeOfOutput; pos += 4)
	{
		int chr1 = decodingTable[input[pos + 0]];
		int chr2 = decodingTable[input[pos + 1]];
		int chr3 = decodingTable[input[pos + 2]];
		int chr4 = decodingTable[input[pos + 3]];

		unsigned int word = (chr1 << 18) | (chr2 << 12) | (chr3 << 6) | (chr4);

		output[processed + 0] = ((word & (255 << 16)) >> 16);
		output[processed + 1] = ((word & (255 << 8)) >> 8);
		output[processed + 2] = (word & 255);
		processed += 3;
	}

	return processed;
}


size_t CDataEncoder::decodeByteRun(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput)
{
	size_t processed = 0;

	return processed;
}


size_t CDataEncoder::decodeRLE(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput)
{
	size_t processed = 0;

	return processed;
}


size_t CDataEncoder::decodeLZW(unsigned char *input, size_t sizeOfInput, unsigned char* output, size_t sizeOfOutput)
{
	size_t processed = 0;

	return processed;
}

