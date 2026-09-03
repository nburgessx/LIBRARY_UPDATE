/*! @file
    @brief Hull-White sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef LACalibrateModelHW_h
#define LACalibrateModelHW_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelHW.h
//
//  DESCRIPTION :       Hull-Whete SDE generator 
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
class LAMathVolatility;
class LAPriceDriftHWQuantAdjustment;


//===================== Class Declare LACalibrateModelHW==================================
/*! 
    @brief Hull-White SDE generator 
	

*/
class LACalibrateModelHW : public LACalibrateModelIR
{

public:
	// constructor
	explicit LACalibrateModelHW(const LAString &baseCurrency);
	// destructor
	virtual ~LACalibrateModelHW(void);
	// copy constructor
	LACalibrateModelHW(const LACalibrateModelHW &rhs);
	LACalibrateModelHW &operator=(const LACalibrateModelHW &rhs);
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
	// set interpolation method 
	virtual  void setInterpolationMethod(const LAString &currency, LARatesSDEBase &sde) const;
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
	// create foreign drift
	virtual LAFunctionBase *createForeinDrift(const LAString &fx, const LAString &sdeBase, const LAString &sdeName, const LAString &fx_sdeName) const;
	//==============================================
	// get iszerovol
	virtual bool isZeroVol(const LAString &currency) const;
	//==============================================
	// get isSZModel
	virtual bool isSZModel(const LAString &currency) const;

private:

};
#endif
