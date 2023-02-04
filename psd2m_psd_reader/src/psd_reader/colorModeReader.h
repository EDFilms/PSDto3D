//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file ColorModeReader.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef COLORMODEREADER_H
#define COLORMODEREADER_H

#include <fstream>

namespace psd_reader
{
#pragma region DATA

	//----------------------------------------------------------------------------------------------
	struct ColorModeData
	{
		int Length;
		unsigned char* ColorData;

		ColorModeData()
		{
			Length = -1;
			ColorData = nullptr;
		};

		~ColorModeData()
		{
			delete ColorData;
		};
	};

#pragma endregion

#pragma region READER

	//----------------------------------------------------------------------------------------------
	class ColorModeReader
	{
	public:
		static bool Read(FILE* pFile, ColorModeData& colorModeInfo);
	};

#pragma endregion
}

#endif // COLORMODEREADER_H
