//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file FbxCommon.cpp
//  @author Michaelson Britt
//  @date 04-01-2020
//
//  @section DESCRIPTION
//  Based on sample project ExportScene03 from the Autodesk FBX SDK 2020
//
//----------------------------------------------------------------------------------------------

#include "FbxCommon.h"
#ifdef IOS_REF
	#undef  IOS_REF
	#define IOS_REF (*(fbxManager->GetIOSettings()))
#endif

int kFbxWriterFormatBinary;
int kFbxWriterFormatAscii;


void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
    // The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if( !pManager )
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        exit(1);
    }
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

	// Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	// Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

    int fbxFormatIndex, fbxFormatCount = pManager->GetIOPluginRegistry()->GetWriterFormatCount();

    // Find the writer format indices, ASCII and Binary
	// Look for desired writer formats, sets to -1 if not found
	kFbxWriterFormatBinary = pManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX binary (*.fbx)");
	kFbxWriterFormatAscii = pManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX ascii (*.fbx)");

	// Look for alternate writer formats, if desired format not found
    for( fbxFormatIndex=0; fbxFormatIndex<fbxFormatCount; fbxFormatIndex++ )
    {
        if( pManager->GetIOPluginRegistry()->WriterIsFBX(fbxFormatIndex) )
        {
            FbxString lDesc = pManager->GetIOPluginRegistry()->GetWriterFormatDescription(fbxFormatIndex);
			if( kFbxWriterFormatBinary < 0 )
			{
				const char *lBINARY = "binary";
				if( lDesc.Find(lBINARY)>=0 )
				{
					kFbxWriterFormatBinary = fbxFormatIndex;
				}
			}
			if( kFbxWriterFormatAscii < 0 )
			{
				const char *lASCII = "ascii";
				if (lDesc.Find(lASCII)>=0)
				{
					kFbxWriterFormatAscii = fbxFormatIndex;
				}
			}
        }
    } 

	// Use baseline writer formats, if other formats not found
	if( kFbxWriterFormatBinary < 0 )
	{
		kFbxWriterFormatBinary = pManager->GetIOPluginRegistry()->GetNativeWriterFormat();
	}
	if( kFbxWriterFormatAscii < 0 )
	{
		kFbxWriterFormatAscii = pManager->GetIOPluginRegistry()->GetNativeWriterFormat();
	}

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    pScene = FbxScene::Create(pManager, "My Scene");
	if( !pScene )
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        exit(1);
    }
}

void DestroySdkObjects(FbxManager* fbxManager, bool fbxExitStatus)
{
    //Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
    if( fbxManager ) fbxManager->Destroy();
	if( fbxExitStatus ) FBXSDK_printf("Program Success!\n");
}

bool SaveScene(FbxManager* fbxManager, FbxDocument* fbxScene, const char* outputFileName, int outputFileFormat, bool pEmbedMedia)
{
    int fbxMajor, fbxMinor, fbxRevision;
    bool fbxStatus = true;

    // Create an exporter.
    FbxExporter* fbxExporter = FbxExporter::Create(fbxManager, "");

    // Set the export states. By default, the export states are always set to 
    // true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
    // shows how to change these states.
    IOS_REF.SetBoolProp(EXP_FBX_MATERIAL,        true);
    IOS_REF.SetBoolProp(EXP_FBX_TEXTURE,         true);
    IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED,        pEmbedMedia);
    IOS_REF.SetBoolProp(EXP_FBX_SHAPE,           true);
    IOS_REF.SetBoolProp(EXP_FBX_GOBO,            true);
    IOS_REF.SetBoolProp(EXP_FBX_ANIMATION,       true);
    IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

    // Initialize the exporter by providing a filename.
    if( fbxExporter->Initialize(outputFileName, outputFileFormat, fbxManager->GetIOSettings()) == false )
    {
        FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: %s\n\n", fbxExporter->GetStatus().GetErrorString());
        return false;
    }

	// TODO: Delete this, testing only
	//int lFormat = fbxManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX 7.4 binary (*.fbx)");
	//char const* const* versionIO = fbxManager->GetIOPluginRegistry()->GetWritableVersions(outputFileFormat);
	//char const* const* versionsCur = fbxExporter->GetCurrentWritableVersions();

	fbxExporter->SetFileExportVersion( FbxString("FBX201400"), FbxSceneRenamer::eNone ); // Use FBX 2014 by default

    FbxManager::GetFileFormatVersion( fbxMajor, fbxMinor, fbxRevision );
    FBXSDK_printf("FBX file format version %d.%d.%d\n\n", fbxMajor, fbxMinor, fbxRevision);

    // Export the scene.
    fbxStatus = fbxExporter->Export( fbxScene );
    if( fbxStatus==false )
    {
        fbxStatus = fbxExporter->GetStatus();
    }

    // Destroy the exporter.
    fbxExporter->Destroy();
    return fbxStatus;
}

bool LoadScene(FbxManager* fbxManager, FbxDocument* fbxScene, const char* outputFileName)
{
    int fbxFileMajor, fbxFileMinor, fbxFileRevision;
    int fbxSDKMajor,  fbxSDKMinor,  fbxSDKRevision;
    //int fbxFileFormat = -1;
    int fbxAnimStackCount;
    bool fbxStatus;
    char fbxPassword[1024];

    // Get the file version number generate by the FBX SDK.
    FbxManager::GetFileFormatVersion( fbxSDKMajor, fbxSDKMinor, fbxSDKRevision );

    // Create an importer.
    FbxImporter* fbxImporter = FbxImporter::Create(fbxManager,"");

    // Initialize the importer by providing a filename.
    const bool fbxImportStatus = fbxImporter->Initialize(outputFileName, -1, fbxManager->GetIOSettings());
    fbxImporter->GetFileVersion( fbxFileMajor, fbxFileMinor, fbxFileRevision );

    if( !fbxImportStatus )
    {
        FbxString error = fbxImporter->GetStatus().GetErrorString();
        FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
        FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

        if( fbxImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion )
        {
            FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", fbxSDKMajor, fbxSDKMinor, fbxSDKRevision);
            FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", outputFileName, fbxFileMajor, fbxFileMinor, fbxFileRevision);
        }

        return false;
    }

    FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", fbxSDKMajor, fbxSDKMinor, fbxSDKRevision);

    if( fbxImporter->IsFBX() )
    {
        FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", outputFileName, fbxFileMajor, fbxFileMinor, fbxFileRevision);

        // From this point, it is possible to access animation stack information without
        // the expense of loading the entire file.

        FBXSDK_printf("Animation Stack Information\n");

        fbxAnimStackCount = fbxImporter->GetAnimStackCount();

        FBXSDK_printf("    Number of Animation Stacks: %d\n", fbxAnimStackCount);
        FBXSDK_printf("    Current Animation Stack: \"%s\"\n", fbxImporter->GetActiveAnimStackName().Buffer());
        FBXSDK_printf("\n");

        for(int i = 0; i < fbxAnimStackCount; i++)
        {
            FbxTakeInfo* fbxTakeInfo = fbxImporter->GetTakeInfo(i);

            FBXSDK_printf("    Animation Stack %d\n", i);
            FBXSDK_printf("         Name: \"%s\"\n", fbxTakeInfo->mName.Buffer());
            FBXSDK_printf("         Description: \"%s\"\n", fbxTakeInfo->mDescription.Buffer());

            // Change the value of the import name if the animation stack should be imported 
            // under a different name.
            FBXSDK_printf("         Import Name: \"%s\"\n", fbxTakeInfo->mImportName.Buffer());

            // Set the value of the import state to false if the animation stack should be not
            // be imported. 
            FBXSDK_printf("         Import State: %s\n", fbxTakeInfo->mSelect ? "true" : "false");
            FBXSDK_printf("\n");
        }

        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        true);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         true);
        IOS_REF.SetBoolProp(IMP_FBX_LINK,            true);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           true);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO,            true);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION,       true);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }

    // Import the scene.
    fbxStatus = fbxImporter->Import(fbxScene);
	if( fbxStatus == true )
	{
		// Check the scene integrity!
		FbxStatus status;
		FbxArray< FbxString*> details;
		FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(fbxScene), &status, &details);
		fbxStatus = sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData);
		bool fbxNotify = (!fbxStatus && details.GetCount() > 0) || (fbxImporter->GetStatus().GetCode() != FbxStatus::eSuccess);
		if( fbxNotify )
		{
			FBXSDK_printf("\n");
			FBXSDK_printf("********************************************************************************\n");
			if( details.GetCount() )
			{
				FBXSDK_printf("Scene integrity verification failed with the following errors:\n");
				for (int i = 0; i < details.GetCount(); i++)
					FBXSDK_printf("   %s\n", details[i]->Buffer());
				
				FbxArrayDelete<FbxString*>(details);
			}

			if( fbxImporter->GetStatus().GetCode() != FbxStatus::eSuccess )
			{
				FBXSDK_printf("\n");
				FBXSDK_printf("WARNING:\n");
				FBXSDK_printf("   The importer was able to read the file but with errors.\n");
				FBXSDK_printf("   Loaded scene may be incomplete.\n\n");
				FBXSDK_printf("   Last error message:'%s'\n", fbxImporter->GetStatus().GetErrorString());
			}
			FBXSDK_printf("********************************************************************************\n");
			FBXSDK_printf("\n");
		}
	}

    if( fbxStatus == false && fbxImporter->GetStatus().GetCode() == FbxStatus::ePasswordError )
    {
        FBXSDK_printf("Please enter password: ");

        fbxPassword[0] = '\0';

        FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
        scanf("%s", fbxPassword);
        FBXSDK_CRT_SECURE_NO_WARNING_END

        FbxString fbxPropString( fbxPassword );

        IOS_REF.SetStringProp(IMP_FBX_PASSWORD, fbxPropString);
        IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

        fbxStatus = fbxImporter->Import(fbxScene);

        if( fbxStatus == false && fbxImporter->GetStatus().GetCode() == FbxStatus::ePasswordError )
        {
            FBXSDK_printf("\nPassword is wrong, import aborted.\n");
        }
    }

    // Destroy the importer.
    fbxImporter->Destroy();

    return fbxStatus;
}
