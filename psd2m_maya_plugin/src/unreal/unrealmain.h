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

#include <set>

// PsdToUnrealPluginOutput
class UMaterialInterface; // forward declaration
class UMaterial; // forward declaration
class UTexture2D; // forward declaration

class PsdToUnrealPluginOutput : public psd_to_3d::IPluginOutput
{
public:
	//static bool outputTextureOpDone; // TODO: Add threadsafety

	struct OutputLayersTask
	{
		OutputLayersTask( PsdToUnrealPluginOutput* parent, const psd_to_3d::GroupByNameMap& tree, int& running )
		: parent(parent), tree(tree), running(running) {}

		PsdToUnrealPluginOutput* parent;
		const psd_to_3d::GroupByNameMap& tree;
		int& running;
	};

	struct OutputLayerTask
	{
		OutputLayerTask( PsdToUnrealPluginOutput* parent, const psd_to_3d::GraphLayer& graphLayer )
		: parent(parent), graphLayer(graphLayer) {}

		PsdToUnrealPluginOutput* parent;
		const psd_to_3d::GraphLayer& graphLayer;
		UMaterialInterface* material;
	};

	struct OutputTexturesTask
	{
		OutputTexturesTask( PsdToUnrealPluginOutput* parent, int& running )
		: parent(parent), running(running) {}

		PsdToUnrealPluginOutput* parent;
		int& running;
	};

	struct OutputTextureTask
	{
		OutputTextureTask( PsdToUnrealPluginOutput* parent, const char* textureFilepath, const char* textureName )
		: parent(parent), textureFilepath(textureFilepath), textureName(textureName) {}

		bool operator<( const OutputTextureTask& that ) { return (this->textureFilepath<that.textureFilepath); }
		PsdToUnrealPluginOutput* parent;
		std::string textureFilepath;
		std::string textureName;
	};



	static void OutputLayers( OutputLayersTask task );
	static void OutputLayer( OutputLayerTask task );
	static void OutputTextures( OutputTexturesTask task );

	static UTexture2D* ObtainTexture( const char* textureFilepath, const char* assetName, const char* packageName );

	static bool CheckMaterial( const char* assetName, const char* packageName ); // true if already created in the scene
	static UMaterialInterface* ObtainCustomMaterial( const char* masterAssetPath, const char* assetName, const char* packageName );
	static void SetupCustomMaterial( UMaterialInterface* material, UTexture2D* diffuseTexture );
	static UMaterial* ObtainStandardMaterial( const char* assetName, const char* packageName );
	static void SetupStandardMaterial( UMaterial* material, UTexture2D* diffuseTexture );
	
	virtual void BeginSession(  const PsdData& psdData, const IPluginOutputParameters& params ) override;
	virtual void OutputMesh(    const PsdData& psdData, const IPluginOutputParameters& params, const DataSurface& dataSurface, int layerIndex ) override;
	virtual void OutputTree(    const PsdData& psdData, const IPluginOutputParameters& params, const GroupByNameMap& tree ) override;
	virtual void OutputTexture( const PsdData& psdData, const IPluginOutputParameters& params, const char* textureFilepath, const char* textureName ) override;
	virtual void EndSession(    const PsdData& psdData, const IPluginOutputParameters& params ) override;
	virtual void CancelSession( const PsdData& psdData, const IPluginOutputParameters& params ) override;
	virtual void GetSaveDialogParams( void* ofn, const IPluginOutputParameters& params );
protected:
	char session_psdFileName[1024];
	int session_psdSceneWidth, session_psdSceneHeight;
	std::string session_packageName;
	std::set<std::string> session_texturesOutput;  // asset names of textures output to disk during this session
	std::set<std::string> session_texturesCreated; // asset names of textures created as new scene entities during this session

	const char* GetPackageName() { return session_packageName.c_str(); }
};
