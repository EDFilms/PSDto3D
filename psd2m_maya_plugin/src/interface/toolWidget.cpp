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
#include <qfiledialog>
#include <qdialog>
#include <qmessagebox>
#include <maya/MGlobal.h>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <set>

namespace maya_plugin
{
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
	ToolWidget::ToolWidget(QDockWidget *parent) :
		QDockWidget(parent), Ui(new Ui::DockWidget)
	{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
		qInstallMessageHandler(MessageRelay);
#else
		qInstallMsgHandler(MessageRelay);
#endif
		// Fill the current instance with the GUI information
		Ui->setupUi(this);

		// buttons
		connect(Ui->psdSelectorBtn, SIGNAL(clicked(bool)), this, SLOT(GetPath(bool)));
		connect(Ui->psdReloadBtn, SIGNAL(clicked(bool)), this, SLOT(ReloadPath(bool)));
		connect(Ui->generateBtn, SIGNAL(clicked(bool)), this, SLOT(GenerateMeshes(bool)));
		connect(Ui->exportBtn, SIGNAL(clicked(bool)), this, SLOT(ExportPng(bool)));

		// global settings
		connect(Ui->depthModifierField, SIGNAL(valueChanged(double)), this, SLOT(SetDepthModifier(const double &)));
		connect(Ui->meshScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(SetMeshScale(const int &)));
		connect(Ui->keepGroupStructureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetActiveKeepGroup(int)));
		connect(Ui->aliasPsdNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(SetAliasPsdName(QString)));
		

		// layerList
		connect(Ui->layerList, SIGNAL(itemSelectionChanged()), this, SLOT(SetSelectedLayers()));

		// algorithm
		connect(Ui->algoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetAlgorithm(const int &)));

		// linear settings
		connect(Ui->linearPrecisionField, SIGNAL(valueChanged(double)), this, SLOT(SetLinearPrecision(const double &)));
		connect(Ui->gridOrientationDial, SIGNAL(valueChanged(int)), this, SLOT(SetGridDirection(int)));

		// curve settings
		connect(Ui->mergeVertexDistanceField, SIGNAL(valueChanged(double)), this, SLOT(SetMergeDistance(const double &)));

		// influence settings
		connect(Ui->influenceTitle, SIGNAL(clicked(bool)), this, SLOT(SetActiveInfluence(bool)));
		connect(Ui->minimalPolygonSizeField, SIGNAL(valueChanged(double)), this, SLOT(SetMinPolygonSize(const double &)));
		connect(Ui->maximalPolygonSizeField, SIGNAL(valueChanged(double)), this, SLOT(SetMaxPolygonSize(const double &)));

		QRegExp rx("^\\S+$");
		QValidator *validator = new QRegExpValidator(rx, this);
		Ui->aliasPsdNameLineEdit->setValidator(validator);

		this->Progress.SetProgressBar(Ui->progressBar);
		
		UpdateUi();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	ToolWidget::~ToolWidget()
	{
		disconnect(Ui->psdSelectorBtn, SIGNAL(clicked(bool)), this, SLOT(GetPath(bool)));
		disconnect(Ui->psdReloadBtn, SIGNAL(clicked(bool)), this, SLOT(ReloadPath(bool)));
		disconnect(Ui->generateBtn, SIGNAL(clicked(bool)), this, SLOT(GenerateMeshes(bool)));
		disconnect(Ui->exportBtn, SIGNAL(clicked(bool)), this, SLOT(ExportPng(bool)));
		disconnect(Ui->depthModifierField, SIGNAL(valueChanged(double)), this, SLOT(SetDepthModifier(const double &)));
		disconnect(Ui->meshScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(SetMeshScale(const int &)));
		disconnect(Ui->keepGroupStructureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetActiveKeepGroup(int)));
		disconnect(Ui->aliasPsdNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(SetAliasPsdName(QString)));
		disconnect(Ui->layerList, SIGNAL(itemSelectionChanged()), this, SLOT(SetSelectedLayers()));
		disconnect(Ui->algoComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetAlgorithm(const int &)));
		disconnect(Ui->linearPrecisionField, SIGNAL(valueChanged(double)), this, SLOT(SetLinearPrecision(const double &)));
		disconnect(Ui->gridOrientationDial, SIGNAL(valueChanged(int)), this, SLOT(SetGridDirection(int)));
		disconnect(Ui->mergeVertexDistanceField, SIGNAL(valueChanged(double)), this, SLOT(SetMergeDistance(const double &)));
		disconnect(Ui->minimalPolygonSizeField, SIGNAL(valueChanged(double)), this, SLOT(SetMinPolygonSize(const double &)));
		disconnect(Ui->maximalPolygonSizeField, SIGNAL(valueChanged(double)), this, SLOT(SetMaxPolygonSize(const double &)));

		delete Ui;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetPsdData(psd_reader::PsdData & psdData)
	{
		QString sstr;
		sstr.append(std::to_string(psdData.HeaderData.Width).c_str());
		sstr.append(" x ");
		sstr.append(std::to_string(psdData.HeaderData.Height).c_str());
		Ui->selectedPsdSizeValue->setText(sstr);

		this->Data.UpdateLayers(psdData);

		// Fill model value
		Ui->layerList->clear();
		for (int r = 0; r < psdData.LayerMaskData.LayerCount; r++)
		{
			psd_reader::LayerData& layer = psdData.LayerMaskData.Layers[r];
			// folder or endfolder separator
			if (layer.Type > psd_reader::TEXTURE_LAYER) continue;

			QListWidgetItem* widgetItem = new QListWidgetItem();
			this->WidgetMap.try_emplace(widgetItem, layer.LayerName);
			Ui->layerList->addItem(widgetItem);

			ListFrame* frame = new ListFrame();
			frame->setFrameShape(QFrame::WinPanel);
			frame->setFrameShadow(QFrame::Raised);

			QVBoxLayout* layout = new QVBoxLayout;
			ApplyLayerDescription(layer, layout);
			frame->setLayout(layout);

			widgetItem->setSizeHint(frame->sizeHint());
			Ui->layerList->setItemWidget(widgetItem, frame);
		}

		UpdateUi();
	}

#pragma endregion

#pragma region ACCESSORS

	//--------------------------------------------------------------------------------------------------------------------------------------
	GlobalParameters& ToolWidget::GetParameters()
	{
		return this->Data;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	Progress& ToolWidget::GetProgress()
	{
		return this->Progress;
	}

#pragma endregion

#pragma region SLOTS

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetSelectedLayers()
	{
		UpdateUi();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::GenerateMeshes(bool /* checked */)
	{
		if (this->Data.FilePath.isEmpty()) return;

		this->Data.Generate = true;
		Refresh();

		this->Data.Generate = false;
	
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::ExportPng(bool /* checked */)
	{
		if (this->Data.FilePath.isEmpty()) return;

		this->Data.Exportation = true;
		Refresh();

		this->Data.Exportation = false;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::GetPath(bool /* checked */)
	{
		const auto path = QFileDialog::getOpenFileName(this, tr("Load Psd"), "/home", tr("psd file (*.psd)"));
		QFileInfo fi(path);
		if (path.isEmpty()) return;
		this->Data.SelectionChanged = true;
		this->Data.FilePath = path;
		this->Data.PsdName = fi.completeBaseName();
		this->Ui->psdReloadBtn->setEnabled(true);
		this->Data.UpdateValuesFromJson();
		this->Ui->selectedPsdNameValue->setText(fi.fileName());
		Refresh();

		this->Data.SelectionChanged = false;
		this->Data.Exported = false;
		this->Data.Generated = false;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::ReloadPath(bool /* checked */)
	{
		QFileInfo fi(this->Data.FilePath);
		if(!fi.exists())
		{
			QMessageBox messageBox;
			messageBox.critical(0, "Error", "File is missing, please use 'select' to specify the path of the file!");
			messageBox.setFixedSize(500, 200);
			return;
		}
		this->Data.SelectionChanged = true;
		this->Data.UpdateValuesFromJson();
		Refresh();

		this->Data.SelectionChanged = false;
		this->Data.Exported = false;
		this->Data.Generated = false;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetDepthModifier(const double value)
	{
		this->Data.Depth = value / 10.0f;
		this->Data.WriteValuesToJson();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	float ToolWidget::GetDephtUi()
	{
		return this->Data.Depth * 10.0f;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetMeshScale(const int value)
	{
		this->Data.Scale = (float(value) + 100.0f) / 100.0f;
		this->Data.WriteValuesToJson();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	float ToolWidget::GetMeshScaleUi()
	{		
		return this->Data.Scale * 100.0f - 100.f;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetActiveKeepGroup(const int value)
	{
		this->Data.KeepGroupStructure = value == 0;
		this->Data.WriteValuesToJson();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetAliasPsdName(const QString value)
	{
		this->Data.AliasPsdName = value;
		this->Data.WriteValuesToJson();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetLinearPrecision(const double value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = GetLayerParameters(item);
			params->LinearParameters.LinearHeightPoly = value;
			params->UpdateDescription();
		}

		this->Data.WriteValuesToJson();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetGridDirection(const int value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = GetLayerParameters(item);
			params->LinearParameters.GridOrientation = value;
			params->UpdateDescription();
		}

		this->Data.WriteValuesToJson();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetMergeDistance(const double& value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = GetLayerParameters(item);
			params->CurveParameters.MergeVertexDistance = value;
			params->UpdateDescription();
		}

		this->Data.WriteValuesToJson();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetActiveInfluence(bool value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = GetLayerParameters(item);
			params->InfluenceActivated = value;
			params->UpdateDescription();
		}

		this->Data.WriteValuesToJson();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetMinPolygonSize(const double& value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = GetLayerParameters(item);
			params->InfluenceParameters.MinPolygonSize = value;
			params->UpdateDescription();
		}

		this->Data.WriteValuesToJson();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetMaxPolygonSize(const double& value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = GetLayerParameters(item);
			params->InfluenceParameters.MaxPolygonSize = value;
			params->UpdateDescription();
		}

		this->Data.WriteValuesToJson();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::SetAlgorithm(const int value)
	{
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = GetLayerParameters(item);
			params->Algo = static_cast<LayerParameters::Algorithm>(value);
			params->UpdateDescription();
		}

		this->Data.WriteValuesToJson();
		UpdatePanels();

		this->Data.Generate = false;
		this->Data.Generated = false;
	}

#pragma endregion

#pragma region CONTROLLER

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::Refresh()
	{
		if (this->Controllers.empty()) return;
		for (auto controller : Controllers)
		{
			controller->Update();
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::AddController(IControllerUpdate* controller)
	{
		for (auto it : Controllers)
		{
			if (it == controller) return;
		}
		this->Controllers.push_back(controller);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::RemoveController(IControllerUpdate* controller)
	{
		for (auto it = Controllers.cbegin(); it != Controllers.cend(); ++it)
		{
			if ((*it) == controller) this->Controllers.erase(it);
		}
	}

#pragma endregion

#pragma region PRIVATE

	//--------------------------------------------------------------------------------------------------------------------------------------
	QString ToolWidget::ApplyLayerDescription(psd_reader::LayerData& layerData, QVBoxLayout* layout)
	{
		// TITLE
		QString titleLabel;
		titleLabel.append("<b>");
		titleLabel.append(layerData.LayerName.c_str());
		titleLabel.append("</b>");
		QFont font;
		font.setBold(true);
		font.setPointSize(12);
		QLabel* labelTitle = new QLabel(titleLabel);
		labelTitle->setFont(font);
		layout->addWidget(labelTitle);

		// SIZE
		QString sizeLabel;
		sizeLabel.append("Size: <b>");
		int size = layerData.AnchorBottom - layerData.AnchorTop;
		sizeLabel.append(std::to_string(size).c_str());
		sizeLabel.append(" x ");
		size = layerData.AnchorRight - layerData.AnchorLeft;
		sizeLabel.append(std::to_string(size).c_str());
		sizeLabel.append("</b>");
		QLabel* labelSize = new QLabel(sizeLabel);
		labelSize->setIndent(15);
		layout->addWidget(labelSize);
		
		// DESCRIPTION
		LayerParameters* layerParams = this->Data.GetLayerParameter(layerData.LayerName);
		layerParams->LabelAlgoSelected = new QLabel();
		layerParams->LabelAlgoSelected->setIndent(15);
		layout->addWidget(layerParams->LabelAlgoSelected);

		layerParams->LabelInfluence = new QLabel();
		layerParams->LabelInfluence->setIndent(15);
		layout->addWidget(layerParams->LabelInfluence);

		layerParams->LabelDescription = new QLabel();
		layerParams->LabelDescription->setIndent(15);
		layerParams->UpdateDescription();
		
		
		layout->addWidget(layerParams->LabelDescription);

		return sizeLabel;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateUi()
	{
		UpdatePanels();
		UpdateFields();
		UpdateButtons();
		UpdateActiveLayers();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdatePanels()
	{
		std::set<float> algorithms;
		bool anyInfluenceLayer = false;
		for (const auto& item : Ui->layerList->selectedItems())
		{
			const LayerParameters* params = GetLayerParameters(item);
			algorithms.insert(params->Algo);
			anyInfluenceLayer |= params->HasInfluenceLayer;
		}

		const int algo = algorithms.empty() ? -1 : *algorithms.begin();
		Ui->generationAlgoPanel->setVisible(!algorithms.empty());
		Ui->linearAlgorithmPanel->setVisible(algorithms.size() == 1 && algo == LayerParameters::Algorithm::LINEAR);
		Ui->curveAlgorithmPanel->setVisible(algorithms.size() == 1 && algo == LayerParameters::Algorithm::CURVE);
		Ui->multipleAlgoPanel->setVisible(algorithms.size() > 1);
		Ui->influenceLayerPanel->setVisible(!algorithms.empty() && anyInfluenceLayer);
		Ui->emptySettingsPanel->setVisible(algorithms.empty());
		Ui->multipleSettingsPanel->setVisible(!algorithms.empty() && AreSelectedValuesDifferent());
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	bool ToolWidget::AreSelectedValuesDifferent()
	{
		if (Ui->layerList->selectedItems().count() <= 1)
			return false;

		// Check if values are different through layers
		std::set<int> linearPrecision;
		std::set<float> gridOrientation;
		std::set<float> mergeVertex;
		std::set<bool> influenceActivated;
		std::set<float> minPolygonSize;
		std::set<float> maxPolygonSize;

		for (const auto& item : Ui->layerList->selectedItems())
		{
			const LayerParameters* params = GetLayerParameters(item);

			linearPrecision.insert(params->LinearParameters.LinearHeightPoly);
			gridOrientation.insert(params->LinearParameters.GridOrientation);
			mergeVertex.insert(params->CurveParameters.MergeVertexDistance);
			influenceActivated.insert(params->InfluenceActivated);
			minPolygonSize.insert(params->InfluenceParameters.MinPolygonSize);
			maxPolygonSize.insert(params->InfluenceParameters.MaxPolygonSize);
		}

		return  linearPrecision.size() > 1
			|| gridOrientation.size() > 1
			|| mergeVertex.size() > 1
			|| influenceActivated.size() > 1
			|| minPolygonSize.size() > 1
			|| maxPolygonSize.size() > 1;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateFields()
	{
		Ui->aliasPsdNameLineEdit->setText(this->Data.AliasPsdName);
		Ui->depthModifierField->setValue(GetDephtUi());
		Ui->keepGroupStructureComboBox->setCurrentIndex(this->Data.KeepGroupStructure);
		Ui->meshScaleSlider->setValue(GetMeshScaleUi());

		if (Ui->layerList->selectedItems().size() != 1)
			return;

		// Write values to field (use latest selected)
		const LayerParameters* params = GetLayerParameters(Ui->layerList->selectedItems()[0]);

		Ui->linearPrecisionField->setValue(params->LinearParameters.LinearHeightPoly);
		Ui->gridOrientationDial->setValue(params->LinearParameters.GridOrientation);
		Ui->mergeVertexDistanceField->setValue(params->CurveParameters.MergeVertexDistance);
		Ui->influenceTitle->setChecked(params->InfluenceActivated);
		Ui->minimalPolygonSizeField->setValue(params->InfluenceParameters.MinPolygonSize);
		Ui->maximalPolygonSizeField->setValue(params->InfluenceParameters.MaxPolygonSize);
		Ui->algoComboBox->setCurrentIndex(params->Algo);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateButtons() const
	{
		QString generateMesh;
		QString generatePng;
		generateMesh.append("Generate ");
		generatePng.append("Generate ");
		if (Ui->layerList->selectedItems().empty())
		{
			generateMesh.append("All");
			generatePng.append("All");
		}
		else
		{
			generateMesh.append(std::to_string(Ui->layerList->selectedItems().size()).c_str());
			generatePng.append(std::to_string(Ui->layerList->selectedItems().size()).c_str());
		}

		generateMesh.append(" Mesh(es)");
		generatePng.append(" PNG(s)");
		Ui->generateBtn->setText(generateMesh);
		Ui->exportBtn->setText(generatePng);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ToolWidget::UpdateActiveLayers()
	{
		std::vector<LayerParameters*> selectedParams;
		for (const auto& item : Ui->layerList->selectedItems())
		{
			LayerParameters* params = GetLayerParameters(item);
			selectedParams.push_back(params);
		}

		for (auto layerParam : this->Data.GetAllParameters())
		{
			auto it = std::find(selectedParams.begin(), selectedParams.end(), layerParam);
			layerParam->IsActive = it != selectedParams.end() || selectedParams.empty();
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	LayerParameters* ToolWidget::GetLayerParameters(QListWidgetItem* item)
	{
		return this->Data.GetLayerParameter(this->WidgetMap.at(item));
	}

#pragma endregion
}
