//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file pluginController.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

// TODO: Need this define, otherwise error linking method QByteArray QString::toUtf8() const & Q_REQUIRED_RESULT
//#define QT_COMPILING_QSTRING_COMPAT_CPP // added in project settings
// --
// Local headers

#include "pluginController.h"
#include "IPluginOutput.h"
#include "mainWindowCmd.h" // for class PluginContext
#include "mayaUtils.h" // for NormalizeName()
#include "interface/toolWidget.h"
#include "interface/toolWidgetLocalization.h"
#include "scene_controller/sceneController.h"
#include "texture_exporter/textureExporter.h"
#include "psd_reader/psdReader.h"
#include "util/utils.h"
#include "util/progressJob.h"

// System headers
// --
typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <Windows.h> // for OPENFILENAMEW

#include <chrono>
#include <QString>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox.h>


namespace psd_to_3d
{
	using psd_reader::PsdReader;
	using psd_reader::LayerData;
	using psd_reader::ResourceBlockPath;


	//--------------------------------------------------------------------------------------------------------------------------------------
	PluginController::PluginController(PluginContext* context)
	: context(context), psdData(nullptr), scene(nullptr)
	{
		// set up string translation
		if( context->GetConfParams()->language == ConfParameters::french )
			util::GetLocalizationStringTable()->AddStrings( IDC_MAIN, GetStringTableItems_french(), false );
		else // english
			util::GetLocalizationStringTable()->AddStrings( IDC_MAIN, GetStringTableItems_english(), true );

		psdData = new PsdData();
		scene = new SceneController(*psdData,*this);
		TimerStart();
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	PluginController::~PluginController()
	{
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	const PluginContext& PluginController::GetContext() const
	{
		return *context;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	PluginContext& PluginController::GetContext()
	{
		return *context;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	const PsdData& PluginController::GetPsdData() const
	{
		return *psdData;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	PsdData& PluginController::GetPsdData()
	{
		return *psdData;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	const SceneController& PluginController::GetScene() const
	{
		return *scene;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	IPluginOutput& PluginController::GetOutput()
	{
		return *(context->GetPluginOutput());
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	SceneController& PluginController::GetScene()
	{
		return *scene;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	IPluginController::NotifyStatus& PluginController::GetNotifyStatus()
	{
		return notifyStatus;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// Displays the file save dialog
	int GetFileExportPath( PluginController* controller, IPluginOutput* output ) // TODO: Should be member method of PluginController?
	{
		bool retval = true;
		SceneController& scene = controller->GetScene();
		GlobalParameters& globalParams = scene.GetGlobalParameters();
#if (defined PSDTO3D_UNREAL_VERSION) || (defined PSDTO3D_MAYA_VERSION)
		QString exportDir = globalParams.FileImportPath + "/" + globalParams.PsdName + "/";
		globalParams.FileExportPath = exportDir;
#elif defined PSDTO3D_FBX_VERSION
		// get the file extension from the output module
		OPENFILENAMEW ofnw;
		PluginOutputParameters pluginOutputParams(globalParams);
		output->GetSaveDialogParams( &ofnw, pluginOutputParams );
		// ofnw.lpstrDefExt points to a static buffer, owned by the output module
		globalParams.FileExportExt = QString::fromWCharArray( ofnw.lpstrDefExt, (int)wcslen(ofnw.lpstrDefExt) );
		if( globalParams.FileExportPath.isEmpty() || globalParams.FileExportName.isEmpty() )
		{
			ToolWidget* toolWidget = controller->GetContext().GetToolWidget();
			retval = toolWidget->ExportFilenameSelector();
		}
#endif
		return (retval? 1:0);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::NotifyCommand()
	{
		PingTimer::SuspendGuard suspendPing(*pingTimer); // suspend background scene evaluation until export is finished

		GlobalParameters& globalParams = GetScene().GetGlobalParameters();
		PluginOutputParameters pluginOutputParams(globalParams);

		if( (notifyStatus.FileImportRequest) && (!notifyStatus.FileImportFilepath.empty()) )
		{
			const QString path( notifyStatus.FileImportFilepath.data() );
		
			ParsePsdData(path); // delete the scene, read the psd data, and re-initialize the scene
		}
		else if( notifyStatus.ParametersChanged )
		{
			Repaint(true);
		}
		else if( notifyStatus.DescriptionsChanged )
		{
			Repaint(false);
		}

		IPluginOutput* output = this->context->GetPluginOutput();
		bool doSession = (notifyStatus.ExportPng || notifyStatus.ExportMesh) && (output!=nullptr);

		// Plugin output session
		if( doSession )
		{
			// Query for the output file pathname
			int ok = GetFileExportPath( this, output );
			if( ok==0 )
			{
				// user clicked cancel, abort
				doSession = false;
			}
			else
			{
				// Notify plugin of initial params
				output->BeginSession( GetPsdData(), pluginOutputParams );
			}
		}

		// Double-check, because user might have cancelled ...
		if( doSession )
		{
			ActiveLayerFilter filter( GetPsdData().LayerMaskData, GetScene(), notifyStatus.ExportAll );
			BeginProgressExport( filter, notifyStatus.ExportPng, notifyStatus.ExportMesh, notifyStatus.ExportAll );

			// mesh preparation task; advance progress bar
			UpdateProgressExport( "Calculating meshes..." ); // TODO: localize this
			scene->PrepareMeshes( filter, GetProgressExport().GetTask() );

			// Export PNG
			if( notifyStatus.ExportPng  &&  !(GetProgressExport().IsCancelled()) )
			{
				ExportTexture( filter, globalParams.FileExportPath );
			}

			// Generate mesh
			if( notifyStatus.ExportMesh  &&  !(GetProgressExport().IsCancelled()) )
			{
				GenerateMesh( filter );
			}

			if( GetProgressExport().IsCancelled() )
				 output->CancelSession( GetPsdData(), pluginOutputParams );
			else
			{
				output->EndSession( GetPsdData(), pluginOutputParams );
				// clear modified flags after successful export
				if( notifyStatus.ExportPng ) GetScene().NotifyPostExportTexture(filter); 
				if( notifyStatus.ExportMesh ) GetScene().NotifyPostExportMesh(filter);
			}

			// finish progress bar, and repaint
			EndProgressExport();
			context->GetToolWidget()->RequestRepaint(true);
		}

		notifyStatus.Reset(); // clear the status command flags
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::ParsePsdData(QString const& pathStr)
	{
		GlobalParameters& globalParams = GetScene().GetGlobalParameters();
		ToolWidget* toolWidget = context->GetToolWidget();
		ProgressAgent* progressImport = &(toolWidget->GetProgress());
		QFileInfo fileInfo( pathStr.toUtf8().data() );
		QFileInfo folderInfo( fileInfo.path() + "/" + fileInfo.baseName() ); // new folder adjacent to PSD file
		QString prevFileImportFilepath = globalParams.FileImportFilepath();
		QString prevFileExportPath = globalParams.FileExportPath;
		QString prevFileExportName = globalParams.FileExportName;

		if( !(fileInfo.exists()) )
			return; // should never happen

		// Update global defaults
		globalParams.Prefs.FileImportPath = fileInfo.path();
		globalParams.Prefs.Store();

		progressImport->BeginProgressBar( "Loading PSD file...", 1, true ); // PSD reader will update number of steps later

		// Create folder before the Parse starts
		std::string newFolderName_utf8 = folderInfo.absoluteFilePath().toUtf8();
		if (!folderInfo.exists())
			_wmkdir( util::to_utf16(newFolderName_utf8).c_str() );

		// Perform the parse
		PsdReader reader(fileInfo.absoluteFilePath().toUtf8().data());
		reader.SetProgress( progressImport );
		PsdData tempData = reader.ParsePsd();

		if( !progressImport->IsCancelled() )
		{
			GetScene().Free(); // delete the previous scene

			*(this->psdData) = tempData; // copy result into local psdData

			// Update global params
			globalParams.PsdName = fileInfo.completeBaseName();
			globalParams.FileImportFilename = fileInfo.fileName();
			globalParams.FileImportPath = fileInfo.path();
			bool isNewFile = (prevFileImportFilepath!=globalParams.FileImportFilepath()); // not a reload

			// Convert special characters in layer names
			std::vector<LayerData>& layers = GetPsdData().LayerMaskData.Layers;
			for(std::vector<LayerData>::iterator it = layers.begin(); it!=layers.end(); it++ )
			{
				// convert layer name from Latin1 (ascii) to UTF8 (multibyte)
				it->LayerName = QString::fromLatin1( it->LayerName.c_str() ).toUtf8();
				it->LayerDisplayName = it->LayerName;
#if defined PSDTO3D_MAYA_VERSION
				// convert special characters which Maya can't display
				it->LayerName = maya_plugin::MayaUtils::NormalizeName(it->LayerDisplayName);
#endif // PSDTO3D_MAYA_VERSION
			}
			// Convert special characters in resources, must match layer names
			std::vector<ResourceBlockPath>& resources = GetPsdData().ImageResourceData.ResourceBlockPaths;
			for (std::vector<ResourceBlockPath>::iterator it_path = resources.begin(); it_path != resources.end(); it_path++)
			{
				// convert path name from Latin1 (ascii) to UTF8 (multibyte)
				it_path->Name = QString::fromLatin1( it_path->Name.c_str() ).toUtf8();
#if defined PSDTO3D_MAYA_VERSION
				it_path->Name = maya_plugin::MayaUtils::NormalizeName(it_path->Name);
#endif // PSDTO3D_MAYA_VERSION
			}

			GetScene().Init(); // initialize scene with the new psdData

			if( IsFbxVersion )
			{
				// FBX version forces these blank during regular load, or keeps them blank if not already set during reload;
				// system will prompt with file save dialog at first export
				if( isNewFile || (prevFileExportPath.isEmpty()) ) 
					globalParams.FileExportPath = "";
				if( isNewFile || (prevFileExportName.isEmpty()) )
					globalParams.FileExportName = "";
			}
			else
			{
				// Calculate export path from filename
				QString exportDir = fileInfo.path() + "/" + fileInfo.baseName() + "/";
				globalParams.FileExportPath = exportDir;
				globalParams.FileExportName = fileInfo.completeBaseName();
			}

			toolWidget->SetPsdData(GetPsdData()); // initial UI
		}

		progressImport->EndProgressBar(false); // don't keep progress dialog open
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::Repaint(bool updateLayout)
	{
		// Trigger signal for UI refresh
		ToolWidget* toolWidget = context->GetToolWidget();
		toolWidget->RequestRepaint(updateLayout);
	}

	// TODO: Create full PerfLog class, or whatever it's supposed to be called
	typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
	double perf_log_a_total = 0.0; // total time for all texture and atlas writes
	double perf_log_b_total = 0.0; // time only for file writing to disk
	int perf_log_a_count = 0;
	int perf_log_b_count = 0;

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::ExportTexture(ActiveLayerFilter& filter, QString const& path) const
	{
		const GlobalParameters globalParams = GetScene().GetGlobalParameters();
		const PsdData& psdData = GetPsdData();
		ToolWidget* toolWidget = context->GetToolWidget();

		ExportAtlas(filter,path);

		time_point time_start_a = std::chrono::high_resolution_clock::now();

		for( auto const& layer : GetPsdData().LayerMaskData.Layers )
		{
			int layerIndex = iter_index(layer,GetPsdData().LayerMaskData.Layers);
			if( filter(layerIndex ) )
			{
				const LayerAgent& layerAgent = GetScene().GetLayerAgent(layerIndex); 
				const LayerParameters& layerParams = layerAgent.GetLayerParameters();
				bool layerAtlassed = (layerParams.AtlasIndex>=0);

				// Verify the layer is exportable, and not part of an atlas (already exported above)
				if( !layerAtlassed )
				{
					// texture export task; advance progress bar
					// update progress for each active layer WITHOUT atlas
					this->UpdateProgressExport("Exporting textures...", true); // TODO: localize this

					// Generate the layer bounds; requires mesh evaluation, and waits until evaluation is finished
					boundsPixels layerBounds;
					layerAgent.GenerateLayerBounds( layerBounds );

					//std::vector<unsigned char> tmptexture;
					TextureMap tmptexture;

					if( layerParams.EnableTextureCrop )
					{
						//void Init( int width, int height, int bitDepth, int channels=4, int padding=0, int widthScaled=-1, int heightScaled=-1 );
						tmptexture.Init( layerBounds.WidthPixels(), layerBounds.HeightPixels() );

						TextureExporter::ConvertIffFormat( tmptexture, layer, psdData.HeaderData.BitsPerPixel, layerBounds );

						tmptexture.ApplyPadding( globalParams.Padding );
					}
					else
					{
						boundsPixels fullBounds = boundsPixels( 0, 0, psdData.HeaderData.Width, psdData.HeaderData.Height );

						tmptexture.Init( psdData.HeaderData.Width, psdData.HeaderData.Height );

						TextureExporter::ConvertIffFormat( tmptexture, layer, psdData.HeaderData.BitsPerPixel, fullBounds);
					}

					if( tmptexture.Empty() ) continue; // error handling
					if( GetProgressExport().IsCancelled() ) break; // cancel handling

					bool hasAlpha = (layer.NbrChannel==4);
					if( hasAlpha && (!tmptexture.Empty()) ) // cannot defringe transparent pixels if no alpha channel
					{
						TextureExporter::Defringe( tmptexture, globalParams.Defringe, GetProgressExport().GetTask() );
					}

					if( GetProgressExport().IsCancelled() ) break; // cancel handling

					// Verify the conversion from PSD to PNG succeeded
					if( !tmptexture.Empty() )
					{
						std::string pathname_utf8 = path.toUtf8();
						int pathCreated = _wmkdir( util::to_utf16(pathname_utf8).c_str() ); // 0 if created, EEXIST if already exists, ENOENT if not found
#if defined PSDTO3D_FBX_VERSION
						// TODO: centralize calculation of output filename,
						// currently smeared between ExportTexture(), ExportAtlas(), CreateTreeStructure() and IPluginOutput methods
						QString pngNameFile = QString(path + "/" + globalParams.FileExportName + "_" + layer.LayerName.c_str() + ".png");
#else
						QString pngNameFile = QString(path + "/" + layer.LayerName.c_str() + ".png");
#endif
						std::string pngNameFileStr_utf8 = pngNameFile.toUtf8();
						std::string psdNameFileStr_utf8 = globalParams.PsdName.toUtf8();
						std::string psdLayerNameStr = layer.LayerName;
						int textureWidth = layerBounds.WidthPixels();
						int textureHeight = layerBounds.HeightPixels();

						// apply proxy scaling if appropriate
						if( globalParams.TextureProxy!=1 )
						{
							int texWidth = tmptexture.Width() / globalParams.TextureProxy;
							int texHeight = tmptexture.Height() / globalParams.TextureProxy;
							tmptexture.ApplyScaling( texWidth, texHeight ); // rescale source bitmap, changes size and reallocates data
						}

						time_point time_start_b = std::chrono::high_resolution_clock::now();

						TextureExporter::SaveToDiskLibpng( pngNameFile, tmptexture.Buffer(),
							tmptexture.Width(), tmptexture.Height(), GetProgressExport().GetTask() );

						time_point time_now = std::chrono::high_resolution_clock::now();
						std::chrono::duration<float> time_diff_b = time_now-time_start_b;
						perf_log_b_total += time_diff_b.count();
						perf_log_b_count++;

						// notify output plugin of the texture file (for FBX or Unreal)
						std::string textureFilepath = pngNameFileStr_utf8;
						std::string textureName = layer.LayerName;
						IPluginOutput* output = this->context->GetPluginOutput();
						if( output!=nullptr )
						{
							PluginOutputParameters pluginOutputParams(globalParams);
							output->OutputTexture( GetPsdData(), pluginOutputParams, textureFilepath.c_str(), textureName.c_str() );
						}
					}
				}
			}
		}

		time_point time_now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> time_diff_a = time_now-time_start_a;
		perf_log_a_total += time_diff_a.count();
		perf_log_a_count++;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::ExportAtlas(ActiveLayerFilter& filter, QString const& path) const
	{
		time_point time_start_a = std::chrono::high_resolution_clock::now();

		const GlobalParameters globalParams = GetScene().GetGlobalParameters();
		maya_plugin::ToolWidget* toolWidget = this->context->GetToolWidget();

		const AtlasAgentList& atlasAgents = GetScene().GetAtlasAgents(); // use const version since this is a const method
		const LayerAgentList layerAgents = GetScene().CollectLayerAgents(true);

		for( const AtlasAgent* const& atlasAgentPtr : atlasAgents )
		{
			int atlasIndex = iter_index( atlasAgentPtr, atlasAgents );
			if( atlasAgentPtr==nullptr ) continue; // should never happen

			if( GetProgressExport().IsCancelled() ) continue; // cancel handling

			const AtlasAgent atlasAgent = *atlasAgentPtr;
			const AtlasParameters& atlasParams = atlasAgent.GetAtlasParameters();

			// Generate the layer bounds; requires mesh evaluation, and waits until evaluation is finished
			boundsPixels atlasBounds;
			BoundsByLayerIndexMap dstLayerBounds;
			atlasAgent.GenerateAtlasBounds( atlasBounds, dstLayerBounds );

			int atlasWidth = atlasBounds.WidthPixels();
			int atlasHeight = atlasBounds.WidthPixels();
			const QString atlasNameNoSpace = atlasParams.atlasName.simplified().replace( " ", "_" );

			bool anyActive = false;
			for( const LayerAgent* const& layerAgentPtr : layerAgents )
			{
				int layerIndex = iter_index( layerAgentPtr, layerAgents );
				if( layerAgentPtr==nullptr ) continue; // should never happen

				const LayerAgent& layerAgent = *layerAgentPtr;
				const LayerParameters& layerParams = layerAgent.GetLayerParameters();

				if( filter(layerIndex) && (layerParams.AtlasIndex == atlasIndex) )
				{
					anyActive=true;
				}
			}

			if( anyActive && (atlasWidth>0) && (atlasHeight>0) )
			{
				TextureMap atlasTexture;
				atlasTexture.Init( atlasWidth, atlasHeight );

				for( const LayerAgent* const& layerAgentPtr : layerAgents )
				{
					int layerIndex = iter_index( layerAgentPtr, layerAgents );
					if( layerAgentPtr==nullptr ) continue; // should never happen

					if( GetProgressExport().IsCancelled() ) continue; // cancel handling

					const LayerAgent& layerAgent = *layerAgentPtr;
					const LayerParameters& layerParams = layerAgent.GetLayerParameters();

					// ignore the filter; include all layers using this atlas

					if( layerParams.AtlasIndex == atlasIndex )
					{
						// atlas export task; advance progress bar
						// update progress for each active layer WITH and atlas
						UpdateProgressExport( "Exporting atlases...", true ); // TODO: localize this

						const PsdData& psdData = GetPsdData();
						const LayerData& layer = psdData.LayerMaskData.Layers[layerIndex];

						// Generate the layer bounds; requires mesh evaluation, and waits until evaluation is finished
						boundsPixels srcLayerBounds;
						layerAgent.GenerateLayerBounds( srcLayerBounds );

						// sizes of entire src and dst images, not bounding boxes, for traversal
						int dstImageWidth = atlasWidth;
						int dstImageHeight = atlasHeight;
					
						boundsPixels srcBounds = srcLayerBounds;
						boundsPixels dstBounds = dstLayerBounds[layerIndex]; // bounds includes the padding pixels...
						// only copy pixels to unpadded area of dstRegion; exclude padding in AtlasRegion

						// TODO: apply downscaling to padding
						dstBounds.Expand( -globalParams.Padding );

						TextureMap tmptexture;
						tmptexture.Init( srcBounds.WidthPixels(), srcBounds.HeightPixels() );

						TextureExporter::ConvertIffFormat( tmptexture, layer, psdData.HeaderData.BitsPerPixel, srcBounds );

						bool is_rotated = srcBounds.IsRotatedRelativeTo( dstBounds ); // Rotate texture if packing rotated the layer
						int copy_width  = MIN(srcBounds.WidthPixels(),   (is_rotated?  dstBounds.HeightPixels() : dstBounds.WidthPixels()) );
						int copy_height = MIN(srcBounds.HeightPixels(),  (is_rotated?  dstBounds.WidthPixels()  : dstBounds.HeightPixels()) );
						int pixel_size = 4; // assume output is rgba 1 byte per channel

						if( (tmptexture.Data()==nullptr) || (copy_width<=0) || (copy_height<=0) ) continue;

						tmptexture.ApplyScaling( copy_width, copy_height ); // rescale source bitmap, changes size and reallocates data

						unsigned char* src = tmptexture.Data();
						unsigned char* dst = atlasTexture.Data();

						for( int i=0; i<copy_height; i++ )
						{
							int src_offset = (i * copy_width) * pixel_size;
							if( is_rotated )
							{
								for( int j=(copy_width-1); j>=0; j--, src_offset+=4 )
								{
									int dst_offset = (((j + dstBounds.YPixels()) * dstImageWidth) + (i + dstBounds.XPixels())) * pixel_size;
									memcpy( dst + dst_offset, src + src_offset, pixel_size );
								}
							}
							else
							{
								// Not rotated, more efficient copy
								int dst_offset = (((i + dstBounds.YPixels()) * dstImageWidth) + dstBounds.XPixels()) * pixel_size;
								memcpy( dst + dst_offset, src + src_offset, copy_width * pixel_size );
							}
						}
					}
				}

#if defined PSDTO3D_FBX_VERSION
				// TODO: centralize calculation of output filename,
				// currently smeared between ExportTexture(), ExportAtlas(), CreateTreeStructure() and IPluginOutput methods
				QString binFilename = QString(path + "/" + globalParams.FileExportName + "_TextureAtlas_" + atlasNameNoSpace + ".png");
#else
				QString binFilename = QString(path + "/" + "TextureAtlas_" + atlasNameNoSpace + ".png");
#endif
				if( !(GetProgressExport().IsCancelled()) ) // cancel handling
				{
					TextureExporter::Defringe( atlasTexture, globalParams.Defringe, GetProgressExport().GetTask() );

					// apply proxy scaling if appropriate
					if( globalParams.TextureProxy!=1 )
					{
						atlasWidth /= globalParams.TextureProxy;
						atlasHeight /= globalParams.TextureProxy;
						atlasTexture.ApplyScaling( atlasWidth, atlasHeight ); // rescale source bitmap, changes size and reallocates data
					}

					time_point time_start_b = std::chrono::high_resolution_clock::now();

					TextureExporter::SaveToDiskLibpng( binFilename, atlasTexture.Buffer(),
						atlasWidth, atlasHeight, GetProgressExport().GetTask() );

					time_point time_now = std::chrono::high_resolution_clock::now();
					std::chrono::duration<float> time_diff_b = time_now-time_start_b;
					perf_log_b_total += time_diff_b.count();
					perf_log_b_count++;

					// notify output plugin of the texture file (for FBX or Unreal)
					std::string textureFilepath = binFilename.toUtf8();
					std::string textureName = std::string("TextureAtlas_") + std::string( atlasNameNoSpace.toUtf8().data() );
					IPluginOutput* output = this->context->GetPluginOutput();
					if( output!=nullptr )
					{
						PluginOutputParameters pluginOutputParams(globalParams);
						output->OutputTexture( GetPsdData(), pluginOutputParams, textureFilepath.c_str(), textureName.c_str() );
					}

				}
			}
		}

		time_point time_now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> time_diff_a = time_now-time_start_a;
		perf_log_a_total += time_diff_a.count();
		perf_log_a_count++;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::GenerateMesh(ActiveLayerFilter& filter) const
	{
		const GlobalParameters globalParams = GetScene().GetGlobalParameters();
		ToolWidget* toolWidget = this->context->GetToolWidget();
		IPluginOutput* output = this->context->GetPluginOutput();
		int layerCount = filter.Count();

		// mesh generation task; advance progress bar
		UpdateProgressExport( "Exporting meshes...", true ); // TODO: localize this

		// STEP 1. Generate all meshes
		GraphLayerByIndexMap layerOutputs;
		scene->GenerateMeshes( layerOutputs, filter, GetProgressExport().GetTask() );

		// Check results for error message handling below
		int meshCountProduced = (int)(layerOutputs.size());
		int meshCountExpected = filter.Count();
		for( int layer_index=0; layer_index<GetPsdData().LayerMaskData.Layers.size(); layer_index++ )
		{
			if( filter(layer_index) ) // if layer was supposed to be output
			{
				LayerAgent& layer = this->scene->GetLayerAgent(layer_index);
				LayerParameters& layerParams = layer.GetLayerParameters();
				// set flag if layer failed to output, clear flag otherwise
				layerParams.IsFailedMesh = (layerOutputs.find(layer_index)==layerOutputs.end());
			}
		}


		// STEP 2. Generate the hierarchy tree, centralize data, transfer ownership of meshes
		// Creates the GraphLayer objects, centralizing all information about each mesh and its params,
		// including calculation of xformUV for each mesh based on its layerRegion and atlasRegion
		// Tree takes ownership of all mesh objects; previous mesh list becomes empty
		GroupByNameMap tree;
		if( !layerOutputs.empty() )
		{
			scene->CreateTreeStructure(tree, layerOutputs, filter);
		}

		// STEP 3. Notify the PluginOutput for each mesh, if applicable
		if( (!tree.empty()) && (output!=nullptr) )
		{
			// STEP 3a. Create a lookup from layerIndex->DataSurface
			MeshByLayerIndexRefMap meshesRef;
			for( auto const& groupItem : tree )
			{
				const GraphLayerGroup& graphLayerGroup = groupItem.second;
				//for( auto const& layerItem : groupItem.second.GraphLayers )
				for( int graphLayerIndex=0; graphLayerIndex<graphLayerGroup.GetLayerCount(); graphLayerIndex++ )
				{
					const GraphLayer* graphLayerPtr = graphLayerGroup[graphLayerIndex];
					if( graphLayerPtr==nullptr ) continue;

					const GraphLayer& layerItem = *graphLayerPtr;
					meshesRef.insert( std::pair<int,const DataSurface&>(layerItem.LayerIndex, layerItem.Mesh) );
				}
			}

			// STEP 3b. Iterate through meshes in order of layerIndex
			for( auto const& meshItem : meshesRef )
			{
				int layerIndex = meshItem.first;
				const DataSurface& mesh = meshItem.second;
				if( filter(layerIndex) )
				{
					PluginOutputParameters pluginOutputParams(globalParams);
					output->OutputMesh(GetPsdData(), pluginOutputParams, mesh, layerIndex);
				}
			}
		}

		// STEP 4. Notify the plugin for the hierarchy tree

		// mesh finalizing task; advance progress bar
		UpdateProgressExport( "Finalizing...", true ); // TODO: localize this

		if( (!tree.empty()) && (output!=nullptr) )
		{
			PluginOutputParameters pluginOutputParams(globalParams);
			output->OutputTree(GetPsdData(), pluginOutputParams, tree);
		}

		tree.clear(); // delete all meshes

		if( meshCountProduced != meshCountExpected )
		{
			QMessageBox msgBox;
			// TODO: Localize this section
			msgBox.setWindowTitle("PSD to 3D Message");
			msgBox.setText("ERROR: Some layers failed to export.");
			QString successCountStr = std::to_string(meshCountProduced).c_str();
			QString errorCountStr = std::to_string(meshCountExpected-meshCountProduced).c_str();
			QString infoStr = ("Export succeessful for "+successCountStr+" mesh(es), failed for "+errorCountStr+" mesh(es).");
			if( IsFullVersion )
			{
				infoStr += " \n\n";
				infoStr += "Vector mode: Ensure layers in the PSD file have vector masks assigned.\n";
				infoStr += "Delaunay mode: Ensure the PSD file contains path objects corresponding to the layers.\n";
			}
			msgBox.setInformativeText(infoStr);
			msgBox.setStandardButtons(QMessageBox::Ok);
			msgBox.setDefaultButton(QMessageBox::Ok);
			QGridLayout* layout = (QGridLayout*)msgBox.layout();
			QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
			layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
			int ret = msgBox.exec();
		}
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::TimerStart()
	{
		this->pingTimer = new PingTimer(TimerPing,this);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginController::TimerPing(void* param)
	{
		PluginController* parent = (PluginController*)param;
		parent->GetScene().Ping();
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// Helper, progress bar handling during export
	ProgressAgent& PluginController::GetProgressExport() const
	{
		return context->GetToolWidget()->GetProgress();
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// Helper, progress bar handling during export
	void PluginController::BeginProgressExport(ActiveLayerFilter& filter, bool exportPNG, bool exportMesh, bool exportAll)
	{
		int taskCount = 1; // at least one task for mesh preparation (mesh controller cache coherency)
		ActiveLayerFilter activeFilter( GetPsdData().LayerMaskData, GetScene(), exportAll );
		int activeCount = activeFilter.Count(); // one task per layer mesh exported
		//ActiveNoAtlasLayerFilter activeNoAtlasFilter( GetPsdData().LayerMaskData, GetScene(), exportAll );
		//int activeNoAtlasCount = activeFilter.Count(); // one task per layer mesh exported
		//int atlasCount = GetScene().GetAtlasCount(); // one task per atlas exported
		if( exportPNG ) taskCount += (activeCount); // atlasCount + activeNoAtlasCount
		if( exportMesh ) taskCount += 2; // additional tasks mesh generation, and for finalizing (writing file)
		ProgressAgent& progressExport = context->GetToolWidget()->GetProgress();
#if defined PSDTO3D_FBX_VERSION
		progressExport.BeginProgressBar( "Exporting FBX...", taskCount, true ); // param cancelButton->true
#else
		progressExport.BeginProgressBar( "Exporting Mesh...", taskCount, true ); // param cancelButton->true
#endif
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// Helper, progress bar handling during export
	void PluginController::EndProgressExport()
	{
		ProgressAgent& progressExport = context->GetToolWidget()->GetProgress();
#if defined PSDTO3D_FBX_VERSION
		progressExport.EndProgressBar(true); // do keep progress dialog open after export, in standalone version
#else
		progressExport.EndProgressBar(false); // don't keep progress dialog open, by default
#endif

	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// Helper, progress bar handling during export
	void PluginController::UpdateProgressExport(QString const& taskName, bool nextTask, float taskValue) const
	{
		ProgressAgent& progressExport = context->GetToolWidget()->GetProgress();
		progressExport.SetLabel( taskName.toUtf8().data() );
		if( nextTask )
			progressExport.NextTask();
		else
			progressExport.GetTask().SetValueAndUpdate( taskValue );
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// Helper, progress bar handling during export
	void PluginController::SetValueProgressExport( void* param, float value )
	{
		PluginController* parent = (PluginController*)param;
		ProgressAgent& progressExport = parent->context->GetToolWidget()->GetProgress();
		progressExport.SetValue(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// From IPluginOutputHelper, string conversion for hosts not linked against Qt
	std::string PluginController::to_utf8( const QString& string )
	{
		return string.toUtf8().toStdString();
	}

} // namespace psd_to_3d

