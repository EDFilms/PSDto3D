//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file curveMeshGenerator.cpp
//  @author Benjamin Drouin
//  @date 25-09-2018
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef CURVE_MESH_GENERATOR_H
#define CURVE_MESH_GENERATOR_H

#include "util/math_2D.h"
#include <vector>
#include "curve.h"
#include "curveData.h"
#include "../dataMesh.h"

using namespace util;

namespace mesh_generator
{
	struct CurveParameters
	{
		float MergeVertexDistance = 0.01f;
	};

	//----------------------------------------------------------------------------------------------
	class CurveMeshGenerator
	{
	public:
		static DataMesh GenerateMesh(std::vector<Curve> curves, std::string const& name, CurveParameters const& params);

	private:
		static bool SortNodesBasedOnX(Node* a, Node* b) { return a->GetVertex().x < b->GetVertex().x; }

		static void FindIntersections(CurveData& pathA, CurveData& pathB, std::vector<Node*>& nodes);
		static bool IsInsideClosedCurve(std::vector<CurveData> const& paths, Node* node);
		static bool ValidateFace(Node* node, std::vector<int> const& face);
		static void MinimalCycleSearch(std::vector<Node*> const& nodes, DataMesh& mesh);
		static void CompleteFace(Node* first, Node* second, std::vector<int>& vertices);
	};
}
#endif // CURVE_MESH_GENERATOR_H