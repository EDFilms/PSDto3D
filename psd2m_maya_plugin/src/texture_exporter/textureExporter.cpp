//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file textureExporter.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "textureExporter.h"
#include "psd_reader/layerAndMaskReader.h"
#include <maya/MTextureManager.h>
#include <algorithm>

namespace maya_plugin
{
	//--------------------------------------------------------------------------------------------------------------------------------------
	std::vector<unsigned char> TextureExporter::ConvertIffFormat(bool isCropped, psd_reader::LayerData layer, int width, int height, int depth)
	{
		const int bytesPerPixel = depth / 8;
		std::vector<unsigned char> textureIff;
		if (layer.NbrChannel < 4) return textureIff;
		auto const length = (layer.AnchorBottom - layer.AnchorTop) * (layer.AnchorRight - layer.AnchorLeft);
		textureIff.reserve(width * height * 4 * bytesPerPixel);

		int padTop = std::max(0, ( 0 - layer.AnchorTop));
		int padHeightTop = std::max(0, layer.AnchorTop);
		int padLeft = std::max(0, (0 - layer.AnchorLeft));
		int padRight = std::max(0, (layer.AnchorRight - width));

		int rowSize = layer.AnchorRight - layer.AnchorLeft;
		int globalPaddingTop = padTop * rowSize ;

		// fill content
		int indexProgression = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{				
				if (!isCropped && (i < layer.AnchorTop || i >= layer.AnchorBottom || j < layer.AnchorLeft || j >= layer.AnchorRight))
				{
					for (int k = 0; k < bytesPerPixel; k++)
					{
						textureIff.push_back(0);
						textureIff.push_back(0);
						textureIff.push_back(0);
						textureIff.push_back(0);
					}
				}
				else
				{
					int pos = indexProgression + ((globalPaddingTop + (padLeft * (i - padHeightTop + 1)) + (padRight * (i - padHeightTop))) * bytesPerPixel);

					for (int k = 0; k < bytesPerPixel; k++)
					{
						textureIff.push_back(layer.ImageContent[1][pos + k]);
					}
					for (int k = 0; k < bytesPerPixel; k++)
					{
						textureIff.push_back(layer.ImageContent[2][pos + k]);
					}for (int k = 0; k < bytesPerPixel; k++)
					{
						textureIff.push_back(layer.ImageContent[3][pos + k]);
					}for (int k = 0; k < bytesPerPixel; k++)
					{
						textureIff.push_back(layer.ImageContent[0][pos + k]);
					}
					indexProgression += bytesPerPixel;
				}
			}
		}
		return textureIff;
	}
	//--------------------------------------------------------------------------------------------------------------------------------------
	std::vector<unsigned char> TextureExporter::ConvertToMask(bool isCropped, psd_reader::LayerData layer, int width, int height, int depth)
	{
		const int bytesPerPixel = depth / 8;
		std::vector<unsigned char> maskTexture;
		auto const length = (layer.AnchorBottom - layer.AnchorTop) * (layer.AnchorRight - layer.AnchorLeft);
		maskTexture.reserve(width * height * bytesPerPixel);

		int padTop = std::max(0, (0 - layer.AnchorTop));
		int padHeightTop = std::max(0, layer.AnchorTop);
		int padLeft = std::max(0, (0 - layer.AnchorLeft));
		int padRight = std::max(0, (layer.AnchorRight - width));

		int rowSize = layer.AnchorRight - layer.AnchorLeft;
		int globalPaddingTop = padTop * rowSize;

		// fill content
		int indexProgression = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				if (!isCropped && (i < layer.AnchorTop || i >= layer.AnchorBottom || j < layer.AnchorLeft || j >= layer.AnchorRight))
				{
					for (int k = 0; k < bytesPerPixel; k++)
					{
						maskTexture.push_back(0);
					}
				}
				else
				{
					int pos = indexProgression + ((globalPaddingTop + (padLeft * (i - padHeightTop + 1)) + (padRight * (i - padHeightTop))) * bytesPerPixel);

					for (int k = 0; k < bytesPerPixel; k++)
					{
						maskTexture.push_back(layer.ImageContent[1][pos + k]);
					}
					indexProgression += bytesPerPixel;
				}
			}
		}
		return maskTexture;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void TextureExporter::SaveToDisk(MString path, std::vector<unsigned char>& srcTexture, int width, int height)
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		MHWRender::MTextureManager* textureMgr = renderer ? renderer->getTextureManager() : NULL;

		if (textureMgr)
		{
			unsigned char* fTextureData = srcTexture.data();
			MHWRender::MTextureDescription desc;
			{
				desc.setToDefault2DTexture();
				desc.fWidth = width;
				desc.fHeight = height;
				desc.fDepth = 1;
				desc.fBytesPerRow = width * 4;
				desc.fFormat = MHWRender::kR8G8B8A8_UNORM;
				desc.fTextureType = MHWRender::kImage2D;
			}
			MHWRender::MTexture* ftexture = textureMgr->acquireTexture("tmp_Export_texture_PSD2M", desc, fTextureData);
			textureMgr->saveTexture(ftexture, path);
			textureMgr->releaseTexture(ftexture);
		}
	}
}
