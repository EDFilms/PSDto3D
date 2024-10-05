//----------------------------------------------------------------------------------------------
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

#include <Windows.h> // for GetTickCount()

typedef unsigned long ULONG; // TODO: Remove this; with older Maya vesions, and with Visual Studio 2017 toolset, Qt headers depend on windows.h or require this
#include <QProgressDialog.h>

#include "qtProgress.h"
#include "interface/ui_wrapper.h"

#include "ui_toolWidget.h"


#define PROGRESS_MAXIMUM 100

namespace psd_to_3d
{
	//--------------------------------------------------------------------------------------------------------------------------------------
	void ProgressAgent::SetProgressBar(QProgressBar* progressBar)
	{
		this->ProgressBar = progressBar;
		if( this->ProgressBar == nullptr )
			return;

		this->ProgressBar->setTextVisible(false);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ProgressAgent::BeginProgressBar(const QString& windowTitle, unsigned taskCount, bool cancelButton)
	{
		//if (this->ProgressBar == nullptr)
		//	return;

		if( this->ProgressBar != nullptr )
		{
			this->ProgressBar->setMinimum(0);
			this->ProgressBar->setMaximum(PROGRESS_MAXIMUM);
			this->ProgressBar->setValue(0);
			this->ProgressBar->setTextVisible(true);
			//this->ProgressBar->setAutoReset(false);
		}

		if( this->ProgressDialog == nullptr )
			delete this->ProgressDialog;
		this->ProgressDialog = new QProgressDialog( windowTitle, "Cancel", 0, PROGRESS_MAXIMUM, nullptr); // TODO: localize this
		this->ProgressDialog->setWindowModality(Qt::ApplicationModal); // make window modal, blocks mouse interaction with main window
		this->ProgressDialog->setWindowTitle( windowTitle ); // set the window title
		this->ProgressDialog->setMinimumDuration(0); // window appears immediately, no delay
		this->ProgressDialog->setAutoClose(false); // window stays open after progress is done
		this->ProgressDialog->setAutoReset(false); // window does not reset progress to 0% when it reaches 100%
		this->ProgressDialog->setMinimum(0);
		this->ProgressDialog->setMaximum(PROGRESS_MAXIMUM);
		this->ProgressDialog->setValue(0);
		this->ProgressDialog->setWindowFlags( this->ProgressDialog->windowFlags() & ~(Qt::WindowContextHelpButtonHint) ); // remove "help" button on title bar
		if( !cancelButton )
			this->ProgressDialog->setCancelButton(nullptr); // window has no cancel button (may still have OK button in EndProgressbar())
		this->cancelButton = cancelButton;

		this->Reset(taskCount);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void ProgressAgent::EndProgressBar(bool okButton)
	{
		//this->SetValue(1.0); // display progress as 100%
		if (this->ProgressBar != nullptr)
			this->ProgressBar->setValue(PROGRESS_MAXIMUM);

		if (this->ProgressDialog != nullptr)
			this->ProgressDialog->setValue(PROGRESS_MAXIMUM);


		if (this->ProgressBar != nullptr)
		{
			this->ProgressBar->setValue(this->ProgressBar->minimum());
			this->ProgressBar->setTextVisible(false);
		}
		if( this->ProgressDialog !=nullptr )
		{
			// don't close the progress dialog, instead display an OK button and let user close it
			if( okButton && !(this->ProgressDialog->wasCanceled()) ) // still close immediately if cancelled
			{
				if( !this->cancelButton ) // might need to set the cancel button if none existed
				{
					this->ProgressDialog->setCancelButton( new QPushButton() );
				}
				this->ProgressDialog->setCancelButtonText("OK"); // TODO: localize this
				this->ProgressDialog->setLabelText("Done!"); // TODO: localize this
			}
			else
			{
				delete this->ProgressDialog; // TODO: Safe to delete while still open?
				this->ProgressDialog = nullptr;
			}
		}
	}


	//--------------------------------------------------------------------------------------------------------------------------------------
	void ProgressAgent::SetValue( float value )
	{
		if (this->ProgressBar != nullptr)
			this->ProgressBar->setValue(value * (PROGRESS_MAXIMUM-1));

		if (this->ProgressDialog != nullptr)
			this->ProgressDialog->setValue(value * (PROGRESS_MAXIMUM-1));
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// From ProgressJob, virtual method override
	void ProgressAgent::Reset( int taskCount )
	{
		ProgressJob::Reset(taskCount); // call superclass method
		if (this->ProgressBar != nullptr)
			this->ProgressBar->setValue(0);

		if (this->ProgressDialog != nullptr)
			this->ProgressDialog->setValue(0);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// From ProgressJob, virtual method override
	void ProgressAgent::Update()
	{
		ProgressJob::Update(); // call superclass method
		SetValue( GetJobValue() );
		DWORD timeNow = GetTickCount();
		if( (timeNow-timeLastUpdate) > 1000 ) // update at maximum of 60fps, once per 16ms
		{
			if( this->ProgressDialog != nullptr )
				this->ProgressDialog->repaint();
			qApp->processEvents(); // repaint the window
			timeLastUpdate = timeNow;
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// From ProgressJob, virtual method override
	void ProgressAgent::SetLabel( const char* label )
	{
		ProgressJob::SetLabel(label); // call superclass method
		if( this->ProgressDialog != nullptr )
			this->ProgressDialog->setLabelText(label);
		Update();
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	// From ProgressJob, virtual method override
	bool ProgressAgent::IsCancelled()
	{
		if( this->ProgressDialog != nullptr )
		{
			qApp->processEvents(); // check for mouse click
			this->SetCancelled( this->ProgressDialog->wasCanceled() );
		}
		return ProgressJob::IsCancelled();
	}


}
