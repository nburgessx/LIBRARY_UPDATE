/*! @file
    @brief LACalibratePool class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibratePool.cpp
//
//  DESCRIPTION :       LACalibratePool 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <algorithm>
#include "LACoreAppError.h"
#include "LACalibratePool.h"
#include "LACalibrationThread.h"
#ifdef __HAS_MIC__

#endif

using namespace std;

LACalibratePool *LACalibratePool::mpInstance = 0;

//================ LACalibratePool ===================================
// constructor
/*!

*/
LACalibratePool::LACalibratePool(void)
:mTail(0), mHead(0), mCount(0)
{
}

// destructor
/*!

*/
LACalibratePool::~LACalibratePool(void)
{

	// thread terminate
	terminateThread();

	deleteRequestQueue();
}

// 
/*!
    @brief get unique instance

	@return  LACalibratePool *
*/
LACalibratePool *
LACalibratePool::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new LACalibratePool();
	}
	return mpInstance;
}

// 
/*!
    @brief set max request num
*/
void
LACalibratePool::setMaxRequest(unsigned int num)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::Mutex> lock(mMutex);
#endif
	mRequestQueue.clear();
	mTail = 0;
	mHead = 0;
	mCount = 0;
	mRequestQueue.resize(num);
}

// 
/*!
    @brief set thread num
*/
void
LACalibratePool::setThreadNum(unsigned int num)
{
	// thread terminate
	terminateThread();

	resizeThreads(num);
}


// 
/*!
    @brief set start thread
*/
void
LACalibratePool::startThread()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::Mutex> lock(mMutex);
#endif
	unsigned int size = static_cast<unsigned int>(mThreads.size());
	for (unsigned int i = 0; i < size; ++i)
	{
		if (mThreads[i])
		{
			mThreads[i]->start();
		}
	}
}

// 
/*!
    @brief set calibration request

	@param[in] request
*/
void
LACalibratePool::putRequest(LACalibrate *request)
{
	if (!request)
	{
		throw LACoreInvalidData("Request pointer is NULL", __FILE__, __LINE__);
	}
#ifdef __HAS_MIC__
	mMutex.lock();
#endif
	while (mCount >= mRequestQueue.size() || mRequestQueue[mTail])
	{
#ifdef __HAS_MIC__
		mMutex.unlock();
#endif
		mEvent.wait();
#ifdef __HAS_MIC__
		mMutex.lock();
#endif
	}

	mRequestQueue[mTail] = request;
	mTail = (mTail + 1) % mRequestQueue.size();
	++mCount;
	mEvent.notifyAll();
	
#ifdef __HAS_MIC__
	mMutex.unlock();
#endif
}


// 
/*!
    @brief release calibration request

	@param[in] request
*/
void
LACalibratePool::releaseRequest(LACalibrate *request)
{
	if (!request)
	{
		throw LACoreInvalidData("Request pointer is NULL", __FILE__, __LINE__);
	}
#ifdef __HAS_MIC__
	mMutex.lock();
#endif
	vector<LACalibrate *>::iterator it = 
		find(mRequestQueue.begin(), mRequestQueue.end(), request);

	if (it == mRequestQueue.end())
	{
		throw LACoreInvalidData("Request pointer is not registed in requestqueue", __FILE__, __LINE__);
	}
	delete (*it);
	(*it) = 0;
	mEvent.notifyAll();

#ifdef __HAS_MIC__
	mMutex.unlock();
#endif
}

// 
/*!
    @brief take calibration request

	@return request
*/
LACalibrate *
LACalibratePool::takeRequest(void)
{
#ifdef __HAS_MIC__
	mMutex.lock();
#endif
	while (mCount <= 0)
	{
#ifdef __HAS_MIC__
		mMutex.unlock();
#endif
		mEvent.wait();
#ifdef __HAS_MIC__
		mMutex.lock();
#endif
	}

	LACalibrate *request = mRequestQueue[mHead];
	mHead = (mHead + 1) % mRequestQueue.size();
	--mCount;
	mEvent.notifyAll();

#ifdef __HAS_MIC__
	mMutex.unlock();
#endif
	return request;
}

// Terminate Thread
/*!

*/
void
LACalibratePool::terminateThread()
{
#ifdef __HAS_MIC__
	mMutex.lock();
#endif
	unsigned int threadSize = static_cast<unsigned int>(mThreads.size());
	for (unsigned int i = 0; i < threadSize; ++i)
	{
		mThreads[i]->doTerminate();
	}

	if (threadSize > mCount)
	{
		// put terminate request
		unsigned int terminateReqSize = threadSize - mCount; 
		for (unsigned int i = 0; i < terminateReqSize; ++i)
		{
			LACalibrate *terminateReq = new LACalibrateTerminate();
			putRequest(terminateReq); 	
		}
	}

#ifdef __HAS_MIC__
	mMutex.unlock();
#endif
#ifdef WINDOWS
	HANDLE *tmpArray = new HANDLE[threadSize];
	for (unsigned int i = 0; i < threadSize; ++i)
	{
		tmpArray[i] = mThreads[i]->mThread;
	}

	WaitForMultipleObjects(threadSize, tmpArray, true, INFINITE);
	delete[] tmpArray;
#endif

	// join
	for (unsigned int i = 0; i < threadSize; ++i)
	{
		mThreads[i]->join();
		delete mThreads[i];
	}

	
}

// Resize Thread
void
LACalibratePool::resizeThreads(unsigned int num)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::Mutex> lock(mMutex);
#endif
	mThreads.resize(num);
	for (unsigned int i = 0; i < num; ++i)
	{
		mThreads[i] = new MACalibrationThread(this);
	}
}

// Delete Request Queue
void
LACalibratePool::deleteRequestQueue()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::Mutex> lock(mMutex);
#endif
	unsigned int requestSize = static_cast<unsigned int>(mRequestQueue.size());
	for (unsigned int i = 0; i < requestSize; ++i)
	{
		if (mRequestQueue[i])
		{
			delete mRequestQueue[i];
		}
	}
}
