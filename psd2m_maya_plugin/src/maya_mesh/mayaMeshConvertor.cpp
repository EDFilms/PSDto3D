//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file MayaMeshConvertor.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "mayaMeshConvertor.h"
#include <maya/MDagModifier.h>
#include <maya/MGlobal.h>

namespace maya_plugin
{
#pragma region CONSTRUCTOR

	//----------------------------------------------------------------------------------------
	MayaMeshConvertor::MayaMeshConvertor(DataMesh const& dataMesh, float scale)
	{
		GenerateMayaMeshData(dataMesh, scale);
	}

	//----------------------------------------------------------------------------------------
	MayaMeshConvertor::~MayaMeshConvertor() = default;

#pragma endregion

#pragma region PUBLIC MAYA MESH ACCESS

	//----------------------------------------------------------------------------------------
	MObject MayaMeshConvertor::CreateMayaMFnMesh(MObject const& parent)
	{
		auto mesh = this->CurrentMesh.create(this->NumVertices,
			this->NumPolygons,
			this->VertexArray,
			this->PolygonCounts,
			this->PolygonConnects,
			parent);
		return mesh;
	}

	//----------------------------------------------------------------------------------------
	void MayaMeshConvertor::SetUvs()
	{
		// Get name
		MStringArray uvNames;
		MStatus status = this->CurrentMesh.getUVSetNames(uvNames);

		// Set position 0 -> 1
		status = this->CurrentMesh.setUVs(this->UArray, this->VArray, &uvNames[0]);
		if (status == MS::kFailure)
		{
			MGlobal::displayInfo("[Set UV]:  " + status.errorString());
		}
		else if (status == MS::kInvalidParameter)
		{
			MGlobal::displayInfo("[Set UV]:  " + status.errorString());
		}

		// Assign UV to polygon
		status = this->CurrentMesh.assignUVs(this->PolygonCounts, this->PolygonConnects, &uvNames[0]);
		if (status == MS::kFailure)
		{
			MGlobal::displayInfo("Assign uv: " + status.errorString());
		}
		else if (status == MS::kInvalidParameter)
		{
			MGlobal::displayInfo("Assign uv: " + status.errorString());
		}

	}

#pragma endregion

#pragma region BUILDER MFNMESH

	//----------------------------------------------------------------------------------------
	void MayaMeshConvertor::AddVertice(float x, float y, int index)
	{
		this->VertexArray[index].x = x;
		this->VertexArray[index].y = y;
		this->VertexArray[index].z = 1.0f;
		this->VertexArray[index].w = 1.0f;
	}

	//----------------------------------------------------------------------------------------
	void MayaMeshConvertor::AddUv(float x, float y, int index)
	{
		this->UArray[index] = x;
		this->VArray[index] = y;
	}

	//----------------------------------------------------------------------------------------
	float MayaMeshConvertor::TransformToMayaCoordinates(float y)
	{
		return 1.f - y;
	}

	//----------------------------------------------------------------------------------------
	void MayaMeshConvertor::GenerateMayaMeshData(DataMesh const& dataMesh, float scale)
	{
		// Vertex
		const int vertexCount = dataMesh.GetVerticesCount();
		this->NumVertices = vertexCount;
		this->VertexArray.setLength(vertexCount);
		// UV
		this->UArray.setLength(vertexCount);
		this->VArray.setLength(vertexCount);

		std::vector<Vector2F> vertices = dataMesh.GetVertices();
		int index = 0;
		for (auto it : vertices)
		{
			const float xPos = it.x;
			const float yPos = TransformToMayaCoordinates(it.y);
			AddVertice((xPos * this->BasedScaleFactor) * scale, (yPos * this->BasedScaleFactor) * scale, index);
			AddUv(xPos, yPos, index);
			index++;
		}

		// Poly
		const int polygonCount = int(dataMesh.GetFacesCountCount());
		this->NumPolygons = polygonCount;
		this->PolygonCounts.setLength(polygonCount);
		std::vector<int> facesCount = dataMesh.GetFacesCount();
		for (int i = 0; i < polygonCount; i++)
		{
			this->PolygonCounts[i] = facesCount[i];
		}

		// Connection
		const int connectionCount = int(dataMesh.GetFacesIndicesCount());
		this->PolygonConnects.setLength(connectionCount);
		std::vector<int> facesIndices = dataMesh.GetFacesIndices();
		for (int i = 0; i < connectionCount; i++)
		{
			this->PolygonConnects[i] = facesIndices[i];
		}
	}
	

#pragma endregion
}
