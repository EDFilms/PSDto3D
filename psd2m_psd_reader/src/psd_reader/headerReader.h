//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file HeaderReader.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef HEADERREADER_H
#define HEADERREADER_H

#include <fstream>

namespace psd_reader
{
#pragma region DATA

	//----------------------------------------------------------------------------------------------
	struct HeaderData
	{
		short Channels; // The number of channels in the image, including any alpha channels. Supported range is 1 to 56.
		int Height; // The height of the image in pixels. Supported range is 1 to 30,000. (**PSB** max of 300, 000.)
		int Width; // The width of the image in pixels. Supported range is 1 to 30,000. (*PSB** max of 300, 000)
		short BitsPerPixel; // Depth: the number of bits per channel. Supported values are 1, 8, 16 and 32.
		short ColourMode; // The color mode of the file. Supported values are: Bitmap = 0; Grayscale = 1; Indexed = 2; RGB = 3; CMYK = 4; Multichannel = 7; Duotone = 8; Lab = 9.

		HeaderData()
		{
			Channels = -1;
			Height = -1;
			Width = -1;
			BitsPerPixel = -1;
			ColourMode = -1;
		};
		~HeaderData() {};
	};

#pragma endregion

#pragma region READER

	//----------------------------------------------------------------------------------------------
	class HeaderReader
	{
	public:
		static bool Read(FILE* file, HeaderData& headerData);
	};

#pragma endregion
}

#endif // HEADERREADER_H