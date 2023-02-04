//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file mesh.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "linearMesh.h"
#include <cmath>

namespace mesh_generator
{

#pragma region MESH POLY

	//----------------------------------------------------------------------------------------
	std::vector<int> MeshPoly::GetVertexIndexes()
	{
		std::vector<int> indexes;
		for (auto i = 0; i < std::size(this->Vertex); i++)
		{
			if (this->Vertex[i] == nullptr || this->Vertex[i]->Index == -1) continue;
			indexes.push_back(Vertex[i]->Index);
		}
		return indexes;
	}

#pragma endregion

#pragma region  GENERATION

	//----------------------------------------------------------------------------------------
	DataMesh LinearMesh::GenerateMesh(std::string const& name, LinearParameters const& params, boundingBox& bounds, std::vector<BezierCurve*> curves)
	{
		GlobalParameters* paramsGeneration = new GlobalParameters(name, params);

		MeshVertex** grid = GenerateGrid(bounds, paramsGeneration);
		std::vector<MeshPoly*> polys = FilterGrid(grid, curves, bounds, paramsGeneration);
		DataMesh mesh = BuildDataMesh(grid, polys, paramsGeneration);

		// Clean
		polys.clear();

		if (grid != nullptr)
		{
			for (auto i = 0; i < paramsGeneration->RowCount; i++)
			{
				delete[] grid[i];
			}
			delete[] grid;
		}
		
		delete paramsGeneration;

		return mesh;
	}

#pragma endregion

#pragma region MESH PROCESS METHODS

	//----------------------------------------------------------------------------------------
	// Create the initial grid of polygon from the bounding box
	//----------------------------------------------------------------------------------------
	MeshVertex** LinearMesh::GenerateGrid(boundingBox & bounds, GlobalParameters* paramsGeneration)
	{
		MeshVertex** grid = nullptr;
		if (paramsGeneration->GetPrecision() <= 0.0f) return grid;

		const Vector2F topLeft = bounds.TopLeftPoint();
		const Vector2F topRight = bounds.TopRightPoint();
		const Vector2F bottomLeft = bounds.BottomLeftPoint();

		paramsGeneration->Precision = Vector2F::Magnitude(topLeft, bottomLeft) / paramsGeneration->GetPrecision();


		const int nColumn = int(ceil(Vector2F::Magnitude(topLeft, topRight) / paramsGeneration->Precision) + 3);
		const int nRow = int(ceil(Vector2F::Magnitude(topLeft, bottomLeft) / paramsGeneration->Precision) + 3);

		grid = new MeshVertex*[nRow];

		// Get the projection and keep the extreme point
		float r0 = sqrt(powf((topRight.x - topLeft.x), 2.0) + powf((topRight.y - topLeft.y), 2.0));
		float r1 = sqrt(powf((bottomLeft.x - topLeft.x), 2.0) + powf((bottomLeft.y - topLeft.y), 2.0));

		paramsGeneration->VectNormalH = Vector2F((topRight.x - topLeft.x) / r0, (topRight.y - topLeft.y) / r0);
		paramsGeneration->VectNormalV = Vector2F((bottomLeft.x - topLeft.x) / r1, (bottomLeft.y - topLeft.y) / r1);

		// Create the origin topleft position for create the gris
		Vector2F initPos = Vector2F(topLeft) - ( paramsGeneration->VectNormalH * paramsGeneration->Precision) - ( paramsGeneration->VectNormalV * paramsGeneration->Precision);
		paramsGeneration->GeneratedTopLeft = initPos;
		paramsGeneration->GeneratedTopRight = initPos + (paramsGeneration->VectNormalH * float(nColumn));
		paramsGeneration->GeneratedBottomRight = paramsGeneration->GeneratedTopRight + (paramsGeneration->VectNormalV *  float(nRow));
		paramsGeneration->GeneratedBottomLeft = initPos + (paramsGeneration->VectNormalV * float(nRow));

		// For each row set the postion of all column vertice.
		for (int row = 0; row < nRow; row++)
		{
			grid[row] = new MeshVertex[nColumn];

			for (int column = 0; column < nColumn; column++)
			{
				grid[row][column].Position = initPos + (paramsGeneration->VectNormalH *paramsGeneration->Precision * (float)column); // Go From left to right
			}
			initPos += (paramsGeneration->VectNormalV * paramsGeneration->Precision); // Go from top to bottom
		}

		paramsGeneration->ColumnCount = nColumn;
		paramsGeneration->RowCount = nRow;
		return grid;
	}

	//----------------------------------------------------------------------------------------
	std::vector<MeshPoly*> LinearMesh::FilterGrid(MeshVertex** initialGrid, const std::vector<BezierCurve*>& curves, boundingBox & bounds, GlobalParameters* paramsGeneration)
	{
		MeshPoly ** polys = new MeshPoly*[paramsGeneration->RowCount - 1];

		for (int row = 0; row < paramsGeneration->RowCount - 1; row++)
		{
			polys[row] = new MeshPoly[paramsGeneration->ColumnCount - 1];
			for (int column = 0; column < paramsGeneration->ColumnCount - 1; column++)
			{
				polys[row][column].Vertex[0] = &initialGrid[row][column];
				polys[row][column].Vertex[1] = &initialGrid[row][column + 1];
				polys[row][column].Vertex[2] = &initialGrid[row + 1][column + 1];
				polys[row][column].Vertex[3] = &initialGrid[row + 1][column];
			}
		}
		for (auto it = curves.cbegin(); it != curves.cend(); it++)
		{
			IdentificationContourPoly(polys, **it, bounds, paramsGeneration);
		}
		CleanAloneSplit(polys, paramsGeneration);
		return BuildPolyMesh(polys, paramsGeneration);
	}

	//----------------------------------------------------------------------------------------
	int OrientationPositive(Vector2F const& VectorPoint, Vector2F const& vectNorme)
	{
		float proj = vectNorme.x*VectorPoint.x + vectNorme.y*VectorPoint.y;
		return  (proj < 0) ? -1 : 1;
	}

	//----------------------------------------------------------------------------------------
	bool isPositive(Vector2F const& v1, Vector2F const& v2, Vector2F const& point)
	{
		float val = (v2.x - v1.x)*(point.y - v1.y) - (v2.y - v1.y)*(point.x - v1.x);
		return val >= 0;
	}

	//----------------------------------------------------------------------------------------
	// TO DO: split and optimize.
	//----------------------------------------------------------------------------------------
	void LinearMesh::IdentificationContourPoly(MeshPoly** polys, BezierCurve const& curve, boundingBox & bounds, GlobalParameters* paramsGeneration)
	{
		Vector2F * currentPoint = curve.GetCurve()[0];
		Vector2F * lastPoint;

		const Vector2F vectH = Vector2F(paramsGeneration->GeneratedTopRight.x - paramsGeneration->GeneratedTopLeft.x, paramsGeneration->GeneratedTopRight.y - paramsGeneration->GeneratedTopLeft.y);
		const Vector2F vectV = Vector2F(paramsGeneration->GeneratedBottomLeft.x - paramsGeneration->GeneratedTopLeft.x, paramsGeneration->GeneratedBottomLeft.y - paramsGeneration->GeneratedTopLeft.y);
		const float magnitudeH = Vector2F::Magnitude(paramsGeneration->GeneratedTopRight, paramsGeneration->GeneratedTopLeft);
		const float magnitudeV = Vector2F::Magnitude(paramsGeneration->GeneratedBottomLeft, paramsGeneration->GeneratedTopLeft);

		const Vector2F vectInitPoint = Vector2F(currentPoint->x - paramsGeneration->GeneratedTopLeft.x, currentPoint->y - paramsGeneration->GeneratedTopLeft.y);
		float valProjH = ((vectH.x*vectInitPoint.x) + (vectH.y*vectInitPoint.y)) / magnitudeH;
		float valProjV = ((vectV.x*vectInitPoint.x) + (vectV.y*vectInitPoint.y)) / magnitudeV;
		int row = int(valProjV / paramsGeneration->Precision);
		int column = int(valProjH / paramsGeneration->Precision);

		// Follow the Curve points
		for (int i = 1; i < curve.GetCurveSize(); i++)
		{
			lastPoint = currentPoint;
			currentPoint = curve.GetCurve()[i];
			
			Vector2F Point = Vector2F(currentPoint->x - paramsGeneration->GeneratedTopLeft.x, currentPoint->y - paramsGeneration->GeneratedTopLeft.y);
			float valProjH = ((vectH.x*Point.x) + (vectH.y*Point.y)) / magnitudeH;
			float valProjV = ((vectV.x*Point.x) + (vectV.y*Point.y)) / magnitudeV;
			int tmpRow = int(valProjV / paramsGeneration->Precision);
			int tmpColumn = int(valProjH / paramsGeneration->Precision);

			if(tmpRow != row)
			{
				// direction vers le bas
				if(tmpRow > row)
				{
					polys[row][column].BottomIntersection = *lastPoint;
					polys[row][column].SideSplit[2] = true;
					polys[tmpRow][column].SideSplit[0] = true;
				}
				else // direction vers le haut
				{
					polys[tmpRow][column].BottomIntersection = *currentPoint;
					polys[row][column].SideSplit[0] = true;
					polys[tmpRow][column].SideSplit[2] = true;
				}
				row = tmpRow;
			}

			if (tmpColumn != column)
			{
				// direction vers la droite
				if (tmpColumn > column)
				{
					polys[row][column].RigthIntersection = *lastPoint;
					polys[row][column].SideSplit[1] = true;
					polys[row][tmpColumn].SideSplit[3] = true;
				}
				else // direction vers la gauche
				{
					polys[row][tmpColumn].RigthIntersection = *currentPoint;
					polys[row][tmpColumn].SideSplit[1] = true;
					polys[row][column].SideSplit[3] = true;
				}
				column = tmpColumn;
			}
		}
	}

#pragma endregion

#pragma region FILTRATION

	//----------------------------------------------------------------------------------------------
	void LinearMesh::CleanAloneSplit(MeshPoly ** polys, GlobalParameters* paramsGeneration)
	{
		for (int row = 0; row < paramsGeneration->RowCount - 1; row++)
		{
			for (int column = 0; column < paramsGeneration->ColumnCount - 1; column++)
			{
				int indexMemory = -1;
				for (int i = 0; i < 4; i++)
				{
					if (!polys[row][column].SideSplit[i]) continue;
					if (indexMemory >= 0)
					{
						indexMemory = -1;
						break;
					}
					indexMemory = i;
				}
				if (indexMemory == -1) continue;

				polys[row][column].SideSplit[indexMemory] = false;

				if (indexMemory == 2 && row < paramsGeneration->RowCount - 1)
					polys[row + 1][column].SideSplit[0] = false;

				if (indexMemory == 3 && column - 1 > 0)
					polys[row][column - 1].SideSplit[1] = false;

				if (indexMemory == 0 && row - 1 > 0) 
					polys[row - 1][column].SideSplit[2] = false;

				if (indexMemory == 1 && column < paramsGeneration->ColumnCount - 1) 
					polys[row][column + 1].SideSplit[3] = false;
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	std::vector<MeshPoly*> LinearMesh::BuildPolyMesh(MeshPoly ** polys, GlobalParameters* paramsGeneration)
	{
		std::vector<MeshPoly*> polyFinal;
		// start parcours
		for (int row = 0; row < paramsGeneration->RowCount - 1; row++)
		{
			bool isSelected = false;
			bool isSelectedInversed = false;

			for (int column = 0; column < paramsGeneration->ColumnCount - 1; column++)
			{
				bool isAddedNormal = false;
				bool isAddedInversed = false;

				// test Bot
				// Focus on the bot to check if an intersection start the definition of the contour.
				if (polys[row][column].SideSplit[2])
				{
					isAddedNormal = true;
					isSelected = !isSelected;
				}

				isAddedNormal |= isSelected;

				// test top
				// Maybe the have detected the bottom intersection for out of the outline, 
				// but the next poly i included because they don't continue to the top.
				if (polys[row][column].SideSplit[0])
				{
					isAddedInversed = true;
					isSelectedInversed = !isSelectedInversed;
				}
				isAddedInversed |= isSelectedInversed;

				FixEdgeCaseThreeAndFiveVertice(polys, row, column, isSelected, isSelectedInversed);

				// Move vertex.
				if (polys[row][column].SideSplit[2] && polys[row][column].SideSplit[0])
				{
					if (isSelected) polys[row][column].Vertex[3]->Position = polys[row][column].BottomIntersection;
					if (!isSelected ) polys[row][column].Vertex[2]->Position = polys[row][column].BottomIntersection;
				}

				// Add poly.
				if (isAddedNormal && !polys[row][column].IsIncluded)
				{
					polys[row][column].IsIncluded = true;
					polyFinal.push_back(&polys[row][column]);
					continue;
				}

				if(isAddedInversed && !polys[row][column].IsIncluded)
				{
					polys[row][column].IsIncluded = true;
					polyFinal.push_back(&polys[row][column]);
				}
			}
		}

		for (int column = paramsGeneration->ColumnCount - 2; column >= 0 ; column--)
		{
			bool isSelected = false;
			bool isSelectedInversed = false;

			for (int row = 0; row < paramsGeneration->RowCount - 1; row++)
			{
				bool isAddedNormal = false;
				bool isAddedInversed = false;

				// test Bot
				// Focus on the bot to check if an intersection start the definition of the contour.
				if (polys[row][column].SideSplit[3])
				{
					isAddedNormal = true;
					isSelected = !isSelected;
				}
				isAddedNormal |= isSelected;

				// test top
				// Maybe the have detected the bottom intersection for out of the outline, 
				// but the next poly i included because they don't continue to the top.
				if (polys[row][column].SideSplit[1])
				{
					isAddedInversed = true;
					isSelectedInversed = !isSelectedInversed;
				}
				isAddedInversed |= isSelectedInversed;

				// Move vertex.
				if (polys[row][column].SideSplit[1] && polys[row][column].SideSplit[3])
				{
					if (isSelected) polys[row][column].Vertex[1]->Position = polys[row][column].RigthIntersection;
					if (!isSelected) polys[row][column].Vertex[2]->Position = polys[row][column].RigthIntersection;
				}
			}
		}
		return polyFinal;
	}

	//----------------------------------------------------------------------------------------------
	DataMesh LinearMesh::BuildDataMesh(MeshVertex** grid, std::vector<MeshPoly*> polys, GlobalParameters* paramsGeneration)
	{
		DataMesh dataMesh = DataMesh(paramsGeneration->GetName());
		if (grid == nullptr) return dataMesh;

		// Add vertices
		std::vector<Vector2F> vertice;
		int currentVertexIndex = 0;
		for (int row = 0; row < paramsGeneration->RowCount; row++)
		{
			for (int column = 0; column < paramsGeneration->ColumnCount; column++)
			{
				if (&grid[row][column] == nullptr) continue;

				grid[row][column].Index = currentVertexIndex;
				vertice.push_back(grid[row][column].Position);
				currentVertexIndex++;
			}
		}
		dataMesh.SetVertices(vertice);

		// Add faces
		for (auto poly : polys)
		{
			dataMesh.AddFace(poly->GetVertexIndexes());
		}

		return dataMesh;
	}

	//----------------------------------------------------------------------------------------
	// Temporary Method
	//----------------------------------------------------------------------------------------
	Vector2F LinearMesh::GetCenterSegment(Vector2F pos1, Vector2F pos2)
	{
		Vector2F center;
		center.x = (pos1.x + pos2.x) / 2.0f;
		center.y = (pos1.y + pos2.y) / 2.0f;
		return center;
	}

	//----------------------------------------------------------------------------------------
	void LinearMesh::FixEdgeCaseThreeAndFiveVertice(MeshPoly ** polys, int row, int column, bool isSelected, bool isSelectedInversed)
	{

		if (polys[row][column].SideSplit[2] && polys[row][column].SideSplit[1] && isSelected)
		{
			polys[row][column].Vertex[1]->Position = polys[row][column].RigthIntersection;
			polys[row][column].Vertex[3]->Position = polys[row][column].BottomIntersection;
			polys[row][column].Vertex[0]->Position = GetCenterSegment(polys[row][column].RigthIntersection, polys[row][column].BottomIntersection);
			return;

		}
		if (polys[row][column].SideSplit[2] && polys[row][column].SideSplit[3] && !isSelected)
		{
			polys[row][column].Vertex[0]->Position = polys[row][column-1].RigthIntersection;
			polys[row][column].Vertex[2]->Position = polys[row][column].BottomIntersection;
			polys[row][column].Vertex[1]->Position = GetCenterSegment(polys[row][column-1].RigthIntersection, polys[row][column].BottomIntersection);

		}
		if (polys[row][column].SideSplit[0] && polys[row][column].SideSplit[1] && isSelectedInversed)
		{
			polys[row][column].Vertex[2]->Position = polys[row][column].RigthIntersection;
			polys[row][column].Vertex[0]->Position = polys[row-1][column].BottomIntersection;
			polys[row][column].Vertex[3]->Position = GetCenterSegment(polys[row][column].RigthIntersection, polys[row-1][column].BottomIntersection);

		}
		if (polys[row][column].SideSplit[0] && polys[row][column].SideSplit[3] && !isSelectedInversed)
		{
			polys[row][column].Vertex[1]->Position = polys[row-1][column].BottomIntersection;
			polys[row][column].Vertex[3]->Position = polys[row][column-1].RigthIntersection;
			polys[row][column].Vertex[2]->Position = GetCenterSegment(polys[row-1][column].BottomIntersection, polys[row][column-1].RigthIntersection);
		}

		if (polys[row][column].SideSplit[2] && polys[row][column].SideSplit[3] && isSelected)
			polys[row][column].Vertex[3]->Position = GetCenterSegment(polys[row][column].BottomIntersection, polys[row][column-1].RigthIntersection);

		if (polys[row][column].SideSplit[2] && polys[row][column].SideSplit[1] && !isSelected)
			polys[row][column].Vertex[2]->Position = GetCenterSegment(polys[row][column].BottomIntersection, polys[row][column].RigthIntersection);

		if (polys[row][column].SideSplit[0] && polys[row][column].SideSplit[3] && isSelectedInversed)
			polys[row][column].Vertex[0]->Position = GetCenterSegment(polys[row-1][column].BottomIntersection, polys[row][column - 1].RigthIntersection);

		if (polys[row][column].SideSplit[0] && polys[row][column].SideSplit[1] && !isSelectedInversed)
			polys[row][column].Vertex[1]->Position = GetCenterSegment(polys[row-1][column].BottomIntersection, polys[row][column].RigthIntersection);

	}


#pragma endregion
	
}
