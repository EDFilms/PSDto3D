//----------------------------------------------------------------------------------------------
//
//  @file delaunayAlgo.h
//  @author Michaelson Britt
//  @date 09-02-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

//
// Delaunay triangulation algorithm core code
//


#ifndef DELAUNAY_ALGO_H
#define DELAUNAY_ALGO_H


//#include "resource.h"

#include "delaunayMesh.h"
#include "delaunayGeom.h"
#include "util/helpers.h"
#include "util/math_2D.h"
#include "util/bitmap.h"
#include "../dataMesh.h"
#include <utility>
#include <vector>
#include <set>
#include <map>

using namespace util;
typedef unsigned char BYTE;

namespace mesh_generator
{
	//----------------------------------------------------------------------------------------------
	class DelaunayAlgo
	{
	public:
		DelaunayAlgo( const DelaunayMeshParameters& params, const DelaunayMeshInput& input );
		virtual ~DelaunayAlgo() {}

		typedef unsigned short WORD;
		struct Pos2N
		{
			int x;
			int y;
			Pos2N() : x(0), y(0) {}
			Pos2N( int x, int y ) : x(x), y(y) {}
			inline static double Dist( const Pos2N& a, const Pos2N& b )
			{
				double x = b.x-a.x, y = b.y-a.y;
				return sqrt( (x*x) + (y*y) );
			}
		};

		// pixel in the data bitmap, for topographic (sidewalk strip) point selection in delaunay triangulation
		struct TopoPix
		{
			WORD flags;
			WORD height;
			WORD curve_index; // for pixels on a perimeter edge, the index of the spline
			WORD segment_index;   // for pixels on a perimeter edge, the index of the knot within the spline
			TopoPix( WORD flags, WORD height, WORD curve_index, WORD segment_index )
			: flags(flags), height(height), curve_index(curve_index), segment_index(segment_index) {}
		};
		// data bitmap, for topographic (sidewalk strip) point selection in delaunay triangulation
		typedef Bitmap<TopoPix> TopoMap;

		// pixel in the normal vector bitmap
		struct NormalPix
		{
			BYTE x,y,z; // x tangent, y bitangent, z normal
			NormalPix( BYTE x, BYTE y, BYTE z ) : x(x), y(y), z(z) {}
		};
		// data bitmap, for topographic (sidewalk strip) point selection in delaunay triangulation
		typedef Bitmap<NormalPix> NormalMap;

		// pixel in the input image bitmap
		struct ColorPix
		{
			BYTE r,g,b,a;
			ColorPix( BYTE r, BYTE g, BYTE b, BYTE a ) : r(r), g(g), b(b), a(a) {}
		};

		// point on the perimeter of the mesh, one list of these needed per spline
		struct BorderVert
		{
			int knot_index;
			int vert_index;
		};
		// one line of points per spline
		typedef std::vector< std::vector<BorderVert> > BorderList;

		// conversion Topographic pixel <-> Topographic space
		Pos2N TopoSpaceToTopoPixel( Vector2F pos );
		Vector2F TopoPixelToTopoSpace( Pos2N pos );
		// conversion Source UV <-> Topographic space
		Vector2F SourceUVtoTopoSpace( Vector2F pos );
		Vector2F TopoSpaceToSourceUV( Vector2F pos );

		// methods related to the delaunay mesh
		void InitBorderPoints();
		void AddBorderPoints( int curve_index, ProgressTask& progressTask );
		void AddHeightPoints( ProgressTask& progressTask );
		void RemoveInteriorTris();
		void GetDataMesh( DataMesh& mesh_out ); // converts verts from Topographic space -> Source UV space
		GeomMesh& GetGeomMesh();

		// methods related to the image bitmap
		ColorPix GetColorCropPix( int x, int y );
		NormalPix GetColorCropNormal( int x, int y ); // calculate normal for a pixel in the input color image

		// methods related to the topograhic data bitmap
		void DrawNormals();
		void DrawHeight();
		void DrawHeightBorder( int curve_index );			// helper for DrawHeight()
		void DrawHeightPass( int x_dir, int y_dir );	    // helper for DrawHeight()
		void DrawPoints();
		void DrawInterior();
		void DrawInteriorBorder( int curve_index );			// helper for DrawInterior()
		void DrawInteriorFill( int x, int y );				// helper for DrawInterior()

		static int DrawInteriorEdge( void* param, int x, int y );
		static int CheckInteriorEdge( void* param, int x, int y );

		// Debugging
		void DebugSaveMesh();
		void DebugSaveBitmap( int which );

	protected:

		// input params passed to GenerateMesh()
		const DelaunayMeshParameters& params;
		const DelaunayMeshInput& input;

		// user params calculated from DelaunayMeshParameters
		float border_step; // in spline uv space
		int height_step; // in bitmap pixel space
		float height_falloff;

		// algorithm data
		GeomMesh mesh;
		TopoMap bitmapTopo; // topographic data map
		NormalMap bitmapNorm; // normals map
		BorderList border_list;
		boundsUV bounds; // uv bounds, calculated
		std::multimap<int,int> vert_to_edge_lookup;
		int points_total=0, points_finished=0; // total number of delaunay points needed, and current number actually finished
		// The bitmaps used for topographic calculations are different sizes for each layer, according to layer size,
		// up to a maximum width and height, which is the topographic bitmap "domain".
		// This domain has the same aspect ratio as the original image, and the longer side equals TOPO_BITMAP_DOMAIN_SIZE.
		// For example,
		// - If the original image is 1024x1024, the topographic bitmap domain is square,
		//   with [topo_bitmap_domain_width = TOPO_BITMAP_DOMAIN_SIZE, topo_bitmap_domain_height = TOPO_BITMAP_DOMAIN_SIZE]
		// - If the original image is 2048x512, the topographic bitmap domain is retangular,
		//   with [topo_bitmap_domain_width = TOPO_BITMAP_DOMAIN_SIZE, topo_bitmap_domain_height = TOPO_BITMAP_DOMAIN_SIZE / 4.0]
		// - In both cases, a layer which covers the upper-left quarter of the image uses a topographic calculation bitmap
		//   with [width = topo_bitmap_domain_width / 2.0, height = topo_bitmap_domain_height / 2.0]
		// This ensures the delaunay algorithm produces the same mesh density (total number of triangles)
		// for any given parameter values, regardless of the resolution of the input image
		Vector2F topo_space_range; // size of the topo space in abstract units, long side is 1.0, short side less per aspect ratio
		//Pos2N topo_bitmap_range; // size of the topo space in pixel units, long size is TOPO_BITMAP_MAXIMUM, short size less
		Pos2N topo_bitmap_pos; // pixel position of the topo bitmap within the topo space
		int topo_bitmap_maximum;
		static const int TOPO_BITMAP_MAXIMUM; // maximum topo bitmap size assuming a square image (width and height)
		static const int TOPO_BITMAP_PADDING; // number of blank pixels around outside of image

	};
}

#endif // DELAUNAY_ALGO_H
