// Algorithm copied from : https://github.com/erich666/GraphicsGems/tree/master/gemsiv/curve_isect
// Slightly modified to fit with Vector2F instead of their "Point" class.

#ifndef _BEZIER_INCLUDED_
#define _BEZIER_INCLUDED_

#include "util/math_2D.h"
#include <vector>

using namespace util;

struct BezierIntersection
{
	float CurveAPercentage;
	float CurveBPercentage;
	Vector2F IntersectionPoint;

	static BezierIntersection Default;
};

class Bezier {
public:
	Vector2F P0, P1, P2, P3;
	Bezier() = default;
	~Bezier() = default;

	Bezier(Vector2F const& p0, Vector2F const& p1, Vector2F const& p2, Vector2F const& p3)
	{
		P0 = p0; P1 = p1; P2 = p2; P3 = p3;
	}

	// Bezier Intersection
	std::vector<BezierIntersection> FindAllIntersections(Bezier const& bezier, bool allowBoundaries);

	// Point Intersection
	bool PointBelongsToCurve(Vector2F const& p) const;

	// Linear Intersection
	BezierIntersection FindLinearIntersection(Bezier const& bezier, bool allowBoundaries) const;

private:
	Bezier * Split() const;
	int GetCurveDepth() const;

	// Bezier Intersection
	static int IntersectBB(Bezier const& a, Bezier const& b, bool allowBoundaries);
	static void RecursivelyIntersect(Bezier const& a, double t0, double t1, int depthA, Bezier const& b, double u0, double u1, int depthB,
		std::vector<BezierIntersection>& intersections, bool allowBoundaries);
	void FindIntersections(Bezier const& a, Bezier const& b, std::vector<BezierIntersection>& intersections, bool allowBoundaries) const;

	// Point Intersection
	static bool IsPointInBoundingBox(Vector2F const& p, Bezier a);
	static bool RecursivePointSearch(Vector2F const& p, Bezier const& bez, int depth);

	// Linear Intersection
	static bool IsIntersecting(Vector2F const& p1, Vector2F const& p2, Vector2F const& q1, Vector2F const& q2);
	static bool Compare(Vector2F const& a, Vector2F const& b);
};
#endif