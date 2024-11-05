// ===============================================
//  Copyright (C) 2024, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PSDtoUnrealCommands.h
//  @author Michaelson Britt
//  @date 2024-10-16
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "PSDtoUnrealStyle.h"

class FPSDtoUnrealCommands : public TCommands<FPSDtoUnrealCommands>
{
public:

	FPSDtoUnrealCommands();

	// TCommands<> interface
	virtual void RegisterCommands() override;
	static TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetCommands();

	TSharedPtr<FUICommandInfo> OpenEditorCommand;
	
	TSharedPtr<FUICommandInfo> SimpleTool;
	TSharedPtr<FUICommandInfo> InteractiveTool;

public:
	TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> Commands; // TODO: Same as above

};
