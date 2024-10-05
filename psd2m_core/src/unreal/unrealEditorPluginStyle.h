//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file unrealEditorPluginStyle.h
//  @author Michaelson Britt
//  @date 06-25-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#pragma once

#include "unrealEditorPluginFlags.h"

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class PsdToUnrealEditorPluginStyle
{
public:

	PsdToUnrealEditorPluginStyle();
	~PsdToUnrealEditorPluginStyle();

	static void Initialize();

	static void Shutdown();

	/** reloads textures used by slate renderer */
	static void ReloadTextures();

	/** @return The Slate style set for the Shooter game */
	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:

	static TSharedRef< class FSlateStyleSet > Create();

private:

	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};