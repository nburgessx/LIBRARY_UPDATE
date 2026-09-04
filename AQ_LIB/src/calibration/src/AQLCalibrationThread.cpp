/*! @file
    @brief Thread class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLCoreError.h"
#include "AQLCalibrationThread.h"
#include "AQLCalibrate.h"
#include "AQLCalibratePool.h"

using namespace std;

// constructor
/*!

*/
AQLCalibrationThread::AQLCalibrationThread(AQLCalibratePool *channel)
:AQLCoreThread(), mpChannel(channel), mIsTerminateReq(false)
{
}

// destructor
/*!

*/
AQLCalibrationThread::~AQLCalibrationThread()
{
}

// 
/*!
    @brief run
*/
void
AQLCalibrationThread::doTerminate()
{
	mIsTerminateReq = true;
}

// 
/*!
    @brief run
*/
void
AQLCalibrationThread::run()
{
	while (!mIsTerminateReq)
	{
		try
		{
			AQLCalibrate *request = mpChannel->takeRequest();
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

