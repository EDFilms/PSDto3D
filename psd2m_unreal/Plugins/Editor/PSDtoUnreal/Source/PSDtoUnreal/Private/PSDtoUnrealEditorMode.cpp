// Copyright Epic Games, Inc. All Rights Reserved.

#include "PSDtoUnrealEditorMode.h"
#include "PSDtoUnrealEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "PSDtoUnrealCommands.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/PSDtoUnrealSimpleTool.h"
#include "Tools/PSDtoUnrealInteractiveTool.h"

// step 2: register a ToolBuilder in FPSDtoUnrealEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "PSDtoUnrealEditorMode"

const FEditorModeID UPSDtoUnrealEditorMode::EM_PSDtoUnrealEditorModeId = TEXT("EM_PSDtoUnrealEditorMode");

FString UPSDtoUnrealEditorMode::SimpleToolName = TEXT("PSDtoUnreal_ActorInfoTool");
FString UPSDtoUnrealEditorMode::InteractiveToolName = TEXT("PSDtoUnreal_MeasureDistanceTool");


UPSDtoUnrealEditorMode::UPSDtoUnrealEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UPSDtoUnrealEditorMode::EM_PSDtoUnrealEditorModeId,
		LOCTEXT("ModeName", "PSDtoUnreal"),
		FSlateIcon(),
		true);
}


UPSDtoUnrealEditorMode::~UPSDtoUnrealEditorMode()
{
}


void UPSDtoUnrealEditorMode::ActorSelectionChangeNotify()
{
}

void UPSDtoUnrealEditorMode::Enter()
{
	UEdMode::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FPSDtoUnrealCommands& commands = FPSDtoUnrealCommands::Get();

	RegisterTool(commands.SimpleTool, SimpleToolName, NewObject<UPSDtoUnrealSimpleToolBuilder>(this));
	RegisterTool(commands.InteractiveTool, InteractiveToolName, NewObject<UPSDtoUnrealInteractiveToolBuilder>(this));

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, SimpleToolName);
}

void UPSDtoUnrealEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FPSDtoUnrealEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UPSDtoUnrealEditorMode::GetModeCommands() const
{
	return FPSDtoUnrealCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
