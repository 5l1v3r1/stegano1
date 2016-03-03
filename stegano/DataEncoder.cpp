#include "DataEncoder.h"

#include <iostream>
#include <iomanip>
#include <iterator>
#include <cstring>
#include <new>
#include <vector>
#include <algorithm>
#include <map>
#include <memory>


using namespace std;

#define OUT_VEC(str, vec) cout << str << " ";std::copy(vec.begin(), vec.end(), std::ostream_iterator<Dictionary_Element_Type>(std::cout, ", ")); cout << endl
#define OUT_VEC2(str, vec) cout << str << " ";std::copy(vec.begin(), vec.end(), std::ostream_iterator<char>(std::cout, "")); cout << endl


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


void hexdump(const char *data, size_t sizeOfData)
{
	for (size_t i = 0; i < sizeOfData; i += 16)
	{
		cout << std::setfill(' ') << std::setw(5) << std::hex << i << "  ";

		for (size_t j = 0; j < 16; j++)
		{
			if (i + j < sizeOfData)
				cout << std::setw(2) << std::setfill('0') << std::hex 
						<< (int)(data[i + j] & 0xff) << " ";
			else
				cout << std::setw(3) << std::setfill(' ');
		}
		
		if(i + 16 > sizeOfData) 
		{
			int n = (16 - (sizeOfData - i)) * 3 + 2;
			cout << std::setw(n) << std::setfill(' ');
		}
		else 
			cout << " ";

		for (int j = 0; j < 16; j++)
		{
			if (i + j < sizeOfData)
				cout << char(isprint(abs(data[i + j])) ? data[i + j] : '.');
		}

		cout << endl;
	}
}

const wchar_t *getCompressionName(EncodingSchemes method)
{
	switch (method)
	{
	case EncodingSchemes::Encode_None: return L"None";
	case EncodingSchemes::Encode_Base64: return L"Base64";
	case EncodingSchemes::Encode_Compress_ByteRun: return L"ByteRun";
	case EncodingSchemes::Encode_Compress_LZW: return L"LZW";
	case EncodingSchemes::Encode_Compress_RLE: return L"RLE";
	}

	return L"Unknown";
}


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


size_t CDataEncoder::encode(EncodingSchemes scheme, char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	size_t encoded = 0;

	switch (scheme)
	{
		case Encode_None:
		{
			encoded = sizeOfInput;
			memcpy(output, input, std::min(sizeOfInput, sizeOfOutput));
			break;
		}
		case Encode_Base64: encoded = encodeBase64(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_ByteRun: encoded = encodeByteRun(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_RLE: encoded = encodeRLE(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_LZW: encoded = encodeLZW(input, sizeOfInput, output, sizeOfOutput); break;
	}

	return encoded;
}


size_t CDataEncoder::decode(EncodingSchemes scheme, char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	size_t decoded = 0;

	switch (scheme)
	{
		case Encode_None: 
		{
			decoded = sizeOfInput;
			memcpy(output, input, std::min(sizeOfInput, sizeOfOutput));
			break;
		}
		case Encode_Base64: decoded = decodeBase64(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_ByteRun: decoded = decodeByteRun(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_RLE: decoded = decodeRLE(input, sizeOfInput, output, sizeOfOutput); break;
		case Encode_Compress_LZW: decoded = decodeLZW(input, sizeOfInput, output, sizeOfOutput); break;
	}

	return decoded;
}


size_t CDataEncoder::encodeBase64(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	const char *alphabet = Base64_Alphabet;

	size_t processed = 0;
	for (int pos = 0; pos < sizeOfInput && processed < sizeOfOutput; pos += 3)
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


size_t CDataEncoder::encodeByteRun(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	size_t processed = 0;
	size_t pos = 0;
	signed short tmp = 0;

	while( pos < sizeOfInput && processed < sizeOfOutput - 2)
	{
		tmp = 0;
		if(pos < sizeOfInput - 1 && input[pos] == input[pos+1] && processed + 2 < sizeOfOutput)
		{
			while( (pos + tmp) < (sizeOfInput - 1) && tmp < 127)
			{
				if(input[pos + tmp] != input[pos + tmp + 1])
					break;

				tmp++;
			}

			output[processed] = ((-tmp) & 0xff);
			output[processed + 1] = input[pos];

			pos += (tmp + 1);
			processed += 2;
		}
		else
		{
			while( (pos + tmp) < (sizeOfInput - 1) && tmp < 128)
			{
				if(input[pos + tmp] == input[pos + tmp + 1])
					break;

				tmp++;
			}

			if( tmp < processed - 1)
			{
				if( (pos + tmp) == (sizeOfInput - 1) && tmp < 128)
					tmp++;

				output[processed] = (tmp - 1) & 0xff;
				
				for( size_t k = 0; k < tmp; k++)
					output[processed + k + 1] = input[pos + k];

				pos += tmp;
				processed += tmp + 1;
			}
		}
	}
	
	return processed;
}


size_t CDataEncoder::encodeRLE(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	size_t processed = 0;
	size_t pos = 0;
	signed short tmp = 0;

	while( pos < sizeOfInput && processed < sizeOfOutput - 2)
	{
		tmp = 0;
		if(input[pos] == input[pos+1] && pos < sizeOfInput - 1 && processed + 2 < sizeOfOutput)
		{
			// repeating bytes

			while( (pos + tmp) < (sizeOfInput - 1) && tmp < 254)
			{
				if(input[pos + tmp] != input[pos + tmp + 1])
					break;
				tmp++;
			}

			// since we're counting from 0
			tmp++;

			if(tmp > 2)
			{
				output[processed] = (tmp & 0xff);
				output[processed + 1] = input[pos];
			}
			else
			{
				output[processed] = 1;
				output[processed + 1] = input[pos];
				
				if(tmp > 1)
				{
					output[processed + 2] = 1;
					output[processed + 3] = input[pos+1];
					processed += 2;
				}
				
			}

			processed += 2;
			pos += tmp;
		}
		else
		{
			// copying bytes

			while( (pos + tmp) < (sizeOfInput - 1) && tmp < 254)
			{
				if(input[pos + tmp] == input[pos + tmp + 1])
					break;

				tmp++;
			}

			if( tmp < sizeOfOutput - 1)
			{
				if( (pos + tmp) == (sizeOfInput - 1) && tmp < 254)
					tmp++;

				output[processed] = 0;
				output[processed+1] = tmp & 0xff;
				
				for( size_t k = 0; k < tmp; k++)
					output[processed + k + 2] = input[pos + k];

				pos += tmp;
				processed += tmp + 2;
			}
		}
	}
	
	return processed;
}


size_t CDataEncoder::encodeLZW(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	const size_t sizeOfElement = sizeof(Dictionary_Element_Type);
	const size_t outputSize = sizeOfOutput / sizeof(Dictionary_Element_Type);

	vector<Dictionary_Element_Type> out;

	size_t processed = internal_compressLZW(input, sizeOfInput, out, outputSize);

	for(size_t i = 0; i < out.size() && i < outputSize; i++)
	{
		memcpy(&output[i * sizeOfElement], &out[i], sizeOfElement);
	}

	return processed * sizeOfElement;
}



size_t CDataEncoder::decodeBase64(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	const char *alphabet = Base64_Alphabet;

	char decodingTable[256];
	memset(decodingTable, 0, sizeof(decodingTable));

	for (int i = 0; i < 64; i++)
	{
		decodingTable[alphabet[i]] = i;
	}

	int sizeOfCode = sizeOfInput;
	while (input[sizeOfCode-- - 1] == '=');

	size_t processed = 0;
	for (int pos = 0; pos < sizeOfCode && processed < sizeOfOutput; pos += 4)
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


size_t CDataEncoder::decodeByteRun(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	size_t processed = 0;
	size_t pos = 0;
	signed short tmp = 0;

	while( pos < sizeOfInput && processed < sizeOfOutput - 2)
	{
		tmp = 0;

		if(input[pos] == -128)
			continue;

		if(input[pos] < 0)
		{
			// repeating bytes
			tmp = -(input[pos] - 1);
			for( size_t j = 0; j < tmp && (processed + j) < sizeOfOutput - 1; j++)
			{
				output[processed + j] = input[pos + 1];
			}

			pos += 2;
			processed += tmp ;
		}
		else
		{
			// Copying bytes
			tmp = (input[pos] + 1);
			for( size_t j = 0; j < tmp && (processed + j) < sizeOfOutput - 1; j++)
			{
				output[processed + j] = input[pos + 1 + j];
			}

			pos += tmp + 1;
			processed += tmp;
		}
	}

	return processed;
}


size_t CDataEncoder::decodeRLE(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	size_t processed = 0;
	size_t pos = 0;
	signed short tmp = 0;

	while( pos < sizeOfInput && processed < sizeOfOutput - 2)
	{
		tmp = 0;

		if(input[pos] > 0)
		{
			// repeating bytes
			tmp = input[pos];
			for( size_t j = 0; j < tmp && (processed + j) < sizeOfOutput - 1; j++)
			{
				output[processed + j] = input[pos + 1];
			}

			pos += 2;
			processed += tmp;
		}
		else
		{
			// Copying bytes
			tmp = input[pos + 1];
			for( size_t j = 0; j < tmp && (processed + j) < sizeOfOutput - 1; j++)
			{
				output[processed + j] = input[pos + 2 + j];
			}

			pos += tmp + 2;
			processed += tmp;
		}
	}

	return processed;
}


size_t CDataEncoder::decodeLZW(char *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	const Dictionary_Element_Type* in = reinterpret_cast<Dictionary_Element_Type*>(input);
	const size_t inputSize = sizeOfInput / (sizeof(Dictionary_Element_Type));

	return internal_decompressLZW(in, inputSize, output, sizeOfOutput);
}


size_t CDataEncoder::internal_compressLZW(char *input, size_t sizeOfInput, vector<Dictionary_Element_Type>& output, size_t sizeOfOutput )
{

	size_t processed = 0;
	Dictionary_Element_Type dict_size = LZW_Dictionary_Size;

	// Dictionary for the LZW algorithm. It will be mapping substring words
	// with the index number in it.
	std::map<std::vector<char>, Dictionary_Element_Type> dictionary;

	// Building the dictionary from the basic words of length 1
	for(Dictionary_Element_Type i = 0; i < dict_size; i++)
	{
		dictionary[{ static_cast<char>(i) }] = i;
	}

	// This vector will be holding previous word, that is present in the dictionary.
	std::vector<char> prev_word;

	// Compressing
	for(size_t i = 0; i < sizeOfInput && processed < sizeOfOutput; i++)
	{
		char byte = input[i];

		// This vector will hold a currently processed word, that is or isn't in dictionary
		std::vector<char> word = prev_word;

		// We're adding next character to the previously processed word.
		word.push_back(byte);

		if(dictionary.count(word))
		{
			// If there is already a currently processed word in dictionary - we
			// change temporary word to the currently built one.
			prev_word = word;
		}
		else
		{
			// We've built a new word, that isn't present in the dictionary yet.
			// Adding position of the previous word to the dictionary.
			output.push_back(dictionary[prev_word]);
			processed++;

			// Adding next word to the dictionary
			dictionary[word] = dict_size++;

			// Storing new previously processed word.
			prev_word = std::vector<char>{byte};
		}
	}

	if(!prev_word.empty())
	{
		output.push_back(dictionary[prev_word]);
		processed++;
	}

	return processed;
}



size_t CDataEncoder::internal_decompressLZW(const Dictionary_Element_Type *input, size_t sizeOfInput, char* output, size_t sizeOfOutput)
{
	Dictionary_Element_Type dict_size = LZW_Dictionary_Size;

	// Dictionary for the LZW algorithm. It will be mapping substring words
	// with the index number in it.
	std::map<Dictionary_Element_Type, std::vector<char>> dictionary;

	// Building the dictionary from the basic words of length 1
	for(Dictionary_Element_Type i = 0; i < dict_size; i++)
	{
		dictionary[i] = { static_cast<char>(i) };
	}

	// This vector will be holding previous word, that is present in the dictionary.
	std::vector<char> prev_word = { static_cast<char>(input[0]) };

	// This vector will hold a currently processed word, that is or isn't in dictionary
	std::vector<char> word, tmp;

	// Output vector
	std::vector<char> out = prev_word;

	// Decompressing
	for(size_t i = 1; i < sizeOfInput; i++)
	{
		Dictionary_Element_Type elem = input[i];

		// We're adding next character to the previously processed word.
		//word.push_back(elem);

		if(dictionary.count(elem))
		{
			// If there is already a currently processed word in dictionary - we
			// change temporary word to the currently built one.
			word = dictionary[elem];
		}
		else if(elem == dict_size)
		{
			tmp = prev_word;
			tmp.push_back(prev_word[0]);

			word = tmp;
		}
		else
		{
			// Error while decompression.
			out.clear();
			return 0;
		}

		out.insert(out.end(), word.begin(), word.end());

		tmp = prev_word;
		tmp.push_back(word[0]);

		dictionary[dict_size++] = tmp;

		prev_word = word;
	}

	const size_t outSize = min(sizeOfOutput, out.size());
	std::copy(out.begin(), out.begin() + outSize, output);

	return outSize;
}
