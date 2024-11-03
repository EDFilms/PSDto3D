// Copyright Epic Games, Inc. All Rights Reserved.

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
