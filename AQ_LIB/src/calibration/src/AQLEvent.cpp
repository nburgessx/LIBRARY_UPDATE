/*! @file
    @brief Event class (for multi thread)
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLEvent.cpp
//
//  DESCRIPTION :        Event class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLEvent.h"

// constructor
/*!

*/
AQLEvent::AQLEvent(void)
{
#ifdef WINDOWS
	mEvent = CreateEvent(0, false, false, 0);
#else
	pthread_mutexattr_t mattr;
	
	pthread_mutexattr_init(&mattr);
	pthread_mutex_init(&mMutex, &mattr);
	pthread_cond_init(&mReady, 0);
#endif	
}

// destructor
/*!

*/
AQLEvent::~AQLEvent(void)
{
#ifdef WINDOWS
	CloseHandle(mEvent);
#else
	pthread_cond_destroy(&mReady);
	pthread_mutex_destroy(&mMutex);
#endif
}


// 
/*!
    @brief notify an event to signaled

*/
void
AQLEvent::notify()
{
#ifdef WINDOWS
	SetEvent(mEvent);
#else
	pthread_mutex_lock(&mMutex);
	pthread_cond_signal(&mReady);
	pthread_mutex_unlock(&mMutex);
#endif
}

// 
/*!
    @brief notify an event to signaled

*/
void
AQLEvent::notifyAll()
{
#ifdef WINDOWS
	SetEvent(mEvent);
#else
	pthread_mutex_lock(&mMutex);
	pthread_cond_broadcast(&mReady);
	pthread_mutex_unlock(&mMutex);
#endif
}

// 
/*!
    @brief wait for an event -- wait for an event object

*/
bool
AQLEvent::wait()
{
#ifdef WINDOWS
	if (WaitForSingleObject(mEvent, INFINITE) != WAIT_OBJECT_0)
	{
		return false;
	}
	return true;
#else
	pthread_mutex_lock(&mMutex);
	pthread_cond_wait(&mReady, &mMutex);
	pthread_mutex_unlock(&mMutex);
	return true;
#endif
}

// 
/*!
    @brief reset an event flag to unsignaled

*/
void
AQLEvent::reset()
{
#ifndef WINDOWS
	pthread_mutex_unlock(&mMutex);
#endif
}

