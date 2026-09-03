/*! @file
    @brief  Calibration method create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrationParametersHW_h
#define AQLCalibrationParametersHW_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrationParametersHW.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "AQLCalibrationParameters.h"





//===================== Class Declare AQLCalibrationParametersHW==================================
/*! 
    @brief Calibration method create class
	

*/
class AQLCalibrationParametersHW : public AQLCalibrationParameters
{
public:
	// constructor
	explicit AQLCalibrationParametersHW(void);
	// destructor
	virtual ~AQLCalibrationParametersHW(void);

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &ccy); 
private:
	//==============================================
	// create  calibration  property value
	AQLString getCalibStaticDataValue(const AQLString &key, const AQLString &grid);
};


#endif
