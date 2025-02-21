//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file pluginController.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef PLUGINCONTROLLER_H
#define PLUGINCONTROLLER_H

#include "IControllerUpdate.h"
#include "IPluginOutput.h"
#include "util/progressJob.h"

class QString; // forward declaration

namespace psd_reader
{
	struct PsdData; // forward declaration
}

namespace psd_to_3d
{
	class IPluginOutput; // forward declaration
	class ProgressAgent; // forward declaration
	class SceneController; // forward declaration
	class ActiveLayerFilter;
	class PluginContext; // forward declaration
	class PingTimer; // forward declaration
	using psd_reader::PsdData; // forward declaration
	using util::ProgressJob;

	class PluginController : public IPluginController
	{
	public:
		PluginController(PluginContext* context);
		~PluginController();

		// Local methods
		const PluginContext& GetContext() const;
		PluginContext& GetContext();
		const PsdData& GetPsdData() const;
		PsdData& GetPsdData();
		const SceneController& GetScene() const;

		// From IPluginController
		IPluginOutput& GetOutput();
		SceneController& GetScene();
		NotifyStatus& GetNotifyStatus();
		void NotifyCommand();

	protected:

		void ExportTexture(ActiveLayerFilter& filter, QString const& path) const;
		void ExportAtlas(ActiveLayerFilter& filter, QString const& path) const;
		void GenerateMesh(ActiveLayerFilter& filter) const;
		void ParsePsdData(QString const& path);
		void Repaint(bool updateLayout);

	private:
		void TimerStart();
		static void TimerPing(void* param);

		// helper methods, progress bar handling for export operations
		ProgressAgent& GetProgressExport() const;
		void BeginProgressExport(ActiveLayerFilter& filter, bool exportPNG, bool exportMesh, bool exportAll);
		void EndProgressExport();
		void UpdateProgressExport(QString const& taskName, bool nextTask=false) const;
		static void SetValueProgressExport( void* param, float value );

		// from IPluginOutputHelper
		std::string to_utf8( const QString& string );

		PluginContext* context;
		PingTimer* pingTimer;
		PsdData* psdData;
		SceneController* scene;
		NotifyStatus notifyStatus;
	};
}

#endif // PLUGINCONTROLLER_H
