//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PythonMain.cpp
//  @author Michaelson Britt
//  @date 24-JAN-2025
//
//  @section DESCRIPTION
//  Exporter helpers for Python
//  Simplified interface, suitable for use with Python CTYPES library
//  Alternative to interface IPluginOutput / openPlugin() / setPluginOutput(), for use with FBX
//
//----------------------------------------------------------------------------------------------


// Local headers
#include "PythonMain.h"
#include <util\utils.h>
#include <util\math_2D.h>
#include <maya_mesh\MayaMeshConvertor.h>
#include <scene_controller\MeshGeneratorController.h>

// System headers
#include <string.h>
#include <locale.h>
#include <tchar.h>
#include <vector>

using util::Vector2F;
typedef std::vector<const GraphLayer*> GraphLayerList; // layer list
typedef std::map< std::string, GraphLayerList > GraphLayerListLookup; // texture name -> layer list lookup
// main plugin interface function, implemented in core
extern void setPluginOutput(void* pluginOutput );
extern void openPlugin( int runLoop );



extern "C" {
//--------------------------------------------------------------------------------------------------------------------------------------
// Python interface functions
//
// Wrapper functions, expose classes DataMesh + GraphLayer as PythonDataMesh
//
	typedef struct {
		float x,y,z;
	} PythonVector3F;
	const IPluginOutputParameters* GetParams( void* _self ) {
		const PythonDataMesh* self = (const PythonDataMesh*)_self;
		return (const IPluginOutputParameters*)(self->paramData);
	}
	const GraphLayer* GetGraphLayer( void* _self ) {
		const PythonDataMesh* self = (const PythonDataMesh*)_self;
		return (const GraphLayer*)(self->layerData);
	}
	const DataMesh* GetDataMesh( void* _self ) { return GetGraphLayer(_self)->Mesh.GetDataMesh(); }
	PythonVector3F MeshPositionFn(void* _self) {
		PythonDataMesh* self = (PythonDataMesh*)_self;
		PythonVector3F pos = {0, 0, 0};
		float layoutWidth = 1.0f;
		float layoutHeight = 1.0f / self->sceneAspect;
		if( GetParams(self)->PivotPosition() == IPluginOutputParameters::PivotPosition::LAYER_CENTER )
		{
			Vector2F center = GetDataMesh(self)->GetBoundsUV().GetCenter();
			pos.y =        (center.x)  * GetGraphLayer(self)->Scale * layoutWidth;
			pos.z = (1.0f -(center.y)) * GetGraphLayer(self)->Scale * layoutWidth;
		}
		pos.x = GetGraphLayer(self)->Depth;
		return pos;
	}
	int MeshVertCountFn(void* _self) { return GetDataMesh(_self)->GetVerticesCount(); }
	int MeshFaceCountFn(void* _self) { return GetDataMesh(_self)->GetFacesCount(); }
	int MeshFaceVertCountFn(void* _self, int f) { return GetDataMesh(_self)->GetFaceSizes()[f]; }
	int MeshFaceVertFn(void* _self, int f, int fv) {
		const DataMesh* mesh = GetDataMesh(_self);
		int mark = mesh->GetFaceMarks()[f];
		return GetDataMesh(_self)->GetFaceVerts()[mark+fv];
	}
	PythonVector3F MeshVertPosFn(void* _self, int v) {
		PythonDataMesh* self = (PythonDataMesh*)_self;
		util::Vector2F xy = GetDataMesh(self)->GetVertices()[v];
		util::Vector2F pivot(0,1); // lower-left corner by default
		float layoutWidth = 1.0f;
		float layoutHeight = 1.0f / self->sceneAspect;
		if( GetParams(self)->PivotPosition() == IPluginOutputParameters::PivotPosition::LAYER_CENTER )
		{
			Vector2F center = GetDataMesh(self)->GetBoundsUV().GetCenter();
			pivot = Vector2F( center.x, center.y );
		}
		// set x for depth, set to zero, and (y,z) for position, also flip vertically
		float x = 0;
		float y =  (xy.x - pivot.x) * GetGraphLayer(self)->Scale * layoutWidth;
		float z = -(xy.y - pivot.y) * GetGraphLayer(self)->Scale * layoutHeight; // y direction flipped
		PythonVector3F xyz = {x, y, z};
		return xyz;
	}
	PythonVector3F MeshVertUVFn(void* self, int v) {
		util::xformUV xformuv = GetGraphLayer(self)->XFormUV;
		util::Vector2F uv = xformuv.Transform( GetDataMesh(self)->GetVertices()[v] );
		// flip vertically
		PythonVector3F uvw = {uv.x, (1.0f-uv.y), 0};
		return uvw;
	}
	PythonDataMesh* NewPythonDataMesh()
	{
		// Manually implementing object-oriented programming in basic C... yay
		PythonDataMesh* mesh = new PythonDataMesh;
		// Set the self pointer
		mesh->self = mesh;
		// Set each member function pointer
		mesh->MeshPositionFn = MeshPositionFn;
		mesh->MeshVertCountFn = MeshVertCountFn;
		mesh->MeshFaceCountFn = MeshFaceCountFn;
		mesh->MeshFaceVertCountFn = MeshFaceVertCountFn;
		mesh->MeshFaceVertFn = MeshFaceVertFn;
		mesh->MeshVertPosFn = MeshVertPosFn;
		mesh->MeshVertUVFn = MeshVertUVFn;
		return mesh;
	}

	void DeletePythonDataMesh( PythonDataMesh* mesh )
	{
		if( mesh!=nullptr )
		{
			delete mesh;
		}
	}

//--------------------------------------------------------------------------------------------------------------------------------------
// Python interface function
// Main hook into python layer
// 
// DLL exported function, callable from Python CTYPES
// Requires linker option, under Linker -> All Options -> /export:python_setCallback
//
	typedef int (*CallbackFn)( PythonDataMesh* pythonDataMesh );
	void pythonSetCallback( CallbackFn callback )
	{
		PythonPluginOutput& instance = GetPythonPluginOutput();
		instance.SetCallback_OutputLayer( callback );

		openPlugin( 0 ); // open dialog, don't block running event loop
	}
}


namespace psd_to_3d
{

//--------------------------------------------------------------------------------------------------------------------------------------
//
// OutputTree()
// Runs at end of export process after all meshes and textures are generated
// Calls the python-registered callback, if any
//
void PythonPluginOutput::OutputTree( const PsdData& psdData, const IPluginOutputParameters& params, const GroupByNameMap& tree )
{
	if( callback_outputLayer==nullptr )
		return; // no python callback defined, abort

	PythonDataMesh* mesh = NewPythonDataMesh();
	int iterIndex = 0;
	float aspectRatio = (float)psdData.HeaderData.Width / (float)psdData.HeaderData.Height;

	// ----- Iterate over each groups (texture atlases)
	for( const GroupByNameMap::value_type& groupLayerItem : tree )
	{
		//  ----- Iterate over each layer in the current group ...
		const GraphLayerGroup& graphLayerGroup = groupLayerItem.second;
		for( int graphLayerIndex=0; graphLayerIndex<graphLayerGroup.GetLayerCount(); graphLayerIndex++ )
		{
			const GraphLayer* graphLayerPtr = graphLayerGroup[graphLayerIndex];
			if( graphLayerPtr==nullptr ) continue;
			
			std::wstring psdNameUTF16 = util::to_utf16( params.PsdName() );
			std::wstring layerNameUTF16 = util::to_utf16( graphLayerPtr->LayerName );
			std::wstring materialNameUTF16 = util::to_utf16( graphLayerPtr->MaterialName );
			std::wstring textureFilepathUTF16 = util::to_utf16( graphLayerPtr->TextureFilepath );
			mesh->paramData = (const void*)(&params);
			mesh->psdData = (const void*)(&psdData);
			mesh->layerData = (const void*)(graphLayerPtr);
			mesh->psdName = psdNameUTF16.data();
			mesh->exportIndex = iterIndex;
			mesh->layerIndex = graphLayerPtr->LayerIndex;
			mesh->sceneScale = graphLayerPtr->Scale;
			mesh->sceneDepth = graphLayerPtr->Depth;
			mesh->sceneAspect = aspectRatio;
			mesh->meshName = layerNameUTF16.data();
			mesh->materialName = materialNameUTF16.data();
			mesh->textureFilepath = textureFilepathUTF16.data();
			DebugPrint( "OutputTree(), mesh: 0x%I64X, callback: 0x%I64X\n", (__int64)mesh, (__int64)callback_outputLayer );
			callback_outputLayer( mesh );
			iterIndex++;
		}
	}

	DeletePythonDataMesh( mesh );
}

//--------------------------------------------------------------------------------------------------------------------------------------
//
// Startup code
// Installs the default plugin output module for Python when dll is loaded
// 
// This might be replaced by another plugin output,
// specifically in PSDtoFBX and PSDtoUnreal versions
// 
PythonPluginOutput& GetPythonPluginOutput() // store the singleton instance here
{
	static PythonPluginOutput* singleton = nullptr;
	if( singleton==nullptr )
	{
		singleton = new PythonPluginOutput();
	}
	return *singleton;
}
class PythonPluginInitializer // register the singleton instance here
{
public:
	PythonPluginInitializer()
	{
		setPluginOutput( &(GetPythonPluginOutput()) ); // constructor invokes startup code
	}
};
// singleton of the initializer registers singleton of the plugin output
PythonPluginInitializer pythonPluginInitializer; // runs when dll is loaded into memory

} // namespace psd_to_3d