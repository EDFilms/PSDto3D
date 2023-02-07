//----------------------------------------------------------------------------------------------
//
//  @file Threadpool.cpp
//  @author Michaelson Britt
//  @date 01-25-2021
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#include "threadpool.h"

#include <stdio.h>
#include <conio.h>
#include <vector>
#include <thread>
#include <chrono>
#include <shared_mutex>

#define THREAD_COUNT_DEFAULT 16

using namespace std::chrono_literals;
typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;

namespace util
{

	ThreadPool::ThreadPool()
	: thread_count_max(0), thread_count_running(0)
	{
		int check_size = sizeof(ThreadData); // debugging only
		thread_count_max = std::thread::hardware_concurrency();
		if( thread_count_max==0 ) thread_count_max = THREAD_COUNT_DEFAULT;
		// create threads later on demand
	}

	int ThreadPool::Run( ThreadFunc func, void* param )
	{
		bool done = false;
		int thread_index = -1;
		while( !done )
		{
			{	// enter new scope for lock
				LockWriteGuard guardGlobal(lock); // hold global lock
				// 1) If fewer than optimum number of threads exist, create threads and add to queue
				if( threads.size()<thread_count_max )
				{
					thread_index = (int)(threads.size());
					CreateThreadData( thread_index );
					queue.push_back( threads[thread_index] ); // add to queue
				}
        
				// 2) Grab an available thread from queue
				for( int i=0; i<queue.size(); i++ )
				{
					ThreadData* thread_data = queue.back();
					queue.pop_back(); // remove from queue
					LockWriteGuard guardThread(thread_data->lock); // hold thread lock while setting task
					if( thread_data->state==0 ) // item should always be idle, but check just in case
					{
						thread_index = thread_data->index;
						thread_data->state = 1;
						thread_data->func = func;
						thread_data->param = param;
						thread_index = thread_data->index;
						// thread will run the task immediately once its lock is released
					}
					if( thread_index>=0 ) break; // found one, return it
				}

				// 3) No threads in queue, but threads count is maximum, hmm...
				// Check if any threads have timed out and are set to null
				for( int i=0; (thread_index<0) && (i<thread_count_max); i++ )
				{
					if( threads[i]==nullptr )
					{
						thread_index = i;
						CreateThreadData( thread_index );
						// don't add to queue, will be used immediately
					}
				}

				// 4) If thread found, assign the task
				if( thread_index>=0 )
				{
					ThreadData* thread_data = threads[thread_index];
					LockWriteGuard guardThread(thread_data->lock); // hold thread lock while updating task
					thread_data->state = 1;
					thread_data->func = func;
					thread_data->param = param;
					thread_count_running++;
					done = true;
				}
			}
        
			// 5) If no resources available, wait and repeat
			if( thread_index<0 )
			{
				std::this_thread::yield();
			}
		}
		return thread_index;
	}

	void ThreadPool::Wait()
	{
		bool done = false;
		while( !done )
		{
			bool isRunning = false;
			{	// enter new scope for lock
				LockWriteGuard guardGlobal(lock); // hold global lock
				isRunning = (thread_count_running!=0);
				if( !isRunning )
					done = true;
			}
		}
	}

	void ThreadPool::CreateThreadData( int thread_index )
	{
		LockWriteGuard guardGlobal(lock); // hold global lock
		ThreadData* thread_data = new ThreadData(this,thread_index);
		thread_data->parent = this;
		thread_data->index = thread_index;
		while( threads.size()<=thread_index ) threads.push_back(nullptr);
		threads[thread_index] = thread_data;
	}

	void ThreadPool::DestroyThreadData( int thread_index )
	{
		LockWriteGuard guardGlobal(lock); // hold global lock
		ThreadData* thread_data = threads[thread_index];
		threads[thread_index]=nullptr;
		for( int i=(int)(queue.size()-1); i>=0; i-- )
		{
			if( queue[i]==thread_data )
				queue.erase(queue.begin()+i);
		}
		delete thread_data;
	}

	void ThreadPool::ThreadFuncWrapper( ThreadData* thread_data )
	{
		ThreadPool* parent = thread_data->parent;
		ThreadFunc func = nullptr;
		void* param = nullptr;
		time_point time_sleep = std::chrono::high_resolution_clock::now();
		bool destroy_now = false;
		bool sleep_now = false;

		bool done = false;
		while( !done )
		{
			{	// enter new scope for lock
				LockWriteGuard guardThread(thread_data->lock); // hold thread lock while updating task
				time_point time_now = std::chrono::high_resolution_clock::now();
				std::chrono::duration<float> time_diff = time_now-time_sleep;
				if( time_diff.count()>1.0f )
				{
					// after a timeout, end this thread and destroy the data object;
					// the pool will create another later as needed
					thread_data->state=2;
					destroy_now = true;
				}
				else if( thread_data->state==1 )
				{
					func = thread_data->func;
					param = thread_data->param;
				}
				else
				{
					sleep_now = true;
				}
			}

			// actions after unlocking
			if( destroy_now )
			{
				parent->DestroyThreadData( thread_data->index );
				done = true;
			}
			else if( sleep_now )
			{
				// doesn't work, poor performance
				//std::this_thread::yield();
				//util::Spin(512);
				sleep_now = false;
			}
        
			if( func!=nullptr )
			{
				func( param ); // RUN THE TASK
            
				// Finished the task, update thread pool
				{	// enter new scope for lock
					LockWriteGuard guardGlobal(parent->lock); // hold global lock
					LockWriteGuard guardThread(thread_data->lock); // hold thread lock while updating task
					thread_data->state = 0;
					thread_data->func = func = nullptr;
					thread_data->param = param = nullptr;
					// add thread to the idle queue
					parent->queue.push_back( thread_data );
					parent->thread_count_running--;
				}
				time_sleep = std::chrono::high_resolution_clock::now();
			}
		}
	}

} // namespace util