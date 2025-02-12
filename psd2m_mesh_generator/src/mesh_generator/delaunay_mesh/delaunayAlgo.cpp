//----------------------------------------------------------------------------------------------
//
//  @file delaunayAlgo.cpp
//  @author Michaelson Britt
//  @date 09-02-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

//
// Delaunay triangulation algorithm core code
//


#include "delaunayAlgo.h"
#include "util/utils.h"
#include "util/helpers.h"
#include "..\..\..\..\psd2m_core\src\json\JSONValue.h" // TODO: Move JSON support to Util project
#include "..\..\..\..\psd2m_core\src\json\JSON.h"

#include <vector>
#include <queue>
#include <map> // includes std::multimap


// debugging
//#pragma strict_gs_check(on)


//--------------------------------------------------------------------------------------------------------------------------------------
// Test functions
extern const char* DebugGetSaveFolder();
const char* DebugGetSavePath( const char* layername, const char* ext )
{
	static char path[1024];
	if( strlen(DebugGetSaveFolder())==0 )
		return nullptr;
	strcpy_s( path, 1024, DebugGetSaveFolder() );
	strcat_s( path, 1024, "/" );
	strcat_s( path, 1024, layername );
	strcat_s( path, 1024, ext );
	return path;
}
const wchar_t* DebugGetSavePath( const wchar_t* layername, const wchar_t* ext )
{
	static wchar_t path[1024];
	if( strlen(DebugGetSaveFolder())==0 )
		return nullptr;
	const char* folder = DebugGetSaveFolder();
	for( int i=0; (i<1024) && ((i==0) || (folder[i-1]!='\0')); i++ ) path[i]=folder[i];
	wcscat_s( path, 1024, L"/" );
	wcscat_s( path, 1024, layername );
	wcscat_s( path, 1024, ext );
	return path;
}


namespace mesh_generator
{

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Helper function
	// expands min and max values according to input curve
	void SetCurveBounds( float& x_min, float& y_min, float& x_max, float& y_max, BezierCurve* curve )
	{
		if( curve!=nullptr )
		{
			const std::vector<Vector2F*>& points = curve->GetCurves();
			for( int i=0; i<points.size(); i++ )
			{
				Vector2F* point = points[i];
				if( point!=nullptr )
				{
					x_min = MIN( x_min, point->x ), y_min = MIN( y_min, point->y );
					x_max = MAX( x_max, point->x ), y_max = MAX( y_max, point->y );
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Helper function
	// Given two adjacent spline edges prev->cur and cur->next, and a mesh edge from cur->vert
	// return whether the mesh edge is interior or exterior to the arc formed by the spline edges
	// assumes the interior is always "to the right" as the spline travels
	bool IsEdgeInterior( Vector2F& p_prev, Vector2F& p_cur, Vector2F& p_next, Vector2F& p_check )
	{
		float a_prev = (float)atan2( p_prev.x-p_cur.x, p_prev.y-p_cur.y );
		float a_next = (float)atan2( p_next.x-p_cur.x, p_next.y-p_cur.y );
		float a_check = (float)atan2( p_check.x-p_cur.x, p_check.y-p_cur.y );

		a_next = a_next - a_prev;
		a_check = a_check - a_prev;
		if( a_next<0 ) a_next = a_next + (float)TWO_PI;
		if( a_check<0 ) a_check = a_check + (float)TWO_PI;
		return( (a_check>0) && (a_check<a_next) );
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------
	// Class DelaunayAlgo
	//--------------------------------------------------------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------------------------------------------------------


	// Bitmap pixel flags, for Delaunay rasterized height map / topographic map calculations
	#define TOPO_FLAG_EXTERIOR 1
	#define TOPO_FLAG_INTERIOR 2
	#define TOPO_FLAG_BORDER 4
	#define TOPO_FLAG_VERT 8
	#define TOPO_FLAG_NOVERT 16
	// TODO: delete this, testing only
	#define TOPO_FLAG_KEEPER 32

	const int DelaunayAlgo::TOPO_BITMAP_MAXIMUM = 4096;
	const int DelaunayAlgo::TOPO_BITMAP_PADDING = 2;


	//--------------------------------------------------------------------------------------------------------------------------------------
	//
	// Type definitions
	//

	struct DrawInteriorEdgeParam // parameter to DrawInteriorEdge()
	{
		DelaunayAlgo* parent;
		int curve_index_1, segment_index_1;
		int curve_index_2, segment_index_2;
		int found_x, found_y;
		int found;
	};

	struct CheckInteriorEdgeParam // parameter to CheckInteriorEdge()
	{
		DelaunayAlgo* parent;
		int is_interior;
	};


	//--------------------------------------------------------------------------------------------------------------------------------------
	//
	// Class methods
	//


	//--------------------------------------------------------------------------------------------------------------------------------------
	DelaunayAlgo::DelaunayAlgo( const DelaunayMeshParameters& params, const DelaunayMeshInput& input )
	    : params(params), input(input), points_total(0), points_finished(0)
	{
		int largest_dim = MAX(input.width, input.height);
		this->topo_bitmap_maximum = MIN( TOPO_BITMAP_MAXIMUM, 2*largest_dim);

		// Convert UI params to internal algorithm params
		float border_step_w = (params.OuterDetail-1.0f)/99.0f; // PerimeterDetail range is [1,100]
		this->border_step = params.OuterDetailInterp.Interp( border_step_w ); // measured in UV units
		float height_step_w = (params.InnerDetail-1.0f)/99.0f; // RadialDetail range is [1,100]
		this->height_step = (int)(params.InnerDetailInterp.Interp( height_step_w ) * topo_bitmap_maximum);  // measured in pixel units
		float height_falloff_w = (params.FalloffDetail)/100.0f; // RadialDetail range is [0,100]
		this->height_falloff = params.FalloffDetailInterp.Interp( height_falloff_w );

		// don't allow steps less than 2 pixels
		// (but shouldn't happen if DelaunayInnerDetailHi / DelaunayOuterDetailHi are defined properly)
		int min_step_pix = 2;
		float min_step_uv = min_step_pix / (float)topo_bitmap_maximum;
		this->height_step = MAX( this->height_step, min_step_pix );
		this->border_step = MAX( this->border_step, min_step_uv );
		

		//
		// Topograhic space calculation
		// See comments in header for explanation of topographic domain size and bitmap size
		//

		// Calculate topo space range (abstract units) ...
		if( input.width > input.height )
			 this->topo_space_range = Vector2F( 1.0f, (input.height / (float)input.width) );
		else this->topo_space_range = Vector2F( (input.width / (float)input.height), 1.0f );


		// Calculate topo space bounds of layer (abstract units) ...
		for( int j=0; j<input.curves.size(); j++ )
		{
			boundsUV bounds_curve;
			bounds_curve.GenerateBoundingBox( input.curves[j]->GetCurves() );
			this->bounds.Expand( bounds_curve );
		}


		// calculate topo bitmap range (maximum size of a topo bitmap, pixel units) ...
		int topo_bitmap_maximum_x = (int)(topo_bitmap_maximum * this->topo_space_range.x);
		int topo_bitmap_maximum_y = (int)(topo_bitmap_maximum * this->topo_space_range.y);
		//this->topo_bitmap_range = Pos2N(
		//	topo_bitmap_maximum_x + 1,
		//	topo_bitmap_maximum_y + 1 );


		// calculate topo bitmap cropped position within the range (pixel units) ...
		int topo_bitmap_lo_x = (int)(topo_bitmap_maximum_x * this->bounds.MinPoint().x);
		int topo_bitmap_lo_y = (int)(topo_bitmap_maximum_y * this->bounds.MinPoint().y);
		// the topographic data map is "pixel floating", so the pixels of this map
		// may not align to pixels in the input texture (may have some sub-pixel displacement between them)
		this->topo_bitmap_pos = Pos2N( topo_bitmap_lo_x, topo_bitmap_lo_y );


		// calculate topo bitmap cropped size within the range (pixel units) ...
		// use abstract bounds, based on the boundary curves of the layer,
		// instead of input.boundsPixels, which does not reflect real layer contents,
		// because input.boundsPixels reflects only the layer's allocated storage size, not its contents.
		int topo_bitmap_hi_x = (int)(topo_bitmap_maximum_x * this->bounds.MaxPoint().x);
		int topo_bitmap_hi_y = (int)(topo_bitmap_maximum_y * this->bounds.MaxPoint().y);
		int topo_bitmap_width_unpadded = (topo_bitmap_hi_x-topo_bitmap_lo_x)+1;
		int topo_bitmap_height_unpadded = (topo_bitmap_hi_y-topo_bitmap_lo_y)+1;
		int topo_bitmap_width  = topo_bitmap_width_unpadded  + (2*TOPO_BITMAP_PADDING);
		int topo_bitmap_height = topo_bitmap_height_unpadded + (2*TOPO_BITMAP_PADDING);

		// create the topographic calculation bitmap
		bitmapTopo.Init( topo_bitmap_width, topo_bitmap_height, TopoPix(0,0,0,0) );


		// the normals map is "pixel locked", so the pixels of this map
		// exactly align to pixels in the input texture (no sub-pixel misalignment between maps)
		int img_bitmap_x_lo = (int)(input.width * bounds.MinPoint().x), img_bitmap_x_hi = (int)(input.width * bounds.MaxPoint().x);
		int img_bitmap_width = (int)(img_bitmap_x_hi - img_bitmap_x_lo) + 1;
		int img_bitmap_y_lo = (int)(input.height * bounds.MinPoint().y), img_bitmap_y_hi = (int)(input.height * bounds.MaxPoint().y);
		int img_bitmap_height = (int)(img_bitmap_y_hi - img_bitmap_y_lo) + 1;
		bitmapNorm.Init( img_bitmap_width, img_bitmap_height, NormalPix(0,0,0) );

		mesh.InitQuad( Vector2F(-0.1f,-0.1f), Vector2F(1.1f,1.1f) );
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	//
	// Topographic space <-> UV space transformations
	//
	//   Source image 1024x512
	//   ------------------------------------------------------------
	//   |                                                         D|
	//   |                                                          |
	//   |            Cropped layer 512x256                         |
	//   |            ------------------------------                |
	//   |            |                           C|                |
	//   |            |                            |                |
	//   |            |                            |                |
	//   |            |                            |                |
	//   |            |B                           |                |
	//   |            ------------------------------                |
	//   |                                                          |
	//   |A                                                         |
	//   ------------------------------------------------------------
	//
    //      Source UV   Source Pixel  Crop UV   Crop Pixel   Topo Space   Topo Pixel
	//   A  [0,0]       [0,0] 
	//   B  [0.25,0.25] [256,128]     [0,0]     [0,0]        [0,0]        [0,0]
	//   C  [0.75,0.75] [768,384]     [1,1]     [512,256]   [1,0.5]       [2048,1024]
	//   D  [1,1]       [1024,512]
	//
	//   Rectangular source images like [1024,512] have a square UV space [0,0]->[1,1]
	//   Cropped layer textures like [512,256] also use a square UV space [0,0]->[1,1]
	//   Topo calculations occur in a RECTANGULAR SPACE, 1.0 on the long side;
	//     if the image is wide [0,0]->[1,height/width]
	//     if the image is tall [0,0]->[width/height,1]
	//   Topo calculations also use a bitmap space,  4096 pixels on the long side,
	//   less on the short side.  Calculations for individual layers use a bitmap
	//   cropped from that space. In the example above, topo bitmap space is size [4096,2048],
	//   and bitmap for calculation on the cropped layer is size [2048,1024]
	//
	//--------------------------------------------------------------------------------------------------------------------------------------


	//--------------------------------------------------------------------------------------------------------------------------------------
	DelaunayAlgo::Pos2N DelaunayAlgo::TopoSpaceToTopoPixel( Vector2F v )
	{
		// position in full topo space (pixel units)
		int topo_range_pos_x = (int)(v.x * topo_bitmap_maximum);
		int topo_range_pos_y = (int)(v.y * topo_bitmap_maximum);
		// position in bitmap
		return Pos2N(
			(topo_range_pos_x - this->topo_bitmap_pos.x) + TOPO_BITMAP_PADDING,
			(topo_range_pos_y - this->topo_bitmap_pos.y) + TOPO_BITMAP_PADDING
		);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	Vector2F DelaunayAlgo::TopoPixelToTopoSpace( Pos2N p )
	{
		// position in full topo space (pixel units)
		int topo_bitmap_pos_x = (p.x - TOPO_BITMAP_PADDING) + this->topo_bitmap_pos.x;
		int topo_bitmap_pos_y = (p.y - TOPO_BITMAP_PADDING) + this->topo_bitmap_pos.y;
		// position in full topo space (abstract units)
		return Vector2F(
			(topo_bitmap_pos_x + 0.5f) / (float)topo_bitmap_maximum,
			(topo_bitmap_pos_y + 0.5f) / (float)topo_bitmap_maximum 
		);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	Vector2F DelaunayAlgo::SourceUVtoTopoSpace( Vector2F v )
	{
		// Example: input image is 2048x1024, with rectangular aspect ratio, then
		// source uv range is [0, 0] -> [1, 1] and topo space range is [0, 0] -> [1, 0.5]
		return Vector2F( v.x * this->topo_space_range.x, v.y * this->topo_space_range.y );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	Vector2F DelaunayAlgo::TopoSpaceToSourceUV( Vector2F v )
	{
		// Example: input image is 2048x1024, with rectangular aspect ratio, then
		// source uv range is [0, 0] -> [1, 1] and topo space range is [0, 0] -> [1, 0.5]
		return Vector2F( v.x / this->topo_space_range.x, v.y / this->topo_space_range.y );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void DelaunayAlgo::InitBorderPoints()
	{
		// ensure one list of border verts for each list of curve points
		if( border_list.size()<=input.curves.size() )
			border_list.resize( input.curves.size() );

		// Stretch to ensure uv distances are proportional to pixel distances.
		// UV range is always [0,0]-[1,1] from lowerleft to upper right of image, regardless of aspect ratio.
		// Therefore need to squeeze uvs along smaller dimension, so they're proprtional to larger dimension.
		float squeeze_x = 1.0f, squeeze_y = 1.0f;
		if( input.width > input.height )
			 squeeze_y = (input.height / (float)input.width);  // wide image, reduce distances in height
		else squeeze_x = (input.width  / (float)input.height); // tall image, reduce distances in width

		Vector2F p_last(-1,-1);
		for( int curve_index=0; curve_index<input.curves.size(); curve_index++ )
		{
			BezierCurve* curve = input.curves[curve_index];
			if( curve!=nullptr )
			{
				for( int knot_index=0; knot_index<curve->GetCurveSize(); knot_index++ )
				{
					Vector2F p = curve->GetPoint(knot_index);
					p.x = p.x * squeeze_x;  // squeeze to to compensate for aspect ratio
					p.y = p.y * squeeze_y;  // squeeze to to compensate for aspect ratio
					// points in the original curve are usually too densly packed...
					// so, only allow next point when beyond threshold distance from the last used point
					if( Dist( p, p_last ) > border_step )
					{
						BorderVert b = { knot_index, 0 }; // set the vert index later
						border_list[curve_index].push_back(b);
						p_last = p;
						points_total++;
					}
				}

				// only include borders large enough to contain at least three points
				if( border_list[curve_index].size()<3 )
				{
					border_list[curve_index].clear();
				}

			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void DelaunayAlgo::AddBorderPoints( int curve_index, ProgressTask& progressTask )
	{
		bool cancelled = false;
		int points_since_update = 0;

		BezierCurve* curve = input.curves[curve_index];
		if( curve!=nullptr )
		{
			// grab a list of precalculated border points
			std::vector<BorderVert>& border_verts = border_list[curve_index];
			int points_added = 0;

			for( int segment_index=0; segment_index<border_verts.size() && !cancelled; segment_index++ )
			{
				BorderVert b = border_verts[segment_index];
				Vector2F v = curve->GetPoint(b.knot_index);
				v = SourceUVtoTopoSpace( v );
				b.vert_index = mesh.AddDelaunayVert( v );
				if( b.vert_index>=0 )
				{
					mesh.GetVert( b.vert_index ).SetFlag( GeomVertData::FLAG_BORDER,true );
					border_verts[segment_index] = b;
				}
				points_finished++;
				points_since_update++;
				if( points_since_update>1000 )
				{
					progressTask.SetValueAndUpdate( points_finished / (float)points_total );
					cancelled = progressTask.IsCancelled();
					points_since_update=0;
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void DelaunayAlgo::AddHeightPoints(ProgressTask& progressTask)
	{
		bool cancelled = false;
		int points_since_update = 0;

		for( int y=0; y<bitmapTopo.height && !cancelled; y++ )
		{
			for( int x=0; x<bitmapTopo.width && !cancelled; x++ )
			{
				TopoPix val = bitmapTopo.Get(x,y);
				bool isVert = ((val.flags&TOPO_FLAG_VERT) > 0);
				bool isBorder = ((val.flags&TOPO_FLAG_BORDER) > 0);
				if( isVert && !isBorder )
				{
					Vector2F v = TopoPixelToTopoSpace( Pos2N(x, y) );
					mesh.AddDelaunayVert( v );
					points_finished++;
					points_since_update++;
					if( points_since_update>1000 )
					{
						progressTask.SetValueAndUpdate( points_finished / (float)points_total );
						cancelled = progressTask.IsCancelled();
						points_since_update=0;
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void DelaunayAlgo::RemoveInteriorTris()
	{
		for( int ei=0; ei<mesh.GetEdgeCount(); ei++ )
		{
			GeomEdge e = mesh.GetEdge(ei);
			if( e.IsValid() )
			{
				GeomVert vert0 = mesh.GetVert( e.data().vi[0] );
				GeomVert vert1 = mesh.GetVert( e.data().vi[1] );
				Vector2F v0 = vert0.data().pos;
				Vector2F v1 = vert1.data().pos;
				// if it's one of the four corner verts, any triangles using this edge are definitely exterior
				if( (v0.x<-0.05) || (v0.y<-0.05) || (v1.x<-0.05) || (v1.y<-0.05) ||
					(v0.x>1.05)  || (v0.y>1.05)  || (v1.x>1.05)  || (v1.y>1.05) )
				{
					e.SetFlag( GeomEdgeData::FLAG_CULLED, true );
				}
				else
				{
					Pos2N p0 = TopoSpaceToTopoPixel( v0 );
					Pos2N p1 = TopoSpaceToTopoPixel( v1 );
					CheckInteriorEdgeParam param = {this,false};

					bitmapTopo.CheckLine( CheckInteriorEdge, &param, p0.x, p0.y, p1.x, p1.y );
					// If the edge is definitely interior and not border, any triangles using it are interior
					if( param.is_interior )
					{
						e.SetFlag( GeomEdgeData::FLAG_INTERIOR, true );
					}
				}
			}
		}

		for( int ti=(mesh.GetTriCount()-1); ti>=0; ti-- )
		{
			GeomTri t = mesh.GetTri(ti);
			if( t.IsValid() )
			{
				int ei0 = t.data().ei[0];
				int ei1 = t.data().ei[1];
				int ei2 = t.data().ei[2];
				GeomEdge e0 = mesh.GetEdge( ei0 );
				GeomEdge e1 = mesh.GetEdge( ei1 );
				GeomEdge e2 = mesh.GetEdge( ei2 );
				bool is_interior_0 = ((e0.data().flags & GeomEdgeData::FLAG_INTERIOR)>0);
				bool is_interior_1 = ((e1.data().flags & GeomEdgeData::FLAG_INTERIOR)>0);
				bool is_interior_2 = ((e2.data().flags & GeomEdgeData::FLAG_INTERIOR)>0);
				bool any_interior = (is_interior_0 || is_interior_1 || is_interior_2);
				bool is_culled_0 = ((e0.data().flags & GeomEdgeData::FLAG_CULLED)>0);
				bool is_culled_1 = ((e1.data().flags & GeomEdgeData::FLAG_CULLED)>0);
				bool is_culled_2 = ((e2.data().flags & GeomEdgeData::FLAG_CULLED)>0);
				bool any_culled = (is_culled_0 || is_culled_1 || is_culled_2);
				// if none of the three edges are interior, or any are culled, then delete the triangle
				if( (!any_interior) || (any_culled) )
				{
					mesh.DeleteTri(ti);
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void DelaunayAlgo::GetDataMesh( DataMesh& mesh_out )
	{
		std::vector<Vector2F> vertices;
		std::vector<int> facesCount;
		std::vector<int> facesMarks;
		std::vector<int> facesIndices;
		vertices.reserve( this->mesh.GetVertCount() );
		facesCount.reserve( this->mesh.GetTriCount() );
		facesMarks.reserve( this->mesh.GetTriCount() );
		facesIndices.reserve( this->mesh.GetTriCount()*3 );

		for( int vi=0; vi<this->mesh.GetVertCount(); vi++ )
		{
			Vector2F v = TopoSpaceToSourceUV( this->mesh.GetVert(vi).data().pos );
			vertices.push_back( v );
		}
		for( int ti=0; ti<this->mesh.GetTriCount(); ti++ )
		{
			facesCount.push_back( 3 );
			facesMarks.push_back( 3*ti );
			facesIndices.push_back( this->mesh.GetTri(ti).data().vi[0] );
			facesIndices.push_back( this->mesh.GetTri(ti).data().vi[1] );
			facesIndices.push_back( this->mesh.GetTri(ti).data().vi[2] );
		}

		mesh_out.SetName( input.name );
		mesh_out.SetWidth( (float)input.width );
		mesh_out.SetHeight( (float)input.height );
		mesh_out.SetBoundsUV( bounds );
		mesh_out.SetValues( vertices, facesCount, facesMarks, facesIndices );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	GeomMesh& DelaunayAlgo::GetGeomMesh()
	{
		return this->mesh;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	DelaunayAlgo::ColorPix DelaunayAlgo::GetColorCropPix( int x, int y )
	{
		// No error checking; helper for GetColorNormal() which should perform error checking
		const DelaunayMeshInput::Pixels& p = input.pixels;
		int index = ((y * input.boundsPixels.WidthPixels())  + x);
		if( p.size()>3 ) // have ARGB channel data
			return ColorPix( p[1][index], p[2][index], p[3][index], p[0][index] );
		// else have RGB channel data
		return ColorPix( p[0][index], p[1][index], p[2][index], 0 );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	DelaunayAlgo::NormalPix DelaunayAlgo::GetColorCropNormal( int x, int y )
	{
		if( (x<0) || (y<0) || (x>=input.boundsPixels.WidthPixels()) || (y>=input.boundsPixels.HeightPixels()) )
			return NormalPix( 127,127,255 ); // default normal map color
		//ImgPixelToCropPixel(x,y,x,y);
		// vector offsets for eight adjacent pixels around current pixels
		static const float vx[] = { -0.7071f,  0.0f,  0.7071f,     -1.0f,      1.0f,  -0.7071f,      0.0f,    0.7071f };
		static const float vy[] = { -0.7071f, -1.0f, -0.7071f,      0.0f,      0.0f,   0.7071f,      1.0f,    0.7071f };
		// index offsets for eight adjacent pixels around current pixels
		static const int   dx[] = { -1,        0,        1,        -1,         1,        -1,         0,          1 };
		static const int   dy[] = { -1,       -1,       -1,         0,         0,         1,         1,          1 };
		
		float sx=0.0f, sy=0.0f, sz=0.0f; // vector offset sums
		int c=0;
		int alphaCenter = GetColorCropPix(x,y).a; // center pixel alpha
		for( int i=0; i<8; i++ )
		{
			int xi=x+dx[i], yi=y+dy[i];
			if( (xi<0) || (yi<0) || (xi>=input.boundsPixels.WidthPixels()) || (yi>=input.boundsPixels.HeightPixels()) )
				continue;
		    int alphaCurrent = GetColorCropPix( xi, yi ).a; // current pixel alpha
			int delta = alphaCurrent - alphaCenter;
			float weight = delta / 255.0f; // delta magnitude, varies from -1 to +1
			sx += vx[i] * weight, sy += vy[i] * weight; // add weighted vector to sum
			c++;
		}
		sx /= c, sy /= c;
		// solve for z:  sqrt(x^2 + y^2 + z^2)=1 ... square both sides, x^2 + y^2 + z^2 = 1 ... z = sqrt( 1 - (x^2 + y^2))
		sz = (float)sqrt( 1.0f - ((sx*sx) + (sy*sy)) );

		int ix = (int)(127.5f + (127.5f*sx)), iy = (int)(127.5f + (127.5f*sy)), iz = (int)(127.5f + (127.5f*sz));
		return NormalPix( (BYTE)ix, (BYTE)iy, (BYTE)iz );
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void DelaunayAlgo::DrawNormals()
	{
		for( int y=0; y<bitmapNorm.height; y++ )
		{
			for( int x=0; x<bitmapNorm.width; x++ )
			{
				NormalPix normalPix = GetColorCropNormal( x, y );
				bitmapNorm.Set( x,y, normalPix );
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void DelaunayAlgo::DrawHeight()
	{
		// draw the exterior spline as boundaries for the hill values (height map)
		for( int j=0; j<input.curves.size(); j++ )
		{
			DrawHeightBorder( j ); 
		}
		// calculate the hill values (height map) in four passes
		DrawHeightPass( -1,  0 );
		DrawHeightPass(  1,  0 );
		DrawHeightPass(  0, -1 );
		DrawHeightPass(  0,  1 );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void DelaunayAlgo::DrawHeightBorder( int curve_index )
	{
		BezierCurve* curve = input.curves[curve_index];
		TopoPix val = {TOPO_FLAG_BORDER,0,0,0};
		bool fill_corners = true; // so perimeter is extra watertight, important for exterior culling later
		if( curve!=nullptr )
		{
			const std::vector<Vector2F*>& points = curve->GetCurves();
			std::vector<BorderVert>& border_verts = border_list[curve_index];
			int vert_count = (int)(border_verts.size());
			if( vert_count>0 )
			{
				int knot_index_prev = border_verts.back().knot_index;
				Vector2F point_prev = curve->GetPoint(knot_index_prev);
				for( int segment_index=0; segment_index<vert_count; segment_index++ )
				{
					int knot_index_cur = border_verts[segment_index].knot_index;
					Vector2F point_cur = curve->GetPoint(knot_index_cur);

					val.curve_index = (WORD)curve_index;
					val.segment_index = (WORD)segment_index;
						
					// convert vert location in source UV space to pixel location in topo map
					Pos2N p1 = TopoSpaceToTopoPixel( SourceUVtoTopoSpace( point_prev ) );
					Pos2N p2 = TopoSpaceToTopoPixel( SourceUVtoTopoSpace( point_cur ) );
					// mark all pixels between these two as topo border pixels
					bitmapTopo.DrawLine( val, p1.x, p1.y, p2.x, p2.y, fill_corners );

					// also mark the vertex point
					TopoPix val_vert = val;
					val_vert.flags |= TOPO_FLAG_VERT;
					bitmapTopo.Set( p1.x, p1.y, val_vert );
					bitmapTopo.Set( p2.x, p2.y, val_vert );

					point_prev = point_cur;
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// search direction should be +1 or -1 in one dimension, and 0 in other dimension
	void DelaunayAlgo::DrawHeightPass( int x_search_dir, int y_search_dir )
	{  
		int x_begin=0, x_end=bitmapTopo.width, x_dir = 1;
		if( x_search_dir>0 )
		{
			x_begin = bitmapTopo.width-1;
			x_end = -1;
			x_dir = -1;
		}
		int y_begin=0, y_end=bitmapTopo.height, y_dir = 1;
		if( y_search_dir>0 )
		{
			y_begin = bitmapTopo.height-1;
			y_end = -1;
			y_dir = -1;
		}
  
		for( int y=y_begin; y!=y_end; y+=y_dir )
		{
			for( int x=x_begin; x!=x_end; x+=x_dir )
			{
				TopoPix val_prev = TopoPix(TOPO_FLAG_EXTERIOR,0,0,0);
				if( bitmapTopo.IsValid(x+x_search_dir, y+y_search_dir) )
					val_prev = bitmapTopo.Get( x+x_search_dir, y+y_search_dir );
				WORD height_prev = val_prev.height;
				TopoPix val_cur = bitmapTopo.Get( x, y );
				WORD height_cur = val_cur.height;
				bool edge_cur = ((val_cur.flags & TOPO_FLAG_BORDER)>0);
				bool exterior_prev = ((val_prev.flags & TOPO_FLAG_EXTERIOR)>0);
				bool exterior_cur = ((val_cur.flags & TOPO_FLAG_EXTERIOR)>0);
				int height_rolling = height_prev+1;
				if( (x==x_begin) || (y==y_begin) )
				{
					val_cur.flags |= TOPO_FLAG_EXTERIOR;
					val_cur.height = 0;
					bitmapTopo.Set( x,y, val_cur );
				}
				else if( !edge_cur )
				{
					TopoPix val_rolling = val_cur; // assume previously known value is already correct by default
					if( exterior_prev || exterior_cur )
					{	// previous or current were exterior, and current is not an edge, therefore it is extior
						val_rolling.flags |= TOPO_FLAG_EXTERIOR;
						val_rolling.height = 0;
					}
					else if(
						(height_cur>height_rolling) || // current is closer to an edge than previously known
						(height_cur==0) ) // current is uninitialized
					{	
						val_rolling.height = height_rolling;
					}
					bitmapTopo.Set( x,y, val_rolling );
				}
			}
		}  
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void DelaunayAlgo::DrawPoints()
	{
		// "up" controls how far up the hill each topographic rung sits
		// "side" control haw far apart points are spaced on each rung
		// side is smaller than up; encourages delaunay triangulation to place edges
		// walking along the topographic rungs instead of perpendicular to them
		float radius_up_float = (float)(this->height_step);
		float radius_side_float = (radius_up_float*0.75f)+1.0f;
		float height_inc = this->height_falloff-1.0f;

		int x_begin = 0, x_end = bitmapTopo.width, x_dir = 1;
		int y_begin = 0, y_end = bitmapTopo.height, y_dir = 1;
		int height_target = 0;
		int points_added = -1; // stop when unable to find anywhere to add more points (after innermost rung)
		for( int rung_count=1; points_added!=0; rung_count++ )
		{
			int radius_up = (int)radius_up_float;
			int radius_side = (int)radius_side_float;

			height_target += radius_up;
			points_added = 0;
			for( int y=y_begin; y!=y_end; y+=y_dir )
			{
				for( int x=x_begin; x!=x_end; x+=x_dir )
				{
					TopoPix val_cur = bitmapTopo.Get( x, y );
					int height_cur = val_cur.height;
					bool allow_cur = ((val_cur.flags&TOPO_FLAG_NOVERT)==0);
					if( allow_cur && (height_cur>=height_target) && (height_cur<=(height_target+2)) )
					{
						// found a pixel at our target elevation (distance value) ...
						for( int xr=(x-radius_side); xr<(x+radius_side); xr++ )
						{
							for( int yr=(y-radius_side); yr<(y+radius_side); yr++ )
							{
								// ...erase all data values within the given radius,
								// to prevent other nearby points from being marked ...
								if( bitmapTopo.IsValid(xr,yr) )
								{
									TopoPix val_novert = bitmapTopo.Get( xr, yr );
									val_novert.flags |= TOPO_FLAG_NOVERT;
									bitmapTopo.Set( xr, yr, val_novert );
								}
							}
						}
						// ... then mark the original point
						TopoPix val_vert = val_cur;
						val_vert.flags |= TOPO_FLAG_VERT;
						bitmapTopo.Set( x, y, val_vert );
						points_added++;
						points_total++;
					}
				}
			}
			radius_up_float = (radius_up_float * this->height_falloff) + height_inc;
			radius_side_float = (radius_side_float * this->height_falloff) + height_inc;
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void DelaunayAlgo::DrawInterior()
	{
		// 2. After delaunay, create lookup of vertices->edges
		// TODO: Could performance be improved?  Currently uses std::multimap of pairs
		// Alternative;  do tests to see the how many edges per point,
		// then use vector of data structure that has default 6 (or whatever) indices but optionally pointer to more
		for( int ei=0; ei<mesh.GetEdgeCount(); ei++ )
		{
			GeomEdge e = mesh.GetEdge(ei);
			if( e.IsValid() )
			{
				int vi0 = e.data().vi[0], vi1 = e.data().vi[1];
				vert_to_edge_lookup.insert( std::pair<int,int>( vi0, ei ) );
				vert_to_edge_lookup.insert( std::pair<int,int>( vi1, ei ) );
			}
		}

		// 3. Iterate through vertices along the "original" edges, from first step
		//    For each vertex, iterate through edges for that point
		//    Do fancy math to determine whether each of those edges lies within the "inside arc"
		//    described by the two original spline edges for the point
		for( int j=0; j<border_list.size(); j++ )
		{
			BezierCurve* curve = input.curves[j];
			int segment_count = (int)(border_list[j].size());
			if( (curve!=nullptr) && (segment_count>=3) ) // need a closed area, must be at least 3 points
			{
				DrawInteriorBorder( j );
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// visits all verts in the given curve which were used in the delaunay triangulation
	// then marks all edges connecting to each of those as interior if applicable
	// then performs a flood fill (paint bucket) of the topographic map for each interior edge,
	// finding the first applicable interior pixel on the edge;
	// flood fill marks other pixels in the map as interior pixels also
	void DelaunayAlgo::DrawInteriorBorder( int curve_index )
	{
		BezierCurve* curve = input.curves[curve_index];
		if( curve!=nullptr )
		{
			std::vector<BorderVert>& border_verts = border_list[curve_index];
			int segment_count = (int)(border_verts.size());
			// for each vert along this border (segment) mark interior edges connecting to the vert
			for( int segment_index=0; segment_index<segment_count; segment_index++)
			{
				// find the next and previous verts along the border,
				// which describe two current border edges around the current vert
				int segment_index_prev = ((segment_index-1)+segment_count) % segment_count;
				int segment_index_next = ((segment_index+1)+segment_count) % segment_count;
				int segment_index_cur  = segment_index;
				int vert_index_prev = border_verts[ segment_index_prev ].vert_index;
				int vert_index_next = border_verts[ segment_index_next ].vert_index;
				int vert_index_cur  = border_verts[ segment_index_cur  ].vert_index;
				Vector2F point_prev = mesh.GetVert(vert_index_prev).data().pos;
				Vector2F point_next = mesh.GetVert(vert_index_next).data().pos;
				Vector2F point_cur  = mesh.GetVert(vert_index_cur ).data().pos;
				std::multimap<int,int>::iterator it = it=vert_to_edge_lookup.find(vert_index_cur);
				int iter_count = (int)vert_to_edge_lookup.count(vert_index_cur); // number of edges connected to this vert
				for( int iter_index=0; (iter_index<iter_count) && (it!=vert_to_edge_lookup.end()); iter_index++, it++ )
				{
					int edge_index = it->second;
					GeomEdge e = mesh.GetEdge(edge_index);
					// get the other vert on this edge
					int vert_index_check = (e.data().vi[0]==vert_index_cur?  e.data().vi[1] : e.data().vi[0]);
					// if the vert is an adjacent border vert, then it describes a border edge, skip additional checks
					bool isCorner = ((vert_index_cur<4) || (vert_index_check<4));
					bool isBorder ((vert_index_check==vert_index_prev) || (vert_index_check==vert_index_next) || (vert_index_check==vert_index_cur));
					if( (!isCorner) && (!isBorder) )
					{
						// if the vert describes an interior edge, flag that edge
						Vector2F point_check  = mesh.GetVert(vert_index_check).data().pos;
						bool is_interior = IsEdgeInterior( point_prev, point_cur, point_next, point_check );
						if( is_interior )
						{
							e.SetFlag( GeomEdgeData::FLAG_INTERIOR, true );


							// 4. For each "inside" edge, draw it again using similar line drawing algorithm (draw_mode_2),
							//    but this time, skip the first pixel, then flag each subsequent pixel as an "inside pixel",
							//    until reaching another edge pixel or finishing the line

							// (not using this anymore but...)
							// to ensure drawing two pixels per point, assuming horizontal line,
							// take current x value and substract 0.5, then cast to integer for x_lo,
							// cast current x value to integer for x_cur
							// if x_lo and x_cur are the same, the increment x_cur

							DrawInteriorEdgeParam param =
								{this, curve_index,segment_index_prev, curve_index,segment_index_cur, 0,0, false};

							Pos2N p1 = TopoSpaceToTopoPixel( point_cur );
							Pos2N p2 = TopoSpaceToTopoPixel( point_check );
							bitmapTopo.CheckLine( DrawInteriorEdge, &param, p1.x, p1.y, p2.x, p2.y );
							if( param.found )
							{
								// 5. For each "inside pixel" perform a flood fill, marking other pixels as inside,
								//    with the edge pixels as the flood borders
								DrawInteriorFill( param.found_x, param.found_y );
							}
						}
						// (see code inside FillInteriorEdgeFn)
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// flood fill of nearby interior pixels, assumeing [x,y] is an interior pixel
	// stops at any border pixel or other interior pixels
	void DelaunayAlgo::DrawInteriorFill( int x, int y )
	{
		// Flood fill operates by maintaining a queue of pixels to be visited,
		// each visit marks the pixel as inside, then checks if any of the surrounding four
		// are still unmarked, and if so, adds those to the queue, the removes current from queue

		Pos2N sides[] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

		std::queue<Pos2N> queue;
		queue.push( {x,y} ); // instantiating structs this way... was this valid in C++98?

		while( (!queue.empty()) )
		{
			Pos2N pos = queue.front();
			queue.pop();
			TopoPix val = bitmapTopo.Get( pos.x, pos.y );
			bool isInterior = ((val.flags & TOPO_FLAG_INTERIOR)>0);
			bool isExterior = ((val.flags & TOPO_FLAG_EXTERIOR)>0);
			if( (!isInterior) && (!isExterior) )
			{
				bool isAdjacent = false;
				for( int i=0; i<4; i++ )
				{
					Pos2N side = {pos.x+sides[i].x, pos.y+sides[i].y};
					if( bitmapTopo.IsValid( side.x, side.y ) )
					{
						TopoPix val = bitmapTopo.Get(side.x,side.y);
						bool isInterior = ((val.flags & TOPO_FLAG_INTERIOR)>0);
						bool isBorder = ((val.flags & TOPO_FLAG_BORDER)>0);
						bool isBorderOnly = isBorder && !isInterior;

						if( (!isInterior) && (!isBorderOnly) )
							queue.push( side ); // next pixel is neither border nor already marked as interior, visit it

						if( isBorderOnly )
							isAdjacent = true; // current pixel is adjacent to border
					}
				}
				if( isAdjacent )
					 val.flags |= (TOPO_FLAG_INTERIOR | TOPO_FLAG_BORDER); // border adjacent; both border and interior
				else val.flags |= TOPO_FLAG_INTERIOR; // interior only
				bitmapTopo.Set( pos.x,pos.y, val );
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Helper for DrawInterior()
	// used with Bitmap::CheckLine() to draw known interior pixels from known interior edges
	int DelaunayAlgo::DrawInteriorEdge( void* param_in, int x, int y )
	{
		DrawInteriorEdgeParam* param = (DrawInteriorEdgeParam*)param_in;
		DelaunayAlgo* parent = param->parent;
		bool stopped = false;

		DelaunayAlgo::TopoPix val = parent->bitmapTopo.Get(x,y);
		bool isEdge = ((val.flags & TOPO_FLAG_BORDER)>0);
		bool isSpline1 = (val.curve_index==param->curve_index_1) && (val.segment_index==param->segment_index_1);
		bool isSpline2 = (val.curve_index==param->curve_index_2) && (val.segment_index==param->segment_index_2);
		bool isOrigin = isEdge && (isSpline1 || isSpline2);
		if( !isOrigin )
		{
			// stop after finding the first pixel past the origin region
			// if it's not on the perimeter edge, then it must be interior
			stopped = true;
			if( !isEdge )
			{
				// don't flag the edge here, will he handled by called //val.flags |= TOPO_FLAG_INTERIOR;
				param->found_x = x, param->found_y = y;
				param->found = true;
			}
		}

		return (stopped? -1 : 0); // Bitmap::CheckLine() stops if less than zero
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Helper for RemoveInteriorTris()
	// used with Bitmap::CheckLine() to search for interior pixels on along mesh edges
	int DelaunayAlgo::CheckInteriorEdge( void* param_in, int x, int y )
	{
		CheckInteriorEdgeParam* param = (CheckInteriorEdgeParam*)param_in;
		DelaunayAlgo* parent = param->parent;
		bool stopped = false;

		TopoPix val = parent->bitmapTopo.Get(x,y);
		bool is_interior = ((val.flags & TOPO_FLAG_INTERIOR)>0);
		bool is_border = ((val.flags & TOPO_FLAG_BORDER)>0);
		if( is_interior && !is_border )
		{
			stopped = true;
			param->is_interior = true;
			// TODO: delete this, testing only
			val.flags = val.flags | TOPO_FLAG_KEEPER;
			parent->bitmapTopo.Set(x,y,val);
		}

		return (stopped? -1 : 0); // Bitmap::CheckLine() stops if less than zero 
	}
}


//--------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------
// Debugging methods
//--------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------


// Libpng config, required before header include
#define PNGLCONF_H
#define PNG_LINKAGE_API
#define PNG_STDIO_SUPPORTED
#define PNG_INFO_IMAGE_SUPPORTED
#define PNG_WRITE_SUPPORTED
#include "..\..\..\..\psd2m_core\include\png.h" // TODO: Delete this, testing only
// Libpng helpers
extern const char* libpng_user_error_ptr;
void libpng_user_error_fn(png_structp png_ptr,png_const_charp error_msg);
void libpng_user_warning_fn(png_structp png_ptr,png_const_charp warning_msg);

typedef struct { unsigned char r,g,b; } Color;

//--------------------------------------------------------------------------------------------------------------------------------------
// Helper
Color Lerp( Color ca, Color cb, float w )
{
	unsigned char r = (unsigned char)((1.0f-w)*ca.r + (w*cb.r));
	unsigned char g = (unsigned char)((1.0f-w)*ca.g + (w*cb.g));
	unsigned char b = (unsigned char)((1.0f-w)*ca.b + (w*cb.b));
	return {r,g,b};
}
//--------------------------------------------------------------------------------------------------------------------------------------
// Helper
// given two or more color points spaced evenly across range w=0 to w=1,
// finds the nearest pair to a given w and interpolates between them
Color Gradient( Color* colors, int count, float w )
{
	int section_count = count-1;
	int section_index = (int)(w * section_count);
	section_index = MIN( section_index, section_count-1 );
	Color color_a = colors[section_index];
	Color color_b = colors[section_index+1];
	float span = 1.0f / section_count;
	float w_lo = section_index*span;
	float w_interp = (w-w_lo) / span;
	w_interp = MIN( w_interp, 1.0f );
	return Lerp( color_a, color_b, w_interp );
}

//--------------------------------------------------------------------------------------------------------------------------------------
void mesh_generator::DelaunayAlgo::DebugSaveMesh()
{
	const char* filename = DebugGetSavePath(input.name.data(),".json");
	if( filename==nullptr )
		return;

	mesh.WriteJSON( filename );
}

//--------------------------------------------------------------------------------------------------------------------------------------
// which: 0 for topo map, 1 for normals map

void mesh_generator::DelaunayAlgo::DebugSaveBitmap( int which )
{
//  // WINDOWS ONLY
//	std::string filename_utf8 = DebugGetSavePath(input.name.data(),"_normals.png");
//	if( filename_utf8.length()==0 )
//		return;
//
//	Color gradient[] = { {0,0,255}, {0,255,0}, {255,255,0}, {255,0,0} };
//
//	// which==0?  "test_delaunay_data.png" : "test_delaunay_normals.png"
//	png_structp png_ptr;
//	png_infop info_ptr;
//
//	// Open the file
//	FILE *fp = NULL;
//	errno_t err = _wfopen_s(&fp, util::to_utf16(filename_utf8).c_str(), L"wb");
//	if ((err != 0) || (fp == NULL))
//		return; // ERROR
//
//	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
//		(png_voidp)libpng_user_error_ptr, libpng_user_error_fn, libpng_user_warning_fn);
//	if (png_ptr == NULL)
//	{
//		fclose(fp);
//		return; // ERROR
//	}
//
//	// Allocate/initialize the image information data.
//	info_ptr = png_create_info_struct(png_ptr);
//	if (info_ptr == NULL)
//	{
//		fclose(fp);
//		png_destroy_write_struct(&png_ptr,  NULL);
//		return; // ERROR
//	}
//
//	// Configure the image information data.
//	int bitmap_width  = (which==0?  bitmapTopo.width  : bitmapNorm.width);
//	int bitmap_height = (which==0?  bitmapTopo.height : bitmapNorm.height);
//	const int BYTES_PER_PIXEL = 4;
//	const int BITS_PER_COMPONENT = 8; // bit depth
//	const int COLOR_TYPE = PNG_COLOR_TYPE_RGB_ALPHA;
//	const int INTERLACE_TYPE = PNG_INTERLACE_NONE;
//	const int COMPRESSION_TYPE = PNG_COMPRESSION_TYPE_DEFAULT;
//	const int FILTER_METHOD = PNG_FILTER_TYPE_DEFAULT;
//	if( which==0 ) {
//		png_set_IHDR(png_ptr, info_ptr,
//			bitmap_width, bitmap_height, BITS_PER_COMPONENT,
//			COLOR_TYPE, INTERLACE_TYPE, COMPRESSION_TYPE, FILTER_METHOD );
//	}
//	else {
//		png_set_IHDR(png_ptr, info_ptr,
//			bitmap_width, bitmap_height, BITS_PER_COMPONENT,
//			COLOR_TYPE, INTERLACE_TYPE, COMPRESSION_TYPE, FILTER_METHOD );
//	}
//
//	png_init_io(png_ptr, fp);
//
//	// Allocate and configure row pointers
//	Bitmap<int> colors;
//	int val = 0;
//	colors.Init( bitmap_width, bitmap_height, val );
//	png_byte** rows = new png_byte*[bitmap_height];
//	png_byte* row = (png_byte*)(colors.data.data());
//	for( int y=0; y<bitmap_height; y++ )
//	{
//		rows[y] = row;
//		row += (BYTES_PER_PIXEL*bitmap_width);
//		for( int x=0; x<bitmap_width; x++ )
//		{
//			if( which==0 ) {
//				TopoPix w = bitmapTopo.Get(x,y);
//				int color = 0x7F0000FF;
//				if( w.flags & TOPO_FLAG_KEEPER ) color = 0xFFFF3FFF; // light purple
//				else if( w.flags & TOPO_FLAG_VERT ) color = 0xFFFFFFFF; // white
//				else if( w.flags & TOPO_FLAG_BORDER ) color = 0xFF000000;
//				else
//				{
//					bool stipple = (((x+y)%2)==1);
//					Color rgb = Gradient( gradient, 4, w.height/512.0f );
//					if( stipple && (w.flags & TOPO_FLAG_INTERIOR) )
//					{
//						//color = 0xFF007F7F;
//						rgb.r = (rgb.r + 64) / 2;
//						rgb.g = (rgb.g + 64) / 2;
//						rgb.b = (rgb.b + 64) / 2;
//					}
//					color = 0xFF000000 + (((int)rgb.b)<<16) + (((int)rgb.g)<<8) + (((int)rgb.r)<<0);
//				}
//				colors.Set( x,y, color);
//			}
//			else {
//				NormalPix w = bitmapNorm.Get(x,y);
//				int color = 0xFF000000 + (((int)w.z)<<16) + (((int)w.y)<<8) + (((int)w.x)<<0);
//				colors.Set( x,y, color);
//			}
//		}
//	}
//
//	// Assign row pointers
//	png_set_rows(png_ptr, info_ptr, rows);
//
//	int png_transforms = PNG_TRANSFORM_IDENTITY;
//	png_write_png(png_ptr, info_ptr, png_transforms, NULL, NULL, NULL);
//	png_destroy_write_struct(&png_ptr, &info_ptr);
//
//	// Close the file.
//	fclose(fp);
//
//	delete[] rows;
}

