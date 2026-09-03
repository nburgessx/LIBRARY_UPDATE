/*! @file
    @brief Thread class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MACalibrationThread.cpp
//
//  DESCRIPTION :        Thread class
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

#include "AQLCoreError.h"
#include "LACalibrationThread.h"
#include "LACalibrate.h"
#include "LACalibratePool.h"

using namespace std;

// constructor
/*!

*/
MACalibrationThread::MACalibrationThread(LACalibratePool *channel)
:LACoreThread(), mpChannel(channel), mIsTerminateReq(false)
{
}

// destructor
/*!

*/
MACalibrationThread::~MACalibrationThread()
{
}

// 
/*!
    @brief run
*/
void
MACalibrationThread::doTerminate()
{
	mIsTerminateReq = true;
}

// 
/*!
    @brief run
*/
void
MACalibrationThread::run()
{
	while (!mIsTerminateReq)
	{
		try
		{
			LACalibrate *request = mpChannel->takeRequest();
			request->doCalibrate();
			mpChannel->releaseRequest(request);
		}
		catch (AQLCoreError &e)
		{
			e.print();
			join();
			exit(1);
		}
		catch (...)
		{
			cerr << "Unexpected error occured" << endl;
			exit(1);
		}
	}
}

