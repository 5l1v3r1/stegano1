

#include "Stegano.h"
#include "getopt.h"

#include <iostream>
#include <iomanip>
#include <wchar.h>
#include <fstream>
#include <algorithm>
#include <memory>


using namespace std;

bool g_verboseFlag = false;


#ifdef _DEBUG
#pragma warning(disable:4996)

static char Data_To_Encode[] = "Wiadomosc testowa. 1234!@#$. Ala ma kota.\xAB\xCD\xEF";
static const size_t Size_Of_Data = sizeof(Data_To_Encode);

#endif



size_t getFileSize(const std::string filePath)
{
	std::fstream file(filePath, std::fstream::in | std::fstream::binary);

	if (file.good() && file.is_open())
	{
		size_t prev = file.tellg();
		file.seekg(0, std::ios_base::end);
		size_t fileSize = file.tellg();
		file.seekg(prev, std::ios_base::beg);

		return fileSize;
	}

	return 0;
}


char parse_options(int argc, char **argv, char *data_file, char* image_file, CStegano::SteganoMethod &method, EncodingSchemes& compression)
{
	int opt;
	int ptr;
	char ret = '?';

	/* prevent getopt displaying error message */
	opterr = 0;

	static struct option opts[] = {
		{ "encode", optional_argument, 0, 'e' },
		{ "decode", optional_argument, 0, 'd' },
		{ "method", optional_argument, 0, 'm' },
		{ "compress", optional_argument, 0, 'c' },
		//{ "h", no_argument, 0, 'h' },
		//{ "e", no_argument, 0, 'e' },
		//{ "d", no_argument, 0, 'd' },
		//{ "v", no_argument, 0, 'v' },
		{ 0, 0, 0, 0 }
	};

	if (argc == 1) 
		return 'h';

	while ((opt = getopt_long(argc, argv, "vc::m::e::d::h", opts, &ptr)) != -1) 
	{
		switch (opt) 
		{
		case 'd':
			if (argc > optind + 1)
			{
				strcpy(data_file, argv[optind]);
				strcpy(image_file, argv[optind + 1]);
			}
			else if (optind == argc - 1)
			{
				strcpy(image_file, argv[optind]);
				strcpy(data_file, "!");
			}
			else
			{
				wcout << L"Error: image_file path not supplied!\n";
				return '?';
			}

			ret = 'd';
			break;

		case 'e':
			if (argc > optind + 1)
			{
				strcpy(data_file, argv[optind]);
				strcpy(image_file, argv[optind + 1]);
			}
			else if (argc == optind + 1)
			{
				strcpy(data_file, "@");
				strcpy(image_file, argv[optind]);
			}
			else
			{
				wcout << L"Error: image_file path not supplied!\n";
				return '?';
			}

			ret = 'e';
			break;

		case 'm':
		{
			long m = atol(argv[optind]);

			if (m < 1 || m > 6)
			{
				wcout << L"Chosen method is not available.\n";
				return '?';
			}

			method = static_cast<CStegano::SteganoMethod>(m - 1);
			break;
		}

		case 'c':
		{
			long c = atol(argv[optind]);

			if (c < 1 || c > 5)
			{
				wcout << L"Chosen method is not available.\n";
				return '?';
			}

			compression = static_cast<EncodingSchemes>(c - 1);
			break;
		}

		case 'v': g_verboseFlag = true; break;
		case 'h': return 'h';
		case '?':
			wcout << L"Unknown option: " << (char)optopt << L".\n";
			return '?';
		}
	}

	if (strlen(image_file) == 0)
	{
		wcout << L"Error: image_file path not supplied!\n";
		return '?';
	}

	return ret;
}


bool copy_file(const char* filePath)
{
	if (filePath == nullptr)
	{
		return false;
	}

	std::string filePath2 = filePath;
	filePath2 += ".bak";

	std::fstream from(filePath, std::fstream::in | std::fstream::binary);
	std::fstream to(filePath2, std::fstream::out | std::fstream::binary);

	if (!from.is_open() || !to.is_open())
	{
		return false;
	}

	size_t fileSize = getFileSize(filePath);

	std::unique_ptr<char> buffer(new char[fileSize]);
	if (!buffer)
	{
		return false;
	}

	from.read(buffer.get(), fileSize);
	to.write(buffer.get(), fileSize);

	from.close();
	to.close();

	return true;
}


std::unique_ptr<char> read_working_buffer(const std::string filePath, size_t &sizeOfData)
{
	sizeOfData = 0;
	std::unique_ptr<char> buffer;

	if (filePath == "@")
	{
		std::string input;

		wcout << L"Start writing data to encode:\n" << wstring(30, L'-') << endl;
		getline(cin, input);

		buffer.reset(new char[input.size() + 1]);
		if (buffer)
		{
			for (size_t i = 0; i < input.size(); i++)
			{
				buffer.get()[i] = input[i];
			}
		}

		sizeOfData = input.size();

		wcout << endl;
	}
	else
	{
		std::fstream input(filePath, std::fstream::in | std::fstream::binary);

		if (!input.is_open())
		{
			wcout << L"Couldn't open specified file for the input data." << endl;
			return nullptr;
		}

		size_t fileSize = getFileSize(filePath);

		buffer.reset(new char[fileSize]);
		if (!buffer)
		{
			wcout << L"Couldn't allocate memory for the input file's data!" << endl;
			return nullptr;
		}

		input.read(buffer.get(), fileSize);
		input.close();

		sizeOfData = fileSize;
	}

	return buffer;
}

bool write_working_buffer(const std::string filePath, char *output, size_t sizeOfData)
{
	std::unique_ptr<char> buffer;

	if (filePath == "!")
	{
		wcout << L"Decoded message (" << sizeOfData << " bytes):\n" << wstring(30, L'-') << endl;
		cout << output << endl;
		
		wcout << endl;
	}
	else
	{
		std::fstream outFile(filePath, std::fstream::out | std::fstream::binary);

		if (!outFile.is_open())
		{
			wcout << "Coulnd't open specified file for the output data." << endl;
			return false;
		}

		outFile.write(output, sizeOfData);
		outFile.close();
	}

	return true;
}


void usage(const char *app)
{
	wcout << L"Usage:\n\t";
	std::cout << app << " [options] image_file\n";
	wcout << L"\nWhere:\n\n  -h\t\t\t    prints help and exits\n";
	wcout << L"  -e, --encode [data_file]  hide data from data_file into image_file. If";
	wcout << L"\n\t\t\t    data_file hasn't been specified, will use stdin\n\t\t";
	wcout << L"\n  -d, --decode [data_file]  extract data out of image_file into data_file.";
	wcout << L"\n\t\t\t    If data_file hasn't been specified, will use stdout\n";
	wcout << L"\n  -m, --method [1-6]\t    specifies the encoding method to use during actual";
	wcout << L"\n\t\t\t    data hiding in an image. Possible values:";
	wcout << L"\n\t\t\t    1. LSB in entire pixels matrix (default)";
	wcout << L"\n\t\t\t    2. LSB IncDec method resulting in LSB";
	wcout << L"\n\t\t\t       after incrementing or decrementing pixel's value";
	wcout << L"\n\t\t\t    3. LSB Edges - following LSB method along the edges";
	wcout << L"\n\t\t\t    4. 2LSB Color - altering two lsb bits of blue color";
	wcout << L"\n\t\t\t    5. Storing encoded data in image's metadata";
	wcout << L"\n\t\t\t    6. Appending encoded data to the image file\n";
	wcout << L"\n  -c, --compress [1-5]\t    specifies the data compression method\n\t\t\t    during encoding. Possible values:";
	wcout << L"\n\t\t\t    1. No compression over supplied data (default)";
	wcout << L"\n\t\t\t    2. Base64";
	wcout << L"\n\t\t\t    3. ByteRun";
	wcout << L"\n\t\t\t    4. RLE";
	wcout << L"\n\t\t\t    5. LZW";
	wcout << L"\n\n  -v\t\t\t    verbose flag, shows debugging/info messages\n";
	wcout << L"\n  data_file\t\t    file with data to hide/extract\n";
	wcout << L"  image_file\t\t    file to store/extract from the data\n";
}


int main(int argc, char** argv)
{
	char image_file[256] = "";
	char data_file[256] = "";
	wchar_t dataFile[256] = L"";

	CStegano::SteganoMethod method = CStegano::SteganoMethod::Stegano_LSB;
	EncodingSchemes compression = EncodingSchemes::Encode_None;

	char opt = parse_options(argc, argv, data_file, image_file, method, compression);

	wcout << L"\n\tImage Steganography utility, v0.1\n\tMariusz B. <mariusz.bit@gmail.com> 2014/15\n";
	wcout << endl;

#if 1 // defined (_DEBUG)
	if (opt == '?' || opt == 'h'){
		usage(argv[0]);
		return 0;
	}

	wchar_t imagePath[256] = L"";

	mbstowcs(imagePath, image_file, _countof(imagePath));
	mbstowcs(dataFile, data_file, _countof(dataFile));

	CStegano steg;
	if (!steg.init(imagePath, method, compression, g_verboseFlag))
	{
		wcout << "Couldn't load and parse working image file!" << endl;
		return 0;
	}

	if(opt == 'e')
	{
		// Encoding

		size_t sizeOfData = 0;
		std::unique_ptr<char> buffer = read_working_buffer(data_file, sizeOfData);
		if (!buffer)
		{
			return 0;
		}

		if (!copy_file(image_file))
		{
			wcout << "Couldn't make a backup copy of the image file!" << endl;
		}
		else
		{
			size_t encoded = steg.encode(buffer.get(), sizeOfData);
			wcout << L"Message has been encode inside the image file. Written bytes: " << encoded << endl;
		}
	}
	else if (opt == 'd')
	{
		// Decoding

		// Setting size of output data to the maximal size of the input file.
		size_t sizeOfData = getFileSize(image_file);

		const size_t sizeOfDecodedBuff = CDataEncoder::getBufferSizeForEncoded(compression, sizeOfData);
		std::unique_ptr<char[]> decodedBytes(new char[sizeOfDecodedBuff]);
		if (!decodedBytes)
		{
			return 0;
		}

		memset(decodedBytes.get(), 0, sizeOfData);
		size_t decoded = steg.decode(decodedBytes.get(), sizeOfDecodedBuff);

		write_working_buffer(data_file, decodedBytes.get(), decoded);
	}

	return 0;

#else

	const wchar_t *imagePath = LR"(d:\!_PK\Prace\3\GKIM\proj\images\image3.bmp)";
	//const wchar_t *imagePath = LR"(d:\data\mine\pk\gkim\proj\images\image3.bmp)";

	CStegano steg;
	steg.init(imagePath, method, compression, g_verboseFlag);

	// Encoding
	size_t encoded = steg.encode(Data_To_Encode, Size_Of_Data);
	if (encoded > Size_Of_Data)
	{
		wcout << L"Successfully encoded entire message" << endl;
	}
	else
	{
		wcout << L"Unable to encode entire message. Encoded bytes: " << encoded << endl;
	}

	// Decoding
	const size_t sizeOfDecodedBuff = CDataEncoder::getBufferSizeForEncoded(compression, Size_Of_Data);
	std::unique_ptr<char[]> decodedBytes(new char[sizeOfDecodedBuff]);
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

#endif
}