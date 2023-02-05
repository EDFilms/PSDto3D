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

#include "interface/toolWidget.h"
#include "psd_reader/psdReader.h"
#include "mesh_generator/dataMesh.h"
#include <map>
#include <maya/MObject.h>
#include <maya/MDagModifier.h>
#include "qtProgress.h"

using namespace mesh_generator;
using namespace psd_reader;

namespace maya_plugin
{
	//----------------------------------------------------------------------------------------------
	struct GroupLayer
	{
		MObject Transform;
		std::vector<std::string> LayerNames;
	};

	//----------------------------------------------------------------------------------------------
	class MeshGeneratorController
	{
	public:
		MeshGeneratorController();
		~MeshGeneratorController();

		static void GenerateMayaMeshes(PsdData& data, GlobalParameters& params, Progress& progress);

	private:
		static void InitializeProgressBar(PsdData& data, GlobalParameters& params, Progress& progress);

		static void CreateEditorMayaComponents(GlobalParameters& params, Progress& progress, std::map<std::string, GroupLayer>& tree, std::map<std::string, DataMesh>& meshes);
		static std::map<std::string, GroupLayer> CreateTreeStructure(GlobalParameters const& params, psd_reader::PsdData const& data);
		static void CreateShapeEditorComponent(MDagModifier& dag, GlobalParameters const& params, DataMesh const& mesh, float, MObject & transformParent);
		static void UpdateShapeEditorComponent(MObject& mFnMesh, MDagModifier& dag, GlobalParameters const& params,
		                                DataMesh const& mesh, float);

		static DataMesh GenerateDataLinearMesh(ResourceBlockPath const& resourceBlockPath, LayerParameters const* params);
		static DataMesh GenerateDataCurveGridMesh(LayerData const& layer, LayerParameters const* params);

		static void ApplyInfluenceLayer(PsdData const& data, std::map<std::string, DataMesh>& meshes, GlobalParameters& params, Progress& progress);
	};
}
#endif // MESHGENERATORCONTROLLER_H
