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
#include "util\progressJob.h"
#include <functional>
using namespace util;

namespace psd_reader
{
	//----------------------------------------------------------------------------------------------
	// Data extract from the PSD.
	//----------------------------------------------------------------------------------------------
	struct PsdData
	{
		// Photoshop format blocks, see https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/
		HeaderData HeaderData;					// "File Header" block
		ColorModeData ColorModeData;			// "Color Mode Data" block
		ImageResourceData ImageResourceData;	// "Image Resources" block
		LayerAndMaskData LayerMaskData;			// "Layer and Mask Information" block
		ImageData ImageData;					// "Image Data" block
	};

	//----------------------------------------------------------------------------------------------
	class PsdReader
	{
	public:
		PsdReader(const char* filename_utf8);
		virtual	~PsdReader();
		void SetProgress( ProgressJob* progressJob );
		PsdData ParsePsd();

	private:

		FILE* File;
		ProgressJob* ProgressJob;

		static bool DoesFileExist(const char* filename_utf8);
		
		
        void ParseSection(PsdData & data) const;
		bool LoadHeader(PsdData & data) const;
		bool LoadColorModeData(PsdData & data) const;
		bool LoadImageResource(PsdData & data) const;
		bool LoadLayerAndMask(PsdData& data) const;
		bool LoadImageData(PsdData & data) const;
	};
}

#endif // PSDREADER_H
