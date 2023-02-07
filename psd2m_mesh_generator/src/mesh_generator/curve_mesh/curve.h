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

#ifndef CURVE_H
#define CURVE_H

#include <vector>
#include "util/bounds_2D.h"
#include "util/vectorialPath.h"
#include "bezier.h"

using namespace util;

namespace mesh_generator
{
	//----------------------------------------------------------------------------------------------
	class Curve
	{
	public:
		Curve() = default;
		Curve(PathRecord const& refPoint);

		bool IsClosedPath;
		const std::vector<Bezier>& GetBezierCurve() const { return this->PathBezier; }
		std::vector<PathPoints*> GetPathPoints() const { return this->PathPointsCurve; }

	private:
		std::vector<PathPoints*> PathPointsCurve;
		std::vector<Bezier> PathBezier;
	};
}
#endif // CURVE_H
