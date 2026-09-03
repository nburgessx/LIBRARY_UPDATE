/*! @file
    @brief  Calibration method create class
*/
//  2008, Mizuho International London.
#ifndef LACalibrationParametersLMM_h
#define LACalibrationParametersLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersLMM.h
//
//  DESCRIPTION :         Calibration info create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////


#include "LACalibrationParameters.h"
#include "LAString.h"




struct MACalibCorrelationParam
{
	double			funcParam_x;
	double			funcParam_y;
	double          maxTerm;
	LAString		corFuncType;
	unsigned int	factorNum;
	LAString		optCorType;
	double			boundaryMax;
	double			boundaryMin;
	size_t			maxIteration;
	size_t			maxStationaryStateIteration;
	double			rootEpsilon;
	double			functionEpsilon;
	double			gradientNormEpsilon;
	LAString		optCorMethodType;
	bool            isExtraTenorCalib;
};


//===================== Class Declare LACalibrationParametersLMM==================================
/*! 
    @brief Calibration method create class
	

*/
class LACalibrationParametersLMM : public LACalibrationParameters
{
public:
	// constructor
	explicit LACalibrationParametersLMM(void);
	// destructor
	virtual ~LACalibrationParametersLMM(void);

	//==============================================
	// create  calibration info object
	virtual LAString createCalibrationInfo(LAObjectPool &objPool, const LAString &ccy); 
	//==============================================
	// get parameters for calibration of correlation
	void getCalibCorParam(const LAString &ccy, MACalibCorrelationParam &param);
private:
	//==============================================
	// create  calibration  property value
	LAString getCalibStaticDataValue(const LAString &key, const LAString &grid);
};


#endif
