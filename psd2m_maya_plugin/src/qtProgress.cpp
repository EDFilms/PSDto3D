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

#include "qtProgress.h"

namespace maya_plugin
{
	//--------------------------------------------------------------------------------------------------------------------------------------
	void Progress::SetProgressBar(QProgressBar* progressBar)
	{
		this->ProgressBar = progressBar;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void Progress::InitializeProgressBar(unsigned steps)
	{
		if (this->ProgressBar == nullptr)
			return;

		this->TotalSteps = steps;
		this->CurrentStep = 0;
		this->TotalSubSteps = 0;
		this->CurrentSubStep = 0;
		this->ProgressBar->setValue(0);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void Progress::InitializeSubProgress(unsigned steps)
	{
		this->TotalSubSteps = steps;
		this->CurrentSubStep = 0;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void Progress::IncrementProgressBar()
	{
		if (this->ProgressBar == nullptr)
			return;

		if (this->TotalSubSteps > 0)
		{
			++this->CurrentSubStep;
		}
		else
		{
			++this->CurrentStep;
		}

		const float subValue = this->CurrentSubStep >= this->TotalSubSteps ? 0.f : float(this->CurrentSubStep) / this->TotalSubSteps;
		const float currentValue = ((float(this->CurrentStep) + subValue) / this->TotalSteps) * this->ProgressBar->maximum();
		this->ProgressBar->setValue(currentValue);
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void Progress::CompleteSubProgress()
	{
		this->TotalSubSteps = 0;
		this->CurrentSubStep = 0;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------
	void Progress::CompleteProgressBar()
	{
		if (this->ProgressBar == nullptr)
			return;

		this->ProgressBar->setValue(this->ProgressBar->maximum());
	}
}
