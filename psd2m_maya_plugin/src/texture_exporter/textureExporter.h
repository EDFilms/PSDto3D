//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file TextureExporter.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef TEXTUREEXPORTER_H
#define TEXTUREEXPORTER_H
#include <vector>
#include <maya/MString.h>
#include "psd_reader/layerAndMaskReader.h"

namespace maya_plugin
{
	class TextureExporter
	{
	public:
		static std::vector<unsigned char> ConvertIffFormat(bool isCropped, psd_reader::LayerData layer, int width, int height, int depth);
		static std::vector<unsigned char> ConvertToMask(bool isCropped, psd_reader::LayerData layer, int width, int height, int depth);
		static void SaveToDisk(MString path, std::vector<unsigned char>& srcTexture, int width, int height);
	};
}
#endif // TEXTUREEXPORTER_H
