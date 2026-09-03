//
// BasisCurveCalibration.h
// This file was previously called YieldCurvePro.h and before that LAMathYieldCurvePro.h
//
#pragma once



#include <algorithm>
#include "LAObject.h"
#include "LACoreAutoPtr.h"
#include "LAPriceDataConvention.h"
#include "LAFunctionBase.h"
#include "LACurvePricingObject.h"
#include "LADataBasics.h"
#include "LADateHelpers.h"
#include "LAModelDynamicsCurve.h"

#include "LAMathYieldCurvePro.h"
#include "CurveCalibration.h"
#include "LADefinitions.h"

#ifndef IR_CALIBRATION_DATA_AGTCURVECOLLECTION
#define IR_CALIBRATION_DATA_AGTCURVECOLLECTION	 "AgtCurveCollection"		//!<  Against curve collection for Constant FX FWD
#endif

class LAObjectPool;

//=================== YIELD CURVE ==================================
/*! 
    @brief Class of YieldCurve
*/

class BasisCurveCalibration : public LAMathYieldCurve
{
public:
// LIFECYCLE
	//! constructor
	BasisCurveCalibration(LADataInstance* dataInstance);
	//! copy constructor
	BasisCurveCalibration(const BasisCurveCalibration& curve);
	//! destructor
	virtual ~BasisCurveCalibration();

//  QUERY
	//! get EntityType
	virtual object_t	getType(void) const;
    //! this object have specified object or not
	virtual bool		isTypeOf(object_t id) const;

	//! get AsOfDate
	const LADataDate&	getAsOfDate(void) const;
    //! get AsOfDate
	LADataDate&			getAsOfDate(void);
	//! get IsArbFree
	const LADataBool&	getIsArbFree(void) const;
    //! get AsOfDate
	LADataBool&			getIsArbFree(void);
	//!	get MarketData
	const LADataMultiReference&
						getMarketData() const;
	//!	get MarketData
	LADataMultiReference&
						getMarketData();
    //! get BasisRates
	const LADataDoubleMatrix&
						getBasisRates(void) const;
    //! get BasisRates
	LADataDoubleMatrix&		getBasisRates(void);
	//! get BasisData
	const LADataMultiReference&
						getBasisData() const;
    //! get BasisData
	LADataMultiReference&
						getBasisData();
	//! get IsFutureUse
	const LADataBool&	getIsFutureUse(void) const;
    //! get IsFutureUse
	LADataBool&			getIsFutureUse(void);
	//! get IsFRAUse
	const LADataBool&	getIsFRAUse(void) const;
    //! get IsFRAUse
	LADataBool&			getIsFRAUse(void);
	//! get BaseYieldCurve
	const LADataReference&
						getBaseYieldCurve() const;
    //! get BaseYieldCurve
	LADataReference&
						getBaseYieldCurve();
	//!	get Interpolation to generate yield 
	const LAPriceDataInterpolation&
						getInterpolation_yg() const;
	//!	get Interpolation to generate yield 
	LAPriceDataInterpolation&
						getInterpolation_yg();	
	//!	get Interpolation to use forward 
	const LAPriceDataInterpolation&
						getInterpolation_fw() const;
	//!	get Interpolation to use forward
	LAPriceDataInterpolation&	
						getInterpolation_fw();	
	//!	get Interpolation to set basis rates 
	const LAPriceDataInterpolation&
						getInterpolation_bs() const;
	//!	get Interpolation to set basis rates
	LAPriceDataInterpolation&	
						getInterpolation_bs();	
	//!	get Basis Function
	const LAPriceDataFunction&
						getBasisFunction() const;
	//!	get getBasisFunction
	LAPriceDataFunction&		getBasisFunction();
	
	LADataStrings&		getRatePriority();

	const LADataStrings&		getRatePriority() const;
	
	//! get fx object
	const LADataReference&
						getFXEntity() const;
	//! get fx object. The setting of fx object is also possible. 
	LADataReference&
						getFXEntity();	

	//!	clone this class
	LAObject*			clone() const throw (LACoreSystemError);// %%% COVARIANT RETURN %%%
	
//  OPERATION 
	//! remove specified Data. If Data is nothing, do nothing.
    virtual void        remove(const LAString& dataName);
	//! Initialize this Object.
	virtual void		reset(void);
	//!
	
	//!	set Interpolation
	void				setInterpolation(LAInterpolationBase* a, 
											const LAString& name);
	//!	set Interpolation
	void				setInterpolation(const LAString& name);
	//!	set Generator class
	void				setDFGenerator(const LACoreProcedure* a,const LAString& name);
	//!	set Generator class
	void				setDFGenerator(const LAString& name);
	//!	culc DF from marketData
	void				calcDiscountFactor(const LADate& asof);
	//!	Set DF By ShiftZero
	void				setDFByShiftZero(double width, unsigned int pos, const UintArray& grids);
	//!	Set DF By parallel ShiftZero
	void				setDFByShiftZero(double width, FloorType=NORMAL, double floor=0.00001);
	//!	Set BasisDF By ShiftZero
	void				setBasisDFByShiftZero(double width, unsigned int pos, const UintArray& grids);
	//!	Set BasisDF By parallel ShiftZero
	void				setBasisDFByShiftZero(double width, FloorType=NORMAL, double floor=0.00001);

	//!	Set Basis rate
	void				setBasisRates(void);
	//!	Set Basis rate
	void				setBasisRates2(const LAString& basisCurveID);

	void				setBasisRates(const LAString &curveType);
	//!	Set Basis rate
	void				setBasisRates_old(void);
	//!	Set DF2
	void				setDF2(void);

	//!	Set Basis rate
	void				setBasisRates(const DoubleMatrix& values);

	void				addBasisRates(void);

	//! Set ArbFree Curve Flag
	void				setArbFreeCurveName(const LAStringVector& curveNames_6ML, 
											const LAStringVector& curveNames_DF, 
											const LAStringVector& curveNames_3ML);
	//! Set dNPV/dm
	void				setdNPVdm(const LAString &curveType);
	//! get conversion matirx
	const LADataDoubleMatrix&	getConversionMatrix(const LAString &curveType) const;
	//! get conversion matirx term
	const LADataDoubles&	getConversionMatrixTerm(const LAString &curveType) const;
	//! get conversion matirx rates
	const LADataDoubles&	getConversionMarketRates(const LAString &curveType) const;
	//! get conversion matirx term types
	const LADataStrings&	getConversionMatrixTermTypes(const LAString &curveType) const;
	//! get conversion matirx term types
	const LADataStrings&	getConversionMarketTypes(const LAString &curveType) const;

	//! isOmitGridsExist
	bool					isOmitGridsExist(const LAString &curveType) const;
	//! getConversionOmitGrids
	const LADataInts&	getConversionOmitGrids(const LAString &curveType) const;
	//! setCurveDependencyMap
	void				setCurveDependencyMap(void);
	//! getCurveDependeny
	std::map<LAString, double>& getCurveDependeny(const LAString &curveType) const ;
	//! setGCurveGenerateMap
	void setGCurveGenerateMap(const LAString &curveType){mGCurveGenMap[curveType] = true;};
	//! set affecting currency
	void setAffectingCcy(const LAString &currency)
	{
		if (std::find(mAffectingCcys.begin(), mAffectingCcys.end(), currency) == mAffectingCcys.end()) 
			mAffectingCcys.push_back(currency);
	};
	//! set affected currency
	void				setAffectedCcy(const LAString &currency){mAffectedCcy = currency;};
	//! isBasisCurve
	bool				isBasisCurve(const LAString &curveType) { return (mGCurveGenMap.find(curveType) == mGCurveGenMap.end());};
	//! getBCurveGenerateMap
	const std::map<LAString, bool>& getBCurveGenerateMap(void) const {return mBCurveGenMap;};
	//! getGCurveGenerateMap
	const std::map<LAString, bool>& getGCurveGenerateMap(void) const {return mGCurveGenMap;};
	//! getGCurveGenerateMap
	std::map<LAString, bool>& getGCurveGenerateMap(void){return mGCurveGenMap;};
	//! get affecting currency
	const LAStringVector& getAffectingCcy(void) const {return mAffectingCcys;};
	//! get affected currency
	const LAString& getAffectedCcy(void) const {return mAffectedCcy;};
	//! changeZeroRiskIntoMarketRisk
	void changeZeroRiskIntoMarketRisk(LAString curveType, const DoubleVector& termZeroVals, const DoubleVector& riskZeroVals,
												LAStringVector& termMarketGrids, DoubleVector& riskMarketVals) const;
		//! getGCurveGenerateMap
	void				clearGCurveGenerateMap(void) const {mGCurveGenMap.clear();};
	//! get mAssignedCurveMktMap
	LAString getMarketForCurve(const LAString &curveName) const
	{
		if (mAssignedCurveMktMap.find(curveName) == mAssignedCurveMktMap.end())	return IR_NO_DATA;
		else return mAssignedCurveMktMap[curveName];
	};
	//! get mAssignedCurveMktMap
	const std::map<LAString, LAString>& getAssignedCurveMktMap() const {return mAssignedCurveMktMap;};
	//! get mDualBootstrapSwapCurveMktMap
	const std::map<LAString, LAString>& getDualBootstrapSwapCurveMktMap() const {return mDualBootstrapSwapCurveMktMap;};
	//! get mDualBootstrapOISCurveMktMap
	const std::map<LAString, LAString>& getDualBootstrapOISCurveMktMap() const {return mDualBootstrapOISCurveMktMap;};
	//! remove an element from the market map
	void removeAssignedCurveMktMap(const LAString& key);
	//! set mAssignedCurveMktMap
	void setAssignedCurveMktMap(const LAString &curveName, const LAString &mktName){mAssignedCurveMktMap[curveName] = mktName;};
	//! set mDualBootstrapSwapCurveMktMap
	void setDualBootstrapSwapCurveMktMap(const LAString &curveName, const LAString &mktName){mDualBootstrapSwapCurveMktMap[curveName] = mktName;};
	//! set mDualBootstrapOISCurveMktMap
	void setDualBootstrapOISCurveMktMap(const LAString &curveName, const LAString &mktName){mDualBootstrapOISCurveMktMap[curveName] = mktName;};
	//! get IsSwapTenorCahnge
	const LADataBool&	getIsSwapTenorAdjust(void) const;
    //! get IsSwapTenorCahnge
	LADataBool&			getIsSwapTenorAdjust(void);
	//! get ForeignYieldData
	const LADataReference&	
						getForeignYieldData() const;
    //! get ForeignYieldData
	LADataReference&	getForeignYieldData();
	//! calc floater PV
	void setFloater(const LAString& curveName);
	//! check whether attricutes of curves exist or not
	bool checkCurveAttr(const LAString& mktName) const;
	//! remove all curve data
	void removeAllCuveData(LAObject &yieldData) const;
	//! remove curve data
	void removeCuveData(LAObject &yieldData, const LAString& mktName) const;
	//! remove basis curve data
	void removeBasisCuveData(LAObject &yieldData) const;
	
    //! get forward convention
	void getForwardConvention(const LAString &curveName, LAPriceDataDayCount &dc, LAPriceDataSlidingRule &sld, LAPriceDataCalendar &cal, LAString &accessary) const;
    
    //! get Basis Curve frequency - TODO: This function should be deprecated!!! It's completely mad to load and search all basis instruments, just to get the curve frequency. Mad Mad Mad!!!
	void getBasisCurveFrequency(const LAString &curveName, LAString &accessary) const;
	
    //! insert non removable market
	void insertNonRemovableMarket(const LAString& mktName);
	//! erase non removable market
	void eraseNonRemovableMarket(const LAString& mktName);

	//! calc fwdfx constant curve
	void calcFwdFXConstantCurve(void);
    //! calc fwdfx constant curve
	void calcFwdFXConstantCurveUsingMarketName(const LAString &mktName = FWDFXCONST);
	//! calc fwdfx constant curve
	void calcFwdFXConstantCurve(const LAString &curveType);
	//! get colateral YieldData
	const LADataReference& getColYieldData() const;
    //! get colateral YieldData
	LADataReference& getColYieldData();
	//! get colateral affecting currency
	const LAStringVector& getColAffectingCcy(void) const {return mColAffectingCcys;};
	//! set colateral affecting currency
	void setColAffectingCcy(const LAString &currency)
	{
		if (std::find(mColAffectingCcys.begin(), mColAffectingCcys.end(), currency) == mColAffectingCcys.end()) 
			mColAffectingCcys.push_back(currency);
	};
	//! get colateral affected currency
	const LAString& getColAffectedCcy(void) const {return mColAffectedCcy;};
	//! set colateral affected currency
	void setColAffectedCcy(const LAString &currency){mColAffectedCcy = currency;};
	//! Cacculate cheapest-to-deliver curves
	void BasisCurveCalibration::calcCheapestToDeliverCurve(const LAString& curveName, const LAStringVector& csaCurves);
	//! Retrieve the BasisCurveCalibration object for the current curve set
	static BasisCurveCalibration* getYieldCurvePro( LAObjectPool& objPool, const LAString& curveCollectionID, const LAString& errMsg = "");
	//! get market data reference
	const LADataMultiReference& getMarketDataRef(const LAString& curveType) const;
	//!	calc Index grid
	void calcIndexGrid(const LADate &asofdate, const LADate &date, int resetLag, const LAPriceDataDayCount &dc, const LAPriceDataCalendar &fixcal, const LAPriceDataCalendar &paycal, const LAPriceDataSlidingRule &sld, const LAString &freq, const LAString &accessary,
		DoubleVector &gridVec, DoubleVector &termVec);
	//!	save basis curve
	void saveBasisCurve(const LAString& curveType, const DoubleArray& terms, const DoubleMatrix& termsmtx_fwd, const DoubleArray& dfs, const LAString& interpolation, std::vector<LAObject*>& data, double interpolationJoinDate = 0.0, bool isBuiltFromEngine = false);

protected:
	//! copy		 
	virtual LAObject&	copy(const LAObject& e)
								throw(LACoreInvalidData);
	//!	calc Basis rate
	void				calcBasisDiscountFactor(const LAString &type, const DoubleArray &dfs_base, DoubleArray &dfs_mod, DoubleArray &yield_mod, bool isSetAttr = true, const DoubleArray *pBasis = 0);
private:
	//!	calc basis cashflow
	void calcBasisCF(const LAPriceDataInterpolation &s_inter, const bool isDiscount, const bool isFwdRen, const bool isUSD, const bool isFWDInter, const LAPriceDataInterpolation &f_inter, const LAPriceDataInterpolation *fwd_inter, const LAPriceDataInterpolation &d_inter, 
	    const LAPriceDataInterpolation &a_f_inter, const LAPriceDataInterpolation *a_fwd_inter, const LAPriceDataInterpolation &a_d_inter, const double a_d_df_adjust, const LAPriceDataInterpolation *adjust_inter, 
	    const bool isAgtSpread, const DoubleArray &spreadVec, const int cpd_times, const int a_cpd_times, const double spotTerm, const double a_spotTerm, 
	    const std::vector<DoubleArray> &gridVec, const std::vector<DoubleArray> &tauVec, const std::vector<DoubleMatrix> &i_gridMatVec, const std::vector<DoubleMatrix> &i_termMatVec, 
	    const std::vector<DoubleArray> &a_gridVec, const std::vector<DoubleArray> &a_tauVec, const std::vector<DoubleMatrix> &a_i_gridMatVec, const std::vector<DoubleMatrix> &a_i_termMatVec, 
	    const std::vector<DoubleMatrix> &b_yieldTimeMatVec, DoubleArray &a_targetPVVec, DoubleArray &out, const bool isNegative,
		const std::vector<double>& effectiveStartGridVec_s);
	//! calc against PV (spot renotional)
	double					
	calcAgainstPV(const bool isFWDInter, const LAPriceDataInterpolation &f_inter, const LAPriceDataInterpolation *fwd_inter, 
		const LAPriceDataInterpolation &d_inter, const double spread, const int cpd_times, const double term_spot, 
		const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, const double effectiveStartingTerm);
	//! calc against PV (forward renotional)
	double					
	calcAgainstPV(const bool isFWDInter, const LAPriceDataInterpolation &f_inter, const LAPriceDataInterpolation *fwd_inter, 
		const LAPriceDataInterpolation &d_inter, const LAPriceDataInterpolation &a_s_inter, const LAPriceDataInterpolation *adjust_inter, const double spread, const int cpd_times,	const double term_spot, const double a_term_spot, 
		const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, const double effectiveStartingTerm);
	//! calc target PV for Newton-Raphson method to generate Discount Curve (spot renotional)
	double
	calcTargetPV(const LAPriceDataInterpolation &s_inter, const bool isFWDInter, const LAPriceDataInterpolation &f_inter, const LAPriceDataInterpolation *fwd_inter, const double spread, const int cpd_times, 
		const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, 
		const DoubleMatrix &b_yieldTimeMat, const double effectiveStartTerms);
	//! calc target PV for Newton-Raphson method to generate Discount Curve (forward renotional)
	double
	calcTargetPV(const LAPriceDataInterpolation &s_inter, const bool isFWDInter, const LAPriceDataInterpolation &f_inter, const LAPriceDataInterpolation *fwd_inter, const LAPriceDataInterpolation &a_d_inter, const LAPriceDataInterpolation *adjust_inter, const double spread, const int cpd_times, 
		const double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat,
		const DoubleMatrix &b_yieldTimeMat);
	//! calc target PV for Newton-Raphson method to generate Forecast Curve
	double	
	calcTargetPV(const LAPriceDataInterpolation &s_inter, const LAPriceDataInterpolation &d_inter, const double spread, const int cpd_times, 
		const double term_spot, const DoubleArray &terms_grid, const DoubleArray &terms_interval, const DoubleMatrix &i_gridMat, const DoubleMatrix &i_termMat, 
		const DoubleMatrix &b_yieldTimeMat, const double effectiveStartTerms);
	//!	set a rate convention into a curve data object
	void				setCurveConvention(LAObjectHolder& objHolder, std::vector<LAObject*>& mktData, const LAString& curveName);		
	//! set curve interpolation
	void setCurveInterpolation(const LAString& curveName, const LAObject &yieldData, LAPriceDataInterpolation& inter) const;
	

	//! Set data by name
	LADataHolder&		add(const LAString& name);
    LADataHolder&		reset(const LAString& name); // remove then add
	
	LADataHolder*       mpAsOfDate;			//!< AsOfDate							(DATA_DATE)
	LADataHolder*		mpProcedure;		//!< Procedure to genarate DF			(DATA_PROCEDURE) 
	LADataHolder*		mpMarketData;		//!< Market Data						(DATA_MULTIREFERENCE) 
	LADataHolder*       mpBasisRates;		//!< BasisRates of Terms				(DATA_DOUBLE_MATRIX)
	LADataHolder*		mpBasisData;		//!< BasisData							(DATA_MULTIREFERENCE) 
	LADataHolder*		mpIsFutureUse;		//!< Use FutureRate or not				(DATA_BOOL) 
	LADataHolder*		mpBaseYieldCurve;	//!< BaseYieldCurve						(DATA_REFERENCE) 
	LADataHolder*		mpCurrency;			//!< Currency							(DATA_STRING)
	LADataHolder*		mpInterYG;			//!< interpolation to getnerate yeild	(DATA_PROCEDURE) 
	LADataHolder*		mpInterFW;			//!< interpolation to use foward		(DATA_PROCEDURE) 
	LADataHolder*		mpInterBS;			//!< interpolation to use basis			(DATA_PROCEDURE) 
	LADataHolder*		mpBasisFunction;	//!< Basis Function						(DATA_FUNCTION)
	LADataHolder*		mpInterOC;			//!< interpolation to optional curve	(DATA_PROCEDURE) 
	LADataHolder*		mpRatePriority;		//!< interpolation to use basis			(DATA_STRINGS)
	LADataHolder*		mpIsArbFree;		//!< ArbFree Flag						(DATA_BOOL)
	LADataHolder*		mpIsFRAUse;			//!< FRA Flag							(DATA_BOOL)
	LADataHolder*		mpIsSwapTenorAdjust;//!< Swap Tenor Change Flag				(DATA_BOOL)
	LADataHolder*		mpFXEntity;			//!< FXEntity							(DATA_REFERENCE) 
	std::map<LAString, bool>  mBCurveGenMap;  //!< BasisCurveGenerateMap
	std::map<LAString, bool>  mArbFreeCurveGenMap;  //!< ArbFreeCurveGenerateMap
	mutable std::map<LAString, bool> mGCurveGenMap; //!< GenerateCurveGenMap
	std::map<LAString, std::map<LAString, double> >mDpnCurveMap; 
	mutable LAString mBfCurveType;
	mutable std::map<LAString, double> mBfDpnMap;
	LADataHolder*		mpForeignYieldData;	//!< Foreign currency YieldData			(DATA_REFERENCE)
	mutable std::map<LAString, LAString> mAssignedCurveMktMap; //!< map(curve name, market name)
	mutable std::map<LAString, LAString> mDualBootstrapSwapCurveMktMap; 
	mutable std::map<LAString, LAString> mDualBootstrapOISCurveMktMap; 
	LAStringVector mAffectingCcys;
	LAString mAffectedCcy;
	StringSet mNonRemovableMarket; //!< Non removable market

	LADataHolder*		mpColYieldData;	//!< Collateral currency YieldData			(DATA_REFERENCE)
	LAStringVector mColAffectingCcys;	//!< Collateral affecting currency
	LAString mColAffectedCcy;	//!< Collateral  affected currency
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
	bool operator()(const LAObject* _Left, const LAObject* _Right) const
	{
		LADate asof, ldate, rdate;
		LAString dataType = dynamic_cast<const LADataString&> ((_Left->getData(IR_CALIBRATION_DATA_DATATYPE, NOCHECK)).get()).get();
		dataType.toUpper(); 

		if (dataType == FRA3M || dataType == FRA6M || dataType == FRA)
		{
			const LADataHolder *dh = &_Left->getData(IR_CALIBRATION_DATA_INSTRUMENTSUBTYPE, NOCHECK);
			if (dh->isDefined() && !dh->isNull()) 
			{
				// IMM FRA
				LAString dataSubType = dynamic_cast<const LADataString&>(dh->get()).get();
				if (dataSubType == IMMFRA)
				{
					const bool isDate = dynamic_cast<const LADataBool&> ((_Left->getData(PRICING_DATA_ISDATE, ISNOTNULL)).get()).get();
					if (isDate)
					{
						ldate = dynamic_cast<const LADataDate&> ((_Left->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
						rdate = dynamic_cast<const LADataDate&> ((_Right->getData(PRICING_DATA_STARTDATE, ISNOTNULL)).get()).get();
						return ldate < rdate;
					}
					else
					{
						LAString lterm = dynamic_cast<const LADataString&> ((_Left->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
						LAString rterm = dynamic_cast<const LADataString&> ((_Right->getData(PRICING_DATA_STARTTERM, ISNOTNULL)).get()).get();
						asof.setSystemDate();
						ldate = etrading::LADateHelpers::getDate(asof, lterm, true);
						rdate = etrading::LADateHelpers::getDate(asof, rterm, true);
						return ldate < rdate;
					}
				}
			}

			LAString lterm_x = dynamic_cast<const LADataString&> ((_Left->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			LAString rterm_x = dynamic_cast<const LADataString&> ((_Right->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			LAString lterm = LAPriceYieldGenerator::changeFRATermFormat(lterm_x);
			LAString rterm = LAPriceYieldGenerator::changeFRATermFormat(rterm_x);
			asof.setSystemDate();
			ldate = etrading::LADateHelpers::getDate(asof, lterm, true);
			rdate = etrading::LADateHelpers::getDate(asof, rterm, true);
			return ldate < rdate;
		}
		else if(dataType != FUTURE && dataType != BOJ && dataType != FEDFUNDRATE && dataType != YIELD_TYPE_ARR_FUTURE)
		{
			LAString lterm = dynamic_cast<const LADataString&> ((_Left->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			LAString rterm = dynamic_cast<const LADataString&> ((_Right->getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL)).get()).get();
			if ( lterm.toUpper() == rterm.toUpper() )
			{
				// In the event of a tie in swap term, compare on the swap end date (if available),
				// or compare the swap start-terms, in the case of forward-starting swaps.
				const LADataHolder* lah = &( _Left->getData(PRICING_DATA_ISDATE) );
				const LADataHolder* rah = &( _Right->getData(PRICING_DATA_ISDATE) );
				if ( lah->isDefined() && !lah->isNull()  && rah->isDefined() && !rah->isNull() )
				{
					const bool l_is_date = dynamic_cast<const LADataBool&> ( lah->get() ).get();
					const bool r_is_date = dynamic_cast<const LADataBool&> ( rah->get() ).get();
					if (l_is_date && r_is_date )
					{
						ldate = dynamic_cast<const LADataDate&> ((_Left->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
						rdate = dynamic_cast<const LADataDate&> ((_Right->getData(PRICING_DATA_ENDDATE, ISNOTNULL)).get()).get();
						return ldate < rdate;
					}
				}

				const LADataHolder* lsah = &( _Left->getData(PRICING_DATA_STARTTERM) );
				const LADataHolder* rsah = &( _Right->getData(PRICING_DATA_STARTTERM) );
				if ( lsah->isDefined() && !lsah->isNull()  && rsah->isDefined() && !rsah->isNull() )
				{
					LAString lsterm = dynamic_cast<const LADataString&> ( lsah->get() ).get();
					LAString rsterm = dynamic_cast<const LADataString&> ( rsah->get() ).get();
					asof.setSystemDate();
					ldate = etrading::LADateHelpers::getDate(asof, lsterm, true);
					rdate = etrading::LADateHelpers::getDate(asof, rsterm, true);
					return ldate < rdate;
				}
			}

			asof.setSystemDate();
			ldate = etrading::LADateHelpers::getDate(asof, lterm, true);
			rdate = etrading::LADateHelpers::getDate(asof, rterm, true);
			return ldate < rdate;
		}

		ldate = dynamic_cast<const LADataDate&> ((_Left->getData(IR_CALIBRATION_DATA_STARTDATE, ISNOTNULL)).get()).get();
		rdate = dynamic_cast<const LADataDate&> ((_Right->getData(IR_CALIBRATION_DATA_STARTDATE, ISNOTNULL)).get()).get();
		return ldate < rdate;
	};
};
