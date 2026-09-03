#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LACoreMutex.h"

namespace
{
const int DEFAULTTHREADID = 0;
}
// constructor
/*!

*/
LACoreMutex::LACoreMutex(void)
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
LACoreMutex::~LACoreMutex(void)
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
LACoreMutex::lock()
{
	ThreadID id = LACoreThread::getThreadID();
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
	mOwnerID = LACoreThread::getThreadID();
}

// 
/*!
    @brief unlock

*/
void 
LACoreMutex::unlock()
{
	ThreadID id = LACoreThread::getThreadID();
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

