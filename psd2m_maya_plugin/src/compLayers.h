//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2023, ED Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file parameters.h
//  @author Michaelson Britt
//  @date 17-04-2023
//
//  @section DESCRIPTION
//  Component Layer textures.  Components are PSD layers that combine with a base layer,
//  to create a single material on export.  For example, a painted normal map is placed in a
//  texture slot of the exported material.  In Photoshop, they are painted as additional layers
//  using the base name plus a postfix.  They must be pixel-matched to the base layer.  They use
//  the same UV coordinates on export.  Only the base layer is shown in the UI of PSDto3D.
//
//----------------------------------------------------------------------------------------------

#ifndef COMPLAYERS_H
#define COMPLAYERS_H

#include <string>


	//--------------------------------------------------------------------------------------------------------------------------------------
	// PSDtoUnreal, Master Material texture slot names and corresponding Photoshop layer name postfixes
	// 
	// PSD layer name postfix	| Map name				| Required parameter toggles
	//   "_normalmap"			|   "Normal"			|   "UseNormalMap"
	//   "_heightmap"			|   "Height"			|   "UseHeightMap"
	//   "_roughness"			|   "RoughnessMap"		|   "UseRoughness" + "UseRoughnessMap"
	//   "_occlusion"			|   "OcclusionMap"		|   "UseOcclusion" + "UseOcclusionMap"
	//   "_detailbasecolor"		|   "DetailBaseColor"	|   "UseDetailMap"
	//   "_detailnormalmap"		|   "DetailNormal"		|   "UseDetailMap"
	//   "_detailheightmap"		|   "DetailHeight"		|   "UseDetailMap"
	//   "_detailroughness"		|   "DetailRoughness"	|   "UseDetailMap"
	//   "_detailocclusion"		|   "DetailOcclusion"	|   "UseDetailMap"
	//   "_wpowind"				|   "WpoMaskMap"		|   "UseWpoWind"  + "UseWpoMask" + "UseWpoMaskMap"
	//   "_wponoise"			|   "WpoMaskMap"		|   "UseWpoNoise" + "UseWpoMask" + "UseWpoMaskMap"

	// BUG: "UseOcclusionRoughnessMetallicMap" has no corresponding texture?


namespace psd_to_3d
{

	//--------------------------------------------------------------------------------------------------------------------------------------
	// texture component layers
	enum COMP_LAYER_TYPE
	{
		NONE = -1, // not a component layer
		INFLUENCE_LAYER			= 0,	// mesh density influence layer (deprecated)
		NORMALMAP_LAYER			= 1,	// PSDtoUnreal, Normal map
		HEIGHTMAP_LAYER			= 2,	// PSDtoUnreal, Height map
		ROUGHNESS_LAYER			= 3,	// PSDtoUnreal, Roughness map
		OCCLUSION_LAYER			= 4,	// PSDtoUnreal, Occlusion map
		DETAILCOLOR_LAYER		= 5,	// PSDtoUnreal, Detail Color map
		DETAILNORMALMAP_LAYER	= 6,	// PSDtoUnreal, Detail Normal map
		DETAILHEIGHTMAP_LAYER	= 7,	// PSDtoUnreal, Detail Height map
		DETAILROUGHNESS_LAYER	= 8,	// PSDtoUnreal, Detail Roughness map
		DETAILOCCLUSION_LAYER	= 9,	// PSDtoUnreal, Detail Occlusion map
		WPOWIND_LAYER			= 10,	// PSDtoUnreal, WPO Mask map for WPO Wind
		WPONOISE_LAYER			= 11,	// PSDtoUnreal, WPO Mask map for WPO Noise
		COMP_LAYER_COUNT
	};

	static const std::string INFLUENCE_LAYER_TAG("influence");
	static const std::string NORMALMAP_LAYER_TAG("normalmap");
	static const std::string HEIGHTMAP_LAYER_TAG("heightmap");
	static const std::string ROUGHNESS_LAYER_TAG("roughness");
	static const std::string OCCLUSION_LAYER_TAG("occlusion");
	static const std::string DETAILCOLOR_LAYER_TAG("detailcolor");
	static const std::string DETAILNORMALMAP_LAYER_TAG("detailnormalmap");
	static const std::string DETAILHEIGHTMAP_LAYER_TAG("detailheightmap");
	static const std::string DETAILROUGHNESS_LAYER_TAG("detailroughness");
	static const std::string DETAILOCCLUSION_LAYER_TAG("detailocclusion");
	static const std::string WPOWIND_LAYER_TAG("wpowind");
	static const std::string WPONOISE_LAYER_TAG("wponoise");
	inline std::string const& GetCompLayerTag( int compLayerType ) // inline, avoids plugin linker problems
	{
		static const std::string blank("");
		switch( compLayerType )
		{
		case INFLUENCE_LAYER:		return INFLUENCE_LAYER_TAG;
		case NORMALMAP_LAYER:		return NORMALMAP_LAYER_TAG;
		case HEIGHTMAP_LAYER:		return HEIGHTMAP_LAYER_TAG;
		case ROUGHNESS_LAYER:		return ROUGHNESS_LAYER_TAG;
		case OCCLUSION_LAYER:		return OCCLUSION_LAYER_TAG;
		case DETAILCOLOR_LAYER:		return DETAILCOLOR_LAYER_TAG;
		case DETAILNORMALMAP_LAYER:	return DETAILNORMALMAP_LAYER_TAG;
		case DETAILHEIGHTMAP_LAYER:	return DETAILHEIGHTMAP_LAYER_TAG;
		case DETAILROUGHNESS_LAYER:	return DETAILROUGHNESS_LAYER_TAG;
		case DETAILOCCLUSION_LAYER:	return DETAILOCCLUSION_LAYER_TAG;
		case WPOWIND_LAYER:			return WPOWIND_LAYER_TAG;
		case WPONOISE_LAYER:		return WPONOISE_LAYER_TAG;
		}
		return blank;
	}

}

#endif // COMPLAYERS_H
