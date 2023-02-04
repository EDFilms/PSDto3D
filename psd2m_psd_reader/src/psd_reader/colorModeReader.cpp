//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file ColorModeReader.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include <string>
#include <iostream>
#include "colorModeReader.h"
#include "util/utils.h"

using namespace util;

namespace psd_reader
{
	//----------------------------------------------------------------------------------------
	bool ColorModeReader::Read(FILE* pFile, ColorModeData& colorModeInfo)
	{
		if (colorModeInfo.Length > 0)
		{
			delete[] colorModeInfo.ColorData;
		}
		colorModeInfo.ColorData = nullptr;

		// Get the length
		unsigned char length[4];
		fread(&length, sizeof(length), 1, pFile);
		colorModeInfo.Length = Utils::Calculate(length, sizeof(colorModeInfo.Length));
		const std::string mess("[COLOR MODE DATA] Size " + std::to_string(colorModeInfo.Length));

		std::cout << mess << std::endl;
		// Return if nothing to get.
		if (colorModeInfo.Length <= 0) return true;

		// Get the Color Data.
		colorModeInfo.ColorData = new unsigned char[colorModeInfo.Length];	
		memset(colorModeInfo.ColorData, 254, colorModeInfo.Length);
		fread(colorModeInfo.ColorData, colorModeInfo.Length, 1, pFile);
		return true;
	}
}
