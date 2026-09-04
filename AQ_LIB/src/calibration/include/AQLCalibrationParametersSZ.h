/*! @file
@brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrationParametersSZ_h
#define AQLCalibrationParametersSZ_h


#include "AQLCalibrationParameters.h"





//===================== Class Declare AQLCalibrationParametersPtberg==================================
/*!
@brief Calibration info create class


*/
class AQLCalibrationParametersSZ : public AQLCalibrationParameters
{
public:
	// constructor
	explicit AQLCalibrationParametersSZ(void);
	// destructor
	virtual ~AQLCalibrationParametersSZ(void);

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx);
	virtual AQLString createCalibrationInfo2(AQLObjectPool &objPool, const AQLString &fx);
protected:
	//==============================================
	// create  calibration  property value
	AQLString getCalibStaticDataValue(const AQLString &key, const AQLString &grid);

};


#endif
