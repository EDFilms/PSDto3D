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
		void Free();

		void CloneFrom( const BezierCurve& that );

		const std::vector<PathPoints>& GetPaths() const { return this->Paths; }
		const std::vector<Vector2F*>& GetCurves() const { return this->Curves; };
		PathPoints GetPathPoints(int index) const { return this->Paths[index]; };
		void SetPathPoints(int index, const PathPoints& points ) { this->Paths[index]=points; }
		Vector2F GetPoint(int index) const { return *(this->Curves[index]); };
		int GetPathSize() const { return int(Paths.size()); }; // TODO: rename to GetPathCount()
		int GetCurveSize() const { return int(Curves.size()); }; // TODO: rename to GetPointCount()
		void GenerateBezierCurve(PathRecord const& refPoint, Vector2F clampMin, Vector2F clampMax, bool createCurves=true);
		void ClampCurvePoints( Vector2F min, Vector2F max );

	private:
		std::vector<PathPoints> Paths; // original low-resolution control points
		std::vector<Vector2F*> Curves; // generated high-resolution points on curve

		static Vector2F* CalculateBezierPoint(float const& t, Vector2F const& p0, Vector2F const& p1, Vector2F const& p2, Vector2F const& p3);
		Vector2F* CalculateBezierPoint(float const& t, PathPoints const& p0, PathPoints const& p1) const;
		void GenerateBezierClosedCurve(std::vector<PathPoints*> const& refPoint, bool createCurves);
		void GenerateBezierOpenCurve(std::vector<PathPoints*> const& refPoint, bool createCurves);
	};
}
#endif // BEZIERCURVE_H
