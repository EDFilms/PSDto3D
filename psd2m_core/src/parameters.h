//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file parameters.h
//  @author Benjamin Drouin
//  @date 19-10-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef PARAMETERS_H
#define PARAMETERS_H


// ===============================================
// Versioning build settings
#define PSDTO3D_LICENSING_DISABLE
// ===============================================


#include "compLayers.h"
#include "mesh_generator/curve_mesh/curveMeshGenerator.h"	// for CurveParameters
#include "mesh_generator/influence_mesh/influenceMesh.h"	// for InfluenceParameters
#include "mesh_generator/linear_mesh/linearMesh.h"			// for BillboardMeshParameters, LinearMeshParameters
#include "mesh_generator/delaunay_mesh/delaunayMesh.h"		// for DelaunayMeshParameters
#include "util/helpers.h"
#include "util/bounds_2D.h"

typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <QString.h>
#include <set>

class QString;					// forward declaration
class QFrame;					// forward declaration
class QLabel;					// forward declaration

namespace psd_reader
{
	struct PsdData;				// forward declaration
	struct LayerData;			// forward declaration
}

namespace psd_to_3d
{
	struct GlobalParameters;	// forward declaration
	class SceneController;		// forward declaration
	using util::boundsPixels;


	//----------------------------------------------------------------------------------------------
	struct LayerParameters
	{
		enum Algorithm
		{
			LINEAR,
			DELAUNAY,
			CURVE,
			BILLBOARD
		};

		// User parameters
		Algorithm Algo = BILLBOARD;
		int AtlasIndex = -1; // texture atlas group index, -1 if no atlas
		bool EnableTextureCrop = true; // texture output cropping, if not using an atlas
		bool EnableInfluence = true;

		// Algorithm Parameters
		mesh_generator::BillboardMeshParameters BillboardParameters;
		mesh_generator::LinearMeshParameters LinearParameters;
		mesh_generator::DelaunayMeshParameters DelaunayParameters;
		mesh_generator::CurveParameters CurveParameters;
		mesh_generator::InfluenceParameters InfluenceParameters;

		// Setup flags, layer contents and geometry
		QString LayerName; // name of layer in psd file
		boundsPixels AnchorRegion; // layer anchor, copied from psd file
		int DepthIndex = 0;
		int CompLayerIndex[COMP_LAYER_COUNT]; // for base layers, index of each component layer
		int CompLayerCount = 0;         // number of component layers referring to this layer
		int CompLayerType = -1;          // for component layers, the component type
		int CompBaseLayerIndex = -1;     // for component layers, index of the layer this component refers to
		bool HasVectorSupport = false;   // has vector mask, for vector mode
		bool HasLinearSupport = false;   // has path object, for linear mode
		bool HasDelaunaySupport = false; // has path object, for delaunay mode (same as HasLinearSupport)

		// Status flags, for UI display
		// true if layer properties were modified since the last export
		bool IsModifiedMesh = false;
		bool IsModifiedTexture = false;
		bool IsFailedMesh = false;
		bool IsActive = true;

		// User Interface - Controls related to this layer
		QFrame* ListFrame;
		QLabel* LabelLayerTitle;
		QLabel* LabelAlgoSelected;
		QLabel* LabelLayerSize;
		QLabel* LabelAtlasName; // texture atlas group name
		QLabel* LabelAtlasSize;
		QLabel* LabelInfluence;
		QLabel* LabelDescription;

		LayerParameters() = default; // should not be used, make this private to verify
		LayerParameters( const QString& layerName );
		LayerParameters( const LayerParameters& that ) = default;
		~LayerParameters() = default;

		void ClearWidgets();
	};


	//----------------------------------------------------------------------------------------------
	// Layer display information, for the UI
	struct LayerStats
	{
		bool isAlgoSupported;
		bool isLayerReady;
		bool isAtlasReady;
		boundsPixels layerBounds;
		boundsPixels atlasBounds; // actual size of atlas, scaled down or padded up from fit size
		boundsPixels atlasBoundsFit; // raw fit size of atlas packing
	};


	//----------------------------------------------------------------------------------------------
	struct AtlasParameters
	{
		bool isModifiedTexture;
		bool isCustomSize;
		QString atlasName;
		int customSize;
		int customPadding;
		int packingAlgo; // from rbp::MaxRectsBinPack::FreeRectChoiceHeuristic
		std::set<int> layerIndices;

		AtlasParameters( const QString& atlasName )
		: isModifiedTexture(false), isCustomSize(false), atlasName(atlasName),
		  customSize(1024), customPadding(2), packingAlgo(3)
		{}
		~AtlasParameters() = default;
		bool IsMatch( const AtlasParameters& that ) const;

	private:
		AtlasParameters(); // not implemented, prevent default construction
	};


	//----------------------------------------------------------------------------------------------
	struct PreferenceParameters
    {
		QString FileImportPath = "";
		QString FileExportPath = "";
		// slider control variables; the internal parameter values corresponding to slider at 0, 50 and 100
		double DelaunayInnerDetailLo = 0.2000f, DelaunayInnerDetailMid = 0.0150f, DelaunayInnerDetailHi = 0.001f;
		double DelaunayOuterDetailLo = 0.2000f, DelaunayOuterDetailMid = 0.0150f, DelaunayOuterDetailHi = 0.001f;
		double DelaunayFalloffDetailLo = 1.00f, DelaunayFalloffDetailMid = 1.40f, DelaunayFalloffDetailHi = 2.00f;

        void Reset();
		void Store();
		void Fetch();
    };


	//----------------------------------------------------------------------------------------------
	struct GlobalParameters
	{
		GlobalParameters() = default;
		~GlobalParameters() = default;

		enum FileWriteMode // For PSDtoFBX
		{
			BINARY,
			ASCII
		};

		enum FileWriteLayout // For PSDtoFBX
		{
			SINGLE,				// combined FBX file for entire scene
			MULTI_PER_TEXTURE,	// separate FBX file per texture atlas
			MULTI_PER_LAYER		// separate FBX file per PSD layer
		};

		// Settings, user visible
		int TextureProxy = 1; // 1 for full size, 2 for half size, 4 for quarter size, 8 for eighth size
		float Depth = 0;
		float Scale = 1.0f;
		bool KeepGroupStructure = false;
		QString AliasPsdName = ""; // name of the root object or group in the exported file
		FileWriteMode FileWriteMode = FileWriteMode::BINARY;
		FileWriteLayout FileWriteLayout = FileWriteLayout::SINGLE;

		// Settings, non-user visible
		const int Padding = 5; // pixels of padding around images (atlas islands use setting in AtlasParams)
		const int Defringe = 4; // pixels of defringe around opaque regions of images

		QString PsdName = ""; // import filename without extension
		QString FileImportFilename = ""; // import filename with extension
		QString FileImportPath = ""; // import drive and path
		QString FileImportFilepath() { return FileImportPath + "/" + FileImportFilename; }
		QString FileExportPath = ""; // export drive and path
		QString FileExportName = ""; // export filename without extention, also filename prefix for export
		QString FileExportExt = ""; // export file extention

		// Preferences
		PreferenceParameters Prefs;

		void Reset(); // set default values
		void Fetch();
	};


	//----------------------------------------------------------------------------------------------
	struct LicensingParameters
    {
		QString UserInfoFirstName = "";
		QString UserInfoLastName = "";
		QString UserInfoEmail = "";
		QString LicenseKey = "";

        void Reset();
		void Store();
		void Fetch();
    };

	//----------------------------------------------------------------------------------------------
	struct ConfParameters
	{
		int language;
		enum { english, french };

        void Reset();
		void Fetch();
	};

}
#endif // PARAMETERS_H
