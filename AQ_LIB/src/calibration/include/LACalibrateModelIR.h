/*! @file
    @brief IR sde generator class
*/
//  2007, Mizuho International London.
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

#include "LAString.h"
#include "LACalibrateModel.h"


class LADataInstance;
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
class LARatesCurveLogLinearInterpolation;
class LAMathCorrelation;
#endif 
class LAStaticData;
class LAMathYieldCurvePro;
class LAMathYieldCurve;
class LAObject;
class LAObjectPool;


//===================== Class Declare LACalibrateModelIR==================================
/*! 
    @brief IR SDE generator 
	
	this class is abstract

*/
class LACalibrateModelIR : public LACalibrateModel
{
public:
	// constructor
	explicit LACalibrateModelIR(const LAString &baseCurrency);
	// destructor
	virtual ~LACalibrateModelIR(void);
	// copy constructor
	LACalibrateModelIR(const LACalibrateModelIR &rhs);
	LACalibrateModelIR &operator=(const LACalibrateModelIR &rhs);
	//==============================================
	// generate SDE market data
	virtual void loadModelDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, const bool isCurve = true, const bool isModel = true, const LAString & curveID = "", const LAString & marketName = "") const;
    //==============================================
	// generate sde initial value
	virtual void loadYieldCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false ) const;
	//==============================================
	// generate sde initial value for fwdfx constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false) const;
	//==============================================
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	// generate sde correlation
	virtual void loadCorrelationDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const;
#endif 

protected:
	//==============================================
	// generate sde initial value when arbfree curve is generated
	virtual void generateInitialValueArbfree(const LAString &currency, LADataInstance &dataInstance) const;
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const LAString &currency, LARatesSDEBase &sde) const;
	//==============================================
	// create sde instance 
//	virtual LARatesSDEBase *createSDEInstance(const LAString &currency) const;
	//==============================================
	// setup correlation input type
	virtual LAString getCorTye(const LAString &currency) const = 0;
	//==============================================
	// setup correlation factor
	virtual void setUpCorFactor(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const = 0;
	//==============================================
	// setup correlation data
	virtual void setUpCorData(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const = 0;
	//==============================================
	// setup correlation method
	virtual void setUpCorFunc(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const = 0;
#endif
	//==============================================
	// get SDE data name
	virtual LAString getSDEAttrName(const LAString &currency) const;
	//==============================================
	// get grid propval
	LAString getGridStaticData(const LAString &key, const LAString &grid) const;
	//==============================================
	// set up generate configuration
	void setUpGenerateConfig(LADataInstance &dataInstance, const LADate &asOfDate, const LAString &currency, LAMathYieldCurve &yc, LAMathYieldCurvePro &ycPro,
							 LAObject &ycData, bool &isAudExtra, bool &isSwapTenorAdjust, bool &isSpotUse, bool isArbFree) const;
	//==============================================
	// set up basis curve data
	void setUpBasisCurveData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &curveCurrency, 
							 const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, LAMathYieldCurvePro &ycPro, const LAString* pMktCurrency = 0, bool isCalcFwdBeforeFwdFXConsant = false) const;
	//==============================================
	// set up base curve data
	void setUpGenCurveData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
						   const LAString &marketName, const LAString &yieldDataName, bool isSpotUse,
						   bool isAudExtra, LAMathYieldCurvePro &ycPro, std::map<LAString, std::map<LAString, double> > &aud_origSwapRate) const;
	//==============================================
	// set up ois curve data
	void setUpGenCurveDataOIS(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
							  const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, bool isAudExtra, LAMathYieldCurvePro &ycPro, 
							  std::map<LAString, std::map<LAString, double> >& aud_origSwapRate) const;
	//==============================================
	// set up libor-ois basis data
	void setUpLiborOISBasisCurveData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
							         const LAString &marketName, const LAString&marketNameOIS, const LAString &yieldDataName, bool isSpotUse, LAMathYieldCurvePro &ycPro) const;
	//==============================================
	// set up 36basis curve dummy data
	void setUp36BasisDummyData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
							   const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, LAMathYieldCurvePro &ycPro) const;
	//==============================================
	// set up floater
	void setUpFloater(const LAString &currency, LAMathYieldCurvePro &ycPro, const LAString &genFloaterName) const;
	//==============================================
	// set up curve data by reading file
	void setUpCurveDataByReadFile(LADataInstance &dataInstance, const LADate &asOfDate, const LAString &currency, const LAString &marketName, const LAString &yieldDataName, LAMathYieldCurvePro &ycPro ) const;
	//==============================================
	// set up daycount
	void setUpCurveTypeDayCount(LAMathYieldCurvePro &ycPro, LAMathYieldCurve &yc) const;

	void dataoutCurve(const LAStringVector &dataoutCurves, LAObject &eData, const LAString &yieldDataName) const;

///// update for XLL Plus //////////////////////////
	//==============================================
	// generate sde initial value when use curve that generated by excel pricer.
	void generateInitialValueForPricer(const LAString &currency, LADataInstance &dataInstance) const;
	//==============================================
	// set up curve data by context for excel pricer
	void setUpCurveDataByContext(LAMathYieldCurvePro &ycPro, LAMathYieldCurve &yc, LAObject *eData, const LAString& currency, const LAString& marketName ) const;
////////////////////////////////////////////////////

	const LAString mBaseCurrency; // baseCurrency

	mutable std::map<LAString, bool> mCurveGenCcyMap; // Generate Curve Currency Map
	//==============================================
	// check frequency
	virtual bool checkFrequency(const LAString& freq, const LAString& mktRateTerm) const;
	
private:
	//==============================================
	// convertcurve name from properties
	void convertCurveName(const LAString &propCurve, const LAString &ccy, LAString &curveCcy, LAString &curveName, const bool isPricer, LAObjectPool& objPool) const;

};
#endif
