//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file mainWindowCmd.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include <mainWindowCmd.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

namespace maya_plugin
{

	//--------------------------------------------------------------------------------------------------------------------------------------
	void Psd23DPlugin::OpenGuiWindow()
	{
		getInstance().GuiPsdMaya->show();
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	bool Psd23DPlugin::IsHidden()
	{
		return getInstance().GuiPsdMaya->isHidden();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void Psd23DPlugin::CloseGuiWindow()
	{
		getInstance().GuiPsdMaya->close();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void Psd23DPlugin::RaiseGuiWindow()
	{
		getInstance().GuiPsdMaya->raise();
	}


	// ====================================================================================================================================
	const MString MainWindowCmd::COMMAND_NAME("Open");
	const MString MainWindowCmd::MENU_COMMAND_OPEN("Generator Editor");
	const MString MainWindowCmd::NAME_PLUGIN("PSD23D");

	//--------------------------------------------------------------------------------------------------------------------------------------
	MString MainWindowCmd::MENU_NAME("PSD23D");

	//--------------------------------------------------------------------------------------------------------------------------------------
	MStatus MainWindowCmd::doIt(const MArgList&)
	{
		Psd23DPlugin& plugin = Psd23DPlugin::getInstance();
		if (plugin.IsHidden())
		{			
			plugin.OpenGuiWindow();
			return MS::kSuccess;
		}

		plugin.RaiseGuiWindow();
		return MS::kSuccess;
	}

	// ====================================================================================================================================
	//
	//	Plugin load/unload
	//
	// ====================================================================================================================================
	MStatus initializePlugin(MObject plugin)
	{
		cout.rdbuf(cerr.rdbuf());
		MStatus		st;
		MFnPlugin pluginFn(plugin, "E->D Films", "1.0.0", "2017", &st);

		if (!st) {
			MGlobal::displayError(
				MString("mainWindowsCmd - could not initialize plugin: ")
				+ st.errorString()
			);
			return st;
		}

		//	Register the command used to open the menu.
		MainWindowCmd::MENU_NAME = MGlobal::executeCommandStringResult("menu -parent MayaWindow -to true -label " + MainWindowCmd::NAME_PLUGIN);
		st = pluginFn.registerCommand(MainWindowCmd::COMMAND_NAME, &MainWindowCmd::Creator);
		pluginFn.addMenuItem(MainWindowCmd::MENU_COMMAND_OPEN, MainWindowCmd::MENU_NAME, MainWindowCmd::COMMAND_NAME, "");

		if (!st) {
			MGlobal::displayError(
				MString("mainWindowCmd - could not register '")
				+ MainWindowCmd::COMMAND_NAME + "' command: "
				+ st.errorString()
			);
			return st;
		}
		return st;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	MStatus uninitializePlugin(MObject plugin)
	{
		MStatus		st;
		MFnPlugin	pluginFn(plugin, "E->D Films", "1.0.0", "2017", &st);

		if (!st) {
			MGlobal::displayError(
				MString("mainWindowCmd - could not uninitialize plugin: ")
				+ st.errorString()
			);
			return st;
		}

		//pluginFn.removeMenuItem(mainWindowCmd::menuItem);
		MGlobal::executeCommandStringResult("deleteUI -m " + MainWindowCmd::MENU_NAME);
		//	Deregister the command.
		st = pluginFn.deregisterCommand(MainWindowCmd::COMMAND_NAME);

		if (!st) {
			MGlobal::displayError(
				MString("mainWindowCmd - could not deregister '")
				+ MainWindowCmd::COMMAND_NAME + "' command: "
				+ st.errorString()
			);
			return st;
		}
		return st;
	}
}
