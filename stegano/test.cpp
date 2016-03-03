#include <iostream>
#include <iomanip>
#include <cstddef>
#include <cmath>
#include "DataEncoder.h"

using namespace std;


int main(int argc, char **argv)
{	
	char input[] = "WWWWWWWWiiiiaaaaaaiiiidddoooommoosc "
					"testtestowaowa, alalala mmmmmmaa kot"
					"aota kot ma ale ";

	const size_t sizeOfInput = sizeof(input);
	const size_t maxHexDump = sizeOfInput * 0.7;

	char output[512] = {0};
	char output2[512] = {0};
	const size_t sizeOfOutput = sizeof(output);

	EncodingSchemes scheme = Encode_Compress_LZW;

	CDataEncoder enc;

	cout << "\nEncoding (data len: " << sizeOfInput << ")...\n" << endl;
	size_t sizeOfEncoded = enc.encode(scheme, input, sizeOfInput, output, sizeOfOutput);

	size_t s1 = min(maxHexDump, sizeOfInput);
	size_t s2 = min(maxHexDump * 4, sizeOfEncoded);
	
	cout << "First " << std::dec << s1 << " bytes of data before encoding:\n";
	hexdump(input, s1);
	cout << "\nFirst " << std::dec << s1 << " bytes of data encoded using ";
	wcout << getCompressionName(scheme) << endl;
	hexdump(output, s2);
	
	cout << "\nCompression rate: " << std::dec << sizeOfEncoded << " / " << sizeOfInput 
		<< ", ratio: " << std::setprecision(3) << (((double)sizeOfEncoded) / sizeOfInput) << "\n";


	cout << "Decoding...\n" << endl;
	size_t sizeOfDecoded = enc.decode(scheme, output, sizeOfEncoded, output2, sizeOfOutput);

	s1 = min(maxHexDump, sizeOfEncoded);
	s2 = min(maxHexDump, sizeOfDecoded);

	cout << "First " << std::dec << s1 << " bytes of data decoded using ";
	wcout << getCompressionName(scheme) << endl;
	hexdump(output2, s2);
	
	int correct = 0;
	for(int i = 0; i < sizeOfInput; i++)
	{
		if(input[i] == output2[i]) correct++;
	}

	double ratio = (((double)correct) / sizeOfInput);

	if(abs(ratio - 1.0) > 1e-7)
	{
		cout << "\nCorrectly decompressed bytes: " << std::dec << correct << " / " << sizeOfInput 
			<< ", ratio: " << std::setprecision(3) << ratio << endl;
	}
	else
	{
		cout << "\nCompression algorithm is working correctly." << endl;
	}
}