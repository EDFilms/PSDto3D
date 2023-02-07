//----------------------------------------------------------------------------------------------
//
//  @file sceneControllerCache.h
//  @author Michaelson Britt
//  @date 17-03-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

//
// Scene Controller Cache
// Helper class for SceneController, provides caching and multithreading
//

#ifndef SCENE_CONTROLLER_CACHE_H
#define SCENE_CONTROLLER_CACHE_H

#include "parameters.h"
#include "meshGeneratorController.h"
#include "mesh_generator/dataMesh.h"
#include "util/readerWriterLock.h"

#include <thread>
#include <vector>
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
	using util::ProgressJob;
	using util::ProgressTask;
	using mesh_generator::DataSurface;


	//----------------------------------------------------------------------------------------------
	typedef std::map<int, const DataSurface&> MeshByLayerIndexRefMap;
	typedef std::map<int,boundsPixels> BoundsByLayerIndexMap;


	//----------------------------------------------------------------------------------------------
	class SceneControllerCache
	{
	public:
		struct LayerEntry
		{
		public:
			LayerEntry( int layerIndex, const QString& layerName )
			: isInitialized(false), isReady(false), layerIndex(layerIndex), layerParams(layerName), progressTask(nullptr) {}
			LayerEntry( const LayerEntry& that );
			bool isInitialized = false; // true after layerParams is set the first time
			bool isReady = false; // true if mesh finished evaluated with current params
			int layerIndex;
			LayerParameters layerParams; // cache copy of params, may not match UI
			ProgressTask* progressTask; // while layer is calculating, this is non-null
			mutable DataSurface mesh; // cache result // TODO: avoid using mutable
		};
		typedef std::vector<LayerEntry> LayerEntries;

		struct AtlasEntry
		{
		public:
			AtlasEntry( int atlasIndex, const QString& atlasName )
			: isReady(false), atlasIndex(atlasIndex), atlasScale(1.0f), atlasParams(atlasName), atlasBounds(0,0,-1,-1) {}
			AtlasEntry( const AtlasEntry& that );
			bool isReady; // true if atlas finished evaluated with current params
			int atlasIndex;
			float atlasScale;
			AtlasParameters atlasParams; // cache copy of params, may not match UI
			boundsPixels atlasBounds; // actual size of atlas PNG
			boundsPixels atlasBoundsFit; // unscaled raw size of atlas packing
			BoundsByLayerIndexMap layerBounds; // cache result
		};
		typedef std::vector<AtlasEntry> AtlasEntries;

		class ProgressTaskCache : public ProgressTask
		{
		public:
			ProgressTaskCache( SceneControllerCache* parent )
			: ProgressTask(nullptr), parent(parent), layerIndex(-1) {}
			bool IsCancelled();
			void SetTask( int layerIndex, LayerParameters layerParams );  // layer currently being processed in background
			void ClearTask();
			SceneControllerCache* parent;
			LayerParameters layerParams;
			int layerIndex;
		};

		SceneControllerCache(const PsdData& psdData, GlobalParameters& globalParams, MeshGeneratorController& meshGeneratorController);
		~SceneControllerCache();
		void Free();
		void Init();

		bool IsEvalEnabled();

		void SetLayerCount( int layerCount );
		void SetLayer( const LayerParameters& layerParamsRequired, int layerIndex ); // copy required params into cache, may trigger recalculation
		bool IsLayerMatch( const LayerParameters& layerParamsRequired, int layerIndex ); // true if entry in cache matches required params

		void SetAtlasCount( int atlasCount );
		void SetAtlas( const AtlasParameters& atlasParamsRequired, int atlasIndex ); // copy required params into cache, may trigger recalculation
		bool IsAtlasMatch( const AtlasParameters& atlasParamsRequired, int atlasIndex ); // true if entry in cache matches required params

		bool IsStatsChanged(bool clear); // true if any layer or atlas has finished evaluating since last check; clear resets the flag to false
		LayerStats GetLayerStats( int layerIndex );

		float GetProgressTotal(); // progress value of all background tasks

		// Generate methods, block until cache is valid, return deep copy of cache result
		void GenerateLayerMesh( int layerIndex, ProgressTask& progressTask ); // blocks until mesh is generated only or cancel
		void GenerateLayerMesh( DataSurface& mesh_out, int layerIndex );
		void GenerateLayerBounds( boundsPixels& layerBounds_out, int layerIndex ); // bounds within psd
		void GenerateAtlasBounds( boundsPixels& atlasBounds_out, BoundsByLayerIndexMap& layerBounds_out, int atlasIndex ); // bounds within atlas

	protected:
		void CalculateLayer( LayerEntry& layerEntry, ProgressTask& progressTask );
		void CalculateAtlas( AtlasEntry& atlasEntry, ProgressTask& progressTask );

		const PsdData& psdData;
		GlobalParameters& globalParams;
		MeshGeneratorController& meshGeneratorController;

		bool isThreadEnabled; // set to false by main thread to terminate the cache thread
		bool isEvalEnabled;   // set to false by main thread to pause the cache thread, during file reload
		bool isEvalRunning;   // set to false by main thread to pause the cache thread, during file reload
		bool isStatsChanged; // set to true by cache thread to request UI redraw in main thread
		ReaderWriterLock lock;
		LayerEntries layerEntries;
		AtlasEntries atlasEntries;
		ProgressTaskCache progressTaskCache;
		std::thread cacheThread;
		static void CacheThread( SceneControllerCache* parent );
	};

}


#endif // SCENE_CONTROLLER_CACHE_H
