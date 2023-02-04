#include <cstddef> // for size_t
// #include <sys/stdtypes.h> // for size_t, on some systems
#define _USE_MATH_DEFINES
#include <cmath>
#include "bezier.h"
#include <vector>
/* The value of 1.0 / (1L<<23) is float machine epsilon. */
#ifdef FLOAT_ACCURACY
#define INV_EPS (1L<<23)
#else
/* The value of 1.0 / (1L<<14) is enough for most applications */
#define INV_EPS (1L<<14)
#endif
#define log2(x) (log(x)/log(2.))

BezierIntersection BezierIntersection::Default = {};

/*
 * Split the curve at the midpoint, returning an array with the two parts
 * Temporary storage is minimized by using part of the storage for the result
 * to hold an intermediate value until it is no longer needed.
 */
#define left r[0]
#define right r[1]
Bezier *Bezier::Split() const
{
	Bezier* r = new Bezier[2];
	left.P0 = P0;
	right.P3 = P3;
	left.P1 = Vector2F::Mid(P0, P1);
	right.P2 = Vector2F::Mid(P2, P3);
	right.P1 = Vector2F::Mid(P1, P2); // temporary holding spot
	left.P2 = Vector2F::Mid(left.P1, right.P1);
	right.P1 = Vector2F::Mid(right.P1, right.P2); // Real value this time
	left.P3 = right.P0 = Vector2F::Mid(left.P2, right.P1);
	return r;
}
#undef left
#undef right


/*
* Test the bounding boxes of two Bezier curves for interference.
* Several observations:
*	First, it is cheaper to compute the bounding box of the second curve
*	and test its bounding box for interference than to use a more direct
*	approach of comparing all control points of the second curve with
*	the various edges of the bounding box of the first curve to test
* 	for interference.
*	Second, after a few subdivisions it is highly probable that two corners
*	of the bounding box of a given Bezier curve are the first and last
*	control point.  Once this happens once, it happens for all subsequent
*	subcurves.  It might be worth putting in a test and then short-circuit
*	code for further subdivision levels.
*	Third, in the final comparison (the interference test) the comparisons
*	should both permit equality.  We want to find intersections even if they
*	occur at the ends of segments.
*	Finally, there are tighter bounding boxes that can be derived. It isn't
*	clear whether the higher probability of rejection (and hence fewer
*	subdivisions and tests) is worth the extra work.
*/
int Bezier::IntersectBB(Bezier const& a, Bezier const& b, bool allowBoundaries)
{
	// Compute bounding box for a
	double minax, maxax, minay, maxay;
	if (a.P0.x > a.P3.x)	 // These are the most likely to be extremal
		minax = a.P3.x, maxax = a.P0.x;
	else
		minax = a.P0.x, maxax = a.P3.x;
	if (a.P2.x < minax)
		minax = a.P2.x;
	else if (a.P2.x > maxax)
		maxax = a.P2.x;
	if (a.P1.x < minax)
		minax = a.P1.x;
	else if (a.P1.x > maxax)
		maxax = a.P1.x;
	if (a.P0.y > a.P3.y)
		minay = a.P3.y, maxay = a.P0.y;
	else
		minay = a.P0.y, maxay = a.P3.y;
	if (a.P2.y < minay)
		minay = a.P2.y;
	else if (a.P2.y > maxay)
		maxay = a.P2.y;
	if (a.P1.y < minay)
		minay = a.P1.y;
	else if (a.P1.y > maxay)
		maxay = a.P1.y;
	// Compute bounding box for b
	double minbx, maxbx, minby, maxby;
	if (b.P0.x > b.P3.x)
		minbx = b.P3.x, maxbx = b.P0.x;
	else
		minbx = b.P0.x, maxbx = b.P3.x;
	if (b.P2.x < minbx)
		minbx = b.P2.x;
	else if (b.P2.x > maxbx)
		maxbx = b.P2.x;
	if (b.P1.x < minbx)
		minbx = b.P1.x;
	else if (b.P1.x > maxbx)
		maxbx = b.P1.x;
	if (b.P0.y > b.P3.y)
		minby = b.P3.y, maxby = b.P0.y;
	else
		minby = b.P0.y, maxby = b.P3.y;
	if (b.P2.y < minby)
		minby = b.P2.y;
	else if (b.P2.y > maxby)
		maxby = b.P2.y;
	if (b.P1.y < minby)
		minby = b.P1.y;
	else if (b.P1.y > maxby)
		maxby = b.P1.y;
	// Test bounding box of b against bounding box of a
	if (allowBoundaries)
	{
		if ((minax > maxbx) || (minay > maxby)
			|| (minbx > maxax) || (minby > maxay))
		{
			return 0; // they don't intersect
		}
	}
	else
	{
		if ((minax >= maxbx) || (minay >= maxby)
			|| (minbx >= maxax) || (minby >= maxay))
		{
			return 0; // they don't intersect
		}
	}

	return 1; // they intersect
}

/*
* Recursively intersect two curves keeping track of their real parameters
* and depths of intersection.
* The results are returned in a 2-D array of doubles indicating the parameters
* for which intersections are found.  The parameters are in the order the
* intersections were found, which is probably not in sorted order.
* When an intersection is found, the parameter value for each of the two
* is stored in the index elements array, and the index is incremented.
*
* If either of the curves has subdivisions left before it is straight
*	(depth > 0)
* that curve (possibly both) is (are) subdivided at its (their) midpoint(s).
* the depth(s) is (are) decremented, and the parameter value(s) corresponding
* to the midpoints(s) is (are) computed.
* Then each of the subcurves of one curve is intersected with each of the
* subcurves of the other curve, first by testing the bounding boxes for
* interference.  If there is any bounding box interference, the corresponding
* subcurves are recursively intersected.
*
* If neither curve has subdivisions left, the line segments from the first
* to last control point of each segment are intersected.  (Actually the
* only the parameter value corresponding to the intersection point is found).
*
* The apriori flatness test is probably more efficient than testing at each
* level of recursion, although a test after three or four levels would
* probably be worthwhile, since many curves become flat faster than their
* asymptotic rate for the first few levels of recursion.
*
* The bounding box test fails much more frequently than it succeeds, providing
* substantial pruning of the search space.
*
* Each (sub)curve is subdivided only once, hence it is not possible that for
* one final line intersection test the subdivision was at one level, while
* for another final line intersection test the subdivision (of the same curve)
* was at another.  Since the line segments share endpoints, the intersection
* is robust: a near-tangential intersection will yield zero or two
* intersections.
*/
void Bezier::RecursivelyIntersect(Bezier const& a, double t0, double t1, int depthA,
	Bezier const& b, double u0, double u1, int depthB,
	std::vector<BezierIntersection> & intersections, bool allowBoundaries)
{
	if (depthA > 0)
	{
		Bezier *A = a.Split();
		double tmid = (t0 + t1)*0.5;
		depthA--;
		if (depthB > 0)
		{
			Bezier *B = b.Split();
			double umid = (u0 + u1)*0.5;
			depthB--;
			if (IntersectBB(A[0], B[0], allowBoundaries))
				RecursivelyIntersect(A[0], t0, tmid, depthA,
					B[0], u0, umid, depthB,
					intersections, allowBoundaries);
			if (IntersectBB(A[1], B[0], allowBoundaries))
				RecursivelyIntersect(A[1], tmid, t1, depthA,
					B[0], u0, umid, depthB,
					intersections, allowBoundaries);
			if (IntersectBB(A[0], B[1], allowBoundaries))
				RecursivelyIntersect(A[0], t0, tmid, depthA,
					B[1], umid, u1, depthB,
					intersections, allowBoundaries);
			if (IntersectBB(A[1], B[1], allowBoundaries))
				RecursivelyIntersect(A[1], tmid, t1, depthA,
					B[1], umid, u1, depthB,
					intersections, allowBoundaries);
		}
		else
		{
			if (IntersectBB(A[0], b, allowBoundaries))
				RecursivelyIntersect(A[0], t0, tmid, depthA,
					b, u0, u1, depthB,
					intersections, allowBoundaries);
			if (IntersectBB(A[1], b, allowBoundaries))
				RecursivelyIntersect(A[1], tmid, t1, depthA,
					b, u0, u1, depthB,
					intersections, allowBoundaries);
		}
	}
	else
	{
		if (depthB > 0)
		{
			Bezier *B = b.Split();
			double umid = (u0 + u1)*0.5;
			depthB--;
			if (IntersectBB(a, B[0], allowBoundaries))
				RecursivelyIntersect(a, t0, t1, depthA,
					B[0], u0, umid, depthB,
					intersections, allowBoundaries);
			if (IntersectBB(a, B[1], allowBoundaries))
				RecursivelyIntersect(a, t0, t1, depthA,
					B[1], umid, u1, depthB,
					intersections, allowBoundaries);
		}
		else // Both segments are fully subdivided; now do line segments
		{
			double xlk = a.P3.x - a.P0.x;
			double ylk = a.P3.y - a.P0.y;
			double xnm = b.P3.x - b.P0.x;
			double ynm = b.P3.y - b.P0.y;
			double xmk = b.P0.x - a.P0.x;
			double ymk = b.P0.y - a.P0.y;
			double det = xnm * ylk - ynm * xlk;
			if (1.0 + det == 1.0)
				return;

			double detinv = 1.0 / det;
			double s = (xnm * ymk - ynm * xmk) * detinv;
			double t = (xlk * ymk - ylk * xmk) * detinv;
			if ((s < 0.0) || (s > 1.0) || (t < 0.0) || (t > 1.0))
				return;

			// We'll calculate inter.intersectionPoint later in "Bezier::FindAllIntersections" because we don't have all the info here
			BezierIntersection inter;
			inter.CurveAPercentage = float(t0 + s * (t1 - t0));
			inter.CurveBPercentage = float(u0 + t * (u1 - u0));
			inter.IntersectionPoint = Vector2F(
				float(a.P0.x + (a.P3.x - a.P0.x) * s),
				float(a.P0.y + (a.P3.y - a.P0.y) * s)
			);

			intersections.push_back(inter);
		}
	}
}

inline double log4(double x) { return 0.5 * log2(x); }

int Bezier::GetCurveDepth() const
{
	Vector2F la1 = Vector2F::Abs((this->P2 - this->P1) - (this->P1 - this->P0));
	Vector2F la2 = Vector2F::Abs((this->P3 - this->P2) - (this->P2 - this->P1));
	Vector2F la;
	if (la1.x > la2.x) la.x = la1.x; else la.x = la2.x;
	if (la1.y > la2.y) la.y = la1.y; else la.y = la2.y;
	double l0;
	if (la.x > la.y)
		l0 = la.x;
	else
		l0 = la.y;
	int ra;
	if (l0 * 0.75 * M_SQRT2 + 1.0 == 1.0)
		ra = 0;
	else
		ra = static_cast<int>(ceil(log4(M_SQRT2 * 6.0 / 8.0 * INV_EPS * l0)));

	return ra;
}

/*
 * Wang's theorem is used to estimate the level of subdivision required,
 * but only if the bounding boxes interfere at the top level.
 * Assuming there is a possible intersection, RecursivelyIntersect is
 * used to find all the parameters corresponding to intersection points.
 * these are then sorted and returned in an array.
 */

void Bezier::FindIntersections(Bezier const& a, Bezier const& b, std::vector<BezierIntersection> & intersections, bool allowBoundaries) const
{
	if (!IntersectBB(a, b, allowBoundaries))
	{
		return;
	}

	RecursivelyIntersect(a, 0., 1., a.GetCurveDepth(), b, 0., 1., b.GetCurveDepth(), intersections, allowBoundaries);
}

/*
 * Intersect two curves, returning an array of two arrays of curves.
 * The first array of curves corresponds to `this' curve, the second
 * corresponds to curve B, passed in.
 * The intersection parameter values are computed by FindIntersections,
 * and they come back in the range 0..1, using the original parameterization.
 * Once one segment has been removed, ie the curve is split at splitT, the
 * parameterization of the second half is from 0..1, so the parameter for
 * the next split point, if any, must be adjusted.
 * If we split at t[i], the split point at t[i+1] is
 * ( t[i+1] - t[i] ) / ( t - t[i] ) of the way to the end from the new
 * start point.
 */
std::vector<BezierIntersection> Bezier::FindAllIntersections(Bezier const& bezier, bool allowBoundaries)
{
	std::vector<BezierIntersection> intersections;
	FindIntersections(*this, bezier, intersections, allowBoundaries);

	return intersections;
}

bool Bezier::IsPointInBoundingBox(Vector2F const& p, Bezier a)
{
	// Compute bounding box for a
	double minax, maxax, minay, maxay;
	if (a.P0.x > a.P3.x)	 // These are the most likely to be extremal
		minax = a.P3.x, maxax = a.P0.x;
	else
		minax = a.P0.x, maxax = a.P3.x;
	if (a.P2.x < minax)
		minax = a.P2.x;
	else if (a.P2.x > maxax)
		maxax = a.P2.x;
	if (a.P1.x < minax)
		minax = a.P1.x;
	else if (a.P1.x > maxax)
		maxax = a.P1.x;
	if (a.P0.y > a.P3.y)
		minay = a.P3.y, maxay = a.P0.y;
	else
		minay = a.P0.y, maxay = a.P3.y;
	if (a.P2.y < minay)
		minay = a.P2.y;
	else if (a.P2.y > maxay)
		maxay = a.P2.y;
	if (a.P1.y < minay)
		minay = a.P1.y;
	else if (a.P1.y > maxay)
		maxay = a.P1.y;

	return !(minax > p.x || minay > p.y || p.x > maxax || p.y > maxay);
}

bool Bezier::RecursivePointSearch(Vector2F const& p, Bezier const& bez, int depth)
{
	if (!IsPointInBoundingBox(p, bez))
		return false;

	if (depth <= 0)
	{
		// Check if C is between A and B
		const Vector2F a = bez.P0;
		const Vector2F b = bez.P3;
		const Vector2F c = p;
		const float epsilon = 0.00001f;
		return (((c.x >= a.x) && (c.x <= b.x)) || ((c.x >= b.x) && (c.x <= a.x))) &&
			(((c.y >= a.y) && (c.y <= b.y)) || ((c.y >= b.y) && (c.y <= a.y))) &&
			(abs(((b.x - a.x) * (c.y - a.y)) - ((c.x - a.x) * (b.y - a.y))) < epsilon);
	}

	--depth;
	Bezier* split = bez.Split();
	return  RecursivePointSearch(p, split[0], depth) || RecursivePointSearch(p, split[1], depth);
}

bool Bezier::PointBelongsToCurve(Vector2F const& p) const
{
	return RecursivePointSearch(p, *this, GetCurveDepth());
}

bool Bezier::IsIntersecting(Vector2F const& p1, Vector2F const& p2, Vector2F const& q1, Vector2F const& q2)
{
	const float v1 = ((q1.x - p1.x)*(p2.y - p1.y) - (q1.y - p1.y)*(p2.x - p1.x))
		* ((q2.x - p1.x)*(p2.y - p1.y) - (q2.y - p1.y)*(p2.x - p1.x));
	const float v2 = ((p1.x - q1.x)*(q2.y - q1.y) - (p1.y - q1.y)*(q2.x - q1.x))
		* ((p2.x - q1.x)*(q2.y - q1.y) - (p2.y - q1.y)*(q2.x - q1.x));
	return (v1 <= 0.f && v2 < 0.f) || (v1 < 0.f && v2 <= 0.f);
}

// Linear intersection based on : https://www.geeksforgeeks.org/program-for-point-of-intersection-of-two-lines/
BezierIntersection Bezier::FindLinearIntersection(Bezier const& bezier, bool allowBoundaries) const
{
	Vector2F p0 = this->P0;
	Vector2F p1 = this->P2;
	Vector2F q0 = bezier.P0;
	Vector2F q1 = bezier.P2;

	const bool compare1 = allowBoundaries && p0 == q0;
	const bool compare2 = allowBoundaries && p0 == q1;
	const bool compare3 = allowBoundaries && p1 == q0;
	const bool compare4 = allowBoundaries && p1 == q1;
	if (compare1 || compare2 || compare3 || compare4)
	{
		BezierIntersection bIntersect;
		bIntersect.CurveAPercentage = compare1 || compare2 ? 0.f : nexttowardf(1.f, 0);
		bIntersect.CurveBPercentage = compare1 || compare3 ? 0.f : nexttowardf(1.f, 0);
		bIntersect.IntersectionPoint = compare1 || compare2 ? p0 : p1;
		return bIntersect;
	}

	if (!IsIntersecting(p0, p1, q0, q1))
		return BezierIntersection::Default;

	double a1 = p1.y - p0.y;
	double b1 = p0.x - p1.x;
	double c1 = a1 * p0.x + b1 * p0.y;

	double a2 = q1.y - q0.y;
	double b2 = q0.x - q1.x;
	double c2 = a2 * q0.x + b2 * q0.y;

	double determinant = a1 * b2 - a2 * b1;
	if (determinant == 0)
		return BezierIntersection::Default;

	double px = (b2 * c1 - b1 * c2) / determinant;
	double py = (a1 * c2 - a2 * c1) / determinant;
	Vector2F intersection = Vector2F(float (px), float(py));

	double perc1 = Vector2F::Magnitude(p0, intersection) / Vector2F::Magnitude(p0, p1);
	double perc2 = Vector2F::Magnitude(q0, intersection) / Vector2F::Magnitude(q0, q1);

	BezierIntersection bIntersect;
	bIntersect.CurveAPercentage = float(perc1);
	bIntersect.CurveBPercentage = float(perc2);
	bIntersect.IntersectionPoint = intersection;

	return bIntersect;
}
