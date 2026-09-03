/*! @file
    @brief MACalibrationThread class 
*/
//  2008, AlgoQuantHub.
#ifndef MACalibrationThread_h
#define MACalibrationThread_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MACalibrationThread.h
//
//  DESCRIPTION :        Thread class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreThread.h"

class LACalibratePool;


class MACalibrationThread : public LACoreThread
{
public:
	// constructor
	explicit MACalibrationThread(LACalibratePool *channel);
	// destructor
	virtual ~MACalibrationThread();
	//==============================================
	// terminate request	
	void doTerminate();
protected:
	//==============================================
	// run
	virtual void run();
	
private:
	// copy constructor
	MACalibrationThread(const MACalibrationThread &rhs);
	MACalibrationThread &operator=(const MACalibrationThread &rhs);

	LACalibratePool *mpChannel; // channel
	bool mIsTerminateReq; //mTerminate request

	
};
#endif

