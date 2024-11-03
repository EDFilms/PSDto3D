// Copyright Epic Games, Inc. All Rights Reserved.

#include "PSDtoUnrealEditorModeToolkit.h"
#include "PSDtoUnrealEditorMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "PSDtoUnrealEditorModeToolkit"

FPSDtoUnrealEditorModeToolkit::FPSDtoUnrealEditorModeToolkit()
{
}

void FPSDtoUnrealEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FPSDtoUnrealEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}


FName FPSDtoUnrealEditorModeToolkit::GetToolkitFName() const
{
	return FName("PSDtoUnrealEditorMode");
}

FText FPSDtoUnrealEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "PSDtoUnrealEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
