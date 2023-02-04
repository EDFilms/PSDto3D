//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file influenceMesh.cpp
//  @author Benjamin Drouin
//  @date 12-10-2018
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef INFLUENCE_MESH_GENERATOR_H
#define INFLUENCE_MESH_GENERATOR_H

#include <vector>
#include "util/math_2D.h"
#include "mesh_generator/dataMesh.h"
#include "meshFace.h"
#include <queue>

using namespace util;

namespace mesh_generator
{
	struct InfluenceParameters
	{
		float MinPolygonSize = 0.02f;
		float MaxPolygonSize = 0.25f;
	};

	//----------------------------------------------------------------------------------------------
	class InfluenceMesh
	{
	public:
		static void SubdivideFaces(::mesh_generator::DataMesh& mesh, MaskData const& influenceLayer, InfluenceParameters const& params);

	private:
		static void UpdateFaceWithDividedEdges(MeshFace& face, std::vector<SplitEdge>& edges);
		static std::vector<SplitEdge>::const_iterator FindEdge(std::vector<SplitEdge>& edges, int vertexA, int vertexB);
		static void ReturnCompletedFaceIfSplit(SplitEdge& edge, std::vector<MeshFace>& completedFaces, std::queue<MeshFace>& toProcess);
	};
}
#endif // INFLUENCE_MESH_GENERATOR_H