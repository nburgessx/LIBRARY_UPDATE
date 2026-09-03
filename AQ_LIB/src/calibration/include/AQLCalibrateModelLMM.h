/*! @file
    @brief LMM sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef AQLCalibrateModelLMM_h
#define AQLCalibrateModelLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateModelLMM.h
//
//  DESCRIPTION :       LMM SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLString.h"
#include "AQLCalibrateModelIR.h"



class AQLDataInstance;
class AQLRatesCurveLogLinearInterpolation;
class AQLMathCorrelation;
class AQLMathCorrelationLMMDiscAngle;
class AQLMathVolatility;

//===================== Class Declare AQLCalibrateModelLMM==================================
/*! 
    @brief LMM SDE generator 
	

*/
class AQLCalibrateModelLMM : public AQLCalibrateModelIR
{

public:
	// constructor
	explicit AQLCalibrateModelLMM(const AQLString &baseCurrency);
	// destructor
	virtual ~AQLCalibrateModelLMM(void);
	// copy constructor
	AQLCalibrateModelLMM(const AQLCalibrateModelLMM &rhs);
	AQLCalibrateModelLMM &operator=(const AQLCalibrateModelLMM &rhs);
protected:
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const AQLString &currency) const;
	//==============================================
	// check LJ
	virtual bool isLJ(const AQLString &currency) const;
	//==============================================
	// create sde instance 
	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//// generate sde initial value
	//virtual void loadYieldCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const AQLString &currency, AQLRatesSDEBase &sde)  const;	
	//==============================================
	// set drift 
	virtual  void setDrift(const AQLString &currency, AQLRatesSDEBase &sde)  const;
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const AQLString &currency, AQLRatesSDEBase &sde)  const;
	//==============================================
	// set path 
	virtual  void setOutputTemplate(const AQLString &currency, AQLRatesSDEBase &sde)  const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &currency, AQLRatesSDEBase &sde)  const;
	//==============================================
	// get function master regist name 
	virtual AQLString getFunctionMasterResistName(const AQLString &currency) const;
	//==============================================
	// setup correlation input type
	virtual AQLString getCorTye(const AQLString &currency) const;

	//==============================================
	// setup correlation factor
	virtual void setUpCorFactor(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const;
	//==============================================
	// setup correlation data
	virtual void setUpCorData(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const;
	//==============================================
	// setup correlation method
	virtual void setUpCorFunc(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const;
	//==============================================
	// get vol type
	virtual AQLString getVolType(const AQLString &currency) const;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const AQLString &currency, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const AQLString &currency, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &currency, AQLRatesSDEBase &sde) const;
	//==============================================
	// create log linear interpolation
	virtual  AQLRatesCurveLogLinearInterpolation *createCurveLogLinearInterpolation() const;

private:
	//==============================================
	// setup
	void setUp();
	//==============================================
	// setup correlation object
	void setUpCorEntity(const AQLString &currency, AQLMathCorrelation &cor) const;
	//==============================================
	// setup volatility object
	void setUpVolEntity(const AQLString &currency, AQLMathVolatility &vol) const;
	//==============================================
	// setup tenor information for calibration
	void setUpTenorForCalib(const AQLString &currency, DoubleArray &tenor_30_360, DoubleArray &tenor, DoubleArray &deltatenor, BoolVector &extraTenorFlag) const;

	DoubleArray mTenor_30_360;   // 30_360 tenor
	DoubleArray mTenor;          // tenor
	DoubleArray mDeltatenor;     // deltatenor
	BoolVector  mExtraTenorFlag; // extra tenor flag

};
#endif
