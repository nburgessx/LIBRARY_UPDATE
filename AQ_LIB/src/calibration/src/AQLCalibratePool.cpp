/*! @file
    @brief AQLCalibratePool class
*/
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <algorithm>
#include "AQLCoreAppError.h"
#include "AQLCalibratePool.h"
#include "AQLCalibrationThread.h"

using namespace std;

AQLCalibratePool *AQLCalibratePool::mpInstance = 0;

//================ AQLCalibratePool ===================================
// constructor
/*!

*/
AQLCalibratePool::AQLCalibratePool(void)
:mTail(0), mHead(0), mCount(0)
{
}

// destructor
/*!

*/
AQLCalibratePool::~AQLCalibratePool(void)
{

	// thread terminate
	terminateThread();

	deleteRequestQueue();
}

// 
/*!
    @brief get unique instance

	@return  AQLCalibratePool *
*/
AQLCalibratePool *
AQLCalibratePool::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new AQLCalibratePool();
	}
	return mpInstance;
}

// 
/*!
    @brief set max request num
*/
void
AQLCalibratePool::setMaxRequest(unsigned int num)
{
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
AQLCalibratePool::setThreadNum(unsigned int num)
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
AQLCalibratePool::startThread()
{
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
AQLCalibratePool::putRequest(AQLCalibrate *request)
{
	if (!request)
	{
		throw AQLCoreInvalidData("Request pointer is NULL", __FILE__, __LINE__);
	}
	while (mCount >= mRequestQueue.size() || mRequestQueue[mTail])
	{
		mEvent.wait();
	}

	mRequestQueue[mTail] = request;
	mTail = (mTail + 1) % mRequestQueue.size();
	++mCount;
	mEvent.notifyAll();
	
}


// 
/*!
    @brief release calibration request

	@param[in] request
*/
void
AQLCalibratePool::releaseRequest(AQLCalibrate *request)
{
	if (!request)
	{
		throw AQLCoreInvalidData("Request pointer is NULL", __FILE__, __LINE__);
	}
	vector<AQLCalibrate *>::iterator it = 
		find(mRequestQueue.begin(), mRequestQueue.end(), request);

	if (it == mRequestQueue.end())
	{
		throw AQLCoreInvalidData("Request pointer is not registed in requestqueue", __FILE__, __LINE__);
	}
	delete (*it);
	(*it) = 0;
	mEvent.notifyAll();

}

// 
/*!
    @brief take calibration request

	@return request
*/
AQLCalibrate *
AQLCalibratePool::takeRequest(void)
{
	while (mCount <= 0)
	{
		mEvent.wait();
	}

	AQLCalibrate *request = mRequestQueue[mHead];
	mHead = (mHead + 1) % mRequestQueue.size();
	--mCount;
	mEvent.notifyAll();

	return request;
}

// Terminate Thread
/*!

*/
void
AQLCalibratePool::terminateThread()
{
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
			AQLCalibrate *terminateReq = new AQLCalibrateTerminate();
			putRequest(terminateReq); 	
		}
	}

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
AQLCalibratePool::resizeThreads(unsigned int num)
{
	mThreads.resize(num);
	for (unsigned int i = 0; i < num; ++i)
	{
		mThreads[i] = new AQLCalibrationThread(this);
	}
}

// Delete Request Queue
void
AQLCalibratePool::deleteRequestQueue()
{
	unsigned int requestSize = static_cast<unsigned int>(mRequestQueue.size());
	for (unsigned int i = 0; i < requestSize; ++i)
	{
		if (mRequestQueue[i])
		{
			delete mRequestQueue[i];
		}
	}
}
