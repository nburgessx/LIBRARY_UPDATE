#include "AQLCoreThread.h"

class AQLCoreMutex
{
public:
	// constructor
	explicit AQLCoreMutex(void);
	// destructor
	virtual ~AQLCoreMutex(void);
	//===============
	// lock method
	void lock();
	//===============
	// lock method
	void unlock();
private:
	// copy constructor
	AQLCoreMutex(const AQLCoreMutex &rhs);
	AQLCoreMutex &operator=(const AQLCoreMutex &rhs);

#ifdef WINDOWS
	HANDLE mMutex; // mutex for multi thread
#else
	pthread_mutex_t mMutex; // mutex for multi thread
#endif
	ThreadID mOwnerID; // owner thread id
};

