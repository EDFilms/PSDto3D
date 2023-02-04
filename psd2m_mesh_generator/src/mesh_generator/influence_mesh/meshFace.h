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
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef MESH_FACE_H
#define MESH_FACE_H

#include <vector>
#include "util/math_2D.h"

using namespace util;

namespace mesh_generator
{
	struct MaskData
	{
		std::vector<unsigned char> Data;
		int Width;
		int Height;
		int BytesPerPixel;
	};

	struct SplitEdge
	{
		SplitEdge(int startPoint, int endPoint, int splitPoint);

		bool IsSameEdge(int otherStart, int otherEnd) const;

		int StartPoint;
		int EndPoint;
		int SplitPoint;
	};

	class MeshFace
	{
	public:
		MeshFace(std::vector<int>::const_iterator begin, std::vector<int>::const_iterator end);

		void SetShouldSubdivide(std::vector<Vector2F> const& vertices, MaskData const& influenceLayer, float minPolygonSize, float maxPolygonSize);
		bool GetShouldSubdivide() const { return this->ShouldSubdivide; }
		bool HasEdge(SplitEdge edge);

		std::vector<int> Indices;

	private:
		bool ShouldSubdivide;

		static float Clamp01(float x);
	};
}
#endif // MESH_FACE_H