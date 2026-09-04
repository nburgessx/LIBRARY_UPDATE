/*! @file
    @brief AQLCalibrationThread class 
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrationThread_h
#define AQLCalibrationThread_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreThread.h"

class AQLCalibratePool;


class AQLCalibrationThread : public AQLCoreThread
{
public:
	// constructor
	explicit AQLCalibrationThread(AQLCalibratePool *channel);
	// destructor
	virtual ~AQLCalibrationThread();
	//==============================================
	// terminate request	
	void doTerminate();
protected:
	//==============================================
	// run
	virtual void run();
	
private:
	// copy constructor
	AQLCalibrationThread(const AQLCalibrationThread &rhs);
	AQLCalibrationThread &operator=(const AQLCalibrationThread &rhs);

	AQLCalibratePool *mpChannel; // channel
	bool mIsTerminateReq; //mTerminate request

	
};
#endif

