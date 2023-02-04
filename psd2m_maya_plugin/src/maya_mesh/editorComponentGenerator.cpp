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
#include <maya/MFnTransform.h>
#include <maya/MFnSet.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>
#include "mayaMeshConvertor.h"

namespace maya_plugin
{
#pragma region EDITOR OPERATION

	//--------------------------------------------------------------------------------------------------------------------------------------
	MObject EditorComponentGenerator::CreateTransformShape(MDagModifier& dag, MString const& name, float const& depth, MObject & parent)
	{
		MObject& transform = dag.createNode("transform", parent);
		dag.renameNode(transform, name);

		MFnTransform transformFn(transform);
		transformFn.setTranslation(MVector(0, 0, depth), MSpace::kTransform);
		dag.doIt();
		return transform;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	MString EditorComponentGenerator::CreateMaterialNode(MFnLambertShader& fnLambert, MDagModifier& dag, MString const& name)
	{
		const MObject lambert = fnLambert.create(true);
		return fnLambert.setName(name + "_Lb");
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void EditorComponentGenerator::CreateGroupShaderNode(MDagModifier& dag, MString const& name, MString const& materialName, MObject & meshObj)
	{
		// MStatus status;
		const MString nameSG(name + "_SG");

		MFnSet fnSet;
		MSelectionList selList;

		MObject shadingGroup = fnSet.create(selList, MFnSet::kRenderableOnly, false);
		MString resNameSg = fnSet.setName(nameSG);
		fnSet.addMember(meshObj);

		dag.commandToExecute("connectAttr -f " + materialName + ".outColor " + resNameSg + ".surfaceShader;");
		dag.commandToExecute("setAttr \"" + materialName + ".ambientColor\" - type double3 1 1 1;");
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void EditorComponentGenerator::CreatePlaced2DTexture(MDagModifier& dag, MString const& name, MString const& materialName)
	{
		// hack "mel scripting", C++ class not found yet.
		dag.commandToExecute("shadingNode - asTexture - isColorManaged -name " + name + "_file file;");
		dag.commandToExecute("shadingNode - asUtility -name " + name + "_place2dTexture place2dTexture;");
		   
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.coverage " + name + "_file.coverage;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.translateFrame " + name + "_file.translateFrame;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.rotateFrame " + name + "_file.rotateFrame;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.mirrorU " + name + "_file.mirrorU;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.mirrorV " + name + "_file.mirrorV;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.stagger " + name + "_file.stagger;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.wrapU " + name + "_file.wrapU;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.wrapV " + name + "_file.wrapV;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.repeatUV " + name + "_file.repeatUV;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.offset " + name + "_file.offset;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.rotateUV " + name + "_file.rotateUV;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.noiseUV " + name + "_file.noiseUV;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.vertexUvOne " + name + "_file.vertexUvOne;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.vertexUvTwo " + name + "_file.vertexUvTwo;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.vertexUvThree " + name + "_file.vertexUvThree;");
		dag.commandToExecute("connectAttr - f " + name + "_place2dTexture.vertexCameraOne " + name + "_file.vertexCameraOne;");
		dag.commandToExecute("connectAttr " + name + "_place2dTexture.outUV " + name + "_file.uv;");
		dag.commandToExecute("connectAttr " + name + "_place2dTexture.outUvFilterSize " + name + "_file.uvFilterSize;");

		dag.commandToExecute("connectAttr - f " + name + "_file.outColor " + materialName + ".color;");
		dag.commandToExecute("connectAttr - f " + name + "_file.outTransparency " + materialName + ".transparency;");
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void EditorComponentGenerator::SetTexture(MString const& name, MString const& pathTexture, MFnLambertShader const& lambertMat)
	{
		// Get Color plug
		MPlug fnLambertColorplug = lambertMat.findPlug("color");

		// Get connection from lambert to get the file texture name plug
		MPlugArray nodeplugs;
		bool res = fnLambertColorplug.connectedTo(nodeplugs, true, false);
		if (!res) return; // TODO: error to catch.

		MObject textureNode = nodeplugs[0].node();
		MFnDependencyNode depNode(textureNode);
		MPlug plugFileTextureName = depNode.findPlug("fileTextureName");

		// Set value to fileTextureName.
		const MString imageLocation(pathTexture + "\\" + name + ".png");
		plugFileTextureName.setValue(imageLocation);
	}

#pragma endregion
}
