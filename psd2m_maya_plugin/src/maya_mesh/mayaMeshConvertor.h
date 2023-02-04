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

#include "mesh_generator/linear_mesh/linearMesh.h"
#include <maya/MObject.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MFnMesh.h>

using namespace mesh_generator;

namespace maya_plugin
{
	class MayaMeshConvertor
	{
	public:
		MayaMeshConvertor(DataMesh const& dataMesh, float scale);
		~MayaMeshConvertor();
		//----------------------------------------------------------------------------------------
		MObject CreateMayaMFnMesh(MObject const& parent);
		void SetUvs();

	private:
		int const BasedScaleFactor = 15;
		MFnMesh CurrentMesh;

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
		static float TransformToMayaCoordinates(float y);
		//bool AddPolygon(int const & indexPoly, mesh_generator::MeshPoly * meshData);
		void GenerateMayaMeshData(DataMesh const& dataMesh, float scale);
		
	};
}
#endif // MAYAMESH_H
