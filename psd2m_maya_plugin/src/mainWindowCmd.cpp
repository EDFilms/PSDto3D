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

// TODO: Need this, otherwise error linking method QByteArray QString::toUtf8() const & Q_REQUIRED_RESULT
//#define QT_COMPILING_QSTRING_COMPAT_CPP // added in project settings

#include "parameters.h" // versioning build settings like PSDTO3D_LICENSING_DISABLE
#include "mainWindowCmd.h"
#include "pluginController.h"
#include "interface/toolWidget.h"
#include "interface/toolWidgetLocalization.h"

#if defined PSDTO3D_MAYA_VERSION
#include <maya/MTypes.h>
#else
#include "mayaStub.h"
#include <iostream>
#include <thread>
using std::cout;
using std::cerr;
#endif

typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <QString.h>
#include <QMessageBox>
#if defined PSDTO3D_MAYA_VERSION
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
  #ifdef PSDTO3D_FULL_VERSION
  #pragma message("Building PSDtoMaya FULL VERSION ...")
  #else
  #pragma message("Building PSDtoMaya LITE VERSION ...")
  #endif
#endif
#if (defined PSDTO3D_UNREAL_VERSION) || (defined PSDTO3D_LICENSING_DISABLE)
// TODO: Why is this necessary to prevent linker error with windows MessageBox() function?
#pragma comment(lib, "user32.lib")
#endif

#include <windows.h> // for Sleep()

// threading variables
//volatile bool threadApp_running = false;
//volatile bool threadApp_done = false;


// Set output filename depending on Full or Lite version, and depending on Maya version
#if MAYA_API_VERSION >= 20220000
#define MAYA_VER_STR "2022"
#elif MAYA_API_VERSION >= 20200000
#define MAYA_VER_STR "2020"
#elif MAYA_API_VERSION >= 20190000
#define MAYA_VER_STR "2019"
#elif MAYA_API_VERSION >= 20180000
#define MAYA_VER_STR "2018"
#else MAYA_API_VERSION >= 20170000
#define MAYA_VER_STR "2017"
#endif
#ifdef PSDTO3D_FULL_VERSION
#pragma comment(linker, "/out:../../Builds/plugin/RelWithDebInfo_Maya" MAYA_VER_STR "_Full/PSDto3D_Maya" MAYA_VER_STR "_dev.mll")
#else
#pragma comment(linker, "/out:../../Builds/plugin/RelWithDebInfo_Maya" MAYA_VER_STR "_Lite/PSDto3D_Maya" MAYA_VER_STR "_dev.mll")
#endif

namespace psd_to_3d
{
	//-------------------- ------------------------------------------------------------------------------------------------------------------
	void PluginContext::InitSafe()
	{
		ConfParams.Fetch(); // read build configuration params

		if( this->Controller==nullptr )
		{
			this->Controller = new PluginController(this);
		}
		if( this->GuiLicensing==nullptr )
		{
#if (!defined PSDTO3D_UNREAL_VERSION) && (!defined PSDTO3D_LICENSING_DISABLE)
			QMainWindow* mainWindow = qobject_cast<QMainWindow*>(MQtUtil::mainWindow());
			this->GuiLicensing = new LicensingWidget( mainWindow );
#endif
		}
		if( this->GuiTool==nullptr )
		{
			QMainWindow* mainWindow = qobject_cast<QMainWindow*>(MQtUtil::mainWindow());
			this->GuiTool = new ToolWidget( mainWindow, this->Controller );
		}
	}

	//-------------------- ------------------------------------------------------------------------------------------------------------------
	void PluginContext::Free()
	{
		if( this->GuiLicensing!=nullptr )
		{
			//this->GuiLicensing->close(); // crashes on exit
			if( !this->GuiLicensing->isHidden() ) this->GuiLicensing->hide(); // TODO: need to call close()?
			delete this->GuiLicensing;
		}
		if( this->GuiTool!=nullptr )
		{
			//this->GuiTool->close(); // crashes on exit
			if( !this->GuiTool->isHidden() ) this->GuiTool->hide();
			delete this->GuiTool;
		}
		if( this->Controller!=nullptr )
		{
			delete this->Controller;
		}
		Zero();
	}

	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// When user clicks the Activate button in the licensing dialog
	void PluginContext::OnActivateLicense( LicensingParameters& data )
	{
		GuiLicensing->Store(); // write license data to preferences
		if( !LicenseCheck() )
		{
			QMessageBox messageBox;
			messageBox.critical(0,
				util::LocalizeString( IDC_MAIN, IDS_LICENSING_ERROR ), // use string table for popup messages
				util::LocalizeString( IDC_MAIN, IDS_LICENSING_ERROR_MSG )
			);

			messageBox.setFixedSize(500, 200);
		}
		else
			OpenGuiWindow(); // will perform another license check, oh well
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginContext::OpenGuiWindow()
	{
		InitSafe();
		if( LicenseCheck() )
		{
			if( this->GuiLicensing!=nullptr )
			{
				if( !IsLicensingHidden() ) this->GuiLicensing->hide();
			}
			if( this->GuiTool!=nullptr )
			{
				this->GuiTool->show();
			}
		}
		else if( IsLicensingHidden() ) {
			if( this->GuiLicensing!=nullptr )
			{
				this->GuiLicensing->ClearUI(); // clear user info
				this->GuiLicensing->SetCallback(this);
				this->GuiLicensing->show();
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool PluginContext::IsLicensingHidden()
	{
		return ((this->GuiLicensing==nullptr) || (this->GuiLicensing->isHidden()));
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool PluginContext::IsGuiHidden()
	{
		return ((this->GuiTool==nullptr) || (this->GuiTool->isHidden()));
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginContext::CloseGuiWindow()
	{
		Free(); // calls GuiTool->close();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginContext::RaiseGuiWindow()
	{
		this->GuiTool->raise();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool PluginContext::LicenseCheck()
	{
#if (defined PSDTO3D_LICENSING_DISABLE)
		return true;
#elif (defined PSDTO3D_UNREAL_VERSION)
		// Licensing enabled but in PSDtoUnreal?  Still disable licensing but with a message
		MessageBox( nullptr, "PSDtoUnreal is an unlicensed demo plugin developed by ED Films.  Enjoy!", "PSDtoUnreal", MB_OK);
		return true;
#else
		getInstance().GuiLicensing->Fetch(); // read license data from preferences
		LicensingParameters& params = getInstance().GuiLicensing->GetParameters();
		// IMPORTANT: Ensure TRIMMED WHITESPACE, LOWERCASE, and UTF-8 encoding, for correct hash
		int verify = ECDSA_Verify(
			params.UserInfoFirstName.trimmed().toLower().toUtf8().data(),
			params.UserInfoLastName.trimmed().toLower().toUtf8().data(),
			params.UserInfoEmail.trimmed().toLower().toUtf8().data(),
			params.LicenseKey.trimmed().toLower().toUtf8().data() );
		return (verify!=0);
#endif
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PluginContext::doIt()
	{
		if (IsGuiHidden())
		{			
			OpenGuiWindow();
			return;
		}

		RaiseGuiWindow();
	}
}

namespace maya_plugin
{
	using namespace psd_to_3d;

	// ====================================================================================================================================
#if defined PSDTO3D_MAYA_VERSION
	MayaPluginOutput MainWindowCmd::mayaPluginOutput;
#endif
	const MString MainWindowCmd::COMMAND_NAME("PSDto3D_Open");
	const MString MainWindowCmd::MENU_COMMAND_OPEN("Generator Editor");
	const MString MainWindowCmd::NAME_PLUGIN("PSDto3D");

	//--------------------------------------------------------------------------------------------------------------------------------------
	MString MainWindowCmd::MENU_NAME("PSDto3D");

	//--------------------------------------------------------------------------------------------------------------------------------------
	MStatus MainWindowCmd::doIt(const MArgList&)
	{
		PluginContext& context = PluginContext::getInstance();
#if defined PSDTO3D_MAYA_VERSION
		context.SetPluginOutput( &mayaPluginOutput );
#endif
		context.doIt();
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
		st = pluginFn.registerCommand(MainWindowCmd::COMMAND_NAME, &MainWindowCmd::CreateInstance);
#if MAYA_API_VERSION >= 20220000
		pluginFn.addMenuItem(MainWindowCmd::MENU_COMMAND_OPEN, MString("PSDto3D"), MainWindowCmd::MENU_NAME, MainWindowCmd::COMMAND_NAME, MString(""));
#else
		pluginFn.addMenuItem(MainWindowCmd::MENU_COMMAND_OPEN, MainWindowCmd::MENU_NAME, MainWindowCmd::COMMAND_NAME, MString(""));
#endif

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

//--------------------------------------------------------------------------------------------------------------------------------------
volatile QApplication* qtApp = nullptr;
void threadApp()
{
	int argc = 1;
	char arg1[] = "app";
	char *argv[] = {arg1};

	// Support 4k high-dpi monitors, also requires manifest setting,
	//  Configuration Properties->Manifest Tool->Input and Output->DPI Awarenes: High DPI Aware
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling); 

	// Create the app
	if( qtApp == nullptr )
	{
		qtApp = new QApplication(argc,argv);
//#if defined PSDTO3D_FBX_VERSION
//		qtApp->setQuitOnLastWindowClosed(true);
//#elif defined PSDTO3D_UNREAL_VERSION
//		qtApp->setQuitOnLastWindowClosed(false);
//#endif
	}

	// Start a plugin session and block
//	pluginContext = &(psd_to_3d::PluginContext::getInstance());
	psd_to_3d::PluginContext& pluginContext = psd_to_3d::PluginContext::getInstance();
	pluginContext.InitSafe();
	pluginContext.doIt(); // opens the gui window
//	threadApp_running = true;
	qtApp->exec(); // run UI handler; blocking call, returns when mainWindowCmd is closed

	// End the plugin session
//	context.Free();
//	threadApp_running = false;

	// TODO: Create separate exported function closePlugin() ...

	// Destroy the app (?)
	delete qtApp;
	qtApp = nullptr;
	//threadApp_done = true;
}

void openPlugin( int runLoop )
{
	//if( threadApp_running && !threadApp_done )
	//	return;

	//threadApp_running = threadApp_done = false;

	// Start the thread
	bool qtAppFirstRun = false;
	if( qtApp==nullptr )
	{
		qtAppFirstRun = true;
		std::thread pthreadApp( threadApp );
		pthreadApp.detach();
		// Wait until thread is started (and UI is visible)
		//while( (!threadApp_running) && (!threadApp_done) ) {
		//	std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//}
		while( qtApp==nullptr ) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}

	if( runLoop )
		 qtApp->setQuitOnLastWindowClosed(true);
	else qtApp->setQuitOnLastWindowClosed(false);

	psd_to_3d::PluginContext& pluginContext = psd_to_3d::PluginContext::getInstance();
	//pluginContext.InitSafe();
	if( !qtAppFirstRun )
		pluginContext.doIt(); // opens the gui window

	// Wait until thread is finished, if applicable
	// use this for standalone operation outside of any host app
	if( runLoop )
	{
		//while( !threadApp_done ) { Sleep(10); }
		while( qtApp!=nullptr ) { Sleep(10); }
	}
}

void setPluginOutput( void* output )
{
	psd_to_3d::PluginContext& context = psd_to_3d::PluginContext::getInstance();
	context.SetPluginOutput( (psd_to_3d::IPluginOutput*)output );
}