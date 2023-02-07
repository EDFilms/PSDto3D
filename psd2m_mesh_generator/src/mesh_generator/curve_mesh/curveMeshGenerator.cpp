//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file curveMeshGenerator.cpp
//  @author Benjamin Drouin
//  @date 25-09-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "curveMeshGenerator.h"
#include <queue>
#include <map>

namespace mesh_generator
{
	DataMesh CurveMeshGenerator::GenerateMesh(std::vector<Curve> curves, std::string const& name, CurveParameters const& params,
		int width, int height, ProgressTask& progressTask )
	{
		DataMesh mesh = DataMesh(name, (float)width, (float)height);

		// Create curves (data contains Curve & Intersection vector
		std::vector<CurveData> paths;
		for (auto curve : curves)
		{
			CurveData data = CurveData(curve);
			paths.push_back(data);
		}

		// Find intersections of all curves
		std::vector<Node*> nodes;
		const unsigned int size = int(paths.size());
		for (unsigned i = 0; i < size; ++i)
		{
			for (unsigned j = i; j < size; ++j)
			{
				FindIntersections(paths[i], paths[j], nodes);
			}
		}

		progressTask.SetValueAndUpdate( 0.25f );

		// Connect nodes of all intersections (this is how we find the neighbours)
		for (auto pathData : paths)
		{
			pathData.ConnectNodes();
		}

		progressTask.SetValueAndUpdate( 0.50f );

		// Remove nodes outside of "original" path
		// Or remove neighbours so close that they overlap each other
		std::vector<Node*> allNodesToRemove;
		for (auto node : nodes)
		{
			if (!IsInsideClosedCurve(paths, node))
			{
				node->ClearNeighbours();
				allNodesToRemove.push_back(node);
			}
		}

		float mergeVertexDistance = ( params.MergeVertexEnabled?  params.MergeVertexDistance : 0.0f );
		for (auto node : nodes)
		{
			auto toRemove = node->MergeCloseNeighbours(mergeVertexDistance);
			allNodesToRemove.insert(allNodesToRemove.end(), toRemove.begin(), toRemove.end());
		}

		for (auto node : allNodesToRemove)
		{
			const auto it = std::find(nodes.begin(), nodes.end(), node);
			if (it != nodes.end())
			{
				nodes.erase(it);
			}
		}

		progressTask.SetValueAndUpdate( 0.75f );

		// Create Vertex vector for mesh and set index value
		std::vector<Vector2F> vertices;
		int count = 0;
		for (auto node : nodes)
		{
			vertices.push_back(node->GetVertex());

			node->SetIndex(count);
			++count;
		}

		mesh.SetVertices(vertices);

		// Create Points vector with pointers to vertices, and calculate bounds
		// DO NOT modify the vertices array during this operation
		boundsUV bounds = boundsUV();
		std::vector<Vector2F*> points;
		points.resize(vertices.size(),nullptr);
		for( size_t i=0; i<vertices.size(); i++ ) points[i]=&(vertices[i]); // array of pointers
		bounds.GenerateBoundingBox(points);
		mesh.SetBoundsUV(bounds);
		// Do not set XFormUV with mesh.SetXFormUV(xform);
		// see calculation of GraphLayer.XFormUV in CreateTreeStructure()

		// Use "Minimal Cycle Basis" to find all faces
		MinimalCycleSearch(nodes, mesh);

		// We're done so remove all created objects
		for (auto node : nodes)
		{
			delete node;
		}

		return mesh;
	}

	void CurveMeshGenerator::FindIntersections(CurveData & pathA, CurveData & pathB, std::vector<Node*> & nodes)
	{
		auto pathBezierA = pathA.GetCurve().GetBezierCurve();
		auto pathBezierB = pathB.GetCurve().GetBezierCurve();
		const unsigned sizeA = int(pathBezierA.size());
		const unsigned sizeB = int(pathBezierB.size());

		const bool isSamePath = &pathA == &pathB;

		for (unsigned i = 0; i < sizeA; ++i)
		{
			for (unsigned j = (isSamePath ? i : 0); j < sizeB; j++)
			{
				Bezier bezierA = pathBezierA[i];
				Bezier bezierB = pathBezierB[j];

				const bool areBezierFollowing = isSamePath && (j - i <= 1 || (i == 0 && j == sizeA - 1));

				std::vector<BezierIntersection> intersections = bezierA.FindAllIntersections(bezierB, !areBezierFollowing);
				for (auto intersection : intersections)
				{
					Node* node = new Node(intersection.IntersectionPoint, int(nodes.size()));
					nodes.push_back(node);

					float valueA = i + intersection.CurveAPercentage;
					PathIntersection intersectionA = PathIntersection(node, valueA);
					pathA.AddIntersection(intersectionA);

					float valueB = j + intersection.CurveBPercentage;
					PathIntersection intersectionB = PathIntersection(node, valueB);
					pathB.AddIntersection(intersectionB);
				}
			}
		}
	}

	bool CurveMeshGenerator::IsInsideClosedCurve(std::vector<CurveData> const& paths, Node* node)
	{
		// Choose a point that is assured to by outside of any curve (all curves are in the range [x=0-1, y=0-1], so -1 is good)
		const Vector2F farAway = Vector2F(-1.f, -1.f);
		const Vector2F nodeVertex = node->GetVertex();
		Bezier nodeBezier = Bezier(nodeVertex, nodeVertex, farAway, farAway);

		bool atLeastOneClosedPath = false;

		for (const auto& path : paths)
		{
			if (!path.GetCurve().IsClosedPath)
			{
				continue;
			}

			atLeastOneClosedPath = true;

			int intersectionCount = 0;
			auto bezierPath = path.GetCurve().GetBezierCurve();
			for (const auto& bezier : bezierPath)
			{
				if (bezier.PointBelongsToCurve(nodeVertex))
					return true;

				const std::vector<BezierIntersection> intersections = nodeBezier.FindAllIntersections(bezier, false);
				intersectionCount += int(intersections.size());
			}

			if (intersectionCount % 2 == 1)
				return true;
		}

		return !atLeastOneClosedPath;
	}

	bool CurveMeshGenerator::ValidateFace(Node* node, std::vector<int> const& face)
	{
		/*
		 * To be valid, a face must :
		 * 0. Have equal or more than 3 sides
		 * 1. Start with the Index of the starting node
		 * 2. The second index of the face must be one of the neighbours of the starting node
		 * 3. And finally, the last index of the face must be one of the neighbours of the starting node, but not the index from the second point.
		 */

		if (face.size() < 3)
			return false;

		auto it = face.begin();
		if (node->GetIndex() != *it)
			return false;

		++it;
		bool any = false;
		int firstFace = 0;
		for (const auto& neighbour : node->GetNeighbours())
		{
			if (neighbour->GetIndex() == *it)
			{
				any = true;
				firstFace = *it;
				break;
			}
		}

		if (!any)
			return false;

		auto itEnd = face.rbegin();
		any = false;
		for (auto neighbour : node->GetNeighbours())
		{
			if (neighbour->GetIndex() == *itEnd && neighbour->GetIndex() != firstFace)
			{
				any = true;
				break;
			}
		}

		return any;
	}

	// MinimalCycleSearch algorithm based on : https://www.geometrictools.com/Documentation/MinimalCycleBasis.pdf
	void CurveMeshGenerator::MinimalCycleSearch(std::vector<Node*> const& nodes, DataMesh& mesh)
	{
		std::vector<Node*> sortedNodes;
		sortedNodes.insert(sortedNodes.end(), nodes.begin(), nodes.end());
		std::sort(sortedNodes.begin(), sortedNodes.end(), SortNodesBasedOnX);

		std::vector<int> face;
		for (auto node : sortedNodes)
		{
			if (node->GetNeighbours().empty())
			{
				continue;
			}

			Vector2F leftPos = node->GetVertex() + Vector2F::Left;
			while (node->GetNeighbours().size() > 1)
			{
				Node* neighbour = node->GetClockwiseMost(leftPos, nullptr);

				face.clear();
				CompleteFace(node, neighbour, face);

				if (!ValidateFace(node, face))
				{
					node->RemoveNeighbour(neighbour);
					neighbour->RemoveNeighbour(node);
					continue;
				}

				node->RemoveNeighbour(neighbour);
				neighbour->RemoveNeighbour(node);
				if( face.size()>2 ) // ignore degenerate faces; need at least three vertices
				{
					mesh.AddFace(face);
				}
			}

			node->ClearNeighbours();
		}
	}

	void CurveMeshGenerator::CompleteFace(Node* first, Node* second, std::vector<int> & vertices)
	{
		vertices.push_back(first->GetIndex());

		Node* previous = first;
		Node* current = second;
		while (current != first && current != nullptr)
		{
			vertices.push_back(current->GetIndex());
			Node* next = current->GetCounterClockwiseMost(previous->GetVertex(), previous);

			previous = current;
			current = next;
		}

		if (current == nullptr)
		{
			vertices.clear();
		}
	}
}
