// ===============================================
//  Copyright (C) 2024, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PSDtoUnrealOutput.h
//  @author Michaelson Britt
//  @date 2024-10-16
//
//  @section DESCRIPTION
//  Main PSDtoUnreal module
//  Interfaces with the PSDto3D core and implements IPluginOutput
//  Translates meshes and textures from PSDto3D into Unreal entities
//
//----------------------------------------------------------------------------------------------


#pragma once

#include "IPluginOutput.h"

#include <set>
#include <string>

// PsdToUnrealPluginOutput
class UMaterialInterface; // forward declaration
class UMaterial; // forward declaration
class UTexture2D; // forward declaration

class PsdToUnrealPluginOutput : public psd_to_3d::IPluginOutput
{
public:
	//static bool outputTextureOpDone; // TODO: Add threadsafety

	PsdToUnrealPluginOutput() : hModule(nullptr) {};

	static PsdToUnrealPluginOutput& GetInstance();
	void OpenDialog( void* hLibrary ); // HMODULE hLibrary

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

	struct TextureSet
	{
		TextureSet()
		: diffuseTexture(nullptr), normalMapTexture(nullptr), heightMapTexture(nullptr),
		  roughnessTexture(nullptr), occlusionTexture(nullptr),
		  detailColorTexture(nullptr), detailNormalMapTexture(nullptr), detailHeightMapTexture(nullptr),
		  detailRoughnessTexture(nullptr), detailOcclusionTexture(nullptr),
		  wpoWindTexture(nullptr), wpoNoiseTexture(nullptr) {}

		// this could be an array for compactness, but it's more readable this way
		UTexture2D* diffuseTexture;
		UTexture2D* normalMapTexture;
		UTexture2D* heightMapTexture;
		UTexture2D* roughnessTexture;
		UTexture2D* occlusionTexture;
		UTexture2D* detailColorTexture;
		UTexture2D* detailNormalMapTexture;
		UTexture2D* detailHeightMapTexture;
		UTexture2D* detailRoughnessTexture;
		UTexture2D* detailOcclusionTexture;
		UTexture2D* wpoWindTexture;
		UTexture2D* wpoNoiseTexture;
	};


	static void OutputLayers( OutputLayersTask task );
	static void OutputLayer( OutputLayerTask task );
	static void OutputTextures( OutputTexturesTask task );

	static TextureSet ObtainTextureSet( const psd_to_3d::GraphLayer& graphLayer, const char* assetName, const char* packageName );
	static UTexture2D* ObtainTexture( const char* textureFilepath, const char* assetName, const char* packageName );

	static bool CheckMaterial( const char* assetName, const char* packageName ); // true if already created in the scene
	static UMaterialInterface* ObtainCustomMaterial( const char* masterAssetPath, const char* assetName, const char* packageName );
	static void SetupCustomMaterial( UMaterialInterface* material, TextureSet* textureSet );
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
	void* hModule; //HMODULE
	char session_psdFileName[1024];
	int session_psdSceneWidth, session_psdSceneHeight;
	int session_pivotPosition;
	std::string session_packageName;
	std::set<std::string> session_texturesOutput;  // asset names of textures output to disk during this session
	std::set<std::string> session_texturesCreated; // asset names of textures created as new scene entities during this session

	const char* GetPackageName() { return session_packageName.c_str(); }
};
