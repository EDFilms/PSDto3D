//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file mainWindowCmd.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef mainWindowCmd_h
#define mainWindowCmd_h

#include <maya/MArgList.h>
#include <maya/MPxCommand.h>

#include "pluginController.h"
#include "interface/toolWidget.h"
#include "maya_mesh/meshGeneratorController.h"

namespace maya_plugin
{
	//--------------------------------------------------------------------------------------------------------------------------------------
	class Psd23DPlugin
	{
	public:

		static Psd23DPlugin& getInstance()
		{
			static Psd23DPlugin Instance;
			return Instance;
		}

		Psd23DPlugin(Psd23DPlugin const&) = delete;
		void operator=(Psd23DPlugin const&) = delete;

		void OpenGuiWindow();
		bool IsHidden();
		void CloseGuiWindow(); 
		void RaiseGuiWindow();

	private:

		Psd23DPlugin()
		{
			this->GuiPsdMaya = new ToolWidget();
			this->Plugin = new PluginController(this->GuiPsdMaya);
			this->GuiPsdMaya->AddController(this->Plugin);
		}

		~Psd23DPlugin()
		{
			this->GuiPsdMaya->close();
			delete this->Plugin;
			delete this->GuiPsdMaya;
		} 
		
		ToolWidget* GuiPsdMaya;
		PluginController* Plugin;
	};

	//--------------------------------------------------------------------------------------------------------------------------------------
	class MainWindowCmd: MPxCommand
	{
	public:

		static MString MENU_NAME;
		static const MString COMMAND_NAME;
		static const MString MENU_COMMAND_OPEN;
		static const MString NAME_PLUGIN;

		MainWindowCmd() = default;
		~MainWindowCmd() = default;

		static void* Creator() 
		{
			return new MainWindowCmd(); 
		}
		MStatus doIt(const MArgList& args) override;
	};
}
#endif
