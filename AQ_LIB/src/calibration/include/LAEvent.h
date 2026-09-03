/*! @file
    @brief Event class (for multi thread)
*/
//  2008, Mizuho International London.
#ifndef MAEVENT_h
#define MAEVENT_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MAEvent.h
//
//  DESCRIPTION :        Event class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#include "LACoreThread.h"
//// FROTOTYPE ////

//===================== Class Declare MAEvent==================================
/*! 
    @brief Event class
	

*/
class MAEvent
{
public:
	// constructor
	explicit MAEvent(void);
	// destructor
	virtual~MAEvent(void);
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
	MAEvent(const MAEvent &rhs);
	MAEvent &operator=(const MAEvent &rhs);

#ifdef WINDOWS
	HANDLE mEvent; // event
#else
	pthread_cond_t mReady; // ready
	pthread_mutex_t mMutex; // mMutex
#endif

};

#endif

