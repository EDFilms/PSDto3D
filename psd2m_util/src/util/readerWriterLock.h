//----------------------------------------------------------------------------------------------
//
//  @file ReaderWriterLock.h
//  @author Michaelson Britt
//  @date 08-27-2016
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------

#ifndef READERWRITERLOCK_H
#define READERWRITERLOCK_H


#include "utils.h"

#include <intrin.h> // for __nop()
#include <thread>
#include <shared_mutex>

#define SIZEOF_CACHE_LINE 4096 // usually 4096 bytes on intel architecture
#define READERWRITERLOCK_IMPL_1

namespace util
{
	inline void Spin( int cycles_max=512 )
	{
		int cycles_count = (int)(cycles_max*(rand()/(float)RAND_MAX));
		for( int i=0; i<cycles_count; i++ )
			__nop();
	}

	//----------------------------------------------------------------------------------------
	// Reader Writer Lock, supporting one thread many writers
	// Safely allows a thread to obtain writer lock multiple times in nested code

#ifdef READERWRITERLOCK_IMPL_1
	class ReaderWriterLock
	{
		public:
			typedef union
			{
				__int64 qword;
				struct{
					unsigned long usage_count;
					unsigned long writer_thread;
				} dwords;
			} Data;
		
			Data* m_flags;
			long long acquire_writer_count;
			long long acquire_reader_count;
	
		public:
			inline ReaderWriterLock()
			{
				m_flags = (Data*)_aligned_malloc(sizeof(Data),SIZEOF_CACHE_LINE);
				if( m_flags!=nullptr ) m_flags->qword = 0;
				acquire_writer_count = 0;
				acquire_reader_count = 0;
			}
		
			inline ~ReaderWriterLock()
			{
				if( m_flags!=nullptr ) _aligned_free(m_flags);
			}

			inline void AcquireWriter()
			{
				if( m_flags==nullptr ) return;
				Data get, set, comparand;
				unsigned long self_thread = Windows_GetCurrentThreadId();
				bool is_busy = false;

				do
				{
					if( is_busy ) util::Spin(); // idle if busy

					// ideal case:
					//  current thread ID and usage count are zero (comparand)
					//  set thread ID to self and usage count to one
					comparand.dwords.usage_count = 0;
					comparand.dwords.writer_thread = 0;
					set.dwords.usage_count = 1;
					set.dwords.writer_thread = self_thread;
					get.qword = Windows_InterlockedCompareExchange64( (__int64*)m_flags,
						set.qword, comparand.qword ); // if comparand matches flags, assign set value to flags

					// self writer case: writer thread was same as self but usage was not zero
					if( (get.dwords.writer_thread==self_thread) && (get.dwords.usage_count!=0) )
					{
						// try again but increment the usage count
						comparand.dwords.writer_thread = self_thread;
						comparand.dwords.usage_count = get.dwords.usage_count;
						set.dwords.writer_thread = self_thread;
						set.dwords.usage_count = comparand.dwords.usage_count+1;
						get.qword = Windows_InterlockedCompareExchange64( (__int64*)m_flags,
							set.qword, comparand.qword ); // if comparand matches flags, assign set value to flags
						// if that still doesn't work (shouldn't happen), then get!=comparand, and loop continues ...
						is_busy = false; // in that case, continue the loop WITHOUT sleep before next check
					}
					else
					{
						// if the loop continues, it means the flags were in an unexpected state ...
						is_busy = true; // in that case, continue the loop WITH sleep before next check
					}
				} while( get.qword != comparand.qword );
				acquire_writer_count++;
			}
		
			inline void ReleaseWriter()
			{
				if( m_flags==nullptr ) return;
				Data get, set, comparand;
				unsigned long self_thread = Windows_GetCurrentThreadId();
				bool is_busy = false;
			
				do
				{
					if( is_busy ) util::Spin(); // idle if busy
				
					// ideal case:
					//  current thread ID is self and usage count is one (comparand)
					//  set thread ID to zero and usage count to zero
					comparand.dwords.usage_count = 1;
					comparand.dwords.writer_thread = self_thread;
					set.dwords.usage_count = 0;
					set.dwords.writer_thread = 0;				
					get.qword = Windows_InterlockedCompareExchange64( (__int64*)m_flags,
						set.qword, comparand.qword ); // if comparand matches flags, assign set value to flags

					// self writer case: writer thread was same as self but usage was not one
					if( (get.dwords.writer_thread==self_thread) && (get.dwords.usage_count!=1) )
					{
						// try again but decrement the usage count
						comparand.dwords.writer_thread = self_thread;
						comparand.dwords.usage_count = get.dwords.usage_count;
						set.dwords.writer_thread = self_thread;
						set.dwords.usage_count = comparand.dwords.usage_count-1;
						get.qword = Windows_InterlockedCompareExchange64( (__int64*)m_flags,
							set.qword, comparand.qword ); // if comparand matches flags, assign set value to flags
						// if that still doesn't work (shouldn't happen), then get!=comparand, and loop continues ...
						is_busy = false; // in that case, continue the loop WITHOUT sleep before next check
					}
					else
					{
						// if the loop continues, it means the flags were in an unexpected state ...
						is_busy = true; // in that case, continue the loop WITH sleep before next check
					}
				
				} while( get.qword != comparand.qword );
			}

			inline void AcquireReader()
			{
				if( m_flags==nullptr ) return;
				Data get, set, comparand;
				bool is_busy = false;

				do
				{
					if( is_busy ) util::Spin(); // idle if busy

					// ideal case:
					//  thread ID is zero and usage count is zero (comparand)
					//  set thread ID to zero and usage count to one
					comparand.dwords.usage_count = 0;
					comparand.dwords.writer_thread = 0;
					set.dwords.usage_count = 1;
					set.dwords.writer_thread = 0;
					get.qword = Windows_InterlockedCompareExchange64( (__int64*)m_flags,
						set.qword, comparand.qword ); // if comparand matches flags, assign set value to flags

					// many reader case: writer thread was zero but usage was not zero
					if( (get.dwords.writer_thread==0) && (get.dwords.usage_count!=0) )
					{
						// try again but increment the usage count
						comparand.dwords.usage_count = get.dwords.usage_count;
						set.dwords.usage_count = comparand.dwords.usage_count+1;
						get.qword = Windows_InterlockedCompareExchange64( (__int64*)m_flags,
							set.qword, comparand.qword ); // if comparand matches flags, assign set value to flags
						// if that still doesn't work (shouldn't happen), then get!=comparand, and loop continues ...
						is_busy = false; // in that case, continue the loop WITHOUT sleep before next check
					}
					else
					{
						// if the loop continues, it means the flags were in an unexpected state ...
						is_busy = true; // in that case, continue the loop WITH sleep before next check
					}
				} while( get.qword != comparand.qword );
				acquire_reader_count++;
			}
		
			inline void ReleaseReader()
			{
				if( m_flags==nullptr ) return;
				Data get, set, comparand;
				bool is_busy = false;

				do
				{
					if( is_busy ) util::Spin(); // idle if busy

					// ideal case:
					//  thread ID is zero and usage count is one (comparand)
					//  set thread ID to zero and usage count to zero
					comparand.dwords.usage_count = 1;
					comparand.dwords.writer_thread = 0;
					set.dwords.usage_count = 0;
					set.dwords.writer_thread = 0;
					get.qword = Windows_InterlockedCompareExchange64( (__int64*)m_flags,
						set.qword, comparand.qword ); // if comparand matches flags, assign set value to flags

					// many reader case: writer thread was zero but usage was not one
					if( (get.dwords.writer_thread==0) && (get.dwords.usage_count!=1) )
					{
						// try again but increment the usage count
						comparand.dwords.usage_count = get.dwords.usage_count;
						set.dwords.usage_count = comparand.dwords.usage_count-1;
						get.qword = Windows_InterlockedCompareExchange64( (__int64*)m_flags,
							set.qword, comparand.qword ); // if comparand matches flags, assign set value to flags
						// if that still doesn't work (shouldn't happen), then get!=comparand, and loop continues ...
						is_busy = false; // in that case, continue the loop WITHOUT sleep before next check
					}
					else
					{
						// if the loop continues, it means the flags were in an unexpected state ...
						is_busy = true; // in that case, continue the loop WITH sleep before next check
					}
				} while( get.qword != comparand.qword );				
			}
		
	};
#endif // READERWRITERLOCK_IMPL_1


#ifdef READERWRITERLOCK_IMPL_2
	//----------------------------------------------------------------------------------------
	// Helper for ReaderWriterLock
	struct LockMutexWriteGuard
	{
		std::shared_mutex& Mutex;
		LockMutexWriteGuard(std::shared_mutex& mutex) : Mutex(mutex) {Mutex.lock();}
		~LockMutexWriteGuard() {Mutex.unlock();}
	};

	struct ReaderWriterLock
	{
		std::shared_mutex Lock;
		std::thread::id Thread;
		int State;
		ReaderWriterLock() : State(0) {}
		int LockRead()
		{
			while( true )
			{
				{
					LockMutexWriteGuard guard(Lock);
					if( State>=0 )
					{
						return (++State); // Increment and return
					}
				}
				std::this_thread::yield(); // still waiting, let other threads run
			}
		}
		int UnlockRead()
		{
			LockMutexWriteGuard guard(Lock);
			return (--State); // Decrement and return
		}
		int LockWrite()
		{
			std::thread::id currentThread = std::this_thread::get_id();
			while( true )
			{
				{
					LockMutexWriteGuard guard(Lock);
					if( (State==0) || ((State<0) && (Thread==currentThread)) )
					{
						Thread = currentThread;
						return (--State); // Decrement and return
					}
				}
				std::this_thread::yield(); // still waiting, let other threads run
			}
		}
		int UnlockWrite()
		{
			LockMutexWriteGuard guard(Lock);
			return (++State); // Increment and return
		}
	};
#endif // READERWRITERLOCK_IMPL_2

}


//----------------------------------------------------------------------------------------
// Concurrency / Threadsafety helpers

#ifdef READERWRITERLOCK_IMPL_1
class LockWriteGuard {
	public:
		LockWriteGuard( util::ReaderWriterLock& lock ) : m_lock(lock) { m_lock.AcquireWriter(); }
		~LockWriteGuard( ) { m_lock.ReleaseWriter(); }
	protected:
		util::ReaderWriterLock& m_lock;
};
	
class LockReadGuard {
	public:
		LockReadGuard( util::ReaderWriterLock& lock ) : m_lock(lock) { m_lock.AcquireReader(); }
		~LockReadGuard( ) { m_lock.ReleaseReader(); }
	protected:
		util::ReaderWriterLock& m_lock;
};
#endif // READERWRITERLOCK_IMPL_1


#ifdef READERWRITERLOCK_IMPL_2
struct LockReadGuard
{
	util::ReaderWriterLock& Lock;
	LockReadGuard(util::ReaderWriterLock& lock) : Lock(lock) {Lock.LockRead();}
	~LockReadGuard() {Lock.UnlockRead();}
};

struct LockWriteGuard
{
	util::ReaderWriterLock& Lock;
	LockWriteGuard(util::ReaderWriterLock& lock) : Lock(lock) {Lock.LockWrite();}
	~LockWriteGuard() {Lock.UnlockWrite();}
};
#endif // READERWRITERLOCK_IMPL_2


#endif // READERWRITERLOCK_H
