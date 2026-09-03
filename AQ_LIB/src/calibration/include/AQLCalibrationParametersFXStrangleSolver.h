/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrationParametersFXStrangleSolver_h
#define AQLCalibrationParametersFXStrangleSolver_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrationParametersFXStrangleSolver.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "AQLCalibrationParameters.h"





//===================== Class Declare AQLCalibrationParametersFXStrangleSolver==================================
/*! 
    @brief Calibration info create class
	

*/
class AQLCalibrationParametersFXStrangleSolver : public AQLCalibrationParameters
{
public:
	// constructor
	explicit AQLCalibrationParametersFXStrangleSolver(void);
	// destructor
	virtual ~AQLCalibrationParametersFXStrangleSolver(void);

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx); 
private:
	//==============================================
	// create  calibration  property value
	AQLString getCalibStaticDataValue(const AQLString &key, const AQLString &grid);

};


#endif
