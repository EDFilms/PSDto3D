//----------------------------------------------------------------------------------------------
// ===============================================
//	Copyright (C) 2016, CDRIN.
//	All Rights Reserved.
// ===============================================
//	Unauthorized copying of this file, via any medium is strictly prohibited
//	Proprietary and confidential
//
//	@file mesh.h
//	@author Fabien Raspail
//	@date 11-11-2016
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef LINEAR_MESH_H
#define LINEAR_MESH_H

#include <utility>
#include <vector>
#include "util/math_2D.h"
#include "bezierCurve.h"
#include "boundingBox.h"
#include "../dataMesh.h"

using namespace util;

namespace mesh_generator
{
	struct LinearParameters
	{
		int LinearHeightPoly = 10;
		float GridOrientation = 0.0f;
	};

	//----------------------------------------------------------------------------------------------
	struct MeshVertex
	{
		// Use to keep the position in the array of vertices.
		int Index = -1;
		Vector2F Position;
	};

	//----------------------------------------------------------------------------------------------
	struct MeshPoly
	{
		MeshVertex* Vertex[4]{}; // order: topLeft, topRight, bottomRight, bottomLeft
		Vector2F RigthIntersection = Vector2F(-1.f, -1.f);
		Vector2F BottomIntersection = Vector2F(-1.f, -1.f);
		int IndexToRemove = -1;
		bool IsIncluded = false;
		bool SideSplit[4] = { false, false, false, false }; // top, right, bottom, left;

		std::vector<int> GetVertexIndexes();
	};

	struct GlobalParameters
	{
	public:
		int RowCount = 0;
		int ColumnCount = 0;
		float Precision = 0.f;
		Vector2F VectNormalH, VectNormalV = Vector2F();
		Vector2F GeneratedTopLeft, GeneratedTopRight, GeneratedBottomRight, GeneratedBottomLeft = Vector2F();

		GlobalParameters(std::string name, LinearParameters const& params) : Name(std::move(name)), Parameters(params){}

		std::string GetName() const { return this->Name; }
		float GetPrecision() const
		{
			return (float(this->Parameters.LinearHeightPoly));
		}

	private:
		std::string Name;
		LinearParameters Parameters;
	};

	//----------------------------------------------------------------------------------------------
	class LinearMesh
	{
	public:
		static DataMesh GenerateMesh(std::string const& name, LinearParameters const& params, boundingBox& bounds, std::vector<BezierCurve*> curves);

	private:
		static MeshVertex** GenerateGrid(boundingBox & bounds, GlobalParameters* paramsGeneration);
		static std::vector<MeshPoly*> FilterGrid(MeshVertex ** initialGrid, const std::vector<BezierCurve*>& curves, boundingBox & bounds, GlobalParameters* paramsGeneration);
		static void IdentificationContourPoly(MeshPoly ** polys, BezierCurve const & curve, boundingBox & bounds, GlobalParameters* paramsGeneration);
		static void CleanAloneSplit(MeshPoly ** polys, GlobalParameters* paramsGeneration);
		static std::vector<MeshPoly*> BuildPolyMesh(MeshPoly ** polys, GlobalParameters* paramsGeneration);
		static DataMesh BuildDataMesh(MeshVertex** grid, std::vector<MeshPoly*> polys, GlobalParameters* paramsGeneration);
		static Vector2F GetCenterSegment(Vector2F pos1, Vector2F pos2);
		static void FixEdgeCaseThreeAndFiveVertice(MeshPoly ** polys, int row, int column, bool isSelected, bool isSelectedInversed);

	};
}
#endif // LINEAR_MESH_H
