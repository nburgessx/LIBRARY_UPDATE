/*! @file
@brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrationParametersSZ_h
#define LACalibrationParametersSZ_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersSZ.h
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
class LACalibrationParametersSZ : public LACalibrationParameters
{
public:
	// constructor
	explicit LACalibrationParametersSZ(void);
	// destructor
	virtual ~LACalibrationParametersSZ(void);

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
