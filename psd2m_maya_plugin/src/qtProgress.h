//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file parameters.h
//  @author Benjamin Drouin
//  @date 19-10-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef QT_PROGRESS_H
#define QT_PROGRESS_H

#include "ui_toolWidget.h"

namespace maya_plugin
{
	struct Progress
	{
		void SetProgressBar(QProgressBar* progressBar);
		void InitializeProgressBar(unsigned steps);
		void InitializeSubProgress(unsigned steps);
		void IncrementProgressBar();
		void CompleteSubProgress();
		void CompleteProgressBar();

	private:
		QProgressBar* ProgressBar = nullptr;
		unsigned TotalSteps = 1;
		unsigned CurrentStep = 0;
		unsigned TotalSubSteps = 0;
		unsigned CurrentSubStep = 0;
	};
}
#endif // QT_PROGRESS_H
