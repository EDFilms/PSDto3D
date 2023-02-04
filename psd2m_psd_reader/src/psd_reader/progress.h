//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2018, CDRIN.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file progress.h
//  @author Benjamin Drouin
//  @date 19-10-2018
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef PROGRESS_H
#define PROGRESS_H

#include <functional>

namespace psd_reader
{
	class PsdProgress
	{
	public:
		PsdProgress() = default;
		PsdProgress(std::function<void(unsigned)>& initializeProgress, std::function<void(unsigned)>& initializeSubProgress, std::function<void()>& incrementProgress, std::function<void()>& completeSubProgress)
		{
			this->FuncInitializeProgress = initializeProgress;
			this->FuncInitializeSubProgress = initializeSubProgress;
			this->FuncIncrementProgress = incrementProgress;
			this->FuncCompleteSubProgress = completeSubProgress;
		}

		void InitializeProgress(unsigned n) const
		{
			if (this->FuncInitializeProgress)
			{
				this->FuncInitializeProgress(n);
			}
		}

		void InitializeSubProgress(unsigned n) const
		{
			if (this->FuncInitializeSubProgress)
			{
				this->FuncInitializeSubProgress(n);
			}
		}

		void IncrementProgress() const
		{
			if (this->FuncIncrementProgress)
			{
				this->FuncIncrementProgress();
			}
		}

		void CompleteSubProgress() const
		{
			if (this->FuncCompleteSubProgress)
			{
				this->FuncCompleteSubProgress();
			}
		}

	private:
		std::function<void(unsigned)> FuncInitializeProgress;
		std::function<void(unsigned)> FuncInitializeSubProgress;
		std::function<void()> FuncIncrementProgress;
		std::function<void()> FuncCompleteSubProgress;
	};
}
#endif // PROGRESS_H
