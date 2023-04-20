// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file parameters.cpp
//  @author Benjamin Drouin
//  @date 19-10-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

// TODO: Need this, otherwise error linking method QByteArray QString::toUtf8() const & Q_REQUIRED_RESULT
//#define QT_COMPILING_QSTRING_COMPAT_CPP // added in project settings

#include "parameters.h"
#include "scene_controller/sceneController.h"
#include "interface/ui_wrapper.h" // for version flags, like IsStandaloneVersion

typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <QString.h>
#include <QtGlobal>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include "util/helpers.h"
#if defined PSDTO3D_MAYA_VERSION
#include <maya/MGlobal.h>
#else
#include "mayaStub.h"
#endif

#include <QSettings>

namespace psd_to_3d
{

#pragma region LAYER PARAMETERS

	//--------------------------------------------------------------------------------------------------------------------------------------
	LayerParameters::LayerParameters( const QString& layerName )
	: LayerName(layerName), AnchorRegion(0,0,0,0)
	{
		if( !IsLinearModeSupported )
		{
			Algo = Algorithm::BILLBOARD;
		}
		for( int i=0; i<COMP_LAYER_COUNT; i++ )
		{
			CompLayerIndex[i] = -1;
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void LayerParameters::ClearWidgets()
	{
		ListFrame = nullptr;
		LabelLayerTitle = nullptr;
		LabelAlgoSelected = nullptr;
		LabelLayerSize = nullptr;
		LabelAtlasName =  nullptr;
		LabelAtlasSize = nullptr;
		LabelInfluence = nullptr;
		LabelDescription = nullptr;
	}


// LAYER PARAMETERS
#pragma endregion


#pragma region GLOBAL PARAMETERS

	//--------------------------------------------------------------------------------------------------------------------------------------
	void GlobalParameters::Reset()
	{
		this->TextureProxy = 1;
		this->Depth = 0;
		this->Scale = 1.0f;
		this->KeepGroupStructure = false;
		this->AliasPsdName = "";
		this->FileWriteMode = FileWriteMode::BINARY;
		this->FileWriteLayout = FileWriteLayout::SINGLE;

		this->FileExportPath = "";
		this->FileExportName = ""; // filename without extention
		this->FileExportExt = ""; // file extention
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void GlobalParameters::Fetch()
	{
		this->Prefs.Fetch();
		this->FileImportPath           = this->Prefs.FileImportPath;
		this->FileExportPath           = this->Prefs.FileExportPath;
	}

// GLOBAL PARAMETERS
#pragma endregion


#pragma region LICENSING PARAMETERS

	const char* optionDefaultUserInfoFirstName = "PSDto3D_UserInfoFirstName";
	const char* optionDefaultUserInfoLastName = "PSDto3D_UserInfoLastName";
	const char* optionDefaultUserInfoEmail = "PSDto3D_UserInfoEmail";
	const char* optionDefaultLicenseKey = "PSDto3D_LicenseKey";
#if defined PSDTO3D_MAYA_VERSION
	const char* optionDefaultFilename = "PSDtoMaya";
#else
	const char* optionDefaultFilename = "PSDtoFBX";
#endif
	const char* optionConfLanguage = "Language";
	const char* optionConfFilename = "language";


	//--------------------------------------------------------------------------------------------------------------------------------------
	void LicensingParameters::Reset()
    {
		this->UserInfoFirstName = "";
		this->UserInfoLastName = "";
		this->UserInfoEmail = "";
		this->LicenseKey = "";
    }

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Read internal data values from preferences
	void LicensingParameters::Fetch()
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EDFilms", optionDefaultFilename);
		QString empty("");
		this->UserInfoFirstName = settings.value( optionDefaultUserInfoFirstName, empty ).toString();
		this->UserInfoLastName = settings.value( optionDefaultUserInfoLastName, empty ).toString();
		this->UserInfoEmail = settings.value( optionDefaultUserInfoEmail, empty ).toString();
		this->LicenseKey = settings.value( optionDefaultLicenseKey, empty ).toString();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Write internal data values to preferences
	void LicensingParameters::Store()
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EDFilms", optionDefaultFilename);
		settings.setValue( optionDefaultUserInfoFirstName, this->UserInfoFirstName.toUtf8().data() );
		settings.setValue( optionDefaultUserInfoLastName, this->UserInfoLastName.toUtf8().data() );
		settings.setValue( optionDefaultUserInfoEmail, this->UserInfoEmail.toUtf8().data() );
		settings.setValue( optionDefaultLicenseKey, this->LicenseKey.toUtf8().data() );
	}

#pragma endregion
// LICENSING PARAMETERS [end]

#pragma region CONF PARAMETERS

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ConfParameters::Reset()
	{
		language = ConfParameters::english;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Read internal data values from ini
	void ConfParameters::Fetch()
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EDFilms", optionConfFilename);
		QString english("english");
		QString french("french");
		QString result = settings.value( optionConfLanguage, english ).toString();
		if( result.compare( french, Qt::CaseInsensitive ) == 0 )
			language = ConfParameters::french;
		else language = ConfParameters::english;
	}

#pragma 

// ATLAS PARAMETERS [begin]
#pragma region ATLAS PARAMETERS

	//--------------------------------------------------------------------------------------------------------------------------------------
	// returns true if two atlas params should return the same result, ignoring atlas names
	bool AtlasParameters::IsMatch( const AtlasParameters& that ) const
	{
		return (
			(this->layerIndices == that.layerIndices) &&
			(this->packingAlgo == that.packingAlgo) &&
			(this->isCustomSize == that.isCustomSize) &&
			( // unless isCustomSize is toggled off, the custom size and padding values must also match
			  (this->isCustomSize==false) ||
			  ( (this->customSize == that.customSize) && (this->customPadding == that.customPadding) )
			)
		);
	}

#pragma endregion
// ATLAS PARAMETERS [end]


// PREFERENCE PARAMETERS [begin]
#pragma region PREFERENCE PARAMETERS

	const char* optionDefaultFileImportPath = "PSDto3D_DefaultImportDir";
	const char* optionDefaultFileExportPath = "PSDto3D_DefaultExportDir";
	const char* optionDefaultDelaunayInnerDetailLo     = "PSDto3D_DelaunayInnerDetailLo";
	const char* optionDefaultDelaunayInnerDetailMid    = "PSDto3D_DelaunayInnerDetailMid";
	const char* optionDefaultDelaunayInnerDetailHi     = "PSDto3D_DelaunayInnerDetailHi";
	const char* optionDefaultDelaunayOuterDetailLo     = "PSDto3D_DelaunayOuterDetailLo";
	const char* optionDefaultDelaunayOuterDetailMid    = "PSDto3D_DelaunayOuterDetailMid";
	const char* optionDefaultDelaunayOuterDetailHi     = "PSDto3D_DelaunayOuterDetailHi";
	const char* optionDefaultDelaunayFalloffDetailLo   = "PSDto3D_DelaunayFalloffDetailLo";
	const char* optionDefaultDelaunayFalloffDetailMid  = "PSDto3D_DelaunayFalloffDetailMid";
	const char* optionDefaultDelaunayFalloffDetailHi   = "PSDto3D_DelaunayFalloffDetailHi";

	//--------------------------------------------------------------------------------------------------------------------------------------
	void PreferenceParameters::Reset()
    {
		this->FileImportPath = "";
		this->FileExportPath = "";
    }

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Read internal data values from preferences
	void PreferenceParameters::Fetch()
	{
		(*this) = PreferenceParameters(); // set values to default
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EDFilms", optionDefaultFilename);
		QString empty("");
		this->FileImportPath = settings.value( optionDefaultFileImportPath, empty ).toString();
		this->FileExportPath = settings.value( optionDefaultFileExportPath, empty ).toString();

		// TESTING ONLY: Store these on disk, to experiment with calibrating slider values at runtime
		//this->DelaunayInnerDetailLo    = settings.value( optionDefaultDelaunayInnerDetailLo,    this->DelaunayInnerDetailLo  ).toFloat();
		//this->DelaunayInnerDetailMid   = settings.value( optionDefaultDelaunayInnerDetailMid,   this->DelaunayInnerDetailMid ).toFloat();
		//this->DelaunayInnerDetailHi    = settings.value( optionDefaultDelaunayInnerDetailHi,    this->DelaunayInnerDetailHi  ).toFloat();
		//this->DelaunayOuterDetailLo    = settings.value( optionDefaultDelaunayOuterDetailLo,    this->DelaunayOuterDetailLo  ).toFloat();
		//this->DelaunayOuterDetailMid   = settings.value( optionDefaultDelaunayOuterDetailMid,   this->DelaunayOuterDetailMid ).toFloat();
		//this->DelaunayOuterDetailHi    = settings.value( optionDefaultDelaunayOuterDetailHi,    this->DelaunayOuterDetailHi  ).toFloat();
		//this->DelaunayFalloffDetailLo  = settings.value( optionDefaultDelaunayFalloffDetailLo,  this->DelaunayFalloffDetailLo  ).toFloat();
		//this->DelaunayFalloffDetailMid = settings.value( optionDefaultDelaunayFalloffDetailMid, this->DelaunayFalloffDetailMid ).toFloat();
		//this->DelaunayFalloffDetailHi  = settings.value( optionDefaultDelaunayFalloffDetailHi,  this->DelaunayFalloffDetailHi  ).toFloat();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// Write internal data values to preferences
	void PreferenceParameters::Store()
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "EDFilms", optionDefaultFilename);
		settings.setValue( optionDefaultFileImportPath, this->FileImportPath.toUtf8().data() );
		settings.setValue( optionDefaultFileExportPath, this->FileExportPath.toUtf8().data() );

		// TESTING ONLY: Store these on disk, to experiment with calibrating slider values at runtime
		//settings.setValue( optionDefaultDelaunayInnerDetailLo,  this->DelaunayInnerDetailLo );
		//settings.setValue( optionDefaultDelaunayInnerDetailMid, this->DelaunayInnerDetailMid );
		//settings.setValue( optionDefaultDelaunayInnerDetailHi,  this->DelaunayInnerDetailHi );
		//settings.setValue( optionDefaultDelaunayOuterDetailLo,  this->DelaunayOuterDetailLo );
		//settings.setValue( optionDefaultDelaunayOuterDetailMid, this->DelaunayOuterDetailMid );
		//settings.setValue( optionDefaultDelaunayOuterDetailHi,  this->DelaunayOuterDetailHi );
		//settings.setValue( optionDefaultDelaunayFalloffDetailLo,  this->DelaunayFalloffDetailLo );
		//settings.setValue( optionDefaultDelaunayFalloffDetailMid, this->DelaunayFalloffDetailMid );
		//settings.setValue( optionDefaultDelaunayFalloffDetailHi,  this->DelaunayFalloffDetailHi );

		settings.sync();
	}

// PREFERENCE PARAMETERS
#pragma endregion

#pragma region OUTPUTS

	GraphLayer GraphLayerGroup::null;

	GraphLayerGroup::~GraphLayerGroup()
	{
		for( GraphLayer*& graphLayer : graphLayerList )
		{
			if( graphLayer!=nullptr ) delete graphLayer;
			graphLayer = nullptr;
		}
		graphLayerList.clear();
	}

	GraphLayer* GraphLayerGroup::AllocGraphLayer( int layerIndex )
	{
		while( graphLayerList.size()<=layerIndex )
		{
			graphLayerList.push_back(nullptr);
		}
		if( graphLayerList[layerIndex]==nullptr ) graphLayerList[layerIndex] = new GraphLayer();
		return graphLayerList[layerIndex];
	}

// OUTPUTS
#pragma endregion

}

