//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PsdReaderCmd.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef PSDREADER_H
#define PSDREADER_H

#include "headerReader.h"
#include "colorModeReader.h"
#include "imageResourceReader.h"
#include "layerAndMaskReader.h"
#include "imageDataReader.h"
#include "progress.h"
#include <functional>
using namespace util;

namespace psd_reader
{
	//----------------------------------------------------------------------------------------------
	// Data extract from the PSD.
	//----------------------------------------------------------------------------------------------
	struct PsdData
	{
		HeaderData HeaderData;
		ColorModeData ColorModeData;
		ImageResourceData ImageResourceData;
		LayerAndMaskData LayerMaskData;
		ImageData ImageData;
	};

	//----------------------------------------------------------------------------------------------
	class PsdReader
	{
	public:
		PsdReader(std::string const& pathFile);
		virtual	~PsdReader();
		void SetProgress(std::function<void(unsigned)>& initializeProgress, std::function<void(unsigned)>& initializeSubProgress, std::function<
		                 void()>& incrementProgress, std::function<void()>& completeSubProgress);
		PsdData ParsePsd();

	private:

		FILE* File;
		PsdProgress ProgressData;

		static bool DoesFileExist(const char* filename);
		
		
        void ParseSection(PsdData & data) const;
		bool LoadHeader(PsdData & data) const;
		bool LoadColorModeData(PsdData & data) const;
		bool LoadImageResource(PsdData & data) const;
		bool LoadLayerAndMask(PsdData& data) const;
		bool LoadImageData(PsdData & data) const;
	};
}

#endif // PSDREADER_H
