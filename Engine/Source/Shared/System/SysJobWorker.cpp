/**************************************************************************
*
* File:		SysJobWorker.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/SysJobWorker.h"
#include "System/SysJobQueue.h"
#include "System/SysKernel.h"
#include "Base/BcTimer.h"
#include "Base/BcProfiler.h"

#include <mutex>

//////////////////////////////////////////////////////////////////////////
// Ctor
SysJobWorker::SysJobWorker( class SysKernel* Parent, SysFence& StartFence ):
	Parent_( Parent ),
	StartFence_( StartFence ),
	Active_( BcTrue ),
	PendingJobSchedule_( 0 ),
	PendingJobQueue_( 0 ),
	JobQueueIndex_( 0 )
{
	// Start immediately.
	start();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SysJobWorker::~SysJobWorker()
{
	// Stop immediately.
	stop();

	//
	if( ExecutionThread_.joinable() )
	{
		ExecutionThread_.join();
	}
}

//////////////////////////////////////////////////////////////////////////
// start
void SysJobWorker::start()
{
	// Mark active.
	Active_ = BcTrue;

#if !PLATFORM_HTML5
	// Just start the thread.
	ExecutionThread_ = std::thread( &SysJobWorker::execute, this );
#else
	BcBreakpoint;
#endif
}

//////////////////////////////////////////////////////////////////////////
// stop
void SysJobWorker::stop()
{
	Active_ = BcFalse;
	WorkScheduled_.notify_all();
	ExecutionThread_.join();
}

//////////////////////////////////////////////////////////////////////////
// addJobQueue
void SysJobWorker::addJobQueue( SysJobQueue* JobQueue )
{
	BcAssert( BcIsGameThread() );

	std::lock_guard< std::mutex > Lock( JobQueuesLock_ );
	NextJobQueues_.push_back( JobQueue );
	PendingJobQueue_++;
	WorkScheduled_.notify_all();
}

//////////////////////////////////////////////////////////////////////////
// notifySchedule
void SysJobWorker::notifySchedule()
{
	PendingJobSchedule_++;
	WorkScheduled_.notify_all();
}

//////////////////////////////////////////////////////////////////////////
// debugLog
void SysJobWorker::debugLog()
{
	std::lock_guard< std::mutex > Lock( JobQueuesLock_ );

	PSY_LOG( "PendingJobQueue: %u", PendingJobQueue_.load() );
	PSY_LOG( "NextJobQueues: %u", NextJobQueues_.size() );
	PSY_LOG( "CurrJobQueues: %u", CurrJobQueues_.size() );
}

//////////////////////////////////////////////////////////////////////////
// execute
//virtual
void SysJobWorker::execute()
{
	PSY_LOGSCOPEDCATEGORY( "Worker" );

	// Mark as started.
	StartFence_.decrement();

	// Enter loop.
	while( Active_ )
	{
		PSY_PROFILER_SECTION( WaitSchedule_Profiler, "SysJobWorker_WaitSchedule" );

		// Wait to be scheduled.
		std::unique_lock< std::mutex > Lock( WorkScheduledMutex_ );
		WorkScheduled_.wait( Lock, [ this ]()
			{
				const BcBool PendingJobSchedule = PendingJobSchedule_.load() > 0;
				const BcBool PendingJobQueue = PendingJobQueue_.load() > 0;
				return PendingJobSchedule || PendingJobQueue || !Active_;
			} );

		PSY_PROFILER_SECTION( DoneSchedule_Profiler, "SysJobWorker_DoneSchedule" );

		// Check for a job queues update.
		if( PendingJobQueue_.load() > 0 )
		{
			std::lock_guard< std::mutex > Lock( JobQueuesLock_ );
			CurrJobQueues_.insert( CurrJobQueues_.end(), NextJobQueues_.begin(), NextJobQueues_.end() );

			// Wrap job queue index round to fit into new size.
			JobQueueIndex_ = JobQueueIndex_ % CurrJobQueues_.size();

			// Decrement by size of queue.
			PendingJobQueue_.fetch_sub( NextJobQueues_.size() );
			NextJobQueues_.clear();
		}

		// Grab job from current job queue.
		if( PendingJobSchedule_.load() > 0 )
		{
			SysJob* Job = nullptr;
			for( size_t Idx = 0; Idx < CurrJobQueues_.size(); ++Idx )
			{
				// Grab job queue.
				auto JobQueue( CurrJobQueues_[ JobQueueIndex_ ] );

				// Advance.
				JobQueueIndex_ = ( JobQueueIndex_ + 1 ) % CurrJobQueues_.size();

				// If we can pop, execute and break out.
				if( JobQueue->popJob( Job ) )
				{
					PSY_PROFILER_SECTION( ExecuteJob_Profiler, "SysJobWorker_ExecuteJob" );

					// Execute.
					try
					{
						Job->internalExecute();
					}
					catch( ... )
					{
						PSY_LOG( "Unhandled exception in job.\n" );
					}

					// Delete job.
					delete Job;

					// Tell job queue we've completed the job.
					JobQueue->completedJob();

					break;
				}
			}

			PendingJobSchedule_--;
		}
	}
}
