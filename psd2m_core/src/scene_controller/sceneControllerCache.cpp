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


#include "sceneControllerCache.h"
#include "scene_controller/meshGeneratorController.h"
#include "maya_mesh/editorComponentGenerator.h" // TODO: remove, maya-specific, only for EditorComponentGenerator::materialNamePostfix
#include "mesh_generator/dataMesh.h"
#include "util/math_2D.h"
#include "util/bounds_2D.h"

#include "MaxRectsBinPack.h"

typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <QFileInfo>
#include <windows.h> // for Sleep()


#pragma comment(lib, "RectangleBinPack.lib")


namespace psd_to_3d
{

#pragma region SCENE CACHE

	//--------------------------------------------------------------------------------------------------------------------------------------
	SceneControllerCache::LayerEntry::LayerEntry( const SceneControllerCache::LayerEntry& that )
	: isInitialized(that.isInitialized), isReady(that.isReady), layerIndex(that.layerIndex), layerParams(that.layerParams), progressTask(nullptr)
	{
		this->mesh = that.mesh; // take ownership, no reference counting
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	SceneControllerCache::AtlasEntry::AtlasEntry( const SceneControllerCache::AtlasEntry& that )
	: isReady(that.isReady), atlasIndex(that.atlasIndex), atlasScale(that.atlasScale), atlasParams(that.atlasParams)
	{
		this->atlasBounds = that.atlasBounds;
		this->layerBounds = that.layerBounds;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool SceneControllerCache::ProgressTaskCache::IsCancelled()
	{
		// if cache is still running evaluation, and if task params still match the UI params, the task is still running ...
		bool running = (layerIndex>=0) && parent->IsEvalEnabled() && parent->IsLayerMatch( this->layerParams, this->layerIndex );
		return (!running); // ... else cancel the task
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::ProgressTaskCache::SetTask( int layerIndex, LayerParameters layerParams )
	{
		this->layerIndex = layerIndex;
		this->layerParams = layerParams;
		SetValue(0);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::ProgressTaskCache::ClearTask()
	{
		this->layerIndex = -1;
		this->layerParams = LayerParameters();
		SetValue(0);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	SceneControllerCache::SceneControllerCache(const PsdData& psdData, GlobalParameters& globalParams, MeshGeneratorController& meshGeneratorController)
	: isThreadEnabled(true), isEvalEnabled(false), isEvalRunning(false), isStatsChanged(false), progressTaskCache(this),
	  psdData(psdData), globalParams(globalParams), meshGeneratorController(meshGeneratorController), cacheThread(CacheThread,this)
	{
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	SceneControllerCache::~SceneControllerCache()
	{
		{	// enter scope for lock guard
			LockWriteGuard lockGuard( this->lock );
			this->isThreadEnabled = false; // signal thread to stop
		}
		cacheThread.join(); // wait for cache thread
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::Free()
	{
		{	// enter scope for lock guard
			LockWriteGuard lockGuard( this->lock );
			this->layerEntries.clear(); // delete cache entries; cache thread should handle this safely
			this->atlasEntries.clear();
			this->isEvalEnabled = false; // pause cache evaluation
		}
		for( bool isEvalRunning=true; isEvalRunning; ) // wait until cache evaluation is paused
		{
			LockWriteGuard lockGuard( this->lock );
			isEvalRunning = this->isEvalRunning;
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::Init()
	{
		LockWriteGuard lockGuard( this->lock );
		this->isEvalEnabled = true;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool SceneControllerCache::IsEvalEnabled()
	{
		LockWriteGuard lockGuard( this->lock );
		return this->isEvalEnabled;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::SetLayerCount( int layerCount )
	{
		LockWriteGuard lockCacheGuard( this->lock );

		while( layerCount < this->layerEntries.size() ) // reduce size by one, while too large ...
			this->layerEntries.pop_back();
		while( layerCount > this->layerEntries.size() ) // increase size by one, while too small ...
		{
			int layerIndexNext = (int)(this->layerEntries.size());
			this->layerEntries.push_back( LayerEntry(layerIndexNext,"") ); // add stub entries
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::SetLayer( const LayerParameters& layerParamsRequired, int layerIndex )
	{
		LockWriteGuard lockCacheGuard( this->lock );

		if( layerIndex >= this->layerEntries.size() )
			SetLayerCount( layerIndex+1 );

		bool isInitialized = this->layerEntries[layerIndex].isInitialized;
		bool isMatch = this->IsLayerMatch(layerParamsRequired,layerIndex);

		if( (!isMatch) || (!isInitialized) )
		{
			this->layerEntries[layerIndex].layerParams = layerParamsRequired;
			this->layerEntries[layerIndex].isReady = false;
			int atlasIndex = this->layerEntries[layerIndex].layerParams.AtlasIndex;
			if( atlasIndex >= 0 ) // mark corresponding atlas as invalid
			{
				if( this->atlasEntries.size() > atlasIndex ) // should always be true
					this->atlasEntries[atlasIndex].isReady = false;
			}
			this->isStatsChanged = true;
			this->layerEntries[layerIndex].isInitialized = true;
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool SceneControllerCache::IsLayerMatch( const LayerParameters& layerParamsRequired, int layerIndex )
	{
		LockWriteGuard lockCacheGuard( this->lock );
		if( this->layerEntries.size()>layerIndex )
		{
			const LayerParameters& layerParamsCache = this->layerEntries[layerIndex].layerParams;
			bool isMatch = true;
			if( layerParamsRequired.Algo != layerParamsCache.Algo )									isMatch = false;
			if( layerParamsRequired.AtlasIndex != layerParamsCache.AtlasIndex )						isMatch = false;
			if( layerParamsRequired.InfluenceParameters != layerParamsCache.InfluenceParameters )	isMatch = false;
			if( (layerParamsRequired.Algo == LayerParameters::LINEAR)    && (layerParamsRequired.LinearParameters != layerParamsCache.LinearParameters) )		isMatch = false;
			if( (layerParamsRequired.Algo == LayerParameters::BILLBOARD) && (layerParamsRequired.BillboardParameters != layerParamsCache.BillboardParameters) )	isMatch = false;
			if( (layerParamsRequired.Algo == LayerParameters::DELAUNAY)  && (layerParamsRequired.DelaunayParameters != layerParamsCache.DelaunayParameters) )	isMatch = false;
			if( (layerParamsRequired.Algo == LayerParameters::CURVE)     && (layerParamsRequired.CurveParameters != layerParamsCache.CurveParameters) )			isMatch = false;
			return isMatch;
		}
		return false; // else no entry in cache; might happen after Free() is called in main thread
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::SetAtlasCount( int atlasCount )
	{
		LockWriteGuard lockCacheGuard( this->lock );

		while( atlasCount < this->atlasEntries.size() ) // reduce size by one, while too large ...
			this->atlasEntries.pop_back();
		while( atlasCount > this->atlasEntries.size() ) // increase size by one, while too small ...
		{
			int atlasIndexNext = (int)(this->atlasEntries.size());
			this->atlasEntries.push_back( AtlasEntry(atlasIndexNext,"") ); // add stub entries
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::SetAtlas( const AtlasParameters& atlasParamsRequired, int atlasIndex )
	{
		LockWriteGuard lockCacheGuard( this->lock );

		if( atlasIndex >= this->atlasEntries.size() )
			SetAtlasCount( atlasIndex+1 );

		bool isMatch = this->IsAtlasMatch(atlasParamsRequired,atlasIndex);
		if( !isMatch )
		{
			this->atlasEntries[atlasIndex].atlasParams = atlasParamsRequired;
			this->atlasEntries[atlasIndex].isReady = false;
			this->isStatsChanged = true;
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool SceneControllerCache::IsAtlasMatch( const AtlasParameters& atlasParamsRequired, int atlasIndex )
	{
		LockWriteGuard lockCacheGuard( this->lock );
		if( this->atlasEntries.size()>atlasIndex )
		{
			const AtlasParameters& atlasParamsCache = this->atlasEntries[atlasIndex].atlasParams;
			bool isMatch = true;
			if( !atlasParamsRequired.IsMatch(atlasParamsCache) ) isMatch = false;
			return isMatch;
		}
		return false; // else no entry in cache; might happen after Free() is called in main thread
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool SceneControllerCache::IsStatsChanged(bool clear)
	{
		LockWriteGuard lockCacheGuard( this->lock );
		bool retval = isStatsChanged;
		if( clear ) isStatsChanged = false;
		return retval;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	LayerStats SceneControllerCache::GetLayerStats( int layerIndex )
	{
		LockWriteGuard lockCacheGuard( this->lock );
		LayerStats layerStats;
		const SceneControllerCache::LayerEntry& layerEntryCache = this->layerEntries[layerIndex];
		const LayerParameters& layerParamsCache = layerEntryCache.layerParams;

		bool isBillboard = (layerParamsCache.Algo==LayerParameters::Algorithm::BILLBOARD);
		bool isLinear = (layerParamsCache.Algo==LayerParameters::Algorithm::LINEAR);
		bool isDelaunay = (layerParamsCache.Algo==LayerParameters::Algorithm::DELAUNAY);
		bool isVector = (layerParamsCache.Algo==LayerParameters::Algorithm::CURVE);
		if( isBillboard )
			layerStats.isAlgoSupported = true;
		else if( (isLinear) && (!layerParamsCache.HasLinearSupport) )
			layerStats.isAlgoSupported = false; // unsupported layer, no corresponding path in psd,  when algo requires it
		else if( (isDelaunay) && (!layerParamsCache.HasDelaunaySupport) )
			layerStats.isAlgoSupported = false; // unsupported layer, no corresponding path in psd,  when algo requires it
		else if( (isVector) && (!layerParamsCache.HasVectorSupport) )
			layerStats.isAlgoSupported = false; // unsupported layer, no vector mask, when algo requires it
		else layerStats.isAlgoSupported = true; // supported layer, default
		layerStats.isLayerReady = layerEntryCache.isReady;
		layerStats.layerBounds = layerEntryCache.mesh.GetBoundsPixels();

		if( layerParamsCache.AtlasIndex>=0 )
		{
			const SceneControllerCache::AtlasEntry& atlasEntryCache = this->atlasEntries[layerParamsCache.AtlasIndex];
			layerStats.isAtlasReady = atlasEntryCache.isReady;
			layerStats.atlasBounds = atlasEntryCache.atlasBounds;
			layerStats.atlasBoundsFit = atlasEntryCache.atlasBoundsFit;
		}
		else
		{
			layerStats.isAtlasReady = false;
			layerStats.atlasBounds = boundsPixels(0,0,-1,-1);
		}

		return layerStats;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	float SceneControllerCache::GetProgressTotal() // progress value of all background tasks{
	{
		LockWriteGuard lockCacheGuard( this->lock );
		int layerCount = (int)(this->layerEntries.size());
		float progress = 0.0f;
		float progressPerLayer = (1.0f/layerCount);
		for( int layerIndex=0; layerIndex<layerCount; layerIndex++ )
		{
			LayerEntry& layerEntry = this->layerEntries[layerIndex];
			if( layerEntry.isReady )
				progress += progressPerLayer;
			else if( layerEntry.progressTask!=nullptr )
				progress += progressPerLayer * layerEntry.progressTask->GetValue();
		}
		return progress;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::GenerateLayerMesh( int layerIndex, ProgressTask& progressTask )
	{
		bool isReady = false;
		while( !isReady && !(progressTask.IsCancelled()) )
		{
			// check for valid mesh in cache
			{
				// enter scope for lock guard
				LockWriteGuard lockCacheGuard( this->lock );
				const SceneControllerCache::LayerEntry& layerCacheEntry = this->layerEntries[layerIndex];
				// measure progress of entire background work, not this layer only;
				// delay might be caused by background crunching a different layer, so reflect that in progress bar
				progressTask.SetValueAndUpdate( this->GetProgressTotal() );
				isReady = layerCacheEntry.isReady;
			}
			// if not valid, wait for cache thread to catch; it gets triggered during Ping()
			if( !isReady )
				Sleep(16); // sixteen milliseconds between checks, about 60fps
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::GenerateLayerMesh( DataSurface& mesh_out, int layerIndex )
	{
		bool isReady = false;
		while( !isReady )
		{
			// check for valid mesh in cache
			{
				// enter scope for lock guard
				LockWriteGuard lockCacheGuard( this->lock );
				const SceneControllerCache::LayerEntry& layerCacheEntry = this->layerEntries[layerIndex];
				isReady = layerCacheEntry.isReady;
				if( isReady )
					mesh_out.CloneFrom( layerCacheEntry.mesh );
			}
			// if not valid, wait for cache thread to catch; it gets triggered during Ping()
			if( !isReady )
				Sleep(16); // sixteen milliseconds between checks, about 60fps
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::GenerateLayerBounds( boundsPixels& layerBounds_out, int layerIndex )
	{
		bool isReady = false;
		while( !isReady )
		{
			// check for valid mesh in cache
			{
				// enter scope for lock guard
				LockWriteGuard lockCacheGuard( this->lock );
				const SceneControllerCache::LayerEntry& layerCacheEntry = this->layerEntries[layerIndex];
				isReady = layerCacheEntry.isReady;
				if( isReady )
					layerBounds_out = layerCacheEntry.mesh.GetBoundsPixels();
			}
			// if not valid, wait for cache thread to catch; it gets triggered during Ping()
			if( !isReady )
				Sleep(16); // sixteen milliseconds between checks, about 60fps
		}
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::GenerateAtlasBounds( boundsPixels& atlasBounds_out, BoundsByLayerIndexMap& layerBounds_out, int atlasIndex )
	{
		bool isReady = false;
		while( !isReady )
		{
			// check for valid mesh in cache
			{
				// enter scope for lock guard
				LockWriteGuard lockCacheGuard( this->lock );
				const SceneControllerCache::AtlasEntry& atlasCacheEntry = this->atlasEntries[atlasIndex];
				isReady = atlasCacheEntry.isReady;
				if( isReady )
				{
					atlasBounds_out = atlasCacheEntry.atlasBounds;
					layerBounds_out = atlasCacheEntry.layerBounds;
				}
			}
			// if not valid, wait for cache thread to catch; it gets triggered during Ping()
			if( !isReady )
				Sleep(16); // sixteen milliseconds between checks, about 60fps
		}
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::CalculateLayer( LayerEntry& layerEntry, ProgressTask& progressTask )
	{
		meshGeneratorController.GenerateMesh( layerEntry.mesh, layerEntry.layerParams, layerEntry.layerIndex, progressTask );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::CalculateAtlas( AtlasEntry& atlasEntry, ProgressTask& progressTask )
	{
		AtlasParameters& atlasParams = atlasEntry.atlasParams;

		// clear results in atlasEntry
		atlasEntry.layerBounds.clear();

		// populate list of layer bounds for the atlas, from the raw layer size values, with handling for padding parameters
		// these will converted to the packed bounds by the MaxRectsBinPack algorithm,
		// unless it's the default atlas (for layers not using an atlas) which just lists original raw layer sizes values
		std::vector<rbp::RectSize> binSizes; //struct RectSize from MaxRectsBinPack.h
		int totalPadding = (2*atlasParams.customPadding); // for convenience; total number of padding pixels, width and height, per island

		{	// enter scope for lock guard
			LockWriteGuard lockCacheGuard( lock );
			// for each layer in the atlas params, even in atlas index is -1
			for( int layerIndex : atlasParams.layerIndices )
			{
				// apply the mesh bounds into atlasRegion, for use below
				LayerEntry& layerEntry = layerEntries[layerIndex];
				boundsPixels layerBounds = layerEntry.mesh.GetBoundsPixels();
				atlasEntry.layerBounds[layerIndex] = layerBounds;

				if( atlasEntry.atlasIndex>=0 )
				{
					layerBounds.Clip( psdData.HeaderData.Width, psdData.HeaderData.Height );
					rbp::RectSize rectPaddedSize;
					rbp::Rect rectPadded; // apply padding around each atlas island
					rectPadded.x = layerBounds.XPixels()-(atlasParams.customPadding);
					rectPadded.y = layerBounds.YPixels()-(atlasParams.customPadding);
					rectPadded.width = rectPaddedSize.width = (layerBounds.WidthPixels()+totalPadding);
					rectPadded.height = rectPaddedSize.height = (layerBounds.HeightPixels()+totalPadding);
					rectPadded.id.val = rectPaddedSize.id.val = layerIndex;
					// Algorithm breaks if given any zero-size rectangles
					if( (rectPaddedSize.width>0) && (rectPaddedSize.height>0) )
					{
						binSizes.push_back(rectPaddedSize);
					}
				}
			}
		}

		if( atlasEntry.atlasIndex>=0 )
		{
			std::vector<rbp::Rect> binSrc; //struct Rect from MaxRectsBinPack.h
			std::vector<rbp::Rect> binDst; //struct Rect from MaxRectsBinPack.h

			// determine the optimal packing layout, fiting atlas contents into a square;
			// if the user requested a smaller size, the same layout should be used with downscaling applied, and
			// if the user requested a larger size, padding should be added to right and bottom of the layout 

			// perform binary search to determine optimal packing layout
			std::vector<rbp::Rect> binDstBest; //struct Rect from MaxRectsBinPack.h
			int binSizeMin = 512, binSizeMax = (atlasParams.isCustomSize? atlasParams.customSize : 8192);
			int binSizeLo = 512, binSizeHi = 32768; // binary search moving bounds
			int binSizeFit = 2048; // best packed size, to be determined below
			for( int iters=0; (binSizeLo<binSizeHi) && iters<15; iters++ )
			{
				// MaxRectsBinPack: Perform the packing.
				std::vector<rbp::RectSize> binInput = binSizes;
				rbp::MaxRectsBinPack bin;
				binSizeFit = (binSizeLo + binSizeHi) / 2; // best packed size, to be determined below
				bin.Init(binSizeFit, binSizeFit, true); // allow rotation
				int heuristic = atlasParams.packingAlgo;
				bin.Insert( binInput, binDst, (rbp::MaxRectsBinPack::FreeRectChoiceHeuristic)heuristic );
				if( binInput.size() == 0 )
				{
					binSizeHi = binSizeFit; // everything packed, use same or smaller size
					binDstBest = binDst; // keep a copy of these good results
				}
				else
				{
					binSizeLo = binSizeFit; // not everything packed, use larger size
				}
			}
			binDst = binDstBest;
			binSizeFit = binSizeHi;

			float atlasScale = 1.0f; // default
			int atlasSize = binSizeFit; // default


			// atlas size is the user specified size, if specified, otherwise the optimal size up to a maximum
			if( atlasParams.isCustomSize )
				atlasSize = atlasParams.customSize;

			atlasSize = CLAMP( atlasSize, binSizeMin, binSizeMax );

			// if packing exceeded the max size, then calculate downscaling to fit within the max size
			if( binSizeFit > atlasSize )
				atlasScale = atlasSize/(float)binSizeFit;


			// Update each atlas bounds
			for (auto const& rect : binDst)
			{
				int rect_x = (int)(rect.x * atlasScale);
				int rect_y = (int)(rect.y * atlasScale);
				int rect_width = (int)(rect.width * atlasScale);
				int rect_height = (int)(rect.height * atlasScale);
				int layerIndex = rect.id.val;
				// Do not remove padding in AtlasRegion;
				// see calculation of GraphLayer.XFormUV in CreateTreeStructure()
				atlasEntry.layerBounds[layerIndex] = boundsPixels(rect_x, rect_y, rect_width, rect_height);
				// if the layer was rotated during packing, this needs to be detected during texture export and UV generation
			}

			// Update atlas bounds
			atlasEntry.atlasScale = atlasScale;
			atlasEntry.atlasBounds = boundsPixels(0,0, atlasSize, atlasSize);
			atlasEntry.atlasBoundsFit = boundsPixels(0,0, binSizeFit, binSizeFit);
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void SceneControllerCache::CacheThread( SceneControllerCache* parent )
	{
		SceneControllerCache& cache = *parent;
		MeshGeneratorController& meshGeneratorController = cache.meshGeneratorController;
		bool isThreadEnabled, isEvalEnabled;
		for( isThreadEnabled = true; isThreadEnabled; )
		{
			std::vector<LayerEntry> layerTasks;
			std::vector<AtlasEntry> atlasTasks;

			// Create a task list of each layer and atlas which is not valid (requires update)
			{   // enter scope for lock guard
				LockWriteGuard lockCacheGuard( cache.lock );
				isThreadEnabled = cache.isThreadEnabled; // check for thread termination request
				isEvalEnabled = cache.isEvalEnabled; // check for thread termination request
				if( isThreadEnabled && isEvalEnabled )
				{
					cache.isEvalRunning = true;
					for( const LayerEntry& layerCacheEntry : cache.layerEntries )
					{
						if( !layerCacheEntry.isReady )
							layerTasks.push_back(layerCacheEntry);
					}

					for( const AtlasEntry& atlasCacheEntry : cache.atlasEntries )
					{
						if( !atlasCacheEntry.isReady )
							atlasTasks.push_back(atlasCacheEntry);
					}
				}
			}

			// Generate mesh for each layer task
			for( LayerEntry& task : layerTasks )
			{
			// TODO: whenever SetValue() is called in the custom ProgressTask,
			// this should update a "ready progress" value inside the task (?), for GenerateLayerMesh()
			// from main thread to read, alongside this->layerEntries[layerIndex].isReady
				if( isThreadEnabled && isEvalEnabled )
				{
					{ // enter scope for lock guard
						LockWriteGuard lockCacheGuard( cache.lock );
						// progress object allows task to stop when UI params change, or when scene is closed or reloaded
						cache.progressTaskCache.SetTask( task.layerIndex, task.layerParams );
						cache.layerEntries[task.layerIndex].progressTask = &cache.progressTaskCache; // start progress updates
					}

					// calculate mesh (slow operation), then check if still running
					cache.CalculateLayer( task, cache.progressTaskCache );
					{ // enter scope for lock guard
						LockWriteGuard lockCacheGuard( cache.lock );
						cache.progressTaskCache.ClearTask(); // stop progress object
						cache.layerEntries[task.layerIndex].progressTask = nullptr; // finish progress updates
						// copy the task results back into the cache, if still a match
						if( cache.IsLayerMatch( task.layerParams, task.layerIndex ) )
						{
							task.isReady = true;
							cache.layerEntries[task.layerIndex] = task; // set isReady and other flags in the actual cache
							cache.isStatsChanged = true; // request UI redraw
						}

						isThreadEnabled = cache.isThreadEnabled; // check for termination request again
						isEvalEnabled = cache.isEvalEnabled; // check for pause request again
					}
				}
			}

			// Generate atlas for each atlas task
			for( AtlasEntry& task : atlasTasks )
			{
				if( isThreadEnabled && isEvalEnabled )
				{
					ProgressTask progressTask(nullptr); // stub
					// calculate atlas (slow operation), then check if still running
					cache.CalculateAtlas( task, progressTask );
					{ // enter scope for lock guard
						LockWriteGuard lockCacheGuard( cache.lock );
						// copy the task results back into the cache, if still a match
						bool isReady = cache.IsAtlasMatch( task.atlasParams, task.atlasIndex ); // if atlas params match
						for( BoundsByLayerIndexMap::value_type item : task.layerBounds )
						{
							int layerIndex = item.first;
							if( !(cache.layerEntries[layerIndex].isReady) ) // if each layer param also valid
								isReady = false;
						}
						if( isReady )
						{
							task.isReady = true;
							cache.atlasEntries[task.atlasIndex] = task;
							cache.isStatsChanged = true; // request UI redraw
						}

						isThreadEnabled = cache.isThreadEnabled; // check for thread termination request again
						isEvalEnabled = cache.isEvalEnabled; // check for pause request again
					}
				}
			}

			{ // enter scope for lock guard
				LockWriteGuard lockCacheGuard( cache.lock );
				cache.isEvalRunning = false;
			}

			Sleep(16); // sixteen milliseconds between checks, about 60fps
		}
	}

// SCENE CACHE
#pragma endregion

}
