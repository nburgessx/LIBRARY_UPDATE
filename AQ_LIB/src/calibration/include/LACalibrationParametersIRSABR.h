/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
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
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &ccy); 
private:
	//==============================================
	// create  calibration  property value
	AQLString getCalibStaticDataValue(const AQLString &key, const AQLString &grid);

};


#endif
