//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file BezierCurve.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "bezierCurve.h"

namespace mesh_generator
{
#pragma region PUBLIC CURVE ACCESS

	//----------------------------------------------------------------------------------------
	BezierCurve::BezierCurve() = default;

	//----------------------------------------------------------------------------------------
	BezierCurve::~BezierCurve()
	{
		if (this->Curves.empty()) return;

		for (auto& Curve : this->Curves)
		{
			delete Curve;
		}
		this->Curves.clear();
	}

	//----------------------------------------------------------------------------------------
	void BezierCurve::GenerateBezierCurve(PathRecord const& refPoint)
	{
		if (!Curves.empty()) Curves.clear();

		if (refPoint.IsClosedPath && refPoint.Points.size() > 2)
		{
			GenerateBezierClosedCurve(refPoint.Points);
		}
		else
		{
			GenerateBezierOpenCurve(refPoint.Points);
		}
	}

#pragma endregion

#pragma region PRIVATE GENERATION

	//----------------------------------------------------------------------------------------
	void BezierCurve::GenerateBezierClosedCurve(std::vector<PathPoints*> const& refPoint)
	{
		unsigned int size = int(refPoint.size());
		GenerateBezierOpenCurve(refPoint);

		// Connect the last point and the first point
		for (float t = 0.0f; t <= 1.0f; )
		{
			Vector2F* pointCurve = BezierCurve::CalculateBezierPoint(t, *refPoint[size - 1], *refPoint[0]);
			this->Curves.push_back(pointCurve);
			t += 0.001f;
		}
	}

	//----------------------------------------------------------------------------------------
	void BezierCurve::GenerateBezierOpenCurve(std::vector<PathPoints*> const& refPoint)
	{
		unsigned int size = int(refPoint.size());
		// Not a path just one isolate point.
		if (size <= 2) return;

		// All the Points
		for (unsigned int i = 0; i < size - 1; ++i)
		{
			for (float t = 0.0f; t <= 1.0f; )
			{
				Vector2F* pointCurve = BezierCurve::CalculateBezierPoint(t, *refPoint[i], *refPoint[i + 1]);
				this->Curves.push_back(pointCurve);
				t += 0.005f;
			}
		}
	}

#pragma endregion

#pragma region POINTS

	//----------------------------------------------------------------------------------------
	// Implementation of this formule => [x,y]= ((1-t)^3 * p0 )+ 3 * (1-t)^2 * t * p1 ) + (3 * (1-t) * t^2 * p2) + (t^3 * p3)
	//----------------------------------------------------------------------------------------
	Vector2F* BezierCurve::CalculateBezierPoint(float const& t, Vector2F const& p0, Vector2F const& p1, Vector2F const& p2, Vector2F const& p3)
	{
		float u = 1.0f - t;
		float tt = t * t;
		float uu = u * u;
		float uuu = uu * u;
		float ttt = tt * t;

		Vector2F* p = new Vector2F();
		*p = p0 * uuu;					//first term (1-t)^3 * p0
		*p += p1 * 3 * uu * t;			//second term 3(1-t)^2 * t*p1 
		*p += p2 * 3 * u * tt;			//third term (3(1-t) * t^2 * p2)
		*p += p3 * ttt;					//fourth term (t^3 * p3)
		return p;
	}

	//----------------------------------------------------------------------------------------
	Vector2F* BezierCurve::CalculateBezierPoint(float const& t, PathPoints const& p0, PathPoints const& p1) const
	{
		return BezierCurve::CalculateBezierPoint(t, p0.AnchorPoint, p0.SegOut, p1.SegIn, p1.AnchorPoint);
	}

#pragma endregion
}
