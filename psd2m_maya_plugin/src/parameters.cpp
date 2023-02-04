// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file parameters.cpp
//  @author Benjamin Drouin
//  @date 19-10-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "parameters.h"
#include <QFileInfo>
#include <locale>
#include <codecvt>

namespace maya_plugin
{
#pragma region LAYER PARAMETERS

	//--------------------------------------------------------------------------------------------------------------------------------------
	void LayerParameters::UpdateDescription() const
	{
		QString algoSelected;
		algoSelected.append("Algo selected: ");
		if (this->Algo == LayerParameters::LINEAR)
			algoSelected.append("<b>Linear</b>");
		else
			algoSelected.append("<b>Curve</b>");
		this->LabelAlgoSelected->setText(algoSelected);

		QString Influence;
		Influence.append("Influence: ");
		if (this->HasInfluenceLayer)
			Influence.append(this->InfluenceActivated ? "<b>Active</b>" : "<b>Inactive</b>");
		else
			Influence.append("<font color='#f48c42'>Not available</font >");
		this->LabelInfluence->setText(Influence);


		QString description;
		description.append("Linear (path): ");
		description.append(this->HasGlobalPath ? "<font color='green'>Available</font>" : "<font color='#f48c42'>Not available </font>");
		description.append(" ... | ... Curve (layer mask): ");
		description.append(this->HasVectorMask ? "<font color='green'>Available </font>" : "<font color='#f48c42'>Not available</font>");
		this->LabelDescription->setText(description);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void GlobalParameters::UpdateLayers(psd_reader::PsdData const& psdData)
	{
		std::map<std::string, LayerParameters*> copy(this->NameLayerMap.begin(), this->NameLayerMap.end());
		this->NameLayerMap.clear();

		for (auto layer : psdData.LayerMaskData.Layers)
		{
			if (layer.Type > psd_reader::TEXTURE_LAYER)
				continue;

			LayerParameters* layerParam = nullptr;

			const auto it = copy.find(layer.LayerName);
			if (it == copy.end())
				layerParam = new LayerParameters();
			else
				layerParam = it->second;

			layerParam->SetInfluenceLayer(psdData, layer);
			this->NameLayerMap.try_emplace(layer.LayerName, layerParam);
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	LayerParameters* GlobalParameters::GetLayerParameter(std::string const& name)
	{
		return this->NameLayerMap.at(name);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	std::vector<LayerParameters*> GlobalParameters::GetAllParameters()
	{
		std::vector<LayerParameters*> params;
		for (auto layerMap : NameLayerMap)
		{
			params.push_back(layerMap.second);
		}

		return params;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void GlobalParameters::ClearLayerParameters()
	{
		for (auto layerMap : this->NameLayerMap)
		{
			delete layerMap.second;
		}

		this->NameLayerMap.clear();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void GlobalParameters::UpdateValuesFromJson()
	{
		// Try to get the params metadata.
		QFileInfo qtFilePath(this->FilePath);
		QFileInfo paramsPath = qtFilePath.path() + "/" + qtFilePath.baseName() + "/parameters.json";
		if (!paramsPath.exists())
		{
			SetDefaultValues();
			return;
		}

		// Try to open the metadata.
		MString path = MQtUtil::toMString(paramsPath.absoluteFilePath());
		std::ifstream file(path.asChar());
		if (!file.is_open())
		{
			SetDefaultValues();
			return;
		}

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		JSONValue* value = JSON::Parse(content.c_str());
		JSONObject root = value->AsObject();
		DeserializeContents(root);

		delete value;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void GlobalParameters::WriteValuesToJson()
	{
		if (this->FilePath == nullptr || this->FilePath.isEmpty())
		{
			return;
		}

		QFileInfo qtFilePath(this->FilePath);
		QFileInfo paramsPath = qtFilePath.path() + "/" + qtFilePath.baseName() + "/parameters.json";
		MString path = MQtUtil::toMString(paramsPath.absoluteFilePath());

		std::ofstream file(path.asChar());
		file << WStringToString(SerializeContents()->Stringify(true));
		file.close();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void GlobalParameters::SetDefaultValues()
	{
		this->Depth = 0;
		this->Scale = 1;
		this->KeepGroupStructure = true;
		this->AliasPsdName = "";

		this->ClearLayerParameters();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void GlobalParameters::DeserializeContents(JSONObject & root)
	{
		this->Depth = root[L"Depth"]->AsNumber();
		this->Scale = root[L"Scale"]->AsNumber();
		this->AliasPsdName = MQtUtil::toQString(root[L"AliasPsdName"]->AsString().c_str());
		this->KeepGroupStructure = root[L"KeepGroupStructure"]->AsBool();

		this->ClearLayerParameters();
		JSONArray layers = root[L"Layers"]->AsArray();
		for (auto layer : layers)
		{
			JSONObject layerObject = layer->AsObject();

			LayerParameters* layerParams = new LayerParameters;
			layerParams->Algo = LayerParameters::Algorithm(int(layerObject[L"Algo"]->AsNumber()));
			layerParams->InfluenceActivated = layerObject[L"InfluenceActivated"]->AsBool();

			JSONObject linearParams = layerObject[L"LinearParameters"]->AsObject();
			layerParams->LinearParameters.LinearHeightPoly = linearParams[L"LinearHeightPoly"]->AsNumber();
			layerParams->LinearParameters.GridOrientation = linearParams[L"GridOrientation"]->AsNumber();

			JSONObject curveParams = layerObject[L"CurveParameters"]->AsObject();
			layerParams->CurveParameters.MergeVertexDistance = curveParams[L"MergeVertexDistance"]->AsNumber();

			JSONObject influenceParams = layerObject[L"InfluenceParameters"]->AsObject();
			layerParams->InfluenceParameters.MinPolygonSize = influenceParams[L"MinPolygonSize"]->AsNumber();
			layerParams->InfluenceParameters.MaxPolygonSize = influenceParams[L"MaxPolygonSize"]->AsNumber();

			this->NameLayerMap.try_emplace(WStringToString(layerObject[L"Name"]->AsString()), layerParams);
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	JSONValue* GlobalParameters::SerializeContents()
	{
		JSONObject root;

		root[L"Depth"] = new JSONValue(this->Depth);
		root[L"Scale"] = new JSONValue(this->Scale);
		root[L"AliasPsdName"] = new JSONValue(StringToWString(MQtUtil::toMString(this->AliasPsdName).asChar()));
		root[L"KeepGroupStructure"] = new JSONValue(this->KeepGroupStructure);

		JSONArray layers;
		for (auto pair : this->NameLayerMap)
		{
			JSONObject layerObject;

			layerObject[L"Name"] = new JSONValue(StringToWString(pair.first));
			layerObject[L"Algo"] = new JSONValue(pair.second->Algo);
			layerObject[L"InfluenceActivated"] = new JSONValue(pair.second->InfluenceActivated);

			JSONObject linearParams;
			linearParams[L"LinearHeightPoly"] = new JSONValue(pair.second->LinearParameters.LinearHeightPoly);
			linearParams[L"GridOrientation"] = new JSONValue(pair.second->LinearParameters.GridOrientation);

			JSONObject curveParams;
			curveParams[L"MergeVertexDistance"] = new JSONValue(pair.second->CurveParameters.MergeVertexDistance);

			JSONObject influenceParams;
			influenceParams[L"MinPolygonSize"] = new JSONValue(pair.second->InfluenceParameters.MinPolygonSize);
			influenceParams[L"MaxPolygonSize"] = new JSONValue(pair.second->InfluenceParameters.MaxPolygonSize);

			layerObject[L"LinearParameters"] = new JSONValue(linearParams);
			layerObject[L"CurveParameters"] = new JSONValue(curveParams);
			layerObject[L"InfluenceParameters"] = new JSONValue(influenceParams);

			layers.push_back(new JSONValue(layerObject));
		}

		root[L"Layers"] = new JSONValue(layers);

		return new JSONValue(root);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	std::wstring GlobalParameters::StringToWString(const std::string& str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(str);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	std::string GlobalParameters::WStringToString(const std::wstring& str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(str);
	}

#pragma endregion
}
