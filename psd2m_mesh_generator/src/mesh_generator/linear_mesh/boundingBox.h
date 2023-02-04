//----------------------------------------------------------------------------------------------
// ===============================================
//	Copyright (C) 2016, CDRIN.
//	All Rights Reserved.
// ===============================================
//	Unauthorized copying of this file, via any medium is strictly prohibited
//	Proprietary and confidential
//
//	@file BoundingBox.h
//	@author Fabien Raspail
//	@date 11-11-2016
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <vector>
#include "util/math_2D.h"
#include "util/vectorialPath.h"

using namespace util;

namespace mesh_generator
{
	//----------------------------------------------------------------------------------------------
	class boundingBox
	{
	public:
		Vector2F* BoundPoints();
		Vector2F TopLeftPoint();
		Vector2F TopRightPoint();
		Vector2F BottomLeftPoint();
		Vector2F BottomRightPoint();
		Vector2F GetCenter() { return Vector2F::Mid(TopLeftPoint(), BottomRightPoint()); }

		void GenerateBoundingBox(std::vector<Vector2F*> const& pathPoints);
		void DisplayBoundingBox() const;
		void SetOrientation(PathRecord const& refPoint);
		void SetOrientation(float const angle);
		void GenerateOrientedBoundingBox(std::vector<Vector2F*> const& pathPoints);

	private:
		Vector2F Points[4];
		Vector2F OrientedVector[2];
		Vector2F PosInitialPoint[2];

		static double Cross(Vector2F const& o, Vector2F const& a, Vector2F const& b);
		std::vector<Vector2F> ConvexHull(std::vector<Vector2F>& points);
		void GetDirection(std::vector<PathPoints*> const& refPoint);
		void GetDirection(float angle);
	};
}
#endif // BOUNDINGBOX_H
