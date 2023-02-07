//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file editorComponentGenerator.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "editorComponentGenerator.h"
#if defined PSDTO3D_MAYA_VERSION
#include <maya/MGlobal.h>
#include <maya/MFnTransform.h>
#include <maya/MFnSet.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>
#else
#include "mayaStub.h"
#endif
#include "mayaMeshConvertor.h"

namespace maya_plugin
{
#pragma region EDITOR OPERATION

	const char* EditorComponentGenerator::meshNamePostfix = ""; // Mesh is the most user-visible, has privilige of no postfix
	const char* EditorComponentGenerator::materialNamePostfix = "_Lb"; // stands for Lambert
	const char* EditorComponentGenerator::textureNamePostfix = "_PNG";
	const char* EditorComponentGenerator::shaderGroupNamePostfix = "_SG";

	//--------------------------------------------------------------------------------------------------------------------------------------
	MString EditorComponentGenerator::CreateUniqueName(MString const& name)
	{
		MString uniqueName = name;
		bool isUnique = false;
		for( int i=1; !isUnique; i++ )
		{
			MSelectionList list;
			MStatus status = MGlobal::getSelectionListByName( uniqueName, list );
			if( list.isEmpty() )
				isUnique = true;
			else
				uniqueName = name + i;
		}
		return uniqueName;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	MObject EditorComponentGenerator::CreateTransformShape(MDagModifier& dag, MString const& name, float const& depth, MObject & parent)
	{
		MObject& transform = dag.createNode("transform", parent);
		dag.renameNode( transform, name );

		MFnTransform transformFn(transform);
		transformFn.setTranslation(MVector(0, 0, depth), MSpace::kTransform);
		dag.doIt(); // TODO: maybe disable this, will happen later anyway
		return transform;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	MObject EditorComponentGenerator::CreateMaterialNode(MDagModifier& dag, MString const& name)
	{
		MFnLambertShader fnLambert;
		MObject lambert = fnLambert.create(true);
		fnLambert.setName( name ); // name is usually postfixed with "_Lb"
		return lambert;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void EditorComponentGenerator::CreateShaderGroupNode( MDagModifier& dag, MString const& name, MFnLambertShader const& fnLambert, MObject & meshObj )
	{
		MFnSet fnSet;
		MSelectionList selList;

		MString materialName = fnLambert.name();
		MObject shadingGroup = fnSet.create(selList, MFnSet::kRenderableOnly, false);
		MString resNameSg = fnSet.setName( name );
		fnSet.addMember(meshObj);

		dag.commandToExecute("connectAttr -f " + materialName + ".outColor " + resNameSg + ".surfaceShader;");
		dag.commandToExecute("setAttr \"" + materialName + ".ambientColor\" - type double3 1 1 1;");
		dag.doIt(); // execute the script
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void EditorComponentGenerator::UpdateShaderGroupNode( MDagModifier& dag, MString const& name, MFnLambertShader const& fnLambert, MObject & meshObj )
	{
		bool found = false;

		// Get Output plug
		MPlug fnLambertOutplug = fnLambert.findPlug("outColor",false);

		// Get connection from lambert to get the output plug
		MPlugArray nodeplugs;
		bool res = fnLambertOutplug.connectedTo(nodeplugs, false, true);
		if (res)
		{
			MObject outNode = nodeplugs[0].node();
			if( outNode.hasFn(MFn::kSet) )
			{
				MFnSet fnSet(outNode);
				fnSet.addMember(meshObj);
				found = true;
			}
		}

		if( !found )
		{
			CreateShaderGroupNode( dag, name, fnLambert, meshObj );
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void EditorComponentGenerator::CreatePlaced2DTextureNode( MDagModifier& dag, MString const& name, MFnLambertShader const& fnLambert )
	{
		// hack "mel scripting", C++ class not found yet.
		MString materialName = fnLambert.name();
		dag.commandToExecute("shadingNode - asTexture - isColorManaged -name " + name + " file;");
		dag.commandToExecute("shadingNode - asUtility -name " + name + "_place2dTexture place2dTexture;");
		   
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.coverage " + name + ".coverage;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.translateFrame " + name + ".translateFrame;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.rotateFrame " + name + ".rotateFrame;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.mirrorU " + name + ".mirrorU;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.mirrorV " + name + ".mirrorV;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.stagger " + name + ".stagger;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.wrapU " + name + ".wrapU;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.wrapV " + name + ".wrapV;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.repeatUV " + name + ".repeatUV;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.offset " + name + ".offset;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.rotateUV " + name + ".rotateUV;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.noiseUV " + name + ".noiseUV;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.vertexUvOne " + name + ".vertexUvOne;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.vertexUvTwo " + name + ".vertexUvTwo;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.vertexUvThree " + name + ".vertexUvThree;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.vertexCameraOne " + name + ".vertexCameraOne;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.outUV " + name + ".uv;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.outUvFilterSize " + name + ".uvFilterSize;");

		dag.commandToExecute("connectAttr - f " + name + ".outColor " + materialName + ".color;");
		dag.commandToExecute("connectAttr - f " + name + ".outTransparency " + materialName + ".transparency;");
		dag.doIt(); // execute the script
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void EditorComponentGenerator::UpdateTextureNode( MDagModifier& dag, MString const& name,
		MFnLambertShader const& fnLambert, MString const& textureFilepath )
	{
		dag; // unused

		MStatus status;
		// Get Color plug
		MPlug fnLambertColorplug = fnLambert.findPlug("color",false,&status);

		// Get connection from lambert to get the file texture name plug
		MPlugArray nodeplugs;
		bool res = fnLambertColorplug.connectedTo(nodeplugs, true, false);
		if( res ) // TODO: error to catch.
		{
			MObject textureNode = nodeplugs[0].node();
			MFnDependencyNode depNode(textureNode);
			MPlug plugFileTextureName = depNode.findPlug("fileTextureName",false,&status);

			// Set value to fileTextureName.
			const MString imageLocation(textureFilepath); //pathTexture + "\\" + name + ".png"
			plugFileTextureName.setValue(imageLocation);
			depNode.setName(name);
		}
	}

#pragma endregion
}
