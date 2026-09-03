#pragma once

#ifndef WINDOWS
#if defined(WIN32) || defined(WIN64)
#define WINDOWS
#endif
#endif

#ifndef WINDOWS
#include <pthread.h>
#else
#include <windows.h>
#include <process.h>
#endif

#ifdef WINDOWS
typedef long ThreadID;
#else
typedef pthread_t ThreadID;
#endif

class AQLCalibratePool;
class AQLUSSocketRequestPool;

class AQLCoreThread
{
public:
	// constructor
	explicit AQLCoreThread();
	// destructor
	virtual ~AQLCoreThread();
	//==============================================
	// start
	virtual void start();
	//==============================================
	// join
	virtual void join();
	//==============================================
	// get ThreadID
	static ThreadID getThreadID();
	//==============================================
	// get ThreadID
	static void sleep(unsigned long milliseconds);
	
protected:
	//==============================================
	// run
	virtual void run() = 0;
	
	//==============================================
	// thread 
#ifdef WINDOWS
	HANDLE mThread;
#else
	pthread_t mThread;
#endif

private:
friend class AQLCalibratePool;
friend class AQLUSSocketRequestPool;
	//==============================================
	// start routine
#ifdef WINDOWS
	static void start_routine(LPVOID arg);
#else
	static void* start_routine(void* arg);
#endif

private:
	// copy constructor
	AQLCoreThread(const AQLCoreThread &rhs);
	AQLCoreThread &operator=(const AQLCoreThread &rhs);

	
};
