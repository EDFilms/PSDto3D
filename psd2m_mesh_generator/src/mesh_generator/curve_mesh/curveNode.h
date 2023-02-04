//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file curveNode.cpp
//  @author Benjamin Drouin
//  @date 01-10-2018
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef CURVE_NODE_H
#define CURVE_NODE_H

#include "util/math_2D.h"
#include <vector>
#include <set>

using namespace util;

namespace mesh_generator
{
	class Node
	{
	public:

		Node(Vector2F vertex, int index);

		void AddNeighbour(Node* node);
		Node* GetNeighbour(Node* ignore) const;
		std::set<Node*> GetNeighbours() const;
		void RemoveNeighbour(Node* neighbour);
		void ClearNeighbours();

		std::vector<Node*> MergeCloseNeighbours(float const distance);

		Node* GetClockwiseMost(Vector2F const& previousVertex, Node* ignore) const;
		Node* GetCounterClockwiseMost(Vector2F const& previousVertex, Node* ignore) const;

		Vector2F GetVertex() const { return this->Vertex; }
		int GetIndex() const { return this->Index; }
		void SetIndex(const int i) { this->Index = i; }

	private:
		int Index;
		Vector2F Vertex;

		std::set<Node*> Neighbours;
	};
}
#endif // CURVE_NODE_H