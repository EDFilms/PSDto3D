//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file Mesh.cpp
//  @author Benjamin Drouin
//  @date 24-09-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "dataMesh.h"

namespace mesh_generator
{
	//----------------------------------------------------------------------------------------------
	DataMesh::DataMesh(std::string const& name)
	{
		this->NameId = name;
	}

	//----------------------------------------------------------------------------------------------
	DataMesh::DataMesh(std::string const& name, int const estimatedVertices, int const estimatedFaces)
	{
		this->NameId = name;
		this->Vertices.reserve(estimatedVertices);
		this->FacesCount.reserve(estimatedFaces);
		this->FacesIndices.reserve(estimatedFaces);
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::SetVertices(std::vector<Vector2F> const& vertices)
	{
		this->Vertices = vertices;
	}

	//----------------------------------------------------------------------------------------------
	bool DataMesh::AddFace(std::vector<int> const& vertices)
	{
		if (vertices.empty()) return false;

		this->FacesCount.push_back(int(vertices.size()));
		this->FacesIndices.insert(this->FacesIndices.end(), vertices.begin(), vertices.end());
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool DataMesh::AddFace(int vertices[], int const size)
	{

		this->FacesCount.push_back(size);
		for (int i = 0; i < size; ++i)
		{
			this->FacesIndices.push_back(vertices[i]);
		}
		return true;
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::SetValues(std::vector<Vector2F> const& vertices, std::vector<int> const& facesCount, std::vector<int> const& facesIndices)
	{
		this->Vertices = vertices;
		this->FacesCount = facesCount;
		this->FacesIndices = facesIndices;
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::ClearFaces()
	{
		this->FacesCount.clear();
		this->FacesIndices.clear();
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::ClearVertices()
	{
		this->Vertices.clear();
		this->FacesCount.clear();
		this->FacesIndices.clear();
	}

	//----------------------------------------------------------------------------------------------
	std::string DataMesh::GetName() const
	{
		return this->NameId;
	}
}