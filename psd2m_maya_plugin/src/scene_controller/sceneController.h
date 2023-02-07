//----------------------------------------------------------------------------------------------
//
//  @file sceneController.h
//  @author Michaelson Britt
//  @date 25-02-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

//
// Scene Controller - Stores layer and atlas parameter inputs, and caches evaluation outputs
//

#ifndef SCENE_CONTROLLER_H
#define SCENE_CONTROLLER_H

#include "IControllerUpdate.h"
#include "parameters.h"
#include "outputs.h"
#include "meshGeneratorController.h"
#include "mesh_generator/dataMesh.h"
#include "json/JSON.h"
#include "util/readerWriterLock.h"

#include <thread>
#include <vector>
#include <list>
#include <map>

class QString; // forward declaration

namespace util
{
	class ProgressJob;
	class ProgressTask;
};

namespace psd_reader
{
	struct PsdData; // forward declaration
	struct LayerAndMaskData; // forward declaration
};


namespace psd_to_3d
{
	class LayerAgent; // forward declaration
	class AtlasAgent; // forward declaration
	class SceneControllerCache; // forward declaration
	class LayerParametersFilter; // forward declaration
	class PluginOutputParameters; // forward declaration
	class IPluginController; // forward declaration
	using util::ProgressJob;
	using util::ProgressTask;
	using mesh_generator::DataSurface;


	//----------------------------------------------------------------------------------------------
	typedef std::vector<LayerAgent*> LayerAgentList;
	typedef std::vector<AtlasAgent*> AtlasAgentList;
	typedef std::map<int,int> IndexMap;
	typedef std::map<int, const DataSurface&> MeshByLayerIndexRefMap;
	typedef std::map<int,boundsPixels> BoundsByLayerIndexMap;


	//----------------------------------------------------------------------------------------------
	// Layer Agent
	// Stores current layer parameters, modified by the UI
	// Provides an interface into the evaluation results and scene cache
	class LayerAgent
	{
	public:
		struct Surface {};
		struct Texture {};

		LayerAgent(SceneController& scene, const QString& layerName, int layerIndex, bool isImageLayer=true);

		bool IsImageLayer() const { return isImageLayer; }
		void SetImageLayer( bool b ) { this->isImageLayer=b; }
		int GetLayerIndex() const { return layerIndex; }
		LayerParameters& GetLayerParameters() { return layerParams; } // writable copy, modified by the UI
		const LayerParameters& GetLayerParameters() const { return layerParams; }
		LayerStats GetLayerStats() const;
		bool IsNull() const { return layerIndex<0; }

		// Interface into the evaluation results, waits on evaluation thread as needed
		void GenerateLayerMesh(DataSurface& mesh_out) const; 
		void GenerateLayerBounds(boundsPixels& bounds_out) const; // size within psd file, needed for texture export

	protected:
		bool isImageLayer;
		int layerIndex;
		LayerParameters layerParams;
		SceneController& scene;
	};


	//----------------------------------------------------------------------------------------------
	// Atlas Agent
	// Stores current atlas parameters, modified by the UI
	// Provides an interface into the evaluation result and scene cache
	class AtlasAgent
	{
	public:
		struct Status {};

		AtlasAgent(SceneController& scene, const QString& atlasName, int atlasIndex)
		: atlasIndex(atlasIndex), scene(scene), atlasParams(atlasName) {}

		int GetAtlasIndex() { return atlasIndex; }
		void SetAtlasIndex( int atlasIndex_in ) { this->atlasIndex = atlasIndex_in; }
		AtlasParameters& GetAtlasParameters() { return atlasParams; } // writable copy, modified by the UI
		const AtlasParameters& GetAtlasParameters() const { return atlasParams; }
		bool IsNull() const { return atlasIndex<0; }

		// Interface into the evaluation results, waits on evaluation thread as needed
		void GenerateAtlasBounds(boundsPixels& atlasBounds_out, BoundsByLayerIndexMap& layerBounds_out) const;

	protected:
		int atlasIndex;
		AtlasParameters atlasParams;
		SceneController& scene;
	};


	//----------------------------------------------------------------------------------------------
	// Scene Controller
	// Container for global parameters, and all layer and atlas parameters,
	// and manages evaluation and caching of layer and atlas results
	class SceneController
	{
	public:
		SceneController( const PsdData& psdData, IPluginController& plugin );
		~SceneController();

		void Init();
		void Free();

		// Input Data
		const PsdData& GetPsdData() { return psdData; }

		// Output Data
		void GenerateGraphLayer(GraphLayer& graphLayer_out, DataSurface& mesh_in_out, int layerIndex);
		void GenerateMeshes(GraphLayerByIndexMap& meshes_out, LayerParametersFilter& filter, ProgressTask& progressTask);
		void ApplyInfluenceLayer(GraphLayerByIndexMap& meshes_in_out);
		void CreateTreeStructure(GroupByNameMap& tree_out, GraphLayerByIndexMap& meshes_in_out, LayerParametersFilter& filter);

		// Global Parameters
		GlobalParameters& GetGlobalParameters();
		const GlobalParameters& GetGlobalParameters() const;

		// Layer Management
		int GetLayerCount() const;
		LayerAgent& GetLayerAgent(int layerIndex);
		const LayerAgent& GetLayerAgent(int layerIndex) const;
		LayerAgentList CollectLayerAgents(bool allLayers) const; // if allLayers is true, returns layers as per UI (no null entries), otherwise as per PSD
		void AddLayer(const QString& layerName, int layerIndex);

		// Texture Atlas Management
		int GetAtlasCount() const;
		AtlasAgent& GetAtlasAgent(int atlasIndex);
		const AtlasAgent& GetAtlasAgent(int atlasIndex) const;
		AtlasAgentList& GetAtlasAgents();
		const AtlasAgentList& GetAtlasAgents() const;
		IndexMap CleanupAtlasAgents();
		void AddAtlas( const QString& atlasName, int atlasIndex, int customSize=0, int customPadding=0, int packingAlgo=0 );

		// Caching
		void EnsureCacheCoherency();
		void PrepareMeshes( LayerParametersFilter& filter, ProgressTask& progressTask );

		// Event handling
		void Ping(); // called every few milliseconds from the main thread, to synchronize the UI and cache
		void NotifyPostExportTexture( LayerParametersFilter& filter ); // reset texture modified flags after an export
		void NotifyPostExportMesh( LayerParametersFilter& filter ); // reset texture modified flags after an export

		// Json Serialization
		void LoadValuesFromJson();
		void SaveValuesToJson();

	protected:
		friend class LayerAgent;
		friend class AtlasAgent;

		const PsdData& psdData;
		IPluginController& plugin;

		// Primary data, global params, layer params, atlas params
		GlobalParameters globalParams;
		LayerAgentList layerAgents;
		AtlasAgentList atlasAgents;
		LayerAgent nullLayerAgent;
		AtlasAgent nullAtlasAgent;
		AtlasAgent defaultAtlasAgent; // tracks layers not using an atlas

		// Helper data
		MeshGeneratorController meshGeneratorController;

		// Cache, performs evaluation and stores results
		SceneControllerCache* cachePtr;
		SceneControllerCache& cache; // for convenience

		void DeserializeContents(JSONObject& root);
		JSONValue* SerializeContents();
	};


	//----------------------------------------------------------------------------------------------
	// Helper classes - Layer filtering
	class LayerParametersFilter
	{
	public:
		const LayerAndMaskData& layerMaskData;
		const SceneController& scene;
		bool exportAll;
		LayerParametersFilter( const LayerAndMaskData& layerMaskData, const SceneController& scene, bool exportAll )
		: layerMaskData(layerMaskData), scene(scene), exportAll(exportAll) {}

		int Count();
		virtual bool operator()( int layerIndex ) = 0; // returns true if the layer is applicable, false if rejected
	};

	// select all layers which are active
	class AllLayerFilter : public LayerParametersFilter
	{
	public:
		AllLayerFilter( const LayerAndMaskData& layerMaskData, const SceneController& scene )
		: LayerParametersFilter(layerMaskData,scene,true) {}
		bool operator()( int ) { return true; }
	};

	// select all layers using the given atlas index
	class AtlasIndexLayerFilter : public LayerParametersFilter
	{
	public:
		int AtlasIndex;
		AtlasIndexLayerFilter( const LayerAndMaskData& layerMaskData, const SceneController& scene, bool exportAll, int atlasIndex )
		: LayerParametersFilter(layerMaskData,scene,exportAll), AtlasIndex(atlasIndex) {}
		bool operator()( int layerIndex ); // returns true if the layer is applicable, false if rejected
	};

	// select all layers which are active
	class ActiveLayerFilter : public LayerParametersFilter
	{
	public:
		ActiveLayerFilter( const LayerAndMaskData& layerMaskData, const SceneController& scene, bool exportAll )
		: LayerParametersFilter(layerMaskData,scene,exportAll) {}
		bool operator()( int layerIndex );
	};

	// select all layers which are active AND not using an atlas
	class ActiveNoAtlasLayerFilter : public LayerParametersFilter
	{
	public:
		ActiveNoAtlasLayerFilter( const LayerAndMaskData& layerMaskData, const SceneController& scene, bool exportAll )
		: LayerParametersFilter(layerMaskData,scene,exportAll) {}
		bool operator()( int layerIndex ); // returns true if the layer is applicable, false if rejected
	};


	//----------------------------------------------------------------------------------------------
	// Helper classes - Global params wrangling

	class PluginOutputParameters : public IPluginOutputParameters
	{
	public:
		PluginOutputParameters( const GlobalParameters& params ) : params(params) {}
		int FileWriteMode() const					{ return params.FileWriteMode; }
		int FileWriteLayout() const					{ return params.FileWriteLayout; }
		std::string FileImportPath() const			{ return params.FileImportPath.toUtf8().toStdString(); }
		std::string PsdName() const					{ return params.PsdName.toUtf8().toStdString(); }
		std::string FileExportPath() const			{ return params.FileExportPath.toUtf8().toStdString(); }
		std::string FileExportName() const			{ return params.FileExportName.toUtf8().toStdString(); }
		std::string FileExportExt() const			{ return params.FileExportExt.toUtf8().toStdString(); }
		std::string AliasPsdName() const			{ return params.AliasPsdName.toUtf8().toStdString(); }
	protected:
		const GlobalParameters& params;
	};




	//
	// OPERATIONS TO SUPPORT
	//
	// Post load
	// - Mark all cache entries as invalid
	// - Copy params into all cache entries
	// - Launch the cache update thread
	//
	// UI refresh
	// - [grab cache semaphore] for each layer and atlas, check if still calculating
	// - mark as calculating if so [release cache semaphore]
	//
	// UI parameter change
	// - [grab cache semaphore] Find the given layer cache item
	// - Mark it as invalid
	// - Launch the cache update thread if needed [release semaphore]
	//
	// Export
	// - Loop to check to wait for cache valdity...
	// - [grab cache semaphore] Check if all layers in cache are valid
	// - Count number which are invalid 
	// - If number invalid has decreased, update progress bar
	// - If any still invalid, sleep [release semaphore] then continue loop
	// - Check if all atlas layers are valid
	// - as above, count and update progress otherwise sleep [release semaphore] then continue loop
	// - Package all LayerEvalState and AtlasEvalState into LayerOutput objects
	// - Send these to exporter [release semaphore]
	//
	// CACHE UPDATE THREAD
	// - Loop to evaluate and update caches...
	// - [grab cache semaphore] for each layer, check if invalid
	// - for each layer invalid, mark as calculating, launch layer calculation thread
	// - for each atlas invalid if all layers available (how to check?),
	//   mark as calculating, launch atlas calculation thread [release cache semaphore]
	// - if nothing to calculate, mark thread to close [release cache semaphore]
	// - otherwise, wait for all calculation thread to stop then continue loop,
	//   or maybe just wait finite time (1/5th sec) and repeat, ignoring any states marked calculating
	//
	// LAYER CALCULATION THREAD
	// - Should be delivered a full LayerEvalState with params populated
	// - When calculations done [grab cache semaphore] check if params still match cache
	// - if so then update cache
	// - [release cache semaphore]
	//
	// THREAD POOL
	// - Use a shared pool for all these tasks
	// - Maybe update Delaunay to only split work into as many parts as idle threads,
	//   not just total concurrency

}


#endif // SCENE_CONTROLLER_H
