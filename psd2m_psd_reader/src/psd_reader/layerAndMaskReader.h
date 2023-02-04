//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file LayerAndMashReader.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef LAYERANDMASKREADER_H
#define LAYERANDMASKREADER_H

#include <vector>
#include <iostream>
#include "util/vectorialPath.h"
#include "headerReader.h"
#include "progress.h"

namespace psd_reader
{
#pragma region DATA
	
	static const std::string KEY_LAYERS = "Lr16";
	static const std::string KEY_GROUP = "lsct";
	static const std::string KEY_VECTOR_MASK = "vmsk";
	static const std::string KEY_MASK = "vsms";

	static const int BYTE_VALUE = 8;

	enum TYPE_LAYER
	{
		TEXTURE_LAYER = -1,
		ANY_OTHER_TYPE_LAYER = 0,
		OPEN_FOLDER = 1,
		CLOSED_FOLDER = 2,
		HIDDEN_DIVIDER = 3,
		INFLUENCE_LAYER = 4
	};

	struct LayerData
	{
		std::string LayerName;
		TYPE_LAYER Type = TEXTURE_LAYER;
		std::vector<short> ChannelId;
		std::vector<int> ChannelLength;
		std::vector<util::PathRecord> PathRecords;
		
		int NbrChannel = 0;
		int AnchorTop = 0;
		int AnchorRight = 0;
		int AnchorBottom = 0;
		int AnchorLeft = 0;

		// Layer texture DATA stored per Channel, ordered ARGB.
		std::vector<unsigned char*> ImageContent;

		LayerData()= default;
		~LayerData()
		{
			LayerName = "";
		};
	};

	//----------------------------------------------------------------------------------------------
	struct LayerAndMaskData
	{
		short LayerCount;
		std::vector<LayerData> Layers;
		static const std::string INFLUENCE_LAYER_TAG;

		LayerAndMaskData()
		{
			LayerCount = 0;
		};

		~LayerAndMaskData()
		{
			Layers.clear();
			LayerCount = 0;
		};

		int GetIndexInfluenceLayer(std::string name) const
		{
			for(auto i = 0; i < Layers.size(); i++)
			{
				if (Layers[i].Type == INFLUENCE_LAYER &&
					LayerNameInfluenceAssociated( Layers[i].LayerName) == name)
				{
					return i;
				}
			}
			return -1;
		};

		static std::string LayerNameInfluenceAssociated(std::string const& str);
	};

#pragma endregion

#pragma region READER

	//----------------------------------------------------------------------------------------------
	class LayerAndMaskReader
	{
	public:
		static const int PATH_BLOCK_SIZE;

		LayerAndMaskReader() = default;
		~LayerAndMaskReader() = default;
		static bool Read(FILE* file, const HeaderData& headerData, LayerAndMaskData & layerMaskData, PsdProgress const& progress);
		static void ProcessLayerMaskInformation(FILE* file, int& bytesRead, const HeaderData& headerData, LayerAndMaskData& layerMaskData, PsdProgress const& progress);

	private:
		static bool ReadLayerInfoSection(FILE* file, int& bytesRead, const HeaderData& headerData, LayerAndMaskData& layerMaskData, PsdProgress const& progress);
		static bool ReadLayerRecordsSection(FILE * file, int & bytesRead, LayerData& layerData);
		static bool ReadLayerInfoSectionExtraDataField(FILE* file, int& bytesRead, LayerData& layerData);
		static bool ReadChannelImageData(FILE* file, int& bytesRead, int channelDepth, LayerAndMaskData& layerMaskData, PsdProgress const& progress);
		static bool ReadGlobalLayerMaskInfo(FILE* file, int& bytesRead, LayerAndMaskData& layerMaskData);
		static bool ReadHeaderAdditionalLayerInfo(FILE* file, int& bytesRead, LayerData& layerData);
		static bool ReadHeaderAdditionalLayerGlobalInfo(FILE* file, int& bytesRead, const HeaderData& headerData, LayerAndMaskData& layerDataMaskData, PsdProgress const& progress);
		static void AdditionnalLayerDataGlobal(FILE* file, int& bytesRead, const HeaderData& headerData, LayerAndMaskData& layerDataMaskData, PsdProgress const& progress);
		static void AdditionnalLayerDataLayer(FILE* file, int& bytesRead, LayerData& layerData);
		static void SectionDividerSetting(FILE * file, int const & size, LayerData& layerData);
		static void ReadVectorMask(FILE* file, int const& size, LayerData& layerData);
		static bool ReadPaths(FILE* file, std::vector<util::PathRecord>& pathRecords, int sizeBlock);
		static util::PathPoints* ReadPathPoint(FILE* file);
		static bool psd_unzip_without_prediction(unsigned char *src_buf, int src_len, unsigned char *dst_buf, int dst_len);
		static bool psd_unzip_with_prediction(unsigned char* src_buf, int src_len, unsigned char* dst_buf, int dst_len, int row_size, int color_depth);
	};

#pragma endregion


}

#endif // LAYERANDMASKREADER_H