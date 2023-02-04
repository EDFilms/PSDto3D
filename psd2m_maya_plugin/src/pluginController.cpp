//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file pluginController.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include <pluginController.h>
#include <psd_reader/psdReader.h>
#include <QFileInfo>

#include "texture_exporter/textureExporter.h"
#include "texture_exporter/lodepng.h"
#include <direct.h>
#include "maya_mesh/meshGeneratorController.h"

namespace maya_plugin
{
	//--------------------------------------------------------------------------------------------------------------------------------------
	PluginController::PluginController(ToolWidget* guiPsdMaya)
	{
		this->GuiPsdMaya = guiPsdMaya;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	PluginController::~PluginController(){	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::Update()
	{
		// Extrat the data from the view
		GlobalParameters tmp = this->GuiPsdMaya->GetParameters();

		if (tmp.SelectionChanged && !tmp.FilePath.isEmpty())
		{
			const MString path = MQtUtil::toMString(tmp.FilePath);
		
			ParsePsdData(path);
		}

		// Export PNG
		QFileInfo qtFilePath(tmp.FilePath);
		const MString completePath = MQtUtil::toMString(qtFilePath.path()) + "/" + MQtUtil::toMString(qtFilePath.baseName());
		if (tmp.Exportation && !tmp.Exported)
		{
			ExportTexture(completePath);
		}

		// Generate mesh
		if (tmp.Generate && !tmp.Generated)
		{
			GenerateMesh(tmp);
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::ParsePsdData(MString const& path)
	{
		Progress progress = this->GuiPsdMaya->GetProgress();

		std::function<void(unsigned)> initialize = [&progress](unsigned n) { progress.InitializeProgressBar(n); };
		std::function<void(unsigned)> subInitialize = [&progress](unsigned n) { progress.InitializeSubProgress(n); };
		std::function<void()> increment = [&progress]() { progress.IncrementProgressBar(); };
		std::function<void()> completeSub = [&progress]() { progress.CompleteSubProgress(); };

		PsdReader reader(path.asChar());
		reader.SetProgress(initialize, subInitialize, increment, completeSub);

		// Create folder before the Parse starts
		QFileInfo filePath(path.asChar());
		QFileInfo newFolder = filePath.path() + "/" + filePath.baseName();
		if (!newFolder.exists())
			_mkdir(MQtUtil::toMString(newFolder.absoluteFilePath()).asChar());

		this->PsdData = reader.ParsePsd();
		this->GuiPsdMaya->SetPsdData(PsdData);

		progress.CompleteProgressBar();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::ExportTexture(MString const& path) const
	{
		std::vector<unsigned char> tmptexture;
		this->GuiPsdMaya->GetProgress().InitializeProgressBar(PsdData.LayerMaskData.LayerCount);
		for (int i = 0; i < PsdData.LayerMaskData.LayerCount; i++)
		{
			if (PsdData.LayerMaskData.Layers[i].Type >= ANY_OTHER_TYPE_LAYER) continue;

			LayerParameters* layerParams = this->GuiPsdMaya->GetParameters().GetLayerParameter(PsdData.LayerMaskData.Layers[i].LayerName);
			if (!layerParams->IsActive)
				continue;

			tmptexture.clear();
			tmptexture = TextureExporter::ConvertIffFormat(false, PsdData.LayerMaskData.Layers[i], PsdData.HeaderData.Width, PsdData.HeaderData.Height, PsdData.HeaderData.BitsPerPixel);
			if (tmptexture.empty()) continue;
			unsigned char* pngData;
			size_t pngsize;

			const unsigned error = lodepng_encode32(&pngData, &pngsize, tmptexture.data(), PsdData.HeaderData.Width, PsdData.HeaderData.Height, PsdData.HeaderData.BitsPerPixel);

			if (error)
			{
				// display error.
				continue;
			}

			MString pngNameFile = MString(path + "/" + PsdData.LayerMaskData.Layers[i].LayerName.c_str() + ".png");
			lodepng_save_file(pngData, pngsize, pngNameFile.asChar());

			this->GuiPsdMaya->GetProgress().IncrementProgressBar();
		}

		this->GuiPsdMaya->GetProgress().CompleteProgressBar();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::GenerateMesh(GlobalParameters & params)
	{
		MeshGeneratorController::GenerateMayaMeshes(this->PsdData, params, this->GuiPsdMaya->GetProgress());
	}
}
