/*! @file
    @brief Hull-White sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef AQLCalibrateModelHW_h
#define AQLCalibrateModelHW_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLString.h"
#include "AQLCalibrateModelIR.h"



class AQLDataInstance;
class AQLRatesCurveLogLinearInterpolation;
class AQLMathCorrelation;
class AQLMathVolatility;
class AQLPriceDriftHWQuantAdjustment;


//===================== Class Declare AQLCalibrateModelHW==================================
/*! 
    @brief Hull-White SDE generator 
	

*/
class AQLCalibrateModelHW : public AQLCalibrateModelIR
{

public:
	// constructor
	explicit AQLCalibrateModelHW(const AQLString &baseCurrency);
	// destructor
	virtual ~AQLCalibrateModelHW(void);
	// copy constructor
	AQLCalibrateModelHW(const AQLCalibrateModelHW &rhs);
	AQLCalibrateModelHW &operator=(const AQLCalibrateModelHW &rhs);
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
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &currency, AQLRatesSDEBase &sde) const;
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
	// create foreign drift
	virtual AQLFunctionBase *createForeinDrift(const AQLString &fx, const AQLString &sdeBase, const AQLString &sdeName, const AQLString &fx_sdeName) const;
	//==============================================
	// get iszerovol
	virtual bool isZeroVol(const AQLString &currency) const;
	//==============================================
	// get isSZModel
	virtual bool isSZModel(const AQLString &currency) const;

private:

};
#endif
