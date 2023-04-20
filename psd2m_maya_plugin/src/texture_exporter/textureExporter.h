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
#include <string>
#if defined PSDTO3D_MAYA_VERSION
#include <maya/MString.h>

typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <QtCore/QList>
#include <QtCore/QString>
#else
#include "mayaStub.h"
#endif
#include "util/bounds_2D.h"
#include "psd_reader/layerAndMaskReader.h"

namespace psd_to_3d
{
	using util::boundsPixels;
	using util::boundsUV;
	using util::ProgressTask;

	template <class T>
	struct TexturePixel
	{
		T r,g,b,a;
		void Defringe( const TexturePixel<T>& src ); // copy color from src, if this is transparent (a==0) and src is non-transparent (a>0)
	};

	class TextureMap
	{
	public:
		TextureMap() : width(0), height(0) {}
		virtual ~TextureMap() {}
		void Init( int width, int height ); // assume output is rgba 1 byte per channel

		// accessors
		int Width()		{ return width; }
		int Height()	{ return height; }
		std::vector<unsigned char>& Buffer() { return p; }
		unsigned char* Data() { return (p.empty()? nullptr : p.data()); }
		size_t Size()	{ return p.size(); }
		bool Empty()	{ return p.empty(); }

		void ApplyScaling( int width_output, int height_output );
		void ApplyDefringe( int radius );
		void ApplyPadding( int padding );

	protected:
		std::vector<unsigned char> p;
		int width, height;
	};

	class TextureExporter
	{
	public:
		static void ConvertIffFormat(TextureMap& destTex, const psd_reader::LayerData& srcLayer,
			int srcBitDepth, const boundsPixels bounds ); // padding and scaling defined and handled by TextureConverter as appropriate
		static void ConvertToMask(TextureMap& destTex, const psd_reader::LayerData& srcLayer, int width, int height, int srcBitDepth);
		static boundsPixels GetContentRegion( const psd_reader::LayerData& layer, int width, int height, int depth, int alphaThresh );
		static void Defringe(TextureMap& tex, int radius);
		static void SaveToDiskLibpng(const std::string filepath, std::vector<unsigned char>& srcTexture, int width, int height, ProgressTask& progressTask );
		static void SaveToDiskMTexture(const std::string filepath, std::vector<unsigned char>& srcTexture, int width, int height);
	};
}
#endif // TEXTUREEXPORTER_H
