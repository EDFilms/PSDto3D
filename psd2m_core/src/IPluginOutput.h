//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, EDFilms.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file IPluginOutput.h
//  @author Michaelson Britt
//  @date 04-08-2020
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef IPLUGINOUTPUT_H
#define IPLUGINOUTPUT_H

#include "outputs.h"

class QString; // forward declaration
namespace psd_reader { struct PsdData; } // forward declaration
namespace mesh_generator { struct DataSurface; } // forward declaration
//struct OPENFILENAME; // forward declaration // TODO: Why doesn't this work?

namespace psd_to_3d
{
	// Exposes GlopalParameters, allows exports to avoid including
	class IPluginOutputParameters // rename to IPluginExporterParameters?
	{
	public:
		enum PivotPosition // see GlobalParameters::PivotPosition
		{
			LAYER_CORNER,
			LAYER_CENTER
		};
		enum FileWriteMode // see GlobalParameters::FileWriteMode
		{
			BINARY,
			ASCII
		}; // rename to ExportWriteMode?
		enum FileWriteLayout // see GlobalParameters::FileWriteLayout
		{
			SINGLE,
			MULTI_PER_TEXTURE,
			MULTI_PER_LAYER
		}; // rename to ExportWriteLayout?
		virtual int PivotPosition() const = 0;
		virtual int FileWriteMode() const = 0;
		virtual int FileWriteLayout() const = 0;
		virtual const char* FileImportPath() const = 0;
		virtual const char* PsdName() const = 0;
		virtual const char* FileExportPath() const = 0;
		virtual const char* FileExportName() const = 0;
		virtual const char* FileExportExt() const = 0;
		virtual const char* AliasPsdName() const = 0;
	};

	// Communication class to connect domain-specific exporters with the main export engine.
	// For example, a game engine export module can implement this, and will be notified
	// after the PNG files are created on disk, to embed those into the game scene.
	//
	// Setup is via  DllExport functions setPluginOutput() and openPlugin().
	// The domain exporter, for example a game engine plugin, can load the psd2m dll,
	// call setPluginOutput() to register itself, then call openPlugin() to open the UI.
	class IPluginOutput // rename to IPluginExporter?
	{
	public:
		typedef psd_reader::PsdData PsdData;
		typedef mesh_generator::DataSurface DataSurface;
		typedef psd_to_3d::GroupByNameMap GroupByNameMap;
		typedef psd_to_3d::IPluginOutputParameters IPluginOutputParameters;
		virtual ~IPluginOutput() = default;

		// TODO: Add concept of sessionId, returned by BeginSession() and passing into other methods, instead of assuming singleton session
		virtual void BeginSession(  const PsdData& psdData, const IPluginOutputParameters& params ) = 0;
		virtual void OutputMesh(    const PsdData& psdData, const IPluginOutputParameters& params, const DataSurface& dataSurface, int layerIndex ) = 0;
		virtual void OutputTree(    const PsdData& psdData, const IPluginOutputParameters& params, const GroupByNameMap& tree ) = 0;
		virtual void OutputTexture( const PsdData& psdData, const IPluginOutputParameters& params, const char* textureFilepath, const char* textureName ) = 0;
		virtual void EndSession(    const PsdData& psdData, const IPluginOutputParameters& params ) = 0;
		virtual void CancelSession( const PsdData& psdData, const IPluginOutputParameters& params ) = 0;
		virtual void GetSaveDialogParams( void* ofnw, const IPluginOutputParameters& params ) = 0; // OPENFILENAMEW& ofn // TODO: use real type here
	};

}
#endif // IPLUGINOUTPUT_H
