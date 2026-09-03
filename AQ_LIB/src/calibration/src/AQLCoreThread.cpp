
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLCoreThread.h"
#include "AQLCoreAppError.h"

#ifndef WINDOWS
#include <unistd.h>
#endif

// constructor
/*!

*/
AQLCoreThread::AQLCoreThread()
:mThread(0)
{
}

// destructor
/*!

*/
AQLCoreThread::~AQLCoreThread()
{
}

// 
/*!
    @brief start_routine
	@param[in] arg

*/
#ifdef WINDOWS
void 
AQLCoreThread::start_routine(LPVOID arg)
#else
void*
AQLCoreThread::start_routine(void* arg)
#endif
{
	if (!arg)
	{
#ifdef WINDOWS
		return;
#else
		return 0;
#endif
	}
	AQLCoreThread* p = reinterpret_cast<AQLCoreThread *>(arg);	
	p->run();

#ifndef WINDOWS
	return arg;
#endif
}

// 
/*!
    @brief start
*/
void
AQLCoreThread::start()
{
#ifdef WINDOWS
	//mThread = new HANDLE();
	mThread = reinterpret_cast<HANDLE>(_beginthread(AQLCoreThread::start_routine, 0, reinterpret_cast<LPVOID>(this)));
	if (!mThread)
	{
		throw AQLCoreInvalidData("Thread create failed.", __FILE__, __LINE__);
	}
#else
	//mThread = new pthread_t();
	int ret = pthread_create(&mThread, 0, AQLCoreThread::start_routine, this);
	if (ret != 0)
	{
		throw AQLCoreInvalidData("Thread create failed.", __FILE__, __LINE__);
	}
#endif
}

// 
/*!
    @brief join
*/
void
AQLCoreThread::join()
{
#ifdef WINDOWS
	CloseHandle(mThread);
#else
	pthread_join(mThread, 0);
#endif
}


// 
/*!
    @brief getCurrent thread ID
	@return ThreadID
*/
ThreadID
AQLCoreThread::getThreadID()
{
#ifdef WINDOWS
	return static_cast<ThreadID>(GetCurrentThreadId());
#else
	return static_cast<ThreadID>(pthread_self());
#endif
}

// 
/*!
    @brief sleep mthod
	@in millisecond
*/
void
AQLCoreThread::sleep(unsigned long milliseconds)
{
#ifdef WINDOWS
	Sleep(milliseconds);
#else
	::sleep(milliseconds / 1000);
#endif
}

