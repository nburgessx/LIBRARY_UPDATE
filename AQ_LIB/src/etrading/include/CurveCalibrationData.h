//
// CurveCalibrationData.h
// This file was previously called YieldCurvePro.h and before that LAMathYieldCurvePro.h
//
#pragma once

#include <algorithm>
#include "AQLObject.h"
#include "AQLCoreAutoPtr.h"
#include "AQLPriceDataConvention.h"
#include "AQLFunctionBase.h"
#include "LACurvePricingObject.h"
#include "AQLDataBasics.h"
#include "LADateHelpers.h"
#include "LAModelDynamicsCurve.h"

#include "LAMathYieldCurvePro.h" // *** Include this just to use the define constants
#include "CurveCalibration.h"
#include "LADefinitions.h"
#include "LACurvePricingObject.h"

#ifndef IR_CALIBRATION_DATA_AGTCURVECOLLECTION
#define IR_CALIBRATION_DATA_AGTCURVECOLLECTION	 "AgtCurveCollection"		//  Against curve collection for Constant FX FWD
#endif

class AQLObjectPool;

//=================== YIELD CURVE ==================================
/*! 
    @brief Class of YieldCurve
*/

class CurveCalibrationData : public etrading::LACurvePricingObject
{
	public:
	// LIFECYCLE
	// constructor
	CurveCalibrationData();
	// constructor
	CurveCalibrationData(AQLDataInstance* dataInstance);
	// copy constructor
	CurveCalibrationData(const CurveCalibrationData& curve);
	// destructor
	virtual ~CurveCalibrationData();

//  QUERY
	// get EntityType
	virtual object_t	getType(void) const;
    // this object have specified object or not
	virtual bool		isTypeOf(object_t id) const;

	// get AsOfDate
	const AQLDataDate&	getAsOfDate(void) const;
    // get AsOfDate
	AQLDataDate&			getAsOfDate(void);
	// get IsArbFree
	const AQLDataBool&	getIsArbFree(void) const;
    // get AsOfDate
	AQLDataBool&			getIsArbFree(void);
	//	get MarketData
	const AQLDataMultiReference&
						getMarketData() const;
	//	get MarketData
	AQLDataMultiReference&
						getMarketData();
    // get BasisRates
	const AQLDataDoubleMatrix&
						getBasisRates(void) const;
    // get BasisRates
	AQLDataDoubleMatrix&		getBasisRates(void);
	// get BasisData
	const AQLDataMultiReference&
						getBasisData() const;
    // get BasisData
	AQLDataMultiReference&
						getBasisData();
	// get IsFutureUse
	const AQLDataBool&	getIsFutureUse(void) const;
    // get IsFutureUse
	AQLDataBool&			getIsFutureUse(void);
	// get IsFRAUse
	const AQLDataBool&	getIsFRAUse(void) const;
    // get IsFRAUse
	AQLDataBool&			getIsFRAUse(void);
	// get BaseYieldCurve
	const AQLDataReference&
						getBaseYieldCurve() const;
    // get BaseYieldCurve
	AQLDataReference&
						getBaseYieldCurve();
	//	get Interpolation to generate yield 
	const AQLPriceDataInterpolation&
						getInterpolation_yg() const;
	//	get Interpolation to generate yield 
	AQLPriceDataInterpolation&
						getInterpolation_yg();	
	//	get Interpolation to use forward 
	const AQLPriceDataInterpolation&
						getInterpolation_fw() const;
	//	get Interpolation to use forward
	AQLPriceDataInterpolation&	
						getInterpolation_fw();	
	//	get Interpolation to set basis rates 
	const AQLPriceDataInterpolation&
						getInterpolation_bs() const;
	//	get Interpolation to set basis rates
	AQLPriceDataInterpolation&	
						getInterpolation_bs();	
	//	get Basis Function
	const AQLPriceDataFunction&
						getBasisFunction() const;
	//	get getBasisFunction
	AQLPriceDataFunction&		getBasisFunction();
	
	AQLDataStrings&		getRatePriority();

	const AQLDataStrings&		getRatePriority() const;
	
	// get fx object
	const AQLDataReference&
						getFXEntity() const;
	// get fx object. The setting of fx object is also possible. 
	AQLDataReference&
						getFXEntity();	

	//	clone this class
	AQLObject*			clone() const;// %%% COVARIANT RETURN %%%
	
//  OPERATION 
	// remove specified Data. If Data is nothing, do nothing.
    virtual void        remove(const AQLString& dataName);
	// Initialize this Object.
	virtual void		reset(void);
	//
	
	//	set Interpolation
	void				setInterpolation(AQLInterpolationBase* a, 
											const AQLString& name);
	//	set Interpolation
	void				setInterpolation(const AQLString& name);
	//	set Generator class
	void				setDFGenerator(const AQLCoreProcedure* a,const AQLString& name);
	//	set Generator class
	void				setDFGenerator(const AQLString& name);
	//	culc DF from marketData
	void				calcDiscountFactor(const AQLDate& asof);
	//	Set DF By ShiftZero
	void				setDFByShiftZero(double width, unsigned int pos, const UintArray& grids);
	//	Set DF By parallel ShiftZero
	void				setDFByShiftZero(double width, FloorType=NORMAL, double floor=0.00001);
	//	Set BasisDF By ShiftZero
	void				setBasisDFByShiftZero(double width, unsigned int pos, const UintArray& grids);
	//	Set BasisDF By parallel ShiftZero
	void				setBasisDFByShiftZero(double width, FloorType=NORMAL, double floor=0.00001);

	//	Set Basis rate
	void				setBasisRates(void);
	//	Set Basis rate
	void				setBasisRates2(const AQLString& basisCurveID);

	void				setBasisRates(const AQLString &curveType);
	//	Set Basis rate
	void				setBasisRates_old(void);
	//	Set DF2
	void				setDF2(void);

	//	Set Basis rate
	void				setBasisRates(const DoubleMatrix& values);

	void				addBasisRates(void);

	// Set ArbFree Curve Flag
	void				setArbFreeCurveName(const AQLStringVector& curveNames_6ML, 
											const AQLStringVector& curveNames_DF, 
											const AQLStringVector& curveNames_3ML);
	// Set dNPV/dm
	void				setdNPVdm(const AQLString &curveType);
	// get conversion matirx
	const AQLDataDoubleMatrix&	getConversionMatrix(const AQLString &curveType) const;
	// get conversion matirx term
	const AQLDataDoubles&	getConversionMatrixTerm(const AQLString &curveType) const;
	// get conversion matirx rates
	const AQLDataDoubles&	getConversionMarketRates(const AQLString &curveType) const;
	// get conversion matirx term types
	const AQLDataStrings&	getConversionMatrixTermTypes(const AQLString &curveType) const;
	// get conversion matirx term types
	const AQLDataStrings&	getConversionMarketTypes(const AQLString &curveType) const;

	// isOmitGridsExist
	bool					isOmitGridsExist(const AQLString &curveType) const;
	// getConversionOmitGrids
	const AQLDataInts&	getConversionOmitGrids(const AQLString &curveType) const;
	// setCurveDependencyMap
	void				setCurveDependencyMap(void);
	// getCurveDependeny
	std::map<AQLString, double>& getCurveDependeny(const AQLString &curveType) const ;
	// setGCurveGenerateMap
	void setGCurveGenerateMap(const AQLString &curveType){mGCurveGenMap[curveType] = true;};
	// set affecting currency
	void setAffectingCcy(const AQLString &currency)
	{
		if (std::find(mAffectingCcys.begin(), mAffectingCcys.end(), currency) == mAffectingCcys.end()) 
			mAffectingCcys.push_back(currency);
	};
	// set affected currency
	void				setAffectedCcy(const AQLString &currency){mAffectedCcy = currency;};
	// isBasisCurve
	bool				isBasisCurve(const AQLString &curveType) { return (mGCurveGenMap.find(curveType) == mGCurveGenMap.end());};
	// getBCurveGenerateMap
	const std::map<AQLString, bool>& getBCurveGenerateMap(void) const {return mBCurveGenMap;};
	// getGCurveGenerateMap
	const std::map<AQLString, bool>& getGCurveGenerateMap(void) const {return mGCurveGenMap;};
	// getGCurveGenerateMap
	std::map<AQLString, bool>& getGCurveGenerateMap(void){return mGCurveGenMap;};
	// get affecting currency
	const AQLStringVector& getAffectingCcy(void) const {return mAffectingCcys;};
	// get affected currency
	const AQLString& getAffectedCcy(void) const {return mAffectedCcy;};
	// changeZeroRiskIntoMarketRisk
	void changeZeroRiskIntoMarketRisk(AQLString curveType, const DoubleVector& termZeroVals, const DoubleVector& riskZeroVals,
												AQLStringVector& termMarketGrids, DoubleVector& riskMarketVals) const;
		// getGCurveGenerateMap
	void				clearGCurveGenerateMap(void) const {mGCurveGenMap.clear();};
	
	// Helper Method to Manage Case Inconsistency for Tenor Basis Market
	AQLString formatBasisCurveName( const AQLString& curveName ) const
	{
		// Note: Some idiot has stupidly made the static data market table for tenor basis curves case inconsistent.
		// The input market is CamelCase and yet modified to Uppercase from the get accessor. So We can't round trip
		// and verify the basis market, sigh!!!

		// To help manage this legacy issue we modify curveName searchKeys containing the keyword "BASIS" to "Basis"
		// in CamelCase, since the curve market map is expecting tenor basis searches in this format.
		std::string modifiedCurveName = curveName.c_str();

		size_t pos = modifiedCurveName.find( "BASIS" );
		if( pos != std::string::npos )
		{
			modifiedCurveName.erase( pos, modifiedCurveName.length() );
			modifiedCurveName.insert( pos, "Basis" );
		}

		return modifiedCurveName.c_str();
	}

	// get mAssignedCurveMktMap
	AQLString getMarketForCurve(const AQLString &curveName) const
	{
		// Note: Some idiot has stupidly made the static data market table for tenor basis curves case inconsistent.
		// The input market is CamelCase and yet modified to Uppercase from the get accessor. So We can't round trip
		// and verify the basis market, sigh!!!
		
		// To help manage this legacy issue we modify curveName searchKeys and the market result containing the keyword
		// "BASIS" to "Basis" in CamelCase, since the object pool and curve market map is expecting tenor basis searches
		// in this format.
		
		if( mAssignedCurveMktMap.find( curveName ) == mAssignedCurveMktMap.end() )
		{
			// Did not find the curveName in the map.
			// Try again using CamelCase version
			const AQLString searchKey = formatBasisCurveName(curveName); // Format CurveNames with "BASIS" -> "Basis"
			if (mAssignedCurveMktMap.find( searchKey ) == mAssignedCurveMktMap.end() )
			{
				return IR_NO_DATA;
			}
			return mAssignedCurveMktMap[searchKey];
		}
		
		return mAssignedCurveMktMap[curveName];
	};

	// get mAssignedCurveMktMap
	const std::map<AQLString, AQLString>& getAssignedCurveMktMap() const {return mAssignedCurveMktMap;};
	
	// get mDualBootstrapSwapCurveMktMap
	const std::map<AQLString, AQLString>& getDualBootstrapSwapCurveMktMap() const {return mDualBootstrapSwapCurveMktMap;};
	
	// get mDualBootstrapOISCurveMktMap
	const std::map<AQLString, AQLString>& getDualBootstrapOISCurveMktMap() const {return mDualBootstrapOISCurveMktMap;};
	
	// remove an element from the market map
	void removeAssignedCurveMktMap(const AQLString& key);
	
	// set mAssignedCurveMktMap
	void setAssignedCurveMktMap(const AQLString &curveName, const AQLString &mktName){mAssignedCurveMktMap[curveName] = mktName;};
	
	// set mDualBootstrapSwapCurveMktMap
	void setDualBootstrapSwapCurveMktMap(const AQLString &curveName, const AQLString &mktName){mDualBootstrapSwapCurveMktMap[curveName] = mktName;};
	
	// set mDualBootstrapOISCurveMktMap
	void setDualBootstrapOISCurveMktMap(const AQLString &curveName, const AQLString &mktName){mDualBootstrapOISCurveMktMap[curveName] = mktName;};
	
	// get IsSwapTenorCahnge
	const AQLDataBool&	getIsSwapTenorAdjust(void) const;
    
	// get IsSwapTenorCahnge
	AQLDataBool&			getIsSwapTenorAdjust(void);
	
	// get ForeignYieldData
	const AQLDataReference&	
						getForeignYieldData() const;
    // get ForeignYieldData
	AQLDataReference&	getForeignYieldData();
	
	// calc floater PV
	void setFloater(const AQLString& curveName);
	
	// check whether attricutes of curves exist or not
	bool checkCurveAttr(const AQLString& mktName) const;
	
	// remove all curve data
	void removeAllCuveData(AQLObject &yieldData) const;
	
	// remove curve data
	void removeCuveData(AQLObject &yieldData, const AQLString& mktName) const;
	
	// remove basis curve data
	void removeBasisCuveData(AQLObject &yieldData) const;
	
    // get forward convention
	void getForwardConvention(const AQLString &curveName, AQLPriceDataDayCount &dc, AQLPriceDataSlidingRule &sld, AQLPriceDataCalendar &cal, AQLString &accessary) const;
    
    // get Basis Curve frequency - TODO: This function should be deprecated!!! It's completely mad to load and search all basis instruments, just to get the curve frequency. Mad Mad Mad!!!
	void getBasisCurveFrequency(const AQLString &curveName, AQLString &accessary) const;
	
    // insert non removable market
	void insertNonRemovableMarket(const AQLString& mktName);
	
	// erase non removable market
	void eraseNonRemovableMarket(const AQLString& mktName);

	// calc fwdfx constant curve
	void calcFwdFXConstantCurve(void);
    
	// calc fwdfx constant curve
	void calcFwdFXConstantCurveUsingMarketName(const AQLString &mktName = "FWDFXCONST" );
	
	// calc fwdfx constant curve
	void calcFwdFXConstantCurve(const AQLString &curveType);
	
	// get colateral YieldData
	const AQLDataReference& getColYieldData() const;
    
	// get colateral YieldData
	AQLDataReference& getColYieldData();
	
	// get colateral affecting currency
	const AQLStringVector& getColAffectingCcy(void) const {return mColAffectingCcys;};
	
	// set colateral affecting currency
	void setColAffectingCcy(const AQLString &currency)
	{
		if (std::find(mColAffectingCcys.begin(), mColAffectingCcys.end(), currency) == mColAffectingCcys.end()) 
			mColAffectingCcys.push_back(currency);
	};
	
	// get colateral affected currency
	const AQLString& getColAffectedCcy(void) const {return mColAffectedCcy;};
	
	// set colateral affected currency
	void setColAffectedCcy(const AQLString &currency){mColAffectedCcy = currency;};
	
	// Cacculate cheapest-to-deliver curves
	void CurveCalibrationData::calcCheapestToDeliverCurve(const AQLString& curveName, const AQLStringVector& csaCurves);
	
	// Retrieve the CurveCalibrationData object for the current curve set
	static CurveCalibrationData* getYieldCurvePro( AQLObjectPool& objPool, const AQLString& curveCollectionID, const AQLString& errMsg = "");
	
	// get market data reference
	const AQLDataMultiReference& getMarketDataRef(const AQLString& curveType) const;
	
	//	calc Index grid
	void calcIndexGrid(const AQLDate &asofdate, const AQLDate &date, int resetLag, const AQLPriceDataDayCount &dc, const AQLPriceDataCalendar &fixcal, const AQLPriceDataCalendar &paycal, const AQLPriceDataSlidingRule &sld, const AQLString &freq, const AQLString &accessary,
		DoubleVector &paymentDates, DoubleVector &termVec);
	
	//	save basis curve
	void saveBasisCurve(const AQLString& curveType, const DoubleArray& terms, const DoubleMatrix& termsmtx_fwd, const DoubleArray& dfs, const AQLString& interpolation, std::vector<AQLObject*>& data, bool isHybrid=false, double interpolationJoinDate = 0.0, bool isBuiltFromEngine = false);

protected:
	
	// copy		 
	virtual AQLObject&	copy(const AQLObject& e);
	
	//	calc Basis rate
	void				calcBasisDiscountFactor(const AQLString &type, const DoubleArray &dfs_base, DoubleArray &dfs_mod, DoubleArray &yield_mod, bool isSetAttr = true, const DoubleArray *pBasis = 0);

private:
	
	//	calc basis cashflow
	void calcBasisCF( const AQLPriceDataInterpolation &spreadTimesTime,
					  const bool isTargetDiscountCurve,
					  const bool isMarkedToMarketXccySwap,
					  const bool isCurveUSD,
					  const bool useForwardInterpolation,
					  const AQLPriceDataInterpolation &forwardRatesTargetLeg,
					  const AQLPriceDataInterpolation *forwardInterpolationTargetLeg,
					  const AQLPriceDataInterpolation &discountFactorsTargetLeg, 
					  const AQLPriceDataInterpolation &forwardRatesAgainstLeg,
					  const AQLPriceDataInterpolation *forwardInterpolationAgainstLeg,
					  const AQLPriceDataInterpolation &discountFactorsAgainstLeg,
					  const double spotTermAdjustmentAgainstLeg,
					  const AQLPriceDataInterpolation *convexityAdjustment, 
					  const bool isSpreadOnAgainstLeg,
					  const DoubleArray &swapSpreads,
					  const int numberOfTimesToCompoundTargetLeg,
					  const int numberOfTimesToCompoundAgainstLeg,
					  const double spotTermTargetLeg,
					  const double spotTermAgainstLeg, 
					  const std::vector<DoubleArray> &paymentDatesTargetLeg,
					  const std::vector<DoubleArray> &accrualPeriodsTargetLeg,
					  const std::vector<DoubleMatrix> &fixingDatesTargetLeg,
					  const std::vector<DoubleMatrix> &indexTermsTargetLeg, 
					  const std::vector<DoubleArray> &paymentDatesAgainstLeg,
					  const std::vector<DoubleArray> &accrualPeriodsAgainstLeg,
					  const std::vector<DoubleMatrix> &fixingDatesAgainstLeg,
					  const std::vector<DoubleMatrix> &indexTermsAgainstLeg, 
					  const std::vector<DoubleMatrix> &basisCurveRateTimesTimeVector,
					  DoubleArray &againstLegPVs,
					  DoubleArray &basisSwapPVs,
					  const bool isNegative,
					  const std::vector<double>& spotLag );
	
	// calc against PV (spot renotional)
	double					
	calcAgainstPV( const bool useForwardInterpolation,
				   const AQLPriceDataInterpolation &forwardRates,
				   const AQLPriceDataInterpolation *forwardInterpolation, 
				   const AQLPriceDataInterpolation &discountFactor,
				   const double spread,
				   const int numberOfTimesToCompound,
				   const double spotDate, 
				   const DoubleArray &cashflowDates,
				   const DoubleArray &cashflowAccuralPeriods,
				   const DoubleMatrix &indexStartAndEndDates,
				   const DoubleMatrix &indexAccrualPeriods,
				   const double effectiveStartingTerm );
	
	// calc against PV (forward renotional)
	double					
	calcAgainstPV( const bool useForwardInterpolation,
				   const AQLPriceDataInterpolation &forwardRates,
				   const AQLPriceDataInterpolation *forwardInterpolation, 
				   const AQLPriceDataInterpolation &discountFactor,
				   const AQLPriceDataInterpolation &spreadInterpolation,
				   const AQLPriceDataInterpolation *convexityAdjustment,
				   const double spread,
				   const int numberOfTimesToCompound,
				   const double spotDate,
				   const double spotDateAgainstLeg, 
				   const DoubleArray &cashflowDates,
				   const DoubleArray &cashflowAccuralPeriods,
				   const DoubleMatrix &indexStartAndEndDates,
				   const DoubleMatrix &indexAccrualPeriods,
				   const double effectiveStartingTerm );
	
	// calc target PV for Newton-Raphson method to generate Discount Curve (spot renotional)
	double
	calcTargetPV( const AQLPriceDataInterpolation &spreadTimesTime,
				  const bool useForwardInterpolation,
				  const AQLPriceDataInterpolation &forwardRates,
				  const AQLPriceDataInterpolation *forwardInterpolation,
				  const double spread,
				  const int numberOfTimesToCompound, 
				  const DoubleArray &cashflowDates,
				  const DoubleArray &cashflowAccuralPeriods,
				  const DoubleMatrix &indexStartAndEndDates,
				  const DoubleMatrix &indexAccrualPeriods, 
				  const DoubleMatrix &basisCurveRateTimesTime,
				  const double spotLag );
	
	// calc target PV for Newton-Raphson method to generate Discount Curve (forward renotional)
	double
	calcTargetPV( const AQLPriceDataInterpolation &spreadTimesTime,
				  const bool useForwardInterpolation,
				  const AQLPriceDataInterpolation &forwardRates,
				  const AQLPriceDataInterpolation *forwardInterpolation,
				  const AQLPriceDataInterpolation &discountFactorsAgainstLeg,
				  const AQLPriceDataInterpolation *convexityAdjustment,
				  const double spread,
				  const int numberOfTimesToCompound, 
				  const double spotDate,
				  const DoubleArray &cashflowDates, const
				  DoubleArray &cashflowAccuralPeriods,
				  const DoubleMatrix &indexStartAndEndDates,
				  const DoubleMatrix &indexAccrualPeriods,
		          const DoubleMatrix &basisCurveRateTimesTime );
	
	// calc target PV for Newton-Raphson method to generate Forecast Curve
	double	
	calcTargetPV( const AQLPriceDataInterpolation &spreadTimesTime,
				  const AQLPriceDataInterpolation &discountFactor,
				  const double spread, const int numberOfTimesToCompound, 
				  const double spotDate,
				  const DoubleArray &cashflowDates,
				  const DoubleArray &cashflowAccuralPeriods,
				  const DoubleMatrix &indexStartAndEndDates,
				  const DoubleMatrix &indexAccrualPeriods, 
		          const DoubleMatrix &basisCurveRateTimesTime,
				  const double spotLag );

	//	set a rate convention into a curve data object
	void				setCurveConvention(AQLObjectHolder& objHolder, std::vector<AQLObject*>& mktData, const AQLString& curveName);		
	
	// set curve interpolation
	void setCurveInterpolation(const AQLString& curveName, const AQLObject &yieldData, AQLPriceDataInterpolation& inter) const;
	

	// Set data by name
	AQLDataHolder&		add(const AQLString& name);
    AQLDataHolder&		reset(const AQLString& name); // remove then add
	
	AQLDataHolder*       mpAsOfDate;			// AsOfDate							(DATA_DATE)
	AQLDataHolder*		mpProcedure;		// Procedure to genarate DF			(DATA_PROCEDURE) 
	AQLDataHolder*		mpMarketData;		// Market Data						(DATA_MULTIREFERENCE) 
	AQLDataHolder*       mpBasisRates;		// BasisRates of Terms				(DATA_DOUBLE_MATRIX)
	AQLDataHolder*		mpBasisData;		// BasisData							(DATA_MULTIREFERENCE) 
	AQLDataHolder*		mpIsFutureUse;		// Use FutureRate or not				(DATA_BOOL) 
	AQLDataHolder*		mpBaseYieldCurve;	// BaseYieldCurve						(DATA_REFERENCE) 
	AQLDataHolder*		mpCurrency;			// Currency							(DATA_STRING)
	AQLDataHolder*		mpInterYG;			// interpolation to getnerate yeild	(DATA_PROCEDURE) 
	AQLDataHolder*		mpInterFW;			// interpolation to use foward		(DATA_PROCEDURE) 
	AQLDataHolder*		mpInterBS;			// interpolation to use basis			(DATA_PROCEDURE) 
	AQLDataHolder*		mpBasisFunction;	// Basis Function						(DATA_FUNCTION)
	AQLDataHolder*		mpInterOC;			// interpolation to optional curve	(DATA_PROCEDURE) 
	AQLDataHolder*		mpRatePriority;		// interpolation to use basis			(DATA_STRINGS)
	AQLDataHolder*		mpIsArbFree;		// ArbFree Flag						(DATA_BOOL)
	AQLDataHolder*		mpIsFRAUse;			// FRA Flag							(DATA_BOOL)
	AQLDataHolder*		mpIsSwapTenorAdjust;// Swap Tenor Change Flag				(DATA_BOOL)
	AQLDataHolder*		mpFXEntity;			// FXEntity							(DATA_REFERENCE) 
	std::map<AQLString, bool>  mBCurveGenMap;  // BasisCurveGenerateMap
	std::map<AQLString, bool>  mArbFreeCurveGenMap;  // ArbFreeCurveGenerateMap
	mutable std::map<AQLString, bool> mGCurveGenMap; // GenerateCurveGenMap
	std::map<AQLString, std::map<AQLString, double> >mDpnCurveMap; 
	mutable AQLString mBfCurveType;
	mutable std::map<AQLString, double> mBfDpnMap;
	AQLDataHolder*		mpForeignYieldData;	// Foreign currency YieldData			(DATA_REFERENCE)
	mutable std::map<AQLString, AQLString> mAssignedCurveMktMap; // map(curve name, market name)
	mutable std::map<AQLString, AQLString> mDualBootstrapSwapCurveMktMap; 
	mutable std::map<AQLString, AQLString> mDualBootstrapOISCurveMktMap; 
	AQLStringVector mAffectingCcys;
	AQLString mAffectedCcy;
	StringSet mNonRemovableMarket; // Non removable market

	AQLDataHolder*		mpColYieldData;	// Collateral currency YieldData			(DATA_REFERENCE)
	AQLStringVector mColAffectingCcys;	// Collateral affecting currency
	AQLString mColAffectedCcy;	// Collateral  affected currency
};


class InstrumentComp
{
public:
	/*!
		@brief compare term
		@param[in] _Left one object
		@param[in] _Right another object
		@return true when right argument maturity date > left argument maturity date
	*/
	bool operator()(const AQLObject* _Left, const AQLObject* _Right) const
	{
		AQLDate asof, ldate, rdate;
		AQLString dataType = dynamic_cast<const AQLDataString&> ((_Left->getData(IR_CALIBRATION_DATA_DATATYPE, NOCHECK)).get()).get();
		dataType.toUpper(); 

		if (dataType == FRA3M || dataType == FRA6M || dataType == FRA)
		{
			const AQLDataHolder *dh = &_Left->getData(IR_CALIBRATION_DATA_INSTRUMENTSUBTYPE, NOCHECK);
			if (dh->isDefined() && !dh->isNull()) 
			{
				// IMM FRA
				AQLString dataSubType = dynamic_cast<const AQLDataString&>(dh->get()).get();
				if (dataSubType == IMMFRA)
				{
					const bool isDate = dynamic_cast<const AQLDataBool&> ((_Left->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
					if (isDate)
					{
						ldate = dynamic_cast<const AQLDataDate&> ((_Left->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
						rdate = dynamic_cast<const AQLDataDate&> ((_Right->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
						return ldate < rdate;
					}
					else
					{
						AQLString lterm = dynamic_cast<const AQLDataString&> ((_Left->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
						AQLString rterm = dynamic_cast<const AQLDataString&> ((_Right->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
						asof.setSystemDate();
						ldate = etrading::LADateHelpers::getDate(asof, lterm, true);
						rdate = etrading::LADateHelpers::getDate(asof, rterm, true);
						return ldate < rdate;
					}
				}
			}

			AQLString lterm_x = dynamic_cast<const AQLDataString&> ((_Left->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			AQLString rterm_x = dynamic_cast<const AQLDataString&> ((_Right->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			AQLString lterm = LAPriceYieldGenerator::changeFRATermFormat(lterm_x);
			AQLString rterm = LAPriceYieldGenerator::changeFRATermFormat(rterm_x);
			asof.setSystemDate();
			ldate = etrading::LADateHelpers::getDate(asof, lterm, true);
			rdate = etrading::LADateHelpers::getDate(asof, rterm, true);
			return ldate < rdate;
		}
		else if(dataType != FUTURE && dataType != BOJ && dataType != FEDFUNDRATE && dataType != YIELD_TYPE_ARR_FUTURE)
		{
			AQLString lterm = dynamic_cast<const AQLDataString&> ((_Left->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			AQLString rterm = dynamic_cast<const AQLDataString&> ((_Right->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			if ( lterm.toUpper() == rterm.toUpper() )
			{
				// In the event of a tie in swap term, compare on the swap end date (if available),
				// or compare the swap start-terms, in the case of forward-starting swaps.
				const AQLDataHolder* lah = &( _Left->getData(PRICING_DATA_ISDATE) );
				const AQLDataHolder* rah = &( _Right->getData(PRICING_DATA_ISDATE) );
				if ( lah->isDefined() && !lah->isNull()  && rah->isDefined() && !rah->isNull() )
				{
					const bool l_is_date = dynamic_cast<const AQLDataBool&> ( lah->get() ).get();
					const bool r_is_date = dynamic_cast<const AQLDataBool&> ( rah->get() ).get();
					if (l_is_date && r_is_date )
					{
						ldate = dynamic_cast<const AQLDataDate&> ((_Left->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
						rdate = dynamic_cast<const AQLDataDate&> ((_Right->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
						return ldate < rdate;
					}
				}

				const AQLDataHolder* lsah = &( _Left->getData(PRICING_DATA_STARTTERM) );
				const AQLDataHolder* rsah = &( _Right->getData(PRICING_DATA_STARTTERM) );
				if ( lsah->isDefined() && !lsah->isNull()  && rsah->isDefined() && !rsah->isNull() )
				{
					AQLString lsterm = dynamic_cast<const AQLDataString&> ( lsah->get() ).get();
					AQLString rsterm = dynamic_cast<const AQLDataString&> ( rsah->get() ).get();
					asof.setSystemDate();
					ldate = etrading::LADateHelpers::getDate(asof, lsterm, true);
					rdate = etrading::LADateHelpers::getDate(asof, rsterm, true);
					return ldate < rdate;
				}
			}

			asof.setSystemDate();

			ldate = etrading::LADateHelpers::getDate( asof, lterm, true );
			rdate = etrading::LADateHelpers::getDate(asof, rterm, true);
			return ldate < rdate;
		}

		ldate = dynamic_cast<const AQLDataDate&> ((_Left->getData(IR_CALIBRATION_DATA_STARTDATE, ISNOTNULL)).get()).get();
		rdate = dynamic_cast<const AQLDataDate&> ((_Right->getData(IR_CALIBRATION_DATA_STARTDATE, ISNOTNULL)).get()).get();
		return ldate < rdate;
	};
};
