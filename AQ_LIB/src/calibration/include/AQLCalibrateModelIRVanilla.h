/*! @file
    @brief IRVanilla sde generator class
*/
#ifndef AQLCalibrateModelIRVanilla_h
#define AQLCalibrateModelIRVanilla_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLCalibrateModelIR.h"


class AQLDataInstance;
class AQLRatesCurveLogLinearInterpolation;
class AQLMathCorrelation;
class AQLStaticData;

//===================== Class Declare AQLCalibrateModelIRVanilla==================================
/*! 
    @brief IRVanilla SDE generator 
	
	this class is abstract

*/
class AQLCalibrateModelIRVanilla : public AQLCalibrateModelIR
{
public:
	// constructor
	explicit AQLCalibrateModelIRVanilla(const AQLString &baseCurrency);
	// destructor
	virtual ~AQLCalibrateModelIRVanilla(void);
	// copy constructor
	AQLCalibrateModelIRVanilla(const AQLCalibrateModelIRVanilla &rhs);
	AQLCalibrateModelIRVanilla &operator=(const AQLCalibrateModelIRVanilla &rhs);
	// generate sde correlation
	virtual void loadCorrelationDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const {return;};
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	// generate sde volatility
	virtual void loadVolatilityDataAndCalibrate(const AQLString &key, AQLDataInstance &dataInstance) const {return;};
#endif
	
protected:
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const AQLString &key) const {SDE_TYPE a; return a;};
	//==============================================
	// check LJ
	virtual bool isLJ(const AQLString &key) const {return false;};
	//==============================================
	// set volatility 
	virtual  void setVolatility(const AQLString &key, AQLRatesSDEBase &sde)  const {return;};	
	//==============================================
	// set drift 
	virtual  void setDrift(const AQLString &key, AQLRatesSDEBase &sde)  const {return;};
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const AQLString &key, AQLRatesSDEBase &sde)  const {return;};
	//==============================================
	// set path 
	virtual  void setOutputTemplate(const AQLString &key, AQLRatesSDEBase &sde)  const  {return;};
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &key, AQLRatesSDEBase &sde)  const {return;};
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &key, AQLRatesSDEBase &sde) const {return;};
	//==============================================
	// get function master regist name 
	virtual AQLString getFunctionMasterResistName(const AQLString &key) const {AQLString ret; return ret;};
	//==============================================
	// get create sde instance 
	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &key, AQLDataInstance &dataInstance) const {return 0;};
	//==============================================
	// setup vol type
	virtual AQLString getVolType(const AQLString &key) const {AQLString ret; return ret;};
	//==============================================
	// setup vol data
	virtual void setUpVolData(const AQLString &key, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const {return;};
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const AQLString &key,AQLMathVolatility &vol, AQLDataInstance &dataInstance) const {return;};
	//==============================================
	// setup correlation input type
	virtual AQLString getCorTye(const AQLString &currency) const {AQLString ret; return ret;};
	//==============================================
	// setup correlation factor
	virtual void setUpCorFactor(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const {return;};
	//==============================================
	// setup correlation data
	virtual void setUpCorData(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const {return;};
	//==============================================
	// setup correlation method
	virtual void setUpCorFunc(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const {return;};
	//==============================================
		
};
#endif
