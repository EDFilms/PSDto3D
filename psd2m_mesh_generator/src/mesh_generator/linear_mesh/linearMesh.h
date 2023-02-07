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
#include <set>
#include "util/math_2D.h"
#include "util/bounds_2D.h"
#include "util/progressJob.h"
#include "bezierCurve.h"
#include "../dataMesh.h"

using namespace util;

namespace mesh_generator
{
	struct BillboardMeshParameters
	{
		int BillboardAlphaThresh = 4;

		bool operator==(const BillboardMeshParameters& that) const { return (this->BillboardAlphaThresh==that.BillboardAlphaThresh); }
		bool operator!=(const BillboardMeshParameters& that) const { return !(this->operator==(that)); }
	};

	struct LinearMeshParameters
	{
		int LinearHeightPoly = 10;

		bool operator==(const LinearMeshParameters& that) const { return (this->LinearHeightPoly==that.LinearHeightPoly); }
		bool operator!=(const LinearMeshParameters& that) const { return !(this->operator==(that)); }
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
		std::set<Vector2F> curvePoints; // points contained by poly, not in original order

		bool ContainsPoint( const Vector2F& p ); // if point is bounded by the poly
		// Searches the curve, adding points bounded by the poly to the curvePoints list
		// Walks using the given index and direction, stops when outside the poly bounds
		void AddCurvePoints( const BezierCurve& curve, int indexStart, int indexDir );
		Vector2F FitVertexToCurvePoints( int index ); // tucks in vertex to fit the curve
		std::vector<int> GetVertexIndexes();
	};

	struct BillboardMeshAlgoParameters
	{
		BillboardMeshAlgoParameters( const BillboardMeshParameters& params ) : Parameters(params) {}
		int GetAlphaThresh() const
		{
			return this->Parameters.BillboardAlphaThresh;
		}
	private:
		BillboardMeshParameters Parameters;
	};

	struct LinearMeshAlgoParameters
	{
	public:
		int RowCount = 0;
		int ColumnCount = 0;
		float Precision = 0.f;
		Vector2F VectNormalH, VectNormalV = Vector2F();
		Vector2F VectSpacerH, VectSpacerV = Vector2F();
		Vector2F GeneratedTopLeft, GeneratedTopRight, GeneratedBottomRight, GeneratedBottomLeft = Vector2F();

		LinearMeshAlgoParameters(std::string name, const LinearMeshParameters& params) : Name(std::move(name)), Parameters(params){}

		std::string GetName() const { return this->Name; }
		float GetPrecision() const
		{
			return (float(this->Parameters.LinearHeightPoly));
		}

	private:
		std::string Name;
		LinearMeshParameters Parameters;
	};

	//----------------------------------------------------------------------------------------------
	class LinearMesh
	{
	public:
		static DataMesh GenerateMesh(const std::string& name, const LinearMeshParameters& params,
			boundsUV& bounds, const std::vector<BezierCurve*>& curves, int width, int height, ProgressTask& progressTask);
		static DataSpline GenerateSpline(const std::string& name, const LinearMeshParameters& params,
			boundsUV& bounds, const std::vector<BezierCurve*>& curves, int width, int height, ProgressTask& progressTask);

	private:
		static MeshVertex** GenerateGrid(boundsUV & bounds, LinearMeshAlgoParameters* paramsGeneration);
		static std::vector<MeshPoly*> FilterGrid(MeshVertex ** initialGrid, const std::vector<BezierCurve*>& curves, boundsUV & bounds, LinearMeshAlgoParameters* paramsGeneration);
		static void IdentificationContourPoly(MeshPoly ** polys, BezierCurve const & curve, boundsUV & bounds, LinearMeshAlgoParameters* paramsGeneration);
		static void CleanAloneSplit(MeshPoly ** polys, LinearMeshAlgoParameters* paramsGeneration);
		static std::vector<MeshPoly*> BuildPolyMesh(MeshPoly ** polys, LinearMeshAlgoParameters* paramsGeneration);
		static DataMesh BuildDataMesh(MeshVertex** grid, std::vector<MeshPoly*> polys, LinearMeshAlgoParameters* paramsGeneration, int width, int height);
		static Vector2F GetCenterSegment(Vector2F pos1, Vector2F pos2);
		static void FixEdgeCaseThreeAndFiveVertice(MeshPoly ** polys, int row, int column, bool isSelected, bool isSelectedInversed, LinearMeshAlgoParameters* paramsGeneration);

	};
}
#endif // LINEAR_MESH_H
