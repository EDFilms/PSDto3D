//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file unrealEditorPluginCommands.h
//  @author Michaelson Britt
//  @date 04-01-2020
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#pragma once

#include "unrealEditorPluginFlags.h"
#include "unrealEditorPluginStyle.h"

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"



class PsdToUnrealEditorPluginCommands: public TCommands<PsdToUnrealEditorPluginCommands>
{
public:
	PsdToUnrealEditorPluginCommands()
		: TCommands<PsdToUnrealEditorPluginCommands>( TEXT("PSDtoUnreal"), NSLOCTEXT("Contexts", "PSDtoUnreal", "PSDto3D Plugin for Unreal Engine"), NAME_None, PsdToUnrealEditorPluginStyle::GetStyleSetName() )
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginAction;
};
