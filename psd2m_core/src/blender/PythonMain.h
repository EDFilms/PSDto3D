//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PythonCommon.h
//  @author Michaelson Britt
//  @date 24-JAN-2020
//
//  @section DESCRIPTION
//  Exporter helpers for Python
//  Simplified interface, suitable for use with Python CTYPES library
//  Alternative to interface IPluginOutput / openPlugin() / setPluginOutput(), for use with FBX
//
//----------------------------------------------------------------------------------------------

#ifndef _PYTHONMAIN_H
#define _PYTHONMAIN_H

#include <IPluginOutput.h>

// Python interface functions
extern "C" {
	typedef const wchar_t* (PythonStringVoidFn)(void* self);
	typedef int (PythonIntVoidFn)(void* self);
	struct PythonDataMeshStruct {
		const void* self; // pointer to self
		const void* paramData; // IPluginOutputParameters
		const void* psdData; // PsdData
		const void* layerData; // GraphLayer

		int exportIndex; // Sequence number during export
		int layerIndex;
		float sceneScale;
		float sceneDepth;
		float sceneAspect; // Aspect ratio of the PSD file
		const wchar_t* meshName;
		const wchar_t* materialName;
		const wchar_t* textureFilepath;

		const void* MeshPositionFn;
		const void* MeshVertCountFn;
		const void* MeshFaceCountFn;
		const void* MeshFaceVertCountFn;
		const void* MeshFaceVertFn;
		const void* MeshVertPosFn;
		const void* MeshVertUVFn;
	};
	typedef struct PythonDataMeshStruct PythonDataMesh;
	PythonDataMesh* NewPythonDataMesh();
	void DeletePythonDataMesh( PythonDataMesh* mesh );

	typedef int (*OutputLayerCallback)( PythonDataMesh* mesh );
}

namespace psd_to_3d
{

class PythonPluginOutput : public IPluginOutput
{
public:
	PythonPluginOutput() : callback_outputLayer(nullptr) {}
	~PythonPluginOutput() {}

	// Simplified Python exposure does not implement any of these except OutputTree()
	void BeginSession(  const PsdData& psdData, const IPluginOutputParameters& params ) {}
	void OutputMesh(    const PsdData& psdData, const IPluginOutputParameters& params, const DataSurface& dataSurface, int layerIndex ) {}

	void OutputTree(    const PsdData& psdData, const IPluginOutputParameters& params, const GroupByNameMap& tree );
	void OutputTexture( const PsdData& psdData, const IPluginOutputParameters& params, const char* textureFilepath, const char* textureName ) {}
	void EndSession(    const PsdData& psdData, const IPluginOutputParameters& params ) {}
	void CancelSession( const PsdData& psdData, const IPluginOutputParameters& params ) {}
	void GetSaveDialogParams( void* ofnw, const IPluginOutputParameters& params ) {} // OPENFILENAMEW& ofn

	void SetCallback_OutputLayer( OutputLayerCallback cb ) { this->callback_outputLayer = cb; }

protected:
	OutputLayerCallback callback_outputLayer;
};

PythonPluginOutput& GetPythonPluginOutput(); // singleton instance

}


#endif // #ifndef _PYTHONMAIN_H


