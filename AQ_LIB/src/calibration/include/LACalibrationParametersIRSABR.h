/*! @file
    @brief  Calibration info create class
*/
//  2008, Mizuho International London.
#ifndef LACalibrationParametersIRSABR_h
#define LACalibrationParametersIRSABR_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersIRSABR.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "LACalibrationParameters.h"





//===================== Class Declare LACalibrationParametersIRSABR==================================
/*! 
    @brief Calibration info create class
	

*/
class LACalibrationParametersIRSABR : public LACalibrationParameters
{
public:
	// constructor
	explicit LACalibrationParametersIRSABR(void);
	// destructor
	virtual ~LACalibrationParametersIRSABR(void);

	//==============================================
	// create  calibration info object
	virtual LAString createCalibrationInfo(LAObjectPool &objPool, const LAString &ccy); 
private:
	//==============================================
	// create  calibration  property value
	LAString getCalibStaticDataValue(const LAString &key, const LAString &grid);

};


#endif
