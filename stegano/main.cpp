

#include "Stegano.h"
#include "getopt.h"

#include <iostream>
#include <wchar.h>
#include <fstream>
#include <memory>

#ifdef _DEBUG
#pragma warning(disable:4996)
#endif

using namespace std;


static char Data_To_Encode[] = "Wiadomosc testowa. 1234!@#$. Ala ma kota.\xAB\xCD\xEF";
static const size_t Size_Of_Data = sizeof(Data_To_Encode);



char parse_options(int argc, char **argv, char *data_file, char* image_file)
{
	int opt;
	int ptr;
	char ret = '?';

	/* prevent getopt displaying error message */
	opterr = 0;

	static struct option opts[] = {
		{ "encode", optional_argument, 0, 'e' },
		{ "decode", optional_argument, 0, 'd' },
		{ 0, 0, 0, 0 }
	};

	if (argc == 1) return 'h';

	if ((opt = getopt_long(argc, argv, "e::d::h", opts, &ptr)) != -1) {
		switch (opt) {
		case 'd':
			if (optind == argc - 1){
				strcpy(image_file, argv[optind]);
				strcpy(data_file, "!");
			}
			else{
				strcpy(data_file, argv[optind]);
				strcpy(image_file, argv[optind + 1]);
			}
			ret = 'd';
			break;
		case 'e':
			if (optind == argc - 1){
				strcpy(data_file, "@");
				strcpy(image_file, argv[optind]);
			}
			else{
				strcpy(data_file, argv[optind]);
				strcpy(image_file, argv[optind + 1]);
			}
			ret = 'e';
			break;
		case 'h': return 'h';
		case '?':
			printf("Unknown option: %c.\n", optopt);
			return '?';
		}
	}
	return ret;
}


void usage(const char *app)
{
	wcout << L"\nUsage:\n\t";
	cout << app << " [options] image_file\n";
	wcout << L"Where:\n\t-h\t\t\t\t- prints help and exits\n";
	wcout << L"\t-e, --encode [data_file]\t- hide data from data_file into";
	wcout << L"\n\t\t\t\t\t    image_file. If data_file hasn't\n\t\t";
	wcout << L"\t\t\t    been specified, will use stdin";
	wcout << L"\n\t-d, --decode [data_file]\t- extract data out of image_file ";
	wcout << L"\n\t\t\t\t\t    into data_file. If data_file ";
	wcout << L"\n\t\t\t\t\t    hasn't been specified, will use stdout\n";
	wcout << L"\tdata_file\t\t\t- file with data to hide/extract\n";
	wcout << L"\timage_file\t\t\t- file to store/extract from the data\n\n";
}

int main(int argc, char** argv)
{
	char image_file[256] = "";
	char data_file[256] = "";
	wchar_t dataFile[256] = L"";

	char opt = parse_options(argc, argv, data_file, image_file);

#ifndef _DEBUG
	if (opt == '?' || opt == 'h'){
		usage(argv[0]);
		return 0;
	}

	wchar_t imagePath[256] = L"";

	mbstowcs(imagePath, image_file, _countof(imagePath));
	mbstowcs(dataFile, data_file, _countof(dataFile));

	wcout << L"Release mode main function not implemented yet.";
	return 0;

#else
	const wchar_t *imagePath = LR"(d:\!_PK\Prace\GKiM_proj\images\image3.bmp)";
#endif

	CStegano steg;
	steg.init(imagePath, CStegano::Stegano_LSB, EncodingSchemes::Encode_Base64);
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
