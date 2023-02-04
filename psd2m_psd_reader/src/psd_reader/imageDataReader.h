//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file ImageDataReader.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef IMAGEDATAREADER_H
#define IMAGEDATAREADER_H

#include "imageResourceReader.h"
#include "headerReader.h"
using namespace util;
namespace psd_reader
{
#pragma region DATA

	//----------------------------------------------------------------------------------------------
	struct ImageData
	{
		unsigned char* Data;

		ImageData();;

		~ImageData();;
	};

#pragma endregion

#pragma region READER

	//----------------------------------------------------------------------------------------------
	class ImageDataReader
	{
	public:
		static bool Read(FILE* file, ImageData& combinedImage, ResolutionInfo const& resInfo, HeaderData const& headerInfo);

	private:
		static void RleCompressionReader(FILE* file, ImageData & combinedImage, ResolutionInfo const& resInfo, HeaderData const& headerInfo);
		static void RawDataReader(FILE* file, ImageData & combinedImage, ResolutionInfo const& resInfo, HeaderData const& headerInfo);
	};

#pragma endregion
}

#endif // IMAGEDATAREADER_H
