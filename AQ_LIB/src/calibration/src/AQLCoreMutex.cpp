#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLCoreMutex.h"

namespace
{
const int DEFAULTTHREADID = 0;
}
// constructor
/*!

*/
AQLCoreMutex::AQLCoreMutex(void)
: mOwnerID(DEFAULTTHREADID)
{
#ifdef WINDOWS
   mMutex = CreateMutex(0, false, 0);
#else
   pthread_mutexattr_t mattr;

   pthread_mutexattr_init(&mattr);
   pthread_mutex_init(&mMutex, &mattr);

#endif

}

// destructor
/*!

*/
AQLCoreMutex::~AQLCoreMutex(void)
{
#ifdef WINDOWS
	WaitForSingleObject(mMutex, INFINITE);
	CloseHandle(mMutex);
#else
	pthread_mutex_lock(&mMutex);
	pthread_mutex_unlock(&mMutex);
	pthread_mutex_destroy(&mMutex);
#endif
}

// 
/*!
    @brief lock method

*/
void
AQLCoreMutex::lock()
{
	ThreadID id = AQLCoreThread::getThreadID();
	// the mutex is already locked by this thread
	if(id == mOwnerID)
	{
		return;
	}
#ifdef WINDOWS
	WaitForSingleObject(mMutex,INFINITE);
#else
	pthread_mutex_lock(&mMutex);
#endif
	mOwnerID = AQLCoreThread::getThreadID();
}

// 
/*!
    @brief unlock

*/
void 
AQLCoreMutex::unlock()
{
	ThreadID id = AQLCoreThread::getThreadID();
	// on the thread that has locked the mutex can release the mutex
	if(id != mOwnerID)
	{
		return;
	}
	//memset(&mOwnerID, DEFAULTTHREADID, sizeof(ThreadId_t));
	mOwnerID = DEFAULTTHREADID;

#ifdef WINDOWS
	ReleaseMutex(mMutex);
#else
	pthread_mutex_unlock(&mMutex);
#endif
}

