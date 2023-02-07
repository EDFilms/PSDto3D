//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file ImageResourceReader.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include <cassert>
#include <bitset>
#include "imageResourceReader.h"
#include "util/utils.h"
#include <algorithm>

namespace psd_reader
{
	typedef unsigned char BYTE;

	//----------------------------------------------------------------------------------------
	static float BitValue(const char& val, int index)
	{
		return (val & (1 << index)) ? 1.0f : 0.0f;
	}

	//----------------------------------------------------------------------------------------
	float ImageResourceReader::fixedPoint(const unsigned char*  value, bool checkSign, int size, int positionPoint)
	{
		float res    = 0.f;
		float sign = BitValue(value[0], 7);
		int exposant = positionPoint - 5; // 3 bits guard

		for (int i = 4; i < size; i++)
		{
			float bit = BitValue(value[i / 8], 7 - (i % 8));
			float power = float(pow(2, exposant));
			res += (bit * power);
			exposant--;
		}
		if(checkSign)
		{
			int signe = int(BitValue(value[0], 7));
			res = res - (16.0f * sign);
		}
		return res;
	}

	//----------------------------------------------------------------------------------------
	bool ImageResourceReader::Read(FILE* file, ImageResourceData& imageResource)
	{
		const bool success = false;

		// Get length
		unsigned char length[4];
		fread(&length, sizeof(length), 1, file);
		imageResource.Length = util::StringAsInt(length, sizeof(imageResource.Length));

		// start the reading
		int bytesRead = 0;
		const int totalBytes = imageResource.Length;

		while (!feof(file) && (bytesRead < totalBytes))
		{
			if (!ReadResourceBlocks(file, imageResource, bytesRead)) return success;
		}
		return (bytesRead == totalBytes);
	}

	//----------------------------------------------------------------------------------------
	bool ImageResourceReader::ReadResourceBlocks(FILE* file, ImageResourceData& imageResource, int& bytesRead)
	{
		char osType[4];
		char* name = nullptr;
		bool success = false;

	
		// Read OSType
		fread(&osType, sizeof(osType), 1, file);
		bytesRead += sizeof(osType);
		assert(0 == (bytesRead % 2));

		// Read resource ID
		unsigned char ID[2];
		fread(&ID, sizeof(ID), 1, file);
		bytesRead += sizeof(ID);
		short id = (short)util::StringAsInt(ID, sizeof(ID));

		// Read Name
		unsigned char sizeReaded;
		fread(&sizeReaded, sizeof(sizeReaded), 1, file);
		bytesRead += sizeof(sizeReaded);

		// Pascal string, padded to make the size even (a null name consists of two bytes of 0)
		int elementSize = util::StringAsInt(&sizeReaded, sizeof(sizeReaded));
		if( 0 < elementSize)
		{ 
			name = new char[elementSize];
			fread(name, elementSize, 1, file);
			bytesRead += sizeof(char) * elementSize;
		}

		if(0 == (elementSize % 2))
		{
			fread(&sizeReaded, sizeof(sizeReaded), 1, file);
			bytesRead += sizeof(unsigned char);
		}
	
		// Read size
		unsigned char Size[4];
		fread(&Size, sizeof(Size), 1, file);
		bytesRead += sizeof(Size);
		int size = util::StringAsInt(Size, sizeof(size));
		if (size == 0) return true;
		size += size % 2;

		// Continue if OSType == "8BIM"
		if (!util::CheckSignature(osType, "8BIM")) return success;
		
		// Read the specific Image Resource IDs
		if (id >= 2000 && id <= 2997 /*|| id == 2999 || id == 1025*/)
		{
			ResourceBlockPath resourcebp;
			std::string vectorName = std::string(name, elementSize);
			resourcebp.Name = vectorName;
			ReadPaths(file, resourcebp, size, bytesRead);
			imageResource.ResourceBlockPaths.push_back(resourcebp);
		}
		else if (id == 1005)
		{
			ReadResolutionInfo(file, imageResource, bytesRead);
		}
		else
		{
			unsigned char c[1];
			for (int n = 0; n < size; ++n)
			{
				fread(&c, sizeof(c), 1, file);
				bytesRead += sizeof(c);
			}
		}
		return true;
	}

	//----------------------------------------------------------------------------------------
	// TODO: Duplicate code from LayerAndMaskReader::ReadPaths()
	bool ImageResourceReader::ReadPaths(FILE* file, ResourceBlockPath & resourceBlockPath, int sizeBlock, int& bytesRead)
	{
		unsigned char selectorData[2];
		const int size = sizeBlock / 26;

		PathRecord pathRecord;

		for (int i = 0; i < size; i++)
		{
			fread(selectorData, sizeof(selectorData), 1, file);
			bytesRead += sizeof(selectorData);
			const auto selector = short(util::StringAsInt(selectorData, sizeof(selectorData)));
			switch (selector)
			{
			case 0:
			case 3:
			{
				// Disregard paths containing only a single point or no points, and
				// disregard paths claiming to be closed paths but with only two points
				if( (pathRecord.Points.size() >= 2) && !(pathRecord.IsClosedPath && (pathRecord.Points.size()==2)) )
				{
					resourceBlockPath.PathRecords.push_back(pathRecord);
				}

				pathRecord = PathRecord();
				pathRecord.IsClosedPath = (selector == 0);

				unsigned char pathRecordData[24];
				fread(pathRecordData, sizeof(pathRecordData), 1, file);
				bytesRead += sizeof(pathRecordData);
				break;
			}
			case 1:
			case 4:
			case 2:
			case 5:
			{
				PathPoints* points = ReadPathPoint(file, bytesRead);
				points->IsLinked = (selector == 1 || selector == 4);
				pathRecord.Points.push_back(points);
				break;
			}
			default:
			{
				unsigned char pathRecordData[24];
				fread(pathRecordData, sizeof(pathRecordData), 1, file);
				bytesRead += sizeof(pathRecordData);
				break;
			}
			}
		}
		if (!pathRecord.Points.empty() && !(pathRecord.IsClosedPath && pathRecord.Points.size() < 2))
		{
			resourceBlockPath.PathRecords.push_back(pathRecord);
		}
		return true;
	}

	//----------------------------------------------------------------------------------------
	bool ImageResourceReader::ReadResolutionInfo(FILE* file, ImageResourceData & imageResource, int& bytesRead)
	{
		unsigned char intValue[4];
		unsigned char shortValue[2];

		fread(&shortValue, sizeof(shortValue), 1, file);
		bytesRead += sizeof(shortValue);
		imageResource.ResolutionInfo.HRes = short(util::StringAsInt(shortValue, sizeof(imageResource.ResolutionInfo.HRes)));

		fread(&intValue, sizeof(intValue), 1, file);
		bytesRead += sizeof(intValue);
		imageResource.ResolutionInfo.HResUnit = util::StringAsInt(intValue, sizeof(imageResource.ResolutionInfo.HResUnit));

		fread(&shortValue, sizeof(shortValue), 1, file);
		bytesRead += sizeof(shortValue);
		imageResource.ResolutionInfo.WidthUnit = short(util::StringAsInt(shortValue, sizeof(imageResource.ResolutionInfo.WidthUnit)));

		fread(&shortValue, sizeof(shortValue), 1, file);
		bytesRead += sizeof(shortValue);
		imageResource.ResolutionInfo.VRes = short(util::StringAsInt(shortValue, sizeof(imageResource.ResolutionInfo.VRes)));

		fread(&intValue, sizeof(intValue), 1, file);
		bytesRead += sizeof(intValue);
		imageResource.ResolutionInfo.VResUnit = util::StringAsInt(intValue, sizeof(imageResource.ResolutionInfo.VResUnit));

		fread(&shortValue, sizeof(shortValue), 1, file);
		bytesRead += sizeof(shortValue);
		imageResource.ResolutionInfo.HeightUnit = short(util::StringAsInt(shortValue, sizeof(imageResource.ResolutionInfo.HeightUnit)));

		return true;
	}

	//----------------------------------------------------------------------------------------
	PathPoints* ImageResourceReader::ReadPathPoint(FILE * file, int & bytesRead)
	{
		
		unsigned char pathRecordData[24];
		PathPoints* pathPoints = new PathPoints();

		fread(pathRecordData, sizeof(pathRecordData), 1, file);
		bytesRead += sizeof(pathRecordData);

		pathPoints->SegIn.y = fixedPoint(&pathRecordData[0], true, 32, 8);
		pathPoints->SegIn.x = fixedPoint(&pathRecordData[4], true, 32, 8);

		pathPoints->AnchorPoint.y = fixedPoint(&pathRecordData[8], true, 32, 8);
		pathPoints->AnchorPoint.x = fixedPoint(&pathRecordData[12], true, 32, 8);

		pathPoints->SegOut.y = fixedPoint(&pathRecordData[16], true, 32, 8);
		pathPoints->SegOut.x = fixedPoint(&pathRecordData[20], true, 32, 8);

		return pathPoints;
	}
}
