//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file unrealMain.cpp
//  @author Michaelson Britt
//  @date 04-01-2020
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#include "unrealMain.h"
#include "unrealEditorPluginFlags.h"
#include "psd_reader/psdReader.h"


// Header file list, 12 May 2022
// Unreal version 4.27.2

#include "CanvasTypes.h" // for FImageUtils
#include "ImageUtils.h"

#include "PackageTools.h" // for UPackageTools

#include "AssetRegistry/AssetRegistryModule.h" // for FAssetRegistryModule

#include "Editor.h" // for GEditor

#include "UObject/SavePackage.h" // for FSavePackageArgs

#include "Factories/MaterialFactoryNew.h" // for UMaterialFactoryNew

#include "Materials/Material.h" // for UMaterial

#include "StaticMeshAttributes.h" // for FStaticMeshAttributes

#include "Engine/StaticMeshActor.h" // for AStaticMeshActor

#include "Async/Async.h" // for AsyncTask

#include "Framework/MultiBox/MultiBoxBuilder.h" // for FToolBarBuilder
#include "Framework/Commands/UIAction.h"

#include "Widgets/Docking/SDockTab.h" // for ETabRole::NomadTab
#include "Framework/Docking/TabManager.h" 

#include "LevelEditor.h" // for FLevelEditorModule

#include "Widgets/DeclarativeSyntaxSupport.h" // for SCompoundWidget
#include "Widgets/SCompoundWidget.h"

#include "Modules/ModuleManager.h" // for FModuleManager

// Include after Unreal headers, because they separately declare many windows values
#include <Windows.h> // for LoadLibrary() and GetProcAddress(); how to support this cross-platform?


typedef void (*t_vfni)( int );
typedef void (*t_vfnvp)( void* );

PsdToUnrealPluginOutput pluginOutput;
HMODULE hLibrary = NULL;

void OnPluginButtonClicked()
{

	if( hLibrary==NULL )
	{
		FString modulePath = FModuleManager::Get().GetModuleFilename("PsdToUnreal");
		modulePath = FPaths::ConvertRelativePathToFull(modulePath);
		FString moduleDir, moduleFileName, moduleFileExt;
		FPaths::Split( modulePath, moduleDir, moduleFileName, moduleFileExt );
		FString binDir = moduleDir + TEXT("\\");
		// load the core module PSDto3D_Standalone_dev.dll (this plugin is PSDto3D_Unreal_dev.dll)
		FString binPath = binDir + TEXT("PSDto3D_Standalone_dev.dll");

		// Set environment variables so Qt can find its \platforms directory
		// Use _putenv_s() and its variant _tputenv_s(), instead of SetEnvironmentVariable(), which does not seem to work
		_tputenv_s( TEXT("QT_PLUGIN_PATH"), *binDir );
		_tputenv_s( TEXT("PATH"), *binDir );

		hLibrary = LoadLibrary( *binPath );
	}

	if( hLibrary!=NULL )
	{
		FARPROC lpfProcFunc = NULL;
		lpfProcFunc = GetProcAddress( hLibrary, "setPluginOutput" );
		if(lpfProcFunc!=NULL)
		{
			void* temp = lpfProcFunc; // to avoid compiler warning about function pointer type conversion
			t_vfnvp fn_setPluginOutput = (t_vfnvp)temp;
			fn_setPluginOutput( &pluginOutput );
		}

		lpfProcFunc = GetProcAddress( hLibrary, "openPlugin" );
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
	// Iterate through all groups in the Photoshop file ...
	psd_to_3d::GroupByNameMap::const_iterator iter;

	std::map< std::string, UMaterial* > materialLookup;

	std::string basePackageName = std::string("/Game/Textures/") + std::string(task.psdFileName) + std::string("/");

	for( iter = task.tree.begin(); iter!=task.tree.end(); iter++ )
	{
		// Iterate through all layers in the current group ...
		const psd_to_3d::GraphLayerGroup& graphLayerGroup = iter->second;

		for( int graphLayerIndex = 0; graphLayerIndex<graphLayerGroup.GetLayerCount(); graphLayerIndex++ )
		{
			const psd_to_3d::GraphLayer* graphLayerPtr = graphLayerGroup[graphLayerIndex];
			if( graphLayerPtr==nullptr ) continue;

			// create material if not already in lookup
			if( materialLookup.find( graphLayerPtr->TextureFilepath ) == materialLookup.end() )
			{
				UMaterial* material = CreateMaterial(
					graphLayerPtr->TextureFilepath.c_str(), graphLayerPtr->TextureName.c_str(), basePackageName.c_str() );
				materialLookup[ graphLayerPtr->TextureFilepath ] = material;
			}

			OutputLayerTask subtask( *graphLayerPtr );
			subtask.psdFileName = task.psdFileName;
			subtask.psdSceneWidth = task.psdSceneWidth;
			subtask.psdSceneHeight = task.psdSceneHeight;
			subtask.material = materialLookup[ graphLayerPtr->TextureFilepath ]; // lookup material
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

	FString layerName = FString( task.graphLayer.LayerName.c_str() );
	FString basePackageName = FString("/Game/Textures/") + FString(task.psdFileName) + FString("/");

	util::boundsUV layerRegion = task.graphLayer.LayerRegion; // relative to scene, range [0-1]
	int layerIndex = task.graphLayer.LayerIndex;
	float layerScale = task.graphLayer.Scale;
	float layerDepth = task.graphLayer.Depth;

	float aspectRatio = (float)task.psdSceneWidth / (float)task.psdSceneHeight;
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

	FVector meshSceneScale(1,1,1);

	FActorSpawnParameters meshParams;
	meshParams.Name = *layerName;
	meshParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested; // rename if name conflict

	AStaticMeshActor* meshActor = world->SpawnActor<AStaticMeshActor>(meshPos, meshRot, meshParams);
	if( meshActor!=nullptr )
	{
		FString folderPath = FString("/") + FString(task.psdFileName);
		meshActor->SetFolderPath( *folderPath );
		meshActor->SetActorLabel( layerName ); // TODO: Works in development builds only?
		meshActor->GetStaticMeshComponent()->SetStaticMesh( mesh );
		meshActor->GetStaticMeshComponent()->SetMaterial( 0, task.material );
		meshActor->GetRootComponent()->SetRelativeScale3D( meshSceneScale );
		meshActor->MarkComponentsRenderStateDirty();
	}
}

UMaterial* PsdToUnrealPluginOutput::CreateMaterial( const char* textureFilepath, const char* assetName, const char* packageName )
{
	UTexture2D* textureOriginal = FImageUtils::ImportFileAsTexture2D( textureFilepath );
	if( (textureOriginal==nullptr) || (textureOriginal->GetPlatformData()==nullptr) || (textureOriginal->GetPlatformData()->Mips.Num()<=0) )
		return nullptr; // need a texture to work from


	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Create texture asset

	int textureHeight = textureOriginal->GetSizeY(); // texture file width
	int textureWidth  = textureOriginal->GetSizeX(); // texture file height

	FTexture2DMipMap& mip = textureOriginal->GetPlatformData()->Mips[0];

	FString textureAssetName = FString(assetName) + FString("_png");
	FString texturePackageName = FString(packageName) + FString(textureAssetName);
	texturePackageName = UPackageTools::SanitizePackageName(texturePackageName);

	UPackage* texturePackage = CreatePackage(*texturePackageName); // v4.26.2
	//UPackage* texturePackage = CreatePackage(nullptr,*texturePackageName); // v4.24

	FCreateTexture2DParameters params;
	params.bUseAlpha = textureOriginal->HasAlphaChannel();

	TArray<FColor> bulkDataOutput;
	int bulkDataSize = mip.BulkData.GetBulkDataSize();
	int bulkDataEntries = bulkDataSize / 4;
	bulkDataOutput.Init( FColor(64,128,192,255), bulkDataEntries );

	void* bulkDataInput = mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(bulkDataOutput.GetData(), bulkDataInput, bulkDataSize );
	mip.BulkData.Unlock();
	UTexture2D* texture =
		FImageUtils::CreateTexture2D( textureWidth, textureHeight, bulkDataOutput,
			texturePackage, textureAssetName, RF_Public | RF_Standalone, params );

	texture->LODGroup = TEXTUREGROUP_Cinematic;
	FAssetRegistryModule::AssetCreated(texture);
	texturePackage->FullyLoad();
	texturePackage->Modify();

	texture->PostEditChange();
	texture->MarkPackageDirty();

	FString texturePackageFilename = FPackageName::LongPackageNameToFilename(texturePackageName, FPackageName::GetAssetPackageExtension());
	try
	{
		// texturePackage,				// UPackage* InOuter
		// texture,						// UObject* Base
		// RF_Public | RF_Standalone,	// EObjectFlags TopLevelFlags
		// *texturePackageFilename,		// const TCHAR* Filename
		// GError,						// FOutputDevice* Error = GError
		// nullptr,						// FLinkerNull* Conform = nullptr
		// false,						// bool bForceByteSwapping = false
		// true,						// bool bWarnOfLongFilename = true
		// SAVE_None					// uint32 SaveFlags = SAVE_None

		// const ITargetPlatform* TargetPlatform = nullptr,
		// const FDateTime& FinalTimeStamp = FDateTime::MinValue(),
		// bool bSlowTask = true

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
		if( GEditor->SavePackage(texturePackage, texture, *texturePackageFilename, args) )
		{
			texturePackage->PostEditChange();
		}
	}
	catch (...)
	{
		UE_LOG(LogTemp, Error, TEXT("Error saving %s"), *texturePackageFilename);
	}

	// TODO: See BaseDeviceProfiles.ini, to control the texture detauls->Level of Detail->Texture Group settings
	// For example, see \Projects\SunTemple\Config\DefaultDeviceProfiles.ini to control per-platform detail settings
	// Note that MIP generation may produce non-desirable results in the Alpha channel, controlled on a per-texture basis,
	// not controlled through ini file. See texture details->Level of Detail->Mip Gen Settings

	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Create material

	FString materialAssetName = FString( assetName ) + FString("_material");
	FString materialPackageName = FString( packageName ) + materialAssetName;
	materialPackageName = UPackageTools::SanitizePackageName(materialPackageName);
	UPackage* materialPackage = CreatePackage(*materialPackageName); // v4.26.2
	//UPackage* materialPackage = CreatePackage(NULL, *materialPackageName); // v4.24

	auto materialFactory = NewObject<UMaterialFactoryNew>();
	UMaterial* material = (UMaterial*)materialFactory->FactoryCreateNew(UMaterial::StaticClass(), materialPackage, *materialAssetName, RF_Standalone | RF_Public, NULL, GWarn);

	FAssetRegistryModule::AssetCreated(material);
	materialPackage->FullyLoad();
	materialPackage->SetDirtyFlag(true);

	//// make sure that any static meshes, etc using this material will stop using the FMaterialResource of the original
	//// material, and will use the new FMaterialResource created when we make a new UMaterial in place
	//FGlobalComponentReregisterContext RecreateComponents;


	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Create texture map connected to material

	material->PreEditChange(NULL);
	if( texture )
	{
		// set diffuse texture sampler
		UMaterialExpressionTextureSample* textureDiffuse = NewObject<UMaterialExpressionTextureSample>(material);
		textureDiffuse->Texture = texture;
		textureDiffuse->SamplerType = SAMPLERTYPE_Color;

		UMaterialEditorOnlyData* materialData = material->GetEditorOnlyData();

		material->GetExpressionCollection().AddExpression(textureDiffuse);
		materialData->BaseColor.Expression = textureDiffuse;
		materialData->BaseColor.MaskR = 1;
		materialData->BaseColor.MaskG = 1;
		materialData->BaseColor.MaskB = 1;
		materialData->BaseColor.MaskA = 0;

		// set opacity texture sampler
		materialData->OpacityMask.Expression = textureDiffuse;
		materialData->OpacityMask.MaskR = 0;
		materialData->OpacityMask.MaskG = 0;
		materialData->OpacityMask.MaskB = 0;
		materialData->OpacityMask.MaskA = 1;
		materialData->OpacityMask.Mask = 1;

		// set blend mode
		material->BlendMode = BLEND_Masked;

		// Tiling
		//textureExpression->Coordinates.Expression = Multiply;
	}
	material->PostEditChange();

	return material;
}


void PsdToUnrealPluginOutput::BeginSession( const IPluginOutputParameters& params )
{
	int bufCount = sizeof(session_psdFileName)/sizeof(session_psdFileName[0]);
	strcpy_s( session_psdFileName, bufCount, params.PsdName() );
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

	OutputLayersTask task( tree, running );
	task.psdFileName = session_psdFileName;
	task.psdSceneWidth = psdData.HeaderData.Width;
	task.psdSceneHeight = psdData.HeaderData.Height;

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

void PsdToUnrealPluginOutput::EndSession( const PsdData& psdData, const IPluginOutputParameters& params )
{
	session_psdFileName[0] = '\0'; // reset session variables
	psdData; // unused
	params; // unused
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
