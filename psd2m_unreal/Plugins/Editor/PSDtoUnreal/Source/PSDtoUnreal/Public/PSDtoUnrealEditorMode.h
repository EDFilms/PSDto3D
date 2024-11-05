// ===============================================
//  Copyright (C) 2024, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PSDtoUnrealEditorMode.h
//  @author Michaelson Britt
//  @date 2024-10-16
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Tools/UEdMode.h"
#include "PSDtoUnrealEditorMode.generated.h"

/**
 * This class provides an example of how to extend a UEdMode to add some simple tools
 * using the InteractiveTools framework. The various UEdMode input event handlers (see UEdMode.h)
 * forward events to a UEdModeInteractiveToolsContext instance, which
 * has all the logic for interacting with the InputRouter, ToolManager, etc.
 * The functions provided here are the minimum to get started inserting some custom behavior.
 * Take a look at the UEdMode markup for more extensibility options.
 */
UCLASS()
class UPSDtoUnrealEditorMode : public UEdMode
{
	GENERATED_BODY()

public:
	const static FEditorModeID EM_PSDtoUnrealEditorModeId;

	static FString SimpleToolName;
	static FString InteractiveToolName;

	UPSDtoUnrealEditorMode();
	virtual ~UPSDtoUnrealEditorMode();

	/** UEdMode interface */
	virtual void Enter() override;
	virtual void ActorSelectionChangeNotify() override;
	virtual void CreateToolkit() override;
	virtual TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> GetModeCommands() const override;
};
