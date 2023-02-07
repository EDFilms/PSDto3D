//----------------------------------------------------------------------------------------------
//
//  @file delaunayMesh.cpp
//  @author Michaelson Britt
//  @date 09-02-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

//
// Delaunay triangulation entry code
//


#ifndef DELAUNAY_MESH_H
#define DELAUNAY_MESH_H

#include <utility>
#include <vector>
#include <set>
#include <map>
#include "util/helpers.h"
#include "util/math_2D.h"
#include "util/bitmap.h"
#include "util/progressJob.h"
#include "../dataMesh.h"

class GeomMesh;

using namespace util;

namespace mesh_generator
{
	struct DelaunayMeshParameters
	{
		// UI params
		float OuterDetail = 50.0f;
		float InnerDetail = 50.0f;
		float FalloffDetail = 40.0f;
		// Conversion from UI params to internal algorithm params
		CurveInterp OuterDetailInterp; 
		CurveInterp InnerDetailInterp;
		CurveInterp FalloffDetailInterp; 

		bool operator==(const DelaunayMeshParameters& that) const {
			return (this->OuterDetail==that.OuterDetail) && (this->InnerDetail==that.InnerDetail) && (this->FalloffDetail==that.FalloffDetail); }
		bool operator!=(const DelaunayMeshParameters& that) const { return !(this->operator==(that)); }
	};

	struct DelaunayMeshInput
	{
		typedef std::vector<unsigned char*> Pixels; // ordered ARGB
		typedef std::vector<BezierCurve*> Curves;
		DelaunayMeshInput( const Pixels& pixels, const Curves& curves )
		: pixels(pixels), curves(curves), width(0), height(0) {}
		const Pixels pixels; // region of image for the layer
		const Curves curves;
		boundsPixels boundsPixels; // pixels bounds of mesh within entire image
		boundsUV boundsCurves; // curves bounds of mesh in UV space
		int width; // width of entire source image
		int height; // height of entire source image
		std::string name;
	};

	//----------------------------------------------------------------------------------------------
	class DelaunayMesh
	{
	public:
		static DataMesh GenerateMesh( const DelaunayMeshParameters& params, const DelaunayMeshInput& input,
			ProgressTask& progressTask );
	};
}

#endif // DELAUNAY_MESH_H
