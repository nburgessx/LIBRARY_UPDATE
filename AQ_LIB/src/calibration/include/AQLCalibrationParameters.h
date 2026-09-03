/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrationParameters_h
#define AQLCalibrationParameters_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrationParameters.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "AQLCoreDataService.h"
#include "AQLStaticDataManager.h"


class AQLStaticData;
class AQLString;
class AQLObjectPool;


//===================== Class Declare AQLCalibrationParameters==================================
/*! 
    @brief Calibration info create class
	
	this class is abstract

*/
class AQLCalibrationParameters
{
public:
	// constructor
	explicit AQLCalibrationParameters(void)
		:mpStaticData(&AQLCoreDataService::getStaticDataManager().getStaticData()),
		 mpCalibStaticData(&AQLCoreDataService::getStaticDataManager().getCalibStaticData()) {}
	// destructor
	virtual ~AQLCalibrationParameters(void) {}

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &ccy) = 0; 

protected:
	AQLStaticData *mpCalibStaticData; // calib property accessor
	AQLStaticData *mpStaticData; // ir property accessor
};


#endif
