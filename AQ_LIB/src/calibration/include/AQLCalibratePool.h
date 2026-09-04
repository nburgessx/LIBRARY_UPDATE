/*! @file
    @brief AQLCalibratePool manager class 
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibratePool_h
#define AQLCalibratePool_h


#include <vector>
#include "AQLObjectPool.h"
#include "AQLEvent.h"
#include "AQLCalibrate.h"
#include "AQLScenarioConfiguration.h"

class AQLCoreDataService;
class AQLCalibrationThread;


//===================== Class Declare AQLCalibratePool==================================
/*! 
    @brief Calibration request channel class

    This class is singleton
*/
class AQLCalibratePool
{
public:
	//==============================================
	// return singleton 
	static AQLCalibratePool     *getInstance();
	//==============================================
	// start calibration thread
	 void startThread();
	//==============================================
	// put request
	 void putRequest(AQLCalibrate *request);
	//==============================================
	// release request
	 void releaseRequest(AQLCalibrate *request);
	//==============================================
	// take request
	 AQLCalibrate *takeRequest();
	//==============================================
	// set thread num
	void  setThreadNum(unsigned int num);
	//==============================================
	// set request num
	 void  setMaxRequest(unsigned int num);

private:
	class AQLCalibrateTerminate : public AQLCalibrate
	{
	public:
		// constructor
		AQLCalibrateTerminate(void) : AQLCalibrate(){};
		// destructor
		virtual ~AQLCalibrateTerminate(void){};
       	//=============================================
        //  setup
    	virtual void  setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos = -1) { (void)objPool, (void)param, (void)method, (void)gridPos;}
        //=============================================
        //  calibration method
    	virtual void    doCalibrate()
		{
			std::cout << "Terminate Calibration Thread .." << std::endl;
		}
	};

friend class AQLCoreDataService;
	// constructor
	AQLCalibratePool(void);
	// destructor
	~AQLCalibratePool(void);
	// copy constructor
	AQLCalibratePool(const AQLCalibratePool &rhs);
	AQLCalibratePool &operator=(const AQLCalibratePool &rhs);

private:
	//==============================================
	// terminate thread
	void terminateThread();
	void resizeThreads(unsigned int num);
	void deleteRequestQueue();

	static AQLCalibratePool *mpInstance; // single instance
	std::vector<AQLCalibrate *> mRequestQueue; // request queue
	unsigned int mTail; // tail
	unsigned int mHead;  // head
	unsigned int mCount;  // count

	std::vector<AQLCalibrationThread *> mThreads; // calibration thread
	mutable AQLEvent mEvent;  // event
#ifdef __HAS_MIC__
	mutable common_lib::Mutex mMutex;
#endif
};

#endif
