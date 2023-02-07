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


#include "delaunayMesh.h"
#include "delaunayAlgo.h"

#include <tchar.h>

namespace mesh_generator
{

	//--------------------------------------------------------------------------------------------------------------------------------------
	DataMesh DelaunayMesh::GenerateMesh(const DelaunayMeshParameters& params, const DelaunayMeshInput& input,
		ProgressTask& progressTask)
	{
		// 1. While adding spline points to delaunay, record list of points used for each spline

		// 6. For each edge in the delaunay mesh, draw it again using a similar line drawing algorithm (draw_mode_3),
		//    but this time, go all the way to both ends, looking for any pixel marked as an "inside pixel",
		//    and if any pixel of any edge is "inside" then the entire triangle is inside,
		//    otherwise the triangle is outside

		DelaunayAlgo* delaunayAlgo = new DelaunayAlgo( params, input );
		DataMesh mesh;

		if( !progressTask.IsCancelled() ) // first block, drawing points
		{
			// initialize knot_indices, must be done first
			delaunayAlgo->InitBorderPoints();

			// CREATE DATA MAP AND COMPUTE SIDEWALK POINTS
			delaunayAlgo->DrawHeight();
			delaunayAlgo->DrawNormals();
			delaunayAlgo->DrawPoints();
			//progressTask.SetValueAndUpdate( 0.10f );
		}

		if( !progressTask.IsCancelled() ) // second block, interior vertices
		{
			// CREATE MESH, vertices along mesh perimeter
			delaunayAlgo->AddHeightPoints(progressTask);
		}

		if( !progressTask.IsCancelled() ) // third block, border vertices
		{
			// CREATE MESH, vertices within mesh interior
			for( int i=0; (!progressTask.IsCancelled()) && (i<input.curves.size()); i++ )
				delaunayAlgo->AddBorderPoints(i,progressTask);
		}

		if( !progressTask.IsCancelled() ) // fourth block, removing triangles
		{
			// REMOVE EXTERIOR TRIANGLES
			// First draw all pixels which are within the interior of a mesh, based on analysis of edge
			delaunayAlgo->DrawInterior();
			// Next remove triangles unless they have one vert in interior space,
			// or one edge touching the border and pointing toward the interior from the border
			delaunayAlgo->RemoveInteriorTris();

			// compact the mesh, remove the dead triangles (those subdivided into smaller tris when adding delaunay points)
			delaunayAlgo->GetGeomMesh().Compact();

			// DEBUGGING
#ifdef PSDTO3D_DEBUG_DELAUNAY_NORMALS
			delaunayAlgo->DebugSaveBitmap(1); // testing only
#endif
#ifdef PSDTO3D_DEBUG_DELAUNAY_TOPO
			delaunayAlgo->DebugSaveBitmap(0); // testing only
#endif
#ifdef PSDTO3D_DEBUG_DELAUNAY_MESH
			delaunayAlgo->DebugSaveMesh(); // testing only
#endif

			delaunayAlgo->GetDataMesh( mesh );
		}

		delete delaunayAlgo;

		return mesh;
	}

}