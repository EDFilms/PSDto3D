//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2022, EDFilms.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file toolWidgetLocalization_english.cpp
//  @author Michaelson Britt
//  @date 15-Mar-2022
//  
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#include "toolWidgetLocalization.h"

const util::StringTable::StringTableItem stringTableItems_english[] =
{
	//----------------------------------------------------------------------------------------------
	// Format strings and popup messages

	{ IDS_LICENSING_ERROR, "Error" },
	{ IDS_LICENSING_ERROR_MSG, "License activation failed.  Please check your registration information,\nand re-enter exactly shown in your Order Confirmation email." },

	{ IDS_TEXTURE_ATLAS_GROUP_DEFAULT, "Untitled" },
	{ IDS_TEXTURE_ATLAS_INFO_CURRENT_SIZE_FORMAT, "Atlas Resolution %i x %i" },
	{ IDS_TEXTURE_ATLAS_INFO_OPTIMAL_SIZE_FORMAT, "Optimal Atlas Resolution %i x %i" },
	{ IDS_TEXTURE_ATLAS_INFO_OPTIMAL_SIZE_CALCULATING, "calculating..." },
	{ IDS_TEXTURE_ATLAS_INFO_SCALING_FORMAT, "<font color='#f48c42'>Layers will be downscaled %i%% to fit</font>" },
	{ IDS_TEXTURE_ATLAS_INFO_PADDING_FORMAT, "<font color='#f48c42'>Padding of %i pixels will be applied</font>" },
	{ IDS_LAYER_LIST_SELECTED_STATUS_FORMAT, "%i of %i selected" }, // %i are numbers of selected and total layers, for example "1 of 20 selected"
	{ IDS_LAYER_LIST_CALCULATING, "calculating..." },
	{ IDS_LAYER_LIST_MODE_SELECTED, "Mode selected: " },
	{ IDS_LAYER_LIST_MODE_SUPPORTED, "Mode supported: " },
	{ IDS_LAYER_LIST_LINEAR, "Linear" },
	{ IDS_LAYER_LIST_DELAUNAY, "Delaunay" },
	{ IDS_LAYER_LIST_VECTOR, "Vector" },
	{ IDS_LAYER_LIST_BILLBOARD, "Billboard" },
	{ IDS_LAYER_LIST_SIZE, "Size: " },
	{ IDS_LAYER_LIST_GROUP, "Group: " },
	{ IDS_LAYER_LIST_GROUP_NONE, "None" },
	{ IDS_LAYER_LIST_ATLAS, "Atlas: " },
	{ IDS_LAYER_LIST_ATLAS_OVERSIZE, "oversize - remove layers" },
	{ IDS_LAYER_LIST_ATLAS_SCALE, "scale" },
	{ IDS_LAYER_LIST_ATLAS_PADDING, "padding" },
	{ IDS_LAYER_LIST_INFLUENCE, "Influence: " },
	{ IDS_LAYER_LIST_INFLUENCE_NOT_AVAILABLE, "Not available" },
	{ IDS_LAYER_LIST_INFLUENCE_ACTIVE, "Active" },
	{ IDS_LAYER_LIST_INFLUENCE_INACTIVE, "Inactive" },

	{ IDS_IMPORTING_PSD, "Loading PSD file..." },

	{ IDS_EXPORTING_CALCULATING_MESHES, "Calculating meshes..." },
	{ IDS_EXPORTING_PNG_TEXTURES, "Exporting textures..." },
	{ IDS_EXPORTING_PNG_ATLASES, "Exporting atlases..." },
	{ IDS_EXPORTING_JOB_FBX, "Exporting FBX..." },
	{ IDS_EXPORTING_JOB_MESH, "Exporting Mesh..." },
	{ IDS_EXPORTING_MESH_MESHES, "Exporting meshes..." },
	{ IDS_EXPORTING_MESH_SUCCCESS_1, "Export succeessful for " },
	{ IDS_EXPORTING_MESH_SUCCCESS_2, " mesh(es), failed for " },
	{ IDS_EXPORTING_MESH_SUCCCESS_3, " mesh(es)." },
	{ IDS_EXPORTING_PNG_ERROR_DIALOG, "PSD to 3D Message" },
	{ IDS_EXPORTING_FINALIZING, "Finalizing..." },
	{ IDS_EXPORTING_OK, "OK" },
	{ IDS_EXPORTING_CANCEL, "Cancel" },
	{ IDS_EXPORTING_DONE, "Done!" },

	{ IDS_PSD_LOAD_FILE_DIALOG, "Load Psd" },
	{ IDS_PSD_LOAD_FILE_DIALOG_PATTERN, "psd file (*.psd)" },

	{ IDS_FBX_SAVE_FILE_DIALOG, "Save Fbx" },
	{ IDS_FBX_SAVE_FILE_DIALOG_PATTERN_ASCII, "FBX ascii (*.fbx)" },
	{ IDS_FBX_SAVE_FILE_DIALOG_PATTERN_BINARY, "FBX binary (*.fbx)" },

	{ IDS_FBX_SAVE_PATH_DIALOG, "Export Path" },

	{ IDS_MAYA_TEXTURE_NODE_PREFIX, "TextureAtlas_" },
	{ IDS_MAYA_GROUP_NODE_POSTFIX, "_Group_PSDto3D" },
	{ IDS_MAYA_SPLINE_NODE_POSTFIX, "_Spline" },

	{ IDS_HELP_URL, "https://edfilms.notion.site/PSD-to-3D-user-manual-6221458afd534a5aa14a46b77048a396" },

	//----------------------------------------------------------------------------------------------
	// UI controls

	{ IDS_MAIN_WIDGET_FBX, "PSD to FBX" },
	{ IDS_MAIN_WIDGET_MAYA, "PSD to Maya" },
	{ IDS_LICENSING_WIDGET_FBX, "PSD to FBX License" },
	{ IDS_LICENSING_WIDGET_MAYA, "PSD to Maya License" },

	//----------------------------------------------------------------------------------------------
	// Tooltips

	{ IDS_GENERATE_MESH_TOOLTIP_FBX, "Generate meshes of the selected layers as an FBX. Prompts for output folder." },
	{ IDS_GENERATE_PNG_TOOLTIP_FBX, "Export textures of the selected layers as PNG files. Prompts for output folder." },
	{ IDS_GENERATE_BOTH_TOOLTIP_FBX, "Export the meshes and textures of the selected layers as an FBX and multiple PNG files.  Prompts for output FBX file name; PNG files are saved in the same folder." },

	{ IDS_GENERATE_MESH_TOOLTIP_MAYA, "Generate meshes of the selected layers as objects in the scene." },
	{ IDS_GENERATE_PNG_TOOLTIP_MAYA, "Export textures of the selected layers as PNG files in a folder beneath the PSD source." },
	{ IDS_GENERATE_BOTH_TOOLTIP_MAYA, "Export meshes and textures of the selected layers. Generates meshes as textured objects in the scene.  PNG files are saved in a folder beneath the PSD source." },

	{ IDS_GENERATION_ALGO_TOOLTIP_FBX, "<html><head/><body><p><span style=\" font-weight:600;\">Delaunay Mode</span>: Generates a mesh with adjustable density using the Delaunay Triangulation algorithm. Requires a closed path to define the mesh perimeter, saved in the Photoshop Paths list, with a name matching the layer name.</p><p><span style=\" font-weight:600;\">Vector Mode</span>: Generates a mesh from custom curves drawn by the user. Requires a Photoshop Vector Mask consisting of at least one closed perimeter path with additional paths drawn through the perimeter. Each path intersection will result in one vertex.</p><p><span style=\" font-weight:600;\">Billboard Mode</span>: Generates a rectangular billboard mesh cropped to the alpha of the layer. Does not require additional Photoshop setup.</p></body></html>" },
	{ IDS_GENERATION_ALGO_TOOLTIP_MAYA, "<html><head/><body><p><span style=\" font-weight:600;\">Linear Mode</span>: Generates a mesh with adjustable density using the Planar NURBS Tessellation modifier in Maya. Requires a closed path to define the mesh perimeter, saved in the Photoshop Paths list, with a name matching the layer name.</p><p><span style=\" font-weight:600;\">Delaunay Mode</span>: Generates a mesh with adjustable density using the Delaunay Triangulation algorithm. Requires a closed path to define the mesh perimeter, saved in the Photoshop Paths list, with a name matching the layer name.</p><p><span style=\" font-weight:600;\">Vector Mode</span>: Generates a mesh from custom curves drawn by the user. Requires a Photoshop Vector Mask consisting of at least one closed perimeter path with additional paths drawn through the perimeter. Each path intersection will result in one vertex.</p><p><span style=\" font-weight:600;\">Billboard Mode</span>: Generates a rectangular billboard mesh cropped to the alpha of the layer. Does not require additional Photoshop setup.</p></body></html>" },



	{ -1, "" } // bookend
};

const util::StringTable::StringTableItem* GetStringTableItems_english() { return stringTableItems_english; }
