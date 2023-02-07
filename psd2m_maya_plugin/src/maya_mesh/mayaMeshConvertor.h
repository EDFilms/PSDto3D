//----------------------------------------------------------------------------------------------
// ===============================================
//	Copyright (C) 2016, CDRIN.
//	All Rights Reserved.
// ===============================================
//	Unauthorized copying of this file, via any medium is strictly prohibited
//	Proprietary and confidential
//
//	@file MayaMeshConvertor.h
//	@author Fabien Raspail
//	@date 11-11-2016
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef MAYAMESH_H
#define MAYAMESH_H

#include "IPluginOutput.h"
#include "parameters.h"
#include "scene_controller/sceneController.h"
#include "mesh_generator/linear_mesh/linearMesh.h"
#if defined PSDTO3D_MAYA_VERSION
#include <maya/MObject.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>
//#include <maya/MFnNurbsCurve.h>
#include <maya/MFnTransform.h>
#include <maya/MDagModifier.h>
#else
#include "mayaStub.h"
#endif
#include <set>

using namespace mesh_generator;

namespace maya_plugin
{
	using psd_to_3d::GraphLayer;

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Produces a Maya mesh MObject output, from a DataSurface input which is either DataMesh or DataSpline
	// Creates a real mesh using DataMesh (via internal geometry algorithm) ... OR ...
	// Creates a scripted mesh using DataSpline (via Maya surface and tessellation operators)
	//
	//   DataSurface -> DataMesh or DataSpline
	//
	//   MayaMeshGenerator -> DataSurface, generates MayaMeshConvertor or MayaMeshScriptor
	//     MayaMeshConvertor -> DataMesh, creates real mesh
	//     MayaMeshScriptor  -> DataSpline, creates scripted mesh
	//
	class MayaMeshGenerator
	{
	public:
		MayaMeshGenerator();
		// Copy constructor transfers ownership of pointers, no reference counting
		// TODO: Add reference counting if arbitrary copies need to be made
		MayaMeshGenerator( MayaMeshGenerator& that );  // override copy constructor for ownership handling
		~MayaMeshGenerator();
		//----------------------------------------------------------------------------------------
		// Creates a MayaMeshConvertor from DataMesh, if available in DataSurface (preferred)
		// Creates a MayaMeshScriptor from DataSpline, if available in DataSurface (alternate)
		static MayaMeshGenerator CreateMayaMeshGenerator(MDagModifier& dag, MObject& parent, const GraphLayer& graphLayer);
		virtual MObject GetMayaMObject();
		virtual void SetUvs();
	protected:
		MayaMeshGenerator* Impl;

		static float TransformToMayaCoordinates(float y);
		static const int BasedScaleFactor = 15;
	};

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Produces a Maya mesh MObject output, from a DataSpline input
	// Scripts the creation of a mesh using the curves inputs of the DataSpline
	class MayaMeshScriptor : public MayaMeshGenerator
	{
	public:
		//----------------------------------------------------------------------------------------
		// Creates a Maya scripted object from a DataSpline, resolves later with dag.doIt()
		MObject CreateMayaMFnMesh(MDagModifier& dag, MObject& parent, const GraphLayer& graphLayer);
		virtual MObject GetMayaMObject();
		virtual void SetUvs();

	private:
		MFnMesh CurrentMesh;
		MObject CurrentObject;
	};

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Produces a Maya mesh MObject output, from a DataMesh input
	// Generates the mesh geometry directly,
	// assumes an internal geometry algorithm was used to create the DataMesh
	class MayaMeshConvertor : public MayaMeshGenerator
	{
	public:
		MayaMeshConvertor(const GraphLayer& graphLayer);
		~MayaMeshConvertor();
		//----------------------------------------------------------------------------------------
		// Creates a Maya mesh from a DataMesh, resolves immediately
		MObject CreateMayaMFnMesh(MObject& parent);
		MObject GetMayaMObject();
		void SetUvs();

	private:
		MFnMesh CurrentMesh;
		MObject CurrentObject;

		int NumVertices = 0;
		int NumPolygons = 0;
		MFloatPointArray VertexArray;
		MIntArray PolygonCounts;
		MIntArray PolygonConnects;

		MFloatArray UArray;
		MFloatArray VArray;

		//----------------------------------------------------------------------------------------
		void AddVertice(float x, float y, int index);
		void AddUv(float x, float y, int index);
		//bool AddPolygon(int const & indexPoly, mesh_generator::MeshPoly * meshData);
		void GenerateMayaMeshData(const GraphLayer& graphLayer);
		
	};


	class MayaPluginOutput : public psd_to_3d::IPluginOutput
	{
		// TODO: Avoid dependecy on these extra types
		typedef psd_reader::PsdData PsdData;
		typedef mesh_generator::DataSurface DataSurface;
		typedef psd_to_3d::GroupByNameMap GroupByNameMap;
		typedef psd_to_3d::GraphLayerGroup GraphLayerGroup;
		typedef psd_to_3d::GlobalParameters GlobalParameters;

		void BeginSession( const IPluginOutputParameters& params );
		void OutputMesh( const PsdData& psdData, const IPluginOutputParameters& params, const DataSurface& dataSurface, int layerIndex );
		void OutputTree( const PsdData& psdData, const IPluginOutputParameters& params, const GroupByNameMap& tree );
		void EndSession( const PsdData& data, const IPluginOutputParameters& params ); // TODO: avoid need to pass PsdData and GlobalParameters
		void CancelSession( const PsdData& psdData, const IPluginOutputParameters& params );
		void GetSaveDialogParams( void*, const IPluginOutputParameters& ) {} // not used for plugin, only for standalone

	private:
		std::string psdPath, psdName;
		std::map< std::string, std::string > materialNameRemap;

		void CreateEditorMayaComponents( const GroupByNameMap& tree );
		void CreateMayaTreeStructure( const psd_reader::PsdData& data, const GroupByNameMap& tree );
		void CreateShapeEditorMaterial(  MDagModifier& dag, const GraphLayer& graphLayer,
										 MObject& lambert_out );
		void CreateShapeEditorComponent( MDagModifier& dag, const GraphLayer& graphLayer,
										 MObject& lambert, MObject& transformParent, bool createMaterial );
		void UpdateShapeEditorComponent( MObject& mFnMesh, MDagModifier& dag,  const GraphLayer& graphLayer,
										 MObject& lambert );
	};

} // namespace maya_mesh


#endif // MAYAMESH_H
