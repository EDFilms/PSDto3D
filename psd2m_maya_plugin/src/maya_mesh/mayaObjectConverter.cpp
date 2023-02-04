//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file MayaMesh.cpp
//  @author Benjamin Drouin
//  @date 25-09-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include <maya/MObject.h>
#include <maya/MFnMesh.h>
#include "mayaObjectConverter.h"

namespace maya_plugin
{
	MObject MayaObjectConverter::Convert(DataMesh* mesh, MObject const& parent)
	{
		std::vector<Vector2F*>* vertices = mesh->GetVertices();
		std::vector<int> facesCount = mesh->GetFacesCount();
		std::vector<int> facesIndices = mesh->GetFacesIndices();

		MFnMesh fnMesh;

		MObject obj = fnMesh.create(
			int(vertices->size()),
			int(facesCount.size()),
			*Vector2FVectorToMayaArray(*vertices),
			*IntVectorToMayaArray(facesCount),
			*IntVectorToMayaArray(facesIndices),
			parent
		);

		BuildUVs();
		return obj;
	}

	MFloatPointArray* MayaObjectConverter::Vector2FVectorToMayaArray(std::vector<Vector2F*> vectors)
	{
		MFloatPointArray* array = new MFloatPointArray();

		for (Vector2F* vertex : vectors)
		{
			MFloatPoint* point = new MFloatPoint();
			point->x = vertex->x;
			point->y = vertex->y;
			point->z = 1.0f;
			point->w = 1.0f;

			array->append(*point);
		}

		return array;
	}

	MIntArray* MayaObjectConverter::IntVectorToMayaArray(std::vector<int> ints)
	{
		MIntArray* array = new MIntArray();

		for (int count : ints)
		{
			array->append(count);
		}

		return array;
	}

	void MayaObjectConverter::BuildUVs()
	{

	}
}
