/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrationParametersFXStrangleSolver_h
#define LACalibrationParametersFXStrangleSolver_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersFXStrangleSolver.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "LACalibrationParameters.h"





//===================== Class Declare LACalibrationParametersFXStrangleSolver==================================
/*! 
    @brief Calibration info create class
	

*/
class LACalibrationParametersFXStrangleSolver : public LACalibrationParameters
{
public:
	// constructor
	explicit LACalibrationParametersFXStrangleSolver(void);
	// destructor
	virtual ~LACalibrationParametersFXStrangleSolver(void);

	//==============================================
	// create  calibration info object
	virtual LAString createCalibrationInfo(LAObjectPool &objPool, const LAString &fx); 
private:
	//==============================================
	// create  calibration  property value
	LAString getCalibStaticDataValue(const LAString &key, const LAString &grid);

};


#endif
