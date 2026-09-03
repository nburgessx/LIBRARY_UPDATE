/*! @file
    @brief  Calibration info create class
*/
//  2008, AlgoQuantHub.
#ifndef LACalibrationParametersFXVannaVolga_h
#define LACalibrationParametersFXVannaVolga_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersFXVannaVolga.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifndef IR_CALIBRATION_DATA_OPTIONMATURITY
#define IR_CALIBRATION_DATA_OPTIONMATURITY				"OptionMaturity"		//  data name of OptionMaturity
#endif
#ifndef PRICING_DATA_SPOTORFWD
#define PRICING_DATA_SPOTORFWD						"SpotOrFwd"				//  data name of SpotOrFwd
#endif
#ifndef PRICING_DATA_FIXINGCALENDAR
#define PRICING_DATA_FIXINGCALENDAR			"FixingCalendar"		//  data name of calendar for fixing date
#endif
#ifndef PRICING_DATA_STRIKES
#define PRICING_DATA_STRIKES						"Strikes"				//  data name of Strikes
#endif

#ifndef PRICING_DATA_FXVOLATILITYMATRIX
#define PRICING_DATA_FXVOLATILITYMATRIX						"FXVolatilityMatrix"				//  data name of FXVolatilityMatrix
#endif
#ifndef PRICING_DATA_BASESTRIKES
#define PRICING_DATA_BASESTRIKES						"BaseStrikes"				//  data name of FXVolatilityMatrix
#endif






#include "LACalibrationParameters.h"





//===================== Class Declare LACalibrationParametersFXVannaVolga==================================
/*! 
    @brief Calibration info create class
	

*/
class LACalibrationParametersFXVannaVolga : public LACalibrationParameters
{
public:
	// constructor
	explicit LACalibrationParametersFXVannaVolga(void);
	// destructor
	virtual ~LACalibrationParametersFXVannaVolga(void);

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &fx); 
private:
	//==============================================
	// create  calibration  property value
	AQLString getCalibStaticDataValue(const AQLString &key, const AQLString &grid);
};


#endif
