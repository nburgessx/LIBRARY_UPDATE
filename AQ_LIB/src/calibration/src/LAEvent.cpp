/*! @file
    @brief Event class (for multi thread)
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MAEvent.cpp
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

#include "LAEvent.h"

// constructor
/*!

*/
MAEvent::MAEvent(void)
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
MAEvent::~MAEvent(void)
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
MAEvent::notify()
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
MAEvent::notifyAll()
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
MAEvent::wait()
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
MAEvent::reset()
{
#ifndef WINDOWS
	pthread_mutex_unlock(&mMutex);
#endif
}

