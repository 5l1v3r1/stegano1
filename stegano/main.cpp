

#include "Stegano.h"
#include "XGetopt.h"

#include <iostream>
#include <wchar.h>
#include <fstream>
#include <memory>

using namespace std;


static char Data_To_Encode[] = "Wiadomosc testowa. 1234!@#$. Ala ma kota.\xAB\xCD\xEF";
static const size_t Size_Of_Data = sizeof(Data_To_Encode);



int main(int argc, char** argv)
{
	const wchar_t *path = LR"(d:\!_PK\Prace\GKiM_proj\images\image3.bmp)";

	CStegano steg;
	steg.init(path, CStegano::Stegano_Append, EncodingSchemes::Encode_Base64);
	size_t encoded = steg.encode(reinterpret_cast<unsigned char*>(Data_To_Encode), Size_Of_Data);

	if (encoded > Size_Of_Data)
	{
		wcout << L"Successfully encoded entire message" << endl;
	}
	else
	{
		wcout << L"Unable to encode entire message. Encoded bytes: " << encoded << endl;
	}

	
	const size_t sizeOfDecodedBuff = CDataEncoder::getBufferSizeForEncoded(Encode_Base64, Size_Of_Data);
	std::unique_ptr<unsigned char[]> decodedBytes(new unsigned char[sizeOfDecodedBuff]);
	if (!decodedBytes)
	{
		return 0;
	}

	memset(decodedBytes.get(), 0, Size_Of_Data);

	size_t decoded = steg.decode(decodedBytes.get(), sizeOfDecodedBuff);

	if (decoded >= Size_Of_Data)
	{
		wcout << L"Successfully decoded entire message" << endl;
	}
	else
	{
		wcout << L"Unable to decode entire message. Encoded bytes: " << encoded << endl;
	}

	wcout << L"Decoded message:\n";
	cout << decodedBytes.get() << endl << endl;


	if (memcmp(decodedBytes.get(), Data_To_Encode, Size_Of_Data) == 0)
	{
		wcout << L"Encoded and decoded bytes correctly!" << endl;
	}
	else
	{
		wcout << L"Encoding and decoding doesn't work correctly yet." << endl;
	}
}


