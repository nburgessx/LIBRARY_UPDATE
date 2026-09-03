//
// LAUpdateCurveObject.h
// This file used to be called CalibratorIR.h and before that LACalibrateModelIR.h
//
#pragma once

#include "AQLString.h"
#include "LAObjectPoolBase.h"
#include "LADefinitions.h"		// Required for CONTEXT_KEY_ASOFDATE key 
#include "LACoreDataService.h"	// Required to import the
#include <map>


// TypeDefs
typedef std::map<AQLString, std::map<AQLString, double> > MarketDataTenorQuotes;


// Forward Declarations
class AQLDataInstance;
class LARatesCurveLogLinearInterpolation;
class LAMathCorrelation;
class CurveCalibrationData;
class LAStaticData;
class LAMathYieldCurve;
class AQLObject;
class AQLObjectPool;
class AQLDataProcedure;

//===================== Class Declare LAUpdateCurveObject ==================================
/*! 
    @brief IR SDE generator 
	
	this class is abstract

*/
class LAUpdateCurveObject : public LAObjectPoolBase
{
public:
	
	// constructor
	explicit LAUpdateCurveObject(const AQLString &baseCurrency);
	
	// destructor
	virtual ~LAUpdateCurveObject(void);
	
	// copy constructor
	LAUpdateCurveObject(const LAUpdateCurveObject &rhs);
	
	// assignment operator
	LAUpdateCurveObject &operator=(const LAUpdateCurveObject &rhs);
	
	// ********************************************************************************************************
	// **************** HELPER METHODS FOR 'loadModelDataAndCalibrate' ROUTINE ********************************

	// Method to Initialize and Check if Legacy Curves are to be Used, if boolean result is true must exit calibration
	bool useLegacyCurves( const AQLString &currency, AQLDataInstance &dataInstance ) const;

	struct CurveObject
	{
		CurveObject( AQLDataInstance & dataInstance, const AQLString & currency, const AQLString & curveID, const AQLString & marketName )
			: mathObj_(NULL),
			  calibrationData_(NULL),
			  instance_(NULL),
			  name_(""),
			  dataInstance_(dataInstance),
              curveCollection_(curveID),
              curveIndex_(marketName),
			  asOfDate_(AQLDate()),
			  currency_(currency),
			  currencyLowercase_(currency), // Make Lowercase in Body
			  isForwardFXCurve_(false),
			  targetCurveType_(""),
			  propertyManagerCurveType_(""),
			  isTargetCurveFound_(false),
			  staticDataName_(""),
			  tenorBasisStaticDataName_(""),
			  listOfCurvesBuilt_(AQLStringVector()),
			  enableCalculation_(true),
			  isSpotStarting_(true),
			  useTenorBasisExtrapolation_(false),
			  useTenorBasisInstruments_(false),
			  swapCurveDiscountFactors_(AQLStringVector()),
			  tenorBasisDiscountFactors_(""),
			  tenorBasisCurves_(AQLStringVector()),
			  tenorBasisMarkets_(AQLStringVector())
		{
			// Update the curve AsOfDate
			asOfDate_ = AQLDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

			// Must keep a copy of lower case currency for property manager references
			currencyLowercase_.toLower();
		}

		LAMathYieldCurve*		mathObj_;
		CurveCalibrationData*	calibrationData_;
		AQLObject*				instance_;
		AQLString				name_;

		// Data Instance
		AQLDataInstance &		dataInstance_;

		// Curve Parameters
		AQLString curveCollection_;  // also known as curve ID
        AQLString curveIndex_;       // also known as marketName
        AQLDate asOfDate_;
		AQLString currency_;
		AQLString currencyLowercase_;
		bool isForwardFXCurve_;
		AQLString targetCurveType_;
		AQLString propertyManagerCurveType_;
		bool isTargetCurveFound_;
		AQLString staticDataName_;
		AQLString tenorBasisStaticDataName_;
		AQLStringVector listOfCurvesBuilt_;
		bool enableCalculation_;
		bool isSpotStarting_;
		bool useTenorBasisExtrapolation_;
		bool useTenorBasisInstruments_;
		MarketDataTenorQuotes tenorBasisMarketQuotes_;
		AQLStringVector swapCurveDiscountFactors_;
		AQLString tenorBasisDiscountFactors_;
		AQLStringVector tenorBasisCurves_;
		AQLStringVector tenorBasisMarkets_;
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
	virtual void loadModelDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, const bool isModel = true, const bool isModelData = false, const AQLString & curveIndex = "", const AQLString & marketName = "" ) const;

	// Load curve object pool data and calibrate with dataInstance as output
	virtual void loadYieldCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, const AQLString & curveID = "", const AQLString & marketName = "" ) const;

	virtual void loadDualBootstrapCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const;

	virtual void loadGlobalCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const;

	// generate sde initial value for fwdfx constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, const AQLString & curveID = "", const AQLString & marketName = "") const;

	// generate sde initial value for cheapest-to-deliver curve
	virtual void loadCheapestToDeliverCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const;

	// generate sde correlation
	virtual void loadCorrelationDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const;

protected:

	// generate sde initial value when arbfree curve is generated
	virtual void generateInitialValueArbfree(const AQLString &currency, AQLDataInstance &dataInstance) const;

	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &currency, LARatesSDEBase &sde) const;

	// setup correlation input type
	virtual AQLString getCorTye(const AQLString &currency) const = 0;
	
	// setup correlation factor
	virtual void setUpCorFactor(const AQLString &currency, LAMathCorrelation &cor, AQLDataInstance &dataInstance) const = 0;
	
	// setup correlation data
	virtual void setUpCorData(const AQLString &currency, LAMathCorrelation &cor, AQLDataInstance &dataInstance) const = 0;
	
	// setup correlation method
	virtual void setUpCorFunc(const AQLString &currency, LAMathCorrelation &cor, AQLDataInstance &dataInstance) const = 0;

	// get SDE data name
	virtual AQLString getSDEAttrName(const AQLString &currency) const;
	
	// get grid properties and static data
	AQLString getGridStaticData(const AQLString &key, const AQLString &curve, const AQLString &grid) const;
	
	// set up generate configuration
	void setUpGenerateConfig(AQLDataInstance &dataInstance, const AQLDate &asOfDate, const AQLString &currency, LAMathYieldCurve &yc, CurveCalibrationData &curveCalibrationData,
							 AQLObject &ycData, bool &useTenorBasis, bool &useTenorBasisInstruments, bool &isSpotUse, bool isArbFree, const AQLString& suffix = "", const AQLString& epSuffix = "") const;
	
	// set up basis curve data
	void setUpBasisCurveData(AQLDataInstance &dataInstance, AQLString &staticDataName, const AQLDate &asOfDate, const AQLString &curveCurrencyLowerCase, 
							 const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, CurveCalibrationData &curveCalibrationData, const AQLString& currentCurveName = AQLString(), const AQLString* pMktCurrency = 0) const;
	
	// set up base curve data
	void setUpGenCurveData(AQLDataInstance &dataInstance, AQLString &staticDataName, const AQLDate &asOfDate, const AQLString &currency, 
						   const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse,
						   bool useTenorBasis, CurveCalibrationData &curveCalibrationData, std::map<AQLString, std::map<AQLString, double> > &aud_origSwapRate, const AQLString& curveName = "") const;
	
	// set up ois curve data
	void setUpGenCurveDataOIS(AQLDataInstance &dataInstance, AQLString &staticDataName, const AQLDate &asOfDate, const AQLString &currency, 
							  const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, bool useTenorBasis, CurveCalibrationData &curveCalibrationData, 
							  std::map<AQLString, std::map<AQLString, double> >& aud_origSwapRate) const;
	
	// set up 36basis curve dummy data
	void setUp36BasisDummyData(AQLDataInstance &dataInstance, AQLString &staticDataName, const AQLDate &asOfDate, const AQLString &currency, 
							   const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, CurveCalibrationData &curveCalibrationData) const;
	
	// set up floater
	void setUpFloater(const AQLString &currency, CurveCalibrationData &curveCalibrationData, const AQLString &genFloaterName) const;
	
	// set up curve data by reading file
	void setUpCurveDataByReadFile(AQLDataInstance &dataInstance, const AQLDate &asOfDate, const AQLString &currency, const AQLString &marketName, const AQLString &yieldDataName, CurveCalibrationData &curveCalibrationData ) const;
	
	// set up daycount
	void setUpCurveTypeDayCount(CurveCalibrationData &curveCalibrationData, LAMathYieldCurve &yc) const;

	void dataoutCurve(const AQLStringVector &dataoutCurves, AQLObject &eData, const AQLString &yieldDataName) const;

	// generate sde initial value when use curve that generated by excel pricer.
	void generateInitialValueForPricer(const AQLString &currency, AQLDataInstance &dataInstance) const;
	
	// set up curve data by context for excel pricer
	void setUpCurveDataByContext(CurveCalibrationData &curveCalibrationData, AQLObject *eData, const AQLString& currency, const AQLString& marketName ) const;

	// Generate Curve Currency Map
	const AQLString mBaseCurrency;

	// Generate Curve Currency Map
	mutable std::map<AQLString, bool> mCurveGenCcyMap; 
	
	// check frequency
	virtual bool checkFrequency(const AQLString& freq, const AQLString& mktRateTerm) const;
	
	// Configure curve by preparing data used for calibration
	void configureCurve(LAStaticData *mpStaticData,
						CurveCalibrationData *curveCalibrationData,
						LAMathYieldCurve *yc,
						AQLDataInstance &dataInstance,
						const AQLDate& asOfDate,
						const AQLString& currency,
						const AQLString& curveName,
						const AQLString& yieldDataName,
						const AQLString& suffix,
						const AQLString& ep_suffix,
						bool enableCalculation,
						bool isSpotUse,
						bool useTenorBasis,
						bool useTenorBasisInstruments) const;
private:

	// convertcurve name from properties
	void convertCurveName(const AQLString &propCurve, const AQLString &ccy, AQLString &curveCcy, AQLString &curveName, const bool isPricer, AQLObjectPool& objPool) const;
	
	// get Accessory from Frequency
	const AQLString getAccFromFreq(const AQLString &freq) const;

};
