// ===============================================
//  Copyright (C) 2024, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PSDtoUnrealCommands.cpp
//  @author Michaelson Britt
//  @date 2024-10-16
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "PSDtoUnrealCommands.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "FPSDtoUnrealModule"

FPSDtoUnrealCommands::FPSDtoUnrealCommands()
	: TCommands<FPSDtoUnrealCommands>(TEXT("PSDtoUnreal"),
		NSLOCTEXT("PSDtoUnreal", "PSDtoUnreal", "PSDtoUnreal Plugin"),
		NAME_None,
		//FEditorStyle::GetStyleSetName()  //deprecated
		FAppStyle::GetAppStyleSetName()
		)
{
}

void FPSDtoUnrealCommands::RegisterCommands()
{
	TArray <TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	UI_COMMAND(OpenEditorCommand, "PSDtoUnreal", "Open Generator Editor", EUserInterfaceActionType::Button, FInputGesture());
	ToolCommands.Add(OpenEditorCommand);

	UI_COMMAND(SimpleTool, "Show Actor Info", "Opens message box with info about a clicked actor", EUserInterfaceActionType::Button, FInputChord());
	ToolCommands.Add(SimpleTool);

	UI_COMMAND(InteractiveTool, "Measure Distance", "Measures distance between 2 points (click to set origin, shift-click to set end point)", EUserInterfaceActionType::ToggleButton, FInputChord());
	ToolCommands.Add(InteractiveTool);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FPSDtoUnrealCommands::GetCommands()
{
	return FPSDtoUnrealCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
