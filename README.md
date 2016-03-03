# stegano1

Simple implementation of couple compression algorithms and image stegano methods. This project comes as a library to be included in desired project. As a usage presentation the `test.cpp` and `test.exe` files have been delivered.

## Usage

```
> test.exe

Encoding (data len: 89)...

First 62 bytes of data before encoding:
    0  57 57 57 57 57 57 57 57 69 69 69 69 61 61 61 61  WWWWWWWWiiiiaaaa
   10  61 61 69 69 69 69 64 64 64 6f 6f 6f 6f 6d 6d 6f  aaiiiidddoooommo
   20  6f 73 63 20 74 65 73 74 74 65 73 74 6f 77 61 6f  osc testtestowao
   30  77 61 2c 20 61 6c 61 6c 61 6c 61 20 6d 6d        wa, alalala mm

First 62 bytes of data encoded using LZW
    0  57 00 00 01 01 01 00 01 69 00 04 01 69 00 61 00  W.......i...i.a.
   10  07 01 08 01 05 01 69 00 64 00 0c 01 6f 00 0e 01  ......i.d...o...
   20  6f 00 6d 00 6d 00 0e 01 73 00 63 00 20 00 74 00  o.m.m...s.c. .t.
   30  65 00 73 00 74 00 17 01 19 01 6f 00 77 00 61 00  e.s.t.....o.w.a.
   40  1d 01 61 00 2c 00 20 00 61 00 6c 00 24 01 26 01  ..a.,. .a.l.$.&.
   50  20 00 11 01 29 01 6d 00 07 01 20 00 6b 00 6f 00   ...).m... .k.o.
   60  74 00 1f 01 30 01 2d 01 2f 01 28 01 61 00 23 01  t...0.-./.(.a.#.
   70  6c 00 65 00 20 00 00 00                          l.e. ...

Compression rate: 120 / 89, ratio: 1.35
Decoding...

First 62 bytes of data decoded using LZW
    0  57 57 57 57 57 57 57 57 69 69 69 69 61 61 61 61  WWWWWWWWiiiiaaaa
   10  61 61 69 69 69 69 64 64 64 6f 6f 6f 6f 6d 6d 6f  aaiiiidddoooommo
   20  6f 73 63 20 74 65 73 74 74 65 73 74 6f 77 61 6f  osc testtestowao
   30  77 61 2c 20 61 6c 61 6c 61 6c 61 20 6d 6d        wa, alalala mm

Compression algorithm is working correctly.

```

One can use the `CStegano` class for proper steganographic class instance. Then by using methods like:
* `CStegano::loadImage`
* `CStegano::encode`
* `CStegano::decode`

perform actual image processing. As it has been stated in `CStegano` declaration, the library currently supports (more or less) following methods:

```
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
```

also, following compression methods can be leveraged:

```
enum EncodingSchemes
{
	Encode_None,
	Encode_Base64,
	Encode_Compress_ByteRun,
	Encode_Compress_RLE,
	Encode_Compress_LZW,
};
```

as stated in DataEncoder.h