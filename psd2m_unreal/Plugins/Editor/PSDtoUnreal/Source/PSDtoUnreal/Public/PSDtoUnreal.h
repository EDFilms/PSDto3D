// ===============================================
//  Copyright (C) 2024, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PSDtoUnreal.h
//  @author Michaelson Britt
//  @date 2024-10-16
//
//  @section DESCRIPTION
//  Entry point module
//  Loads the PSDto3D core and handles Unreal user actions to launch the UI
//
//----------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FPSDtoUnrealModule : public IModuleInterface
{
public:

	// IModuleInterface implementation
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	// This function will be bound to Command
	void PluginButtonClicked();
	
private:

	void RegisterMenus();

	// Handle to the test dll we will load
	void*	PSDto3DLibraryHandle;
	TSharedPtr<class FUICommandList> PluginCommands;
};
