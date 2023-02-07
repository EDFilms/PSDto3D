//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file ui_wrapper.h
//  @author Michaelson Britt
//  @date 06-07-2019
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef UI_WRAPPER_H
#define UI_WRAPPER_H

//#include "ui_licensingWidget.h"

// TODO: These must be configured by installer somehow
const int IsLocalizationEnglish = 1;
const int IsLocalizationFrench = 0;


#ifdef PSDTO3D_ATLAS_VERSION
const int IsAtlasVersion = 1;
#else
const int IsAtlasVersion = 0;
#endif


#ifdef PSDTO3D_FULL_VERSION
//#include "ui_toolWidget_full.h"
const int IsFullVersion = 1;
const int IsVectorModeSupported = 1;
#else
//#include "ui_toolWidget_lite.h"
const int IsFullVersion = 0;
const int IsVectorModeSupported = 0; // Lite version does not support Vector Mode
#endif


#if defined PSDTO3D_FBX_VERSION
const int IsFbxVersion = 1;
const int IsMayaVersion = 0;
const int IsUnrealVersion = 0;
const int IsLinearModeSupported = 0; // PSDtoFBX version does not support older Linear Mode, only newer Delaunay Mode
#elif defined PSDTO3D_UNREAL_VERSION
const int IsFbxVersion = 0;
const int IsMayaVersion = 0;
const int IsUnrealVersion = 1;
const int IsLinearModeSupported = 0; // PSDtoUnreal version does not support older Linear Mode, only newer Delaunay Mode
#elif defined PSDTO3D_MAYA_VERSION
const int IsFbxVersion = 0;
const int IsMayaVersion = 1;
const int IsUnrealVersion = 0;
const int IsLinearModeSupported = 1;
#endif

#ifdef PSDTO3D_FULL_VERSION
const int IsDelaunayModeSupported = 1;
#else
const int IsDelaunayModeSupported = 0;
#endif

const int IsInfluenceSupported = 0; // current disabled in all versions

#endif // UI_WRAPPER_H
