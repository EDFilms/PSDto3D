//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file unrealEditorPlugin.cpp
//  @author Michaelson Britt
//  @date 06-25-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#include "unrealEditorPluginStyle.h"
#include "unrealEditorPlugin.h"

#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > PsdToUnrealEditorPluginStyle::StyleInstance = NULL;
static int instanceCount = 0;

PsdToUnrealEditorPluginStyle::PsdToUnrealEditorPluginStyle()
{
	instanceCount++;
}
PsdToUnrealEditorPluginStyle::~PsdToUnrealEditorPluginStyle() {}

void PsdToUnrealEditorPluginStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void PsdToUnrealEditorPluginStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName PsdToUnrealEditorPluginStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("PsdToUnrealEditorPluginStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > PsdToUnrealEditorPluginStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("PsdToUnrealEditorPluginStyle"));

	TSharedPtr<IPlugin> plugin = IPluginManager::Get().FindPlugin("PsdToUnreal");
	Style->SetContentRoot( plugin->GetBaseDir() / TEXT("Resources") );

	Style->Set("PSDtoUnreal.PluginAction", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));

	//Style->StyleSetName = GetStyleSetName();

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void PsdToUnrealEditorPluginStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& PsdToUnrealEditorPluginStyle::Get()
{
	return *StyleInstance;
}
