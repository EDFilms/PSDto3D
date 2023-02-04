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

#include "interface/toolWidget.h"
#include "IControllerUpdate.h"
#include "psd_reader/psdReader.h"

namespace maya_plugin
{
	class PluginController : public IControllerUpdate
	{
	public:
		PluginController(ToolWidget* guiPsdMaya);
		~PluginController();

		void Update();
		void ExportTexture(MString const& path) const;
		void GenerateMesh(GlobalParameters & params);
		void ParsePsdData(MString const& path);

	private:
		
		ToolWidget* GuiPsdMaya; // Maya interface
		psd_reader::PsdData PsdData;
	};
}
#endif // PLUGINCONTROLLER_H
