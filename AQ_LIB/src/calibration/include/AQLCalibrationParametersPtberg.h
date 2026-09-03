/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrationParametersPtberg_h
#define AQLCalibrationParametersPtberg_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrationParametersPtberg.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "AQLCalibrationParameters.h"





//===================== Class Declare AQLCalibrationParametersPtberg==================================
/*! 
    @brief Calibration info create class
	

*/
class AQLCalibrationParametersPtberg : public AQLCalibrationParameters
{
public:
	// constructor
	explicit AQLCalibrationParametersPtberg(void);
	// destructor
	virtual ~AQLCalibrationParametersPtberg(void);

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx); 
protected:
	//==============================================
	// create  calibration  property value
	AQLString getCalibStaticDataValue(const AQLString &key, const AQLString &grid);

};


#endif
