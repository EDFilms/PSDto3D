//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file curveData.cpp
//  @author Benjamin Drouin
//  @date 25-09-2018
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef CURVE_DATA_H
#define CURVE_DATA_H

#include "util/math_2D.h"
#include <vector>
#include "curve.h"
#include "curveNode.h"

using namespace util;

namespace mesh_generator
{
	struct PathIntersection
	{
		PathIntersection(Node* node, const float value)
		{
			this->Node = node;
			this->Value = value;
		};

		float GetValue() const { return this->Value; };
		Node* GetNode() const { return this->Node; };

	private:
		float Value;
		Node* Node;
	};

	struct CurveData
	{
		CurveData(Curve const& curve);
		void AddIntersection(PathIntersection const& intersection);
		void ConnectNodes();

		Curve GetCurve() const { return Curve; }

	private:
		Curve Curve;
		std::vector<PathIntersection> Intersections;

		static bool IntersectionSort(PathIntersection p1, PathIntersection p2) { return p1.GetValue() < p2.GetValue(); }
	};
}
#endif // CURVE_DATA_H