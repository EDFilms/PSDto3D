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
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "curveNode.h"
#include <queue>
#include <map>

namespace mesh_generator
{
	Node::Node(Vector2F vertex, int index)
	{
		this->Index = index;
		this->Vertex = vertex;
	};

	void Node::AddNeighbour(Node* node)
	{
		if (node == this)
		{
			return;
		}

		this->Neighbours.insert(node);
	}

	Node* Node::GetNeighbour(Node* ignore) const
	{
		for (auto neighbour : this->Neighbours)
		{
			if (neighbour == ignore)
			{
				continue;
			}

			return neighbour;
		}

		return nullptr;
	}

	std::set<Node*> Node::GetNeighbours() const
	{
		return this->Neighbours;
	}

	void Node::RemoveNeighbour(Node* neighbour)
	{
		const auto it = std::find(this->Neighbours.cbegin(), this->Neighbours.cend(), neighbour);
		if (it != this->Neighbours.cend())
		{
			this->Neighbours.erase(it);
		}
	}

	void Node::ClearNeighbours()
	{
		for (auto neighbour : this->Neighbours)
		{
			neighbour->RemoveNeighbour(this);
		}

		this->Neighbours.clear();
	}

	std::vector<Node*> Node::MergeCloseNeighbours(float const distance)
	{
		std::vector<Node*> toRemove;
		for (auto neighbour : this->Neighbours)
		{
			Vector2F n1 = this->Vertex;
			Vector2F n2 = neighbour->Vertex;
			if( ((distance==0.0f) && (n1==n2)) ||
			    ((distance!=0.0f) && Vector2F::AreSimilar(n1, n2, distance)) )
			{
				toRemove.push_back(neighbour);
			}
		}

		float toRemoveSize = 1.f;
		for (auto remove : toRemove)
		{
			for (auto node : remove->Neighbours)
			{
				if (node != this)
				{
					AddNeighbour(node);
					node->AddNeighbour(this);
				}
			}

			this->Vertex.x += remove->Vertex.x;
			this->Vertex.y += remove->Vertex.y;
			toRemoveSize += 1.f;

			remove->ClearNeighbours();
		}

		if (toRemoveSize > 1.f)
		{
			this->Vertex.x /= toRemoveSize;
			this->Vertex.y /= toRemoveSize;
		}

		return toRemove;
	}

	// Algorithm based on https://www.geometrictools.com/Documentation/MinimalCycleBasis.pdf (Listing 1.)
	Node* Node::GetClockwiseMost(Vector2F const& previousVertex, Node* ignore = nullptr) const
	{
		Vector2F dcurr = this->Vertex - previousVertex;
		Node* vnext = this->GetNeighbour(ignore);
		if (vnext == nullptr)
		{
			return nullptr;
		}

		Vector2F dnext = vnext->Vertex - this->Vertex;
		bool isConvex = dnext * dcurr.Perpendicular() <= 0.f;

		for (auto neighbour : this->Neighbours)
		{
			if (neighbour == ignore)
			{
				continue;
			}

			Vector2F dadj = neighbour->Vertex - this->Vertex;
			if (isConvex)
			{
				if (dcurr * dadj.Perpendicular() < 0.f || dnext * dadj.Perpendicular() < 0.f)
				{
					vnext = neighbour;
					dnext = dadj;
					isConvex = dnext * dcurr.Perpendicular() <= 0.f;
				}
			}
			else
			{
				if (dcurr * dadj.Perpendicular() < 0.f && dnext * dadj.Perpendicular() < 0.f)
				{
					vnext = neighbour;
					dnext = dadj;
					isConvex = dnext * dcurr.Perpendicular() <= 0.f;
				}
			}
		}

		return vnext;
	}

	// Algorithm based on https://www.geometrictools.com/Documentation/MinimalCycleBasis.pdf (Listing 2.)
	Node* Node::GetCounterClockwiseMost(Vector2F const& previousVertex, Node* ignore = nullptr) const
	{
		Vector2F dcurr = this->Vertex - previousVertex;
		Node* vnext = this->GetNeighbour(ignore);
		if (vnext == nullptr)
		{
			return nullptr;
		}

		Vector2F dnext = vnext->Vertex - this->Vertex;
		bool isConvex = dnext * dcurr.Perpendicular() <= 0.f;

		for (auto neighbour : this->Neighbours)
		{
			if (neighbour == ignore)
			{
				continue;
			}

			Vector2F dadj = neighbour->Vertex - this->Vertex;
			if (isConvex)
			{
				if (dcurr * dadj.Perpendicular() > 0.f && dnext * dadj.Perpendicular() > 0.f)
				{
					vnext = neighbour;
					dnext = dadj;
					isConvex = dnext * dcurr.Perpendicular() <= 0.f;
				}
			}
			else
			{
				if (dcurr * dadj.Perpendicular() > 0.f || dnext * dadj.Perpendicular() > 0.f)
				{
					vnext = neighbour;
					dnext = dadj;
					isConvex = dnext * dcurr.Perpendicular() <= 0.f;
				}
			}
		}

		return vnext;
	}
}
