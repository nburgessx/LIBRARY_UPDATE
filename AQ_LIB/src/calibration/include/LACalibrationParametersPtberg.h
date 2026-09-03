/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrationParametersPtberg_h
#define LACalibrationParametersPtberg_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersPtberg.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "LACalibrationParameters.h"





//===================== Class Declare LACalibrationParametersPtberg==================================
/*! 
    @brief Calibration info create class
	

*/
class LACalibrationParametersPtberg : public LACalibrationParameters
{
public:
	// constructor
	explicit LACalibrationParametersPtberg(void);
	// destructor
	virtual ~LACalibrationParametersPtberg(void);

	//==============================================
	// create  calibration info object
	virtual LAString createCalibrationInfo(LAObjectPool &objPool, const LAString &fx); 
protected:
	//==============================================
	// create  calibration  property value
	LAString getCalibStaticDataValue(const LAString &key, const LAString &grid);

};


#endif
