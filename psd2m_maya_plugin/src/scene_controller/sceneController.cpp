//----------------------------------------------------------------------------------------------
//
//  @file sceneController.cpp
//  @author Michaelson Britt
//  @date 25-02-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

//
// Mesh and Atlas parameters and evaluation
//


#include "sceneController.h"
#include "sceneControllerCache.h"
#include "maya_mesh/editorComponentGenerator.h" // TODO: remove, maya-specific, only for EditorComponentGenerator::materialNamePostfix
#include "interface/ui_wrapper.h" // for version flags, like IsStandaloneVersion
#include "json/JSON.h"
#include "util/utils.h"
#include "util/math_2D.h"
#include "util/bounds_2D.h"

typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <QFileInfo>
#include <locale>
#include <codecvt>

#pragma comment(lib, "RectangleBinPack.lib")


//--------------------------------------------------------------------------------------------------------------------------------------
// Helpers
std::wstring StringToWString(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

std::string WStringToString(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(str);
}


namespace psd_to_3d
{

#pragma region LAYER AGENT

	//--------------------------------------------------------------------------------------------------------------------------------------
	LayerAgent::LayerAgent( SceneController& scene, const QString& layerName, int layerIndex, bool isImageLayer )
	: scene(scene), layerParams(layerName), layerIndex(layerIndex), isImageLayer(isImageLayer)
	{
	const GlobalParameters& globalParams = this->scene.GetGlobalParameters();
	const PreferenceParameters& prefs = globalParams.Prefs;
	layerParams.DelaunayParameters.OuterDetailInterp.Init(
			prefs.DelaunayOuterDetailLo, prefs.DelaunayOuterDetailMid, prefs.DelaunayOuterDetailHi, false, false );
	layerParams.DelaunayParameters.InnerDetailInterp.Init(
			prefs.DelaunayInnerDetailLo, prefs.DelaunayInnerDetailMid, prefs.DelaunayInnerDetailHi, false, false );
	layerParams.DelaunayParameters.FalloffDetailInterp.Init( // use flat linear interp for lower half, improves handling perception
			prefs.DelaunayFalloffDetailLo, prefs.DelaunayFalloffDetailMid, prefs.DelaunayFalloffDetailHi, true, false );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	LayerStats LayerAgent::GetLayerStats() const
	{
		LayerStats layerStats = scene.cache.GetLayerStats( this->layerIndex );
		return layerStats;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void LayerAgent::GenerateLayerMesh(DataSurface& mesh_out) const
	{
		scene.cache.GenerateLayerMesh( mesh_out, this->layerIndex );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void LayerAgent::GenerateLayerBounds(boundsPixels& bounds_out) const
	{
		scene.cache.GenerateLayerBounds( bounds_out, this->layerIndex );
	}


// LAYER AGENT
#pragma endregion


#pragma region ATLAS AGENT

	//--------------------------------------------------------------------------------------------------------------------------------------
	void AtlasAgent::GenerateAtlasBounds(boundsPixels& atlasBounds_out, BoundsByLayerIndexMap& layerBounds_out) const
	{
		if( this->atlasIndex < 0 )
		{
			atlasBounds_out = boundsPixels(0,0,-1,-1);
			layerBounds_out.clear();
		}
		else
			scene.cache.GenerateAtlasBounds( atlasBounds_out, layerBounds_out, this->atlasIndex );
	}

// ATLAS AGENT
#pragma endregion


#pragma region SCENE CONTROLLER

	//--------------------------------------------------------------------------------------------------------------------------------------
	SceneController::SceneController(const PsdData& psdData, IPluginController& plugin)
	: psdData(psdData), plugin(plugin), meshGeneratorController(psdData,*this),
	  nullLayerAgent(*this,"NULL",-1), nullAtlasAgent(*this,"NULL",-1), defaultAtlasAgent(*this,"DEFAULT",-1),
	  cachePtr(new SceneControllerCache(psdData,globalParams,meshGeneratorController)), cache(*cachePtr)
	{
		Free();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	SceneController::~SceneController()
	{
		Free();
		delete cachePtr;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::Init()
	{
		// Create layer agents for each layer in the input PsdData
		// Loads stored parameters from disk and applies them to the global and layer parameters
		// Mimatches can occur between stored parameters and the PsdData loaded from disk,
		// so, the two data sets must be merged via layer name matching

		// Delete parameters, then load stored parameters from disk (if any)
		LoadValuesFromJson();

		// Create a mapping, layer names -> index values
		NameToNameMap curToJsonLayerMap; // maps layer index in current PSD to layer index in JSON settings
		for( auto& layer : psdData.LayerMaskData.Layers ) // iterate layers in current PSD
		{
			curToJsonLayerMap.InitSrcName( layer.LayerName.c_str() );
		}
		for( int layerIndex = 0; layerIndex < this->layerAgents.size(); layerIndex++ ) // iterate layers in JSON settings
		{
			LayerParameters& layerParams = this->layerAgents[layerIndex]->GetLayerParameters();
			curToJsonLayerMap.InitDstName( layerParams.LayerName.toUtf8().data() );
		}
		
		// Clear the parameters list again, making a copy of the stored parameters from disk first
		LayerAgentList jsonLayerParams(this->layerAgents.begin(), this->layerAgents.end());

		this->layerAgents.clear(); // don't clear atlas settings, only layer settings, which the incoming file might override
		for( int atlasIndex = 0; atlasIndex < this->atlasAgents.size(); atlasIndex++ ) // clear atlas layer indices, to patch
		{
			AtlasParameters& atlasParams = this->atlasAgents[atlasIndex]->GetAtlasParameters();
			atlasParams.layerIndices.clear();
		}

		int depthIndex = 0;

		// Create a new parameters list, copying from old params when possible
		for( auto& layer : psdData.LayerMaskData.Layers )
		{
			int layerIndex = iter_index(layer,psdData.LayerMaskData.Layers);

			LayerAgent* layerAgent = nullptr;
			if (layer.Type > psd_reader::TEXTURE_LAYER)
			{
				this->AddLayer( "NULL", layerIndex );
				GetLayerAgent(layerIndex).SetImageLayer(false); // non-image layer
			}
			else
			{
				QString layerName( QString::fromStdString(layer.LayerName) );
				this->AddLayer( layerName, layerIndex );

				LayerAgent& layerAgent = GetLayerAgent(layerIndex);
				LayerParameters& layerParams = layerAgent.GetLayerParameters();
				int jsonIndex = curToJsonLayerMap.MapSrcToDst(layerIndex);
				if( jsonIndex!=-1 )  // copy params loaded from json, if available
					layerParams = jsonLayerParams[jsonIndex]->GetLayerParameters();

				layerParams.DepthIndex = depthIndex++;
				layerParams.HasVectorSupport = !layer.PathRecords.empty();
				// If a path exists with same name as the layer,
				// but it's empty or contains fewer than three points, then the path is not usable
				layerParams.HasLinearSupport = false; // assume false unless usable path is found...
				if( psdData.ImageResourceData.IsPathExist(layer.LayerName) )
				{
					const ResourceBlockPath& path = psdData.ImageResourceData.GetBlockPath(layer.LayerName);

					// TODO: This should cull any spline in PathRecords of size 2 or less,
					// the check whether any splines remain.  Instead it bails if first spline has only 2 points

					if( (path.PathRecords.size()>0) && (path.PathRecords[0].Points.size()>=3) )
					{
						layerParams.HasLinearSupport = true; // found usable path
						layerParams.HasDelaunaySupport = layerParams.HasLinearSupport; // alias
					}
				}

				if( (!IsLinearModeSupported) && (layerParams.Algo==LayerParameters::LINEAR) )
					layerParams.Algo = LayerParameters::DELAUNAY; // revert from linear to delaunay if necessary

				AtlasParameters& atlasParams = GetAtlasAgent( layerParams.AtlasIndex ).GetAtlasParameters();
				atlasParams.layerIndices.insert( layerIndex );

				// Clear widget pointers until UI is rebuilt
				layerParams.ClearWidgets();
				layerParams.SetInfluenceLayer(psdData, layer);
			}
		}

		// Delete old params
		for (auto item : jsonLayerParams)
		{
			delete item;
		}

		// Cache update
		cache.Init();
		EnsureCacheCoherency(); // ensure cache has correct number of layer and atlas entries
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::Free()
	{
		cache.Free();

		// reset global values to default
		globalParams.Reset();

		// delete scene contents
		for( LayerAgent* layerAgent : this->layerAgents )
		{
			delete layerAgent;
		}
		for( AtlasAgent* atlasAgent : this->atlasAgents )
		{
			delete atlasAgent;
		}
		// remove all layer agents, and all atlas agents
		layerAgents.clear();
		atlasAgents.clear();
		// remove all layer indices from the default atlas agent; that is, clear the list of layers not using an atlas
		defaultAtlasAgent.GetAtlasParameters().layerIndices.clear();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::GenerateGraphLayer(GraphLayer& graphLayer_out, DataSurface& mesh_in_out, int layerIndex)
	{
		const LayerAgent& layerAgent = GetLayerAgent(layerIndex);
		const LayerParameters& layerParams = layerAgent.GetLayerParameters();
		const AtlasAgent& atlasAgent = GetAtlasAgent(layerParams.AtlasIndex);
		const AtlasParameters& atlasParams = atlasAgent.GetAtlasParameters();

		// Override group name with parent from scene, if any
		std::string parentGroupName;
		std::string textureName;
		std::string textureFilepath;
		const std::string& layerName = psdData.LayerMaskData.Layers[layerIndex].LayerName;
		if( layerParams.AtlasIndex >= 0 )
		{
			// TODO: centralize calculation of output filename,
			// currently smeared between ExportTexture(), ExportAtlas(), CreateTreeStructure() and IPluginOutput methods
			QString atlasNameNoSpace = atlasParams.atlasName.simplified().replace( " ", "_" );
			textureName = std::string("TextureAtlas_") + std::string( atlasNameNoSpace.toUtf8().data() );
		}
		else
		{
			textureName = layerName;
		}

		textureFilepath = std::string(globalParams.FileExportPath.toUtf8().data()) + "\\" + textureName + ".png";

		// Compute layer region with padding
		// TODO: is padding needed when EnableOutputCrop is false?
		boundsUV layerRegion = mesh_in_out.GetBoundsUV();
		boundsPixels layerBounds = mesh_in_out.GetBoundsPixels();
		layerRegion.Expand( psdData.HeaderData.Width, psdData.HeaderData.Height, globalParams.Padding ); // ...then expands for padding...

		// Compute xformUV
		xformUV layerXFormUV; // no effect by default
		if( layerParams.AtlasIndex >= 0 )
		{
			// Calculate size and location of layer rect within entire atlas rect,
			// measured in abstract units 0.0-1.0, not pixels.
			// Needed to calculate UV transform for the UV projection operator, later.
			boundsPixels atlasBounds; // bounds of entire atlas (size of atlas in pixels)
			BoundsByLayerIndexMap layerBoundsMap;
			boundsUV atlasRegion(0,0,1.0,1.0);
			atlasAgent.GenerateAtlasBounds(atlasBounds,layerBoundsMap);
			boundsPixels atlasLayerBounds = layerBoundsMap[layerIndex]; // bounds of layer within atlas (in pixels)
			atlasRegion.GenerateBoundingBox(atlasLayerBounds,atlasBounds);
			bool isRotated =  atlasLayerBounds.IsRotatedRelativeTo( layerBounds ); // Rotate the XFormUV if packing rotated the layer
			layerXFormUV.InitTransform( layerRegion, atlasRegion, isRotated );
		}
		else if( layerParams.EnableTextureCrop )
		{
			boundsUV fullRegion( 0.0f,0.0f, 1.0f,1.0f );
			layerXFormUV.InitTransform( layerRegion, fullRegion );
		}
		// else no transform; fullsize texture with no cropping or atlas

		float depth = layerParams.DepthIndex * -globalParams.Depth;
		float scale = globalParams.Scale;

		graphLayer_out.LayerName = layerName;
		graphLayer_out.MaterialName = textureName + maya_plugin::EditorComponentGenerator::materialNamePostfix; // TODO: remove maya-specific value here
		graphLayer_out.TextureName = textureName;
		graphLayer_out.TextureFilepath = textureFilepath;
		graphLayer_out.LayerIndex = layerIndex;
		graphLayer_out.AtlasIndex = layerParams.AtlasIndex;
		graphLayer_out.Depth = depth;
		graphLayer_out.Scale = scale;
		graphLayer_out.LayerRegion = layerRegion;
		graphLayer_out.XFormUV = layerXFormUV;
		graphLayer_out.Mesh = mesh_in_out; // take ownership, no reference counting
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::GenerateMeshes(GraphLayerByIndexMap& meshes_out, LayerParametersFilter& filter, ProgressTask& progressTask)
	{
		int meshesFinished = 0;
		// mesh creation
		for( auto const& layer : psdData.LayerMaskData.Layers )
		{
			int layerIndex = iter_index( layer, psdData.LayerMaskData.Layers );
			if( filter(layerIndex) )
			{
				DataSurface mesh;
				layerAgents[layerIndex]->GenerateLayerMesh( mesh );
				if( !mesh.IsEmpty() )
				{
					GraphLayer graphLayer;
					LayerParameters& layerParams = GetLayerAgent(layerIndex).GetLayerParameters();
					AtlasParameters& atlasParams = GetAtlasAgent(layerParams.AtlasIndex).GetAtlasParameters();
					GenerateGraphLayer( graphLayer, mesh, layerIndex );
					// Do not apply padding to mesh boundsUV;
					// see calculation of GraphLayer.XFormUV in CreateTreeStructure()
					meshes_out.try_emplace( layerIndex, graphLayer );
				}
				meshesFinished++;
				progressTask.SetValue( meshesFinished / (float)filter.Count() );
			}
		}

		if( IsInfluenceSupported )
		{
			ApplyInfluenceLayer( meshes_out ); //, progress
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::ApplyInfluenceLayer(GraphLayerByIndexMap& meshes_in_out)
	{
		for( GraphLayerByIndexItem& item : meshes_in_out )
		{
			// Iterate through surfaces which are meshes, skip any which are splines
			GraphLayer& graphLayer = item.second;
			DataSurface& surface = graphLayer.Mesh;
			DataMesh* meshPtr = surface.GetDataMesh();
			if(meshPtr==nullptr)
				continue; // Surface is not a mesh object, skip
	
			// Surface is a mesh object, proceed
			DataMesh& mesh = (*meshPtr);
			int index = psdData.LayerMaskData.GetIndexInfluenceLayer(mesh.GetName());
			if (index == -1)
				continue;
	
			int layerIndex = item.first;
			const LayerAgent& layerAgent = GetLayerAgent(layerIndex);
			const LayerParameters& layerParams = layerAgent.GetLayerParameters();
			if (!layerParams.EnableInfluence)
				continue;

			meshGeneratorController.ApplyInfluenceLayer( *meshPtr, layerIndex, layerParams.InfluenceParameters );
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::CreateTreeStructure(GroupByNameMap& tree_out, GraphLayerByIndexMap& meshes_in_out,  LayerParametersFilter& filter)
	{
		meshGeneratorController.CreateTreeStructure( tree_out, meshes_in_out, filter );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	GlobalParameters& SceneController::GetGlobalParameters()
	{
		return this->globalParams;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	const GlobalParameters& SceneController::GetGlobalParameters() const
	{
		return this->globalParams;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	int SceneController::GetLayerCount() const
	{
		return (int)(this->layerAgents.size());
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	LayerAgent& SceneController::GetLayerAgent(int layerIndex)
	{
		if( layerAgents.size()<=layerIndex )
		{
			return nullLayerAgent; // TODO: Should never happen
		}

		return *(this->layerAgents[layerIndex]);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	const LayerAgent& SceneController::GetLayerAgent(int layerIndex) const
	{
		if( layerAgents.size()<=layerIndex )
		{
			return nullLayerAgent; // TODO: Should never happen
		}

		return *(this->layerAgents[layerIndex]);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// Layer parameters list,
	// if allLayers is false, returned in parameter order as per the UI, with no null entries
	// if allLayers is true, returned in layer order as per the PSD file, with possible null entries
	LayerAgentList SceneController::CollectLayerAgents( bool allLayers ) const
	{
		LayerAgentList agents;
		int insertIndex = 0;
		for( auto& layerAgent : layerAgents )
		{
			if( allLayers || layerAgent->IsImageLayer() )
			{
				agents.push_back(layerAgent);
			}
		}

		return agents;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::AddLayer(const QString& layerName, int layerIndex)
	{
		if( layerIndex<0 ) // negative index means append to list
			layerIndex = (int)(layerAgents.size());

		if( layerIndex<=layerAgents.size() ) // ensure list is correct size ...
			this->layerAgents.resize( (layerIndex+1), nullptr );
		else if( layerAgents[layerIndex]!=nullptr ) // ...or delete existing entry
			delete layerAgents[layerIndex];

		layerAgents[layerIndex] = new LayerAgent(*this,layerName,layerIndex);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	int SceneController::GetAtlasCount() const
	{
		return (int)(this->atlasAgents.size());
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	AtlasAgent& SceneController::GetAtlasAgent(int atlasIndex)
	{
		if( atlasIndex<0 )
			return defaultAtlasAgent; // for layers not using an atlas
		else if( atlasIndex>=atlasAgents.size() )
		{
			return nullAtlasAgent;
		}

		return *(this->atlasAgents[atlasIndex]);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	const AtlasAgent& SceneController::GetAtlasAgent(int atlasIndex) const
	{
		if( (atlasIndex<0) || (atlasIndex>=atlasAgents.size()) )
		{
			return nullAtlasAgent;
		}

		return *(this->atlasAgents[atlasIndex]);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	AtlasAgentList& SceneController::GetAtlasAgents()
	{
		return this->atlasAgents;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	const AtlasAgentList& SceneController::GetAtlasAgents() const
	{
		return this->atlasAgents;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Delete unused atlas parameters
	IndexMap SceneController::CleanupAtlasAgents()
	{
		int atlasCount = (int)atlasAgents.size();

		std::vector<int> atlasUsage;
		atlasUsage.resize( atlasCount, 0 );

		// STEP 1. Count how many layers use each atlas
		for( LayerAgent* const& layerAgent : layerAgents )
		{
			LayerParameters& layerParams = layerAgent->GetLayerParameters();
			int atlasIndex = layerParams.AtlasIndex;
			if( atlasIndex>=0 )
			{
				atlasUsage[atlasIndex]++;
			}
		}

		IndexMap atlasRemap;
		atlasRemap[-1] = -1; // special case, index -1 indicates not using an atlas, keep unchanged
		int atlasIndexCur = 0;

		// STEP 2. Calculating the remapping and delete unused atlas parameters
		AtlasAgentList atlasParamsRemapped;
		for( AtlasAgent* const& atlasAgent : atlasAgents )
		{
			int atlasIndex = iter_index( atlasAgent, atlasAgents );
			if( atlasUsage[atlasIndex]==0 ) // atlas is not used; delete atlas parameters and update the remapping
			{
				delete atlasAgent;
				atlasRemap[atlasIndex] = -1;
			}
			else // atlas is used; update the remapping
			{
				atlasAgent->SetAtlasIndex( atlasIndexCur ); // update atlas agent to reflect the remapping
				atlasParamsRemapped.push_back( atlasAgent );
				atlasRemap[atlasIndex] = atlasIndexCur;
				atlasIndexCur++;
			}
		}
		atlasAgents = atlasParamsRemapped;

		// STEP 3. Update layers to reflect the remapping
		for( LayerAgent* const& layerAgent : layerAgents )
		{
			LayerParameters& layerParams = layerAgent->GetLayerParameters();
			layerParams.AtlasIndex = atlasRemap[ layerParams.AtlasIndex ];
		}

		return atlasRemap;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::AddAtlas( const QString& atlasName, int atlasIndex, int customSize, int customPadding, int packingAlgo )
	{
		if( atlasIndex<0 ) // negative index means append to list
			atlasIndex = (int)(atlasAgents.size());

		if( atlasIndex<=atlasAgents.size() ) // ensure list is correct size ...
			this->atlasAgents.resize( (atlasIndex+1), nullptr );
		else if( atlasAgents[atlasIndex]!=nullptr ) // ...or delete existing entry
			delete atlasAgents[atlasIndex];

		AtlasAgent* atlasAgent = new AtlasAgent(*this,atlasName,atlasIndex);
		atlasAgent->GetAtlasParameters().isCustomSize = (customSize>0);
		atlasAgent->GetAtlasParameters().customSize = (customSize>0? customSize : 1024);
		atlasAgent->GetAtlasParameters().customPadding = (customSize>0? customPadding : 2);
		atlasAgent->GetAtlasParameters().packingAlgo = packingAlgo;
		atlasAgents[atlasIndex] = atlasAgent;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::EnsureCacheCoherency()
	{
		// cache validity
		// ensure UI and cache have same number of items, and same params for each
		cache.SetLayerCount( (int)(layerAgents.size()) );
		for( LayerAgent*& layerAgent : layerAgents )
		{
			int layerIndex = iter_index(layerAgent, layerAgents);
			LayerParameters& layerParamsRequired = layerAgents[layerIndex]->GetLayerParameters();
			cache.SetLayer( layerParamsRequired, layerIndex );
		}
		cache.SetAtlasCount( (int)(atlasAgents.size()) );
		for( AtlasAgent*& atlasAgent : atlasAgents )
		{
			int atlasIndex = iter_index(atlasAgent, atlasAgents);
			AtlasParameters& atlasParamsRequired = atlasAgents[atlasIndex]->GetAtlasParameters();
			cache.SetAtlas( atlasParamsRequired, atlasIndex );
		}
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::PrepareMeshes( LayerParametersFilter& filter, ProgressTask& progressTask )
	{
		EnsureCacheCoherency();
		for( LayerAgent*& layerAgent : layerAgents )
		{
			int layerIndex = iter_index(layerAgent, layerAgents);
			if( filter(layerIndex) && !(progressTask.IsCancelled()) )
			{
				cache.GenerateLayerMesh( layerIndex, progressTask );
			}
		}
	}



	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::Ping()
	{
		EnsureCacheCoherency();
		if( cache.IsStatsChanged(true) )
		{
			plugin.GetNotifyStatus().DescriptionsChanged = true;
			plugin.NotifyCommand();
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::NotifyPostExportTexture( LayerParametersFilter& filter ) // reset texture modified flags after an export
	{
		for( LayerAgent*& layerAgent : this->layerAgents )
		{
			int layerIndex = iter_index(layerAgent,this->layerAgents);
			LayerParameters& layerParams = layerAgent->GetLayerParameters();
			int atlasIndex = layerParams.AtlasIndex;
			if( filter(layerIndex) )
			{
				layerParams.IsModifiedTexture = false; // TODO: maybe agent or cache should store this value, not params
				if( atlasIndex>=0 )
				{
					AtlasParameters& atlasParams = GetAtlasAgent(atlasIndex).GetAtlasParameters();
					atlasParams.isModifiedTexture = false; // TODO: maybe agent or cache should store this value, not params
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::NotifyPostExportMesh( LayerParametersFilter& filter ) // reset texture modified flags after an export
	{
		for( LayerAgent*& layerAgent : this->layerAgents )
		{
			int layerIndex = iter_index(layerAgent,this->layerAgents);
			if( filter(layerIndex) )
			{
				layerAgent->GetLayerParameters().IsModifiedMesh = false;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::LoadValuesFromJson()
	{
		// Try to get the params metadata.
		QFileInfo qtFilePath( this->globalParams.FileImportPath + "/" + this->globalParams.FileImportFilename );
		QFileInfo pathname = qtFilePath.path() + "/" + qtFilePath.baseName() + "/parameters.json";

		if (!pathname.exists())
		{
			return;
		}

		// Try to open the metadata.
		std::string pathname_utf8 = pathname.absoluteFilePath().toUtf8();
		std::ifstream file( util::to_utf16(pathname_utf8) );
		if (!file.is_open())
		{
			return;
		}

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		if( content.size() > 0 )
		{
			JSONValue* value = JSON::Parse(content.c_str());
			if( value != nullptr )
			{
				JSONObject root = value->AsObject();
				DeserializeContents(root);
				delete value;
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::SaveValuesToJson()
	{
		if (globalParams.FileImportPath == nullptr || globalParams.FileImportPath.isEmpty())
		{
			return;
		}

		QFileInfo qtFilePath( this->globalParams.FileImportPath + "/" + this->globalParams.FileImportFilename );
		QFileInfo paramsPath = qtFilePath.path() + "/" + qtFilePath.baseName() + "/parameters.json";
		std::string filename_utf8 = paramsPath.absoluteFilePath().toStdString();

		std::ofstream file( util::to_utf16(filename_utf8) );
		file << WStringToString(SerializeContents()->Stringify(true));
		file.close();

		globalParams.Prefs.Store(); // also store user preferences
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneController::DeserializeContents(JSONObject & root)
	{
		// Helper for Init()

		// Create layer agents and atlas agents from user preferences on disk, and update agent lists
		// NOTE: layerIndex numbers in JSON may not match PSD; layer->atlas matching should be patched later
		// Assumes the lists have been cleared via Free()
		// Mimatches can occur between stored parameters and the PsdData loaded from disk,
		// so, the two data sets must be merged via layer name matching in Init()

		if( root.count(L"ExportPath")>0 )
		{
			globalParams.FileExportPath = QString::fromWCharArray(root[L"ExportPath"]->AsString().c_str());
		}
		if( root.count(L"ExportName")>0 )
		{
			globalParams.FileExportName = QString::fromWCharArray(root[L"ExportName"]->AsString().c_str());
		}
		if( root.count(L"FileOutputPath")>0 ) // alias for ExportPath
		{
			globalParams.FileExportPath = QString::fromWCharArray(root[L"FileOutputPath"]->AsString().c_str());
		}

		if( root.count(L"TextureProxy")>0 )
		{
			globalParams.TextureProxy = root[L"TextureProxy"]->AsNumber();
		}
		globalParams.Depth = root[L"Depth"]->AsNumber();
		globalParams.Scale = root[L"Scale"]->AsNumber();
		globalParams.AliasPsdName = QString::fromWCharArray(root[L"AliasPsdName"]->AsString().c_str());

		if( root.count(L"WriteMode")>0 )
		{
			globalParams.FileWriteMode = (enum GlobalParameters::FileWriteMode)(int(root[L"WriteMode"]->AsNumber()));
		}
		if (root.count(L"WriteLayout") > 0)
		{
			globalParams.FileWriteLayout = (enum GlobalParameters::FileWriteLayout)(int(root[L"WriteLayout"]->AsNumber()));
		}
		//globalParams.KeepGroupStructure = root[L"KeepGroupStructure"]->AsBool(); // no longer supported

		if( root.count(L"AtlasGroups")>0 ) // Legacy support, Check whether param exist
		{
			JSONArray atlasNames = root[L"AtlasGroups"]->AsArray();

			for (auto atlasName : atlasNames)
			{
				JSONObject nameObject = atlasName->AsObject();
				QString atlasName = QString::fromWCharArray(nameObject[L"Name"]->AsString().c_str());
				int atlasIndex = nameObject[L"Index"]->AsNumber();
				int packingAlgo = 3;
				if( nameObject.count(L"PackingAlgo")>0 ) // Legacy support, Check whether param exist
				{
					packingAlgo = nameObject[L"PackingAlgo"]->AsNumber();
				}
				int customSize = 1024;
				if( nameObject.count(L"CustomSize")>0 ) // Legacy support, Check whether param exist
				{
					customSize = nameObject[L"CustomSize"]->AsNumber();
				}

				int customPadding = 2;
				if( nameObject.count(L"CustomPadding")>0 ) // Legacy support, Check whether param exist
				{
					customPadding = nameObject[L"CustomPadding"]->AsNumber();
				}

				this->AddAtlas( atlasName, atlasIndex, customSize, customPadding, packingAlgo );
			}
		}

		JSONArray layers = root[L"Layers"]->AsArray();
		for( auto& layer : layers )
		{
			int layerIndex = iter_index(layer,layers);
			JSONObject layerObject = layer->AsObject();
			QString layerName = QString::fromWCharArray( layerObject[L"Name"]->AsString().c_str() );

			// Add new layer
			// ---------- ----------
			// BUG: If the json doesn't match the current PSD due to user iteration and changes,
			// then the layerIndex here won't match the actual file, causing atlas indices to be garbled
			// ---------- ----------
			this->AddLayer( layerName, layerIndex );
			LayerAgent& layerAgent = GetLayerAgent(layerIndex); 
			LayerParameters& layerParams = layerAgent.GetLayerParameters();

			if( IsFullVersion )
			{
				layerParams.Algo = LayerParameters::Algorithm(int(layerObject[L"Algo"]->AsNumber()));
				layerParams.EnableInfluence = layerObject[L"InfluenceActivated"]->AsBool();
			}
			else
			{
				layerParams.Algo = LayerParameters::Algorithm::LINEAR;
				layerParams.EnableInfluence = false;
			}

			if( (layerParams.Algo==LayerParameters::Algorithm::LINEAR) && !IsLinearModeSupported )
			{
				layerParams.Algo = LayerParameters::Algorithm::BILLBOARD;
			}

			if( layerObject.count(L"AtlasGroup")>0 ) // Legacy support, Check whether param exist
				 layerParams.AtlasIndex = int(layerObject[L"AtlasGroup"]->AsNumber());
			else layerParams.AtlasIndex = -1;

			if( layerObject.count(L"TextureCropEnabled")>0 ) // Legacy support, Check whether param exist
				 layerParams.EnableTextureCrop = layerObject[L"TextureCropEnabled"]->AsBool();
			else layerParams.EnableTextureCrop = true;

			if( layerObject.count(L"BillboardParameters")>0 )
			{
				JSONObject billboardParams = layerObject[L"BillboardParameters"]->AsObject();
				layerParams.BillboardParameters.BillboardAlphaThresh = billboardParams[L"BillboardAlphaThresh"]->AsNumber();
			}

			JSONObject linearParams = layerObject[L"LinearParameters"]->AsObject();
			layerParams.LinearParameters.LinearHeightPoly = linearParams[L"LinearHeightPoly"]->AsNumber();

			if( layerObject.count(L"DelaunayParameters")>0 )
			{
				JSONObject delaunayParams = layerObject[L"DelaunayParameters"]->AsObject();
				layerParams.DelaunayParameters.OuterDetail = delaunayParams[L"OuterDetail"]->AsNumber();
				layerParams.DelaunayParameters.InnerDetail = delaunayParams[L"InnerDetail"]->AsNumber();
				layerParams.DelaunayParameters.FalloffDetail = delaunayParams[L"FalloffDetail"]->AsNumber();
			}

			JSONObject curveParams = layerObject[L"CurveParameters"]->AsObject();
			if( layerObject.count(L"MergeVertexEnabled")>0 ) // Legacy support, Check whether param exist
				 layerParams.CurveParameters.MergeVertexEnabled = curveParams[L"MergeVertexEnabled"]->AsBool();
			else layerParams.CurveParameters.MergeVertexEnabled = true;
			layerParams.CurveParameters.MergeVertexDistance = curveParams[L"MergeVertexDistance"]->AsNumber();

			JSONObject influenceParams = layerObject[L"InfluenceParameters"]->AsObject();
			layerParams.InfluenceParameters.MinPolygonSize = influenceParams[L"MinPolygonSize"]->AsNumber();
			layerParams.InfluenceParameters.MaxPolygonSize = influenceParams[L"MaxPolygonSize"]->AsNumber();

			// update atlas
			AtlasParameters& atlasParams = GetAtlasAgent( layerParams.AtlasIndex ).GetAtlasParameters();
			atlasParams.layerIndices.insert( layerIndex );
			// NOTE: layerIndex numbers in JSON may not match PSD; layer->atlas matching should be patched later
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	JSONValue* SceneController::SerializeContents()
	{
		JSONObject root;

		root[L"ExportPath"] = new JSONValue(globalParams.FileExportPath.toStdWString());
		root[L"ExportName"] = new JSONValue(globalParams.FileExportName.toStdWString());
		root[L"TextureProxy"] = new JSONValue(globalParams.TextureProxy);
		root[L"Depth"] = new JSONValue(globalParams.Depth);
		root[L"Scale"] = new JSONValue(globalParams.Scale);
		root[L"AliasPsdName"] = new JSONValue(globalParams.AliasPsdName.toStdWString());
		root[L"WriteMode"] = new JSONValue(globalParams.FileWriteMode);
		root[L"WriteLayout"] = new JSONValue(globalParams.FileWriteLayout);
		//root[L"KeepGroupStructure"] = new JSONValue(globalParams.KeepGroupStructure); // no longer supported

		// Remapping atlas index numbers;  unused texture atlas group names are not serialized
		std::vector<int> atlasUsage;
		int atlasCount = (int)(this->atlasAgents.size());
		atlasUsage.insert( atlasUsage.begin(), atlasCount, 0 ); // populate with zero
		// First step, counting. Each entry assigned the count of layers using the given atlas
		for( auto item : this->layerAgents )
		{	//if not using an atlas, do nothing, else increment the corresponding count
			LayerParameters& layerParams = item->GetLayerParameters();
			(layerParams.AtlasIndex<0)? (0) : (atlasUsage[layerParams.AtlasIndex]++);
		}
		// Second step, remapping. Each entry assigned an index, but skip those unused
		for( int i=0, atlasIndex=0; i<atlasCount; i++ )
		{	//if usage count zero, set index to -1, else set to increasing index value
			(atlasUsage[i]<=0)? (atlasUsage[i] = -1) : (atlasUsage[i] = atlasIndex++);
		}

		JSONArray atlasNames; // texture atlas group names
		for( int i=0; i<atlasCount; i++ )
		{
			if( atlasUsage[i]>=0 ) //entry zero in atlasAgents corresponds to index
			{	// there's a valid remapping for this entry, atlas is being used
				JSONObject nameObject;

				AtlasParameters& atlasParams = GetAtlasAgent(i).GetAtlasParameters();
				nameObject[L"Name"] = new JSONValue(atlasParams.atlasName.toStdWString());
				nameObject[L"Index"] = new JSONValue(atlasUsage[i]);
				nameObject[L"CustomSize"] = new JSONValue(atlasParams.isCustomSize? atlasParams.customSize : 1024);
				nameObject[L"CustomPadding"] = new JSONValue(atlasParams.isCustomSize? atlasParams.customPadding : 2);
				nameObject[L"PackingAlgo"] = new JSONValue(atlasParams.packingAlgo);

				atlasNames.push_back(new JSONValue(nameObject));
			}
		}

		root[L"AtlasGroups"] = new JSONValue(atlasNames);

		JSONArray layers;
		for( auto layerAgent : this->layerAgents )
		{
			JSONObject layerObject;
			LayerParameters& layerParams = layerAgent->GetLayerParameters();

			layerObject[L"Name"] = new JSONValue(layerParams.LayerName.toStdWString());
			layerObject[L"Algo"] = new JSONValue(layerParams.Algo);
			layerObject[L"AtlasGroup"] = new JSONValue( (layerParams.AtlasIndex<0)? (-1) : (atlasUsage[layerParams.AtlasIndex]) );
			layerObject[L"TextureCropEnabled"] = new JSONValue(layerParams.EnableTextureCrop);
			layerObject[L"InfluenceActivated"] = new JSONValue(layerParams.EnableInfluence);

			JSONObject billboardParams;
			billboardParams[L"BillboardAlphaThresh"] = new JSONValue(layerParams.BillboardParameters.BillboardAlphaThresh);

			JSONObject linearParams;
			linearParams[L"LinearHeightPoly"] = new JSONValue(layerParams.LinearParameters.LinearHeightPoly);

			JSONObject delaunayParams;
			delaunayParams[L"OuterDetail"] = new JSONValue(layerParams.DelaunayParameters.OuterDetail);
			delaunayParams[L"InnerDetail"] = new JSONValue(layerParams.DelaunayParameters.InnerDetail);
			delaunayParams[L"FalloffDetail"] = new JSONValue(layerParams.DelaunayParameters.FalloffDetail);

			JSONObject curveParams;
			curveParams[L"MergeVertexEnabled"] = new JSONValue(layerParams.CurveParameters.MergeVertexEnabled);
			curveParams[L"MergeVertexDistance"] = new JSONValue(layerParams.CurveParameters.MergeVertexDistance);

			JSONObject influenceParams;
			influenceParams[L"MinPolygonSize"] = new JSONValue(layerParams.InfluenceParameters.MinPolygonSize);
			influenceParams[L"MaxPolygonSize"] = new JSONValue(layerParams.InfluenceParameters.MaxPolygonSize);

			layerObject[L"BillboardParameters"] = new JSONValue(billboardParams);
			layerObject[L"LinearParameters"] = new JSONValue(linearParams);
			layerObject[L"DelaunayParameters"] = new JSONValue(delaunayParams);
			layerObject[L"CurveParameters"] = new JSONValue(curveParams);
			layerObject[L"InfluenceParameters"] = new JSONValue(influenceParams);

			layers.push_back(new JSONValue(layerObject));
		}

		root[L"Layers"] = new JSONValue(layers);

		return new JSONValue(root);
	}


// SCENE CONTROLLER
#pragma endregion


#pragma region LAYER FILTERING HELPERS

	//----------------------------------------------------------------------------------------
	int LayerParametersFilter::Count()
	{
		int count = 0;
		for( int layer_index=0; layer_index<layerMaskData.Layers.size(); layer_index++ )
		{
			if( this->operator()( layer_index ) )
				count++;
		}
		return count;
	}

	//----------------------------------------------------------------------------------------
	bool AtlasIndexLayerFilter::operator()( int layerIndex )
	{
		const LayerAgent& layerAgent = scene.GetLayerAgent(layerIndex);
		if( layerAgent.IsNull() )
			return false;
		const LayerParameters& layerParams = layerAgent.GetLayerParameters();
		return layerParams.AtlasIndex==AtlasIndex;
	}

	//----------------------------------------------------------------------------------------
	static bool ActiveLayerFilterFn( const psd_reader::LayerData& layer, const LayerAgent& layerAgent, bool exportAll )
	{
		if( layer.Type > psd_reader::TEXTURE_LAYER )
			return false;

		const LayerParameters& layerParams = layerAgent.GetLayerParameters();
		// TODO: Allow empty layers, support this case elsewhere
		//bool empty = (layerBounds.WidthPixels()<=0) && (layerBounds.HeightPixels()<=0);
		bool empty = false; // TODO: test with background an unsupported layers
		bool exportable = (layerParams.IsActive || exportAll);
		if( empty || !exportable ) // if the layer is empty or shouldn't be exported otherwise, skip it
			return false;

		return true;
	}

	bool ActiveLayerFilter::operator()( int layerIndex )
	{
		const LayerAgent& layerAgent = scene.GetLayerAgent(layerIndex);
		const psd_reader::LayerData& layer = layerMaskData.Layers[layerIndex];
		return ActiveLayerFilterFn( layer, layerAgent, exportAll );
	}

	bool ActiveNoAtlasLayerFilter::operator()( int layerIndex )
	{
		const LayerAgent& layerAgent = scene.GetLayerAgent(layerIndex);
		const LayerParameters& layerParams = layerAgent.GetLayerParameters();
		const psd_reader::LayerData& layer = layerMaskData.Layers[layerIndex];
		if( ActiveLayerFilterFn(layer, layerAgent, exportAll) )
			return (layerParams.AtlasIndex<0);
		return false; // layer not active
	}

// LAYER FILTERING HELPERS
#pragma endregion


}
