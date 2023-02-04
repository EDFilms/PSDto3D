//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file HeaderReader.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "headerReader.h"
#include "util/utils.h"

namespace psd_reader
{
	//----------------------------------------------------------------------------------------
	bool HeaderReader::Read(FILE* pFile, HeaderData& headerInfo)
	{
		struct HEADER
		{
			char Signature[4];	// always equal 8BPS, do not read file if not
			unsigned char Version[2];	// always equal 1, do not read file if not
			char Reserved[6];	// must be zero
			unsigned char Channels[2];	// number of channels including any alpha channels, supported range 1 to 24
			unsigned char Rows[4];		// height in PIXELS, supported range 1 to 30000
			unsigned char Columns[4];	// width in PIXELS, supported range 1 to 30000
			unsigned char Depth[2];		// number of bpp
			unsigned char Mode[2];		// color mode of the file, Bitmap=0, Grayscale=1, Indexed=2, RGB=3, CMYK=4, Multichannel=7, Duotone=8, Lab=9
		};

		HEADER header{};
		bool success = false;

		const auto itemsRead = int(fread(&header, sizeof(HEADER), 1, pFile));
		if (!itemsRead) return success;
		if (!util::Utils::CheckSignature(header.Signature, "8BPS")) return success;

		const int version = util::Utils::Calculate(header.Version, sizeof(header.Version));
		if (1 != version) return success;

		for (char i : header.Reserved)
		{
			if ('\0' != i) return success;
		}

		success = true;
		headerInfo.Channels = short(util::Utils::Calculate(header.Channels, sizeof(header.Channels)));
		headerInfo.Height = util::Utils::Calculate(header.Rows, sizeof(header.Rows));
		headerInfo.Width = util::Utils::Calculate(header.Columns, sizeof(header.Columns));
		headerInfo.BitsPerPixel = short(util::Utils::Calculate(header.Depth, sizeof(header.Depth)));
		headerInfo.ColourMode = short(util::Utils::Calculate(header.Mode, sizeof(header.Mode)));
		return success;
	}
}
