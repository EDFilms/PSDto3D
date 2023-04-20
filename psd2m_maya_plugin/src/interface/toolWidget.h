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

#include "IControllerUpdate.h"
#include "ui_wrapper.h"
#include "ui_toolWidget.h"

#if defined PSDTO3D_MAYA_VERSION
#include <maya/MTypes.h>
#include <maya/MQtUtil.h>
#include <maya/MString.h>
#else
#include "mayaStub.h"
#endif
typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <qmetatype.h>
#include <QMainWindow.h>
#include <QListWidgetItem>
#include <QTProgress.h>
#include <QTimer.h>
#include <vector>

namespace psd_reader { struct PsdData; } // forward declaration

namespace psd_to_3d
{
	class IPluginOutput;			// forward declaration
	class SceneController;			// forward declaration
	class ProgressAgent;			// forward declaration
	struct GlobalParameters;		// forward declaration
	struct LayerParameters;			// forward declaration
	struct AtlasParameters;			// forward declaration
	class ToolWidgetEventFilter;	// forward declaration
	typedef IPluginController::NotifyStatus NotifyStatus;


	//----------------------------------------------------------------------------------------------
	class ListFrame : public QFrame
	{
		void mousePressEvent(QMouseEvent* e) override;
	};

	//----------------------------------------------------------------------------------------------
	class ToolWidget : public QMainWindow
	{
		Q_OBJECT
	public:
		//ToolWidget(QDockWidget* parent = nullptr, IPluginController* controller = nullptr);
		ToolWidget(QMainWindow* parent = nullptr, IPluginController* controller = nullptr);
		~ToolWidget();

		void Init( IPluginController* controller );
		IPluginOutput& GetOutput();
		SceneController& GetScene();
		GlobalParameters& GetParameters();
		NotifyStatus& GetNotifyStatus();
		ProgressAgent& GetProgress();

		void NotifyCommand();

		bool ExportFilenameSelector(); // prompts for filename, sets global FileExportPath and FileExportName

		// Requests, handled in main thread, if called from another thread
		void RequestRepaint( bool updateDescriptions );

	public slots:
		void OnRepaint( bool updateDescriptions ); // Request event
		void Repaint( bool updateDescriptions );
		void OnExitApp(bool checked=false); // UI event
		void ExitApp(bool checked);
		void OnSetSelectedLayers(); // UI event
		void SetSelectedLayers();
		void OnLayersSelectAll(); // UI event
		void LayersSelectAll();
		void OnLayersSelectNone(); // UI event
		void LayersSelectNone(); // UI event
		void OnGenerateMeshes(bool checked=false); // UI event
		void GenerateMeshes(bool checked);
		void OnExportPng(bool checked=false); // UI event
		void ExportPng(bool checked);
		void OnExportSelected(bool checked=false); // UI event
		void ExportSelected(bool checked);
		// derelict, no longer used
		//void OnExportAll(bool checked=false); // UI event ... standalone only
		//void ExportAll(bool checked);
		void OnHelpButton(bool checked=false); // UI event
		void HelpButton(bool checked);
		void OnImportPSD(bool checked=false); // UI event
		void ImportPSD(bool checked);
		void OnEditedImportPath(); // UI event
		void NotifyImportPSD(QString);
		void OnPickOutputPath(bool checked); // UI event
		void PickOutputPath(bool checked);
		void OnSetOutputPath(QString); // UI event
		void SetOutputPath(QString);
		void OnReloadPSD(bool checked=false); // UI event
		void ReloadPSD(bool checked);
		void OnSetExportName(); // UI event
		void SetExportName(QString value);
		void OnEditExportPath(const QString value); // UI event
		void EditExportPath(const QString value);
		void OnSetExportPath(); // UI event
		void SetExportPath(QString value);
		void OnExportPathSelector(bool checked=false);
		void ExportPathSelector(bool checked);
		void OnSetTextureProxy(const int value); // UI event
		void SetTextureProxy(const int value);
		void OnSetDepthModifier(const double value); // UI event
		void SetDepthModifier(const double value);
		void OnSetAlgorithm(const int value); // UI event
		void SetAlgorithm(const int value);
		void OnSetAtlasIndex(const int value); // UI event
		void SetAtlasIndex(const int value);
		void OnSetAtlasName(QString); // UI event
		void SetAtlasName(QString);
		void OnSetAtlasAlgo(const int value); // UI event
		void SetAtlasAlgo(const int value);
		void OnSetAtlasOverride(bool checked); // UI event
		void SetAtlasOverride(bool checked);
		void OnSetAtlasSize(const int value); // UI event
		void SetAtlasSize(const int value);
		void OnSetAtlasPadding(const int value); // UI event
		void SetAtlasPadding(const int value);
		void OnAtlasSelector(bool checked); // UI event
		void AtlasSelector(bool checked);
		void OnAtlasCleanup(bool checked); // UI event
		void AtlasCleanup(bool checked);
		void OnSetTextureCropIndex(const int value); // UI event
		void SetTextureCropIndex(const int value);
		void OnSetMeshScale(const double value); // UI event
		void SetMeshScale(const double value);
		void OnSetWriteMode(const int value); // UI event
		void SetWriteMode(const int value);
		void OnSetWriteLayout(const int value); // UI event
		void SetWriteLayout(const int value);
		void OnSetActiveKeepGroup(const int value); // UI event
		void SetActiveKeepGroup(const int value);
		void OnSetAliasPsdName(QString); // UI event
		void SetAliasPsdName(QString);
		void OnSetBillboardAlphaThresh(const double value); // UI event
		void SetBillboardAlphaThresh(const double value);
		void OnSetLinearPrecision(const double value); // UI event
		void SetLinearPrecision(const double value);
		void OnSetDelaunayOuterDetail(const double value); // UI event
		void SetDelaunayOuterDetail(const double value);
		void OnSetDelaunayInnerDetail(const double value); // UI event
		void SetDelaunayInnerDetail(const double value);
		void OnSetDelaunayFalloffDetail(const double value); // UI event
		void SetDelaunayFalloffDetail(const double value);
		void OnSetMergeEnabled(bool value); // UI event
		void SetMergeEnabled(bool value);
		void OnSetMergeDistance(const double & value); // UI event
		void SetMergeDistance(const double & value);
		void OnSetEnableInfluence(bool value); // UI event
		void SetEnableInfluence(bool value);
		void OnSetMinPolygonSize(const double & value); // UI event
		void SetMinPolygonSize(const double & value);
		void OnSetMaxPolygonSize(const double & value); // UI event
		void SetMaxPolygonSize(const double & value);

		bool dragCheck( const QMimeData* mimeData, QString& filename_out );
		void dragEnterEvent(QDragEnterEvent *event) override;
		void dragMoveEvent(QDragMoveEvent *event) override;
		void dragLeaveEvent(QDragLeaveEvent *event) override;
		void dropEvent(QDropEvent *event) override;

	private:
		// TODO: GlobalParameters should be member of class PluginController
		IPluginController* Controller;
		ProgressAgent Progress;
		Ui::DockWidget *Ui;
		//std::vector<IPluginController*> Controllers;
		std::map<QListWidgetItem*, int> WidgetMap; // widget to layer index
		ToolWidgetEventFilter* EventFilter;
		int SilenceUi;

		void UpdateLayers(bool allWidgets, bool updateAtlas);
		void ApplyLayerDescription(ListFrame* frame, int layerIndex);
		void UpdateLayerDescription(int layerIndex);
		int GetTextureProxyUi();
		float GetMeshScaleUi();
		float GetDepthUi();
		int GetWriteModeUi();
		int GetWriteLayoutUi();
		void UpdateLocalization(); // translate all UI strings
		void UpdateUi(bool updateLayout=true);
		void UpdatePanels();
		bool AreSelectedValuesDifferent();
		void UpdateFields();
		void UpdateButtons() const;
		void UpdateActiveLayers();
		void UpdateAtlasList();

		int GetGenerationAlgoEnumToComboBox(int algo);  // enum to combo box index; algo is type LayerParameters::Algorithm
		int GetGenerationAlgoComboBoxToEnum(int index); // combo box index to algo; algo is type LayerParameters::Algorithm
		int GetAtlasAlgoEnumToComboBox(int algo); // enum to combo box index; algo is type rbp::MaxRectsBinPack::FreeRectChoiceHeuristic
		int GetAtlasAlgoComboBoxToEnum(int index); // combo box index to enum; algo is type rbp::MaxRectsBinPack::FreeRectChoiceHeuristic

		int GetLayerIndex(QListWidgetItem* item);
		LayerParameters& GetLayerParameters(QListWidgetItem* item);
		LayerParameters& GetLayerParameters(int layerIndex);
		AtlasParameters& GetAtlasParameters(int atlasIndex);
	};

	//----------------------------------------------------------------------------------------------
	class ToolWidgetRepaintRequest : public QObject
	{
		Q_OBJECT
		ToolWidget* Widget;
	public:
		ToolWidgetRepaintRequest(ToolWidget* widget) : QObject(nullptr), Widget(widget)
		{ connect(this, SIGNAL( RequestRepaint(bool) ), widget, SLOT( OnRepaint(bool) ) ); }
		void Emit(bool b) { emit RequestRepaint(b); }
	signals:
		void RequestRepaint(bool);
	};

	//----------------------------------------------------------------------------------------------
	class ListWidgetRepaintRequest : public QObject
	{
		Q_OBJECT
		QListWidget* Widget;
	public:
		ListWidgetRepaintRequest(QListWidget* widget) : QObject(nullptr), Widget(widget)
		{ connect( this, SIGNAL( RequestRepaint(bool) ), widget, SLOT( OnRepaint(bool) ) ); }
		void Emit(bool b) { emit RequestRepaint(b); }
	signals:
		void RequestRepaint(bool);
	};

	//----------------------------------------------------------------------------------------------
	class ToolWidgetEventFilter : public QObject
	{
		Q_OBJECT
		ToolWidget* Widget;
	public:
		ToolWidgetEventFilter(ToolWidget* widget) : QObject(nullptr), Widget(widget) {}
		bool eventFilter(QObject *object, QEvent *event);
	};

	//--------------------------------------------------------------------------------------------------------------------------------------
	class PingTimer : public QObject
	{
		Q_OBJECT
	public:
		typedef void (*PingFunc)( void* param );
		PingTimer( PingFunc pingFunc, void* pingParam )
		: suspend(0), pingFunc(pingFunc), pingParam(pingParam)
		{
			// create a timer
			timer = new QTimer(this);
			// setup signal and slot
			connect(timer, SIGNAL(timeout()), this, SLOT(PingTimerSlot()));
			// start timer
			timer->start(16); // ping at about 60fps
		}

		class SuspendGuard
		{
		public:
			SuspendGuard(PingTimer& parent) :parent(parent) { parent.suspend++; }
			~SuspendGuard() { parent.suspend--; }
			PingTimer& parent;
		};

		int suspend;
		QTimer* timer;
		PingFunc pingFunc;
		void* pingParam;

	public slots:
		void PingTimerSlot()
		{
			if( suspend==0 )
				pingFunc(pingParam);
		}
	};

}
#endif // TOOLWIDGET_H
