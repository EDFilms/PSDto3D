//----------------------------------------------------------------------------------------------
// ===============================================
//	Copyright (C) 2016, CDRIN.
//	All Rights Reserved.
// ===============================================
//	Unauthorized copying of this file, via any medium is strictly prohibited
//	Proprietary and confidential
//
//	@file editorComponentGenerator.h
//	@author Fabien Raspail
//	@date 11-11-2016
//
//	@section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef EDITORCOMPONENTGENERATOR_H
#define EDITORCOMPONENTGENERATOR_H

#include <maya/MFnLambertShader.h>
#include <maya/MDagModifier.h>

namespace maya_plugin
{
	class EditorComponentGenerator
	{
	public:
		static MObject CreateTransformShape(MDagModifier& dag, MString const& name, float const& depth, MObject& parent);
		static MString CreateMaterialNode(MFnLambertShader& fnLambert, MDagModifier& dag, MString const& name);
		static void CreateGroupShaderNode(MDagModifier& dag, MString const& name, MString const& materialName, MObject& meshObj);
		static void CreatePlaced2DTexture(MDagModifier& dag, MString const& name, MString const& materialName);
		static void SetTexture(MString const& name, MString const& pathTexture, MFnLambertShader const& lambertMat);
	};
}
#endif // EDITORCOMPONENTGENERATOR_H
