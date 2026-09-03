/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrationParametersPtberg3F_h
#define AQLCalibrationParametersPtberg3F_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrationParametersPtberg3F.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "AQLCalibrationParametersPtberg.h"





//===================== Class Declare AQLCalibrationParametersPtberg3F==================================
/*! 
    @brief Calibration info create class
	

*/
class AQLCalibrationParametersPtberg3F : public AQLCalibrationParametersPtberg
{
public:
	// constructor
	explicit AQLCalibrationParametersPtberg3F(void);
	// destructor
	virtual ~AQLCalibrationParametersPtberg3F(void);

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx); 

};


#endif
