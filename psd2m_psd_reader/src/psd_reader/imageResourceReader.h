//----------------------------------------------------------------------------------------------
// ===============================================
//	Copyright (C) 2016, CDRIN.
//	All Rights Reserved.
// ===============================================
//	Unauthorized copying of this file, via any medium is strictly prohibited
//	Proprietary and confidential
//
//	@file ImageResourceReader.h
//	@author Fabien Raspail
//	@date 11-11-2016
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef IMAGERESOURCEREADER_H
#define IMAGERESOURCEREADER_H

#include <vector>
#include "util/vectorialPath.h"
using namespace util;

namespace psd_reader
{
#pragma region DATA

	//----------------------------------------------------------------------------------------------
	struct ResolutionInfo
	{
		// Table A-6: ResolutionInfo structure
		//  Type    Name        Description
		//-------------------------------------------
		//  Fixed   hRes        Horizontal resolution in pixels per inch.
		//  int     hResUnit    1 = display horizontal resolution in pixels per inch; 2 = display horizontal resolution in pixels per cm.
		//  short   widthUnit   Display width as 1 = inches; 2 = cm; 3 = points; 4 = picas; 5 = columns.
		//
		//  Fixed   vRe         Vertical resolution in pixels per inch.
		//  int     vResUnit    1 = display vertical resolution in pixels per inch; 2 = display vertical resolution in pixels per cm.
		//  short   heightUnit  Display height as 1 = inches; 2 = cm; 3 = points; 4 = picas; 5 = columns.
		short HRes;
		int HResUnit;
		short WidthUnit;
		short VRes;
		int VResUnit;
		short HeightUnit;

		ResolutionInfo()
		{
			HRes = -1;
			HResUnit = -1;
			WidthUnit = -1;
			VRes = -1;
			VResUnit = -1;
			HeightUnit = -1;
		}
	};



	//----------------------------------------------------------------------------------------------
	struct ResourceBlockPath
	{
		std::string Name = "";
		std::vector<PathRecord> PathRecords;

		ResourceBlockPath() = default;;
		~ResourceBlockPath()
		{
			PathRecords.clear();
		};
	};

	//----------------------------------------------------------------------------------------------
	struct ImageResourceData
	{
		int Length;
		ResolutionInfo ResolutionInfo;
		std::vector<ResourceBlockPath> ResourceBlockPaths;

		ImageResourceData() : Length(0) { };

		~ImageResourceData()
		{
			ResourceBlockPaths.clear();
		}

		bool IsPathExist(std::string const& layerName) const
		{
			for (auto resourceBlockPath : this->ResourceBlockPaths)
			{
				if (resourceBlockPath.Name.compare(layerName) == 0)
				{
					return true;
				}
			}
			return false;
		}

		ResourceBlockPath GetBlockPath(std::string const& layerName)
		{
			for (auto resourceBlockPath : this->ResourceBlockPaths)
			{
				if (resourceBlockPath.Name.compare(layerName) == 0)
				{
					return resourceBlockPath;
				}
			}

			return ResourceBlockPath();
		}
	};

#pragma endregion

#pragma region READER

	//----------------------------------------------------------------------------------------------
	class ImageResourceReader
	{
	public:
		ImageResourceReader() = default;;
		~ImageResourceReader() = default;;
		static float fixedPoint(const unsigned char* value, bool checkSigne, int size, int positionPoint);
		static bool Read(FILE* file, ImageResourceData& imageResource);

	private:
		static bool ReadResourceBlocks(FILE* file, ImageResourceData& imageResource, int& bytesRead);
		static bool ReadPaths(FILE* file, ResourceBlockPath & resourceBlockPath, int sizeBlock, int& bytesRead);
		static bool ReadResolutionInfo(FILE* file, ImageResourceData& imageResource, int& bytesRead);
		static util::PathPoints* ReadPathPoint(FILE* file, int& bytesRead);
	};

#pragma endregion
}

#endif // IMAGERESOURCEREADER_H
