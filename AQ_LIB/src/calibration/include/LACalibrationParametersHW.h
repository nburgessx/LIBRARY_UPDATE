/*! @file
    @brief  Calibration method create class
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrationParametersHW_h
#define LACalibrationParametersHW_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersHW.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "LACalibrationParameters.h"





//===================== Class Declare LACalibrationParametersHW==================================
/*! 
    @brief Calibration method create class
	

*/
class LACalibrationParametersHW : public LACalibrationParameters
{
public:
	// constructor
	explicit LACalibrationParametersHW(void);
	// destructor
	virtual ~LACalibrationParametersHW(void);

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &ccy); 
private:
	//==============================================
	// create  calibration  property value
	AQLString getCalibStaticDataValue(const AQLString &key, const AQLString &grid);
};


#endif
