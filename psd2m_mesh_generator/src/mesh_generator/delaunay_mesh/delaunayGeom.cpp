//----------------------------------------------------------------------------------------------
//
//  @file delaunayGeom.cpp
//  @author Michaelson Britt
//  @date 09-02-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

//
// Delaunay triangulation algorithm geometry helpers
//


#include "DelaunayAlgo.h"
#include "delaunayAlgo.h"
#include "util\utils.h"
#include "..\..\..\..\psd2m_maya_plugin\src\json\JSONValue.h" // TODO: Move JSON support to Util project
#include "..\..\..\..\psd2m_maya_plugin\src\json\JSON.h"

#include <vector>
#include <queue>
#include <map> // includes std::multimap

// debugging
//#pragma strict_gs_check(on)

// ----------
// Class GeomMesh implementation
// ----------

// TODO: adjust these metrics for optimal performance
const int GeomMesh::ISECT_TRI_COUNT_COMPACT_THRESH = 100000; // number of triangles added to mesh before a compaction occurs
const int GeomMesh::ISECT_TRI_COUNT_PER_THREAD = 2000; // number of triangles assigned to each thread during isect_tri
const int GeomMesh::ISECT_TRI_THREAD_MAX = 8; // number of threads maximu during isect_tri


GeomMesh::GeomMesh()
: isect_tri_data(nullptr), isect_tri_count_compact_last(0)
{};

int GeomMesh::WriteJSON( const char* filename_utf8 )
{
	int retval = 0;
	JSONObject json_root;

	JSONArray json_verts;
	for( int vi=0; vi < verts.size(); vi++ )
	{
		JSONObject json_vert;
		json_vert[L"i"] = new JSONValue(vi);
		json_vert[L"x"] = new JSONValue(verts[vi].pos.x);
		json_vert[L"y"] = new JSONValue(verts[vi].pos.y);
		json_vert[L"f"] = new JSONValue(verts[vi].flags);
		json_verts.push_back( new JSONValue(json_vert) );
	}
	json_root[L"verts"] = new JSONValue(json_verts);
  
	JSONArray json_edges;
	for( int ei=0; ei < edges.size(); ei++ )
	{
		//if( GetEdge(ei).Initialized() ) ... export even if null edges, call should compact first
		JSONObject json_edge;
		json_edge[L"i"] = new JSONValue(ei);
		json_edge[L"v0"] = new JSONValue(edges[ei].vi[0]);
		json_edge[L"v1"] = new JSONValue(edges[ei].vi[1]);
		json_edge[L"t0"] = new JSONValue(edges[ei].ti[0]);
		json_edge[L"t1"] = new JSONValue(edges[ei].ti[1]);
		json_edge[L"f"] = new JSONValue(edges[ei].flags);
		json_edges.push_back( new JSONValue(json_edge) );
	}
	json_root[L"edges"] = new JSONValue(json_edges);

	JSONArray json_tris;
	for( int ti=0; ti < tris.size(); ti++ )
	{
		//if( GetTri(ti).Initialized() ) ... export even if null tris, call should compact first
		JSONObject json_face;
		json_face[L"i"] = new JSONValue(ti);
		json_face[L"v0"] = new JSONValue(tris[ti].vi[0]);
		json_face[L"v1"] = new JSONValue(tris[ti].vi[1]);
		json_face[L"v2"] = new JSONValue(tris[ti].vi[2]);
		json_face[L"e0"] = new JSONValue(tris[ti].ei[0]);
		json_face[L"e1"] = new JSONValue(tris[ti].ei[1]);
		json_face[L"e2"] = new JSONValue(tris[ti].ei[2]);
		json_face[L"f"] = new JSONValue(tris[ti].flags);
		json_tris.push_back( new JSONValue(json_face) );
	}
	json_root[L"faces"] = new JSONValue(json_tris);
	JSONValue* json = new JSONValue(json_root);

	std::wstring wstr = json->Stringify(true);
	// TODO: HACK, converting from wchar* to char* on assuption of english input
	char* str = new char[1+wstr.length()];
	for( size_t i=0; i<(wstr.length()); i++ ) str[i]=(char)(wstr[i]);
	str[wstr.length()] = '\0';

	FILE* file = nullptr;
	_wfopen_s( &file, util::to_utf16(filename_utf8).c_str(), L"wt" );
	if( file!=nullptr )
	{
		fprintf(file, str );
		fclose(file);
		retval = 1;
	}
	else
	{
		retval = 0;
	}
	delete[] str;
	return retval;
}

int GeomMesh::AddVert( float x, float y )
{
	verts.push_back(GeomVertData(x,y));
	return (int)(verts.size()-1);
}

int GeomMesh::AddEdge( int vi0, int vi1, int ti0, int ti1 )
{
	GeomEdgeData e = GeomEdgeData(vi0,vi1,ti0,ti1);
	// calculate high-level data for edge
	Vector2F p0 = verts[vi0].pos;
	Vector2F p1 = verts[vi1].pos;
	e.tangent = Tangent( Line2F(p0,p1) );
	edges.push_back(e);
	return (int)(edges.size()-1);
}

int GeomMesh::DeleteEdge( int ei )
{
	GeomEdgeData& ed = edges[ei];
	ed.vi[0] = ed.vi[1] = -1; // all indices to undefined value
	ed.ti[0] = ed.ti[1] = -1;
	return (int)(edges.size());
	// TODO: Validity handling, mark mesh invalid, requires a Compact() operation
}

int GeomMesh::AddTri( int vi0, int vi1, int vi2, int ei0, int ei1, int ei2 )
{
	GeomTriData tri(vi0,vi1,vi2,ei0,ei1,ei2);
	tri.circle = Circumcircle( verts[vi0].pos, verts[vi1].pos, verts[vi2].pos );
	tris.push_back(tri);
	int ti = (int)(tris.size()-1);
	return ti;
}

int GeomMesh::DeleteTri( int ti )
{
	GeomTriData& td = tris[ti];
	td.vi[0] = td.vi[1] = td.vi[2] = -1; // all indices to undefined value
	td.ei[0] = td.ei[1] = td.ei[2] = -1;
	return (int)(tris.size());
	// TODO: Validity handling, mark mesh invalid, require a Compact() operation
}

int GeomMesh::Compact()
{
	int null_elements = 0;
	std::vector<int> vert_remap;
	std::vector<int> edge_remap;
	std::vector<int> tri_remap;
	std::vector<GeomVertData> verts_compact;
	std::vector<GeomEdgeData> edges_compact;
	std::vector<GeomTriData> tris_compact;

	//CheckVector( &(this->verts), (int)(this->verts.size()*sizeof(this->verts[0])) );

	// initally mark all verts and edges as culled
	for( int vi=0; vi<this->verts.size(); vi++ ) verts[vi].flags |= GeomVertData::FLAG_CULLED;
	for( int ei=0; ei<this->edges.size(); ei++ ) edges[ei].flags |= GeomEdgeData::FLAG_CULLED;
	// for any edges and verts used by a valid triangle, mark as non-culled
	for( int ti=0; ti<this->tris.size(); ti++ )
	{
		if( GetTri(ti).IsValid() )
		{
			for( int i=0; i<3; i++ )
			{	// remove the culled flag for verts and edges
				this->verts[ this->tris[ti].vi[i] ].flags &= (~GeomVertData::FLAG_CULLED);
				this->edges[ this->tris[ti].ei[i] ].flags &= (~GeomVertData::FLAG_CULLED);
			}
		}
	}
	// remove invalid verts, update mapping
	for( int vi=0; vi<this->verts.size(); vi++ )
	{
		if( GetVert(vi).IsValid() )
		{
			vert_remap.push_back( (int)(verts_compact.size()) );
			verts_compact.push_back( this->verts[vi] );
		}
		else
		{
			vert_remap.push_back( -1 );
			null_elements++;
		}
	}
	// remove invalid edges, update mapping
	for( int ei=0; ei<this->edges.size(); ei++ )
	{
		if( GetEdge(ei).IsValid() )
		{
			edge_remap.push_back( (int)(edges_compact.size()) );
			edges_compact.push_back( this->edges[ei] );
		}
		else
		{
			edge_remap.push_back( -1 );
			null_elements++;
		}
	}
	// remove invalid triangles, update mapping
	for( int ti=0; ti<this->tris.size(); ti++ )
	{
		if( GetTri(ti).IsValid() )
		{
			tri_remap.push_back( (int)(tris_compact.size()) );
			tris_compact.push_back( this->tris[ti] );
		}
		else
		{
			tri_remap.push_back( -1 );
			null_elements++;
		}
	}
	// remap elements
	if( null_elements>0 )
	{
		for( int ei=0; ei<edges_compact.size(); ei++ )
		{
			for( int i=0; i<2; i++ )
			{
				int vi = edges_compact[ei].vi[i];
				if( vi!=-1 ) edges_compact[ei].vi[i] = vert_remap[vi];
				int ti = edges_compact[ei].ti[i];
				if( ti!=-1 ) edges_compact[ei].ti[i] = tri_remap[ti];
			}
		}
		for( int ti=0; ti<tris_compact.size(); ti++ )
		{
			for( int i=0; i<3; i++ )
			{
				int vi = tris_compact[ti].vi[i];
				if( vi!=-1 ) tris_compact[ti].vi[i] = vert_remap[vi];
				int ei = tris_compact[ti].ei[i];
				if( ei!=-1 ) tris_compact[ti].ei[i] = edge_remap[ei];
			}
		}
		//this->verts = verts_compact;
		//this->edges = edges_compact;
		//this->tris = tris_compact;
		// TODO: diagnose why swap() is slower than assignment operator=()
		this->verts.swap( verts_compact );
		this->edges.swap( edges_compact );
		this->tris.swap( tris_compact );
	}
	return null_elements;
}

int GeomMesh::InitQuad( Vector2F upper_left, Vector2F lower_right )
{
	verts.clear();
	AddVert( upper_left.x, upper_left.y );   // vert 0 ... x,y  position
	AddVert( upper_left.x, lower_right.y );  // vert 1 ... x,y  position
	AddVert( lower_right.x, upper_left.y );  // vert 2 ... x,y  position
	AddVert( lower_right.x, lower_right.y ); // vert 3 ... x,y  position
	edges.clear();
	AddEdge( 0,1, 0,-1 ); // edge 0 ... v0,v1 vert indices ... t0,t1 tri indices
	AddEdge( 1,2, 0, 1 ); // edge 1 ... v0,v1 vert indices ... t0,t1 tri indices
	AddEdge( 2,0, 0,-1 ); // edge 2 ... v0,v1 vert indices ... t0,t1 tri indices
	AddEdge( 1,3, 1,-1 ); // edge 3 ... v0,v1 vert indices ... t0,t1 tri indices
	AddEdge( 3,2, 1,-1 ); // edge 4 ... v0,v1 vert indices ... t0,t1 tri indices
	tris.clear();
	AddTri( 0,1,2, 0,1,2 ); // tri 0 ... v0,v1,v3 vert indices ... t0,t1,t2 tri indices
	AddTri( 2,1,3, 1,3,4 ); // tri 0 ... v0,v1,v3 vert indices ... t0,t1,t2 tri indices
	return 1;
}


// ----------
// Delaunay Triangulation
// ----------
//#pragma strict_gs_check(on)
int GeomMesh::AddDelaunayVert( Vector2F p )
{
	// optimization, compact the mesh when too many culled triangles pile up
	if( (tris.size()-isect_tri_count_compact_last) > ISECT_TRI_COUNT_COMPACT_THRESH )
	{
		Compact();
		isect_tri_count_compact_last = (int)(tris.size());
	}

	// STEP 0. Determine which triangle contains the point (unoptimized for now)...
	int ti; // triangle index
	ti = IsectTri(p);

	if( ti<0 )
	{
		DebugPrint("ERROR: AddDelaunayVert(), point not contained by any triangle, [%.4f,%.4f]\n", p.x, p.y );
		return -1;
	}

	// STEP 1. Add the point
	int vi = (int)(verts.size()); // vertex index of the added point
	verts.push_back( GeomVertData(p) );

	// STEP 2. Delete the triangle...
	GeomTriData td_copy = tris[ti];
	DeleteTri(ti); // delete by setting members to undefined value

	// STEP 3. For each edge of the deleted triangle, call a function which takes the point, the edge across from the point, and the old triangle...
	IntPair vp[3] = { {-1,-1}, {-1,-1}, {-1,-1} };
	for( int i=0; i<3; i++ )
	{
		int ei = td_copy.ei[i]; // edge index
		GeomEdge e = GetEdge(ei);
		bool edge_forward = e.IsForwardTri(ti);
		vp[i] = AddDelaunayTri( vi, ei, edge_forward );
	}

	// STEP 4. Now with three pairs of edges returned by three calls to DelaunayUpdate(),
	//         Figure out within the three pairs which are each the same physical edge
	//         Call function UnifyEdge( e_1, e_2 ) on each of those three pairs
	for( int i=0; i<3; i++ )
	{
		for( int j=(i+1); j<3; j++ )
		{
			for( int ii=0; ii<2; ii++ )
			{
				for( int jj=0; jj<2; jj++ )
				{
					// Try to weld every pair of edges, no effect unless the edges have the same verts
					WeldEdges( vp[i][ii], vp[j][jj] ); // TODO: Inefficient
				}
			}
		}
	}

	// DONE!

	return vi;
}

// Helper for AddDelaunayVert();
IntPair GeomMesh::AddDelaunayTri( int vi, int ei, bool edge_forward )
{
	IntPair retval(-1,-1);
	GeomEdge e = GetEdge( ei );
	int vi0 = vi;  // vert from input vi
	int vi1 = edge_forward? e.data().vi[0] : e.data().vi[1]; // vert from input edge ei, in correct winding order for a triangle from vi to ei
	int vi2 = edge_forward? e.data().vi[1] : e.data().vi[0]; // vert from input edge ei, in correct winding order for a triangle from vi to ei
	int ti_across = edge_forward? e.data().ti[1] : e.data().ti[0];

	//   STEP 1. Calculate the circumcircle of triangle t_old...
	bool inside = false;
	if( ti_across>=0 ) // If there is a non-null triangle across the edge...
	{
		// Check whether the circle fitting it (circumcircle) contains the input vertex
		GeomTri t_across = GetTri( ti_across );
		int vi_across = t_across.GetVertAcross(ei); // opposite vert in the far corner of the triangle across the edge
		// Across triangle consists of verts v_across, v1 and v2.  Near triangle consistes of v0, v1, v2
		GeomVert v_across = GetVert( vi_across );
		GeomVert v0 = GetVert( vi0 ); // vert from input vi
		GeomVert v1 = GetVert( vi1 ); // vert from input edge ei
		GeomVert v2 = GetVert( vi2 ); // vert from input edge ei
		GeomCircleD circle_across = Circumcircle( Vector2D(v_across.data().pos), Vector2D(v1.data().pos), Vector2D(v2.data().pos) );
		inside = circle_across.ContainsPoint( Vector2D(v0.data().pos) );

		//   STEP 2. If the point is within the circumcircle,
		//           Note the furthest away vertex, call it v_across
		//           Delete the triangle which is across from the egde, call it t_across
		//           Delete the edge e
		//           Recursively call this function twice, on the point and the two other edges of t_across -> in any order ...
		//           look at the two pairs of returned edges, figure which edge is in common, have the same vertex v and v_across
		//           Call function UnifyEdge( e_1, e_2 ) to combine those two edges into single edge and update the triangles accordingly
		//           Return the edge pait [e_left,e_right] of the other two edges not combined
		if( inside )
		{
			// Find two new edges and two new across triangles for recursive call
			int ei_after  = t_across.GetEdgeAfter(ei);
			int ei_before = t_across.GetEdgeBefore(ei);
			GeomEdge e_after  = GetEdge(ei_after);
			GeomEdge e_before = GetEdge(ei_before);
			bool e_after_forward  = e_after.IsForwardTri(ti_across);
			bool e_before_forward = e_before.IsForwardTri(ti_across);
			// Delete edge and triangle
			DeleteEdge( ei );
			DeleteTri( ti_across );
			// Recursive calls
			IntPair eip_after  = AddDelaunayTri( vi, ei_after, e_after_forward );  // returns pair of edges created
			IntPair eip_before = AddDelaunayTri( vi, ei_before, e_before_forward ); // returns pair of edges created
			// Weld whichever two edges are actually the same edge, among the returned values (double nested for loop looked uglier)
			if( WeldEdges(  eip_before.i[0], eip_after.i[0] ) )
				retval.Set( eip_before.i[1], eip_after.i[1] );
			if( WeldEdges(  eip_before.i[0], eip_after.i[1] ) )
				retval.Set( eip_before.i[1], eip_after.i[0] );
			if( WeldEdges(  eip_before.i[1], eip_after.i[0] ) )
				retval.Set( eip_before.i[0], eip_after.i[1] );
			if( WeldEdges(  eip_before.i[1], eip_after.i[1] ) )
				retval.Set( eip_before.i[0], eip_after.i[0] );
			// TODO: Error handling, assert( (retval.i[0]!=-1) && (retval.i[1]!=-1) )
		}
	}

	//   STEP 3. If the point is not within the circumcircle, or if across triangle is null, no patching neded.
	//           Create a triangle from v to e, call it t_new, and set reference in edge e to t_new
	//           CAUTION: Make sure triangle respects left hand winding order, do not make assumption about travel direction of edge e
	//           Create and add the other two edges, call them t_left and t_right / forward and reverse in counterclockwise order
	//           CAUTION: Make sure these also respect winding order, first face going 0-1 counterclockwise, second face going 1-0 counterclockwise
	//           Return the edge pair [e_left,e_right] in that order -> actually maybe order doesn't matter ...
	if( !inside )
	{
		int ti = (int)(tris.size()); // index of new triangle
		int ei0 = (int)(edges.size()); // index of first new edge
		int ei1 = ei; // index of existing middle edge
		int ei2 = ei0+1; // index of second new edge
		// add two new edges
		AddEdge( vi0, vi1, ti, -1 ); // "forward" tri is the new tri, "reverse" tri is undefined -1
		AddEdge( vi2, vi0, ti, -1 ); // "forward" tri is the new tri, "reverse" tri is undefined -1
		// add tri using the new edges and existing edge
		AddTri( vi0, vi1, vi2,  ei0, ei1, ei2 );
		// patch the existing middle edge according to its forward/reverse direction
		if( edge_forward )
			e.SetTri( 0, ti );
		else e.SetTri( 1, ti );
		retval = IntPair( ei0, ei2 );
		//CheckDelaunayTri(ti,vi0,vi1,vi2); // TODO: Delete this, testing only
	}

	return retval; // return indices of two new edges created
}

int GeomMesh::CheckDelaunayTri( int ti, int vi0, int vi1, int vi2 ) // TODO: Delete this, testing only
{
	int err_count = 0;
	GeomCircleD circle = Circumcircle(
		Vector2D(GetVert(vi0).data().pos), Vector2D(GetVert(vi1).data().pos), Vector2D(GetVert(vi2).data().pos) );
	for( int vi=0; vi<verts.size(); vi++ )
	{
		if( (vi!=vi0) && (vi!=vi1) && (vi!=vi2) &&
			circle.ContainsPoint( Vector2D(GetVert(vi).data().pos) ) )
		{
			verts[vi].flags |= GeomVertData::FLAG_ERR;
			tris[ti].flags  |= GeomTriData::FLAG_ERR;
			err_count++;
		}
	}
	return err_count;
}


// pass two edge indices; each edge should have one invalid triangle and one valid triangle
int GeomMesh::WeldEdges( int ei0, int ei1 )
{
	GeomEdge e0 = GetEdge(ei0);
	GeomEdge e1 = GetEdge(ei1);
	// TODO: Error checking, ensure each edge has exactly one valid triangle
	// assert( ((e0.data().ti[0]==-1&&e0.data().ti[1]!=-1)||(e0.data().ti[0]!=-1&&e0.data().ti[1]==-1)) &&
	//         ((e1.data().ti[0]==-1&&e1.data().ti[1]!=-1)||(e1.data().ti[0]!=-1&&e1.data().ti[1]==-1))
	if( e0.IsValid() && e1.IsValid() && e0.IsSameVerts(e1) )
	{
		int e0ti0a = e0.data().ti[0], e0ti1a = e0.data().ti[1], e1ti0a = e1.data().ti[0], e1ti1a = e1.data().ti[1];// debugging
		// each edge should have one invalid triangle, which gets set to the other edge's valid triangle
		if( e0.data().ti[0]==-1 )
			 e0.SetTri( 0, (e1.data().ti[0]==-1? e1.data().ti[1] : e1.data().ti[0]) );
		else e0.SetTri( 1, (e1.data().ti[0]==-1? e1.data().ti[1] : e1.data().ti[0]) );
		int e0ti0b = e0.data().ti[0], e0ti1b = e0.data().ti[1], e1ti0b = e1.data().ti[0], e1ti1b = e1.data().ti[1];// debugging
		GeomTri t0 = GetTri(e0.data().ti[0]);
		GeomTri t1 = GetTri(e0.data().ti[1]);
		 // keep e0, replace and delete e1
		t0.ReplaceEdge( ei1, ei0 );
		t1.ReplaceEdge( ei1, ei0 );
		DeleteEdge(ei1);
		return 1;
	}
	return 0;
}

int GeomMesh::IsectTri( Vector2F p )
{
	int thread_count_max = thread_pool.thread_count_max;
	if( isect_tri_data==nullptr )
	{
		isect_tri_data = new IsectTriData[thread_count_max];
	}

	ReaderWriterLock lock;
	int ti_out = -1;

	int thread_count = 1, thread_span = (int)(tris.size()); // default to one thread...
	//if( tris.size()>=thread_count_max ) //...only multithread if able to use all threads
	//{
	//	thread_count = thread_count_max;
	//	thread_span = (int)(tris.size() / thread_count_max);
	//}
	if( tris.size() >= (2*ISECT_TRI_COUNT_PER_THREAD) )
	{
		thread_count = MIN( ISECT_TRI_THREAD_MAX, (int)(tris.size()/ISECT_TRI_COUNT_PER_THREAD) );
		thread_span = (int)(tris.size() / thread_count);
	}

	for( int i=0; i<thread_count; i++ )
	{
		IsectTriData* thread_data = &(isect_tri_data[i]);
		thread_data->mesh_in = this;
		thread_data->thread_index = i;
		thread_data->p_in = p;
		thread_data->ti_begin_in = i*thread_span;
		thread_data->ti_end_in = (i+1)*thread_span;
		thread_data->ti_out = &ti_out;
		thread_data->lock = &lock;
		if( i==(thread_count-1) ) thread_data->ti_end_in = (int)(tris.size()); // special case, last thread assigned up any remainder triangles
		if( thread_count>1 )
			thread_pool.Run( IsectTriWorker, thread_data );
		else IsectTriWorker(thread_data); // just run in local thread 
	}
	if( thread_count>1 )
		thread_pool.Wait();

	if( ti_out>=tris.size() ) // should never happen
		ti_out = -1;

	return ti_out;
}

void GeomMesh::IsectTriWorker( void* param )
{
	IsectTriData* data = (IsectTriData*)param;
	ReaderWriterLock& lock = *(data->lock);
	GeomMesh& mesh = *(data->mesh_in);
	Vector2F p = data->p_in;
	int ti_begin = data->ti_begin_in, ti_end = data->ti_end_in;
	int thread_index = data->thread_index;

	int& ti_out_ref = *(data->ti_out);
	int ti_out;
	{	// get point p with read lock
		LockReadGuard lock_read(lock);
		ti_out = ti_out_ref;
	}

	for( int ti = ti_begin; (ti<ti_end) && (ti_out<0); ti++ )
	{
		// check status of other threads...
		if( (ti%256) == thread_index ) // ...but not too often
		{
			LockReadGuard lock_read(lock);
			ti_out = ti_out_ref;
		}
		if( ti_out<0 ) // if haven't found the triangle yet
		{
			GeomTri tri = mesh.GetTri(ti);
			if( tri.IsValid() && tri.ContainsPoint(p) )
			{
				LockWriteGuard lock_read(lock);
				ti_out_ref = ti_out = ti;
			}
		}
	}
}



ThreadPool GeomMesh::thread_pool;
