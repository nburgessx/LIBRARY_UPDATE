/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrationParametersPtberg3F_h
#define LACalibrationParametersPtberg3F_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersPtberg3F.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "LACalibrationParametersPtberg.h"





//===================== Class Declare LACalibrationParametersPtberg3F==================================
/*! 
    @brief Calibration info create class
	

*/
class LACalibrationParametersPtberg3F : public LACalibrationParametersPtberg
{
public:
	// constructor
	explicit LACalibrationParametersPtberg3F(void);
	// destructor
	virtual ~LACalibrationParametersPtberg3F(void);

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx); 

};


#endif
