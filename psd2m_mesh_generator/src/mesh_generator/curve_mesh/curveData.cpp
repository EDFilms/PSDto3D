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
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "curveData.h"
#include "curve.h"
#include <queue>

namespace mesh_generator
{
	CurveData::CurveData(mesh_generator::Curve const& curve) { this->Curve = curve; }

	void CurveData::AddIntersection(PathIntersection const& intersection)
	{
		this->Intersections.push_back(intersection);
	};

	void CurveData::ConnectNodes()
	{
		std::sort(this->Intersections.begin(), this->Intersections.end(), IntersectionSort);

		if (this->Intersections.empty())
			return;

		for (auto it = this->Intersections.cbegin(), end = this->Intersections.cend() - 1; it != end; ++it)
		{
			auto current = *it;
			auto next = *(it + 1);

			current.GetNode()->AddNeighbour(next.GetNode());
			next.GetNode()->AddNeighbour(current.GetNode());
		}

		const unsigned size = (unsigned int)this->Intersections.size();
		if (this->Curve.IsClosedPath && size > 1)
		{
			auto first = *this->Intersections.begin();
			auto last = *(this->Intersections.end() - 1);

			first.GetNode()->AddNeighbour(last.GetNode());
			last.GetNode()->AddNeighbour(first.GetNode());
		}
	};
}
