//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file IPluginController.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef IPLUGINCONTROLLER_H
#define IPLUGINCONTROLLER_H

#include <string>

namespace psd_to_3d
{
	class IPluginOutput; // forward declaration
	class SceneController; // forward declaration


	class IPluginController
	{
	public:
		struct NotifyStatus
		{
			// State modification
			bool ExportMesh = false; // export meshes button was pressed
			bool ExportPng = false;  // export pngs button was pressed
			bool ExportAll = false;	 // only both selected and nonselected layers, when exporting
			bool ParametersChanged = false; // request to refresh UI, parameters and layer descriptions
			bool DescriptionsChanged = false; // request to refresh UI, layer descriptions only
			bool FileImportRequest = false; // request to load new file
			std::string FileImportFilepath;
			void Reset() {(*this)=NotifyStatus();} // reset flags to default
		};

		virtual IPluginOutput& GetOutput() = 0;
		virtual SceneController& GetScene() = 0;
		virtual NotifyStatus& GetNotifyStatus() = 0; // flags for NotifyCommand()

		// generic notify for any button press or command, refers to NotifyStatus
		virtual void NotifyCommand() = 0;

		virtual ~IPluginController() = default;
	};
}
#endif // IPLUGINCONTROLLER_H
