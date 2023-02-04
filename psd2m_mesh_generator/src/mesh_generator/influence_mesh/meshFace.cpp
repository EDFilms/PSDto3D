//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file meshFace.cpp
//  @author Benjamin Drouin
//  @date 16-10-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "meshFace.h"
#include "mesh_generator/linear_mesh/boundingBox.h"
#include "influenceMesh.h"

namespace mesh_generator
{
	SplitEdge::SplitEdge(int startPoint, int endPoint, int splitPoint)
	{
		this->StartPoint = startPoint;
		this->EndPoint = endPoint;
		this->SplitPoint = splitPoint;
	}

	bool SplitEdge::IsSameEdge(int otherStart, int otherEnd) const
	{
		return (StartPoint == otherStart && EndPoint == otherEnd)
			|| (EndPoint == otherStart && StartPoint == otherEnd);
	}

	MeshFace::MeshFace(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end)
	{
		this->Indices.insert(this->Indices.end(), begin, end);
	}

	bool MeshFace::HasEdge(SplitEdge edge)
	{
		for (unsigned i = 0; i < this->Indices.size() - 1; ++i)
		{
			if (edge.IsSameEdge(this->Indices[i], this->Indices[i + 1]))
			{
				return true;
			}
		}

		return edge.IsSameEdge(*this->Indices.begin(), *(this->Indices.end() - 1));
	}

	void MeshFace::SetShouldSubdivide(std::vector<Vector2F> const& vertices, MaskData const& influenceLayer, float minPolygonSize, float maxPolygonSize)
	{
		// Create bounding box
		std::vector<Vector2F*> faceVertices;
		for (auto index : this->Indices)
		{
			Vector2F* v = new Vector2F(vertices[index].x, vertices[index].y);
			faceVertices.push_back(v);
		}

		boundingBox box;
		box.GenerateBoundingBox(faceVertices);

		// Get highest value of mask inside bounding box
		const int startX = int(influenceLayer.Width * Clamp01(box.BottomLeftPoint().x));
		const int startY = int(influenceLayer.Height * Clamp01(box.BottomLeftPoint().y));
		const int endX = int(influenceLayer.Width * Clamp01(box.TopRightPoint().x));
		const int endY = int(influenceLayer.Height * Clamp01(box.TopRightPoint().y));
		float maxCharValue = 0;
		for (int i = startX; i < endX; ++i)
		{
			for (int j = startY; j < endY; ++j)
			{
				unsigned char c = influenceLayer.Data[(j * influenceLayer.Width + i) * influenceLayer.BytesPerPixel];
				if (c > maxCharValue)
				{
					maxCharValue = c;
				}
			}
		}

		// Use highest value we found to figure if we want to subdivide or not
		const float boxDiag = Vector2F::Magnitude(box.TopLeftPoint(), box.BottomRightPoint());
		const float maxValue = maxCharValue / 255.f;
		const float maxDiag = (minPolygonSize - maxPolygonSize) * maxValue + maxPolygonSize;
		this->ShouldSubdivide = boxDiag > maxDiag;
	}

	float MeshFace::Clamp01(float x)
	{
		if (x < 0.f)
		{
			return 0.f;
		}

		if (x > 1.f)
		{
			return 1.f;
		}

		return x;
	}
}
