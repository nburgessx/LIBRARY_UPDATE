/*! @file
    @brief  Calibration method create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLCalibrationParametersLMM_h
#define AQLCalibrationParametersLMM_h


#include "AQLCalibrationParameters.h"
#include "AQLString.h"




struct AQLCalibCorrelationParam
{
	double			funcParam_x;
	double			funcParam_y;
	double          maxTerm;
	AQLString		corFuncType;
	unsigned int	factorNum;
	AQLString		optCorType;
	double			boundaryMax;
	double			boundaryMin;
	size_t			maxIteration;
	size_t			maxStationaryStateIteration;
	double			rootEpsilon;
	double			functionEpsilon;
	double			gradientNormEpsilon;
	AQLString		optCorMethodType;
	bool            isExtraTenorCalib;
};


//===================== Class Declare AQLCalibrationParametersLMM==================================
/*! 
    @brief Calibration method create class
	

*/
class AQLCalibrationParametersLMM : public AQLCalibrationParameters
{
public:
	// constructor
	explicit AQLCalibrationParametersLMM(void);
	// destructor
	virtual ~AQLCalibrationParametersLMM(void);

	//==============================================
	// create  calibration info object
	virtual AQLString createCalibrationInfo(AQLObjectPool &objPool, const AQLString &ccy); 
	//==============================================
	// get parameters for calibration of correlation
	void getCalibCorParam(const AQLString &ccy, AQLCalibCorrelationParam &param);
private:
	//==============================================
	// create  calibration  property value
	AQLString getCalibStaticDataValue(const AQLString &key, const AQLString &grid);
};


#endif
