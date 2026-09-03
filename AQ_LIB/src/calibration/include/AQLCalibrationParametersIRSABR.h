/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrationParametersIRSABR_h
#define AQLCalibrationParametersIRSABR_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrationParametersIRSABR.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "AQLCalibrationParameters.h"





//===================== Class Declare AQLCalibrationParametersIRSABR==================================
/*! 
    @brief Calibration info create class
	

*/
class AQLCalibrationParametersIRSABR : public AQLCalibrationParameters
{
public:
	// constructor
	explicit AQLCalibrationParametersIRSABR(void);
	// destructor
	virtual ~AQLCalibrationParametersIRSABR(void);

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &ccy); 
private:
	//==============================================
	// create  calibration  property value
	AQLString getCalibStaticDataValue(const AQLString &key, const AQLString &grid);

};


#endif
