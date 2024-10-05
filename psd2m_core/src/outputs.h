//----------------------------------------------------------------------------------------------
//
//  @file outputs.h
//  @author Michaelson Britt
//  @date 08-03-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

//
// Layer and Atlas evaluation results
//

#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "compLayers.h"
#include "mesh_generator/dataMesh.h"
#include "util/bounds_2D.h"

#include <string>
#include <vector>
#include <map>

namespace psd_to_3d
{
	using mesh_generator::DataSurface;

	//----------------------------------------------------------------------------------------------
	struct GraphLayer
	{
		GraphLayer() : Mesh(), LayerIndex(-1), AtlasIndex(-1),
			Depth(0), Scale(0) {}
		GraphLayer( GraphLayer& that ) : // WARNING: Takes ownership of mesh, no reference counting
			Mesh(that.Mesh), MaterialName(that.MaterialName),
			TextureName(that.TextureName), TextureFilepath(that.TextureFilepath),
			CompLayerFilepaths(that.CompLayerFilepaths),
			LayerName(that.LayerName), LayerIndex(that.LayerIndex), AtlasIndex(that.AtlasIndex),
			LayerRegion(that.LayerRegion),
			XFormUV(that.XFormUV), Depth(that.Depth), Scale(that.Scale) {}
		xformUV GetXFormUV() const { return XFormUV; }

		DataSurface Mesh;

		std::string MaterialName;
		std::string TextureName;
		std::string TextureFilepath; // full path and filename
		// component layer textures, one for each comp layer type, listed in enum order, empty string if none
		std::vector<std::string> CompLayerFilepaths; // full path and filename of each

		std::string LayerName;
		int LayerIndex;			// original PSD file layer index
		int AtlasIndex;

		boundsUV LayerRegion;	// Region within PSD layer, includes padding pixels
		xformUV XFormUV;		// Vertex position to UV coordinate, either for texture or atlas as appropriate

		float Depth;
		float Scale;
	};


	//----------------------------------------------------------------------------------------------
	// TODO: Document this, is this conceptually a folder in the photoshop layer stack?
	// Not supported currently to export the those folders as a hierarchy if nested
	class GraphLayerGroup
	{
	public:
		virtual ~GraphLayerGroup();

		// Group name and list of graph layers
		std::string GroupName;

		virtual GraphLayer* AllocGraphLayer(int layerIndex);
		virtual int GetLayerCount() const {return (int)(graphLayerList.size());}
		virtual const GraphLayer* operator[](int layerIndex) const {return graphLayerList[layerIndex];}
		static GraphLayer null;

	protected:
		std::vector<GraphLayer*> graphLayerList;
	};


	//----------------------------------------------------------------------------------------------
	// TODO: Measure performance from copying GraphLayer and GraphLayerGroup during data std::map operations
	typedef std::pair<const int, GraphLayer> GraphLayerByIndexItem;
	typedef std::map<int, GraphLayer> GraphLayerByIndexMap;
	typedef std::map<std::string, GraphLayerGroup> GroupByNameMap;

}

#endif // OUTPUTS_H
