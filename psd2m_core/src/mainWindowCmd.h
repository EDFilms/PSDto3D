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

#if defined PSDTO3D_MAYA_VERSION
#include "maya_mesh/mayaMeshConvertor.h"
#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#else
#include "mayaStub.h"
#endif

#include "IControllerUpdate.h" // rename to IPluginController.h
#include "IPluginOutput.h"
#include "interface/licensingWidget.h"

namespace psd_to_3d
{
	class ToolWidget; // forward declaration
	class QEventRouter; // forward declaration

	//--------------------------------------------------------------------------------------------------------------------------------------
	class PluginContext : public LicensingWidget::LicensingCallback
	{
	public:

		static PluginContext& getInstance()
		{
			static PluginContext Instance;
			return Instance;
		}

		PluginContext(PluginContext const&) = delete;
		void operator=(PluginContext const&) = delete;
		const ConfParameters* GetConfParams() const { return &ConfParams; }
		const ToolWidget* GetToolWidget() const { return GuiTool; }
		ToolWidget* GetToolWidget() { return GuiTool; }
		const IPluginController* GetPluginController() const { return Controller; }
		IPluginController* GetPluginController() { return Controller; }
		// TODO: Add IPluginInput, which takes a path and returns a PsdData
		IPluginOutput* GetPluginOutput() { return Output; }
		void SetPluginOutput(IPluginOutput* output) { this->Output = output; }

		void InitSafe(); // safe to call repeatedly
		void Free();
        void OnActivateLicense( LicensingParameters& data ); // from LicensingWidget::LicensingCallback
		void OpenGuiWindow();
		bool IsLicensingHidden();
		bool IsGuiHidden();
		void CloseGuiWindow(); 
		void RaiseGuiWindow();

		// Run the plugin
		void doIt();

	private:

		PluginContext()
		{
			Zero();
			// Members instantiated in InitSafe()
		}

		~PluginContext()
		{
			Free();
		}

		void Zero()
		{
			this->GuiLicensing = nullptr;
			this->GuiTool = nullptr;
			this->Controller = nullptr;
			this->Output = nullptr;
			this->EventRouter = nullptr;
		}

        bool LicenseCheck();
		
		ConfParameters ConfParams;
		LicensingWidget* GuiLicensing;
		ToolWidget* GuiTool;
		IPluginController* Controller;
		IPluginOutput* Output;
		QEventRouter* EventRouter;
	};

}

namespace maya_plugin
{
	using namespace psd_to_3d;

	//--------------------------------------------------------------------------------------------------------------------------------------
	class MainWindowCmd: MPxCommand
	{
	public:

#if defined PSDTO3D_MAYA_VERSION
		static MayaPluginOutput mayaPluginOutput;
#endif
		static MString MENU_NAME;
		static const MString COMMAND_NAME;
		static const MString MENU_COMMAND_OPEN;
		static const MString NAME_PLUGIN;

		MainWindowCmd() = default;
		~MainWindowCmd() = default;

		static void* CreateInstance() 
		{
			// NOTE: This must NOT be a singleton, it will be deleted by Maya
			return new MainWindowCmd();
		}

		// Run the plugin
		MStatus doIt(const MArgList& args) override;
	};
}
#endif
