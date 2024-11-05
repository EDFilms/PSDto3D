// ===============================================
//  Copyright (C) 2024, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file PSDtoUnrealEditorModeToolkit.h
//  @author Michaelson Britt
//  @date 2024-10-16
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"
#include "PSDtoUnrealEditorMode.h"

/**
 * This FModeToolkit just creates a basic UI panel that allows various InteractiveTools to
 * be initialized, and a DetailsView used to show properties of the active Tool.
 */
class FPSDtoUnrealEditorModeToolkit : public FModeToolkit
{
public:
	FPSDtoUnrealEditorModeToolkit();

	/** FModeToolkit interface */
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode) override;
	virtual void GetToolPaletteNames(TArray<FName>& PaletteNames) const override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
};
