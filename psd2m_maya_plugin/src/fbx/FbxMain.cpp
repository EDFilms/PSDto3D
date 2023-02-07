//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2020, E.D. Films.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file FbxMain.h
//  @author Michaelson Britt
//  @date 04-01-2020
//
//  @section DESCRIPTION
//  Based on sample project ExportScene03 from the Autodesk FBX SDK 2020
//
//----------------------------------------------------------------------------------------------


// Local headers
#include "FbxCommon.h"
#include <util\utils.h>
#include <util\math_2D.h>
#include <maya_mesh\MayaMeshConvertor.h>
#include <scene_controller\MeshGeneratorController.h>

// System headers
#include "windows.h"
#include <string.h>
#include <locale.h>
#include <tchar.h>
#include <vector>

// Library headers
#include <fbxsdk.h>


typedef void (*t_vfni)( int );
typedef void (*t_vfnvp)( void* );


using util::Vector2F;

#define SAMPLE_FILENAME_MC		"ExportScene03_MC.fbx"
#define SAMPLE_FILENAME_PC2		"ExportScene03_PC2.fbx"
#define SAMPLE_CACHE_TYPE		2

#define PID_MY_GEOMETRY_LEMENT	0

// Function prototypes.

void CreateTexture(FbxScene* pScene, FbxMesh* pMesh);
void CreateMaterials(FbxScene* pScene, FbxMesh* pMesh);


bool gExportVertexCacheMCFormat = true;

// Declare the UV names globally so we can create them on the mesh and then assign them properly
// to our textures when we create them
static const char* gDiffuseElementName = "DiffuseUV";
static const char* gAmbientElementName = "AmbientUV";
static const char* gEmissiveElementName = "EmissiveUV";

// gCacheType == 0 (default)  - double vertex array
//            == 1            - int32 array
//            == 2            - float array
int gCacheType = -1;


// Create texture for cube.
void CreateTexture(FbxScene* pScene, FbxMesh* pMesh)
{
    // A texture need to be connected to a property on the material,
    // so let's use the material (if it exists) or create a new one
    FbxSurfacePhong* lMaterial = NULL;

    //get the node of mesh, add material for it.
    FbxNode* lNode = pMesh->GetNode();
    if(lNode)
    {
        lMaterial = lNode->GetSrcObject<FbxSurfacePhong>(0);
        if (lMaterial == NULL)
        {      
            FbxString lMaterialName = "toto";
            FbxString lShadingName  = "Phong";
            FbxDouble3 lBlack(0.0, 0.0, 0.0);
            FbxDouble3 lRed(1.0, 0.0, 0.0);
            FbxDouble3 lDiffuseColor(0.75, 0.75, 0.0);

            FbxLayer* lLayer = pMesh->GetLayer(0);     

            // Create a layer element material to handle proper mapping.
            FbxLayerElementMaterial* lLayerElementMaterial = FbxLayerElementMaterial::Create(pMesh, lMaterialName.Buffer());

            // This allows us to control where the materials are mapped.  Using eAllSame
            // means that all faces/polygons of the mesh will be assigned the same material.
            lLayerElementMaterial->SetMappingMode(FbxLayerElement::eAllSame);
            lLayerElementMaterial->SetReferenceMode(FbxLayerElement::eIndexToDirect);
            
            // Save the material on the layer
            lLayer->SetMaterials(lLayerElementMaterial);

            // Add an index to the lLayerElementMaterial.  Since we have only one, and are using eAllSame mapping mode,
            // we only need to add one.
            lLayerElementMaterial->GetIndexArray().Add(0);

            lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());

            // Generate primary and secondary colors.
            lMaterial->Emissive           .Set(lBlack);
            lMaterial->Ambient            .Set(lRed);
            lMaterial->AmbientFactor      .Set(1.);
            // Add texture for diffuse channel
            lMaterial->Diffuse           .Set(lDiffuseColor);
            lMaterial->DiffuseFactor     .Set(1.);
            lMaterial->TransparencyFactor.Set(0.4);
            lMaterial->ShadingModel      .Set(lShadingName);
            lMaterial->Shininess         .Set(0.5);
            lMaterial->Specular          .Set(lBlack);
            lMaterial->SpecularFactor    .Set(0.3);
            lNode->AddMaterial(lMaterial);
        }
    }

    FbxFileTexture* lTexture = FbxFileTexture::Create(pScene,"Diffuse Texture");

    // Set texture properties.
    lTexture->SetFileName("scene03.jpg"); // Resource file is in current directory.
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);
    lTexture->UVSet.Set(FbxString(gDiffuseElementName)); // Connect texture to the proper UV

    
    // don't forget to connect the texture to the corresponding property of the material
    if (lMaterial)
        lMaterial->Diffuse.ConnectSrcObject(lTexture);

    lTexture = FbxFileTexture::Create(pScene,"Ambient Texture");

    // Set texture properties.
    lTexture->SetFileName("gradient.jpg"); // Resource file is in current directory.
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);
    lTexture->UVSet.Set(FbxString(gAmbientElementName)); // Connect texture to the proper UV

    // don't forget to connect the texture to the corresponding property of the material
    if (lMaterial)
        lMaterial->Ambient.ConnectSrcObject(lTexture);

    lTexture = FbxFileTexture::Create(pScene,"Emissive Texture");

    // Set texture properties.
    lTexture->SetFileName("spotty.jpg"); // Resource file is in current directory.
    lTexture->SetTextureUse(FbxTexture::eStandard);
    lTexture->SetMappingType(FbxTexture::eUV);
    lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
    lTexture->SetSwapUV(false);
    lTexture->SetTranslation(0.0, 0.0);
    lTexture->SetScale(1.0, 1.0);
    lTexture->SetRotation(0.0, 0.0);
    lTexture->UVSet.Set(FbxString(gEmissiveElementName)); // Connect texture to the proper UV

    // don't forget to connect the texture to the corresponding property of the material
    if (lMaterial)
        lMaterial->Emissive.ConnectSrcObject(lTexture);
}

// Create materials for pyramid.
void CreateMaterials(FbxScene* pScene, FbxMesh* pMesh)
{
    int i;

    for (i = 0; i < 5; i++ )
    {
        FbxString lMaterialName = "material";
        FbxString lShadingName = "Phong";
        lMaterialName += i;
        FbxDouble3 lBlack(0.0, 0.0, 0.0);
        FbxDouble3 lRed(1.0, 0.0, 0.0);
        FbxDouble3 lColor;
        FbxSurfacePhong *lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());


        // Generate primary and secondary colors.
        lMaterial->Emissive.Set(lBlack);
        lMaterial->Ambient.Set(lRed);
        lColor = FbxDouble3(i > 2   ? 1.0 : 0.0, 
            i > 0 && i < 4 ? 1.0 : 0.0, 
            i % 2   ? 0.0 : 1.0);
        lMaterial->Diffuse.Set(lColor);
        lMaterial->TransparencyFactor.Set(0.0);
        lMaterial->ShadingModel.Set(lShadingName);
        lMaterial->Shininess.Set(0.5);

        //get the node of mesh, add material for it.
        FbxNode* lNode = pMesh->GetNode();
        if(lNode)             
            lNode->AddMaterial(lMaterial);
    }  
}


//--------------------------------------------------------------------------------------------------------------------------------------


class PsdToFbxPluginOutput : public psd_to_3d::IPluginOutput
{
public:
	typedef std::vector<const GraphLayer*> GraphLayerList; // layer list
	typedef std::map< std::string, GraphLayerList > GraphLayerListLookup; // texture name -> layer list lookup

	typedef struct {
		std::string psdLayerName;
		std::string textureFilename;
		int psdLayerWidth, psdLayerHeight;
	} TextureInfo;
	typedef std::map<int,FbxSurfacePhong*> FbxMaterialLookup;

	// TODO: Restore this.  Problem is that OutputTexture() and OutputTree() are not both called in any given session, needs redesign
	//std::map<std::string,TextureInfo> textureInfoMap; // psdLayerName -> TextureInfo

	std::string psdPath;
	std::string psdName;
	std::string exportPath;
	std::string exportName;
	std::string exportExt;
	std::string exportGroup;
	//TCHAR saveDialogPath[MAX_PATH]; // filename from the file save dialog

	PsdToFbxPluginOutput()
	{
		//saveDialogPath[0] = '\0';
	}

	virtual void BeginSession( const IPluginOutputParameters& params ) override
	{
		this->psdPath = params.FileImportPath();
		this->psdName = params.PsdName();
		this->exportPath = params.FileExportPath();
		this->exportName = params.FileExportName();
		this->exportExt = params.FileExportExt();
		this->exportGroup = params.AliasPsdName();
		if( this->exportGroup.size()<=0 )
			this->exportGroup = this->psdName;
		//textureInfoMap.clear(); 
	}
	virtual void OutputMesh( const PsdData& psdData, const IPluginOutputParameters& params, const DataSurface& dataSurface, int layerIndex )
	{
		psdData; // unused
		params; // unused
		dataSurface; // unused
		layerIndex; // unused
		// TODO: Not yet implemented
	}
	virtual void OutputTree( const PsdData& psdData, const IPluginOutputParameters& params, const GroupByNameMap& tree )
	{
		psdData; // unused
		tree; // unused

		if( params.FileWriteLayout()==IPluginOutputParameters::MULTI_PER_LAYER )
		{
			// ----- EXPORT SINGLE FBX, ALL LAYERS COMBINED
			for( const GroupByNameMap::value_type& groupLayerItem : tree )
			{
				// Iterate through all layers in the current group ...
				const GraphLayerGroup& graphLayerGroup = groupLayerItem.second;
				for( int graphLayerIndex=0; graphLayerIndex<graphLayerGroup.GetLayerCount(); graphLayerIndex++ )
				{
					const GraphLayer* graphLayerPtr = graphLayerGroup[graphLayerIndex];
					if( graphLayerPtr==nullptr ) continue;

					GraphLayerList graphLayerList;
					graphLayerList.push_back(graphLayerPtr);
					const std::string postfix = std::string("_").append( util::NormalizeFilenameString( graphLayerPtr->LayerName ) );
					OutputMeshes( psdData, params, postfix, graphLayerList ); // layer name as prefix
				}
			}
		}
		else if( params.FileWriteLayout()==IPluginOutputParameters::MULTI_PER_TEXTURE )
		{
			GraphLayerListLookup graphLayerListLookup;
			// ----- EXPORT SINGLE FBX, ALL LAYERS COMBINED
			for( const GroupByNameMap::value_type& groupLayerItem : tree )
			{
				// Iterate through all layers in the current group ...
				const GraphLayerGroup& graphLayerGroup = groupLayerItem.second;
				for( int graphLayerIndex=0; graphLayerIndex<graphLayerGroup.GetLayerCount(); graphLayerIndex++ )
				{
					const GraphLayer* graphLayerPtr = graphLayerGroup[graphLayerIndex];
					if( graphLayerPtr==nullptr ) continue;

					GraphLayerList& graphLayerList = graphLayerListLookup[ graphLayerPtr->TextureName ];
					graphLayerList.push_back(graphLayerPtr);
				}
			}
			for( GraphLayerListLookup::iterator iter = graphLayerListLookup.begin(); iter!=graphLayerListLookup.end(); iter++ )
			{
				const GraphLayerList& graphLayerList = iter->second;
				const std::string postfix = std::string("_").append( util::NormalizeFilenameString( iter->first ) );
				OutputMeshes( psdData, params, postfix, graphLayerList ); // material name as prefix
			}
		}
		else
		{
			// ----- EXPORT SINGLE FBX, ALL LAYERS COMBINED
			GraphLayerList graphLayerList;
			for( const GroupByNameMap::value_type& groupLayerItem : tree )
			{
				// Iterate through all layers in the current group ...
				const GraphLayerGroup& graphLayerGroup = groupLayerItem.second;
				for( int graphLayerIndex=0; graphLayerIndex<graphLayerGroup.GetLayerCount(); graphLayerIndex++ )
				{
					const GraphLayer* graphLayerPtr = graphLayerGroup[graphLayerIndex];
					if( graphLayerPtr==nullptr ) continue;

					graphLayerList.push_back(graphLayerPtr);
				}
			}
			OutputMeshes( psdData, params, "", graphLayerList ); // no prefix
		}
	}

	virtual bool OutputMeshes( const PsdData& psdData, const IPluginOutputParameters& params, const std::string& exportPostfix, const GraphLayerList& graphLayerList )
	{

		bool result = true;
		int errorCount = 0;
		FbxManager* fbxSdkManager = NULL;
		FbxScene* fbxScene = NULL;
		char fbxFileName[MAX_PATH];
		// Prepare the FBX SDK
		InitializeSdkObjects(fbxSdkManager, fbxScene);

		// Create the top-level group node
		FbxString fbxGroupNodeName = FbxString( this->exportGroup.data() );
		FbxNode* fbxGroupNode = FbxNode::Create( fbxScene, fbxGroupNodeName );
		fbxScene->GetRootNode()->AddChild( fbxGroupNode );

		FbxMaterialLookup fbxMaterialLookup;

		// Iterate through all groups in the Photoshop file ...
		for( const GraphLayer* graphLayerPtr : graphLayerList )
		{
			const GraphLayer& graphLayer = *graphLayerPtr;

			// ----- CREATE MESH
			FbxNode* fbxSceneNode = AddMesh( *fbxScene, fbxMaterialLookup, psdData, graphLayer );
			// -----

			if( fbxSceneNode==nullptr )
			{
				errorCount++; // record error but don't abort
			}
			else
			{
				fbxGroupNode->AddChild( fbxSceneNode );
			}
		}

		if( errorCount > 0 )
		{
			FBXSDK_printf("\n\nAn error occurred while creating the scene...\n");
		}

		if( result != false )
		{
			// Save the scene.
			int writerFormat = kFbxWriterFormatBinary;
			if( params.FileWriteMode()==IPluginOutputParameters::ASCII )
				writerFormat = kFbxWriterFormatAscii;

			sprintf_s( fbxFileName, sizeof(fbxFileName), "%s/%s%s.%s", exportPath.c_str(), exportName.c_str(), exportPostfix.c_str(), exportExt.c_str() );
			result = SaveScene(fbxSdkManager, fbxScene, fbxFileName, writerFormat);
			if( result == false )
				printf("\n\nAn error occurred while saving the scene...\n");
		}

		DestroySdkObjects(fbxSdkManager, result);

		//textureInfoMap.clear();
		return result;
	}

	virtual FbxNode* AddMesh( FbxScene& fbxScene, FbxMaterialLookup& fbxMaterialLookup, const PsdData& psdData, const GraphLayer& graphLayer )
	{
		FbxNode* fbxSceneNode = nullptr;

		// Check is layer is valid
		if( !graphLayer.Mesh.IsEmpty() )
		{
			FbxString fbxMeshName = FbxString(graphLayer.LayerName.c_str()) + "_mesh";
			FbxMesh* fbxMesh = FbxMesh::Create( &fbxScene, fbxMeshName );

			int psdSceneWidth = psdData.HeaderData.Width;
			int psdSceneHeight = psdData.HeaderData.Height;

			float psdSceneWidthScaled  = 20.0f * graphLayer.Scale;
			float psdSceneHeightScaled = 20.0f * graphLayer.Scale * psdSceneHeight/psdSceneWidth;

			boundsUV layer_region = graphLayer.LayerRegion;
			Vector2F size_region = layer_region.GetSize();
			float width_layer  = psdSceneWidthScaled  * size_region.x;
			float height_layer = psdSceneHeightScaled * size_region.y;

			float x_left_min = -psdSceneWidthScaled  * 0.5f;
			float x_left_region = layer_region.TopLeftPoint().x;

			float x_left = x_left_min + (x_left_region * psdSceneWidthScaled);
			float x_right = x_left + width_layer;

			float y_top_min = 0.0f;
			float y_top_region = (1.0f - layer_region.TopLeftPoint().y);

			float y_top = y_top_min + (y_top_region * psdSceneHeightScaled);
			float y_bottom = y_top + height_layer;

			float z_layer = graphLayer.Depth;

			// Define vertex positions
			FbxVector4 cornerPos0( x_left,  y_top,    0.0f);
			FbxVector4 cornerPos1( x_right, y_top,    0.0f);
			FbxVector4 cornerPos2( x_right, y_bottom, 0.0f);
			FbxVector4 cornerPos3( x_left,  y_bottom, 0.0f);

			// Define vertex texture coordinates
			xformUV transform = graphLayer.GetXFormUV();

			// We want to have one normal for each vertex (or control point),
			// so we set the mapping mode to eByControlPoint.
			FbxGeometryElementNormal* fbxElementNormal= fbxMesh->CreateElementNormal();
			FBX_ASSERT( fbxElementNormal != NULL);
			fbxElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);
			fbxElementNormal->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

			// Create UV for Diffuse channel
			FbxGeometryElementUV* fbxElementUV = fbxMesh->CreateElementUV( gDiffuseElementName );
			FBX_ASSERT( fbxElementUV != NULL);
			fbxElementUV->SetMappingMode( FbxGeometryElement::eByPolygonVertex );
			fbxElementUV->SetReferenceMode( FbxGeometryElement::eIndexToDirect );

			// Add the different normals to the direct array
			FbxVector4 normalVec(0, 0, 1);
			fbxElementNormal->GetDirectArray().Add( normalVec );

			const DataSurface& surface = graphLayer.Mesh;
			const DataMesh* mesh = surface.GetDataMesh();
			if( mesh==nullptr )
			{
				// Mesh is not available; Create rectangular billboard instead

				// Create geometry vertex list
				fbxMesh->InitControlPoints( 4 );
				// Get pointer to geometry vertex list
				FbxVector4* fbxControlPoints = fbxMesh->GetControlPoints();
				// Set geometry vertex values
				fbxControlPoints[0] = cornerPos0; // Geom vertex value
				fbxControlPoints[1] = cornerPos1; // Geom vertex value
				fbxControlPoints[2] = cornerPos2; // Geom vertex value
				fbxControlPoints[3] = cornerPos3; // Geom vertex value

												  // Create the geometry polygon
				fbxMesh->BeginPolygon(-1, -1, false);
				// Set geometry vertex indices
				fbxMesh->AddPolygon( 0 ); // Geom vertex index
				fbxMesh->AddPolygon( 1 ); // Geom vertex index
				fbxMesh->AddPolygon( 2 ); // Geom vertex index
				fbxMesh->AddPolygon( 3 ); // Geom vertex index
										  // End the geometry polygon
				fbxMesh->EndPolygon();

				// Set normal index values; in this case every vertex uses normal 0
				fbxElementNormal->GetIndexArray().Add( 0 ); // Normal index, all same normal
				fbxElementNormal->GetIndexArray().Add( 0 ); // Normal index, all same normal
				fbxElementNormal->GetIndexArray().Add( 0 ); // Normal index, all same normal
				fbxElementNormal->GetIndexArray().Add( 0 ); // Normal index, all same normal

															// Create UV vertex lit
				fbxElementUV->GetIndexArray().SetCount( 4 );
				// Define vertex texture coordinates
				Vector2F cornerTex0v = transform.Transform( layer_region.TopLeftPoint() );
				Vector2F cornerTex1v = transform.Transform( layer_region.TopRightPoint() );
				Vector2F cornerTex2v = transform.Transform( layer_region.BottomRightPoint() );
				Vector2F cornerTex3v = transform.Transform( layer_region.BottomLeftPoint() );
				FbxVector2 cornerTex0( cornerTex0v.x,  (1.0f-cornerTex0v.y) );
				FbxVector2 cornerTex1( cornerTex1v.x,  (1.0f-cornerTex1v.y) );
				FbxVector2 cornerTex2( cornerTex2v.x,  (1.0f-cornerTex2v.y) );
				FbxVector2 cornerTex3( cornerTex3v.x,  (1.0f-cornerTex3v.y) );
				// Set UV vertex values
				fbxElementUV->GetDirectArray().Add( cornerTex0 ); // UV vertex value
				fbxElementUV->GetDirectArray().Add( cornerTex1 ); // UV vertex value
				fbxElementUV->GetDirectArray().Add( cornerTex2 ); // UV vertex value
				fbxElementUV->GetDirectArray().Add( cornerTex3 ); // UV vertex value
																  // Set UV vertex indices
				fbxElementUV->GetIndexArray().SetAt( 0, 0 ); // UV vertex index
				fbxElementUV->GetIndexArray().SetAt( 1, 1 ); // UV vertex index
				fbxElementUV->GetIndexArray().SetAt( 2, 2 ); // UV vertex index
				fbxElementUV->GetIndexArray().SetAt( 3, 3 ); // UV vertex index
			}
			else
			{
				// Mesh is available

				const std::vector<Vector2F>& vertices = mesh->GetVertices();
				const std::vector<int>& faceSizes = mesh->GetFaceSizes();
				const std::vector<int>& faceVerts = mesh->GetFaceVerts();

				// Set geometry vertex values
				fbxMesh->SetControlPointCount( (int)(vertices.size()) );
				for( int i=0; i<vertices.size(); i++ )
				{
					Vector2F pos = Vector2F( vertices[i].x, vertices[i].y ); // pos in original abstract coordinates
					pos.x = x_left_min + (pos.x * psdSceneWidthScaled); // pos in scene coordinates
					pos.y = y_top_min  + ((1.0f - pos.y) * psdSceneHeightScaled); // need to flip y-coordinate

					Vector2F uv  = Vector2F( vertices[i].x, vertices[i].y );  // uv in original texture coordinates
					uv = transform.Transform( uv ); // uv texture coordinates, for atlas or individual texture as applicable
					uv.y = (1.0f - uv.y); // need to flip y-coordinate, after transform

					fbxMesh->SetControlPointAt( FbxVector4( pos.x, pos.y, 0, 0 ), i ); // Geom vertex value
					fbxElementUV->GetDirectArray().Add( FbxVector2( uv.x, uv.y ) ); // UV vertex value
				}

				int fi = 0; // index into facesIndices array
				for( int j=0; j<faceSizes.size(); j++ )
				{
					// Create the geometry polygon
					fbxMesh->BeginPolygon(-1, -1, false);
					for( int i=0; i<faceSizes[j]; i++ )
					{
						int vertIndex = faceVerts[fi++];
						// Set geometry vertex indices
						fbxMesh->AddPolygon( vertIndex ); // Geom vertex index
						fbxElementUV->GetIndexArray().Add( vertIndex ); // UV vertex index
						fbxElementNormal->GetIndexArray().Add( 0 ); // Normal index, all same normal
					}
					// End the geometry polygon
					fbxMesh->EndPolygon();
				}
			}

			FbxString fbxSceneNodeName = FbxString(graphLayer.LayerName.c_str()); // + "_node"
			fbxSceneNode = FbxNode::Create( &fbxScene, fbxSceneNodeName );

			fbxSceneNode->SetNodeAttribute(fbxMesh);

			// TODO: Move this to helper method
			fbxSceneNode->SetShadingMode(FbxNode::eTextureShading);

			FbxSurfacePhong* fbxMaterial = nullptr;

			FbxNode* fbxMeshNode = fbxMesh->GetNode();
			if( fbxMeshNode )
			{
				if( graphLayer.AtlasIndex>=0 ) // if using an atlas, find shared material in lookup
				{
					FbxMaterialLookup::iterator it = fbxMaterialLookup.find(graphLayer.AtlasIndex);
					if( it!=fbxMaterialLookup.end() )
						fbxMaterial = it->second;
				}

				// if material not found, we need to create it
				if( fbxMaterial==nullptr )
				{
					fbxMaterial = fbxMeshNode->GetSrcObject<FbxSurfacePhong>(0);
					FbxString lShadingName  = "Phong";
					FbxDouble3 lBlack(0.0, 0.0, 0.0);
					FbxDouble3 lWhite(1.0, 1.0, 1.0);
					FbxDouble3 lDiffuseColor(0.0, 0.0, 0.0);

					FbxString lShaderName = FbxString(graphLayer.MaterialName.c_str()) + "_shader";
					fbxMaterial = FbxSurfacePhong::Create( &fbxScene, lShaderName );

					// Generate primary and secondary colors.
					fbxMaterial->Emissive          .Set(lBlack);
					fbxMaterial->Ambient           .Set(lBlack);
					fbxMaterial->AmbientFactor     .Set(0.);
					// Add texture for diffuse channel
					fbxMaterial->Diffuse           .Set(lDiffuseColor);
					fbxMaterial->DiffuseFactor     .Set(1.);
					fbxMaterial->TransparentColor  .Set(lDiffuseColor);
					fbxMaterial->TransparencyFactor.Set(1.);
					fbxMaterial->ShadingModel      .Set(lShadingName);
					fbxMaterial->Shininess         .Set(0.);
					fbxMaterial->Specular          .Set(lBlack);
					fbxMaterial->SpecularFactor    .Set(0.);

					FbxString fbxTextureName = FbxString(graphLayer.TextureName.c_str()) + "_texture";
					FbxFileTexture* fbxTexture = FbxFileTexture::Create( &fbxScene, fbxTextureName );

					// Set texture properties.
					// TODO: Shouldn't assume png. Maya exporter currently makes same assumption
					FbxString fbxTextureFile = FbxString(exportName.c_str()) + "_" + graphLayer.TextureName.c_str() + ".png";
					fbxTexture->SetFileName( fbxTextureFile ); // Resource file is in current directory.
					fbxTexture->SetTextureUse( FbxTexture::eStandard );
					fbxTexture->SetMappingType( FbxTexture::eUV );
					fbxTexture->SetMaterialUse( FbxFileTexture::eModelMaterial );
					fbxTexture->SetAlphaSource ( FbxTexture::eBlack );
					fbxTexture->SetBlendMode( FbxTexture::eTranslucent );
					fbxTexture->SetSwapUV( false );
					fbxTexture->SetTranslation( 0.0, 0.0 );
					fbxTexture->SetScale( 1.0, 1.0 );
					fbxTexture->SetRotation( 0.0, 0.0 );
					fbxTexture->UVSet.Set( FbxString(gDiffuseElementName) ); // Connect texture to the proper UV

					if( fbxMaterial )
					{
						fbxMaterial->Diffuse.ConnectSrcObject( fbxTexture );
						fbxMaterial->TransparentColor.ConnectSrcObject( fbxTexture );
						if( graphLayer.AtlasIndex>=0 ) // if using an atlas, add shared material to lookup
						{
							fbxMaterialLookup[graphLayer.AtlasIndex] = fbxMaterial;
						}
					}
				}

				// Create a layer element material to handle proper mapping.
				FbxLayer* lLayer = fbxMesh->GetLayer(0);
				FbxString lMaterialName = FbxString(graphLayer.LayerName.c_str()) + "_material";
				FbxLayerElementMaterial* lLayerElementMaterial = FbxLayerElementMaterial::Create( fbxMesh, lMaterialName );

				// This allows us to control where the materials are mapped.  Using eAllSame
				// means that all faces/polygons of the mesh will be assigned the same material.
				lLayerElementMaterial->SetMappingMode(FbxLayerElement::eAllSame);
				lLayerElementMaterial->SetReferenceMode(FbxLayerElement::eIndexToDirect);

				// Save the material on the layer
				lLayer->SetMaterials(lLayerElementMaterial);

				// Add an index to the lLayerElementMaterial.  Since we have only one, and are using eAllSame mapping mode,
				// we only need to add one.
				lLayerElementMaterial->GetIndexArray().Add(0);

				// Apply the material
				fbxMeshNode->AddMaterial(fbxMaterial);

				fbxSceneNode->LclTranslation.Set( FbxVector4(0, 0, z_layer) );
				fbxSceneNode->LclRotation.Set( FbxVector4(0.0, 0.0, 0.0) );
				fbxSceneNode->LclScaling.Set( FbxVector4(1.0, 1.0, 1.0) );				
			}
		}
		return fbxSceneNode;
	}

	virtual void EndSession( const PsdData& psdData, const IPluginOutputParameters& params )
	{
		psdData; // unused
		params; // unused
	}
	virtual void CancelSession( const PsdData& psdData, const IPluginOutputParameters& params )
	{
		psdData; // unused
		params; // unused
	}
	// Called before BeginSession to query user for output filename
	virtual void GetSaveDialogParams(void* vofnw, const IPluginOutputParameters&)
	{
		OPENFILENAMEW* ofnw = (OPENFILENAMEW*)vofnw;
		// TODO: remove hard-coded strings
		static LPCWSTR szFilter = L"FBX files (*.fbx)\0*.fbx\0All files (*.*)\0*.*\0";
		static LPCWSTR szTitle = L"Export FBX...";
		static LPCWSTR szDefExt = L"fbx";
		ofnw->lpstrFilter = szFilter; // static buffer, otherwise corruption after return
		ofnw->lpstrDefExt = szDefExt;
		ofnw->lpstrTitle = szTitle; // static buffer, otherwise corruption after return
	}

}; // class PsdToFbxPluginOutput


PsdToFbxPluginOutput pluginOutput;

int WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nShowCmd)
{
	hInstance;		// unused
	hPrevInstance;	// unused
	lpCmdLine;		// unused
	nShowCmd;		// unused

	LPWSTR *argv;
	int argc;

	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	FBX_ASSERT( argc > 0 );

	// Calculate the appendix on the filename,
	// Goes from the last underscore ('_') inclusive to the last period ('.') exclusive
	// For example, "PSDto3D_FBX_dev.exe" has an appenix of "_dev"
	wchar_t exeFilename[MAX_PATH];
	wchar_t exeDrive[MAX_PATH];
	wchar_t exeDir[MAX_PATH];
	wchar_t libFilename[MAX_PATH];
	// 1) Take first argument, which is the full path of our exe,
	//    and extract only the filename part without file extension
	_wsplitpath_s( argv[0], exeDrive, MAX_PATH, exeDir, MAX_PATH, exeFilename, MAX_PATH, nullptr, 0 );
	// 2) Tokenize on the underscore ('_') character and find the last token
	wchar_t seps[]   = L"_";
	wchar_t *tok = nullptr, *tokContext = nullptr;
	wchar_t *appendix = nullptr;
	tok = wcstok_s( exeFilename, seps, &tokContext );
	while( tok!=nullptr )
	{
		appendix = tok;
		tok = wcstok_s( nullptr, seps, &tokContext ); // uses thread local state for parsing
	}
	// 3) Print out the final library filename using the appendix token
	if( (appendix!=nullptr) && (appendix!=(argv[0])) )
	{
		swprintf_s( libFilename, MAX_PATH, L"PSDto3D_Standalone_%s.dll", appendix );
	}
	else
	{
		swprintf_s( libFilename, MAX_PATH, L"PSDto3D_Standalone.dll" );
	}
	// 4) Release memory because dumb windows requirement
	LocalFree(argv);

//	SetEnvironmentVariable( _T("QT_PLUGIN_PATH"), _T(deps_path) );
//	SetEnvironmentVariable( _T("PATH"), _T(deps_path) );

	HMODULE hLibrary= LoadLibraryW( libFilename );
 	if( hLibrary!=NULL )
	{
		FARPROC lpfProcFunc = NULL;
		lpfProcFunc = GetProcAddress( hLibrary, "setPluginOutput" );
		if(lpfProcFunc!=NULL)
		{
			t_vfnvp fn_setPluginOutput = (t_vfnvp)lpfProcFunc;
			fn_setPluginOutput( &pluginOutput );
		}

		lpfProcFunc = GetProcAddress( hLibrary, "openPlugin" );
		if(lpfProcFunc!=NULL)
		{
			t_vfni fn_openPlugin = (t_vfni)lpfProcFunc;
			fn_openPlugin(1);
		}
	}
	else // failed to load library
	{
		// print last error
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError(); 

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );
	}

	return 1;
}
