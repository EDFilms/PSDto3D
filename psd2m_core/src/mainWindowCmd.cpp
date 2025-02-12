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
#include <QScreen>
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

// Helper, Qt search path for plugins
// Uses the current DLL location as the base directory for /imageformats and /platforms
void SetQtPluginPath( )
{
	static int dummy; // address of this static variable identifies the current DLL
	HMODULE hModule;
	if( GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR) &dummy, &hModule) )
	{
		// retrieve absolute filename of the DLL and split out the path
		char path[MAX_PATH], drive[MAX_PATH], dir[MAX_PATH];
		GetModuleFileName(hModule, path, MAX_PATH);
		_splitpath_s( path, drive, MAX_PATH, dir, MAX_PATH, nullptr, 0, nullptr, 0 );
		sprintf_s( path, MAX_PATH, "%s%s", drive, dir );
		// inform Qt of the base directory
		QCoreApplication::addLibraryPath(path);
	}
}

// threading variables
//volatile bool threadApp_running = false;
//volatile bool threadApp_done = false;

namespace psd_to_3d
{
	//-------------------- ------------------------------------------------------------------------------------------------------------------
	// Helper, Qt event forwarding from an application thread to the Qt message thread
	// Needed for threadsafety; all interaction with Qt objects must happen in its message thread
	static const QEvent::Type EVENT_MAIN_WINDOW_SHOW = (QEvent::Type)(QEvent::User + 0); // request to open the main window
	class QEventRouter : public QObject
	{
	public:
		QEventRouter( QMainWindow* mainWindow ) : mainWindow(mainWindow) { moveToThread( mainWindow->thread() ); }
		// Post a request to show the main window; will be safely handled in the Qt message thread
		void ShowMainWindow() {	QCoreApplication::postEvent( this, new QEvent( EVENT_MAIN_WINDOW_SHOW ) ); }
	protected:
		bool event( QEvent *ev )
		{   
			// Handle request to show the main window
			if( ev->type() == EVENT_MAIN_WINDOW_SHOW )
			{
				mainWindow->show();
				return true;
			}
			return false;
		}
		QMainWindow* mainWindow;
	};

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
			// Set initial screen position when created
			QSize screenGeometry = QGuiApplication::primaryScreen()->size();
			int x = (screenGeometry.width()-this->GuiTool->width()) / 2;
			int y = (screenGeometry.height()-this->GuiTool->height()) / 2;
			this->GuiTool->move(x, y);
			//int primaryScreen = QApplication::desktop()->primaryScreen();
			//QRect screenGeometry = QApplication::desktop()->screenGeometry(primaryScreen);
		}
		if( this->EventRouter==nullptr )
		{
			EventRouter = new QEventRouter( this->GuiTool );
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
		if( this->EventRouter!=nullptr )
		{
			delete this->EventRouter;
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
				// Show the main window; posts a request to open the window in a threadsafe manner
				EventRouter->ShowMainWindow();
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
#if QT_VERSION < 0x060000
	// Qt 5.x only.  In Qt 6.x, High-DPI scaling is always enabled
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling); 
#endif

	// Create the app
	if( qtApp == nullptr )
	{
		SetQtPluginPath();
		qtApp = new QApplication(argc,argv);
	}

	// Start a plugin session and block
	psd_to_3d::PluginContext& pluginContext = psd_to_3d::PluginContext::getInstance();
	pluginContext.InitSafe();
	pluginContext.doIt(); // opens the gui window
	qtApp->exec(); // run UI handler; blocking call, returns when mainWindowCmd is closed
	// TODO: Create separate exported function closePlugin() ...

	// Destroy the app (?)
	delete qtApp;
	qtApp = nullptr;
}

// DLL EXPORTED FUNCTION, See linker options
void openPlugin( int runLoop )
{
	// Start the thread
	bool qtAppFirstRun = false;
	if( qtApp==nullptr )
	{
		qtAppFirstRun = true;
		std::thread pthreadApp( threadApp );
		pthreadApp.detach();
		// Wait until thread is started (and UI is visible)
		while( qtApp==nullptr ) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}

	if( runLoop )
		 qtApp->setQuitOnLastWindowClosed(true);
	else qtApp->setQuitOnLastWindowClosed(false);

	psd_to_3d::PluginContext& pluginContext = psd_to_3d::PluginContext::getInstance();
	if( !qtAppFirstRun ) // pthreadApp above already calls this on first run
		pluginContext.doIt(); // opens the gui window, only needed on subsequent runs

	// Wait until thread is finished, if applicable
	// use this for standalone operation outside of any host app
	if( runLoop )
	{
		//while( !threadApp_done ) { Sleep(10); }
		while( qtApp!=nullptr ) { Sleep(10); }
	}
}

// DLL EXPORTED FUNCTION, See linker options
void setPluginOutput( void* output )
{
	psd_to_3d::PluginContext& context = psd_to_3d::PluginContext::getInstance();
	context.SetPluginOutput( (psd_to_3d::IPluginOutput*)output );
}

// DLL EXPORTED FUNCTION, See linker options
// Implemented in Blender module
//
// typedef int (*CallbackFn)( PythonDataMesh* pythonDataMesh );
// void pythonSetCallback( CallbackFn callback );
