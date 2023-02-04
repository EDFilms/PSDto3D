//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file besierCurve.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef BEZIERCURVE_H
#define BEZIERCURVE_H

#include <vector>
#include "util/vectorialPath.h"
#include "../curve_mesh/bezier.h"

using namespace util;

namespace mesh_generator
{
	//----------------------------------------------------------------------------------------------
	class BezierCurve
	{
	public:
		BezierCurve();
		~BezierCurve();

		const std::vector<Vector2F*>& GetCurve() const { return this->Curves; };
		int GetCurveSize() const { return int(Curves.size()); };
		void GenerateBezierCurve(PathRecord const& refPoint);

	private:
		std::vector<Vector2F*> Curves;

		static Vector2F* CalculateBezierPoint(float const& t, Vector2F const& p0, Vector2F const& p1, Vector2F const& p2, Vector2F const& p3);
		Vector2F* CalculateBezierPoint(float const& t, PathPoints const& p0, PathPoints const& p1) const;
		void GenerateBezierClosedCurve(std::vector<PathPoints*> const& refPoint);
		void GenerateBezierOpenCurve(std::vector<PathPoints*> const& refPoint);
	};
}
#endif // BEZIERCURVE_H
