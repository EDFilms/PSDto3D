//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file ToolWidget.h
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef TOOLWIDGET_H
#define TOOLWIDGET_H

#include "psd_reader/psdReader.h"
#include "../IControllerUpdate.h"
#include "../parameters.h"
#include "ui_toolWidget.h"
#include "qtProgress.h"
#include <maya/MQtUtil.h>
#include <maya/MString.h>
#include <qmetatype.h>
#include <QListWidgetItem>
#include <vector>

namespace maya_plugin
{
	//----------------------------------------------------------------------------------------------
	class ListFrame : public QFrame
	{
		void mousePressEvent(QMouseEvent* e) override;
	};

	//----------------------------------------------------------------------------------------------
	class ToolWidget : public QDockWidget
	{
		Q_OBJECT
	public:
		ToolWidget(QDockWidget *parent = nullptr);
		~ToolWidget();

		void SetPsdData(psd_reader::PsdData& psdData);
		GlobalParameters& GetParameters();
		Progress & GetProgress();
		void AddController(IControllerUpdate* controller);
		void RemoveController(IControllerUpdate* controller);

		void Refresh();

	public slots:
		void SetSelectedLayers();
		void GenerateMeshes(bool checked);
		void ExportPng(bool checked);
		void GetPath(bool checked);
		void ReloadPath(bool checked);
		void SetDepthModifier(const double value);
		void SetAlgorithm(const int value);
		void SetMeshScale(const int value);
		void SetActiveKeepGroup(const int value);
		void SetAliasPsdName(QString);
		void SetLinearPrecision(const double value);
		void SetGridDirection(const int value);
		void SetMergeDistance(const double & value);
		void SetActiveInfluence(bool value);
		void SetMinPolygonSize(const double & value);
		void SetMaxPolygonSize(const double & value);

	private:
		GlobalParameters Data;
		Progress Progress;
		Ui::DockWidget *Ui;
		std::vector<IControllerUpdate*> Controllers;
		std::map<QListWidgetItem*, std::string> WidgetMap;

		QString ApplyLayerDescription(psd_reader::LayerData& layerData, QVBoxLayout* layout);
		float GetMeshScaleUi();
		float GetDephtUi();
		void UpdateUi();
		void UpdatePanels();
		bool AreSelectedValuesDifferent();
		void UpdateFields();
		void UpdateButtons() const;
		void UpdateActiveLayers();

		LayerParameters* GetLayerParameters(QListWidgetItem* item);
	};
}
#endif // TOOLWIDGET_H
