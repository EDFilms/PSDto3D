//----------------------------------------------------------------------------------------------
//
//  @file Threadpool.h
//  @author Michaelson Britt
//  @date 01-25-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "readerWriterLock.h"

#include <stdio.h>
#include <conio.h>
#include <vector>
#include <thread>
#include <chrono>
#include <shared_mutex>


namespace util
{
	extern ReaderWriterLock hack_lock;
	extern int hack_cycles_work;
	extern int hack_cycles_idle;
	extern int hack_cycles_launch;

	//----------------------------------------------------------------------------------------
	// Thread Pool
	class ThreadPool
	{
	public:
		typedef void (*ThreadFunc)( void* param );
		ReaderWriterLock lock;
		int thread_count_max;
		int thread_count_running;

		ThreadPool();
		int Run( ThreadFunc func, void* param );
		void Wait();

	protected:
		struct ThreadData
		{
			std::thread thread;
			ReaderWriterLock lock;
			ThreadPool* parent;
			int index;
			int state; // 0 starting or idle, 1 running, 2 stopping
			ThreadFunc func;
			void* param;
			// padding to match cache size can result in 5x performance increase..
			unsigned char padding[64]; // pad to total struct size of 128 bytes on x64 architecture

			ThreadData( ThreadPool* parent, int index )
			: parent(parent), index(index), state(0), func(nullptr), param(nullptr)
			{
				thread = std::thread( ThreadPool::ThreadFuncWrapper, this );
				thread.detach();
			}
		};
		std::vector<ThreadData*> threads; // all threads
		std::vector<ThreadData*> queue; // available threads

		void CreateThreadData( int index ); // returns index
		void DestroyThreadData( int index ); // pass index
		static void ThreadFuncWrapper( ThreadData* thread_data );
	};

} // namespace util

#endif // THREADPOOL_H