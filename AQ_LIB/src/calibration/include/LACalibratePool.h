/*! @file
    @brief LACalibratePool manager class 
*/
//  2008, AlgoQuantHub.
#ifndef LACalibratePool_h
#define LACalibratePool_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibratePool.h
//
//  DESCRIPTION :       LACalibratePool 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include <vector>
#include "LAObjectPool.h"
#include "LAEvent.h"
#include "LACalibrate.h"
#include "LAScenarioConfiguration.h"

class LACoreDataService;
class MACalibrationThread;


//===================== Class Declare LACalibratePool==================================
/*! 
    @brief Calibration request channel class

    This class is singleton
*/
class LACalibratePool
{
public:
	//==============================================
	// return singleton 
	static LACalibratePool     *getInstance();
	//==============================================
	// start calibration thread
	 void startThread();
	//==============================================
	// put request
	 void putRequest(LACalibrate *request);
	//==============================================
	// release request
	 void releaseRequest(LACalibrate *request);
	//==============================================
	// take request
	 LACalibrate *takeRequest();
	//==============================================
	// set thread num
	void  setThreadNum(unsigned int num);
	//==============================================
	// set request num
	 void  setMaxRequest(unsigned int num);

private:
	class LACalibrateTerminate : public LACalibrate
	{
	public:
		// constructor
		LACalibrateTerminate(void) : LACalibrate(){};
		// destructor
		virtual ~LACalibrateTerminate(void){};
       	//=============================================
        //  setup
    	virtual void  setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos = -1) { (void)objPool, (void)param, (void)method, (void)gridPos;}
        //=============================================
        //  calibration method
    	virtual void    doCalibrate()
		{
			std::cout << "Terminate Calibration Thread .." << std::endl;
		}
	};

friend class LACoreDataService;
	// constructor
	LACalibratePool(void);
	// destructor
	~LACalibratePool(void);
	// copy constructor
	LACalibratePool(const LACalibratePool &rhs);
	LACalibratePool &operator=(const LACalibratePool &rhs);

private:
	//==============================================
	// terminate thread
	void terminateThread();
	void resizeThreads(unsigned int num);
	void deleteRequestQueue();

	static LACalibratePool *mpInstance; // single instance
	std::vector<LACalibrate *> mRequestQueue; // request queue
	unsigned int mTail; // tail
	unsigned int mHead;  // head
	unsigned int mCount;  // count

	std::vector<MACalibrationThread *> mThreads; // calibration thread
	mutable MAEvent mEvent;  // event
#ifdef __HAS_MIC__
	mutable common_lib::Mutex mMutex;
#endif
};

#endif
