//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file unrealMain.h
//  @author Michaelson Britt
//  @date 06-25-2020
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#pragma once

#include "unrealEditorPluginFlags.h"

#include "IPluginOutput.h"

#include "CoreMinimal.h" // common
#include "Modules/ModuleManager.h" // common

// PsdToUnrealPluginOutput
class UMaterial; // forward declaration

class PsdToUnrealPluginOutput : public psd_to_3d::IPluginOutput
{
public:
	//static bool outputTextureOpDone; // TODO: Add threadsafety

	struct OutputLayersTask
	{
		OutputLayersTask(const psd_to_3d::GroupByNameMap& tree, int& running )
		: tree(tree), running(running) {}

		const psd_to_3d::GroupByNameMap& tree;
		const char* psdFileName;
		int psdSceneWidth, psdSceneHeight;
		int& running;
	};

	struct OutputLayerTask
	{
		OutputLayerTask( const psd_to_3d::GraphLayer& graphLayer )
		: graphLayer(graphLayer) {}

		const psd_to_3d::GraphLayer& graphLayer;
		const char* psdFileName;
		int psdSceneWidth, psdSceneHeight;
		UMaterial* material;
	};

	//static void OutputTextureOp( const OutputTextureDesc textureDesc );
	static void OutputLayers( OutputLayersTask task );
	static void OutputLayer( OutputLayerTask task );
	static UMaterial* CreateMaterial( const char* textureFilepath, const char* assetName, const char* packageName );
	
	virtual void BeginSession( const IPluginOutputParameters& params ) override;
	virtual void OutputMesh( const PsdData& psdData, const IPluginOutputParameters& params, const DataSurface& dataSurface, int layerIndex ) override;
	virtual void OutputTree( const PsdData& psdData, const IPluginOutputParameters& params, const GroupByNameMap& tree ) override;
	virtual void EndSession( const PsdData& psdData, const IPluginOutputParameters& params ) override;
	virtual void CancelSession( const PsdData& psdData, const IPluginOutputParameters& params ) override;
	virtual void GetSaveDialogParams( void* ofn, const IPluginOutputParameters& params );
protected:
	char session_psdFileName[1024];
};
//bool PsdToUnrealPluginOutput::outputTextureOpDone;
