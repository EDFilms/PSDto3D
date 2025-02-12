//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file Mesh.cpp
//  @author Benjamin Drouin
//  @date 24-09-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "dataMesh.h"
#include "util/helpers.h"
#include <cassert>
#include <math.h>
#include <cfenv>

namespace mesh_generator
{
	//----------------------------------------------------------------------------------------------
	DataMesh::DataMesh(std::string const& name, float width, float height )
	{
		this->NameId = name;
		this->Width = width;
		this->Height = height;
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::CloneFrom( const DataMesh& that )
	{
		this->NameId = that.NameId;				// std::string NameId
		this->Width = that.Width;				// float Width
		this->Height = that.Height;				// float Height
		this->BoundsUV = that.BoundsUV;			// boundsUV BoundsUV
		this->Vertices = that.Vertices;			// std::vector<Vector2F>
		this->FaceSizes = that.FaceSizes;		// std::vector<int>
		this->FaceMarks = that.FaceMarks;		// std::vector<int>
		this->FaceVerts = that.FaceVerts;	// std::vector<int>
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::SetVertices(std::vector<Vector2F> const& vertices)
	{
		this->Vertices = vertices;
	}

	//----------------------------------------------------------------------------------------------
	bool DataMesh::AddFace(std::vector<int> const& vertices)
	{
		if (vertices.empty()) return false;

		this->FaceSizes.push_back(int(vertices.size()));
		this->FaceMarks.push_back(int(this->FaceVerts.size()));
		this->FaceVerts.insert(this->FaceVerts.end(), vertices.begin(), vertices.end());
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool DataMesh::AddFace(int vertices[], int const size)
	{

		this->FaceSizes.push_back(size);
		this->FaceMarks.push_back(int(this->FaceVerts.size()));
		for (int i = 0; i < size; ++i)
		{
			this->FaceVerts.push_back(vertices[i]);
		}
		return true;
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::SetValues(std::vector<Vector2F>& vertices, std::vector<int>& faceSizes, std::vector<int>& faceMarks, std::vector<int>& faceVerts)
	{
		//this->Vertices = vertices;
		//this->FacesCount = facesCount;
		//this->FacesIndices = facesIndices;
		// TODO: diagnose why swap() is slower than assignment operator=()
		this->Vertices.swap( vertices );
		this->FaceSizes.swap( faceSizes );
		this->FaceMarks.swap( faceMarks );
		this->FaceVerts.swap( faceVerts );
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::ClearFaces()
	{
		this->FaceSizes.clear();
		this->FaceMarks.clear();
		this->FaceVerts.clear();
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::ClearVertices()
	{
		this->Vertices.clear();
		this->FaceSizes.clear();
		this->FaceMarks.clear();
		this->FaceVerts.clear();
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::SetName( const std::string& name)
	{
		this->NameId = name;
	}

	//----------------------------------------------------------------------------------------------
	std::string DataMesh::GetName() const
	{
		return this->NameId;
	}

	//----------------------------------------------------------------------------------------------
	bool DataMesh::SetBoundsUV(const boundsUV bounds)
	{
		this->BoundsUV = bounds;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::SetWidth( float width )
	{
		this->Width = width;
	}

	//----------------------------------------------------------------------------------------------
	void DataMesh::SetHeight( float height )
	{
		this->Height = height;
	}


#pragma region MESH SPLINE

	//----------------------------------------------------------------------------------------------
	DataSpline::DataSpline(const std::string& name, float width, float height)
	{
		NameId = name;
		Width = width;
		Height = height;
	}

	//----------------------------------------------------------------------------------------------
	DataSpline::DataSpline( DataSpline& that )
	{
		this->NameId    = that.NameId;
		this->Width     = that.Width;
		this->Height     = that.Height;
		this->Curves    = that.Curves;
		this->BoundsUV  = that.BoundsUV;
		this->Precision = that.Precision;
		that.Curves.clear(); // take ownership, no reference counting
	}

	//----------------------------------------------------------------------------------------------
	DataSpline::~DataSpline()
	{
		// Curves array: Delete the items, already took ownership of each with AddCurve()
		for( std::vector<BezierCurve*>::iterator it=Curves.begin(); it!=Curves.end(); it++ )
		{
			delete (*it);
		}
		// Clear arrays, doesn't delete the members
		Curves.clear();
	}

	//----------------------------------------------------------------------------------------------
	void DataSpline::CloneFrom( const DataSpline& that )
	{
		this->NameId = that.NameId;			// std::string NameId
		this->Width = that.Width;			// float Width
		this->Height = that.Height;			// float Height
		this->BoundsUV = that.BoundsUV;		// boundsUV BoundsUV
		for( BezierCurve* curve_old : that.Curves )
		{
			if( curve_old==nullptr )
				Curves.push_back(nullptr);
			else
			{
				BezierCurve* curve_new = new BezierCurve();
				curve_new->CloneFrom(*curve_old);
				this->Curves.push_back( curve_new ); // std::vector<BezierCurve*> Curves
			}
		}
		this->Precision = that.Precision;	// float Precision
	}

	//----------------------------------------------------------------------------------------------
	std::string DataSpline::GetName() const
	{
		return this->NameId;
	}

	//----------------------------------------------------------------------------------------------
	// Takes ownership of pointer, will delete in destructor
	bool DataSpline::AddCurve(BezierCurve* curve)
	{
		Curves.push_back(curve);
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool DataSpline::SetBoundsUV(const boundsUV bounds)
	{
		this->BoundsUV = bounds;
		return true;
	}

	//----------------------------------------------------------------------------------------------
	bool DataSpline::SetPrecision(float precision)
	{
		this->Precision = precision;
		return true;
	}


#pragma endregion

#pragma region MESH SURFACE

	//----------------------------------------------------------------------------------------------
	DataSurface::DataSurface( DataSurface& that ) : Mesh(nullptr), Spline(nullptr)
	{
		(*this) = that; // take ownership, no reference counting
	}

	//----------------------------------------------------------------------------------------------
	DataSurface::DataSurface( DataMesh& mesh ) : Spline(nullptr)
	{
		Mesh = new DataMesh(mesh);
	}

	//----------------------------------------------------------------------------------------------
	DataSurface::DataSurface( DataSpline& spline ) : Mesh(nullptr)
	{
		Spline = new DataSpline(spline);
	}

	//----------------------------------------------------------------------------------------------
	DataSurface::~DataSurface()
	{
		Free();
	}

	//----------------------------------------------------------------------------------------------
	void DataSurface::Free()
	{
		if( Mesh!=nullptr )
		{
			delete Mesh;
		}
		if( Spline!=nullptr )
		{
			delete Spline;
		}
		Mesh = nullptr;
		Spline = nullptr;
	}

	//----------------------------------------------------------------------------------------------
	void DataSurface::CloneFrom( const DataSurface& that )
	{
		Free();
		if( that.Mesh!=nullptr )
		{
			this->Mesh = new DataMesh();
			this->Mesh->CloneFrom( *(that.Mesh) );
		}
		if( that.Spline!=nullptr )
		{
			this->Spline = new DataSpline( that.Spline->GetName(), that.Spline->GetWidth(), that.Spline->GetHeight() );
			this->Spline->CloneFrom( *(that.Spline) );
		}
	}

	//----------------------------------------------------------------------------------------------
	std::string DataSurface::GetName() const
	{
		if( Mesh!=nullptr )
		{
			return Mesh->GetName();
		}
		if( Spline!=nullptr )
		{
			return Spline->GetName();
		}
		// ERROR: should never happen
		assert( "ERROR: DataSurface::GetName(), item is null" == 0 );
		return std::string("");
	}

	//----------------------------------------------------------------------------------------------
	bool DataSurface::SetBoundsUV( const boundsUV bounds)
	{
		if( Mesh!=nullptr )
		{
			return Mesh->SetBoundsUV(bounds);
		}
		else if( Spline!=nullptr )
		{
			return Spline->SetBoundsUV(bounds);
		}
		// ERROR: should never happen
		assert( "ERROR: DataSurface::GetBoundingBoxUV(), item is null" == 0 );
		return false;
	}

	//----------------------------------------------------------------------------------------------
	boundsUV DataSurface::GetBoundsUV() const
	{
		if( Mesh!=nullptr )
		{
			return Mesh->GetBoundsUV();
		}
		if( Spline!=nullptr )
		{
			return Spline->GetBoundsUV();
		}
		// ERROR: should never happen
		assert( "ERROR: DataSurface::GetBoundingBoxUV(), item is null" == 0 );
		return boundsUV();
	}

	//----------------------------------------------------------------------------------------------
	boundsPixels DataSurface::GetBoundsPixels() const
	{
		boundsUV bounds = GetBoundsUV();
		// Maybe verify fegetround()==FE_TONEAREST
		int imageWidth  = lrint(GetWidth());
		int imageHeight = lrint(GetHeight());
		int left = (int)floor( bounds.TopLeftPoint().x * imageWidth );
		int right = (int)ceil( bounds.BottomRightPoint().x * imageWidth );
		// TopLeft is actually BottomLeft and BottomRight is actually TopRight, kinda
		int bottom = (int)floor( bounds.TopLeftPoint().y * imageHeight );
		int top = (int)ceil( bounds.BottomRightPoint().y * imageHeight );
		int width = abs(right-left);
		int height = abs(bottom-top);
		return boundsPixels(left, top, width, height);

	}

	//----------------------------------------------------------------------------------------------
	float DataSurface::GetWidth() const
	{
		if( Mesh!=nullptr )
		{
			return Mesh->GetWidth();
		}
		if( Spline!=nullptr )
		{
			return Spline->GetWidth();
		}
		// ERROR: should never happen
		assert( "ERROR: DataSurface::GetWidth(), item is null" == 0 );
		return 1.0f;
	}

	//----------------------------------------------------------------------------------------------
	float DataSurface::GetHeight() const
	{
		if( Mesh!=nullptr )
		{
			return Mesh->GetHeight();
		}
		if( Spline!=nullptr )
		{
			return Spline->GetHeight();
		}
		// ERROR: should never happen
		assert( "ERROR: DataSurface::GetWidth(), item is null" == 0 );
		return 1.0f;
	}

	DataSurface& DataSurface::operator=(DataSurface& that)
	{
		Free(); // delete old mesh if any
		this->Mesh = that.Mesh;
		this->Spline = that.Spline;
		that.Mesh = nullptr; // take ownership, no reference counting
		that.Spline = nullptr; // take ownership, no reference counting
		return *this;
	}

#pragma endregion

}