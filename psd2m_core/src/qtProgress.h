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

#include "util/progressJob.h"

class QString; // forward declaration
class QProgressBar; // forward declaration
class QProgressDialog; // forward declaration

namespace psd_to_3d
{
	class ProgressAgent : public util::ProgressJob
	{
	public:
		void SetProgressBar(QProgressBar* progressBar);
		void BeginProgressBar(const QString& windowTitle, unsigned taskCount, bool cancelButton=false);
		void EndProgressBar(bool okButton=false);
		void SetValue(float value); // directly set progress bar overall value, range [ 0.0, 1.0 ]

		//  from ProgressJob, virtual methods
		void Reset( int taskCount );
		void Update();
		void SetLabel( const char* text ); // TODO: Handle multibyte strings
		bool IsCancelled();
		//  from ProgressJob, nonvirtual methods
		// ProgressTask& GetTask();
		// int GetTaskTotal();
		// float GetTaskValue();
		// float GetJobValue();
		// void NextTask();

	private:
		QProgressDialog* ProgressDialog = nullptr;
		QProgressBar* ProgressBar = nullptr;
		bool cancelButton = false;
		unsigned long timeLastUpdate = 0;
	};
}
#endif // QT_PROGRESS_H
