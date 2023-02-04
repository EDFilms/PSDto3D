//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file Mesh.h
//  @author Benjamin Drouin
//  @date 24-09-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef DATA_MESH_H
#define DATA_MESH_H

#include "util/math_2D.h"
#include <vector>

using namespace util;

namespace mesh_generator
{
	//----------------------------------------------------------------------------------------
	struct DataMesh
	{
		DataMesh(const std::string& name);
		DataMesh(const std::string& name, int estimatedVertices, int estimatedFaces);

		void SetVertices(std::vector<Vector2F> const& vertices);
		bool AddFace(std::vector<int> const& vertices);
		bool AddFace(int vertices[], int size);
		void SetValues(std::vector<Vector2F> const& vertices, std::vector<int> const& facesCount, std::vector<int> const&
		               facesIndices);

		void ClearFaces();
		void ClearVertices();

		int GetVerticesCount() const { return int(Vertices.size()); }
		int GetFacesCountCount() const { return int(FacesCount.size()); }
		int GetFacesIndicesCount() const { return int(FacesIndices.size()); }
		std::vector<Vector2F> GetVertices() const { return Vertices; }
		std::vector<int> GetFacesCount() const { return FacesCount; }
		std::vector<int> GetFacesIndices() const { return FacesIndices; }
		std::string GetName() const;

	private:
		std::string NameId;

		std::vector<Vector2F> Vertices;
		std::vector<int> FacesCount;
		std::vector<int> FacesIndices;
	};
}

#endif // DATA_MESH_H