/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrationParameters_h
#define LACalibrationParameters_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParameters.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "LACoreDataService.h"
#include "LAStaticDataManager.h"


class LAStaticData;
class AQLString;
class AQLObjectPool;


//===================== Class Declare LACalibrationParameters==================================
/*! 
    @brief Calibration info create class
	
	this class is abstract

*/
class LACalibrationParameters
{
public:
	// constructor
	explicit LACalibrationParameters(void)
		:mpStaticData(&LACoreDataService::getStaticDataManager().getStaticData()),
		 mpCalibStaticData(&LACoreDataService::getStaticDataManager().getCalibStaticData()) {}
	// destructor
	virtual ~LACalibrationParameters(void) {}

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &ccy) = 0; 

protected:
	LAStaticData *mpCalibStaticData; // calib property accessor
	LAStaticData *mpStaticData; // ir property accessor
};


#endif
