#include "LACoreThread.h"

class LACoreMutex
{
public:
	// constructor
	explicit LACoreMutex(void);
	// destructor
	virtual ~LACoreMutex(void);
	//===============
	// lock method
	void lock();
	//===============
	// lock method
	void unlock();
private:
	// copy constructor
	LACoreMutex(const LACoreMutex &rhs);
	LACoreMutex &operator=(const LACoreMutex &rhs);

#ifdef WINDOWS
	HANDLE mMutex; // mutex for multi thread
#else
	pthread_mutex_t mMutex; // mutex for multi thread
#endif
	ThreadID mOwnerID; // owner thread id
};

