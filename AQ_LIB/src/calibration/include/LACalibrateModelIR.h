/*! @file
    @brief IR sde generator class
*/
//  2007, AlgoQuantHub.
#ifndef LACalibrateModelIR_h
#define LACalibrateModelIR_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelIR.h
//
//  DESCRIPTION :       IR SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "LACalibrateModel.h"


class AQLDataInstance;
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
class AQLRatesCurveLogLinearInterpolation;
class AQLMathCorrelation;
#endif 
class LAStaticData;
class AQLMathYieldCurvePro;
class AQLMathYieldCurve;
class AQLObject;
class AQLObjectPool;


//===================== Class Declare LACalibrateModelIR==================================
/*! 
    @brief IR SDE generator 
	
	this class is abstract

*/
class LACalibrateModelIR : public LACalibrateModel
{
public:
	// constructor
	explicit LACalibrateModelIR(const AQLString &baseCurrency);
	// destructor
	virtual ~LACalibrateModelIR(void);
	// copy constructor
	LACalibrateModelIR(const LACalibrateModelIR &rhs);
	LACalibrateModelIR &operator=(const LACalibrateModelIR &rhs);
	//==============================================
	// generate SDE market data
	virtual void loadModelDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, const bool isCurve = true, const bool isModel = true, const AQLString & curveID = "", const AQLString & marketName = "") const;
    //==============================================
	// generate sde initial value
	virtual void loadYieldCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false ) const;
	//==============================================
	// generate sde initial value for fwdfx constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false) const;
	//==============================================
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	// generate sde correlation
	virtual void loadCorrelationDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const;
#endif 

protected:
	//==============================================
	// generate sde initial value when arbfree curve is generated
	virtual void generateInitialValueArbfree(const AQLString &currency, AQLDataInstance &dataInstance) const;
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &currency, AQLRatesSDEBase &sde) const;
	//==============================================
	// create sde instance 
//	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &currency) const;
	//==============================================
	// setup correlation input type
	virtual AQLString getCorTye(const AQLString &currency) const = 0;
	//==============================================
	// setup correlation factor
	virtual void setUpCorFactor(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// setup correlation data
	virtual void setUpCorData(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// setup correlation method
	virtual void setUpCorFunc(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const = 0;
#endif
	//==============================================
	// get SDE data name
	virtual AQLString getSDEAttrName(const AQLString &currency) const;
	//==============================================
	// get grid propval
	AQLString getGridStaticData(const AQLString &key, const AQLString &grid) const;
	//==============================================
	// set up generate configuration
	void setUpGenerateConfig(AQLDataInstance &dataInstance, const AQLDate &asOfDate, const AQLString &currency, AQLMathYieldCurve &yc, AQLMathYieldCurvePro &ycPro,
							 AQLObject &ycData, bool &isAudExtra, bool &isSwapTenorAdjust, bool &isSpotUse, bool isArbFree) const;
	//==============================================
	// set up basis curve data
	void setUpBasisCurveData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &curveCurrency, 
							 const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, AQLMathYieldCurvePro &ycPro, const AQLString* pMktCurrency = 0, bool isCalcFwdBeforeFwdFXConsant = false) const;
	//==============================================
	// set up base curve data
	void setUpGenCurveData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
						   const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse,
						   bool isAudExtra, AQLMathYieldCurvePro &ycPro, std::map<AQLString, std::map<AQLString, double> > &aud_origSwapRate) const;
	//==============================================
	// set up ois curve data
	void setUpGenCurveDataOIS(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
							  const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, bool isAudExtra, AQLMathYieldCurvePro &ycPro, 
							  std::map<AQLString, std::map<AQLString, double> >& aud_origSwapRate) const;
	//==============================================
	// set up libor-ois basis data
	void setUpLiborOISBasisCurveData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
							         const AQLString &marketName, const AQLString&marketNameOIS, const AQLString &yieldDataName, bool isSpotUse, AQLMathYieldCurvePro &ycPro) const;
	//==============================================
	// set up 36basis curve dummy data
	void setUp36BasisDummyData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
							   const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, AQLMathYieldCurvePro &ycPro) const;
	//==============================================
	// set up floater
	void setUpFloater(const AQLString &currency, AQLMathYieldCurvePro &ycPro, const AQLString &genFloaterName) const;
	//==============================================
	// set up curve data by reading file
	void setUpCurveDataByReadFile(AQLDataInstance &dataInstance, const AQLDate &asOfDate, const AQLString &currency, const AQLString &marketName, const AQLString &yieldDataName, AQLMathYieldCurvePro &ycPro ) const;
	//==============================================
	// set up daycount
	void setUpCurveTypeDayCount(AQLMathYieldCurvePro &ycPro, AQLMathYieldCurve &yc) const;

	void dataoutCurve(const AQLStringVector &dataoutCurves, AQLObject &eData, const AQLString &yieldDataName) const;

///// update for XLL Plus //////////////////////////
	//==============================================
	// generate sde initial value when use curve that generated by excel pricer.
	void generateInitialValueForPricer(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
	// set up curve data by context for excel pricer
	void setUpCurveDataByContext(AQLMathYieldCurvePro &ycPro, AQLMathYieldCurve &yc, AQLObject *eData, const AQLString& currency, const AQLString& marketName ) const;
////////////////////////////////////////////////////

	const AQLString mBaseCurrency; // baseCurrency

	mutable std::map<AQLString, bool> mCurveGenCcyMap; // Generate Curve Currency Map
	//==============================================
	// check frequency
	virtual bool checkFrequency(const AQLString& freq, const AQLString& mktRateTerm) const;
	
private:
	//==============================================
	// convertcurve name from properties
	void convertCurveName(const AQLString &propCurve, const AQLString &ccy, AQLString &curveCcy, AQLString &curveName, const bool isPricer, AQLObjectPool& objPool) const;

};
#endif
