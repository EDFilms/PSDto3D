//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file FbxCommon.h
//  @author Michaelson Britt
//  @date 04-01-2020
//
//  @section DESCRIPTION
//  Based on sample project ExportScene03 from the Autodesk FBX SDK 2020
//
//----------------------------------------------------------------------------------------------

#ifndef _COMMON_H
#define _COMMON_H

#include <fbxsdk.h>

extern int kFbxWriterFormatBinary;
extern int kFbxWriterFormatAscii;

void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
void DestroySdkObjects(FbxManager* pManager, bool pExitStatus);

bool SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, int pWriterFormat=-1, bool pEmbedMedia=false);
bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);

#endif // #ifndef _COMMON_H


