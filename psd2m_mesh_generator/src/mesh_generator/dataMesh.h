//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file Mesh.h
//  @author Benjamin Drouin
//  @date 24-09-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef DATA_MESH_H
#define DATA_MESH_H

#include "util/math_2D.h"
#include "util/bounds_2D.h"
#include "linear_mesh/bezierCurve.h"
#include <vector>

using namespace util;

namespace mesh_generator
{

	//----------------------------------------------------------------------------------------
	// An explicit mesh consisting of vertices and faces
	struct DataMesh
	{
		DataMesh() : NameId(""), Width(0), Height(0) {}
		DataMesh(const std::string& name, float width, float height);

		void CloneFrom( const DataMesh& that );

		void SetVertices(std::vector<Vector2F> const& vertices);
		bool AddFace(std::vector<int> const& vertices);
		bool AddFace(int vertices[], int size);
		void SetValues(std::vector<Vector2F>& vertices,
			std::vector<int>& faceSizes, std::vector<int>& faceMarks, std::vector<int>& faceIndices);

		void ClearFaces();
		void ClearVertices();

		int GetVerticesCount() const { return int(Vertices.size()); }
		int GetFacesCount() const { return int(FaceSizes.size()); }
		int GetFaceSizesCount() const { return GetFacesCount(); }
		int GetFaceVertsCount() const { return int(FaceVerts.size()); }
		const std::vector<Vector2F>& GetVertices() const { return Vertices; }		// position of each vert
		const std::vector<int>& GetFaceSizes() const { return FaceSizes; }		// number of verts per face
		const std::vector<int>& GetFaceMarks() const { return FaceMarks; }		// offset to each face's first vert in faceVerts list
		const std::vector<int>& GetFaceVerts() const { return FaceVerts; }	// vertex indices, contiguous list
		void SetName( const std::string& name );
		std::string GetName() const;
		bool SetBoundsUV( const boundsUV bounds);
		boundsUV GetBoundsUV() const { return BoundsUV; } // measured in UV coordinates
		void SetWidth( float width ); // measured in pixels
		float GetWidth() const { return Width; } // measured in pixels
		void SetHeight( float height ); // measured in pixels
		float GetHeight() const { return Height; } // measured in pixels

	private:
		std::string NameId; // for debugging and for object naming in Maya scene
		float Width, Height;
		boundsUV BoundsUV;

		std::vector<Vector2F> Vertices;
		std::vector<int> FaceSizes; // number of verts per face
		std::vector<int> FaceMarks; // offset to first face vert in FaceVerts list
		std::vector<int> FaceVerts; // vertex indices per face, contiguous list
	};


	//----------------------------------------------------------------------------------------
	// An implicit mesh consisting of planar splines, converted to explicit mesh later
	struct DataSpline
	{
		DataSpline( const std::string& name, float width, float height );
		// Copy constructor transfers ownership of pointers, no reference counting
		// TODO: Add reference counting if arbitrary copies need to be made
		DataSpline( DataSpline& that ); // override copy constructor for ownership handling
		~DataSpline();

		void CloneFrom( const DataSpline& that );

		bool AddCurve( BezierCurve* curve); // takes ownership of pointer
		bool SetPrecision( float Precision );

		const std::vector<BezierCurve*> GetCurves() const { return Curves; }
		float GetPrecision() const { return Precision; }
		std::string GetName() const;
		bool SetBoundsUV( const boundsUV bounds);
		boundsUV GetBoundsUV() const { return BoundsUV; } // measured in UV coordinates
		float GetWidth() const { return Width; } // measured in pixels
		float GetHeight() const { return Height; } // measured in pixels
	private:
		std::string NameId;
		float Width, Height;
		boundsUV BoundsUV;

		std::vector<BezierCurve*> Curves;
		float Precision;
	};

	//----------------------------------------------------------------------------------------
	// Either an explicit or implicit mesh, DataSpline or DataMesh
	// If DataMesh, all geometry vertices and faces are available
	// If DataSpline, only planar splines available, must be converted to explicit mesh later
	struct DataSurface
	{
		DataSurface( DataSurface& that ); // takes ownership, no reference counting
		DataSurface() :Spline(nullptr), Mesh(nullptr) {} // empty surface
		DataSurface( DataMesh& mesh );
		DataSurface( DataSpline& spline );
		~DataSurface(); // deletes pointer if non-null
		void Free();  

		void CloneFrom( const DataSurface& that ); // deep copy

		DataSpline* GetDataSpline() const { return Spline; }
		DataMesh* GetDataMesh() const { return Mesh; }
		std::string GetName() const;
		bool SetBoundsUV( const boundsUV bounds);
		boundsUV GetBoundsUV() const; // bounds within the layer, measured in UV coordinates
		boundsPixels GetBoundsPixels() const; // bounds within the layer, measured in pixels
		float GetWidth() const; // size of image containing the layer, measured in pixels
		float GetHeight() const; // size of image containing the layer, measured in pixels
		bool IsEmpty() const { return (Spline==nullptr) && (Mesh==nullptr); }
		DataSurface& operator=( DataSurface& that );

	protected:
		DataSpline* Spline;
		DataMesh* Mesh;
	};
}

#endif // DATA_MESH_H