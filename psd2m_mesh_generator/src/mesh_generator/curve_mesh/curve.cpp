//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file Curve.cpp
//  @author Benjamin Drouin
//  @date 01-10-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "curve.h"

namespace mesh_generator
{
#pragma region PUBLIC CURVE ACCESS

	//----------------------------------------------------------------------------------------
	Curve::Curve(PathRecord const& refPoint)
	{
		this->PathPointsCurve = refPoint.Points;
		this->IsClosedPath = refPoint.IsClosedPath;
		// Create all Bezier Segments
		for (auto it = this->PathPointsCurve.cbegin(), end = this->PathPointsCurve.cend() - 1; it != end; ++it)
		{
			auto current = *it;
			auto next = *(it + 1);
			Bezier bez = Bezier(current->AnchorPoint, current->SegOut, next->SegIn, next->AnchorPoint);
			this->PathBezier.push_back(bez);
		}

		// If loop, create last segment
		if (this->IsClosedPath)
		{
			auto current = *(this->PathPointsCurve.end() - 1);
			auto next = *this->PathPointsCurve.begin();
			Bezier bez = Bezier(current->AnchorPoint, current->SegOut, next->SegIn, next->AnchorPoint);
			this->PathBezier.push_back(bez);
		}
	}
#pragma endregion
}
