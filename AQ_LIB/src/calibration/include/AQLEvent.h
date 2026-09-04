/*! @file
    @brief Event class (for multi thread)
*/
//  2008, AlgoQuantHub.
#ifndef AQLEVENT_h
#define AQLEVENT_h
#include "AQLCoreThread.h"
//// FROTOTYPE ////

//===================== Class Declare AQLEvent==================================
/*! 
    @brief Event class
	

*/
class AQLEvent
{
public:
	// constructor
	explicit AQLEvent(void);
	// destructor
	virtual~AQLEvent(void);
	//===============
	// notify
	void notify();
	//===============
	// notify
	void notifyAll();
	//===============
	// wait
	bool wait();
	//===============
	// reset
	void reset();
private:
	// copy constructor
	AQLEvent(const AQLEvent &rhs);
	AQLEvent &operator=(const AQLEvent &rhs);

#ifdef WINDOWS
	HANDLE mEvent; // event
#else
	pthread_cond_t mReady; // ready
	pthread_mutex_t mMutex; // mMutex
#endif

};

#endif

