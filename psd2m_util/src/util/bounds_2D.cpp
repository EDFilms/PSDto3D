//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file BoundingBox.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "boundingBox.h"
#include <iostream>
#include <string>
#include <algorithm>

namespace mesh_generator
{
#pragma region ACCESSORS

	//----------------------------------------------------------------------------------------
	Vector2F* boundingBox::BoundPoints() { return Points; }

	//----------------------------------------------------------------------------------------
	Vector2F boundingBox::TopLeftPoint() { return Points[0]; }

	//----------------------------------------------------------------------------------------
	Vector2F boundingBox::TopRightPoint() { return Points[1]; }

	//----------------------------------------------------------------------------------------
	Vector2F boundingBox::BottomRightPoint() { return Points[2]; }

	//----------------------------------------------------------------------------------------
	Vector2F boundingBox::BottomLeftPoint() { return Points[3]; }

#pragma endregion

#pragma region CALCUL BOUNDS

	//----------------------------------------------------------------------------------------
	void boundingBox::GenerateBoundingBox(std::vector<Vector2F*> const& pathPoints)
	{
		Vector2F min = Vector2F(1.0f, 1.0f);
		Vector2F max = Vector2F(0.0f, 0.0f);

		for (int i = -0; i < pathPoints.size(); i++)
		{
			min.x = std::min(min.x, pathPoints[i]->x);
			min.y = std::min(min.y, pathPoints[i]->y);
			max.x = std::max(max.x, pathPoints[i]->x);
			max.y = std::max(max.y, pathPoints[i]->y);
		}

		this->Points[0] = Vector2F(min.x, max.y); // top left;
		this->Points[1] = Vector2F(max.x, max.y); // top right;
		this->Points[2] = Vector2F(max.x, min.y); // bottom right;
		this->Points[3] = Vector2F(min.x, min.y); // bottom left;
	}

	//----------------------------------------------------------------------------------------
	void boundingBox::DisplayBoundingBox() const
	{
		std::string boxValue = "BoundingBox\n";
		boxValue.append("Point 1: ");
		boxValue.append(Points[0].ToMString());
		boxValue.append(";\nPoint 2: ");
		boxValue.append(Points[1].ToMString());
		boxValue.append(";\nPoint 3: ");
		boxValue.append(Points[2].ToMString());
		boxValue.append(";\nPoint 4: ");
		boxValue.append(Points[3].ToMString());
		std::cout << boxValue << std::endl;;
	}

#pragma endregion

#pragma region ORIENTED BOUNDING BOX FROM OPEN VECTOR

	//----------------------------------------------------------------------------------------------
	void boundingBox::GetDirection(std::vector<PathPoints*> const& refPoint)
	{
		unsigned int size = int(refPoint.size());
		// Not a path just one isolate point.
		if (size < 2) return;

		Vector2F* firstPoint = new Vector2F(refPoint[0]->AnchorPoint.x, refPoint[0]->AnchorPoint.y);
		Vector2F* lastPoint = new Vector2F(refPoint[size - 1]->AnchorPoint.x, refPoint[size - 1]->AnchorPoint.y);

		this->OrientedVector[0] = firstPoint->x < lastPoint->x ? *firstPoint : *lastPoint;
		this->OrientedVector[1] = firstPoint->x < lastPoint->x ? *lastPoint : *firstPoint;
	}

	//----------------------------------------------------------------------------------------------
	void boundingBox::GetDirection(float angle)
	{
		this->OrientedVector[0] = Vector2F(0.0, 0.0);
		this->OrientedVector[1] = Vector2F(std::abs(std::cos(angle)), std::abs(std::sin(angle)));
	}

	//----------------------------------------------------------------------------------------------
	void boundingBox::SetOrientation(PathRecord const& refPoint)
	{
		this->GetDirection(refPoint.Points);
	}

	//----------------------------------------------------------------------------------------------
	void boundingBox::SetOrientation(float const angle)
	{
		this->GetDirection(angle);
	}

	//----------------------------------------------------------------------------------------------
	void boundingBox::GenerateOrientedBoundingBox(std::vector<Vector2F*> const& pathPoints)
	{
		// determine orientation and find the orthogonal vector
		Vector2F pointOrthogonal;
		bool topDirection = this->OrientedVector[0].y < this->OrientedVector[1].y;

		Vector2F vectBase = Vector2F(this->OrientedVector[1].x - this->OrientedVector[0].x, this->OrientedVector[1].y - this->OrientedVector[0].y);

		pointOrthogonal.x = topDirection ? vectBase.y : vectBase.y * -1;
		pointOrthogonal.y = topDirection ? vectBase.x * -1 : vectBase.x;

		// Get the projection and keep the extreme point
		float r0 = std::sqrt(std::powf(vectBase.x, 2.0) + std::powf(vectBase.y, 2.0));
		float r1 = std::sqrt(std::powf(pointOrthogonal.x, 2.0) + std::powf(pointOrthogonal.y, 2.0));

		Vector2F firstNormalizedVector = Vector2F((vectBase.x) / r0, (vectBase.y) / r0);
		Vector2F SecondeNormalizedVector = Vector2F((pointOrthogonal.x) / r1, (pointOrthogonal.y) / r1);

		//Vector2f MinFirst, MaxFirst, MinSecond, MaxSecond;

		float minDistFirst = 50.f;
		float minDistSeconde = 50.f;
		float maxDistFirst = -50.f;
		float maxDistSeconde = -50.0f;
		float tmpVal = 0.f;

		for (int i = 0; i < pathPoints.size(); i++)
		{
			tmpVal = (firstNormalizedVector.x * pathPoints[i]->x) + (firstNormalizedVector.y * pathPoints[i]->y);
			minDistFirst = std::min(tmpVal, minDistFirst);
			maxDistFirst = std::max(tmpVal, maxDistFirst);

			tmpVal = (SecondeNormalizedVector.x * pathPoints[i]->x) + (SecondeNormalizedVector.y * pathPoints[i]->y);
			minDistSeconde = std::min(tmpVal, minDistSeconde);
			maxDistSeconde = std::max(tmpVal, maxDistSeconde);
		}

		// Find the 3 Points reference for the algorithm, get the projection from point with the point.

		// top left
		Points[0] = (firstNormalizedVector * (topDirection ? maxDistFirst : minDistFirst)) + (SecondeNormalizedVector * (topDirection ? minDistSeconde : maxDistSeconde));
		// top right
		Points[1] = (firstNormalizedVector * maxDistFirst) + (SecondeNormalizedVector * maxDistSeconde);
		// bottom right
		Points[2] = (firstNormalizedVector * (topDirection ? minDistFirst : maxDistFirst)) + (SecondeNormalizedVector * (topDirection ? maxDistSeconde : minDistSeconde));
		// bottom left
		Points[3] = (firstNormalizedVector * minDistFirst) + (SecondeNormalizedVector * minDistSeconde);
	}

#pragma endregion

#pragma region ORIENTED BOUNDING BOX

	//----------------------------------------------------------------------------------------
	double boundingBox::Cross(const Vector2F &o, const Vector2F &a, const Vector2F &b)
	{
		return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
	}

	//----------------------------------------------------------------------------------------
	std::vector<Vector2F> boundingBox::ConvexHull(std::vector<Vector2F>& points)
	{
		int n = int(points.size());
		int k = 0;
		std::vector<Vector2F> H(2 * n);

		// Sort Points lexicographically
		sort(points.begin(), points.end());

		// Build lower hull
		for (int i = 0; i < n; ++i) {
			while (k >= 2 && Cross(H[k - 2], H[k - 1], points[i]) <= 0) k--;
			H[k++] = points[i];
		}

		// Build upper hull
		for (int i = n - 2, t = k + 1; i >= 0; i--) {
			while (k >= t && Cross(H[k - 2], H[k - 1], points[i]) <= 0) k--;
			H[k++] = points[i];
		}

		H.resize(k - 1);
		return H;
	}

#pragma endregion
}
