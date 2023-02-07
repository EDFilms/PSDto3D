//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file psdToUnrealCommands.h
//  @author Michaelson Britt
//  @date 04-01-2020
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#include "unrealEditorPluginCommands.h"

#define LOCTEXT_NAMESPACE "PSDtoUnrealModule"

//namespace unreal_plugin
//{

void PsdToUnrealEditorPluginCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "PSDtoUnreal", "Open Generator Editor", EUserInterfaceActionType::Button, FInputGesture());
}

//};