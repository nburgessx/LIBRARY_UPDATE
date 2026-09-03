//
// LAUpdateCurveObject.h
// This file used to be called CalibratorIR.h and before that LACalibrateModelIR.h
//
#pragma once

#include "LAString.h"
#include "LAObjectPoolBase.h"
#include "LADefinitions.h"		// Required for CONTEXT_KEY_ASOFDATE key 
#include "LACoreDataService.h"	// Required to import the
#include <map>


// TypeDefs
typedef std::map<LAString, std::map<LAString, double> > MarketDataTenorQuotes;


// Forward Declarations
class LADataInstance;
class LARatesCurveLogLinearInterpolation;
class LAMathCorrelation;
class CurveCalibrationData;
class LAStaticData;
class LAMathYieldCurve;
class LAObject;
class LAObjectPool;
class LADataProcedure;

//===================== Class Declare LAUpdateCurveObject ==================================
/*! 
    @brief IR SDE generator 
	
	this class is abstract

*/
class LAUpdateCurveObject : public LAObjectPoolBase
{
public:
	
	// constructor
	explicit LAUpdateCurveObject(const LAString &baseCurrency);
	
	// destructor
	virtual ~LAUpdateCurveObject(void);
	
	// copy constructor
	LAUpdateCurveObject(const LAUpdateCurveObject &rhs);
	
	// assignment operator
	LAUpdateCurveObject &operator=(const LAUpdateCurveObject &rhs);
	
	// ********************************************************************************************************
	// **************** HELPER METHODS FOR 'loadModelDataAndCalibrate' ROUTINE ********************************

	// Method to Initialize and Check if Legacy Curves are to be Used, if boolean result is true must exit calibration
	bool useLegacyCurves( const LAString &currency, LADataInstance &dataInstance ) const;

	struct CurveObject
	{
		CurveObject( LADataInstance & dataInstance, const LAString & currency, const LAString & curveID, const LAString & marketName )
			: mathObj_(NULL),
			  calibrationData_(NULL),
			  instance_(NULL),
			  name_(""),
			  dataInstance_(dataInstance),
              curveCollection_(curveID),
              curveIndex_(marketName),
			  asOfDate_(LADate()),
			  currency_(currency),
			  currencyLowercase_(currency), // Make Lowercase in Body
			  isForwardFXCurve_(false),
			  targetCurveType_(""),
			  propertyManagerCurveType_(""),
			  isTargetCurveFound_(false),
			  staticDataName_(""),
			  tenorBasisStaticDataName_(""),
			  listOfCurvesBuilt_(LAStringVector()),
			  enableCalculation_(true),
			  isSpotStarting_(true),
			  useTenorBasisExtrapolation_(false),
			  useTenorBasisInstruments_(false),
			  swapCurveDiscountFactors_(LAStringVector()),
			  tenorBasisDiscountFactors_(""),
			  tenorBasisCurves_(LAStringVector()),
			  tenorBasisMarkets_(LAStringVector())
		{
			// Update the curve AsOfDate
			asOfDate_ = LADate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

			// Must keep a copy of lower case currency for property manager references
			currencyLowercase_.toLower();
		}

		LAMathYieldCurve*		mathObj_;
		CurveCalibrationData*	calibrationData_;
		LAObject*				instance_;
		LAString				name_;

		// Data Instance
		LADataInstance &		dataInstance_;

		// Curve Parameters
		LAString curveCollection_;  // also known as curve ID
        LAString curveIndex_;       // also known as marketName
        LADate asOfDate_;
		LAString currency_;
		LAString currencyLowercase_;
		bool isForwardFXCurve_;
		LAString targetCurveType_;
		LAString propertyManagerCurveType_;
		bool isTargetCurveFound_;
		LAString staticDataName_;
		LAString tenorBasisStaticDataName_;
		LAStringVector listOfCurvesBuilt_;
		bool enableCalculation_;
		bool isSpotStarting_;
		bool useTenorBasisExtrapolation_;
		bool useTenorBasisInstruments_;
		MarketDataTenorQuotes tenorBasisMarketQuotes_;
		LAStringVector swapCurveDiscountFactors_;
		LAString tenorBasisDiscountFactors_;
		LAStringVector tenorBasisCurves_;
		LAStringVector tenorBasisMarkets_;
	};

	// Method to Initialize the Curve Object and Import Property Manager Data
	void updateCurveObjectAndImportPropertyManagerData( CurveObject & curveObj ) const;

	// Method to Initialize the Curve Object, where curve parameters are stored for calibration
	void initializeCurveObject( CurveObject & curveObj ) const;

	// Method to Initialise the Curve Object by importing Property Manager Data
	void updateCurveObjectPropertyMangerData( CurveObject & curveObj ) const;

	// Method to Set-Up Curve Market Data
	void updateCurveObjectMarketData( CurveObject & curveObj ) const;

	// Method to Update the Curve Object with Risk Data
	void updateCurveObjectRiskData( CurveObject & curveObj ) const;

	// Method to Update the Curve Object with Swap Curve Instruments and Cashflows
	void updateCurveObjectSwapCurveInstrumentsAndCashflows( CurveObject & curveObj ) const;

	// Method to Set-Up Tenor Basis Curve Data
	void updateCurveObjectBasisCurveInstrumentParameters( CurveObject & curveObj ) const;

	// Method to Update the Curve Object with Xccy and FX Forward Curve Parameters
	void updateCurveObjectXccyAndFXForwardParameters( CurveObject & curveObj ) const;

	// Method to Update Curve Data with List of Curves Built
	void updateCurveObjectListOfCurvesBuilt( CurveObject & curveObj ) const;

	// Method to Update the Curve Object with Basis Curve Instruments and Cashflows
	void updateCurveObjectBasisCurveInstrumentCashflows( CurveObject & curveObj ) const;

	// Method to Update the Curve Object Intrument Priority
	void updateCurveObjectInstrumentPriority( CurveObject & curveObj ) const;

	// Update the Curve Object with the Target Curve Discount Factors
	void updateCurveObjectTargetDiscountFactors( CurveObject & curveObj ) const;

	// Calibrate the Yield Curve
	void calibrateOISAndSwapCurve( CurveObject & curveObj ) const;

	// Set Basis Curve Discount Factors to Basis Curve Object in Object Pool
	void calibrateBasisCurve( CurveObject & curveObj ) const;
	
	// ********************************************************************************************************
	// ********************************************************************************************************

	// Load Model Static Data Properties and Calibrate
	virtual void loadModelDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, const bool isModel = true, const bool isModelData = false, const LAString & curveIndex = "", const LAString & marketName = "" ) const;

	// Load curve object pool data and calibrate with dataInstance as output
	virtual void loadYieldCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, const LAString & curveID = "", const LAString & marketName = "" ) const;

	virtual void loadDualBootstrapCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const;

	virtual void loadGlobalCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const;

	// generate sde initial value for fwdfx constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, const LAString & curveID = "", const LAString & marketName = "") const;

	// generate sde initial value for cheapest-to-deliver curve
	virtual void loadCheapestToDeliverCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const;

	// generate sde correlation
	virtual void loadCorrelationDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance) const;

protected:

	// generate sde initial value when arbfree curve is generated
	virtual void generateInitialValueArbfree(const LAString &currency, LADataInstance &dataInstance) const;

	// set interpolation method 
	virtual  void setInterpolationMethod(const LAString &currency, LARatesSDEBase &sde) const;

	// setup correlation input type
	virtual LAString getCorTye(const LAString &currency) const = 0;
	
	// setup correlation factor
	virtual void setUpCorFactor(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const = 0;
	
	// setup correlation data
	virtual void setUpCorData(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const = 0;
	
	// setup correlation method
	virtual void setUpCorFunc(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const = 0;

	// get SDE data name
	virtual LAString getSDEAttrName(const LAString &currency) const;
	
	// get grid properties and static data
	LAString getGridStaticData(const LAString &key, const LAString &curve, const LAString &grid) const;
	
	// set up generate configuration
	void setUpGenerateConfig(LADataInstance &dataInstance, const LADate &asOfDate, const LAString &currency, LAMathYieldCurve &yc, CurveCalibrationData &curveCalibrationData,
							 LAObject &ycData, bool &useTenorBasis, bool &useTenorBasisInstruments, bool &isSpotUse, bool isArbFree, const LAString& suffix = "", const LAString& epSuffix = "") const;
	
	// set up basis curve data
	void setUpBasisCurveData(LADataInstance &dataInstance, LAString &staticDataName, const LADate &asOfDate, const LAString &curveCurrencyLowerCase, 
							 const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, CurveCalibrationData &curveCalibrationData, const LAString& currentCurveName = LAString(), const LAString* pMktCurrency = 0) const;
	
	// set up base curve data
	void setUpGenCurveData(LADataInstance &dataInstance, LAString &staticDataName, const LADate &asOfDate, const LAString &currency, 
						   const LAString &marketName, const LAString &yieldDataName, bool isSpotUse,
						   bool useTenorBasis, CurveCalibrationData &curveCalibrationData, std::map<LAString, std::map<LAString, double> > &aud_origSwapRate, const LAString& curveName = "") const;
	
	// set up ois curve data
	void setUpGenCurveDataOIS(LADataInstance &dataInstance, LAString &staticDataName, const LADate &asOfDate, const LAString &currency, 
							  const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, bool useTenorBasis, CurveCalibrationData &curveCalibrationData, 
							  std::map<LAString, std::map<LAString, double> >& aud_origSwapRate) const;
	
	// set up 36basis curve dummy data
	void setUp36BasisDummyData(LADataInstance &dataInstance, LAString &staticDataName, const LADate &asOfDate, const LAString &currency, 
							   const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, CurveCalibrationData &curveCalibrationData) const;
	
	// set up floater
	void setUpFloater(const LAString &currency, CurveCalibrationData &curveCalibrationData, const LAString &genFloaterName) const;
	
	// set up curve data by reading file
	void setUpCurveDataByReadFile(LADataInstance &dataInstance, const LADate &asOfDate, const LAString &currency, const LAString &marketName, const LAString &yieldDataName, CurveCalibrationData &curveCalibrationData ) const;
	
	// set up daycount
	void setUpCurveTypeDayCount(CurveCalibrationData &curveCalibrationData, LAMathYieldCurve &yc) const;

	void dataoutCurve(const LAStringVector &dataoutCurves, LAObject &eData, const LAString &yieldDataName) const;

	// generate sde initial value when use curve that generated by excel pricer.
	void generateInitialValueForPricer(const LAString &currency, LADataInstance &dataInstance) const;
	
	// set up curve data by context for excel pricer
	void setUpCurveDataByContext(CurveCalibrationData &curveCalibrationData, LAObject *eData, const LAString& currency, const LAString& marketName ) const;

	// Generate Curve Currency Map
	const LAString mBaseCurrency;

	// Generate Curve Currency Map
	mutable std::map<LAString, bool> mCurveGenCcyMap; 
	
	// check frequency
	virtual bool checkFrequency(const LAString& freq, const LAString& mktRateTerm) const;
	
	// Configure curve by preparing data used for calibration
	void configureCurve(LAStaticData *mpStaticData,
						CurveCalibrationData *curveCalibrationData,
						LAMathYieldCurve *yc,
						LADataInstance &dataInstance,
						const LADate& asOfDate,
						const LAString& currency,
						const LAString& curveName,
						const LAString& yieldDataName,
						const LAString& suffix,
						const LAString& ep_suffix,
						bool enableCalculation,
						bool isSpotUse,
						bool useTenorBasis,
						bool useTenorBasisInstruments) const;
private:

	// convertcurve name from properties
	void convertCurveName(const LAString &propCurve, const LAString &ccy, LAString &curveCcy, LAString &curveName, const bool isPricer, LAObjectPool& objPool) const;
	
	// get Accessory from Frequency
	const LAString getAccFromFreq(const LAString &freq) const;

};
