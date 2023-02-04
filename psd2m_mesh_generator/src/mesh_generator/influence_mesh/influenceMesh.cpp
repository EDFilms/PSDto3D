//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file influenceMesh.cpp
//  @author Benjamin Drouin
//  @date 12-10-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "influenceMesh.h"
#include <queue>

namespace mesh_generator
{
	Vector2F GetCenterPoint(MeshFace const& face, std::vector<Vector2F> const& vertices)
	{
		Vector2F point;
		for (auto index : face.Indices)
		{
			point += vertices[index];
		}

		point /= float(face.Indices.size());
		return point;
	}

	void InfluenceMesh::SubdivideFaces(DataMesh& mesh, MaskData const& influenceLayer, InfluenceParameters const& params)
	{
		// Mesh data
		std::vector<Vector2F> vertices = mesh.GetVertices();
		std::vector<int> faceIndices = mesh.GetFacesIndices();

		// Create Queue that will be processed
		std::queue<MeshFace> toProcess;
		auto faceIt = faceIndices.cbegin();
		for (auto facesCount : mesh.GetFacesCount())
		{
			MeshFace face = MeshFace(faceIt, faceIt + facesCount);
			face.SetShouldSubdivide(vertices, influenceLayer, params.MinPolygonSize, params.MaxPolygonSize);
			toProcess.push(face);

			faceIt += facesCount;
		}

		std::vector<SplitEdge> splitEdges;
		std::vector<MeshFace> completedFaces;

		while (!toProcess.empty())
		{
			MeshFace current = toProcess.front();
			toProcess.pop();

			// Temp algo to check when to stop dividing
			if (!current.GetShouldSubdivide())
			{
				UpdateFaceWithDividedEdges(current, splitEdges);
				completedFaces.push_back(current);
				continue;
			}

			Vector2F center = GetCenterPoint(current, vertices);
			int centerIndex = int(vertices.size());
			vertices.push_back(center);

			// Add first element to the end for a loop
			current.Indices.push_back(*current.Indices.begin());

			// Create all vertices in between
			for (unsigned i = 0; i < current.Indices.size() - 1; i += 2)
			{
				const auto indexA = current.Indices[i];
				const auto indexB = current.Indices[i + 1];

				int midIndex;
				auto edgeIt = FindEdge(splitEdges, indexA, indexB);
				if (edgeIt == splitEdges.end())
				{
					// Vertex doesn't exist so we create it
					Vector2F midPoint = Vector2F::Mid(vertices[indexA], vertices[indexB]);
					midIndex = int(vertices.size());
					vertices.push_back(midPoint);

					SplitEdge edge = SplitEdge(indexA, indexB, midIndex);
					splitEdges.push_back(edge);

					ReturnCompletedFaceIfSplit(edge, completedFaces, toProcess);
				}
				else
				{
					// Vertex already exists so we'll use that one
					midIndex = (*edgeIt).SplitPoint;
					splitEdges.erase(edgeIt);
				}

				current.Indices.insert(current.Indices.begin() + i + 1, midIndex);
			}

			// Add second element to the end for even more loop
			current.Indices.push_back(*(current.Indices.begin() + 1));

			// Split into faces
			for (unsigned i = 1; i < current.Indices.size() - 2; i += 2)
			{
				auto it = current.Indices.begin() + i;

				std::vector<int> indices;
				indices.insert(indices.end(), it, it + 3);
				indices.push_back(centerIndex);

				MeshFace face = MeshFace(indices.begin(), indices.end());
				face.SetShouldSubdivide(vertices, influenceLayer, params.MinPolygonSize, params.MaxPolygonSize);
				toProcess.push(face);
			}
		}

		// Rebuild mesh
		mesh.ClearFaces();
		mesh.SetVertices(vertices);
		for (auto face : completedFaces)
		{
			mesh.AddFace(face.Indices);
		}
	}

	void InfluenceMesh::UpdateFaceWithDividedEdges(MeshFace & face, std::vector<SplitEdge> & edges)
	{
		for (unsigned i = 0; i < face.Indices.size() - 1; ++i)
		{
			auto indexA = face.Indices[i];
			auto indexB = face.Indices[i + 1];

			auto edgeIt = FindEdge(edges, face.Indices[i], face.Indices[i + 1]);
			if (edgeIt != edges.end())
			{
				face.Indices.insert(face.Indices.begin() + i + 1, (*edgeIt).SplitPoint);
				edges.erase(edgeIt);
			}
		}

		auto edgeIt = FindEdge(edges, *face.Indices.begin(), *(face.Indices.end() - 1));
		if (edgeIt != edges.end())
		{
			face.Indices.insert(face.Indices.end(), (*edgeIt).SplitPoint);
			edges.erase(edgeIt);
		}
	}

	std::vector<SplitEdge>::const_iterator InfluenceMesh::FindEdge(std::vector<SplitEdge> & edges, int vertexA, int vertexB)
	{
		return std::find_if(edges.begin(), edges.end(), [&vertexA, &vertexB](SplitEdge edge) { return edge.IsSameEdge(vertexA, vertexB); });
	}

	void InfluenceMesh::ReturnCompletedFaceIfSplit(SplitEdge & edge, std::vector<MeshFace> & completedFaces, std::queue<MeshFace> & toProcess)
	{
		const auto completedFaceIt = std::find_if(completedFaces.begin(), completedFaces.end(), [&edge](MeshFace face) { return face.HasEdge(edge); });
		if (completedFaceIt != completedFaces.end())
		{
			toProcess.push(*completedFaceIt);
			completedFaces.erase(completedFaceIt);
		}
	}
}
