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
// Delaunay triangulation algorithm geometry helpers
//


#ifndef DELAUNAY_GEOM_H
#define DELAUNAY_GEOM_H

//#include "resource.h"

#include "delaunayMesh.h"
#include "util/helpers.h"
#include "util/math_2D.h"
#include "util/bitmap.h"
#include "util/threadpool.h"
#include "../dataMesh.h"
#include <utility>
#include <vector>
#include <set>
#include <map>

using namespace util;

#define MAX_THREADS 16

struct DelaunayMeshLayerParameters;
struct GeomVertData;
struct GeomEdgeData;
struct GeomTriData;
class GeomVert;
class GeomEdge;
class GeomTri;
class GeomMesh;
GeomVertData& GetVertData( GeomMesh& mesh, int index ); // accessor helper
GeomEdgeData& GetEdgeData( GeomMesh& mesh, int index ); // accessor helper
GeomTriData& GetTriData( GeomMesh& mesh, int index );   // accessor helper



// ----------
// Class Line2F and helpers
// ----------

struct Line2F
{
	Vector2F pos; // start point
	Vector2F end; // end point
	Vector2F dir; // direction vector
	float length;
	inline Line2F( const Vector2F& a, const Vector2F& b ) : pos(a), end(b), dir(0,0) { Set(a,b); }
	inline void Set( const Vector2F& a, const Vector2F& b )
	{
		pos = a;
		end = b;
		dir.x = (b.x-a.x);
		dir.y = (b.y-a.y);
		length = sqrt( (dir.x*dir.x) + (dir.y*dir.y) );
		dir.x = (length<=0.000001? 1.0f : (dir.x/length));
		dir.y = (length<=0.000001? 0.0f : (dir.y/length));
	}
};

struct Line2D
{
	Vector2D pos; // start point
	Vector2D end; // end point
	Vector2D dir; // direction vector
	double length;
	inline Line2D( const Vector2D& a, const Vector2D& b ) : pos(a), end(b), dir(0,0) { Set(a,b); }
	inline void Set( const Vector2D& a, const Vector2D& b )
	{
		pos = a;
		end = b;
		dir.x = (b.x-a.x);
		dir.y = (b.y-a.y);
		length = sqrt( (dir.x*dir.x) + (dir.y*dir.y) );
		dir.x = (length<=0.000001? 1.0f : (dir.x/length));
		dir.y = (length<=0.000001? 0.0f : (dir.y/length));
	}
};

struct IntPair
{
	int i[2];
	inline IntPair( int i0, int i1 )  { Set(i0,i1); }
	inline void Set( int i0, int i1 ) { i[0]=i0; i[1]=i1; } 
	inline int& operator[](int index) { return i[index]; }
};

struct WordPair
{
	typedef unsigned short WORD;
	WORD i[2];
	WordPair( WORD i0, WORD i1 )  { Set(i0,i1); }
	void Set( WORD i0, WORD i1 ) { i[0]=i0; i[1]=i1; } 
	WORD& operator[](int index) { return i[index]; }
};


// Perpendicular direction using right-hand rule, assume up is +z
inline Vector2F Tangent( const Line2F& line )
{
	Vector2F result(0,0);
  
	// cross product of dir vector from->to, which is [ to.x-from.x, to.y-from.y, 0 ],
	//  with the vertical z-direction vector, which is [ 0, 0, 1 ]
	// assume cross product terms are a = b X c ...
	result.x =   (line.dir.y); // a_x = b_y * c_z + b_z * c_y ... but b_z and c_y are zero 
	result.y =  -(line.dir.x); // a_y = b_z * c_x + b_x * c_z ... but b_z and c_x are zero 
	return Normalize(result);
}

// Perpendicular direction using right-hand rule, assume up is +z
inline Vector2D Tangent( const Line2D& line )
{
	Vector2D result(0,0);
  
	// cross product of dir vector from->to, which is [ to.x-from.x, to.y-from.y, 0 ],
	//  with the vertical z-direction vector, which is [ 0, 0, 1 ]
	// assume cross product terms are a = b X c ...
	result.x =   (line.dir.y); // a_x = b_y * c_z + b_z * c_y ... but b_z and c_y are zero 
	result.y =  -(line.dir.x); // a_y = b_z * c_x + b_x * c_z ... but b_z and c_x are zero 
	return Normalize(result);
}
// return the interpolant values u1 and u2 of the intersection points,
// such that the points Lerp( l1p1, l1p2, u1 ) and Lerp( l2p1, l2p2, u2 ) are the same
// returns u1 and u2 as a point, so that u1=result.x, u2=result.y
inline Vector2F LineIntersectInterp( Line2F line1, Line2F line2 )
{
	Vector2F a = line1.pos;
	Vector2F b = line1.end;
	Vector2F c = line2.pos;
	Vector2F d = line2.end;
  
	// To find the point where both lines intersect, define x and y equations for both lines in terms of the lerp value w,
	// then set the x equations equal and the y equations equal, giving two equations in two unknowns (the two lerp values w for lines 1 and 2)
	// and solve for the lerp values, below the lerp values are called u1 and u2
  
	// From Wolfram alpha
	// u1 = (-c.x a.y + d.x a.y + a.x c.y - d.x c.y - a.x d.y + c.x d.y)/(-c.x a.y + d.x a.y + c.x b.y - d.x b.y + a.x c.y - b.x c.y - a.x d.y + b.x d.y)
	// u2 = (-b.x a.y + c.x a.y + a.x b.y - c.x b.y - a.x c.y + b.x c.y)/(c.x a.y - d.x a.y - c.x b.y + d.x b.y - a.x c.y + b.x c.y + a.x d.y - b.x d.y)
  
	float u1_num = (-(c.x * a.y) + (d.x * a.y) + (a.x * c.y) - (d.x * c.y) - (a.x * d.y) + (c.x * d.y));
	float u1_div = (-(c.x * a.y) + (d.x * a.y) + (c.x * b.y) - (d.x * b.y) + (a.x * c.y) - (b.x * c.y) - (a.x * d.y) + (b.x * d.y));

	float u2_num = (-(b.x * a.y) + (c.x * a.y) + (a.x * b.y) - (c.x * b.y) - (a.x * c.y) + (b.x * c.y));
	float u2_div = ((c.x * a.y) - (d.x * a.y) - (c.x * b.y) + (d.x * b.y) - (a.x * c.y) + (b.x * c.y) + (a.x * d.y) - (b.x * d.y));
  
	if( (u1_div!=0) && (u2_div!=0) )
	{
		float u1 = u1_num / u1_div;
		float u2 = u2_num / u2_div;
		return Vector2F( u1, u2 );
	}
	return Vector2F(0,0); // TODO: Error handling
}
// return the interpolant values u1 and u2 of the intersection points,
// such that the points Lerp( l1p1, l1p2, u1 ) and Lerp( l2p1, l2p2, u2 ) are the same
// returns u1 and u2 as a point, so that u1=result.x, u2=result.y
inline Vector2D LineIntersectInterp( Line2D line1, Line2D line2 )
{
	Vector2D a = line1.pos;
	Vector2D b = line1.end;
	Vector2D c = line2.pos;
	Vector2D d = line2.end;
  
	// To find the point where both lines intersect, define x and y equations for both lines in terms of the lerp value w,
	// then set the x equations equal and the y equations equal, giving two equations in two unknowns (the two lerp values w for lines 1 and 2)
	// and solve for the lerp values, below the lerp values are called u1 and u2
  
	// From Wolfram alpha
	// u1 = (-c.x a.y + d.x a.y + a.x c.y - d.x c.y - a.x d.y + c.x d.y)/(-c.x a.y + d.x a.y + c.x b.y - d.x b.y + a.x c.y - b.x c.y - a.x d.y + b.x d.y)
	// u2 = (-b.x a.y + c.x a.y + a.x b.y - c.x b.y - a.x c.y + b.x c.y)/(c.x a.y - d.x a.y - c.x b.y + d.x b.y - a.x c.y + b.x c.y + a.x d.y - b.x d.y)
  
	double u1_num = (-(c.x * a.y) + (d.x * a.y) + (a.x * c.y) - (d.x * c.y) - (a.x * d.y) + (c.x * d.y));
	double u1_div = (-(c.x * a.y) + (d.x * a.y) + (c.x * b.y) - (d.x * b.y) + (a.x * c.y) - (b.x * c.y) - (a.x * d.y) + (b.x * d.y));

	double u2_num = (-(b.x * a.y) + (c.x * a.y) + (a.x * b.y) - (c.x * b.y) - (a.x * c.y) + (b.x * c.y));
	double u2_div = ((c.x * a.y) - (d.x * a.y) - (c.x * b.y) + (d.x * b.y) - (a.x * c.y) + (b.x * c.y) + (a.x * d.y) - (b.x * d.y));
  
	if( (u1_div!=0) && (u2_div!=0) )
	{
		double u1 = u1_num / u1_div;
		double u2 = u2_num / u2_div;
		return Vector2D( u1, u2 );
	}
	return Vector2D(0,0); // TODO: Error handling
}

// ----------
// Class GeomCircle and helpers
// ----------

class GeomCircle
{
public:
	// -----
	// low-level data
	Vector2F center;
	float radius;
	// -----
	inline GeomCircle( Vector2F center, float radius ) : center(center), radius(radius)
	{
		//static const float THRESH = 0.00001f;
		//this->radius = (radius+THRESH);
	}
	inline bool ContainsPoint( Vector2F p ) const
	{
		return (Dist( center, p ) <= radius);
	}
};

inline GeomCircle Circumcircle( const Vector2F& p0, const Vector2F& p1 ) // two points of a line
{
	Vector2F center = Lerp( p0, p1, 0.5f );
	float radius = Dist( p0, center );
	return GeomCircle( center, radius );
}

inline GeomCircle Circumcircle( const Vector2F& p0, const Vector2F& p1, const Vector2F& p2 ) // three points of a triangle
{
	Line2F line1( p0, p1 );
	Line2F line2( p1, p2 );

	if( line1.length<=0.000001 ) return Circumcircle( p1, p2 );
	if( line2.length<=0.000001 ) return Circumcircle( p0, p1 );

	Vector2F line1_mid = Lerp( line1.pos, line1.end, 0.5f );
	Vector2F line2_mid = Lerp( line2.pos, line2.end, 0.5f );
	Vector2F line1_tangent =  Tangent(line1) + line1_mid;
	Vector2F line2_tangent =  Tangent(line2) + line2_mid;
	Line2F isect1( line1_mid, line1_tangent );
	Line2F isect2( line2_mid, line2_tangent );

	Vector2F isect_interp = LineIntersectInterp( isect1, isect2 );
	if( (isect_interp.x==INFINITY) || (isect_interp.y==INFINITY) )
		return GeomCircle( Vector2F(INFINITY,INFINITY), INFINITY ); // TODO: Error handling

	float u1 = isect_interp.x;
	Vector2F center = Lerp( isect1.pos, isect1.end, u1 );
	float radius = Dist( p0, center );
	return GeomCircle( center, radius );
}

class GeomCircleD
{
public:
	// -----
	// low-level data
	Vector2D center;
	double radius;
	// -----
	inline GeomCircleD( Vector2D center, double radius ) : center(center), radius(radius) {}
	inline bool ContainsPoint( Vector2D p ) const
	{
		return (Dist( center, p ) <= radius);
	}
};

inline GeomCircleD Circumcircle( const Vector2D& p0, const Vector2D& p1 ) // two points of a line
{
	Vector2D center = Lerp( p0, p1, 0.5f );
	double radius = Dist( p0, center );
	return GeomCircleD( center, radius );
}

inline GeomCircleD Circumcircle( const Vector2D& p0, const Vector2D& p1, const Vector2D& p2 ) // three points of a triangle
{
	Line2D line1( p0, p1 );
	Line2D line2( p1, p2 );

	if( line1.length<=0.000001 ) return Circumcircle( p1, p2 );
	if( line2.length<=0.000001 ) return Circumcircle( p0, p1 );

	Vector2D line1_mid = Lerp( line1.pos, line1.end, 0.5 );
	Vector2D line2_mid = Lerp( line2.pos, line2.end, 0.5 );
	Vector2D line1_tangent =  Tangent(line1) + line1_mid;
	Vector2D line2_tangent =  Tangent(line2) + line2_mid;
	Line2D isect1( line1_mid, line1_tangent );
	Line2D isect2( line2_mid, line2_tangent );

	Vector2D isect_interp = LineIntersectInterp( isect1, isect2 );
	if( (isect_interp.x==INFINITY) || (isect_interp.y==INFINITY) )
		return GeomCircleD( Vector2D(INFINITY,INFINITY), (double)INFINITY ); // TODO: Error handling

	double u1 = isect_interp.x;
	Vector2D center = Lerp( isect1.pos, isect1.end, u1 );
	double radius = Dist( p0, center );
	return GeomCircleD( center, radius );
}



// ----------
// Class GeomVert and helpers
// ----------

struct GeomVertData
{
	// -----
	// low-level data, stored in file
	Vector2F pos;
	// -----
	int flags;
	enum {FLAG_ERR=1,FLAG_CULLED=2, FLAG_BORDER=4};
	inline GeomVertData( float x, float y ) : pos(x,y), flags(0) {}
	inline GeomVertData( Vector2F p ) : pos(p), flags(0) {}
	inline GeomVertData( GeomVertData const& that) : pos(that.pos), flags(that.flags) {}
	inline ~GeomVertData() { flags=0xDDDDDDDD; } // debugging
};

class GeomVert
{
protected:
	GeomMesh& mesh;
	int index;
public:
	inline GeomVert( GeomMesh& mesh, int index ); // : mesh(mesh), index(index) {}
	//GeomVertData& data() { return GetVertData(mesh,index); }
	inline const GeomVertData& data() const { return GetVertData(mesh,index); }
	inline bool IsCulled() { return (CheckFlag(GeomVertData::FLAG_CULLED)); }
	inline bool IsValid()  { return (!IsCulled()); }
	inline int SetFlag( int flag, bool b )
	{
		GeomVertData& vd = GetVertData(mesh,index); // writable
		int flags_old = vd.flags;
		vd.flags = (b?  (vd.flags|flag) : (vd.flags&(~flag)));
		return flags_old;
	}
	inline bool CheckFlag( int flag )
	{
		return ((data().flags & flag)>0);
	}
};


// ----------
// Class GeomEdge and helpers
// ----------

struct GeomEdgeData
{
	// -----
	// low-level data, stored in file
	int vi[2]; // vertex indices
	int ti[2]; // triangle indices
	// -----
	// high-level data, calculated after file load
	Vector2F tangent;
	int flags;
	enum {FLAG_ERR=1,FLAG_CULLED=2, FLAG_INTERIOR=4};
	// -----
	inline GeomEdgeData( int v0, int v1, int t0, int t1 ) : vi{v0,v1}, ti{t0,t1}, tangent(0,0), flags(0) {}
};

class GeomEdge // high-level helper class
{
protected:
	GeomMesh& mesh;
	int index;
	// do not hold references to verts, always fetch on demand from mesh

public:
	inline GeomEdge( GeomMesh& mesh, int index ) : mesh(mesh), index(index)
	{}
	//GeomEdgeData& data() { return GetEdgeData(mesh,index); }
	inline const GeomEdgeData& data() const { return GetEdgeData(mesh,index); }
	inline bool IsCulled() { return (CheckFlag(GeomEdgeData::FLAG_CULLED)); }
	inline bool IsValid()  { return ((!IsCulled()) && (data().vi[0]!=-1) && (data().vi[1]!=-1)); }

	inline int SetTri( int i, int ti )
	{
		GeomEdgeData& ed = GetEdgeData(mesh,index); // writable
		int& ti_ref = (i==0? ed.ti[0] : ed.ti[1]);
		int ti_old = ti_ref;
		ti_ref = ti;
		return ti_old;
	}
	inline int SetFlag( int flag, bool b )
	{
		GeomEdgeData& ed = GetEdgeData(mesh,index); // writable
		int flags_old = ed.flags;
		ed.flags = (b?  (ed.flags|flag) : (ed.flags&(~flag)));
		return flags_old;
	}
	inline bool CheckFlag( int flag )
	{
		return ((data().flags & flag)>0);
	}

	// 1 if the point is "left" of the line seen looking from point 0 to point 1
	// 0 if the point lies exactly on the line
	// -1 if the point is "right" of the line seen looking from point 0 to point 1
	// looking "left" of an edge is considered as toward the inside of the triangle 
	inline int DirPoint( Vector2F p, int ti )
	{
		Vector2F p0 = GetVertData(mesh,data().vi[0]).pos;
		Vector2F p1 = GetVertData(mesh,data().vi[1]).pos;
		// special case, point is one of our verts; math below is numerically unstable for this
		if( (p.x==p0.x) && (p.y==p0.y) ) return 0;
		if( (p.x==p1.x) && (p.y==p1.y) ) return 0;
    
		// take the offset vector of the point to either end of the line segment,
		// then find the dot prodict with that and the line normal vector,
		// if positive
		int orient=1;
		if( (ti>0) && (data().ti[0]!=ti) ) orient=-1; // reverse direction if this is the reverse face
		Vector2F offset = Vector2F( p.x - p0.x, p.y - p0.y );
		float f = DotProduct( offset, data().tangent );
		if( f > 0 ) return (1*orient);
		if( f < 0 ) return (-1*orient);
		return 0;
	}

	inline bool IsForwardTri( int ti ) // pass triangle index
	{
		// TODO: Error checking, assert((ti==data.ti[0])||(ti==data.ti[1]))
		return (ti==data().ti[0]); // forward direction for t0 edge, reverse direction for t1 edge
	}

	inline bool IsSameVerts( const GeomEdge& that )
	{ return ( ((that.data().vi[0]==this->data().vi[0]) && (that.data().vi[1]==this->data().vi[1])) ||
		       ((that.data().vi[0]==this->data().vi[1]) && (that.data().vi[1]==this->data().vi[0])) ); }
};


// ----------
// Class GeomTri and helpers
// ----------

struct GeomTriData // low-level data class
{
	// -----
	// low-level data, stored in file
	int vi[3]; // vertex indices
	int ei[3]; // edge indices
	// -----
	// high-level data, calculated after file load
	GeomCircle circle;
	int flags;
	enum {FLAG_ERR=1,FLAG_CULLED=2};
	// -----
	inline GeomTriData( int v0, int v1, int v2, int e0, int e1, int e2 )
	: vi{v0,v1,v2}, ei{e0,e1,e2}, circle(Vector2F(0,0),0), flags(0)
	{}
};

class GeomTri
{
protected:
	GeomMesh& mesh;
	int index;
	// do not hold references to verts or edges, always fetch on demand from mesh

public:
	inline GeomTri( GeomMesh& mesh, int index ) : mesh(mesh), index(index)
	{}
	//GeomTriData& data(){ return GetTriData(mesh,index); }
	inline const GeomTriData& data() const { return GetTriData(mesh,index); }
	inline bool IsCulled() { return (CheckFlag(GeomTriData::FLAG_CULLED)); }
	inline bool IsValid() { return (!IsCulled()) && (data().vi[0]!=-1 && data().vi[1]!=-1 && data().vi[2]!=-1 && data().ei[0]!=-1 && data().ei[1]!=-1 && data().ei[2]!=-1); }
	inline int SetFlag( int flag, bool b )
	{
		GeomTriData& td = GetTriData(mesh,index); // writable
		int flags_old = td.flags;
		td.flags = (b?  (td.flags|flag) : (td.flags&(~flag)));
		return flags_old;
	}
	inline bool CheckFlag( int flag )
	{
		return ((data().flags & flag)>0);
	}

	inline int GetVertAcross( int ei ) // pass edge index; return index of third vert, not a member of that edge
	{ return (ei==data().ei[0]? data().vi[2] : (ei==data().ei[1]? data().vi[0] : data().vi[1])); }

	inline int GetEdgeAfter( int ei ) // pass edge index; return index of edge following that edge, in correct winding order
	{ return (data().ei[0]==ei? data().ei[1] : (data().ei[1]==ei)? data().ei[2] : data().ei[0]); }

	inline int GetEdgeBefore( int ei ) // pass edge index; return index of edge preceeding that edge, in correct winding order
	{ return (data().ei[0]==ei? data().ei[2] : (data().ei[1]==ei)? data().ei[0] : data().ei[1]); }

	inline int ReplaceEdge( int ei_find, int ei_replace )
	{
		int retval = 0;
		GeomTriData& td = GetTriData(mesh,index);
		if( td.ei[0]==ei_find ) { td.ei[0] = ei_replace; retval++; }
		if( td.ei[1]==ei_find ) { td.ei[1] = ei_replace; retval++; }
		if( td.ei[2]==ei_find ) { td.ei[2] = ei_replace; retval++; }
		return retval;
	}

	inline bool ContainsPoint( Vector2F p ) const
	{
		//static const float THRESH = -0.00001f;
		if( data().circle.ContainsPoint(p) ) // check this first, for performance reasons
		{
			GeomEdge e0(mesh,data().ei[0]);
			GeomEdge e1(mesh,data().ei[1]);
			GeomEdge e2(mesh,data().ei[2]);

			// if the point is toward the "inside" of all three edges, it must be inside the triangle
			int dir_0 = e0.DirPoint(p,index);
			int dir_1 = e1.DirPoint(p,index);
			int dir_2 = e2.DirPoint(p,index);
			//bool all_positive = (dir_0>=THRESH) && (dir_1>=THRESH) && (dir_2>=THRESH);
			bool all_positive = (dir_0>=0) && (dir_1>=0) && (dir_2>=0);
			return all_positive;
		}
		return false; // default
	}
};


// ----------
// Class GeomMesh and helpers
// ----------

class GeomMesh
{
public: //protected: // TODO: restore this, testing only
	std::vector<GeomVertData> verts;
	std::vector<GeomEdgeData> edges;
	std::vector<GeomTriData> tris;

public:
	GeomMesh();
	int                  WriteJSON( const char* filename_utf8 );
	inline int           GetVertCount() const  { return (int)(verts.size()); }
	inline GeomVertData& GetVertData( int i )  { return verts[i]; };
	inline GeomVert      GetVert( int i )      { return GeomVert(*this,i); };
	inline int           GetEdgeCount() const  { return (int)(edges.size()); }
	inline GeomEdgeData& GetEdgeData( int i )  { return edges[i]; };
	inline GeomEdge      GetEdge( int i ) 	   { return GeomEdge(*this,i); };
	inline int           GetTriCount() const   { return (int)(tris.size()); }
	inline GeomTriData&  GetTriData( int i )   { return tris[i]; };
	inline GeomTri       GetTri( int i )       { return GeomTri(*this,i); };
	int                  AddVert( float x, float y );
	int                  AddEdge( int vi0, int vi1, int ti0, int ti1 );
	int                  DeleteEdge( int ei );
	int                  AddTri( int vi0, int vi1, int vi2, int ei0, int ei1, int ei2 );
	int                  DeleteTri( int ti );
	int                  Compact();
	int                  InitQuad( Vector2F upper_left, Vector2F lower_right );
	int                  AddDelaunayVert( Vector2F p );

protected:
	// Helper, given a vert, plus the edge and face across from the vert, possibly turn edge and recurse as needed
	IntPair				 AddDelaunayTri( int vi, int ei, bool edge_forward ); // return indices of two new edges created
	int                  CheckDelaunayTri( int ti, int vi0, int vi1, int vi2 );
	// Helper, given two edges, combine into one and patch edge and triangle indices
	int					 WeldEdges( int ei0, int ei1 ); // pass edge indices
	//--------------------
	// Triangle intersection testing
	//--------------------
	int IsectTri( Vector2F p ); // index of triangle containing this point
	struct IsectTriData
	{
		GeomMesh* mesh_in; // input mesh
		int thread_index; // TODO: delete this, testing only
		Vector2F p_in; // input point
		int ti_begin_in, ti_end_in; // input tri index range
		int* ti_out; //output  tri index;
		ReaderWriterLock* lock;
		// padding to match cache size can result in 5x performance increase..
		unsigned char padding[80]; // pad to total struct size of 128 bytes on x64 architecture
	};
	IsectTriData* isect_tri_data;
	// optimization params
	int isect_tri_count_compact_last; // number of triangles when compaction last occurred
    // TODO: adjust these metrics for optimal performance
	static const int ISECT_TRI_COUNT_COMPACT_THRESH; // number of triangles added to mesh before a compaction occurs
	static const int ISECT_TRI_COUNT_PER_THREAD; // number of triangles assigned to each thread during isect_tri
	static const int ISECT_TRI_THREAD_MAX; // number of threads maximu during isect_tri
	static void IsectTriWorker( void* param );
	static ThreadPool thread_pool;
};
// ----- accessor helpers
inline GeomVertData& GetVertData( GeomMesh& mesh, int index ) { return mesh.GetVertData(index); }
inline GeomEdgeData& GetEdgeData( GeomMesh& mesh, int index ) { return mesh.GetEdgeData(index); }
inline GeomTriData&  GetTriData(  GeomMesh& mesh, int index ) { return mesh.GetTriData(index);  }
inline GeomVert::GeomVert( GeomMesh& mesh, int index ) : mesh(mesh), index(index) // TODO: remove this, testing only
{
	if( (index < 0) || (mesh.GetVertCount() <= index) )
		DebugPrint("ERROR: attempt to access invalid vert %i\n", index);
}


#endif // DELAUNAY_GEOM_H
