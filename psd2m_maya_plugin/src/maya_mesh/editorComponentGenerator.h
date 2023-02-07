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

#if defined PSDTO3D_MAYA_VERSION
#include <maya/MFnLambertShader.h>
#include <maya/MDagModifier.h>
#else
#include "mayaStub.h"
#endif

namespace maya_plugin
{
	class EditorComponentGenerator
	{
	public:
		static const char* EditorComponentGenerator::meshNamePostfix; // blank, no postfix
		static const char* EditorComponentGenerator::materialNamePostfix; //"_Lb"
		static const char* EditorComponentGenerator::textureNamePostfix; //"_PNG"
		static const char* EditorComponentGenerator::shaderGroupNamePostfix; //"_SG"
		static MString CreateUniqueName( MString const& name);
		static MObject CreateTransformShape(   MDagModifier& dag, MString const& name, float const& depth, MObject& parent);
		static MObject CreateMaterialNode(     MDagModifier& dag, MString const& name);
		static void CreateShaderGroupNode(     MDagModifier& dag, MString const& name, MFnLambertShader const& fnLambert, MObject& meshObj );
		static void UpdateShaderGroupNode(     MDagModifier& dag, MString const& name, MFnLambertShader const& fnLambert, MObject& meshObj );
		static void CreatePlaced2DTextureNode( MDagModifier& dag, MString const& name, MFnLambertShader const& fnLambert );
		static void UpdateTextureNode(         MDagModifier& dag, MString const& name, MFnLambertShader const& fnLambert, MString const& textureFilepath );
	};
}
#endif // EDITORCOMPONENTGENERATOR_H
