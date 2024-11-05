// ===============================================
//  Copyright (C) 2024, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PSDtoUnrealEditorModeToolkit.cpp
//  @author Michaelson Britt
//  @date 2024-10-16
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

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
