// ===============================================
//  Copyright (C) 2024, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PSDtoUnrealOutput.cpp
//  @author Michaelson Britt
//  @date 2024-10-16
//
//  @section DESCRIPTION
//  Main PSDtoUnreal module
//  Interfaces with the PSDto3D core and implements IPluginOutput
//  Translates meshes and textures from PSDto3D into Unreal entities
//
//----------------------------------------------------------------------------------------------

#include "PSDtoUnrealOutput.h"

#include "Windows/WindowsPlatformAtomics.h"
#include "PackageTools.h"
#include "EditorReimportHandler.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "StaticMeshAttributes.h"
#include "Engine/StaticMeshActor.h"
#include "Factories/MaterialFactoryNew.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "ObjectTools.h"
#include "Utils.h"

// Local includes
#include "psd_reader/psdReader.h"

// Includes conflicting with Unreal includes, must be included last
#include <Windows.h> // for HMODULE

// Helper
inline void DebugPrint(const TCHAR* format, ...)
{
	static const int bufSize = 8192;
	static TCHAR buf[bufSize + 1];
	va_list ap;
	va_start(ap, format);
	_vstprintf_s(buf, bufSize, format, ap);
	OutputDebugString(buf);
	_tprintf(buf);
	va_end(ap);
}

class SanitizeUTF8
{
public:
	FTCHARToUTF8 name;
	SanitizeUTF8( const char* nameRaw )
	: name( *ObjectTools::SanitizeObjectName(nameRaw) )
	{}
	operator const char* () { return name.Get(); }
};



typedef void (*t_vfni)( int );
typedef void (*t_vfnvp)( void* );

typedef PsdToUnrealPluginOutput::TextureSet TextureSet;

PsdToUnrealPluginOutput& PsdToUnrealPluginOutput::GetInstance()
{
	static PsdToUnrealPluginOutput instance;
	return instance;
}

void PsdToUnrealPluginOutput::OpenDialog( void* hModule_in )
{

	if( hModule_in!=NULL )
	{
		this->hModule = hModule_in;

		FARPROC lpfProcFunc = NULL;
		lpfProcFunc = GetProcAddress( (HMODULE)hModule, "setPluginOutput" );
		if(lpfProcFunc!=NULL)
		{
			void* temp = lpfProcFunc; // to avoid compiler warning about function pointer type conversion
			t_vfnvp fn_setPluginOutput = (t_vfnvp)temp;
			fn_setPluginOutput(this);
		}

		lpfProcFunc = GetProcAddress( (HMODULE)hModule, "openPlugin" );
		if(lpfProcFunc!=NULL)
		{
			void* temp = lpfProcFunc; // to avoid compiler warning about function pointer type conversion
			t_vfni fn_openPlugin = (t_vfni)temp;
			fn_openPlugin(0);
		}
	}
}


void PsdToUnrealPluginOutput::OutputLayers( OutputLayersTask task )
{
	std::map< std::string, UMaterialInterface* > materialLookup;

	const char* packageName = task.parent->GetPackageName();
	// System seems to prefer if the "Content" folder name is omitted, but actual file path is:
	// IPluginManager::Get().FindPlugin("PsdToUnreal")->GetBaseDir() + /PSDtoUnreal/Content/Assets/Materials/PsdMaster/M_PsdMaster.uasset
	const char* masterAssetPath = "/PSDtoUnreal/Assets/Materials/PsdMaster/M_PsdMaster.M_PsdMaster";

	// Iterate through all groups in the Photoshop file ...
	psd_to_3d::GroupByNameMap::const_iterator graphIter;
	for( graphIter = task.tree.begin(); graphIter!=task.tree.end(); graphIter++ )
	{
		// Iterate through all layers in the current group ...
		const psd_to_3d::GraphLayerGroup& graphLayerGroup = graphIter->second;

		for( int graphLayerIndex = 0; graphLayerIndex<graphLayerGroup.GetLayerCount(); graphLayerIndex++ )
		{
			const psd_to_3d::GraphLayer* graphLayerPtr = graphLayerGroup[graphLayerIndex];
			if( graphLayerPtr==nullptr ) continue;

			const psd_to_3d::GraphLayer& graphLayer = *graphLayerPtr;
			const char* textureFilepath = graphLayer.TextureFilepath.c_str();
			// Unreal requires spaces are converted to underscores in asset names
			SanitizeUTF8 assetName( graphLayer.TextureName.c_str() );

			bool isNewMaterial = !(CheckMaterial( assetName, packageName )); // true if material does not already exist in scene

			// find or create the material
			UMaterialInterface* material = ObtainCustomMaterial( masterAssetPath, assetName, packageName );

			TextureSet textureSet = ObtainTextureSet( graphLayer, assetName, packageName );
			SetupCustomMaterial( material, &textureSet );

			if( isNewMaterial )
			{
				task.parent->session_texturesCreated.insert( (const char*)assetName );
			}

			materialLookup[ graphLayer.TextureFilepath ] = material;

			OutputLayerTask subtask( task.parent, graphLayer );
			subtask.material = materialLookup[ graphLayer.TextureFilepath ]; // lookup material
			OutputLayer( subtask );
		}
	}

	task.running = 0; // allow UI thread to continue;
}


//
// See \Engine\Plugins\Experimental\AlembicImporter\Source\AlembicLibrary\Private\AbcImporter.cpp  (Unreal 4)
// or  \Engine\Plugins\Importers\AlembicImporter\Source\AlembicLibrary\Private  (Unreal 5)
// for example code
//

void PsdToUnrealPluginOutput::OutputLayer( OutputLayerTask task )
{
	// TODO: Use UTextureFactory::CreateTexturee
	// TODO: Use Life++ instead of Edit and Continue
	// TODO: Understand how to add code into project and use the standard publishing technique

	mesh_generator::DataMesh* dataMesh = task.graphLayer.Mesh.GetDataMesh();
	if( dataMesh==nullptr )
		return; // need a DataMesh to proceed, no support for DataSpline

	// Unreal requires spaces are converted to underscores in asset names
	FString layerName = ObjectTools::SanitizeObjectName( FString( task.graphLayer.LayerName.c_str() ) );
	FString basePackageName = FString( task.parent->GetPackageName() );

	util::boundsUV layerRegion = task.graphLayer.LayerRegion; // relative to scene, range [0-1]
	int layerIndex = task.graphLayer.LayerIndex;
	float layerScale = task.graphLayer.Scale;
	float layerDepth = task.graphLayer.Depth;

	float aspectRatio = (float)task.parent->session_psdSceneWidth / (float)task.parent->session_psdSceneHeight;
	float layoutWidth = 1000.0f;
	float layoutHeight = 1000.0f / aspectRatio;

	float layerPosX = ((layerRegion.TopLeftPoint().x - 0.5f) * layerScale * layoutWidth);
	float layerPosY = ( layerRegion.TopLeftPoint().y * layerScale * layoutHeight) + 20.0f;
	float layerPosZ = (layerDepth * layerIndex * -1.0f);

	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Create mesh asset

	FString meshAssetName = layerName + FString("_mesh");
	FString meshPackageName = basePackageName + meshAssetName;
	meshPackageName = UPackageTools::SanitizePackageName(meshPackageName);
	UPackage* meshPackage = CreatePackage(*meshPackageName); // v4.26.2

	EObjectFlags meshObjectFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
	UStaticMesh*  mesh   = NewObject<UStaticMesh>(meshPackage, *FPaths::GetBaseFilename(meshPackageName), meshObjectFlags); // 
	FAssetRegistryModule::AssetCreated(mesh);
	meshPackage->FullyLoad();
	meshPackage->Modify();

	FStaticMeshSourceModel& meshSourceModel = mesh->AddSourceModel();

	// TODO: Could also use AddSourceModel() instead of CreateMeshDescription()/CommitMeshDescription()
	const int32 meshLODIndex = 0;
	mesh->SetNumSourceModels(1); // Number of LOD levels.  Not really necessary if only 1 level, but...
	FMeshDescription* meshDescription = mesh->CreateMeshDescription(meshLODIndex);

	// Mesh data from PSDto3D
	util::xformUV xformuv = task.graphLayer.XFormUV;
	const std::vector<Vector2F>& verts = dataMesh->GetVertices(); // position of each vert
	const std::vector<int>& faceSizes = dataMesh->GetFaceSizes(); // number of verts per face
	const std::vector<int>& faceVerts = dataMesh->GetFaceVerts();	// vertex indices, contiguous list
	int faceCount = dataMesh->GetFacesCount();
	int vertCount = dataMesh->GetVerticesCount();


	// Mesh data for Unreal
	FStaticMeshAttributes meshAttributes(*meshDescription);
	TArray<FVertexInstanceID> meshVertexInstanceIDs;
	TVertexAttributesRef<FVector3f> meshVertexPositions = meshAttributes.GetVertexPositions();
	TVertexInstanceAttributesRef<FVector2f> meshVertexUVs = meshAttributes.GetVertexInstanceUVs();
	TArray<FVertexID> vertexID;
	TArray<FVertexInstanceID> vertexInstanceID;
	int meshUVLayerIndex = 0;


	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Set vertex data
	meshVertexPositions.SetNumChannels( vertCount );
	meshVertexUVs.SetNumChannels( 1 ); // is this the attribute channel count, rather than vertCount ?
	vertexID.SetNum( vertCount );
	vertexInstanceID.SetNum( vertCount );
	for( int i=0; i<vertCount; i++ )
	{
		FVertexID vid = meshDescription->CreateVertex();
		FVertexInstanceID viid = meshDescription->CreateVertexInstance( vid );
		util::Vector2F uv = xformuv.Transform( verts[i] );

		vertexID[ i ] = vid;
		vertexInstanceID[ i ] = viid;

		meshVertexPositions[ vid ] = FVector3f( 0, verts[i].x * layoutWidth, (1.0f-verts[i].y) * layoutHeight );
		meshVertexUVs.Set( viid, meshUVLayerIndex, FVector2f(uv.x,uv.y) );
	}


	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Set face data
	meshDescription->ReserveNewPolygons( faceCount );
	const FPolygonGroupID& meshPolygonGroupID = meshDescription->CreatePolygonGroup();
	int faceVertsIndex = 0;
	for( int i=0; i<faceCount; i++ )
	{
		meshVertexInstanceIDs.SetNum( faceSizes[i] );
		for( int j=0; j<faceSizes[i]; j++, faceVertsIndex++ )
		{
			int vertIndex = faceVerts[ faceVertsIndex ];
			meshVertexInstanceIDs[j] = vertexInstanceID[ vertIndex ];
		}

		meshDescription->CreatePolygon( meshPolygonGroupID, meshVertexInstanceIDs );
	}


	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Configure mesh parameters

	const bool bMeshHasTangents = false; // TODO: Implement this
	const int32 meshPrimCount = 0; // TODO: Implement this
	const int32 meshMaterialIndex = 0; // TODO: implement this
	const FName meshMaterialSlotName( *FString::FromInt(meshMaterialIndex) );

	const int32 meshSlot = mesh->GetStaticMaterials().Emplace(nullptr, meshMaterialSlotName, meshMaterialSlotName);
	mesh->GetSectionInfoMap().Set(0, meshSlot, FMeshSectionInfo(meshSlot));

		TVertexInstanceAttributesRef<FVector2f> check1 = meshAttributes.GetVertexInstanceUVs();
		int32 check2 = check1.GetNumChannels();

	// Generate a new UV set based off the highest index UV set in the mesh
	meshSourceModel.BuildSettings.bGenerateLightmapUVs = true;
	meshSourceModel.BuildSettings.DstLightmapIndex     = vertCount;
	//meshSourceModel.BuildSettings.SrcLightmapIndex     = vertCount - 1;
	mesh->SetLightMapCoordinateIndex( 1 ); // vertCount

	FMeshBuildSettings& meshSettings = meshSourceModel.BuildSettings;
	meshSettings.bRecomputeNormals       = false;
	meshSettings.bRecomputeTangents      = !bMeshHasTangents;
	meshSettings.bUseMikkTSpace          = true;
	meshSettings.bComputeWeightedNormals = true;

	meshSettings.bRemoveDegenerates        = false;
	//meshSettings.bBuildAdjacencyBuffer     = false;
	meshSettings.bBuildReversedIndexBuffer = false;

	meshSettings.bUseHighPrecisionTangentBasis = false; // 8-bit vs 16-bit, set depending on relative size of object onscreen
	meshSettings.bUseFullPrecisionUVs          = false; // Full float vs half float, set depending on relative size of object onscreen
	mesh->CommitMeshDescription(meshLODIndex);
	mesh->MarkPackageDirty();
	mesh->PostEditChange();


	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Create actor

	UWorld* world = GEngine->GetWorldContexts()[0].World();

	FVector meshPos(layerPosZ, -(0.5f*layoutWidth), 0); //( layerPosZ, layerPosX, layerPosY );

	FRotator meshRot(0,0,0);

	FVector meshSceneScale(layerScale,layerScale,layerScale);

	FActorSpawnParameters meshParams;
	meshParams.Name = *layerName;
	meshParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested; // rename if name conflict

	AStaticMeshActor* meshActor = world->SpawnActor<AStaticMeshActor>(meshPos, meshRot, meshParams);
	if( meshActor!=nullptr )
	{
		FString folderPath = FString("/") + FString(task.parent->session_psdFileName);
		meshActor->SetFolderPath( *folderPath );
		meshActor->SetActorLabel( layerName ); // TODO: Works in development builds only?
		meshActor->GetStaticMeshComponent()->SetStaticMesh( mesh );
		meshActor->GetStaticMeshComponent()->SetMaterial( 0, task.material );
		meshActor->GetRootComponent()->SetRelativeScale3D( meshSceneScale );
		meshActor->MarkComponentsRenderStateDirty();
	}
}

void PsdToUnrealPluginOutput::OutputTextures( OutputTexturesTask task )
{
	// Reimport any textures which already existed in the scene

	// convert from std::set or texture names to TArray of texture objects,
	// also check 
	TArray<UObject*> texturesToUpdate;
	std::set<std::string>::iterator iter = task.parent->session_texturesOutput.begin();
	for( ; iter!=task.parent->session_texturesOutput.end(); iter++ )
	{
		// check if the texture was created as new scene entities during this session, if so, don't update ...
		bool wasCreated = (task.parent->session_texturesCreated.find(*iter) != task.parent->session_texturesCreated.end());
		if( !wasCreated )
		{
			const char* textureName = iter->c_str();
			const char* packageName = task.parent->GetPackageName();
			if( CheckMaterial( textureName, packageName ) ) // true if material exists in scene
			{
				texturesToUpdate.Add( task.parent->ObtainTexture( nullptr, textureName, packageName ) );
			}
		}
	}

	if( !texturesToUpdate.IsEmpty() )
	{
		// compile errors when instantiating UReimportTextureFactory, but FReimportManager seems to work
		bool showNotification = false;
		const int32 sourceFileIndex = INDEX_NONE;
		bool forceNewFile = false;
		bool automated = true;
		FReimportManager::Instance()->ValidateAllSourceFileAndReimport( texturesToUpdate, showNotification, sourceFileIndex, forceNewFile, automated );
	}

	task.running = 0; // allow UI thread to continue;
}

TextureSet PsdToUnrealPluginOutput::ObtainTextureSet( const psd_to_3d::GraphLayer& graphLayer, const char* assetName, const char* packageName )
{
	TextureSet retval;

	const std::vector<std::string>& compLayerFilepaths = graphLayer.CompLayerFilepaths;
	int count = compLayerFilepaths.size();

	for( int compLayerType=0; (compLayerType<psd_to_3d::COMP_LAYER_COUNT) && (compLayerType<count); compLayerType++ )
	{
		const std::string& filepath = compLayerFilepaths[compLayerType];
		UTexture2D* texture = nullptr;

		if( filepath.size() > 0 )
		{
			FString postfix = FString("_") + FString( psd_to_3d::GetCompLayerTag(compLayerType).c_str() );
			FString compAssetName = FString(assetName) + postfix;
			texture = ObtainTexture( filepath.data(), TCHAR_TO_ANSI(*compAssetName), packageName );
		}
		
		switch( compLayerType )
		{
		case psd_to_3d::NORMALMAP_LAYER:		retval.normalMapTexture = texture;			break;
		case psd_to_3d::HEIGHTMAP_LAYER:		retval.heightMapTexture = texture;			break;
		case psd_to_3d::ROUGHNESS_LAYER:		retval.roughnessTexture = texture;			break;
		case psd_to_3d::OCCLUSION_LAYER:		retval.occlusionTexture = texture;			break;
		case psd_to_3d::DETAILCOLOR_LAYER:		retval.detailColorTexture = texture;		break;
		case psd_to_3d::DETAILNORMALMAP_LAYER:	retval.detailNormalMapTexture = texture;	break;
		case psd_to_3d::DETAILHEIGHTMAP_LAYER:	retval.detailHeightMapTexture = texture;	break;
		case psd_to_3d::DETAILROUGHNESS_LAYER:	retval.detailRoughnessTexture = texture;	break;
		case psd_to_3d::DETAILOCCLUSION_LAYER:	retval.detailOcclusionTexture = texture;	break;
		case psd_to_3d::WPOWIND_LAYER:			retval.wpoWindTexture = texture;			break;
		case psd_to_3d::WPONOISE_LAYER:			retval.wpoNoiseTexture = texture;			break;
		}
	}

	// base color
	if( FPaths::FileExists(graphLayer.TextureFilepath.data()) )
		retval.diffuseTexture = ObtainTexture( graphLayer.TextureFilepath.data(), assetName, packageName );

	return retval;
}

UTexture2D* PsdToUnrealPluginOutput::ObtainTexture( const char* textureFilepath, const char* assetName, const char* packageName )
{
	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Create or find texture package

	FString textureAssetPath = FString(textureFilepath);
	FString textureAssetName = FString(assetName) + FString("_png");
	FString texturePackageName = FString(packageName) + FString(textureAssetName); // assumes packageName ends in '/'
	texturePackageName = UPackageTools::SanitizePackageName(texturePackageName);

	UPackage* texturePackage = CreatePackage(*texturePackageName); // v4.26.2
	//UPackage* texturePackage = CreatePackage(nullptr,*texturePackageName); // v4.24
	texturePackage->FullyLoad();

	// Check if package was pre-existing and already has a texture
	UObject* asset = texturePackage->FindAssetInPackage();
	UTexture2D* texture = nullptr;
	if( asset!=nullptr )
	{
		texture = CastChecked<UTexture2D>(asset);
		texturePackage->SetDirtyFlag(true);
	}
	else if( textureFilepath!=nullptr )
	{
		//-------------------- ------------------------------------------------------------------------------------------------------------------
		// Create texture asset

		texturePackage->Modify();

		texture = ImportObject<UTexture2D>(texturePackage, *textureAssetName, RF_Public, *textureAssetPath, nullptr, nullptr, TEXT("NOMIPMAPS=0 NOCOMPRESSION=1"));

		texture->PostEditChange();
		texture->MarkPackageDirty();

		try
		{
			FSavePackageArgs args(
				nullptr, // const ITargetPlatform* InTargetPlatform
				nullptr, // FArchiveCookData* InArchiveCookData (?)
				RF_Public | RF_Standalone, // EObjectFlags InTopLevelFlags
				SAVE_None, // uint32 InSaveFlags
				false, // bool bInForceByteSwapping
				true, // bool bInWarnOfLongFilename
				true, // bool bInSlowTask (?)
				FDateTime::MinValue(), // FDateTime InFinalTimeStamp
				GError, // FOutputDevice* InError
				nullptr // FSavePackageContext* InSavePackageContext = nullptr
			);
			if( GEditor->SavePackage(texturePackage, texture, *texturePackageName, args) )
			{
				FAssetRegistryModule::AssetCreated(texture);
				texturePackage->FullyLoad();
				texturePackage->SetDirtyFlag(true);
				texturePackage->PostEditChange();
			}
		}
		catch (...)
		{
			UE_LOG(LogTemp, Error, TEXT("Error saving %s"), *texturePackageName);
		}

	}

	// TODO: See BaseDeviceProfiles.ini, to control the texture detauls->Level of Detail->Texture Group settings
	// For example, see \Projects\SunTemple\Config\DefaultDeviceProfiles.ini to control per-platform detail settings
	// Note that MIP generation may produce non-desirable results in the Alpha channel, controlled on a per-texture basis,
	// not controlled through ini file. See texture details->Level of Detail->Mip Gen Settings

	return texture;
}

bool PsdToUnrealPluginOutput::CheckMaterial( const char* assetName, const char* packageName )
{
	FString materialAssetName = FString(assetName) + FString("_material");
	FString materialPackageName = FString(packageName) + FString(materialAssetName);
	materialPackageName = UPackageTools::SanitizePackageName(materialPackageName);

	return (FindPackage(nullptr,*materialPackageName) != nullptr );
}


UMaterialInterface* PsdToUnrealPluginOutput::ObtainCustomMaterial( const char* masterAssetPath, const char* assetName, const char* packageName )
{
	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Create or find material package

	FString materialAssetName = FString(assetName) + FString("_material");
	FString materialPackageName = FString(packageName) + FString(materialAssetName);
	materialPackageName = UPackageTools::SanitizePackageName(materialPackageName);

	UPackage* materialPackage = CreatePackage(*materialPackageName);
	materialPackage->FullyLoad();

	// Check if package was pre-existing and already has a texture
	UObject* asset = materialPackage->FindAssetInPackage();
	UMaterialInterface* materialInstance = nullptr;
	if( asset!=nullptr )
	{
		materialInstance = CastChecked<UMaterialInterface>(asset);
		materialPackage->SetDirtyFlag(true);
	}
	else
	{
		//-------------------- ------------------------------------------------------------------------------------------------------------------
		// Create material asset

		materialPackage->Modify();

		FString path = masterAssetPath;
		UClass* Class = UMaterialInterface::StaticClass();
		Class->GetDefaultObject(); // force the CDO to be created if it hasn't already
		UMaterialInterface* materialMaster = LoadObject<UMaterialInterface>(NULL, *path, nullptr, LOAD_None);
		if( (materialMaster!=nullptr) && (materialMaster->GetMaterial()!=nullptr) )
		{
			// Load necessary modules
			FAssetToolsModule& AssetToolsModule = 
				FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");

			UMaterialInstanceConstantFactoryNew* Factory =
				NewObject<UMaterialInstanceConstantFactoryNew>();
			Factory->InitialParent = materialMaster;

			UMaterialInstanceConstant* materialInstanceConstant = 
			  CastChecked<UMaterialInstanceConstant>(
				AssetToolsModule.Get().CreateAsset( materialAssetName, 
					FPackageName::GetLongPackagePath(packageName),
					UMaterialInstanceConstant::StaticClass(), 
					Factory) );
			materialInstance = materialInstanceConstant;

			FAssetRegistryModule::AssetCreated(materialInstance);

			materialPackage->FullyLoad();
			materialPackage->SetDirtyFlag(true);
			materialPackage->PostEditChange();
		}
	}

	return materialInstance;
}


void PsdToUnrealPluginOutput::SetupCustomMaterial( UMaterialInterface* material, TextureSet* textureSet )
{
	UMaterialInstanceConstant* materialInstanceConstant = 
		CastChecked<UMaterialInstanceConstant>(material);

	FStaticParameterSet staticParams;
	materialInstanceConstant->GetStaticParameterValues(staticParams);

	if( textureSet->diffuseTexture!=nullptr )
	{
		for( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseBaseColorMap"))) )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("BaseColor")), textureSet->diffuseTexture);
	}

	if( textureSet->normalMapTexture!=nullptr )
	{
		for( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseNormalMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("Normal")), textureSet->normalMapTexture);
	}

	if( textureSet->heightMapTexture!=nullptr )
	{
		for( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseHeightMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("Height")), textureSet->heightMapTexture);
	}

	if( textureSet->roughnessTexture!=nullptr )
	{
		for ( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseRoughness"))) ||
				(StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseRoughnessMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("RoughnessMap")), textureSet->roughnessTexture);
	}

	if( textureSet->occlusionTexture!=nullptr )
	{
		for( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseOcclusion"))) ||
				(StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseOcclusionMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("OcclusionMap")), textureSet->occlusionTexture);
	}

	if( textureSet->detailColorTexture!=nullptr )
	{
		for( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseDetailMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("DetailBaseColor")), textureSet->detailColorTexture);
	}

	if( textureSet->detailNormalMapTexture!=nullptr )
	{
		for( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseDetailMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("DetailNormal")), textureSet->detailNormalMapTexture);
	}

	if( textureSet->detailHeightMapTexture!=nullptr )
	{
		for( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseDetailMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("DetailHeight")), textureSet->detailHeightMapTexture);
	}

	if( textureSet->detailRoughnessTexture!=nullptr )
	{
		for( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseDetailMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("DetailRoughness")), textureSet->detailRoughnessTexture);
	}

	if( textureSet->detailOcclusionTexture!=nullptr )
	{
		for( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseDetailMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("DetailOcclusion")), textureSet->detailOcclusionTexture);
	}

	if( textureSet->wpoWindTexture!=nullptr )
	{
		for ( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseWpoWind"))) ||
				(StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseWpoMask"))) ||
				(StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseWpoMaskMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("WpoMaskMap")), textureSet->wpoWindTexture);
	}

	if( textureSet->wpoNoiseTexture!=nullptr )
	{
		for( FStaticSwitchParameter& StaticSwitchParameter : staticParams.StaticSwitchParameters )
		{
			if( (StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseWpoNoise"))) ||
				(StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseWpoMask"))) ||
				(StaticSwitchParameter.ParameterInfo.Name == FName(TEXT("UseWpoMaskMap")))    )
			{
				StaticSwitchParameter.Value = true;
				StaticSwitchParameter.bOverride = true;
			}
		}
		materialInstanceConstant->SetTextureParameterValueEditorOnly(FName(TEXT("WpoMaskMap")), textureSet->wpoNoiseTexture);
	}

	materialInstanceConstant->UpdateStaticPermutation(staticParams);
	materialInstanceConstant->PostEditChange();
}


UMaterial* PsdToUnrealPluginOutput::ObtainStandardMaterial(const char* assetName, const char* packageName )
{

	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Create material

	FString materialAssetName = FString( assetName ) + FString("_material");
	FString materialPackageName = FString( packageName ) + materialAssetName;
	materialPackageName = UPackageTools::SanitizePackageName(materialPackageName);
	UPackage* materialPackage = CreatePackage(*materialPackageName); // v4.26.2
	//UPackage* materialPackage = CreatePackage(NULL, *materialPackageName); // v4.24
	materialPackage->FullyLoad();

	auto materialFactory = NewObject<UMaterialFactoryNew>();
	UMaterial* material = (UMaterial*)materialFactory->FactoryCreateNew(UMaterial::StaticClass(), materialPackage, *materialAssetName, RF_Standalone | RF_Public, NULL, GWarn);

	FAssetRegistryModule::AssetCreated(material);
	materialPackage->FullyLoad();
	materialPackage->SetDirtyFlag(true);

	//// make sure that any static meshes, etc using this material will stop using the FMaterialResource of the original
	//// material, and will use the new FMaterialResource created when we make a new UMaterial in place
	//FGlobalComponentReregisterContext RecreateComponents;

	return material;
}

void PsdToUnrealPluginOutput::SetupStandardMaterial( UMaterial* material, UTexture2D* diffuseTexture )
{
	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Create texture map connected to material

	material->PreEditChange(NULL);

	// set diffuse texture sampler
	UMaterialExpressionTextureSample* diffuseTextureSample = NewObject<UMaterialExpressionTextureSample>(material);
	diffuseTextureSample->Texture = diffuseTexture;
	diffuseTextureSample->SamplerType = SAMPLERTYPE_Color;

	UMaterialEditorOnlyData* materialData = material->GetEditorOnlyData();

	material->GetExpressionCollection().AddExpression( diffuseTextureSample );
	materialData->BaseColor.Expression = diffuseTextureSample;
	materialData->BaseColor.MaskR = 1;
	materialData->BaseColor.MaskG = 1;
	materialData->BaseColor.MaskB = 1;
	materialData->BaseColor.MaskA = 0;

	// set opacity texture sampler
	materialData->OpacityMask.Expression = diffuseTextureSample;
	materialData->OpacityMask.MaskR = 0;
	materialData->OpacityMask.MaskG = 0;
	materialData->OpacityMask.MaskB = 0;
	materialData->OpacityMask.MaskA = 1;
	materialData->OpacityMask.Mask = 1;

	// set blend mode
	material->BlendMode = BLEND_Masked;

	// Tiling
	//textureExpression->Coordinates.Expression = Multiply;

	material->PostEditChange();
}


void PsdToUnrealPluginOutput::BeginSession( const PsdData& psdData, const IPluginOutputParameters& params )
{
	int bufCount = sizeof(session_psdFileName)/sizeof(session_psdFileName[0]);
	strcpy_s( session_psdFileName, bufCount, params.PsdName() );
	session_texturesOutput.clear();
	session_texturesCreated.clear();
	session_psdSceneWidth = psdData.HeaderData.Width;
	session_psdSceneHeight = psdData.HeaderData.Height;
	session_packageName = std::string("/Game/Textures/") + std::string(session_psdFileName) + std::string("/");
}

void PsdToUnrealPluginOutput::OutputMesh( const PsdData& psdData, const IPluginOutputParameters& params, const DataSurface& dataSurface, int layerIndex )
{
	psdData; // unused
	params; // unused
	dataSurface; // unused
	//layerIndex; // unused
}

void PsdToUnrealPluginOutput::OutputTree( const PsdData& psdData, const IPluginOutputParameters& params, const GroupByNameMap& tree )
{
	int running = 1;

	OutputLayersTask task( this, tree, running );

	TUniqueFunction<void()> lambda = [task] { OutputLayers(task); };
	AsyncTask( ENamedThreads::GameThread, MoveTemp(lambda) );

	// sleep until task.running is no longer true;
	// ConditionalSleep() waits until the passed lambda "task.running==0" returns true
	FGenericPlatformProcess::ConditionalSleep(
		[&running]() // lambda function, no parameters, returns a bool
			{
				return (running==0);
			},
		0.05
	);

	; // for breakpoint
}

void PsdToUnrealPluginOutput::OutputTexture( const PsdData& psdData, const IPluginOutputParameters& params, const char* textureFilepath, const char* textureName )
{
	session_texturesOutput.insert( textureName );
}

void PsdToUnrealPluginOutput::EndSession( const PsdData& psdData, const IPluginOutputParameters& params )
{
	session_psdFileName[0] = '\0'; // reset session variables
	psdData; // unused
	params; // unused

	// if any textures output already existed in the scene (weren't just created new), then update them
	if( session_texturesOutput.size() > session_texturesCreated.size() )
	{
		int running = 1;

		OutputTexturesTask task( this, running );

		TUniqueFunction<void()> lambda = [task] { OutputTextures(task); };
		AsyncTask( ENamedThreads::GameThread, MoveTemp(lambda) );

		// sleep until task.running is no longer true;
		// ConditionalSleep() waits until the passed lambda "task.running==0" returns true
		FGenericPlatformProcess::ConditionalSleep(
			[&running]() // lambda function, no parameters, returns a bool
				{
					return (running==0);
				},
			0.05
		);
	}
}

void PsdToUnrealPluginOutput::CancelSession( const PsdData& psdData, const IPluginOutputParameters& params )
{
	session_psdFileName[0] = '\0'; // reset session variables
	psdData; // unused
	params; // unused
}


void PsdToUnrealPluginOutput::GetSaveDialogParams( void* ofnw, const IPluginOutputParameters& params )
{
	// Not implemented; Never called because PSDTO3D_FBX_VERSION is not defined
	//ofnw; // unused
	//params; // unused
}
