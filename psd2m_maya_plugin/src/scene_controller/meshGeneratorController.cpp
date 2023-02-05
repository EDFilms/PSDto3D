//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file meshGeneratorController.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "mesh_generator/dataMesh.h"
#include "mesh_generator/linear_mesh/linearMesh.h"
#include "mesh_generator/linear_mesh/bezierCurve.h"
#include "mesh_generator/curve_mesh/curveMeshGenerator.h"
#include "mesh_generator/influence_mesh/influenceMesh.h"
#include "texture_exporter/textureExporter.h"
#include "meshGeneratorController.h"
#include "editorComponentGenerator.h"
#include "qtProgress.h"
#include "mayaMeshConvertor.h"


#include <QtCore/qfileinfo.h>
#include <maya/MFnLambertShader.h>
#include <maya/MGlobal.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnTransform.h>
#include <maya/MFnSet.h>
#include <maya/MDGModifier.h>
#include <maya/MSelectionList.h>
#include <locale>
#include <sstream>

namespace maya_plugin
{
#pragma region CONSTRUCTOR

	//----------------------------------------------------------------------------------------
	MeshGeneratorController::MeshGeneratorController() = default;

	//----------------------------------------------------------------------------------------
	MeshGeneratorController::~MeshGeneratorController() = default;

#pragma endregion

#pragma region GENERAL

	//----------------------------------------------------------------------------------------
	void MeshGeneratorController::GenerateMayaMeshes(PsdData & data, GlobalParameters & params, Progress & progress)
	{
		InitializeProgressBar(data, params, progress);
		std::map<std::string, DataMesh> meshes;

		// mesh creation
		for (auto layer : data.LayerMaskData.Layers)
		{
			if (layer.Type > TEXTURE_LAYER)
				continue;

			const LayerParameters* layerParams = params.GetLayerParameter(layer.LayerName);
			if (!layerParams->IsActive)
				continue;

			progress.IncrementProgressBar();
			switch (layerParams->Algo)
			{
			case LayerParameters::Algorithm::LINEAR:
			{
				ResourceBlockPath blockPath = data.ImageResourceData.GetBlockPath(layer.LayerName);
				if (blockPath.Name.empty()) break;

				meshes.try_emplace(layer.LayerName, GenerateDataLinearMesh(blockPath, layerParams));
				break;
			}
			case LayerParameters::Algorithm::CURVE:
			{
				meshes.try_emplace(layer.LayerName, GenerateDataCurveGridMesh(layer, layerParams));
				break;
			}
			default:
				break;
			}
		}

		ApplyInfluenceLayer(data, meshes, params, progress);
		std::map<std::string, GroupLayer> hierarchy = CreateTreeStructure(params, data);

		// Maya editor node component creation
		CreateEditorMayaComponents(params, progress, hierarchy, meshes);
		progress.CompleteProgressBar();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void MeshGeneratorController::InitializeProgressBar(PsdData & data, GlobalParameters & params, Progress & progress)
	{
		unsigned n = 0;
		for (auto layer : data.LayerMaskData.Layers)
		{
			if (layer.Type > TEXTURE_LAYER)
				continue;

			LayerParameters* layerParams = params.GetLayerParameter(layer.LayerName);
			if (!layerParams->IsActive)
				continue;

			// Active Layer && Mesh Creation
			n += 2;
			
			if (!layerParams->InfluenceActivated || !layerParams->HasInfluenceLayer)
				continue;

			// Active Influence
			++n;
		}

		// Then increment one last time while waiting for Maya to finish
		++n;
		progress.InitializeProgressBar(n);
	}

	//----------------------------------------------------------------------------------------
	void MeshGeneratorController::CreateEditorMayaComponents(GlobalParameters & params, Progress & progress, std::map<std::string, GroupLayer> & tree, std::map<std::string, DataMesh> & meshes)

	{
		if (meshes.empty() || tree.empty()) return;
		MDagModifier dagController;


		std::map<std::string, MObject> existingMesh;
		MItDependencyNodes nodeItMesh(MFn::Type::kShape, nullptr);
		for (; !nodeItMesh.isDone(); nodeItMesh.next())
		{
			MFnMesh tmpTransform(nodeItMesh.item());
			existingMesh.try_emplace(std::string(tmpTransform.name().asChar()), nodeItMesh.item());
		}

		// Shape / Materiel / shader / texture nodes creation and association
		for (auto groupLayer : tree)
		{
			float currentDepth = 0.0f;
			for (auto const& meshName : groupLayer.second.LayerNames)
			{
				auto dataMesh = meshes.find(meshName);
				if (dataMesh == meshes.end()) continue;

				// create or update
				if (existingMesh.find(meshName) == existingMesh.end())
				{
					CreateShapeEditorComponent(dagController, params, dataMesh->second, currentDepth, groupLayer.second.Transform);
				}
				else
				{
					UpdateShapeEditorComponent(existingMesh[meshName], dagController, params, dataMesh->second, currentDepth);
				}
				currentDepth += params.Depth;

				progress.IncrementProgressBar();
			}
		}
	}

	//----------------------------------------------------------------------------------------
	std::map<std::string, GroupLayer> MeshGeneratorController::CreateTreeStructure(GlobalParameters const& params, psd_reader::PsdData const& data)
	{
		MDagModifier dagController;
		std::map<std::string, GroupLayer> groupLayers;
		std::vector<std::string> pastGroup;

		std::map<std::string, MObject> existingTransform;
		MItDependencyNodes nodeIt(MFn::Type::kTransform, nullptr);
		for (; !nodeIt.isDone(); nodeIt.next())
		{
			MFnTransform tmpTransform(nodeIt.item());
			existingTransform.try_emplace(std::string(tmpTransform.name().asChar()), nodeIt.item());
		}

		GroupLayer root;
		std::string alias = std::string(MQtUtil::toMString(params.AliasPsdName).asChar());
		std::string currentGroup = alias.empty() ? std::string(MQtUtil::toMString(params.PsdName).asChar()) : alias;
		if(existingTransform.find(currentGroup) == existingTransform.end())
		{ 
			root.Transform = dagController.createNode("transform", MObject());
			dagController.renameNode(root.Transform, MString(currentGroup.c_str()));
		}
		else
		{
			root.Transform = existingTransform[currentGroup];
		}
		root.LayerNames = std::vector<std::string>();		
		groupLayers.insert(std::pair<std::string, GroupLayer >(currentGroup, root));
		pastGroup.push_back(currentGroup);
		dagController.doIt();
		
		// Create the Transform structure base on the params option and photoshop group
		for (auto layer : data.LayerMaskData.Layers)
		{
			if (params.KeepGroupStructure)
			{
				if (layer.Type == OPEN_FOLDER || layer.Type == CLOSED_FOLDER)
				{
					MObject parent = groupLayers[currentGroup].Transform;
					currentGroup = layer.LayerName;
					GroupLayer group;
					if (existingTransform.find(currentGroup) == existingTransform.end())
					{
						group.Transform = dagController.createNode("transform", parent);
						dagController.renameNode(group.Transform, MString(currentGroup.c_str()));
					}
					else
					{
						group.Transform = existingTransform[currentGroup];
					}
					groupLayers.insert(std::pair<std::string, GroupLayer >(currentGroup, group));
					pastGroup.push_back(currentGroup);
					dagController.doIt();
					continue;
				}

				if (layer.Type == HIDDEN_DIVIDER)
				{
					pastGroup.pop_back();
					currentGroup = pastGroup[pastGroup.size() - 1];
					continue;
				}
			}

			if (layer.Type == TEXTURE_LAYER)
			{
				groupLayers[currentGroup].LayerNames.push_back(layer.LayerName);
			}
		}
		return groupLayers;
	}

	//----------------------------------------------------------------------------------------
	void MeshGeneratorController::CreateShapeEditorComponent(MDagModifier & dag, GlobalParameters const& params, DataMesh const& mesh, float const depth, MObject & transformParent)
	{
		const MString meshName(mesh.GetName().c_str());

		std::map<std::string, MObject> existingMesh;
		MItDependencyNodes nodeIt(MFn::Type::kMesh, nullptr);
		for (; !nodeIt.isDone(); nodeIt.next())
		{
			MFnTransform tmpTransform(nodeIt.item());
			existingMesh.try_emplace(std::string(tmpTransform.name().asChar()), nodeIt.item());
		}

		// Shape generation
		MayaMeshConvertor mayaShape(mesh, params.Scale);
		MObject transform = EditorComponentGenerator::CreateTransformShape(dag, meshName, depth, transformParent);
		MObject meshObj = mayaShape.CreateMayaMFnMesh(transform);
		dag.renameNode(meshObj, meshName);

		// Material creation
		MFnLambertShader fnLambert;
		MString matName = EditorComponentGenerator::CreateMaterialNode(fnLambert, dag, meshName);
		EditorComponentGenerator::CreateGroupShaderNode(dag, meshName, matName, meshObj);
		EditorComponentGenerator::CreatePlaced2DTexture(dag, meshName, matName);
		dag.doIt();

		// Associate texture and UV
		QFileInfo pathFolder(params.FilePath);
		const MString path = MQtUtil::toMString(pathFolder.path()) + "/" + MQtUtil::toMString(pathFolder.baseName());
		EditorComponentGenerator::SetTexture(meshName, path, fnLambert);
		mayaShape.SetUvs();
	}


	//----------------------------------------------------------------------------------------
	void MeshGeneratorController::UpdateShapeEditorComponent(MObject & mFnMesh, MDagModifier & dag, GlobalParameters const& params, DataMesh const& mesh, float const depth)
	{
		const MString meshName(mesh.GetName().c_str());
		const MObject parent = MFnMesh(mFnMesh).parent(0);

		MStringArray result;
		MGlobal::executeCommand("ls -sets", result);
		MSelectionList * setList = new MSelectionList;
		int length = result.length();
		for (int i = 0; i < length; i++)
		{
			setList->add(result[i]);
		}

		MObject mset;
		bool isShadding = false;
		length = setList->length();
		for (auto i = 0; i < length; i++)
		{
			setList->getDependNode(i, mset);
			MFnSet fnSet(mset);
			if (MFnSet::kRenderableOnly == fnSet.restriction())
			{
				if (fnSet.isMember(mFnMesh))
				{
					isShadding = true;
					break;
				}
			}
		}
		delete setList;
		MGlobal::deleteNode(mFnMesh);

		// Shape generation
		MayaMeshConvertor mayaShape(mesh, params.Scale);
		const MObject meshObj = mayaShape.CreateMayaMFnMesh(parent);
		dag.renameNode(meshObj, meshName);
		dag.doIt();

		// Add to shadding group.
		if(isShadding)
		{
			MFnSet tmpSG(mset);
			tmpSG.addMember(meshObj);
		}
		dag.doIt();

		// Set UV
		mayaShape.SetUvs();
	}

#pragma endregion

#pragma region ALGORITHMS

	//----------------------------------------------------------------------------------------
	DataMesh MeshGeneratorController::GenerateDataLinearMesh(ResourceBlockPath const& resourceBlockPath, LayerParameters const* params)
	{
		std::vector<BezierCurve*> curves;
		std::vector<Vector2F*> points;
		boundingBox bounds = boundingBox();

		// read bezier Curves
		for (auto pathRecord : resourceBlockPath.PathRecords)
		{
			if (!pathRecord.IsClosedPath)
			{
				continue;
			}
			BezierCurve* tmpCurve = new BezierCurve();
			tmpCurve->GenerateBezierCurve(pathRecord);
			curves.push_back(tmpCurve);
			points.insert(points.end(), tmpCurve->GetCurve().begin(), tmpCurve->GetCurve().end());
		}

		if ((int(params->LinearParameters.GridOrientation) % 90) != 0)
		{
			bounds.SetOrientation(params->LinearParameters.GridOrientation);
			bounds.GenerateOrientedBoundingBox(points);
		}
		else
		{
			bounds.GenerateBoundingBox(points);
		}

		bounds.DisplayBoundingBox();
		DataMesh newMesh = LinearMesh::GenerateMesh(resourceBlockPath.Name, params->LinearParameters, bounds, curves);
		if(newMesh.GetFacesIndicesCount() == 0)
		{
			newMesh.ClearFaces();
			std::vector<Vector2F> vertices;
			vertices.push_back(bounds.TopLeftPoint());
			vertices.push_back(bounds.TopRightPoint());
			vertices.push_back(bounds.BottomRightPoint());
			vertices.push_back(bounds.BottomLeftPoint());
			newMesh.SetVertices(vertices);
			int indicesFace[]{ 0, 1, 2, 3 };
			newMesh.AddFace(indicesFace, 4);
		}
		return newMesh;
	}

	//----------------------------------------------------------------------------------------
	DataMesh MeshGeneratorController::GenerateDataCurveGridMesh(psd_reader::LayerData const& layer, LayerParameters const* params)
	{
		std::vector<Curve> curves;

		// Read bezier curves
		for (auto pathRecord : layer.PathRecords)
		{
			Curve curve = Curve(pathRecord);
			curves.push_back(curve);
		}

		return CurveMeshGenerator::GenerateMesh(curves, layer.LayerName, params->CurveParameters);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void MeshGeneratorController::ApplyInfluenceLayer(PsdData const& data, std::map<std::string, DataMesh> & meshes, GlobalParameters & params, Progress & progress)
	{
		for (auto& mesh : meshes)
		{
			int index = data.LayerMaskData.GetIndexInfluenceLayer(mesh.second.GetName());
			if (index == -1)
				continue;

			LayerParameters* layerParams = params.GetLayerParameter(mesh.first);
			if (!layerParams->InfluenceActivated)
				continue;

			progress.IncrementProgressBar();

			MaskData maskData;
			maskData.Data = TextureExporter::ConvertToMask(false, data.LayerMaskData.Layers[index], data.HeaderData.Width, data.HeaderData.Height, data.HeaderData.BitsPerPixel);
			maskData.Width = data.HeaderData.Width;
			maskData.Height = data.HeaderData.Height;
			maskData.BytesPerPixel = data.HeaderData.BitsPerPixel / 8;

			InfluenceMesh::SubdivideFaces(mesh.second, maskData, layerParams->InfluenceParameters);
		}
	}

#pragma endregion
}
