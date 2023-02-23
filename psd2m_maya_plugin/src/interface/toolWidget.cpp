//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2016, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file ToolWidget.cpp
//  @author Fabien Raspail
//  @date 11-11-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#include "toolWidget.h"

#include "IPluginOutput.h"
#include "parameters.h"
#include "scene_controller/sceneController.h"
#include "psd_reader/psdReader.h"
#include "toolWidgetLocalization.h"

#include "MaxRectsBinPack.h" // for GetPackingAlgoEnumToComboBox

#if defined PSDTO3D_MAYA_VERSION
#include <maya/MGlobal.h>
#else
#include "mayaStub.h"
#endif

#include <Windows.h> // for OPENFILENAMEW
#include <commdlg.h> // for OPENFILENAMEW ... why isn't Windows.h enough?

typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QListWidgetItem>
#include <QtGui/QMouseEvent>
#include <QDesktopServices>
#include <QMimeData>
#include <set>

// window default size for aesthetic purposes, independent of screen resolution
// based on a multiple of the size of the psdSelectorBtn, which never scales as window is resized
#define DEFAULT_WIDTH_HEURISTIC 9.5f //26.8f
#define DEFAULT_HEIGHT_HEURISTIC 34.3f //44.0f

namespace psd_to_3d
{
	typedef IPluginController::NotifyStatus NotifyStatus;

#pragma region CONSTRUCTORS

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
	void MessageRelay(QtMsgType type, const QMessageLogContext& context, const QString& msg)
#else
	void MessageRelay(QtMsgType type, const char *msg)
#endif
	{
		switch (type) {
		case QtDebugMsg:
			MGlobal::displayInfo(MQtUtil::toMString(msg));
			break;
		case QtWarningMsg:
			MGlobal::displayWarning(MQtUtil::toMString(msg));
			break;
		case QtCriticalMsg:
			MGlobal::displayError(MQtUtil::toMString(msg));
			break;
		case QtFatalMsg:
			MGlobal::displayError(MQtUtil::toMString(msg));
			abort();
		}
	}

	void ListFrame::mousePressEvent(QMouseEvent* e)
	{
		// Ignore event so that the QListWidget receives it instead
		e->ignore();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	ToolWidget::ToolWidget(QMainWindow *parent, IPluginController* controller) 
		: QMainWindow(parent), Ui(new Ui::DockWidget), EventFilter(new ToolWidgetEventFilter(this)), SilenceUi(0)
	{
		this->Controllers.push_back(controller);

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
		qInstallMessageHandler(MessageRelay);
#else
		qInstallMsgHandler(MessageRelay);
#endif
		// Fill the current instance with the GUI information
		Ui->setupUi(this);

		if( !IsFullVersion )
		{
			this->setWindowTitle("PSD to 3D Lite Editor");
		}

		// translate all UI strings
		UpdateLocalization();

		// buttons and menu commands
		connect(Ui->actionFile_Exit, SIGNAL(triggered()),this,SLOT(OnExitApp()) );

		connect(Ui->psdImportBtn, SIGNAL(clicked(bool)), this, SLOT(OnImportPSD(bool)));
		connect(Ui->actionFile_ImportPSD, SIGNAL(triggered()),this,SLOT(OnImportPSD()) );
		connect(Ui->actionFile_Exit, SIGNAL(triggered()),this,SLOT(OnExitApp()) );

		connect(Ui->psdReloadBtn, SIGNAL(clicked(bool)), this, SLOT(OnReloadPSD(bool)));
		connect(Ui->actionFile_Reload, SIGNAL(triggered()), this, SLOT(OnReloadPSD()));
		if( IsFbxVersion )
		{
			// filename allows alphanumeric, plus underscore, hyphen, dot, and space
			QRegExp rx_filename("^[\\w\\-. ]+$"); // "^[\w\-. ]+$" where \w is equivalent of [0-9a-zA-Z_]
			QValidator *validator = new QRegExpValidator(rx_filename, this);
			Ui->psdExportNameLineEdit->setValidator(validator);
			connect(Ui->psdExportNameLineEdit, SIGNAL(editingFinished()), this, SLOT(OnSetExportName()));
			connect(Ui->psdExportPathLineEdit, SIGNAL(textEdited(QString)), this, SLOT(OnEditExportPath(QString)));
			connect(Ui->psdExportPathLineEdit, SIGNAL(editingFinished()), this, SLOT(OnSetExportPath()));
			connect(Ui->psdExportPathBtn, SIGNAL(clicked(bool)), this, SLOT(OnExportPathSelector(bool)));
		}

		connect(Ui->actionHelp_Tutorials, SIGNAL(triggered()), this, SLOT(OnHelpButton()));

		connect(Ui->generateMeshBtn, SIGNAL(clicked(bool)), this, SLOT(OnGenerateMeshes(bool)));
		connect(Ui->generatePngBtn, SIGNAL(clicked(bool)), this, SLOT(OnExportPng(bool)));
		connect(Ui->generateBothBtn, SIGNAL(clicked(bool)), this, SLOT(OnExportSelected(bool)));
		connect(Ui->actionGenerate_GenerateMesh, SIGNAL(triggered()), this, SLOT(OnGenerateMeshes()));
		connect(Ui->actionGenerate_GeneratePng, SIGNAL(triggered()), this, SLOT(OnExportPng()));
		connect(Ui->actionGenerate_GenerateBoth, SIGNAL(triggered()), this, SLOT(OnExportSelected()));

		// global settings
		connect(Ui->textureProxyComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetTextureProxy(int)));
		connect(Ui->depthModifierField, SIGNAL(valueChanged(double)), this, SLOT(OnSetDepthModifier(const double &)));
		connect(Ui->meshScaleSpinner, SIGNAL(valueChanged(double)), this, SLOT(OnSetMeshScale(const double &)));
		if( IsFbxVersion )
		{
			connect(Ui->writeModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetWriteMode(int)));
			connect(Ui->writeLayoutComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetWriteLayout(int)));
		}
		if( IsMayaVersion )
		{
			//connect(Ui->groupStructureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetActiveKeepGroup(int))); // no longer supported
		}
		connect(Ui->customGroupNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(OnSetAliasPsdName(QString)));

		// layerList
		connect(Ui->layerList, SIGNAL(itemSelectionChanged()), this, SLOT(OnSetSelectedLayers()));
		connect(Ui->layersSelectAllBtn, SIGNAL(clicked(bool)), this, SLOT(OnLayersSelectAll()));
		connect(Ui->layersSelectNoneBtn, SIGNAL(clicked(bool)), this, SLOT(OnLayersSelectNone()));

		// algorithm
		// TODO: support this switch at runtime instead of using preprocessor flag
		connect(Ui->generationAlgoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetAlgorithm(const int &)));
		Ui->generationAlgoComboBox->removeItem(4); // remove last item, dev comment "MUST UPDATE TOOLWIDGET CONSTRUCTOR IF CHANGING THIS ITEM LIST"
		if( !IsFullVersion )
		{
			// remove Delaunay and Vector if this is the Lite version
			// must happen after items are localized, as localization assumed original order
			Ui->generationAlgoComboBox->removeItem(3); // remvoe last first, avoid mangling order
			Ui->generationAlgoComboBox->removeItem(2);
		}
		if( !IsMayaVersion )
		{
			// remove Linear if this is not the Maya plugin; leave Delaunay, Vector, and Billboard for FBX and Unreal
			// must happen after items are localized, as localization assumed original order
			Ui->generationAlgoComboBox->removeItem(0);
		}

		if( IsAtlasVersion )
		{
			// atlas settings
			// TODO: support this switch at runtime instead of using preprocessor flag
			connect(Ui->textureAtlasGroupComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetAtlasIndex(const int &)));
			connect(Ui->textureAtlasGroupComboBox, SIGNAL(editTextChanged(QString)), this, SLOT(OnSetAtlasName(QString)));
			connect(Ui->textureAtlasAlgoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetAtlasAlgo(const int &)));
			connect(Ui->textureAtlasOverrideChk, SIGNAL(clicked(bool)), this, SLOT(OnSetAtlasOverride(bool)));
			connect(Ui->textureAtlasSizeSpin, SIGNAL(valueChanged(int)), this, SLOT(OnSetAtlasSize(const int&)));
			connect(Ui->textureAtlasPaddingSpin, SIGNAL(valueChanged(int)), this, SLOT(OnSetAtlasPadding(const int&)));
			connect(Ui->textureAtlasSelectorBtn, SIGNAL(clicked(bool)), this, SLOT(OnAtlasSelector(bool)));
			connect(Ui->textureAtlasCleanupBtn, SIGNAL(clicked(bool)), this, SLOT(OnAtlasCleanup(bool)));
			connect(Ui->textureOutputSizeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetTextureCropIndex(const int &)));
		}

		// billboard settings
		connect(Ui->billboardAlphaThreshField, SIGNAL(valueChanged(double)), this, SLOT(OnSetBillboardAlphaThresh(const double &)));

		// delaunay settings
		if( IsFullVersion )
		{
			connect(Ui->delaunayOuterDetailField, SIGNAL(valueChanged(double)), this, SLOT(OnSetDelaunayOuterDetail(const double &)));
			connect(Ui->delaunayInnerDetailField, SIGNAL(valueChanged(double)), this, SLOT(OnSetDelaunayInnerDetail(const double &)));
			connect(Ui->delaunayFalloffDetailField, SIGNAL(valueChanged(double)), this, SLOT(OnSetDelaunayFalloffDetail(const double &)));
		}

		// linear settings
		if( IsMayaVersion )
		{
			connect(Ui->linearPrecisionField, SIGNAL(valueChanged(double)), this, SLOT(OnSetLinearPrecision(const double &)));
			//connect(Ui->gridOrientationDial, SIGNAL(valueChanged(int)), this, SLOT(SetGridDirection(int)));
		}

		// curve settings
		if( IsFullVersion )
		{
			// TODO: support this switch at runtime instead of using preprocessor flag
			connect(Ui->mergeVerticesChk, SIGNAL(clicked(bool)), this, SLOT(OnSetMergeEnabled(bool)));
			connect(Ui->mergeVertexDistanceField, SIGNAL(valueChanged(double)), this, SLOT(OnSetMergeDistance(const double &)));
		}

		// influence settings
		if( IsInfluenceSupported )
		{
			connect(Ui->influenceLayerChk, SIGNAL(clicked(bool)), this, SLOT(OnSetActiveInfluence(bool)));
			connect(Ui->minimumPolygonSizeField, SIGNAL(valueChanged(double)), this, SLOT(OnSetMinPolygonSize(const double &)));
			connect(Ui->maximumPolygonSizeField, SIGNAL(valueChanged(double)), this, SLOT(OnSetMaxPolygonSize(const double &)));
		}

		// custom group name string validator
		{
			QRegExp rx("^\\S+$");
			QValidator *validator = new QRegExpValidator(rx, this);
			Ui->customGroupNameLineEdit->setValidator(validator);
		}

		if( IsAtlasVersion )
		{
			Ui->textureAtlasGroupComboBox->setInsertPolicy(QComboBox::NoInsert);
			Ui->textureAtlasGroupComboBox->setEditable(false); // Updated later setting atlas index
			Ui->textureAtlasAlgoComboBox->setInsertPolicy(QComboBox::NoInsert);
			Ui->textureAtlasAlgoComboBox->setEditable(false); // Updated later setting atlas index
			Ui->textureOutputSizeComboBox->setInsertPolicy(QComboBox::NoInsert);
			Ui->textureOutputSizeComboBox->setEditable(false); // Updated later setting atlas index
		}

		//Ui->helpButtonStandalone.setIcon(this->style().standardIcon(getattr(QStyle, 'SP_TitleBarContextHelpButton')));

		Ui->progressBar->setVisible(false); // hide the progress bar in favor of progress popup window
		this->Progress.SetProgressBar(Ui->progressBar); // initialize the progress popup window

		// filter out double-click event on the title bar;
		// avoids visual glitch when window tries to becomed docked with no dockable location available
		this->installEventFilter(EventFilter); // override NonClientAreaMouseButtonDblClick

		// set default window for aesthetic purposes according to heuristic
		int preferredWidth = Ui->psdImportBtn->width() * DEFAULT_WIDTH_HEURISTIC;
		int preferredHeight = Ui->psdImportBtn->height() * DEFAULT_HEIGHT_HEURISTIC;
		this->resize( QSize(preferredWidth,preferredHeight) ); // this->minimumSizeHint() // this->sizeHint()
		UpdateUi();

		int buttonWidth = Ui->textureAtlasSelectorBtn->width();
		//Ui->psdImportBtn->setMinimumWidth( buttonWidth );
		//Ui->psdReloadBtn->setMinimumWidth( buttonWidth );


		QGridLayout* layout = Ui->middleSection;
		layout->invalidate();
		layout->update();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	ToolWidget::~ToolWidget()
	{
		disconnect(Ui->actionFile_Exit, SIGNAL(triggered()),this,SLOT(OnExitApp()) );

		disconnect(Ui->psdImportBtn, SIGNAL(clicked(bool)), this, SLOT(OnImportPSD(bool)));
		disconnect(Ui->actionFile_ImportPSD, SIGNAL(triggered()),this,SLOT(OnImportPSD()) );
		disconnect(Ui->actionFile_Exit, SIGNAL(triggered()),this,SLOT(OnExitApp()) );
		if( IsFbxVersion )
		{
			disconnect(Ui->psdSelectorLineEdit, SIGNAL(editingFinished()), this, SLOT(OnEditedImportPath()));
		}

		disconnect(Ui->psdReloadBtn, SIGNAL(clicked(bool)), this, SLOT(OnReloadPSD(bool)));
		disconnect(Ui->actionFile_Reload, SIGNAL(triggered()), this, SLOT(OnReloadPSD()));
		disconnect(Ui->psdExportNameLineEdit, SIGNAL(editingFinished()), this, SLOT(OnSetExportName()));
		disconnect(Ui->psdExportPathLineEdit, SIGNAL(textEdited(QString)), this, SLOT(OnEditExportPath(QString)));
		disconnect(Ui->psdExportPathLineEdit, SIGNAL(editingFinished()), this, SLOT(OnSetExportPath()));
		disconnect(Ui->psdExportPathBtn, SIGNAL(clicked(bool)), this, SLOT(OnExportPathSelector(bool)));
		disconnect(Ui->actionHelp_Tutorials, SIGNAL(triggered()), this, SLOT(OnHelpButton()));

		disconnect(Ui->generateMeshBtn, SIGNAL(clicked(bool)), this, SLOT(OnGenerateMeshes(bool)));
		disconnect(Ui->generatePngBtn, SIGNAL(clicked(bool)), this, SLOT(OnExportPng(bool)));
		disconnect(Ui->generateBothBtn, SIGNAL(clicked(bool)), this, SLOT(OnExportSelecetd(bool)));
		disconnect(Ui->actionGenerate_GenerateMesh, SIGNAL(triggered()), this, SLOT(OnGenerateMeshes()));
		disconnect(Ui->actionGenerate_GeneratePng, SIGNAL(triggered()), this, SLOT(OnExportPng()));
		disconnect(Ui->actionGenerate_GenerateBoth, SIGNAL(triggered()), this, SLOT(OnExportSelected()));

		disconnect(Ui->textureProxyComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetTextureProxy(int)));
		disconnect(Ui->depthModifierField, SIGNAL(valueChanged(double)), this, SLOT(OnSetDepthModifier(const double &)));
		disconnect(Ui->meshScaleSpinner, SIGNAL(valueChanged(double)), this, SLOT(OnSetMeshScale(const double &)));
		if( IsFbxVersion )
		{
			disconnect(Ui->writeModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetWriteMode(int)));
			disconnect(Ui->writeLayoutComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetLayoutMode(int)));
		}
		if( IsMayaVersion )
		{
			//disconnect(Ui->groupStructureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetActiveKeepGroup(int))); // no longer supported
		}
		disconnect(Ui->customGroupNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(OnSetAliasPsdName(QString)));
		disconnect(Ui->layerList, SIGNAL(itemSelectionChanged()), this, SLOT(OnSetSelectedLayers()));
		disconnect(Ui->layersSelectAllBtn, SIGNAL(clicked(bool)), this, SLOT(OnLayersSelectAll()));
		disconnect(Ui->layersSelectNoneBtn, SIGNAL(clicked(bool)), this, SLOT(OnLayersSelectNone()));
		disconnect(Ui->generationAlgoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetAlgorithm(const int &)));
		if( IsAtlasVersion )
		{
			// TODO: support this switch at runtime instead of using preprocessor flag
			disconnect(Ui->textureAtlasGroupComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetAtlasIndex(const int &)));
			disconnect(Ui->textureAtlasGroupComboBox, SIGNAL(editTextChanged(QString)), this, SLOT(OnSetAtlasName(QString)));
			disconnect(Ui->textureAtlasAlgoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetAtlasAlgo(const int &)));
			disconnect(Ui->textureAtlasOverrideChk, SIGNAL(clicked(bool)), this, SLOT(OnSetAtlasOverride(bool)));
			disconnect(Ui->textureAtlasSizeSpin, SIGNAL(valueChanged(int)), this, SLOT(OnSetAtlasSize(const int&)));
			disconnect(Ui->textureAtlasPaddingSpin, SIGNAL(valueChanged(int)), this, SLOT(OnSetAtlasPadding(const int&)));
			
			disconnect(Ui->textureAtlasSelectorBtn, SIGNAL(clicked(bool)), this, SLOT(OnAtlasSelector(bool)));
			disconnect(Ui->textureAtlasCleanupBtn, SIGNAL(clicked(bool)), this, SLOT(OnAtlasCleanup(bool)));
			disconnect(Ui->textureOutputSizeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSetTextureCropIndex(const int &)));
		}

		if( IsFullVersion )
		{
			disconnect(Ui->delaunayOuterDetailField, SIGNAL(valueChanged(double)), this, SLOT(OnSetDelaunayOuterDetail(const double &)));
			disconnect(Ui->delaunayInnerDetailField, SIGNAL(valueChanged(double)), this, SLOT(OnSetDelaunayInnerDetail(const double &)));
			disconnect(Ui->delaunayFalloffDetailField, SIGNAL(valueChanged(double)), this, SLOT(OnSetDelaunayFalloffDetail(const double &)));
		}

		if( IsMayaVersion )
		{
 			disconnect(Ui->linearPrecisionField, SIGNAL(valueChanged(double)), this, SLOT(OnSetLinearPrecision(const double &)));
			//disconnect(Ui->gridOrientationDial, SIGNAL(valueChanged(int)), this, SLOT(SetGridDirection(int)));
		}

		if( IsFullVersion )
		{
			// TODO: support this switch at runtime instead of using preprocessor flag
			disconnect(Ui->mergeVerticesChk, SIGNAL(valueChanged(bool)), this, SLOT(OnSetMergeEnabled(bool)));
			disconnect(Ui->mergeVertexDistanceField, SIGNAL(valueChanged(double)), this, SLOT(OnSetMergeDistance(const double &)));
		}
		disconnect(Ui->minimumPolygonSizeField, SIGNAL(valueChanged(double)), this, SLOT(OnSetMinPolygonSize(const double &)));
		disconnect(Ui->maximumPolygonSizeField, SIGNAL(valueChanged(double)), this, SLOT(OnSetMaxPolygonSize(const double &)));

		delete Ui;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetPsdData(psd_reader::PsdData & psdData)
	{
		QString sstr;
		sstr.append(std::to_string(psdData.HeaderData.Width).c_str());
		sstr.append(" x ");
		sstr.append(std::to_string(psdData.HeaderData.Height).c_str());
		std::string sstrCheck = sstr.toStdString(); // for debugging
		Ui->selectedPsdSizeValue->setText(sstr);

		// Populate list of atlas names (groups) first, before UI elements which require it
		UpdateAtlasList();

		// Fill model value
		Ui->layerList->clear();
		this->WidgetMap.clear(); // TODO: Does this leak widgets?

		Ui->layerList->setStyleSheet("QListView::item:selected { background: palette(Highlight) }");

		for (int layerIndex = 0; layerIndex < psdData.LayerMaskData.LayerCount(); layerIndex++)
		{
			psd_reader::LayerData& layer = psdData.LayerMaskData.Layers[layerIndex];
			// folder or endfolder separator
			if (layer.Type > psd_reader::TEXTURE_LAYER) continue;

			QListWidgetItem* widgetItem = new QListWidgetItem();
			this->WidgetMap.try_emplace(widgetItem, layerIndex);
			Ui->layerList->addItem(widgetItem);

			ListFrame* frame = new ListFrame();
			frame->setFrameShape(QFrame::WinPanel);
			frame->setFrameShadow(QFrame::Raised);

			ApplyLayerDescription(frame, psdData, layerIndex);

			widgetItem->setSizeHint(frame->sizeHint());
			Ui->layerList->setItemWidget(widgetItem, frame);
		}
		// crashes // Ui->layerList->layout()->invalidate();
		// crashes // Ui->layerList->layout()->update();

		// Update UI
		this->Ui->psdReloadBtn->setEnabled(true);
		this->Ui->psdExportNameLineEdit->setEnabled(true);
		this->Ui->psdExportPathLineEdit->setEnabled(true);
		this->Ui->psdExportPathBtn->setEnabled(true);
		this->Ui->actionFile_Reload->setEnabled(true);

		this->Ui->generateMeshBtn->setEnabled(true);
		this->Ui->generatePngBtn->setEnabled(true);
		this->Ui->generateBothBtn->setEnabled(true);
		this->Ui->actionGenerate_GenerateMesh->setEnabled(true);
		this->Ui->actionGenerate_GeneratePng->setEnabled(true);
		this->Ui->actionGenerate_GenerateBoth->setEnabled(true);

		this->Ui->psdSelectorLineEdit->setText( this->GetParameters().FileImportFilepath() );
		this->Ui->psdExportPathLineEdit->setText( this->GetParameters().FileExportPath );
		this->Ui->psdExportNameLineEdit->setText( this->GetParameters().FileExportName );

		UpdateUi();
	}

#pragma endregion

#pragma region ACCESSORS

	//--------------------------------------------------------------------------------------------------------------------------------------
	IPluginOutput& ToolWidget::GetOutput()
	{
		// Support for only one controller, currently
		// Support for many controllers would require refactor
		return this->Controllers[0]->GetOutput();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	SceneController& ToolWidget::GetScene()
	{
		// Support for only one controller, currently
		// Support for many controllers would require refactor
		return this->Controllers[0]->GetScene();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	GlobalParameters& ToolWidget::GetParameters()
	{
		// Support for only one controller, currently
		// Support for many controllers would require refactor
		return this->Controllers[0]->GetScene().GetGlobalParameters();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	IPluginController::NotifyStatus& ToolWidget::GetNotifyStatus()
	{
		// Support for only one controller, currently
		// Support for many controllers would require refactor
		return this->Controllers[0]->GetNotifyStatus();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	ProgressAgent& ToolWidget::GetProgress()
	{
		return this->Progress;
	}

#pragma endregion

#pragma region SLOTS

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetSelectedLayers() // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		SetSelectedLayers();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetSelectedLayers()
	{
		UpdateUi();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnLayersSelectAll() // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		LayersSelectAll();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::LayersSelectAll()
	{
#if MAYA_API_VERSION>=20220000
		for( int i=0; i<Ui->layerList->count(); i++ )
			Ui->layerList->item(i)->setSelected( true );
#else
		for( int i=0; i<Ui->layerList->count(); i++ )
			Ui->layerList->setItemSelected( Ui->layerList->item(i), true );
#endif

		UpdateUi();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnLayersSelectNone() // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		LayersSelectNone();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::LayersSelectNone()
	{
#if MAYA_API_VERSION>=20220000
		for( int i=0; i<Ui->layerList->count(); i++ )
			Ui->layerList->item(i)->setSelected( false );
#else
		for( int i=0; i<Ui->layerList->count(); i++ )
			Ui->layerList->setItemSelected( Ui->layerList->item(i), false );
#endif

		UpdateUi();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnGenerateMeshes(bool b) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		GenerateMeshes(b);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::GenerateMeshes(bool) //unused param
	{
		if (this->GetParameters().FileImportFilename.isEmpty()) return;

		this->GetNotifyStatus().ExportMesh = true;
		this->GetNotifyStatus().ExportPng = false;
		this->GetNotifyStatus().ExportAll = false; // only export selected layers
		NotifyCommand(); // perform the export
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnExportPng(bool b) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		ExportPng(b);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::ExportPng(bool)
	{
		if (this->GetParameters().FileImportFilename.isEmpty()) return;

		this->GetNotifyStatus().ExportMesh = false;
		this->GetNotifyStatus().ExportPng = true;
		this->GetNotifyStatus().ExportAll = false; // only export selected layers
		NotifyCommand(); // perform the export
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnExportSelected(bool b) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		ExportSelected(b);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::ExportSelected(bool) //unused param
	{
		if (this->GetParameters().FileImportFilename.isEmpty()) return;
	
		this->GetNotifyStatus().ExportMesh = true;
		this->GetNotifyStatus().ExportPng = true;
		this->GetNotifyStatus().ExportAll = false; // only export selected layers
		NotifyCommand(); // perform the export
		UpdateLayers(true,false); // refreshed layer descriptions to remove stale markers
		UpdateUi(); // update UI with refreshed layer descriptiosn
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// derelict, no longer used
	//void ToolWidget::OnExportAll(bool b) // UI event
	//{
	//	if( SilenceUi ) return;  // avoid deadlock
	//	ExportAll(b);
	//}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// derelict, no longer used
	//void ToolWidget::ExportAll(bool /* checked */)
	//{
	//	if (this->GetParameters().FileImportPath.isEmpty()) return;
	//
	//	this->GetNotifyStatus().ExportMesh = true;
	//	this->GetNotifyStatus().ExportPng = true;
	//	this->GetNotifyStatus().ExportAll = true; // export both selected and nonselected layers
	//	NotifyCommand(); // perform the export
	//	UpdateLayers(true,false); // refreshed layer descriptions to remove stale markers
	//	UpdateUi(); // update UI with refreshed layer descriptiosn
	//}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnHelpButton(bool b) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		HelpButton(b);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::HelpButton(bool /* checked */)
	{
		const char* help_url = util::LocalizeString( IDC_MAIN, IDS_HELP_URL );
		QDesktopServices::openUrl(QUrl(help_url));
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
    void ToolWidget::OnImportPSD(bool b) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		ImportPSD(b);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
    void ToolWidget::ImportPSD(bool /* checked */)
	{
		this->GetParameters().Prefs.Fetch();
		QString importDir = this->GetParameters().Prefs.FileImportPath;
		const auto path = QFileDialog::getOpenFileName(this, tr("Load Psd"), importDir, tr("psd file (*.psd)")); // TODO: Localize this

		NotifyImportPSD(path);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
    void ToolWidget::OnEditedImportPath() // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock

		// only apply change if different
		QFileInfo newPath( Ui->psdSelectorLineEdit->text() );
		QFileInfo oldPath( this->GetParameters().FileImportFilepath() );
		if( newPath != oldPath )
		{
			NotifyImportPSD( newPath.absoluteFilePath() );
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
    void ToolWidget::NotifyImportPSD(QString path)
	{
		char pathCheck[1024];
		sprintf_s( pathCheck, 1024, path.toUtf8().data() ); // for debugging
		QFileInfo fileInfo(path);
		if (path.isEmpty() ) return;

		if( fileInfo.exists() && !fileInfo.isDir() )
		{
			// update file parameters
			this->GetNotifyStatus().FileImportRequest = true;
			this->GetNotifyStatus().FileImportFilepath = fileInfo.absoluteFilePath().toUtf8().data();

			NotifyCommand();
		}
		// else not a valid path; don't the parameters or UI enables,
		// ignore the error condition, and keep the old parameters and UI state unchanged
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
    void ToolWidget::OnPickOutputPath(bool b) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		PickOutputPath(b);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
    void ToolWidget::PickOutputPath(bool /* checked */)
	{
		// Use the global default output path, instead of the current file output path, when launching dialog
		this->GetParameters().Prefs.Fetch();
		QString dir = this->GetParameters().Prefs.FileExportPath;

		QFileInfo qtFilePath(dir);
		QFileInfo paramsPath = qtFilePath.path() + "/" + qtFilePath.baseName() + "/";
		QString path = paramsPath.path();
		path = QFileDialog::getExistingDirectory( this, tr("Save Fbx"), path ); // TODO: Localize this

		SetOutputPath(path);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
    void ToolWidget::OnSetOutputPath(QString path) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		SetOutputPath(path);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
    void ToolWidget::SetOutputPath(QString path)
	{
		QFileInfo fi(path);
		if (path.isEmpty()) return;

		// Update file parameters
		this->GetParameters().FileExportPath = path;

		// Update persistent global defaults
		this->GetParameters().Prefs.FileExportPath = path;
		this->GetScene().SaveValuesToJson(); // write user settings to disk

		// Update UI
		NotifyCommand();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnReloadPSD(bool b) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		ReloadPSD(b);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::ReloadPSD(bool /* checked */)
	{
		QString filepath = this->GetParameters().FileImportFilepath();
		QFileInfo fi( filepath );
		if(!fi.exists())
		{
			QMessageBox messageBox;
			messageBox.critical(0, "Error", "File is missing, please use 'Import...' to specify the path of the file!"); // TODO: Localize this
			messageBox.setFixedSize(500, 200);
			return;
		}
		this->GetNotifyStatus().FileImportFilepath = filepath.toUtf8().data();
		this->GetNotifyStatus().FileImportRequest = true;
		NotifyCommand();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetExportName() // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		QString exportName(Ui->psdExportNameLineEdit->text());
		SetExportName(exportName);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetExportName(QString value)
	{
		if( !IsFbxVersion )
			return;

		this->GetParameters().FileExportName = value;
		this->GetScene().SaveValuesToJson(); // write user settings to disk
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnEditExportPath(const QString value) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		EditExportPath(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::EditExportPath(const QString value)
	{
		if( !IsFbxVersion )
			return;

		if( value.contains( QChar('\\') ) )
		{
			QString patched( value );
			patched.replace( QChar('\\'), QChar('/') );
			Ui->psdExportPathLineEdit->setText(patched);
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetExportPath() // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		QString exportPath(Ui->psdExportPathLineEdit->text());
		SetExportPath(exportPath);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetExportPath(QString value)
	{
		if( !IsFbxVersion )
			return;

		this->GetParameters().FileExportPath = value;
		this->GetScene().SaveValuesToJson(); // write user settings to disk
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnExportPathSelector(bool b)
	{
		if( SilenceUi ) return;  // avoid deadlock
		ExportPathSelector(b);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::ExportPathSelector(bool)
	{
		IPluginOutput& output = GetOutput();

		// get the file extension from the output module
		OPENFILENAMEW ofnw;
		memset(&ofnw, 0, sizeof(OPENFILENAMEW));
		ofnw.lStructSize = sizeof(OPENFILENAMEW);
		PluginOutputParameters pluginOutputParams(this->GetParameters());
		output.GetSaveDialogParams(&ofnw, pluginOutputParams);
		this->GetParameters().FileExportExt = QString::fromWCharArray( ofnw.lpstrDefExt, (int)wcslen(ofnw.lpstrDefExt) );

		QString qstrCaption(util::LocalizeString(IDC_MAIN, IDS_FBX_SAVE_PATH_DIALOG));
		QString qstrInitialDir( this->GetParameters().Prefs.FileExportPath );
		// Windows doesn't allow folder chooser to show file contents, so ShowDirsOnly flag is on by default; would prefer showing files though
		QString qstrExportDir = QFileDialog::getExistingDirectory( this, qstrCaption, qstrInitialDir );

		if( !(qstrExportDir.isEmpty()) )
		{
			this->GetParameters().FileExportPath = qstrExportDir;
			this->GetParameters().Prefs.FileExportPath = qstrExportDir;

			this->GetScene().SaveValuesToJson();// write user settings to disk
			UpdateFields();
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetTextureProxy(const int value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetTextureProxy(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetTextureProxy(const int value) // UI event
	{
		int proxy = 1; // full size
		switch( value )
		{
		case 1: proxy = 2; break; // half size
		case 2: proxy = 4; break; // quarter size
		case 3: proxy = 8; break; // eighth size
		}
		this->GetParameters().TextureProxy = proxy;
		this->GetScene().SaveValuesToJson(); // write user settings to disk
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	int ToolWidget::GetTextureProxyUi()
	{
		int proxy = this->GetParameters().TextureProxy;
		int proxyUi = 0; // full size
		switch( proxy )
		{
		case 2: proxyUi = 1; break; // half size
		case 4: proxyUi = 2; break; // quarter size
		case 8: proxyUi = 3; break; // eighth size
		}
		return proxyUi; // float based
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetDepthModifier(const double value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetDepthModifier(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetDepthModifier(const double value)
	{
		float depthScale = 1.0f;
		if( IsMayaVersion )
		{
			depthScale = 0.1f; // Maya treats the depth values as multiplied by 1/10th
		}
		this->GetParameters().Depth = value * depthScale;
		this->GetScene().SaveValuesToJson(); // write user settings to disk
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	float ToolWidget::GetDepthUi()
	{
		float depthScale = 1.0f;
		if( IsMayaVersion )
		{
			depthScale = 0.1f; // Maya treats the depth values as multiplied by 1/10th
		}
		return this->GetParameters().Depth / depthScale;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetMeshScale(const double value) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		SetMeshScale(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetMeshScale(const double value)
	{
		//this->GetParameters().Scale = (value + 100.0f) / 100.0f; // integer based
		this->GetParameters().Scale = value; // float based
		this->GetScene().SaveValuesToJson(); // write user settings to disk
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	float ToolWidget::GetMeshScaleUi()
	{		
		//return this->GetParameters().Scale * 100.0f - 100.f; // integer based
		return this->GetParameters().Scale; // float based
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetWriteMode(const int value) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		SetWriteMode(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetWriteMode(const int value)
	{
		this->GetParameters().FileWriteMode = (value == 0?
			GlobalParameters::FileWriteMode::BINARY : GlobalParameters::FileWriteMode::ASCII);
		this->GetScene().SaveValuesToJson(); // write user settings to disk
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	int ToolWidget::GetWriteModeUi()
	{
		if( this->GetParameters().FileWriteMode == GlobalParameters::FileWriteMode::ASCII )
			return 1;
		return 0; // set UI dropdown to "BINARY" by default
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetWriteLayout(const int value) // UI event
	{
		if (SilenceUi) return;  // avoid deadlock
		SetWriteLayout(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetWriteLayout(const int value)
	{
		if (value == 1)
			this->GetParameters().FileWriteLayout = GlobalParameters::FileWriteLayout::MULTI_PER_TEXTURE;
		else if (value == 2)
			this->GetParameters().FileWriteLayout = GlobalParameters::FileWriteLayout::MULTI_PER_LAYER;
		else // set parameter to "SINGLE" by default
			this->GetParameters().FileWriteLayout = GlobalParameters::FileWriteLayout::SINGLE;
		this->GetScene().SaveValuesToJson(); // write user settings to disk
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	int ToolWidget::GetWriteLayoutUi()
	{
		if( this->GetParameters().FileWriteLayout == GlobalParameters::FileWriteLayout::MULTI_PER_TEXTURE )
			return 1;
		if( this->GetParameters().FileWriteLayout == GlobalParameters::FileWriteLayout::MULTI_PER_LAYER )
			return 2;
		return 0; // set UI dropdown to "SINGLE" by default
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetActiveKeepGroup(const int value) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		SetActiveKeepGroup(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetActiveKeepGroup(const int value)
	{
		this->GetParameters().KeepGroupStructure = value == 0;
		this->GetScene().SaveValuesToJson(); // write user settings to disk
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetAliasPsdName(const QString value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetAliasPsdName(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetAliasPsdName(const QString value)
	{
		this->GetParameters().AliasPsdName = value;
		this->GetScene().SaveValuesToJson(); // write user settings to disk
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetBillboardAlphaThresh(const double value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetBillboardAlphaThresh(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetBillboardAlphaThresh(const double value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->BillboardParameters.BillboardAlphaThresh = (int)value;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetLinearPrecision(const double value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetLinearPrecision(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetLinearPrecision(const double value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->LinearParameters.LinearHeightPoly = value;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetDelaunayOuterDetail(const double value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetDelaunayOuterDetail(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetDelaunayOuterDetail(const double value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->DelaunayParameters.OuterDetail = value;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetDelaunayInnerDetail(const double value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetDelaunayInnerDetail(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetDelaunayInnerDetail(const double value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->DelaunayParameters.InnerDetail = value;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetDelaunayFalloffDetail(const double value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetDelaunayFalloffDetail(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetDelaunayFalloffDetail(const double value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->DelaunayParameters.FalloffDetail = value;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetMergeEnabled(bool value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetMergeEnabled(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetMergeEnabled(bool value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->CurveParameters.MergeVertexEnabled = value;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetMergeDistance(const double& value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetMergeDistance(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetMergeDistance(const double& value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->CurveParameters.MergeVertexDistance = value;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetEnableInfluence(bool value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetEnableInfluence(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetEnableInfluence(bool value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->EnableInfluence = value;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetMinPolygonSize(const double& value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetMinPolygonSize(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetMinPolygonSize(const double& value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->InfluenceParameters.MinPolygonSize = value;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetMaxPolygonSize(const double& value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetMaxPolygonSize(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetMaxPolygonSize(const double& value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->InfluenceParameters.MaxPolygonSize = value;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetAlgorithm(const int value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetAlgorithm( GetGenerationAlgoComboBoxToEnum(value) );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetAlgorithm(const int value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->Algo = static_cast<LayerParameters::Algorithm>(value);
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
		UpdatePanels();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetAtlasIndex(const int value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetAtlasIndex(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetAtlasIndex(const int value)
	{
		if( !IsAtlasVersion )
			return;

		bool atlasUpdate = true;
		int atlasIndex = value;
		// Special handling for append options, "New atlas..."
		if( Ui->textureAtlasGroupComboBox!=nullptr )
		{
			bool isAppend = (value==(Ui->textureAtlasGroupComboBox->count() - 1));  // append option
			bool isNone = (value==0); // if atlas option set to "None"

			if( isAppend )
			{
				const char* atlasNameDefault = util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_GROUP_DEFAULT );
				this->GetScene().AddAtlas( atlasNameDefault, -1 ); // append new atlas agent
				// Insert new item, without insertItem(), avoid recursive call from changing index
				// Append a duplicate of the last item, then rename the old last item.
				Ui->textureAtlasGroupComboBox->addItem("");
				int lastIndex = (Ui->textureAtlasGroupComboBox->count()-1);
				int insertIndex = (lastIndex-1); // adjust for trailing item "New atlas..."
				for( int i=lastIndex; i>insertIndex; i-- )
					Ui->textureAtlasGroupComboBox->setItemText( i, Ui->textureAtlasGroupComboBox->itemText(i-1) );
				Ui->textureAtlasGroupComboBox->setItemText( insertIndex, atlasNameDefault );
				atlasIndex = insertIndex-1; // adjust for one leading item "None"
			}
			else
			{
				atlasIndex = value-1;  //first entry means atlas unused, entry 0 -> index -1
			}

			if( atlasUpdate )
			{
				bool editable = (!isNone); // editable unless atlas option set to "None"
				Ui->textureAtlasGroupComboBox->setEditable( editable );
				if( editable )
				{
					QRegExp regExp("[^\\\\/:\\*\\?\"<>\\|]*"); // restrict characters \ / : * ? " < > |
					QRegExpValidator* regExpValidator = new QRegExpValidator(regExp, 0);
					QLineEdit* lineEdit = Ui->textureAtlasGroupComboBox->lineEdit();
					lineEdit->setValidator(regExpValidator);
				}
			}
		}

		if( atlasUpdate )
		{
			// Set the atlas index values
			for (const auto& item : Ui->layerList->selectedItems())
			{
				int layerIndex = GetLayerIndex(item);
				LayerParameters& layerParams = GetLayerParameters(layerIndex);
				AtlasParameters& atlasParamsOld = GetAtlasParameters(layerParams.AtlasIndex);
				AtlasParameters& atlasParamsNew = GetAtlasParameters(atlasIndex);
				layerParams.AtlasIndex = atlasIndex;
				atlasParamsOld.layerIndices.erase(layerIndex);
				atlasParamsNew.layerIndices.insert(layerIndex);
				layerParams.IsModifiedTexture = layerParams.IsModifiedMesh = true; // flag layer as modified
				atlasParamsOld.isModifiedTexture = atlasParamsNew.isModifiedTexture = true; // flag atlas as modified
			}

			// Update UI
			UpdateLayers(true,true); // update all descriptions, calculate atlas
			UpdatePanels();
			UpdateFields();
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetAtlasName(QString value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetAtlasName(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetAtlasName(QString value)
	{
		if( !IsAtlasVersion )
			return;

		// This doesn't change any LayerParameters, but only the AtlasParameters
		// If the combo box doesn't exist, can't determine which entry to change
		// TODO: Add method parameter for atlasIndex or explain why not to
		if( Ui->textureAtlasGroupComboBox==nullptr )
			return;

		int comboIndex = Ui->textureAtlasGroupComboBox->currentIndex();
		if( (comboIndex<=0) || comboIndex>=(Ui->textureAtlasGroupComboBox->count()-1) )
		{
			return; // first item and last item shouldn't be editable but sometimes this is called anyway
		}

		int atlasIndex = comboIndex-1; // adjust by one; first entry means atlas unused, entry 0 -> index -1
		AtlasAgent& atlasAgent = this->GetScene().GetAtlasAgent(atlasIndex); 
		AtlasParameters& atlasParams = atlasAgent.GetAtlasParameters();
		atlasParams.atlasName = value;

		Ui->textureAtlasGroupComboBox->setItemText(comboIndex,value);

		UpdateLayers(true,false); // update all descriptions, don't calculate atlas

		UpdatePanels();
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetAtlasAlgo(const int value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetAtlasAlgo( GetAtlasAlgoComboBoxToEnum(value) );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetAtlasAlgo(const int value)
	{
		if( !IsAtlasVersion )
			return;

		// This doesn't change any LayerParameters, but only the AtlasParameters
		// If the combo box doesn't exist, can't determine which entry to change
		// TODO: Add method parameter for atlasIndex or explain why not to
		if( Ui->textureAtlasGroupComboBox==nullptr )
			return;

		int comboIndex = Ui->textureAtlasGroupComboBox->currentIndex();
		if( (comboIndex<=0) || comboIndex>=(Ui->textureAtlasGroupComboBox->count()-1) )
		{
			return; // first item and last item shouldn't be editable but sometimes this is called anyway
		}

		int atlasIndex = comboIndex-1; // adjust by one; first entry means atlas unused, entry 0 -> index -1
		AtlasAgent& atlasAgent = this->GetScene().GetAtlasAgent(atlasIndex); 
		AtlasParameters& atlasParams = atlasAgent.GetAtlasParameters();

		atlasParams.packingAlgo = value;
		atlasParams.isModifiedTexture = true; // flag as modified

		UpdateLayers(true,true); // update all descriptions, calculate atlas
		UpdatePanels();
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetAtlasOverride(bool b) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetAtlasOverride( b );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetAtlasOverride(bool b)
	{
		if( !IsAtlasVersion )
			return;

		// This doesn't change any LayerParameters, but only the AtlasParameters
		// If the combo box doesn't exist, can't determine which entry to change
		// TODO: Add method parameter for atlasIndex or explain why not to
		if( Ui->textureAtlasGroupComboBox==nullptr )
			return;

		int comboIndex = Ui->textureAtlasGroupComboBox->currentIndex();
		if( (comboIndex<=0) || comboIndex>=(Ui->textureAtlasGroupComboBox->count()-1) )
		{
			return; // first item and last item shouldn't be editable but sometimes this is called anyway
		}

		int atlasIndex = comboIndex-1; // adjust by one; first entry means atlas unused, entry 0 -> index -1
		AtlasAgent& atlasAgent = this->GetScene().GetAtlasAgent(atlasIndex); 
		AtlasParameters& atlasParams = atlasAgent.GetAtlasParameters();

		atlasParams.isCustomSize = b;
		atlasParams.isModifiedTexture = true; // flag as modified

		UpdateLayers(true,true); // update all descriptions, calculate atlas
		UpdatePanels();
		UpdateFields();
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetAtlasSize(const int value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetAtlasSize( value );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetAtlasSize(const int value)
	{
		if( !IsAtlasVersion )
			return;

		// This doesn't change any LayerParameters, but only the AtlasParameters
		// If the combo box doesn't exist, can't determine which entry to change
		// TODO: Add method parameter for atlasIndex or explain why not to
		if( Ui->textureAtlasGroupComboBox==nullptr )
			return;

		int comboIndex = Ui->textureAtlasGroupComboBox->currentIndex();
		if( (comboIndex<=0) || comboIndex>=(Ui->textureAtlasGroupComboBox->count()-1) )
		{
			return; // first item and last item shouldn't be editable but sometimes this is called anyway
		}

		int atlasIndex = comboIndex-1; // adjust by one; first entry means atlas unused, entry 0 -> index -1
		AtlasAgent& atlasAgent = this->GetScene().GetAtlasAgent(atlasIndex); 
		AtlasParameters& atlasParams = atlasAgent.GetAtlasParameters();

		atlasParams.customSize = value;
		atlasParams.isModifiedTexture = true; // flag as modified

		UpdateLayers(true,true); // update all descriptions, calculate atlas
		UpdatePanels();
		UpdateFields();
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetAtlasPadding(const int value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetAtlasPadding( value );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetAtlasPadding(const int value)
	{
		if( !IsAtlasVersion )
			return;

		// This doesn't change any LayerParameters, but only the AtlasParameters
		// If the combo box doesn't exist, can't determine which entry to change
		// TODO: Add method parameter for atlasIndex or explain why not to
		if( Ui->textureAtlasGroupComboBox==nullptr )
			return;

		int comboIndex = Ui->textureAtlasGroupComboBox->currentIndex();
		if( (comboIndex<=0) || comboIndex>=(Ui->textureAtlasGroupComboBox->count()-1) )
		{
			return; // first item and last item shouldn't be editable but sometimes this is called anyway
		}

		int atlasIndex = comboIndex-1; // adjust by one; first entry means atlas unused, entry 0 -> index -1
		AtlasAgent& atlasAgent = this->GetScene().GetAtlasAgent(atlasIndex); 
		AtlasParameters& atlasParams = atlasAgent.GetAtlasParameters();

		atlasParams.customPadding = value;
		atlasParams.isModifiedTexture = true; // flag as modified

		UpdateLayers(true,true); // update all descriptions, calculate atlas
		UpdatePanels();
		UpdateFields();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnAtlasSelector(bool b) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		AtlasSelector(b);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::AtlasSelector(bool b) // UI event
	{
		if( !IsAtlasVersion )
			return;


		// Collect each affected atlas params
		std::set<int> atlasIndexSet;
		for( const auto& item : Ui->layerList->selectedItems() )
		{
			LayerParameters* params = &(GetLayerParameters(item));
			if( params->AtlasIndex>=0 )
				atlasIndexSet.insert(params->AtlasIndex);
		}

		// Select each element using and affected atlas params
		for( const auto& item : WidgetMap )
		{
			LayerParameters* params = &(GetLayerParameters(item.first));
			if( atlasIndexSet.find(params->AtlasIndex) != atlasIndexSet.end() )
			{
#if MAYA_API_VERSION>=20220000
				item.first->setSelected(true);
#else
				Ui->layerList->setItemSelected(item.first, true);
#endif
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnAtlasCleanup(bool b) // UI event
	{
		if( SilenceUi ) return;  // avoid deadlock
		AtlasCleanup(b);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::AtlasCleanup(bool b) // UI event
	{
		if( !IsAtlasVersion )
			return;

		int comboIndex = Ui->textureAtlasGroupComboBox->currentIndex();
		IndexMap remap;
		remap = this->GetScene().CleanupAtlasAgents();

		SilenceUi++; // disregard UI event signals caused by update
		for( int i=(Ui->textureAtlasGroupComboBox->count()-2); i>=1; i-- )
		{
			int atlasIndex = (i-1); // adjust by one; first entry means atlas unused, entry 0 -> index -1
			if( remap[atlasIndex]==-1 )
			{
				Ui->textureAtlasGroupComboBox->removeItem(i);
			}
		}
		Ui->textureAtlasGroupComboBox->setCurrentIndex( remap[comboIndex-1]+1 ); // adjust by one
		SilenceUi--;

		this->GetScene().SaveValuesToJson(); // write user settings to disk
		UpdatePanels();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnSetTextureCropIndex(const int value) // UI event
	{
		if( SilenceUi ) return; // avoid deadlock
		SetTextureCropIndex(value);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetTextureCropIndex(const int value)
	{
		if( !IsAtlasVersion )
			return;

		bool enableTextureCrop = (value==0);
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = &(GetLayerParameters(item));
			params->EnableTextureCrop = enableTextureCrop;
			params->IsModifiedTexture = params->IsModifiedMesh = true; // flag as modified
		}
		UpdateLayers(true,true); // update all descriptions, calculate atlas
		UpdatePanels();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool ToolWidget::dragCheck( const QMimeData* mimeData, QString& filename_out )
	{
		bool retval = false;
		if( (mimeData!=nullptr) && (mimeData->hasUrls()) )
		{
			QList<QUrl> urls = mimeData->urls();
			int count = urls.size();
			if( count>0 )
			{
				QUrl url = urls[0];
				filename_out = url.toLocalFile();
				QFileInfo fileInfo( filename_out.toUtf8().data() );
				retval = fileInfo.exists();
			}
		}
		return retval;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::dragEnterEvent(QDragEnterEvent *event)
	{
		if( event==nullptr )
			return;

		QString filename;
		if( dragCheck(event->mimeData(),filename) )
		{
			event->acceptProposedAction();
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::dragMoveEvent(QDragMoveEvent *event)    { }

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::dragLeaveEvent(QDragLeaveEvent *event) { }

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::dropEvent(QDropEvent *event)
	{
		QString filename;
		if( dragCheck(event->mimeData(),filename) )
		{
			NotifyImportPSD( filename );
		}
	}

#pragma endregion

#pragma region CONTROLLER

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Called when button is pressed, Export Mesh or Export PNG
	void ToolWidget::NotifyCommand()
	{
		if (this->Controllers.empty()) return;
		for (auto controller : Controllers)
		{
			SilenceUi++; // disregard UI event signals caused by update
			controller->NotifyCommand();
			SilenceUi--;
		}
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	bool ToolWidget::ExportFilenameSelector()
	{
		bool retval = false;
		IPluginOutput& output = GetOutput();

		// Get the output file extension from the output module (for example ".fbx")
		OPENFILENAMEW ofnw;
		memset(&ofnw, 0, sizeof(OPENFILENAMEW));
		ofnw.lStructSize = sizeof(OPENFILENAMEW);
		PluginOutputParameters pluginOutputParams(this->GetParameters());
		output.GetSaveDialogParams(&ofnw, pluginOutputParams);
		// Set the file output file extension to the global params
		this->GetParameters().FileExportExt = QString::fromWCharArray( ofnw.lpstrDefExt, (int)wcslen(ofnw.lpstrDefExt) );

		// Get the initial directory and filename to display for the file save dialog,
		// use the export directory if set, otherwise the psd import directory
		QString qstrInitialDir( this->GetParameters().FileExportPath );
		if( qstrInitialDir.isEmpty() )
		{
			qstrInitialDir = this->GetParameters().FileImportPath;
		}
		qstrInitialDir += "/" + this->GetParameters().PsdName + "." + this->GetParameters().FileExportExt; // default filename

		// Get the filename filter (for example "*.fbx")
		int filterStrID = IDS_FBX_SAVE_FILE_DIALOG_PATTERN_ASCII;
		if( this->GetParameters().FileWriteMode==GlobalParameters::FileWriteMode::BINARY )
		{
			filterStrID = IDS_FBX_SAVE_FILE_DIALOG_PATTERN_BINARY;
		}
		QString qstrFilter(util::LocalizeString(IDC_MAIN, filterStrID));

		// Display the file save dialog
		QString qstrCaption(util::LocalizeString(IDC_MAIN, IDS_FBX_SAVE_FILE_DIALOG));
		QString qstrExportFile = QFileDialog::getSaveFileName( this, qstrCaption, qstrInitialDir, qstrFilter );

		// Unless the user cancelled, store the path and filename to the global params
		if( !qstrExportFile.isEmpty() )
		{
			QFileInfo fileInfo( qstrExportFile );
			this->GetParameters().FileExportPath = fileInfo.path();
			this->GetParameters().FileExportName = fileInfo.baseName();
			this->GetParameters().Prefs.FileExportPath = fileInfo.path();

			this->GetScene().SaveValuesToJson();// write user settings to disk
			UpdateFields();
			retval = true;
		}
		return retval;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// Invokes repaint in main thread
	void ToolWidget::RequestRepaint( bool updateLayout )
	{
		ToolWidgetRepaintRequest request( this );
		request.Emit(updateLayout);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnRepaint( bool updateLayout )
	{
		Repaint(updateLayout);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::Repaint( bool updateLayout )
	{
		for (int itemIndex=0; itemIndex<Ui->layerList->count(); itemIndex++)
		{
			QListWidgetItem* widget = Ui->layerList->item(itemIndex);
			int layerIndex = this->WidgetMap.at(widget);
			UpdateLayerDescription(layerIndex);
		}

		this->UpdateUi( updateLayout );
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::OnExitApp( bool checked )
	{
		ExitApp(checked);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::ExitApp( bool checked )
	{
		// crashes on exit //QApplication::quit();
		QApplication::closeAllWindows();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Usually one controller, Psd23DPlugin
	void ToolWidget::AddController(IPluginController* controller)
	{
		for (auto it : Controllers)
		{
			if (it == controller) return;
		}
		this->Controllers.push_back(controller);
		this->UpdateUi();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::RemoveController(IPluginController* controller)
	{
		for (auto it = Controllers.cbegin(); it != Controllers.cend(); ++it)
		{
			if ((*it) == controller) this->Controllers.erase(it);
		}
	}

#pragma endregion

#pragma region PRIVATE

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateLayers(bool allWidgets, bool updateAtlas)
	{
		QList<QListWidgetItem*> widgetList;
		if( allWidgets )
		{
			for( const auto& item : WidgetMap ) widgetList.push_back(item.first);
		}
		else
		{
			widgetList = Ui->layerList->selectedItems();
		}

		if( updateAtlas )
		{
			// Collect each affected atlas params
			std::set<int> atlasIndexSet;
			for( const auto& widget : widgetList )
			{
				LayerParameters* params = &(GetLayerParameters(widget));
				if( params->IsModifiedTexture && (params->AtlasIndex>=0) )
				{
					atlasIndexSet.insert(params->AtlasIndex);
				}
			}
			// Mark each affected atlas params as modified
			for( int atlasIndex : atlasIndexSet )
			{
				AtlasParameters* atlasParams = &(GetAtlasParameters(atlasIndex));
				atlasParams->isModifiedTexture = true;
			}
			// Mark each layer params using an affected atlas params as modified
			int layerCount = this->GetScene().GetLayerCount();
			for( int layerIndex=0; layerIndex<layerCount; layerIndex++ )
			{
				LayerAgent& layerAgent = this->GetScene().GetLayerAgent(layerIndex);
				LayerParameters& layerParams = layerAgent.GetLayerParameters();
				if( atlasIndexSet.find(layerParams.AtlasIndex) != atlasIndexSet.end() )
				{
					layerParams.IsModifiedTexture = true;
				}
			}
		}

		for( const auto& widget : widgetList )
		{
			int layerIndex = this->WidgetMap.at(widget);
			UpdateLayerDescription(layerIndex);
		}

		// assumes one or more settings were changed before this call
		this->GetScene().SaveValuesToJson(); // write user settings to disk

	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::ApplyLayerDescription(ListFrame* frame, psd_reader::PsdData& psdData, int layerIndex)
	{
		psd_reader::LayerData& layer = psdData.LayerMaskData.Layers[layerIndex];
		LayerAgent& layerAgent = this->GetScene().GetLayerAgent(layerIndex);
		LayerParameters& layerParams = layerAgent.GetLayerParameters();

		layerParams.ListFrame = frame;

		QVBoxLayout* layout = new QVBoxLayout;

		// TITLE
		// Title shown in the description field must be set once, here during initialization.
		// Other description details are set later in LayerParameters::UpdateDescription(),
		// but the title derives from psd_reader::LayerData and LayerParameters doesn't store that
		QString titleLabel;
		titleLabel.append("<b>");
		//QString layerDisplayName = QString::fromLatin1( layer.LayerDisplayName.c_str() );
		//titleLabel.append( layerDisplayName );
		titleLabel.append( layer.LayerDisplayName.c_str() );
		titleLabel.append("</b>");
		QFont font;
		//font.setBold(true);
		font.setPointSize(12); // layer name in description pane
		layerParams.LabelLayerTitle = new QLabel(titleLabel);
		layerParams.LabelLayerTitle->setTextInteractionFlags( Qt::NoTextInteraction );
		if( IsMayaVersion ) // set custom font in PSDtoMaya
		{
			// TODO: fix bug, label does not resize to fit this font in PSDtoFBX
			layerParams.LabelLayerTitle->setFont(font);
		}
		//int stretch = 10;
		//labelTitle->setAlignment(Qt::AlignBottom);
		//labelTitle->setIndent(50);
		layout->addWidget(layerParams.LabelLayerTitle); //,stretch

		// SIZE
		int anchorWidth = layer.AnchorRight - layer.AnchorLeft;
		int anchorHeight = layer.AnchorBottom - layer.AnchorTop;
		layerParams.AnchorRegion = boundsPixels( layer.AnchorLeft, layer.AnchorTop, anchorWidth, anchorHeight );

		// DESCRIPTION
		layerParams.LabelAlgoSelected = new QLabel();
		layerParams.LabelAlgoSelected->setIndent(15);
		layerParams.LabelAlgoSelected->setTextInteractionFlags( Qt::NoTextInteraction );
		layerParams.LabelLayerSize = new QLabel();
		layerParams.LabelLayerSize->setIndent(15);
		layerParams.LabelLayerSize->setTextInteractionFlags( Qt::NoTextInteraction );

		// SUMMARY ROW - SIZE AND ALGO SELECTED
		QHBoxLayout *summaryRowLayout = new QHBoxLayout;
		summaryRowLayout->setContentsMargins(0,0,0,0);
		QWidget* summaryRow = new QWidget();
		summaryRowLayout->addWidget(layerParams.LabelAlgoSelected);
		summaryRowLayout->addWidget(layerParams.LabelLayerSize);
		summaryRow->setLayout(summaryRowLayout);
		layout->addWidget(summaryRow);

		if( IsAtlasVersion )
		{
			// ATLAS SIZE
			layerParams.LabelAtlasSize = new QLabel();
			layerParams.LabelAtlasSize->setIndent(15);
			layerParams.LabelAtlasSize->setTextInteractionFlags( Qt::NoTextInteraction );

			// ATLAS NAME
			layerParams.LabelAtlasName = new QLabel();
			layerParams.LabelAtlasName->setIndent(15);
			layerParams.LabelAtlasName->setTextInteractionFlags( Qt::NoTextInteraction );

			// ATLAS ROW - SIZE AND NAME
			QHBoxLayout *atlasRowLayout = new QHBoxLayout;
			atlasRowLayout->setContentsMargins(0,0,0,0);
			QWidget *atlasRow = new QWidget();
			atlasRowLayout->addWidget(layerParams.LabelAtlasName);
			atlasRowLayout->addWidget(layerParams.LabelAtlasSize);
			atlasRow->setLayout(atlasRowLayout);
			layout->addWidget(atlasRow);
		}

		if( IsInfluenceSupported ) // influence only supported for PSDtoMaya
		{
			layerParams.LabelInfluence = new QLabel();
			layerParams.LabelInfluence->setIndent(15);
			layerParams.LabelInfluence->setTextInteractionFlags( Qt::NoTextInteraction );
			layout->addWidget(layerParams.LabelInfluence);
		}

		layerParams.LabelDescription = new QLabel();
		layerParams.LabelDescription->setIndent(15);
		layerParams.LabelDescription->setTextInteractionFlags( Qt::NoTextInteraction );
		
		layout->addWidget(layerParams.LabelDescription);

		UpdateLayerDescription(layerIndex);

		frame->setLayout(layout); // TODO: Does this leak the old layout and widgets?
		layout->invalidate();
		layout->update();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateLayerDescription(int layerIndex)
	{
		const SceneController& scene = GetScene();
		const LayerAgent& layerAgent = scene.GetLayerAgent(layerIndex);
		const LayerParameters& layerParams = GetLayerParameters(layerIndex);
		const AtlasAgent& atlasAgent = scene.GetAtlasAgent( layerParams.AtlasIndex );
		const AtlasParameters& atlasParams = atlasAgent.GetAtlasParameters();
		const LayerStats layerStats = layerAgent.GetLayerStats();

		// TODO: these should be in LayerStats
		bool layerModified = (layerParams.IsModifiedMesh) || (layerParams.IsModifiedTexture);
		bool atlasModified = (IsAtlasVersion) && (atlasParams.isModifiedTexture);
		bool layerFailed = (layerParams.IsFailedMesh);

		bool isNullAtlas = atlasAgent.IsNull();

		if( layerParams.LabelLayerTitle!=nullptr )
		{
			QString titleLabel;
			titleLabel.append("<b>");
			if( layerFailed ) titleLabel.append("<font color='#ff0000'>( ! )  </font>");
			else if( layerModified || atlasModified ) titleLabel.append("<font color='#f48c42'>( ! )  </font>"); // #f48c42
			titleLabel.append( layerParams.LayerName );
			titleLabel.append("</b>");
			layerParams.LabelLayerTitle->setText(titleLabel);
		}

		if( layerParams.LabelAlgoSelected!=nullptr )
		{
			QString algoSelected;
			algoSelected.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_MODE_SELECTED ) ); // "Mode selected: "
			if( !layerStats.isAlgoSupported ) algoSelected.append("<font color='#ff0000'>");
			algoSelected.append("<b>");
			if (layerParams.Algo == LayerParameters::LINEAR)
				algoSelected.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_LINEAR ) ); // "Linear"
			else if (layerParams.Algo == LayerParameters::DELAUNAY)
				algoSelected.append( util::LocalizeString( IDC_MAIN,IDS_LAYER_LIST_DELAUNAY  ) ); // "Delaunay"
			else if (layerParams.Algo == LayerParameters::CURVE)
				algoSelected.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_VECTOR ) ); // "Vector"
			else if (layerParams.Algo == LayerParameters::BILLBOARD)
				algoSelected.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_BILLBOARD ) ); // "Billboard"
			algoSelected.append("</b>");
			if( !layerStats.isAlgoSupported ) algoSelected.append("</font>");
			layerParams.LabelAlgoSelected->setText(algoSelected);
		}

		const boundsPixels& atlasBounds = layerStats.atlasBounds;
		bool isAtlasNegative = (!isNullAtlas) && ((atlasBounds.WidthPixels() <= 0) || (atlasBounds.HeightPixels() <= 0));
		bool isAtlasZero = (!isNullAtlas) && (atlasBounds.WidthPixels() == 0) && (atlasBounds.HeightPixels() == 0);
		bool isAtlasOversize = (!isNullAtlas) && ((atlasBounds.WidthPixels() > 8192) || (atlasBounds.HeightPixels() > 8192));

		if( layerParams.LabelLayerSize!=nullptr )
		{
			QString layerSize("");
			if( !layerStats.isLayerReady )
			{
				// when another thread is calculating layer and atlas bounds, the atlas bounds are negative

				layerSize.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_CALCULATING ) ); // "calculating..."
			}
			else if( layerStats.isAlgoSupported )
			{
				const boundsPixels& bounds = layerStats.layerBounds;
				layerSize.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_SIZE ) ); // "Size: "
				layerSize.append("<b>");
				layerSize.append(std::to_string(bounds.WidthPixels()).c_str());
				layerSize.append(" x ");
				layerSize.append(std::to_string(bounds.HeightPixels()).c_str());
				layerSize.append("</b>");
			}
			layerParams.LabelLayerSize->setText(layerSize);
		}

		if( IsAtlasVersion )
		{
			if( layerParams.LabelAtlasName!=nullptr )
			{
				QString atlasName( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_GROUP ) ); // "Group: "
				atlasName.append("<b>");
				if( !atlasAgent.IsNull() )
				{
					atlasName.append("<font color='yellowgreen'>");
					atlasName.append(atlasParams.atlasName);
					atlasName.append("</font>");
				}
				else
					atlasName.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_GROUP_NONE ) ); // "None"
				atlasName.append("</b>");
				layerParams.LabelAtlasName->setText(atlasName);
			}

			if( layerParams.LabelAtlasSize!=nullptr )
			{
				QString atlasSize("");
				if( layerParams.AtlasIndex>=0 )
				{
					atlasSize.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_ATLAS ) ); // "Atlas: "
					atlasSize.append("<b>");
					if( !layerStats.isAtlasReady )
					{
						// when another thread is calculating layer and atlas bounds, the atlas bounds are negative
						atlasSize.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_CALCULATING ) ); //  "calculating..."
					}
					else if( isAtlasZero )
					{
						// something is wrong with the atlas, display nothing
					}
					else if( isAtlasOversize )
					{
						atlasSize.append("<font color='#f44242'>");
						atlasSize.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_ATLAS_OVERSIZE ) ); // "oversize - remove layers"
						atlasSize.append("</font>");
					}
					else
					{
						const boundsPixels& bounds = layerStats.atlasBounds;
						atlasSize.append(std::to_string(bounds.WidthPixels()).c_str());
						atlasSize.append(" x ");
						atlasSize.append(std::to_string(bounds.HeightPixels()).c_str());
					}
					if( atlasParams.isCustomSize && layerStats.isAtlasReady )
					{
						int customSize = atlasParams.customSize;
						int fitSize = layerStats.atlasBoundsFit.WidthPixels();
						if( customSize < fitSize )
						{
							int downscalePercent = (int)round(100.0f * (customSize/(float)fitSize));
							atlasSize.append(", <font color='#f48c42'> ");
							atlasSize.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_ATLAS_SCALE ) ); // "scale"
							atlasSize.append( " " );
							atlasSize.append( std::to_string(downscalePercent).c_str() );
							atlasSize.append("%</font>");
						}
						else if( customSize > fitSize )
						{
							int paddingPixels = customSize - fitSize;
							atlasSize.append(", <font color='#f48c42'> ");
							atlasSize.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_ATLAS_PADDING ) ); // "padding"
							atlasSize.append( " " );
							atlasSize.append( std::to_string(paddingPixels).c_str() );
							atlasSize.append("</font>");
						}
							atlasSize.append("</b>");
						}

					}
				layerParams.LabelAtlasSize->setText(atlasSize);
			}
		}

		if( layerParams.LabelInfluence!=nullptr )
		{
			QString Influence;
			Influence.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_INFLUENCE ) ); // "Influence: "
			if (layerParams.HasInfluenceLayer)
			{
				Influence.append( "<b>" );
				Influence.append( util::LocalizeString( IDC_MAIN, // "Active"  or "Inactive"
					layerParams.EnableInfluence ? IDS_LAYER_LIST_INFLUENCE_ACTIVE : IDS_LAYER_LIST_INFLUENCE_INACTIVE ) );
				Influence.append( "</b>" );
			}
			else
			{
				Influence.append( "<font color='#6a6a6a'>" ); // '#f48c42'
				Influence.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_INFLUENCE_NOT_AVAILABLE ) ); // "Not available"
				Influence.append( "</font>" );
			}
			layerParams.LabelInfluence->setText(Influence);
		}

		if( layerParams.LabelDescription!=nullptr )
		{
			QString description;
			description.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_MODE_SUPPORTED ) ); // "Mode supported: "
			description.append("<i>");
			description.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_BILLBOARD ) ); // "Billboard"
			description.append("</i>");

			if( IsVectorModeSupported && layerParams.HasVectorSupport )
			{
				description.append(" <i>");
				description.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_VECTOR ) ); // "Vector"
				description.append("</i>");
			}
			if( IsLinearModeSupported && layerParams.HasLinearSupport )
			{
				description.append(" <i>");
				description.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_LINEAR ) ); // "Linear"
				description.append("</i>");
			}
			if( IsDelaunayModeSupported && layerParams.HasDelaunaySupport )
			{
				description.append(" <i>");
				description.append( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_DELAUNAY ) ); // "Delaunay"
				description.append("</i>");
			}
			layerParams.LabelDescription->setText(description);
		}

		if( (layerParams.ListFrame!=nullptr) && (layerParams.ListFrame->layout()!=nullptr) )
		{
			layerParams.ListFrame->layout()->invalidate();
			layerParams.ListFrame->layout()->update();
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateLocalization()
	{
		if( !util::IsLocalizationEnglish() ) // UI in english by default, only translate for non-english
		{
			Ui->menuFile->setTitle( util::LocalizeString( IDC_MAIN, IDS_FILE_MENU ) );
			Ui->actionFile_ImportPSD->setText( util::LocalizeString( IDC_MAIN, IDS_FILE_MENU_IMPORT_PSD ) );
			Ui->actionFile_Reload->setText( util::LocalizeString( IDC_MAIN, IDS_FILE_MENU_RELOAD ) );
			Ui->actionFile_Exit->setText( util::LocalizeString( IDC_MAIN, IDS_FILE_MENU_EXIT ) );

			Ui->menuGenerate->setTitle( util::LocalizeString( IDC_MAIN, IDS_GENERATE_MENU ) );
			Ui->actionGenerate_GenerateMesh->setText( util::LocalizeString( IDC_MAIN, IDS_GENERATE_MENU_GENERATE_MESH ) );
			Ui->actionGenerate_GeneratePng->setText( util::LocalizeString( IDC_MAIN, IDS_GENERATE_MENU_GENERATE_PNG ) );
			Ui->actionGenerate_GenerateBoth->setText( util::LocalizeString( IDC_MAIN, IDS_GENERATE_MENU_GENERATE_BOTH ) );

			Ui->menuHelp->setTitle( util::LocalizeString( IDC_MAIN, IDS_HELP_MENU ) );
			Ui->actionHelp_Tutorials->setText( util::LocalizeString( IDC_MAIN, IDS_HELP_MENU_DOCS ) );

			Ui->psdSelectorLabel->setText( util::LocalizeString( IDC_MAIN, IDS_PSD_IMPORT_LABEL ) );
			Ui->psdSelectorLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_PSD_IMPORT_TOOLTIP ) );
			Ui->psdSelectorLineEdit->setToolTip( util::LocalizeString( IDC_MAIN, IDS_PSD_SELECTOR_TOOLTIP ) );
			Ui->psdImportBtn->setText( util::LocalizeString( IDC_MAIN, IDS_PSD_IMPORT_BTN ) );
			Ui->psdImportBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_PSD_IMPORT_TOOLTIP ) );
			Ui->psdReloadBtn->setText( util::LocalizeString( IDC_MAIN, IDS_PSD_RELOAD_BTN ) );
			Ui->psdReloadBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_PSD_RELOAD_TOOLTIP ) );
			Ui->psdExportPathLabel->setText( util::LocalizeString( IDC_MAIN, IDS_PSD_EXPORT_PATH_LABEL ) );
			Ui->psdExportPathLineEdit->setToolTip( util::LocalizeString( IDC_MAIN, IDS_PSD_EXPORT_PATH_TOOLTIP ) );
			Ui->psdExportNameLineEdit->setToolTip( util::LocalizeString( IDC_MAIN, IDS_PSD_EXPORT_NAME_TOOLTIP ) );
			Ui->psdExportPathBtn->setText( util::LocalizeString( IDC_MAIN, IDS_PSD_EXPORT_PATH_BTN ) );
			Ui->psdExportPathBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_PSD_EXPORT_PATH_BTN_TOOLTIP ) );
			Ui->selectedPsdSizeLabel->setText( util::LocalizeString( IDC_MAIN, IDS_SELECTED_PSD_SIZE_LABEL ) );
			Ui->selectedPsdSizeLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_SELECTED_PSD_SIZE_TOOLTIP ) );
			Ui->selectedPsdSizeValue->setToolTip( util::LocalizeString( IDC_MAIN, IDS_SELECTED_PSD_SIZE_TOOLTIP ) );

			Ui->layerList->setToolTip( util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_TOOLTIP ) );

			Ui->generationAlgoLabel->setText( util::LocalizeString( IDC_MAIN, IDS_GENERATION_ALGO_LABEL ) );
			// must happen before items are removed according to product version during setup
			Ui->generationAlgoComboBox->setItemText( 0, util::LocalizeString( IDC_MAIN, IDS_GENERATION_ALGO_DROPDOWN_LINEAR ) );
			Ui->generationAlgoComboBox->setItemText( 1, util::LocalizeString( IDC_MAIN, IDS_GENERATION_ALGO_DROPDOWN_BILLBOARD ) );
			Ui->generationAlgoComboBox->setItemText( 2, util::LocalizeString( IDC_MAIN, IDS_GENERATION_ALGO_DROPDOWN_DELAUNAY ) );
			Ui->generationAlgoComboBox->setItemText( 3, util::LocalizeString( IDC_MAIN, IDS_GENERATION_ALGO_DROPDOWN_VECTOR ) );

			Ui->billboardAlgoLabel->setText( util::LocalizeString( IDC_MAIN, IDS_BILLBOARD_ALGO_LABEL ) );
			Ui->billboardAlphaThreshLabel->setText( util::LocalizeString( IDC_MAIN, IDS_BILLBOARD_ALPHA_THRESH_LABEL ) );
			Ui->billboardAlphaThreshLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_BILLBOARD_ALPHA_THRESH_TOOLTIP ) );
			Ui->billboardAlphaThreshField->setToolTip( util::LocalizeString( IDC_MAIN, IDS_BILLBOARD_ALPHA_THRESH_TOOLTIP ) );

			Ui->linearAlgoLabel->setText( util::LocalizeString( IDC_MAIN, IDS_LINEAR_ALGO_LABEL ) );
			Ui->linearPrecisionLabel->setText( util::LocalizeString( IDC_MAIN, IDS_LINEAR_PRECISION_LABEL ) );
			Ui->linearPrecisionLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_LINEAR_PRECISION_TOOLTIP ) );
			Ui->linearPrecisionField->setToolTip( util::LocalizeString( IDC_MAIN, IDS_LINEAR_PRECISION_TOOLTIP ) );

			Ui->delaunayAlgoLabel->setText( util::LocalizeString( IDC_MAIN, IDS_DELAUNAY_ALGO_LABEL ) );
			Ui->delaunayOuterDetailLabel->setText( util::LocalizeString( IDC_MAIN, IDS_DELAUNAY_OUTER_DETAIL_LABEL ) );
			Ui->delaunayOuterDetailLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_DELAUNAY_OUTER_DETAIL_TOOLTIP ) );
			Ui->delaunayOuterDetailField->setToolTip( util::LocalizeString( IDC_MAIN, IDS_DELAUNAY_OUTER_DETAIL_TOOLTIP ) );
			Ui->delaunayInnerDetailLabel->setText( util::LocalizeString( IDC_MAIN, IDS_DELAUNAY_INNER_DETAIL_LABEL ) );
			Ui->delaunayInnerDetailLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_DELAUNAY_INNER_DETAIL_TOOLTIP ) );
			Ui->delaunayInnerDetailField->setToolTip( util::LocalizeString( IDC_MAIN, IDS_DELAUNAY_INNER_DETAIL_TOOLTIP ) );
			Ui->delaunayFalloffDetailLabel->setText( util::LocalizeString( IDC_MAIN, IDS_DELAUNAY_INNER_FALLOFF_LABEL ) );
			Ui->delaunayFalloffDetailLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_DELAUNAY_INNER_FALLOFF_TOOLTIP ) );
			Ui->delaunayFalloffDetailField->setToolTip( util::LocalizeString( IDC_MAIN, IDS_DELAUNAY_INNER_FALLOFF_TOOLTIP ) );

			Ui->vectorAlgoLabel->setText( util::LocalizeString( IDC_MAIN, IDS_VECTOR_ALGO_LABEL ) );
			Ui->mergeVerticesChk->setText( util::LocalizeString( IDC_MAIN, IDS_MERGE_VERTICES_CHK ) );
			Ui->mergeVerticesChk->setToolTip( util::LocalizeString( IDC_MAIN, IDS_MERGE_VERTICES_TOOLTIP ) );
			Ui->mergeVertexDistanceLabel->setText( util::LocalizeString( IDC_MAIN, IDS_MERGE_VERTEX_DISTANCE_LABEL ) );
			Ui->mergeVertexDistanceLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_MERGE_VERTEX_DISTANCE_TOOLTIP ) );
			Ui->mergeVertexDistanceField->setToolTip( util::LocalizeString( IDC_MAIN, IDS_MERGE_VERTEX_DISTANCE_TOOLTIP ) );

			Ui->multipleAlgoLabel_1->setText( util::LocalizeString( IDC_MAIN, IDS_MULTIPLE_ALGO_LABEL_1 ) );
			Ui->multipleAlgoLabel_2->setText( util::LocalizeString( IDC_MAIN, IDS_MULTIPLE_ALGO_LABEL_2 ) );

			Ui->influenceLayerChk->setText( util::LocalizeString( IDC_MAIN, IDS_INFLUENCE_LAYER_CHK ) );
			Ui->influenceLayerChk->setToolTip( util::LocalizeString( IDC_MAIN, IDS_INFLUENCE_LAYER_TOOLTIP ) );
			Ui->minimumPolygonSizeLabel->setText( util::LocalizeString( IDC_MAIN, IDS_INFLUENCE_MINIMUM_POLYGON_SIZE ) );
			Ui->minimumPolygonSizeLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_INFLUENCE_MINIMUM_POLYGON_SIZE_TOOLTIP ) );
			Ui->minimumPolygonSizeField->setToolTip( util::LocalizeString( IDC_MAIN, IDS_INFLUENCE_MINIMUM_POLYGON_SIZE_TOOLTIP ) );
			Ui->maximumPolygonSizeLabel->setText( util::LocalizeString( IDC_MAIN, IDS_INFLUENCE_MAXIMUM_POLYGON_SIZE ) );
			Ui->maximumPolygonSizeLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_INFLUENCE_MAXIMUM_POLYGON_SIZE_TOOLTIP ) );
			Ui->maximumPolygonSizeField->setToolTip( util::LocalizeString( IDC_MAIN, IDS_INFLUENCE_MAXIMUM_POLYGON_SIZE_TOOLTIP ) );

			Ui->multipleSettingsLabel_1->setText( util::LocalizeString( IDC_MAIN, IDS_MULTIPLE_SETTINGS_LABEL_1 ) );
			Ui->multipleSettingsLabel_2->setText( util::LocalizeString( IDC_MAIN, IDS_MULTIPLE_SETTINGS_LABEL_2 ) );

			Ui->textureAtlasGroupLabel->setText( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_GROUP_LABEL ) );
			Ui->textureAtlasGroupLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_GROUP_TOOLTIP ) );
			Ui->textureAtlasGroupComboBox->setItemText( 0, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_GROUP_DROPDOWN_NONE ) );
			Ui->textureAtlasGroupComboBox->setItemText( 1, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_GROUP_DROPDOWN_NEW ) );
			Ui->textureAtlasGroupComboBox->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_GROUP_TOOLTIP ) );

			// TODO: Localize packing algorithm dropdown

			Ui->textureAtlasSelectorBtn->setText( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_SELECTOR_BTN ) );
			Ui->textureAtlasSelectorBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_SELECTOR_TOOLTIP ) );
			Ui->textureAtlasCleanupBtn->setText( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_CLEANUP_BTN ) );
			Ui->textureAtlasCleanupBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_CLEANUP_TOOLTIP ) );

			Ui->textureAtlasAlgoLabel->setText( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_ALGO_LABEL ) );
			Ui->textureAtlasAlgoLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_ALGO_TOOLTIP ) );
			Ui->textureAtlasAlgoComboBox->setItemText( 0, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_ALGO_DROPDOWN_BEST_SHORT_SIDE_FIT ) );
			Ui->textureAtlasAlgoComboBox->setItemText( 1, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_ALGO_DROPDOWN_BEST_LONG_SIDE_FIT ) );
			Ui->textureAtlasAlgoComboBox->setItemText( 2, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_ALGO_DROPDOWN_BEST_AREA_FIT ) );
			Ui->textureAtlasAlgoComboBox->setItemText( 3, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_ALGO_DROPDOWN_BOTTOM_LEFT_RULE ) );
			Ui->textureAtlasAlgoComboBox->setItemText( 4, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_ALGO_DROPDOWN_CONTACT_POINT_RULE ) );
			Ui->textureAtlasAlgoComboBox->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_ALGO_TOOLTIP ) );
			Ui->textureAtlasOverrideLabel->setText( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_OVERRIDE_LABEL ) );
			Ui->textureAtlasOverrideLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_OVERRIDE_TOOLTIP ) );
			Ui->textureAtlasOverrideChk->setText( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_OVERRIDE_CHK ) );
			Ui->textureAtlasOverrideChk->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_OVERRIDE_TOOLTIP ) );
			Ui->textureAtlasSizeSpin->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_SIZE_TOOLTIP ) );
			Ui->textureAtlasPaddingLabel->setText( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_PADDING_LABEL ) );
			Ui->textureAtlasPaddingSpin->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_PADDING_TOOLTIP ) );

			Ui->textureOutputCropLabel->setText( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_OUTPUT_SIZE_LABEL ) );
			Ui->textureOutputCropLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_OUTPUT_SIZE_TOOLTIP ) );
			Ui->textureOutputSizeComboBox->setItemText( 0, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_OUTPUT_SIZE_DROPDOWN_CROPPED ) );
			Ui->textureOutputSizeComboBox->setItemText( 1, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_OUTPUT_SIZE_DROPDOWN_FULL ) );
			Ui->textureOutputSizeComboBox->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_OUTPUT_SIZE_TOOLTIP ) );

			Ui->emptySettingsPanelLabel->setText( util::LocalizeString( IDC_MAIN, IDS_EMPTY_SETTINGS_LABEL_1 ) );

			Ui->layersSelectAllBtn->setText( util::LocalizeString( IDC_MAIN, IDS_LAYERS_SELECT_ALL_BTN ) );
			Ui->layersSelectNoneBtn->setText( util::LocalizeString( IDC_MAIN, IDS_LAYERS_SELECT_NONE_BTN ) );

			Ui->textureProxyLabel->setText( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_PROXY_LABEL ) );
			Ui->textureProxyLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_PROXY_TOOLTIP ) );
			Ui->textureProxyComboBox->setItemText( 0, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_PROXY_DROPDOWN_FULL ) );
			Ui->textureProxyComboBox->setItemText( 1, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_PROXY_DROPDOWN_HALF ) );
			Ui->textureProxyComboBox->setItemText( 2, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_PROXY_DROPDOWN_QUARTER ) );
			Ui->textureProxyComboBox->setItemText( 3, util::LocalizeString( IDC_MAIN, IDS_TEXTURE_PROXY_DROPDOWN_EIGHTH ) );
			Ui->textureProxyComboBox->setToolTip( util::LocalizeString( IDC_MAIN, IDS_TEXTURE_PROXY_TOOLTIP ) );

			Ui->globalSettingsLabel->setText( util::LocalizeString( IDC_MAIN, IDS_GLOBAL_SETTINGS_LABEL ) );

			Ui->customGroupNameLabel->setText( util::LocalizeString( IDC_MAIN, IDS_MESH_GROUP_NAME_LABEL ) );
			Ui->customGroupNameLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_MESH_GROUP_NAME_TOOLTIP ) );
			Ui->customGroupNameLineEdit->setToolTip( util::LocalizeString( IDC_MAIN, IDS_MESH_GROUP_NAME_TOOLTIP ) );

			Ui->depthModifierLabel->setText( util::LocalizeString( IDC_MAIN, IDS_DEPTH_MODIFIER_LABEL ) );
			Ui->depthModifierLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_DEPTH_MODIFIER_TOOLTIP ) );
			Ui->depthModifierField->setToolTip( util::LocalizeString( IDC_MAIN, IDS_DEPTH_MODIFIER_TOOLTIP ) );

			Ui->meshScaleLabel->setText( util::LocalizeString( IDC_MAIN, IDS_MESH_SCALE_LABEL ) );
			Ui->meshScaleLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_MESH_SCALE_TOOLTIP ) );
			Ui->meshScaleSpinner->setToolTip( util::LocalizeString( IDC_MAIN, IDS_MESH_SCALE_TOOLTIP ) );

			// no longer supported
			//Ui->groupStructureLabel->setText( util::LocalizeString( IDC_MAIN, IDS_GROUP_STRUCTURE_LABEL ) );
			//Ui->groupStructureLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GROUP_STRUCTURE_TOOLTIP ) );
			//Ui->groupStructureComboBox->setItemText( 0, util::LocalizeString( IDC_MAIN, IDS_GROUP_STRUCTURE_FLAT ) );
			//Ui->groupStructureComboBox->setItemText( 1, util::LocalizeString( IDC_MAIN, IDS_GROUP_STRUCTURE_KEEP ) );
			//Ui->groupStructureComboBox->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GROUP_STRUCTURE_TOOLTIP ) );

			Ui->writeModeLabel->setText( util::LocalizeString( IDC_MAIN, IDS_WRITE_MODE_LABEL ) );
			Ui->writeModeLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_WRITE_MODE_TOOLTIP ) );
			Ui->writeModeComboBox->setItemText( 0, util::LocalizeString( IDC_MAIN, IDS_WRITE_MODE_BINARY ) );
			Ui->writeModeComboBox->setItemText( 1, util::LocalizeString( IDC_MAIN, IDS_WRITE_MODE_ASCII ) );
			Ui->writeModeComboBox->setToolTip( util::LocalizeString( IDC_MAIN, IDS_WRITE_MODE_TOOLTIP ) );

			Ui->writeLayoutLabel->setText(util::LocalizeString(IDC_MAIN, IDS_WRITE_LAYOUT_LABEL));
			Ui->writeLayoutLabel->setToolTip(util::LocalizeString(IDC_MAIN, IDS_WRITE_LAYOUT_TOOLTIP));
			Ui->writeLayoutComboBox->setItemText(0, util::LocalizeString(IDC_MAIN, IDS_WRITE_LAYOUT_SINGLE));
			Ui->writeLayoutComboBox->setItemText(1, util::LocalizeString(IDC_MAIN, IDS_WRITE_LAYOUT_MULTI_TEXTURE));
			Ui->writeLayoutComboBox->setItemText(2, util::LocalizeString(IDC_MAIN, IDS_WRITE_LAYOUT_MULTI_LAYER));
			Ui->writeLayoutComboBox->setToolTip(util::LocalizeString(IDC_MAIN, IDS_WRITE_LAYOUT_TOOLTIP));

			Ui->generateActionLabel->setText( util::LocalizeString( IDC_MAIN, IDS_GENERATE_ACTION_LABEL ) );
			Ui->generateMeshBtn->setText( util::LocalizeString( IDC_MAIN, IDS_GENERATE_MESH_BTN ) );
			Ui->generatePngBtn->setText( util::LocalizeString( IDC_MAIN, IDS_GENERATE_PNG_BTN ) );
			Ui->generateBothBtn->setText( util::LocalizeString( IDC_MAIN, IDS_GENERATE_BOTH_BTN ) );
		}

		// dynamic tooltip to set in all languages including english
		if( IsFbxVersion )
		{
			this->setWindowTitle( util::LocalizeString( IDC_MAIN, IDS_MAIN_WIDGET_FBX ) );
			Ui->generationAlgoLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GENERATION_ALGO_TOOLTIP_FBX ) );
			Ui->generationAlgoComboBox->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GENERATION_ALGO_TOOLTIP_FBX ) );

			Ui->generateMeshBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GENERATE_MESH_TOOLTIP_FBX ) );
			Ui->generatePngBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GENERATE_PNG_TOOLTIP_FBX ) );
			Ui->generateBothBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GENERATE_BOTH_TOOLTIP_FBX ) );
		}
		if( IsMayaVersion )
		{
			this->setWindowTitle( util::LocalizeString( IDC_MAIN, IDS_MAIN_WIDGET_MAYA ) );
			Ui->generationAlgoLabel->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GENERATION_ALGO_TOOLTIP_MAYA ) );
			Ui->generationAlgoComboBox->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GENERATION_ALGO_TOOLTIP_MAYA ) );

			Ui->generateMeshBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GENERATE_MESH_TOOLTIP_MAYA ) );
			Ui->generatePngBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GENERATE_PNG_TOOLTIP_MAYA ) );
			Ui->generateBothBtn->setToolTip( util::LocalizeString( IDC_MAIN, IDS_GENERATE_BOTH_TOOLTIP_MAYA ) );
		}

	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateUi( bool updateLayout )
	{
		this->SilenceUi++; // disregard UI event signals caused by update
		if( updateLayout )
		{
			UpdatePanels();
			//UpdateFields();
			UpdateButtons();
		}
		// TODO: should only need UpdateFields with updateLayout;
		// How to update pesky atlas information fields in parameters panel without this?
		UpdateFields(); // ugh
		UpdateActiveLayers();
		this->SilenceUi--;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdatePanels()
	{
		QList<QListWidgetItem*> selectedItems = Ui->layerList->selectedItems();

		QString Selected;
#if MAYA_API_VERSION>=20220000
		Selected.asprintf(  util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_SELECTED_STATUS_FORMAT ), // "%i of %i selected"
			selectedItems.size(), Ui->layerList->count() );
#else
		Selected.sprintf(  util::LocalizeString( IDC_MAIN, IDS_LAYER_LIST_SELECTED_STATUS_FORMAT ), // "%i of %i selected"
			selectedItems.size(), Ui->layerList->count() );
#endif
		Ui->layersSelectCountLabel->setText(Selected);

		std::set<float> algorithms;
		bool anyAtlas = false; // if at least one layer is using an atlas
		bool anyNonAtlas = false; // if at least one layer is not using an atlas
		bool selAtlas = false;
		if( IsAtlasVersion && (selectedItems.size()>0) )
		{
			for (const auto& item : selectedItems)
			{
				const LayerParameters* params = &(GetLayerParameters(item));
				algorithms.insert(params->Algo);
				if( params->AtlasIndex<0 )
				{
					anyNonAtlas = true;
				}
				else
				{
					anyAtlas = true;
				}
			}

			int layerIndex = GetLayerIndex( Ui->layerList->selectedItems()[0] );
			const LayerParameters& layerParams = GetLayerParameters(layerIndex);
			if( layerParams.AtlasIndex>=0 )
			{
				selAtlas = true;
			}
		}

		const int algo = algorithms.empty() ? -1 : *algorithms.begin();
		Ui->generationAlgoPanel->setVisible(!algorithms.empty());
		if( IsAtlasVersion )
		{
			Ui->textureAtlasGroupPanel->setVisible(!algorithms.empty());
			Ui->textureAtlasSettingsPanel->setVisible(!algorithms.empty() && anyAtlas && selAtlas);
			Ui->textureOutputPanel->setVisible(!algorithms.empty() && anyNonAtlas);
		}
		Ui->billboardAlgorithmPanel->setVisible(algorithms.size() == 1 && algo == LayerParameters::Algorithm::BILLBOARD);
		Ui->linearAlgorithmPanel->setVisible(algorithms.size() == 1 && algo == LayerParameters::Algorithm::LINEAR);
		if( IsFullVersion )
		{
			Ui->delaunayAlgorithmPanel->setVisible(algorithms.size() == 1 && algo == LayerParameters::Algorithm::DELAUNAY);
			Ui->curveAlgorithmPanel->setVisible(algorithms.size() == 1 && algo == LayerParameters::Algorithm::CURVE);
		}
		else
		{
			Ui->delaunayAlgorithmPanel->setVisible(false);
			Ui->curveAlgorithmPanel->setVisible(false);
		}
		Ui->multipleAlgoPanel->setVisible(algorithms.size() > 1);
		if( IsInfluenceSupported)
		{
			Ui->influenceLayerPanel->setVisible(!algorithms.empty());
		}
		else
		{
			Ui->influenceLayerPanel->setVisible(false);
		}
		Ui->emptySettingsPanel->setVisible(algorithms.empty());
		Ui->multipleSettingsPanel->setVisible(!algorithms.empty() && AreSelectedValuesDifferent());
		if( IsFbxVersion || IsUnrealVersion )
		{
			// if linear mode is somehow selected in FBX version, maybe due to stale json file, don't display the settings
			Ui->linearAlgorithmPanel->setVisible(false);
		}
		if( IsMayaVersion || IsUnrealVersion )
		{
			Ui->psdExportPathPanel->setVisible(false);
//			Ui->psdExportNamePanel->setVisible(false);
			Ui->writeModePanel->setVisible(false);
			Ui->writeLayoutPanel->setVisible(false);
		}
		// TO DO: Fix "Keep Group Structure" functionality to support this
		Ui->groupStructurePanel->setVisible(false); // no longer supported; always hide

		QGridLayout* layout = Ui->middleSection;
		layout->invalidate();
		layout->update();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool ToolWidget::AreSelectedValuesDifferent()
	{
		if (Ui->layerList->selectedItems().count() <= 1)
			return false;

		// Check if values are different through layers
		std::set<int> billboardAlphaThresh;
		std::set<int> linearPrecision;
		//std::set<float> gridOrientation;
		std::set<int> delaunayOuterDetail;
		std::set<int> delaunayInnerDetail;
		std::set<int> delaunayFalloffDetail;
		std::set<bool> mergeVertexEnabled;
		std::set<float> mergeVertexDistance;
		std::set<bool> influenceActivated;
		std::set<float> minPolygonSize;
		std::set<float> maxPolygonSize;

		for (const auto& item : Ui->layerList->selectedItems())
		{
			const LayerParameters* params = &(GetLayerParameters(item));

			if( params->Algo==LayerParameters::Algorithm::BILLBOARD )
			{
				billboardAlphaThresh.insert(params->BillboardParameters.BillboardAlphaThresh);
			}
			if( params->Algo==LayerParameters::Algorithm::LINEAR )
			{
				linearPrecision.insert(params->LinearParameters.LinearHeightPoly);
			}
			if( params->Algo==LayerParameters::Algorithm::DELAUNAY )
			{
				delaunayOuterDetail.insert(params->DelaunayParameters.OuterDetail);
				delaunayInnerDetail.insert(params->DelaunayParameters.InnerDetail);
				delaunayFalloffDetail.insert(params->DelaunayParameters.FalloffDetail);
			}
			if( params->Algo==LayerParameters::Algorithm::CURVE )
			{
				mergeVertexEnabled.insert(params->CurveParameters.MergeVertexEnabled);
				mergeVertexDistance.insert(params->CurveParameters.MergeVertexDistance);
			}
			if( IsMayaVersion ) // influence only supported for PSDtoMaya
			{
				influenceActivated.insert(params->EnableInfluence);
				minPolygonSize.insert(params->InfluenceParameters.MinPolygonSize);
				maxPolygonSize.insert(params->InfluenceParameters.MaxPolygonSize);
			}
		}

		// check if more than one parameter value in any list of parameter values used...
		return billboardAlphaThresh.size()>1 
			|| linearPrecision.size() > 1
			//|| gridOrientation.size() > 1
			|| delaunayOuterDetail.size() > 1
			|| delaunayInnerDetail.size() > 1
			|| delaunayFalloffDetail.size() > 1
			|| mergeVertexEnabled.size() > 1
			|| mergeVertexDistance.size() > 1
			|| influenceActivated.size() > 1
			|| minPolygonSize.size() > 1
			|| maxPolygonSize.size() > 1;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateFields()
	{
		if( IsFbxVersion )
		{
			Ui->psdExportPathLineEdit->setText(this->GetParameters().FileExportPath);
			Ui->psdExportNameLineEdit->setText(this->GetParameters().FileExportName);
		}

		Ui->customGroupNameLineEdit->setText(this->GetParameters().AliasPsdName);

		//Ui->groupStructureComboBox->setCurrentIndex(this->GetParameters().KeepGroupStructure); // no longer supported

		Ui->textureProxyComboBox->setCurrentIndex(GetTextureProxyUi());
		Ui->depthModifierField->setValue(GetDepthUi());
		Ui->meshScaleSpinner->setValue(GetMeshScaleUi());
		Ui->writeModeComboBox->setCurrentIndex(GetWriteModeUi());
		Ui->writeLayoutComboBox->setCurrentIndex(GetWriteLayoutUi());

		if (Ui->layerList->selectedItems().size() < 1)
			return;

		// Write values to field (use latest selected)
		int layerIndex = GetLayerIndex( Ui->layerList->selectedItems()[0] );
		const SceneController& scene = GetScene();
		const LayerAgent& layerAgent = scene.GetLayerAgent(layerIndex);
		const LayerParameters& layerParams = GetLayerParameters(layerIndex);
		const LayerStats layerStats = layerAgent.GetLayerStats();


		// TODO: Each call to setValue writes the parameters to disk, too much writing
		if( Ui->billboardAlgorithmPanel->isVisible() )
		{
			Ui->billboardAlphaThreshField->setValue(layerParams.BillboardParameters.BillboardAlphaThresh);
		}

		if( IsFullVersion && Ui->delaunayAlgorithmPanel->isVisible() )
		{
			Ui->delaunayOuterDetailField->setValue(layerParams.DelaunayParameters.OuterDetail);
			Ui->delaunayInnerDetailField->setValue(layerParams.DelaunayParameters.InnerDetail);
			Ui->delaunayFalloffDetailField->setValue(layerParams.DelaunayParameters.FalloffDetail);
		}

		if( IsMayaVersion && Ui->linearAlgorithmPanel->isVisible() )
		{
			Ui->linearPrecisionField->setValue(layerParams.LinearParameters.LinearHeightPoly);
		}

		if( IsFullVersion && Ui->curveAlgorithmPanel->isVisible() )
		{
			// TODO: support this switch at runtime instead of using preprocessor flag
			Ui->mergeVerticesChk->setChecked(layerParams.CurveParameters.MergeVertexEnabled);
			Ui->mergeVertexDistanceField->setValue(layerParams.CurveParameters.MergeVertexDistance);
			Ui->mergeVertexDistanceGroup->setVisible(layerParams.CurveParameters.MergeVertexEnabled);
		}

		if( IsInfluenceSupported && Ui->influenceLayerPanel->isVisible() )
		{
			Ui->influenceLayerChk->setChecked(layerParams.EnableInfluence);
			Ui->minimumPolygonSizeField->setValue(layerParams.InfluenceParameters.MinPolygonSize);
			Ui->maximumPolygonSizeField->setValue(layerParams.InfluenceParameters.MaxPolygonSize);
		}

		// TODO: support this switch at runtime instead of using preprocessor flag
		Ui->generationAlgoComboBox->setCurrentIndex( GetGenerationAlgoEnumToComboBox(layerParams.Algo) );

		if( Ui->textureOutputPanel->isVisible() )
		{
			Ui->textureOutputSizeComboBox->setCurrentIndex(layerParams.EnableTextureCrop? 0:1); // first entry means crop, second means fullsize
		}

		if( IsAtlasVersion )
		{
			int atlasIndex = layerParams.AtlasIndex;
			AtlasParameters& atlasParams = GetAtlasParameters(atlasIndex);

			if( Ui->textureAtlasGroupPanel->isVisible() )
			{
				bool atlasNameEditable = (atlasIndex>=0);
				Ui->textureAtlasGroupComboBox->setEditable(atlasNameEditable); // editable unless set to "None", index -1
				Ui->textureAtlasGroupComboBox->setCurrentIndex(atlasIndex+1); // first entry means atlas unused, index -1 -> entry 0
			}

			if( Ui->textureAtlasSettingsPanel->isVisible() )
			{
				Ui->textureAtlasAlgoComboBox->setCurrentIndex( GetAtlasAlgoEnumToComboBox(atlasParams.packingAlgo) );
				Ui->textureAtlasOverrideChk->setChecked( atlasParams.isCustomSize );
				int fitSize = layerStats.atlasBoundsFit.WidthPixels();
				int customSize = 0, downscalePercent = 100, paddingPixels = 0;
				int customPadding = atlasParams.customPadding;
				if( atlasParams.isCustomSize && layerStats.isAtlasReady )
				{
					Ui->textureAtlasSizeGroup->setVisible(true);
					Ui->textureAtlasPaddingGroup->setVisible(true);
					// textureAtlasInfoFitGroup is always visible
					Ui->textureAtlasInfoScalingGroup->setVisible(true);
					Ui->textureAtlasInfoPaddingGroup->setVisible(true);
					customSize = atlasParams.customSize;
				}
				else
				{
					Ui->textureAtlasSizeGroup->setVisible(false);
					Ui->textureAtlasPaddingGroup->setVisible(false);
					// textureAtlasInfo1Group is always visible
					Ui->textureAtlasInfoScalingGroup->setVisible(false); // scaling info
					Ui->textureAtlasInfoPaddingGroup->setVisible(false);
					if( layerStats.isAtlasReady )
						customSize = fitSize;
				}
				if( customSize < fitSize )
				{
					downscalePercent = (int)round(100.0f * (customSize/(float)fitSize));
					Ui->textureAtlasInfoPaddingGroup->setVisible(false); // no padding if scaling
				}
				else if( customSize > fitSize )
				{
					paddingPixels = customSize - fitSize;
					Ui->textureAtlasInfoScalingGroup->setVisible(false); // no scaling if padding
				}
				else // exact fit
				{
					Ui->textureAtlasInfoPaddingGroup->setVisible(false); // no padding or scaling if exact fit
					Ui->textureAtlasInfoScalingGroup->setVisible(false);
				}
				QString customSizeStr, infoFitStr, infoScalingStr, infoPaddingStr;
				const char* calculatingStr = util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_INFO_OPTIMAL_SIZE_CALCULATING ); // "calculating..."
				const char* resDefaultStr = util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_INFO_CURRENT_SIZE_FORMAT ); // "Atlas Resolution %i x %i"
				const char* resCustomStr = util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_INFO_OPTIMAL_SIZE_FORMAT ); // "Optimal Atlas Resolution %i x %i"
				const char* scalingStr = util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_INFO_SCALING_FORMAT ); // "<font color='#f48c42'>Layers will be downscaled %i%% to fit</font>"
				const char* paddingStr = util::LocalizeString( IDC_MAIN, IDS_TEXTURE_ATLAS_INFO_PADDING_FORMAT ); // "<font color='#f48c42'>Padding of %i pixels will be applied</font>"
#if MAYA_API_VERSION>=20220000
				if( !layerStats.isAtlasReady )
				{
					infoFitStr.asprintf( calculatingStr );
				}
				else
				{
					customSizeStr.asprintf(  "x %i", customSize );
					infoFitStr.asprintf( (atlasParams.isCustomSize? resCustomStr : resDefaultStr), fitSize, fitSize );
					infoScalingStr.asprintf( scalingStr, downscalePercent );
					infoPaddingStr.asprintf( paddingStr, paddingPixels );
				}
#else
				if( !layerStats.isAtlasReady )
				{
					infoFitStr.sprintf( calculatingStr );
				}
				else
				{
					customSizeStr.sprintf(  "x %i", customSize );
					infoFitStr.sprintf( (atlasParams.isCustomSize? resCustomStr : resDefaultStr), fitSize, fitSize );
					infoScalingStr.sprintf( scalingStr, downscalePercent );
					infoPaddingStr.sprintf( paddingStr, paddingPixels );
				}
#endif
				if( atlasParams.isCustomSize && layerStats.isAtlasReady )
				{
					Ui->textureAtlasSizeSpin->setValue( customSize );
					Ui->textureAtlasSizeLabel->setText( customSizeStr );
					Ui->textureAtlasPaddingSpin->setValue( customPadding );
				}
				Ui->textureAtlasInfoFitLabel->setText( infoFitStr );
				Ui->textureAtlasInfoScalingLabel->setText( infoScalingStr );
				Ui->textureAtlasInfoPaddingLabel->setText( infoPaddingStr );
			}
		}
    }

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateButtons() const
	{
		QString prefixStr("Generate "); // TODO: Localize this
		QString countStr;
		int selectionEmpty = Ui->layerList->selectedItems().empty();
		if( selectionEmpty )
		{
			countStr = QString("All"); // TODO: Localize this
		}
		else
		{
			countStr = QString(std::to_string(Ui->layerList->selectedItems().size()).c_str());
		}

		//Ui->generateMeshBtn->setText( prefixStr + countStr + QString(" Meshes(s)") ); // TODO: Localize this
		//Ui->generatePngBtn->setText( prefixStr + countStr + QString(" PNG(s)") ); // TODO: Localize this
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateActiveLayers()
	{
		std::vector<LayerParameters*> selectedParams;
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters& params = GetLayerParameters(item);
			selectedParams.push_back(&params);
		}

		LayerAgentList layerAgents = this->GetScene().CollectLayerAgents(false);
		for( LayerAgent* const& layerAgent : layerAgents )
		{
			int layerIndex = iter_index(layerAgent,layerAgents);

			if( (layerAgent!=nullptr) && (!layerAgent->IsNull()) )
			{
				LayerParameters& layerParams = layerAgent->GetLayerParameters();
				auto it = std::find( selectedParams.begin(), selectedParams.end(), &layerParams );
				layerParams.IsActive = it != selectedParams.end() || selectedParams.empty();
				int i=0; i=1; // stub code for breakpoint
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateAtlasList()
	{
		if( !IsAtlasVersion )
			return;

		if( Ui->textureAtlasGroupComboBox == nullptr )
			return;

		Ui->textureAtlasGroupComboBox->setCurrentIndex(0);
		int i = Ui->textureAtlasGroupComboBox->count() - 2;
		for (; i>0; i--) // Remove all items except first and last one
		{	// keep removing from index 1, items shift down each time
			Ui->textureAtlasGroupComboBox->removeItem(1);
		}
		i = 1; // Insert starting after the first item
		const AtlasAgentList& atlasAgents = this->GetScene().GetAtlasAgents();
		for( auto atlasAgent : atlasAgents )
		{
			const AtlasParameters& atlasParams = atlasAgent->GetAtlasParameters();
			Ui->textureAtlasGroupComboBox->insertItem(i++, atlasParams.atlasName);
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// The combo index which matches the generation algorithm
	int ToolWidget::GetGenerationAlgoEnumToComboBox( int algo )
	{
		if( !IsLinearModeSupported )
		{
			if( algo==LayerParameters::Algorithm::BILLBOARD )
				return 0;
			if( algo==LayerParameters::Algorithm::DELAUNAY )
				return 1;
			if( algo==LayerParameters::Algorithm::CURVE )
				return 2;
		}
		else // IsLinearModeSupported
		{
			if( algo==LayerParameters::Algorithm::LINEAR )
				return 0;
			if( algo==LayerParameters::Algorithm::BILLBOARD )
				return 1;
			if( algo==LayerParameters::Algorithm::DELAUNAY )
				return 2;
			if( algo==LayerParameters::Algorithm::CURVE )
				return 3;
		}
		return 0; // shouldn't get here
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// The generation algorithm which matches the combo index
	int ToolWidget::GetGenerationAlgoComboBoxToEnum( int index )
	{
		if( !IsLinearModeSupported )
		{
			if( index==0 )
				return LayerParameters::Algorithm::BILLBOARD;
			if( index==1 )
				return LayerParameters::Algorithm::DELAUNAY;
			if( index==2 )
				return LayerParameters::Algorithm::CURVE;
		}
		else // IsLinearModeSupported
		{
			if( index==0 )
				return LayerParameters::Algorithm::LINEAR;
			if( index==1 )
				return LayerParameters::Algorithm::BILLBOARD;
			if( index==2 )
				return LayerParameters::Algorithm::DELAUNAY;
			if( index==3 )
				return LayerParameters::Algorithm::CURVE;
		}
		return LayerParameters::Algorithm::BILLBOARD; // shouldn't get here
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	// The combo index which matches the atlas packing algorithm
	int ToolWidget::GetAtlasAlgoEnumToComboBox( int algo )
	{
		if( algo==rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBestShortSideFit )
			return 0;
		if( algo==rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBestLongSideFit )
			return 1;
		if( algo==rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBestAreaFit )
			return 2;
		if( algo==rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBottomLeftRule )
			return 3;
		if( algo==rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectContactPointRule )
			return 4;
		return 0; // shouldn't get here
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// The combo index which matches the atlas packing algorithm
	int ToolWidget::GetAtlasAlgoComboBoxToEnum( int index )
	{
		if( index==0 )
			return rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBestShortSideFit;
		if( index==1 )
			return rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBestLongSideFit;
		if( index==2 )
			return rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBestAreaFit;
		if( index==3 )
			return rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectBottomLeftRule;
		if( index==4 )
			return rbp::MaxRectsBinPack::FreeRectChoiceHeuristic::RectContactPointRule;
		return 0; // shouldn't get here
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	int ToolWidget::GetLayerIndex(QListWidgetItem* item)
	{
		return this->WidgetMap.at(item);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	LayerParameters& ToolWidget::GetLayerParameters(QListWidgetItem* item)
	{
		int layerIndex = this->WidgetMap.at(item);
		return GetLayerParameters(layerIndex);
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	LayerParameters& ToolWidget::GetLayerParameters(int layerIndex)
	{
		LayerAgent& layerAgent = GetScene().GetLayerAgent(layerIndex);
		LayerParameters& layerParams = layerAgent.GetLayerParameters();
		return layerParams;
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	AtlasParameters& ToolWidget::GetAtlasParameters(int atlasIndex)
	{
		AtlasAgent& atlasAgent = GetScene().GetAtlasAgent(atlasIndex);
		AtlasParameters& atlasParams = atlasAgent.GetAtlasParameters();
		return atlasParams;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool ToolWidgetEventFilter::eventFilter(QObject *object, QEvent *event)
	{
		if (object == Widget && event->type() == QEvent::NonClientAreaMouseButtonDblClick) {
			return true;
		}
		return false;
	}


#pragma endregion
}
