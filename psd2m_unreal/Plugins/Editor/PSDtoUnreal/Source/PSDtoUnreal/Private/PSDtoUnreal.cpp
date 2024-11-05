// ===============================================
//  Copyright (C) 2024, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PSDtoUnreal.cpp
//  @author Michaelson Britt
//  @date 2024-10-16
//
//  @section DESCRIPTION
//  Entry point module
//  Loads the PSDto3D core and handles Unreal user actions to launch the UI
//
//----------------------------------------------------------------------------------------------

#include "PSDtoUnreal.h"
#include "PSDtoUnrealStyle.h"
#include "PSDtoUnrealCommands.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "ToolMenus.h"

// Local includes
#include "PSDtoUnrealOutput.h"

#include <Windows.h> // for LoadLibrary() and GetProcAddress(); how to support this cross-platform?

//static const FName PSDtoUnrealTabName("PSDtoUnreal");

#define LOCTEXT_NAMESPACE "FPSDtoUnrealModule"


typedef void (*t_vfni)( int );
typedef void (*t_vfnvp)( void* );


void FPSDtoUnrealModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FPSDtoUnrealStyle::Initialize();
	FPSDtoUnrealStyle::ReloadTextures();

	FPSDtoUnrealCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FPSDtoUnrealCommands::Get().OpenEditorCommand,
		FExecuteAction::CreateRaw(this, &FPSDtoUnrealModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPSDtoUnrealModule::RegisterMenus));

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FPSDtoUnrealModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	FPlatformProcess::FreeDllHandle(PSDto3DLibraryHandle);
	PSDto3DLibraryHandle = nullptr;

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FPSDtoUnrealStyle::Shutdown();
	FPSDtoUnrealCommands::Unregister();
}

void FPSDtoUnrealModule::PluginButtonClicked()
{
#if PLATFORM_WINDOWS
	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("PSDtoUnreal")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath =
		FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/PSDto3DLibrary/Win64/PSDto3D_Standalone.dll"));
	FString LibraryPathAbs = FPaths::ConvertRelativePathToFull(LibraryPath); // LoadLibraryEx() requires and absolute path ...
	LibraryPathAbs.ReplaceCharInline('/', '\\'); // ..with backslashes, not forward slashes

	// Manually load the plugin with LoadLibraryEx(), specifying LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
	// FPlatformProcess::GetDllHandle() fails, doesn't include the DLL location as a search dir for Qt dependencies
	PSDto3DLibraryHandle = LoadLibraryEx(
		*LibraryPathAbs,
		nullptr,
		LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
		LOAD_LIBRARY_SEARCH_APPLICATION_DIR |
		LOAD_LIBRARY_SEARCH_SYSTEM32 );

	if (PSDto3DLibraryHandle)
	{
		PsdToUnrealPluginOutput& instance = PsdToUnrealPluginOutput::GetInstance();
		instance.OpenDialog( PSDto3DLibraryHandle );
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("PSDtoUnreal Error", "Failed to load PSDto3D Standalone library"));
	}

#else // PLATFORM_WINDOWS

	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
		LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
		FText::FromString(TEXT("FPSDtoUnrealModule::PluginButtonClicked()")),
		FText::FromString(TEXT("PSDtoUnreal.cpp"))
	);
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);

#endif  // PLATFORM_WINDOWS
}

void FPSDtoUnrealModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			{
				FToolMenuEntry& Entry = Section.AddMenuEntryWithCommandList(FPSDtoUnrealCommands::Get().OpenEditorCommand, PluginCommands);
				Entry.SetCommandList(PluginCommands);
			}
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FPSDtoUnrealCommands::Get().OpenEditorCommand));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPSDtoUnrealModule, PSDtoUnreal)