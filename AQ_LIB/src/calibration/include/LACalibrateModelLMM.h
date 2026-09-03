/*! @file
    @brief LMM sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef LACalibrateModelLMM_h
#define LACalibrateModelLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelLMM.h
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

#include "LACoreTemplateType.h"
#include "LAString.h"
#include "LACalibrateModelIR.h"



class LADataInstance;
class LARatesCurveLogLinearInterpolation;
class LAMathCorrelation;
class LAMathCorrelationLMMDiscAngle;
class LAMathVolatility;

//===================== Class Declare LACalibrateModelLMM==================================
/*! 
    @brief LMM SDE generator 
	

*/
class LACalibrateModelLMM : public LACalibrateModelIR
{

public:
	// constructor
	explicit LACalibrateModelLMM(const LAString &baseCurrency);
	// destructor
	virtual ~LACalibrateModelLMM(void);
	// copy constructor
	LACalibrateModelLMM(const LACalibrateModelLMM &rhs);
	LACalibrateModelLMM &operator=(const LACalibrateModelLMM &rhs);
protected:
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const LAString &currency) const;
	//==============================================
	// check LJ
	virtual bool isLJ(const LAString &currency) const;
	//==============================================
	// create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const LAString &currency, LADataInstance &dataInstance) const;
	//// generate sde initial value
	//virtual void loadYieldCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const LAString &currency, LARatesSDEBase &sde)  const;	
	//==============================================
	// set drift 
	virtual  void setDrift(const LAString &currency, LARatesSDEBase &sde)  const;
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const LAString &currency, LARatesSDEBase &sde)  const;
	//==============================================
	// set path 
	virtual  void setOutputTemplate(const LAString &currency, LARatesSDEBase &sde)  const;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const LAString &currency, LARatesSDEBase &sde)  const;
	//==============================================
	// get function master regist name 
	virtual LAString getFunctionMasterResistName(const LAString &currency) const;
	//==============================================
	// setup correlation input type
	virtual LAString getCorTye(const LAString &currency) const;

	//==============================================
	// setup correlation factor
	virtual void setUpCorFactor(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const;
	//==============================================
	// setup correlation data
	virtual void setUpCorData(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const;
	//==============================================
	// setup correlation method
	virtual void setUpCorFunc(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const;
	//==============================================
	// get vol type
	virtual LAString getVolType(const LAString &currency) const;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const LAString &currency, LAMathVolatility &vol, LADataInstance &dataInstance) const;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const LAString &currency, LAMathVolatility &vol, LADataInstance &dataInstance) const;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const LAString &currency, LARatesSDEBase &sde) const;
	//==============================================
	// create log linear interpolation
	virtual  LARatesCurveLogLinearInterpolation *createCurveLogLinearInterpolation() const;

private:
	//==============================================
	// setup
	void setUp();
	//==============================================
	// setup correlation object
	void setUpCorEntity(const LAString &currency, LAMathCorrelation &cor) const;
	//==============================================
	// setup volatility object
	void setUpVolEntity(const LAString &currency, LAMathVolatility &vol) const;
	//==============================================
	// setup tenor information for calibration
	void setUpTenorForCalib(const LAString &currency, DoubleArray &tenor_30_360, DoubleArray &tenor, DoubleArray &deltatenor, BoolVector &extraTenorFlag) const;

	DoubleArray mTenor_30_360;   // 30_360 tenor
	DoubleArray mTenor;          // tenor
	DoubleArray mDeltatenor;     // deltatenor
	BoolVector  mExtraTenorFlag; // extra tenor flag

};
#endif
