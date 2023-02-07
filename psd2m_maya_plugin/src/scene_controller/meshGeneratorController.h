//----------------------------------------------------------------------------------------------
// ===============================================
//	Copyright (C) 2016, CDRIN.
//	All Rights Reserved.
// ===============================================
//	Unauthorized copying of this file, via any medium is strictly prohibited
//	Proprietary and confidential
//
//	@file meshGeneratorController.h
//	@author Fabien Raspail
//	@date 11-11-2016
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef MESHGENERATORCONTROLLER_H
#define MESHGENERATORCONTROLLER_H

#include "IPluginOutput.h"
#include "parameters.h"
#include "outputs.h"

#include "psd_reader/psdReader.h"
#include "mesh_generator/dataMesh.h"
#include "util/progressJob.h"
#include <map>
#if defined PSDTO3D_MAYA_VERSION
#include <maya/MObject.h>
#include <maya/MDagModifier.h>
#else
#include "mayaStub.h"
#endif
#include "qtProgress.h"

using namespace mesh_generator;
using namespace psd_reader;
using namespace psd_to_3d;
using util::ProgressTask;


namespace psd_to_3d
{
	class LayerParametersFilter; // forward declaration
	using util::ProgressJob; // forward declaration


	//----------------------------------------------------------------------------------------------
	class MeshGeneratorController
	{
	public:
		MeshGeneratorController( const PsdData& data, const SceneController& scene );
		~MeshGeneratorController();

		void GenerateMesh(DataSurface& mesh_out, const LayerParameters& layerParams, int layerIndex, ProgressTask& progressTask) const;
		void CreateTreeStructure(GroupByNameMap& tree_out, GraphLayerByIndexMap& meshes_in_out, LayerParametersFilter& filter ) const;
		void ApplyInfluenceLayer(DataMesh& mesh_in_out, int layerIndex, const InfluenceParameters& influenceParams) const;

	private:
		const PsdData& data;
		const SceneController& scene;
		DataSurface GenerateDataLinearMesh(const PsdData& data, const LayerParameters& layerParams, int layerIndex, ProgressTask& progressTask) const;
		DataSurface GenerateDataDelaunayMesh(const PsdData& data, const LayerParameters& layerParams, int layerIndex, ProgressTask& progressTask) const;
		DataSurface GenerateDataCurveGridMesh(const PsdData& data, const LayerParameters& layerParams, int layerIndex, ProgressTask& progressTask) const;
		DataSurface GenerateDataBillboardMesh(const PsdData& data, const LayerParameters& layerParams, int layerIndex, ProgressTask& progressTask) const;
	};

}


#endif // MESHGENERATORCONTROLLER_H