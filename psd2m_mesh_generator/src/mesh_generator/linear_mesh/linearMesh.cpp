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
	// Helper function
	float min( float a, float b )
	{ return (a<=b? a:b); }

	//----------------------------------------------------------------------------------------
	// Helper function
	float max( float a, float b )
	{ return (a>=b? a:b); }

	//----------------------------------------------------------------------------------------
	// Helper function
	// Modulo guaranteed to returning non-negative number; count should be positive
	int wrap( int i, int count )
	{
		while( i<0 )
			i = (count+i);
		return (i % count);
	}


	//----------------------------------------------------------------------------------------
	// Helper function
	// Determines on which side of a 2D directed line segment (from v1 to v2) a 2D point (p) lies,
	// Positive means left side, Negative means right side
	inline float LineSegmentSide( const Vector2F& v1, const Vector2F& v2, const Vector2F& p )
	{
		float a_x = v2.x-v1.x, a_y = v2.y-v1.y; // vector a is the offset from v1 -> v2
		float b_x = p.x-v1.x,  b_y = p.y-v1.y;  // vector b is the offset from v1 -> p
		return ( (a_x * b_y) - (a_y * b_x) );   // Z coordinate of Cross Product in 3D
	}

	//----------------------------------------------------------------------------------------
	// Helper function
	// Determines the angle of point p about point a, clockwise from unit vector u
	// For example, if p lies along vector u from point a, it returns 0, or if along -u, it returns -pi
	inline float AngleAboutPoint( const Vector2F& p, const Vector2F& a, const Vector2F& u )
	{
		// "x coordinate" is distance along direction u
		// "y coordinate" is distance along perpendicular direction v
		// result of arctangent of x and y coordinate
		Vector2F v( u.y, -u.x ); // perpendicular unit vector
		Vector2F s = (p-a); // offset vector from a
		float x = (s.x*u.x) + (s.y*u.y); // vector dot unit vector = distance in unit vector direction
		float y = (s.x*v.x) + (s.y*v.y); // vector dot unit vector = distance in perp vector direction
		return atan2(y,x);
	}

	//----------------------------------------------------------------------------------------
	// True if the given point is within the boundaries of the polygon, False otherwise
	// Assumes the polygon is convex, fails for concave polygons
	// Assumes the vertices are listed in clockwise, or in counterclockwise order
	bool MeshPoly::ContainsPoint( const Vector2F& p )
	{
		// Whether point p lies to the left or right, as seen when travelling from vertex 3 to 0
		float sideCur = LineSegmentSide( Vertex[3]->Position, Vertex[0]->Position, p );
		for( int i=0; i<3; i++ )
		{
			// Whether point p lies to the left or right, as seen when travelling from vertex i to i+1
			float sideNext = LineSegmentSide( Vertex[i]->Position, Vertex[i+1]->Position, p );
			if( (sideCur*sideNext) < 0 ) // if one is positive and the other negative...
				return false; // ...different sides; the point is outside polygon

			// Ensure nonzero sign (unless both are zero); only the sign matters, not magnitude
			sideCur += sideNext;
		}
		return true; // point lies on the same side of all polygon edges; point is inside polygon
	}

	//----------------------------------------------------------------------------------------
	// Searches the curve, adding points bounded by the poly to the curvePoints list
	// Walks using the given index and direction, stops when outside the poly bounds
	void MeshPoly::AddCurvePoints( const BezierCurve& curve, int indexStart, int indexDir )
	{
		int count = curve.GetCurveSize();
		for( int index=indexStart, iters=0; iters<count; index+=indexDir, iters++ )
		{
			index = wrap( index, count ); // index can wrap around
			// If past the region of the curve contained by the polygon, done
			if( !ContainsPoint( curve.GetPoint(index) ) )
				return;
			// Else add the curve point to the list
			curvePoints.insert( curve.GetPoint(index) );
		}
	}

	//----------------------------------------------------------------------------------------
	// Return a new position for the indexed vertex, which tucks against the
	// curve points inside the polygon as closely as possible
	Vector2F MeshPoly::FitVertexToCurvePoints( int index )
	{
		Vector2F vertP = Vertex[index]->Position;
		if( curvePoints.size()<=0 )
			return vertP; // cannot fit, no curve points to fit around

		Vector2F vertA = Vertex[wrap(index-1,4)]->Position;
		Vector2F vertB = Vertex[wrap(index+1,4)]->Position;
		Vector2F vecA = vertP-vertA;
		Vector2F vecB = vertP-vertB;
		if( vecA.Magnitude()==0 || vecB.Magnitude()==0 )
			return vertP; // cannot fit, degenerate polygon

		Vector2F unitA = vecA / vecA.Magnitude(); // fails if P and A in same position, degenerate polygon
		Vector2F unitB = vecB / vecB.Magnitude(); // fails if P and B in same position, degenerate polygon
		float angA = float( 2.0f*PI); // search for smallest angle clockwise, start high and find minimum
		float angB = float(-2.0f*PI); // search for smallest angle counterclockwise, start low and find maximum
		for( auto it = curvePoints.cbegin(); it != curvePoints.cend(); it++ )
		{
			angA = min( angA, AngleAboutPoint( *it, vertA, unitA ) );
			angB = max( angB, AngleAboutPoint( *it, vertB, unitB ) );
		}
		Vector2F perpA( unitA.y, -unitA.x );
		Vector2F perpB( unitB.y, -unitB.x );
		Vector2F rayA = (unitA*cos(angA)) + (perpA*sin(angA));
		Vector2F rayB = (unitB*cos(angB)) + (perpB*sin(angB));

		// Find the intersection of rayA starting at vertA, with rayB starting at vertB
		// Messy linear algebra solving yields the following:
		// offsetA * (rayA.y * rayB.x - rayA.x * rayB.y) = (c.y * rayB.x - c.x * rayB.y)
		// where c = vertB-vertA
		Vector2F vecC = vertB-vertA;
		float numerator   = (vecC.y * rayB.x) - (vecC.x * rayB.y);
		float denominator = (rayA.y * rayB.x) - (rayA.x * rayB.y);
		if( denominator==0 )
			return vertP; // cannot fit, the math exploded

		float offsetA = numerator/denominator;
		if( offsetA<0 )
			offsetA = 0;
		if( offsetA>vecA.Magnitude() )
			offsetA = vecA.Magnitude();
		return (vertA + (rayA * offsetA));
	}

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
	DataMesh LinearMesh::GenerateMesh(const std::string& name, const LinearMeshParameters& params,
        boundsUV& bounds, const std::vector<BezierCurve*>& curves, int width, int height, ProgressTask& progressTask)
	{
		LinearMeshAlgoParameters* paramsGeneration = new LinearMeshAlgoParameters(name, params);

		MeshVertex** grid = GenerateGrid(bounds, paramsGeneration);
		std::vector<MeshPoly*> polys = FilterGrid(grid, curves, bounds, paramsGeneration);

		progressTask.SetValueAndUpdate( 0.50f );

		DataMesh mesh = BuildDataMesh(grid, polys, paramsGeneration, width, height);
		// Do not set XFormUV; remains as default, and may be set later if needed to support atlasing
		// spline.SetXFormUV(xform);

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

	DataSpline LinearMesh::GenerateSpline(const std::string& name, const LinearMeshParameters& params,
        boundsUV& bounds, const std::vector<BezierCurve*>& curves, int width, int height, ProgressTask& progressTask)
	{
		LinearMeshAlgoParameters* paramsGeneration = new LinearMeshAlgoParameters(name, params);

		const Vector2F initPos(0.0,0.0);
		const Vector2F topLeft = bounds.TopLeftPoint();
		const Vector2F topRight = bounds.TopRightPoint();
		const Vector2F bottomLeft = bounds.BottomLeftPoint();

		paramsGeneration->Precision = Vector2F::Magnitude(topLeft, bottomLeft) / paramsGeneration->GetPrecision();

		// Get the projection and keep the extreme point
		float r0 = sqrt(powf((topRight.x - topLeft.x), 2.0) + powf((topRight.y - topLeft.y), 2.0));
		float r1 = sqrt(powf((bottomLeft.x - topLeft.x), 2.0) + powf((bottomLeft.y - topLeft.y), 2.0));

		// TODO: bounds uses high numbers as "top", low as "bottom", but for Photoshop curves upper-left is [0,0], lower right [1,1] ...
		// Vertical direction thus calculated as (topLeft-bottomLeft) ... DIFFERENT FROM GenerateMesh()
		paramsGeneration->VectNormalH = Vector2F((topRight.x - topLeft.x) / r0, (topRight.y - topLeft.y) / r0);
		paramsGeneration->VectNormalV = Vector2F((topLeft.x - bottomLeft.x) / r1, (topLeft.y - bottomLeft.y) / r1);

		DataSpline spline(name, (float)width, (float)height);
		spline.SetBoundsUV(bounds);
		// Do not set XFormUV; remains as default, and may be set later if needed to support atlasing
		// spline.SetXFormUV(xform);

		spline.SetPrecision(paramsGeneration->GetPrecision());
		for (auto curve : curves)
		{
			for (int i=0; i<curve->GetPathSize(); i++)
			{
				PathPoints p = curve->GetPathPoints(i);
				p.AnchorPoint = initPos + (paramsGeneration->VectNormalH * p.AnchorPoint.x) + (paramsGeneration->VectNormalV * (1.0f-p.AnchorPoint.y));
				p.SegIn       = initPos + (paramsGeneration->VectNormalH * p.SegIn.x)       + (paramsGeneration->VectNormalV * (1.0f-p.SegIn.y));
				p.SegOut      = initPos + (paramsGeneration->VectNormalH * p.SegOut.x)      + (paramsGeneration->VectNormalV * (1.0f-p.SegOut.y));
				curve->SetPathPoints(i,p);
			}
			spline.AddCurve(curve);
		}

		delete paramsGeneration;
		return spline;
	}

#pragma endregion

#pragma region MESH PROCESS METHODS

	//----------------------------------------------------------------------------------------
	// Create the initial grid of polygon from the bounding box
	//----------------------------------------------------------------------------------------
	MeshVertex** LinearMesh::GenerateGrid(boundsUV & bounds, LinearMeshAlgoParameters* paramsGeneration)
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
		// Spacer distances are equal to a portion of one grid cell
		paramsGeneration->VectSpacerH = Vector2F(0,0); //paramsGeneration->VectNormalH / (10 * paramsGeneration->GetPrecision());
		paramsGeneration->VectSpacerV = Vector2F(0,0); //paramsGeneration->VectNormalV / (10 * paramsGeneration->GetPrecision());

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
	std::vector<MeshPoly*> LinearMesh::FilterGrid(MeshVertex** initialGrid, const std::vector<BezierCurve*>& curves, boundsUV & bounds, LinearMeshAlgoParameters* paramsGeneration)
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
	void LinearMesh::IdentificationContourPoly(MeshPoly** polys, BezierCurve const& curve, boundsUV & bounds, LinearMeshAlgoParameters* paramsGeneration)
	{
		Vector2F * currentPoint = curve.GetCurves()[0];
		Vector2F * lastPoint;

		const Vector2F vectH = Vector2F(paramsGeneration->GeneratedTopRight.x - paramsGeneration->GeneratedTopLeft.x, paramsGeneration->GeneratedTopRight.y - paramsGeneration->GeneratedTopLeft.y);
		const Vector2F vectV = Vector2F(paramsGeneration->GeneratedBottomLeft.x - paramsGeneration->GeneratedTopLeft.x, paramsGeneration->GeneratedBottomLeft.y - paramsGeneration->GeneratedTopLeft.y);
		const float magnitudeH = Vector2F::Magnitude(paramsGeneration->GeneratedTopRight, paramsGeneration->GeneratedTopLeft);
		const float magnitudeV = Vector2F::Magnitude(paramsGeneration->GeneratedBottomLeft, paramsGeneration->GeneratedTopLeft);

		// Distance of point upper-left of grid, as a vector
		const Vector2F vectInitPoint = Vector2F(currentPoint->x - paramsGeneration->GeneratedTopLeft.x, currentPoint->y - paramsGeneration->GeneratedTopLeft.y);
		// Distance of point from upper-left of grid, measured along grid horizontal direction
		float valProjH = ((vectH.x*vectInitPoint.x) + (vectH.y*vectInitPoint.y)) / magnitudeH;
		// Distance of point from upper-left of grid, measured along grid horizontal direction
		float valProjV = ((vectV.x*vectInitPoint.x) + (vectV.y*vectInitPoint.y)) / magnitudeV;

		int row = int(valProjV / paramsGeneration->Precision);
		int column = int(valProjH / paramsGeneration->Precision);

		// Follow the Curve points
		for (int i = 1; i < curve.GetCurveSize(); i++)
		{
			lastPoint = currentPoint;
			currentPoint = curve.GetCurves()[i];
			
			// Distance of point upper-left of grid, as a vector
			Vector2F Point = Vector2F(currentPoint->x - paramsGeneration->GeneratedTopLeft.x, currentPoint->y - paramsGeneration->GeneratedTopLeft.y);
			// Distance of point from upper-left of grid, measured along grid horizontal direction
			float valProjH = ((vectH.x*Point.x) + (vectH.y*Point.y)) / magnitudeH;
			// Distance of point from upper-left of grid, measured along grid horizontal direction
			float valProjV = ((vectV.x*Point.x) + (vectV.y*Point.y)) / magnitudeV;

			int tmpRow = int(valProjV / paramsGeneration->Precision);
			int tmpColumn = int(valProjH / paramsGeneration->Precision);

			if(tmpRow != row)
			{
				// direction vers le bas
				if(tmpRow > row)
				{	// new grid cell is below the old one
					polys[row][column].BottomIntersection = *lastPoint; // store bottom intersection of old grid cell (we don't track top intersections)
					polys[row][column].SideSplit[2] = true; // curve crosses the bottom of the old grid cell, order is (top, right, bottom, left)
					polys[tmpRow][column].SideSplit[0] = true; // curve crosses the top of the new grid cell, order is (top, right, bottom, left)
				}
				else // direction vers le haut
				{	// new grid cell is above the old one
					polys[tmpRow][column].BottomIntersection = *currentPoint; // store bottom intersection of new grid cell (we don't track top intersections)
					polys[row][column].SideSplit[0] = true; // curve crosses the top of the old grid cell, order is (top, right, bottom, left)
					polys[tmpRow][column].SideSplit[2] = true;  // curve crosses the bottom of the new grid cell, order is (top, right, bottom, left)
				}
				polys[row][column].AddCurvePoints(curve, i-1, -1); // populate old grid cell with relevant points starting from i-1, searching backwards
				polys[tmpRow][column].AddCurvePoints(curve, i, 1); // populate new grid cell with relevant points starting from i, searching forwards
				row = tmpRow;
			}

			if (tmpColumn != column)
			{
				// direction vers la droite
				if (tmpColumn > column)
				{
					polys[row][column].RigthIntersection = *lastPoint; // store right intersection of old grid cell (we don't track left intersections)
					polys[row][column].SideSplit[1] = true; // curve crosses the right of the old grid cell, order is (top, right, bottom, left)
					polys[row][tmpColumn].SideSplit[3] = true; // curve crosses the left of the new grid cell, order is (top, right, bottom, left)
				}
				else // direction vers la gauche
				{
					polys[row][tmpColumn].RigthIntersection = *currentPoint; // store right intersection of new grid cell (we don't track top intersections)
					polys[row][column].SideSplit[3] = true; // curve crosses the left of the old grid cell, order is (top, right, bottom, left)
					polys[row][tmpColumn].SideSplit[1] = true; // curve crosses the right of the new grid cell, order is (top, right, bottom, left)
				}
				polys[row][column].AddCurvePoints(curve, i-1, -1); // populate old grid cell with relevant points starting from i-1, searching backwards
				polys[row][tmpColumn].AddCurvePoints(curve, i, 1); // populate new grid cell with relevant points starting from i, searching forwards
				column = tmpColumn;
			}
		}
	}

#pragma endregion

#pragma region FILTRATION

	//----------------------------------------------------------------------------------------------
	// If any grid cell has only one edge which intersects a curve, disregard that intersection,
	// and also disregard that intersection on the adjacent grid cell to the other side of the edge
	void LinearMesh::CleanAloneSplit(MeshPoly ** polys, LinearMeshAlgoParameters* paramsGeneration)
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
	std::vector<MeshPoly*> LinearMesh::BuildPolyMesh(MeshPoly ** polys, LinearMeshAlgoParameters* paramsGeneration)
	{
		std::vector<MeshPoly*> polyFinal;
		// Fit verts to intersections with some extra padding space
		Vector2F& spacerH = paramsGeneration->VectSpacerH;
		Vector2F& spacerV = paramsGeneration->VectSpacerV;
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

				FixEdgeCaseThreeAndFiveVertice(polys, row, column, isSelected, isSelectedInversed, paramsGeneration);

				// Move vertex.
				if (polys[row][column].SideSplit[2] && polys[row][column].SideSplit[0])
				{
					if (isSelected) polys[row][column].Vertex[3]->Position = polys[row][column].BottomIntersection - spacerH;
					if (!isSelected ) polys[row][column].Vertex[2]->Position = polys[row][column].BottomIntersection + spacerH;
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
					if (isSelected) polys[row][column].Vertex[1]->Position = polys[row][column].RigthIntersection - spacerV;
					if (!isSelected) polys[row][column].Vertex[2]->Position = polys[row][column].RigthIntersection + spacerV;
				}
			}
		}
		return polyFinal;
	}

	//----------------------------------------------------------------------------------------------
	DataMesh LinearMesh::BuildDataMesh(MeshVertex** grid, std::vector<MeshPoly*> polys, LinearMeshAlgoParameters* paramsGeneration,
		int width, int height )
	{
		DataMesh dataMesh = DataMesh(paramsGeneration->GetName(), (float)width, (float)height);
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
	// isSelected: first grid cell, or nth grid cell but not the last one, in a left to right run of polys,
	//   where a run starts and ends with cells with a whose BOTTOM edge intersects a curve
	// isSelected: first grid cell, or nth grid cell but not the last one, in a left to right run of polys,
	//   where a run starts and ends with cells with a whose TOP edge intersects a curve
	void LinearMesh::FixEdgeCaseThreeAndFiveVertice(MeshPoly ** polys, int row, int column, bool isSelected, bool isSelectedInversed, LinearMeshAlgoParameters* paramsGeneration)
	{
		// For grid cell corners, order is (top, right, bottom, left)

		// Fit verts to intersections with some extra padding space
		Vector2F& spacerH = paramsGeneration->VectSpacerH;
		Vector2F& spacerV = paramsGeneration->VectSpacerV;

		// grid cell has intersections to bottom and right, and is within a bottom-edge-counted run of cells;
		// case implies the lower-right corner of cell is within the surface interior, so adjust the upper-left corner
		if (polys[row][column].SideSplit[2] && polys[row][column].SideSplit[1] && isSelected)
		{
			polys[row][column].Vertex[0]->Position = polys[row][column].FitVertexToCurvePoints(0);
			polys[row][column].Vertex[1]->Position = polys[row][column].RigthIntersection - spacerV;
			polys[row][column].Vertex[3]->Position = polys[row][column].BottomIntersection - spacerH;
		}

		// grid cell has intersections to bottom and left, and at the end of bottom-edge-counted run of cells;
		// case implies the lower-left corner of cell is within the surface interior, so adjust the upper-right corner
		if (polys[row][column].SideSplit[2] && polys[row][column].SideSplit[3] && !isSelected)
		{
			polys[row][column].Vertex[1]->Position = polys[row][column].FitVertexToCurvePoints(1);
			polys[row][column].Vertex[0]->Position = polys[row][column-1].RigthIntersection - spacerV;
			polys[row][column].Vertex[2]->Position = polys[row][column].BottomIntersection + spacerH;
		}

		// grid cell has intersections to top and right, and within a top-edge-counted run of cells;
		// case implies the upper-right corner of cell is within the surface interior, so adjust the lower-left corner
		if (polys[row][column].SideSplit[0] && polys[row][column].SideSplit[1] && isSelectedInversed)
		{
			polys[row][column].Vertex[3]->Position = polys[row][column].FitVertexToCurvePoints(3);
			polys[row][column].Vertex[2]->Position = polys[row][column].RigthIntersection + spacerV;
			polys[row][column].Vertex[0]->Position = polys[row-1][column].BottomIntersection - spacerH;
		}

		// grid cell has intersections to top and left, and at the end of a top-edge-counted run of cells;
		// case implies the upper-left corner of cell is within the surface interior, so adjust the lower-right corner
		if (polys[row][column].SideSplit[0] && polys[row][column].SideSplit[3] && !isSelectedInversed)
		{
			polys[row][column].Vertex[2]->Position = polys[row][column].FitVertexToCurvePoints(2);
			polys[row][column].Vertex[3]->Position = polys[row][column-1].RigthIntersection + spacerV;
			polys[row][column].Vertex[1]->Position = polys[row-1][column].BottomIntersection + spacerH;
		}

		// grid cell has intersections to bottom and left, and is within a bottom-edge-counted run of cells;
		// special case implies the upper-right corner of cell is within the surface interior, so adjust the lower-left corner
		if (polys[row][column].SideSplit[2] && polys[row][column].SideSplit[3] && isSelected)
			polys[row][column].Vertex[3]->Position = polys[row][column].FitVertexToCurvePoints(3);

		// grid cell has intersections to bottom and right, and at the end of a bottom-edge-counted run of cells;
		// special case implies the upper-left corner of cell is within the surface interior, so adjust the lower-right corner
		if (polys[row][column].SideSplit[2] && polys[row][column].SideSplit[1] && !isSelected)
			polys[row][column].Vertex[2]->Position = polys[row][column].FitVertexToCurvePoints(2);

		// grid cell has intersections to top and left, and within a top-edge-counted run of cells;
		// special case implies the lower-right corner of cell is within the surface interior, so adjust the upper-left corner
		if (polys[row][column].SideSplit[0] && polys[row][column].SideSplit[3] && isSelectedInversed)
			polys[row][column].Vertex[0]->Position = polys[row][column].FitVertexToCurvePoints(0);

		// grid cell has intersections to top and right, and at the end of a top-edge-counted run of cells;
		// special case implies the lower-left corner of cell is within the surface interior, so adjust the upper-right corner
		if (polys[row][column].SideSplit[0] && polys[row][column].SideSplit[1] && !isSelectedInversed)
			polys[row][column].Vertex[1]->Position = polys[row][column].FitVertexToCurvePoints(1);

	}


#pragma endregion
	
}
