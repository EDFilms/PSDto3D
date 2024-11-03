// Copyright Epic Games, Inc. All Rights Reserved.

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
