//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file unrealEditorPlugin.cpp
//  @author Michaelson Britt
//  @date 06-25-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#include "unrealEditorPlugin.h"
#include "unrealEditorPluginStyle.h"
#include "unrealEditorPluginCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName Test_UnrealEngineEditorPluginTabName("PSDtoUnreal");

#define LOCTEXT_NAMESPACE "PsdToUnrealEditorPluginModule"

void PsdToUnrealEditorPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	PsdToUnrealEditorPluginStyle::Initialize();
	PsdToUnrealEditorPluginStyle::ReloadTextures();

	PsdToUnrealEditorPluginCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		PsdToUnrealEditorPluginCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &PsdToUnrealEditorPluginModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &PsdToUnrealEditorPluginModule::RegisterMenus));
}

void PsdToUnrealEditorPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	PsdToUnrealEditorPluginStyle::Shutdown();

	PsdToUnrealEditorPluginCommands::Unregister();
}

void OnPluginButtonClicked();
void PsdToUnrealEditorPluginModule::PluginButtonClicked()
{
	OnPluginButtonClicked();
}

void PsdToUnrealEditorPluginModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(PsdToUnrealEditorPluginCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(PsdToUnrealEditorPluginCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(PsdToUnrealEditorPluginModule, PSDtoUnreal)