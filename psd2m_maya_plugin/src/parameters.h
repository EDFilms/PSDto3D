//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file parameters.h
//  @author Benjamin Drouin
//  @date 19-10-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <vector>
#include "ui_toolWidget.h"
#include <maya/MQtUtil.h>
#include "psd_reader/psdReader.h"
#include "mesh_generator/curve_mesh/curveMeshGenerator.h"
#include "mesh_generator/influence_mesh/influenceMesh.h"
#include "mesh_generator/linear_mesh/linearMesh.h"
#include "json/JSON.h"

namespace maya_plugin
{
	struct LayerParameters
	{
		enum Algorithm
		{
			LINEAR,
			CURVE
		};

		bool IsActive = true;
		Algorithm Algo = Algorithm::LINEAR;

		QLabel* LabelAlgoSelected;
		QLabel* LabelInfluence;
		QLabel* LabelDescription;
		mesh_generator::LinearParameters LinearParameters;
		mesh_generator::CurveParameters CurveParameters;
		mesh_generator::InfluenceParameters InfluenceParameters;

		// Influence Parameters
		bool HasInfluenceLayer = false;
		bool InfluenceActivated = true;
		bool HasVectorMask = false;
		bool HasGlobalPath = false;

		void SetInfluenceLayer(psd_reader::PsdData const& psdData, psd_reader::LayerData const& layer)
		{
			this->HasInfluenceLayer = psdData.LayerMaskData.GetIndexInfluenceLayer(layer.LayerName) != -1;
			this->HasGlobalPath = psdData.ImageResourceData.IsPathExist(layer.LayerName);
			this->HasVectorMask = !layer.PathRecords.empty();
		}

		void UpdateDescription() const;
	};

	struct GlobalParameters
	{
		// State modification
		bool Generate = false;
		bool Exportation = false;

		bool Exported = false;
		bool Generated = false;
		bool SelectionChanged = false;

		QString PsdName = "";
		QString FilePath = "";

		// Settings
		float Depth = 0;
		float Scale = 1;
		bool KeepGroupStructure = true;
		QString AliasPsdName = "";

		// Layer Management
		void UpdateLayers(psd_reader::PsdData const& psdData);
		LayerParameters* GetLayerParameter(std::string const& name);
		std::vector<LayerParameters*> GetAllParameters();
		void ClearLayerParameters();

		// Json Serialization
		void UpdateValuesFromJson();
		void WriteValuesToJson();

	private:
		std::map<std::string, LayerParameters*> NameLayerMap;

		void SetDefaultValues();
		void DeserializeContents(JSONObject& root);
		JSONValue* SerializeContents();

		static std::wstring StringToWString(const std::string& str);
		static std::string WStringToString(const std::wstring& str);
	};
}
#endif // PARAMETERS_H
