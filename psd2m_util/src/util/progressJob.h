//----------------------------------------------------------------------------------------------
//
//  @file ProgressJob.h
//  @author Michaelson Britt
//  @date 02-18-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef PROGRESSJOB_H
#define PROGRESSJOB_H

#include "util/helpers.h"

namespace util
{
	class ProgressJob;

	//----------------------------------------------------------------------------------------
	// Progress Task
	class ProgressTask
	{
	public:
		ProgressJob* parent;
		ProgressTask( ProgressJob* parent ) : parent(parent), value(0.0f) {}
		virtual ~ProgressTask()							{}
		virtual inline void Reset()						{ value = 0.0f; }
		virtual inline void Update();
		virtual inline bool IsCancelled();
		virtual inline float GetValue()					{ return value; }
		virtual inline void  SetValue( float v )		{ this->value = v; }
		virtual inline void SetValueAndUpdate( float v )
		{
			this->value = v;
			Update();
		}
	protected:
		float value; // range [ 0.0, 1.0 ]
	};

	//----------------------------------------------------------------------------------------
	// Progress Job
	//
	// Progress reporting for a fixed number of tasks (integer),
	// with arbitrary progress value reported within each task (floating point)
	//
	class ProgressJob
	{
	public:
		inline ProgressJob();
		virtual inline ~ProgressJob() {}

		virtual inline void Reset( int task_count );
		virtual inline void Update()					{}
		virtual inline void SetLabel( const char* )		{} // default implementation ignores label // TODO: Handle multibyte strings
		virtual inline void SetCancelled( bool b )		{ is_cancelled=b; }
		virtual inline bool IsCancelled()				{ return is_cancelled; }

		inline ProgressTask& GetTask()					{ return task_cur; }
		inline int GetTaskTotal()						{ return total_count; }
		inline float GetTaskValue()						{ return MIN( 1.0f, task_cur.GetValue() ); } // return value in range [0.0,1.0]
		inline float GetJobValue()						{ return MIN( 1.0f, ( (done_count + GetTaskValue()) / (float)total_count ) ); } // return value in range [0.0,1.0]
		inline void NextTask();

	protected:
		int total_count;
		int done_count;
		bool is_cancelled;
		ProgressTask task_cur;
	};

	//----------------------------------------------------------------------------------------
	// Progress Job
	// Inline method implementations
	ProgressJob::ProgressJob() : total_count(0), done_count(0), is_cancelled(false), task_cur(this)
	{
	}
	void ProgressJob::Reset( int task_count )
	{
		this->total_count = task_count;
		this->done_count = 0;
		this->is_cancelled = false;
		if( total_count<=0 ) total_count=1;
		task_cur.Reset();
	}
	void ProgressJob::NextTask()
	{
		task_cur.Reset();
		done_count++;
		Update();
	}
	//----------------------------------------------------------------------------------------
	// Progress Task
	// Inline method implementations
	void ProgressTask::Update()
	{
		if( parent!=nullptr )
			parent->Update();
	}
	void DebugPrint(char* lpszFormat, ...); // TODO: Delete this, testing only
	bool ProgressTask::IsCancelled()
	{
		if( parent!=nullptr )
			return parent->IsCancelled();
		DebugPrint("Error: ProgressTask::IsCancelled(), null parent pointer\n");
		return false; // default
	}


} // namespace util

#endif // PROGRESSJOB_H