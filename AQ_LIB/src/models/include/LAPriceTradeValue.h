#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreValuation.h"
#include "AQLDataValuation.h"
#include "AQLCoreAppError.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "LAPricePayOff.h"


// LAPriceTradeValue's function id
#define FN_IR_TRADEVALUE				2012
// LAPriceTradeValue's function name
#define FN_IR_TRADEVALUE_STR			"fn_ir_tradevalue"

//#define __ITM_CHECK__ 


#ifndef PRICING_DATA_ISCALCRISK
#define PRICING_DATA_ISCALCRISK				"IsCalcRisk"		//  data name of calculate risk index or not
#endif
#ifndef PRICING_DATA_TODAY
#define PRICING_DATA_TODAY					"Today"				//  data name of today
#endif
#ifndef PRICING_DATA_SETTLEDATE
#define PRICING_DATA_SETTLEDATE				"SettleDate"		//  data name of settle date
#endif
#ifndef PRICING_DATA_ISSUEDATE
#define PRICING_DATA_ISSUEDATE				"IssueDate"		//  data name of issue date (currently only used in FundingChange)
#endif
#ifndef PRICING_DATA_FIRSTFIXINGDATE
#define PRICING_DATA_FIRSTFIXINGDATE		"FirstFixingDate"		//  data name of first fixing date (currently only used in FundingChange)
#endif
#ifndef PRICING_DATA_SPOTLAG
#define PRICING_DATA_SPOTLAG				"SpotLag"			//  data name of spot lag
#endif
#ifndef PRICING_DATA_ISDETAILOUTPUT
#define PRICING_DATA_ISDETAILOUTPUT			"IsDetailOutput"	//  data name of detail output or not
#endif
#ifndef PRICING_DATA_CALLINFO
#define PRICING_DATA_CALLINFO				"CallInfo"			// Data Name of Call Information 
#endif
#ifndef PRICING_DATA_TRIGGERINFOS
#define PRICING_DATA_TRIGGERINFOS			"TriggerInfos"		// Data Name of Trigger Information 
#endif
#ifndef PRICING_DATA_PATHENTITY
#define PRICING_DATA_PATHENTITY				"PathEntity"		//  data name of path object
#endif
#ifndef PRICING_DATA_CFGENERATOR
#define PRICING_DATA_CFGENERATOR			"CashFlowGenerator"	//  data name of cash flow generator
#endif
#ifndef PRICING_DATA_MCNUM
#define PRICING_DATA_MCNUM					"MCNum"				//  data name of mc number
#endif
#ifndef PRICING_DATA_CLEANPRICE
#define PRICING_DATA_CLEANPRICE				"CleanPrice"		//  data name of clean price
#endif
#ifndef PRICING_DATA_DIRTYPRICE
#define PRICING_DATA_DIRTYPRICE				"DirtyPrice"		//  data name of dirty price
#endif
#ifndef PRICING_DATA_ACCRUEDINTEREST
#define PRICING_DATA_ACCRUEDINTEREST		"AccruedInterest"	//  data name of accrued interest
#endif
#ifndef PRICING_DATA_CLEANPRICESQUARE
#define PRICING_DATA_CLEANPRICESQUARE		"CleanPriceSquare"	//  data name of clean price square
#endif
#ifndef PRICING_DATA_CLEANPRICEDEVIATION
#define PRICING_DATA_CLEANPRICEDEVIATION	"CleanPriceDeviation"	//  data name of clean price deviation
#endif
#ifndef PRICING_DATA_CALLTRIGGERVALUE
#define PRICING_DATA_CALLTRIGGERVALUE		"CallTriggerValue"	//  data name of call trigger value
#endif
#ifndef PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER
#define PRICING_DATA_CLEANPRICEWITHOUTCALLTRIGGER	"CleanPricewithoutCallTrigger"	//  data name of clean price without call trigger
#endif
#ifndef PRICING_DATA_FEE_EXCLUDED_PV
#define PRICING_DATA_FEE_EXCLUDED_PV	"FeeExcludedPV"
#endif
#ifndef PRICING_DATA_CASH
#define PRICING_DATA_CASH					"Cash"	//  data name of cash
#endif
#ifndef PRICING_DATA_ACTIONPROBABILITIES
#define PRICING_DATA_ACTIONPROBABILITIES	"ActionProbabilities"	//  data name of action probabilities
#endif
#ifndef PRICING_DATA_AVERAGELIFE
#define PRICING_DATA_AVERAGELIFE	"AverageLife"	//  data name of average life
#endif
#ifndef PRICING_DATA_CURRENCY
#define PRICING_DATA_CURRENCY				"Currency"				//  data name of notional currency
#endif
#ifndef PRICING_DATA_SELECTSIDE
#define PRICING_DATA_SELECTSIDE				"SelectSide"			//  data name of SelectSide
#endif
#ifndef PRICING_DATA_SELECTCALLTYPE
#define PRICING_DATA_SELECTCALLTYPE			"SelectCallType"		//  data name of select call tpye(startable or cancelable)
#endif
#ifndef PRICING_DATA_LSMCNUM
#define PRICING_DATA_LSMCNUM				"LSMCNum"				//  data name of lsmc number
#endif
#ifndef PRICING_DATA_POLYNOMIAL
#define PRICING_DATA_POLYNOMIAL				"Polynomial"			//  data name of ploynomial function 
#endif
#ifndef PRICING_DATA_ISOPTIONHOLDER
#define PRICING_DATA_ISOPTIONHOLDER			"IsOptionHolder"			//  data name of option holder
#endif
#ifndef PRICING_DATA_ISPATHDETAILOUTPUT
#define PRICING_DATA_ISPATHDETAILOUTPUT		"IsPathDetailOutput"	//  data name of path detail output or not
#endif
#ifndef PRICING_DATA_PATHDETAIL
#define PRICING_DATA_PATHDETAIL			"PathDetail"	//  data name of path detail
#endif
#ifndef PRICING_DATA_ISLSMCDETAILOUTPUT
#define PRICING_DATA_ISLSMCDETAILOUTPUT		"IsLSMCDetailOutput"	//  data name of lsmc detail output or not
#endif
#ifndef PRICING_DATA_LSMCREGCOEFFICIENTS
#define PRICING_DATA_LSMCREGCOEFFICIENTS	"LSMCRegCoefficients"	//  data name of lsmc regression coefficients
#endif
#ifndef PRICING_DATA_LSMCREGCOEFFICIENTS2
#define PRICING_DATA_LSMCREGCOEFFICIENTS2	"LSMCRegCoefficients2"	//  data name of lsmc regression coefficients
#endif
#ifndef PRICING_DATA_LSMCEXPLANATORYVARIABLES
#define PRICING_DATA_LSMCEXPLANATORYVARIABLES	"LSMCExplanatoryVariables"	//  data name of lsmc explanatory variables
#endif
#ifndef PRICING_DATA_LSMCEXPLAINEDVARIABLES
#define PRICING_DATA_LSMCEXPLAINEDVARIABLES	"LSMCExplainedVariables"	//  data name of lsmc explained variables
#endif
#ifndef PRICING_DATA_LSMCCALLPOSTJUDGE
#define PRICING_DATA_LSMCCALLPOSTJUDGE	"LSMCCallPostJudge"	//  data name of lsmc post judge of call
#endif
#ifndef PRICING_DATA_LSMCCALLJUDGE
#define PRICING_DATA_LSMCCALLJUDGE	"LSMCCallJudge"	//  data name of lsmc judge of call
#endif
#ifndef PRICING_DATA_COEFFICIENTS2
#define PRICING_DATA_COEFFICIENTS2			"Coefficients2"			//  data name of coefficient
#endif
#ifndef PRICING_DATA_CALLREBATE
#define PRICING_DATA_CALLREBATE			"CallRebate"	//  data name of call rebate
#endif
#ifndef PRICING_DATA_ISRECALCTRADEDATA
#define PRICING_DATA_ISRECALCTRADEDATA			"IsRecalcTradeData"			//  data name of tradedatarecalc flag
#endif
#ifndef PRICING_DATA_ISFUNDINGCHANGE
#define PRICING_DATA_ISFUNDINGCHANGE			"IsFundingChange"			//  data name of funding change flag
#endif
#ifndef PRICING_DATA_ISEXCLUDEOUTLIER_Y
#define PRICING_DATA_ISEXCLUDEOUTLIER_Y         "IsExcludeOutlier_Y"        // data name of IsExcludeOutlier_Y flag
#endif
#ifndef PRICING_DATA_ISEXCLUDEOUTLIER_X
#define PRICING_DATA_ISEXCLUDEOUTLIER_X         "IsExcludeOutlier_X"        // data name of IsExcludeOutlier_X flag
#endif
#ifndef PRICING_DATA_ISEXCLUDEOUTLIER_E
#define PRICING_DATA_ISEXCLUDEOUTLIER_E         "IsExcludeOutlier_e"        // data name of IsExcludeOutlier_e flag
#endif
#ifndef PRICING_DATA_CRITERIA_Y
#define PRICING_DATA_CRITERIA_Y         "Criteria_Y"        // data name of Criteria_Y
#endif
#ifndef PRICING_DATA_CRITERIA_X
#define PRICING_DATA_CRITERIA_X         "Criteria_X"       // data name of Criteria_X
#endif
#ifndef PRICING_DATA_CRITERIA_E
#define PRICING_DATA_CRITERIA_E         "Criteria_e"        // data name of Criteria_e
#endif
#ifndef PRICING_DATA_SVDTOLERANCE
#define PRICING_DATA_SVDTOLERANCE       "SVDTolerance"      // data name of SVD Tolerance
#endif
#ifndef PRICING_DATA_ISSHIFT_Y
#define PRICING_DATA_ISSHIFT_Y         "IsShift_Y"        // data name of IsShift_Y flag
#endif
#ifndef PRICING_DATA_ISSCALE_Y
#define PRICING_DATA_ISSCALE_Y         "IsScale_Y"        // data name of IsScale_Y flag
#endif
#ifndef PRICING_DATA_ISSHIFT_X
#define PRICING_DATA_ISSHIFT_X         "IsShift_X"        // data name of IsShift_X flag
#endif
#ifndef PRICING_DATA_ISSCALE_X
#define PRICING_DATA_ISSCALE_X         "IsScale_X"        // data name of IsScale_X flag
#endif
#ifndef PRICING_DATA_ISCONVERT_XY
#define PRICING_DATA_ISCONVERT_XY      "IsConvert_XY"        // data name of IsConvert_XY flag
#endif
#ifndef PRICING_DATA_SHIFT_Y
#define PRICING_DATA_SHIFT_Y            "Shift_Y"        // data name of Shift_Y value
#endif
#ifndef PRICING_DATA_SCALE_Y
#define PRICING_DATA_SCALE_Y            "Scale_Y"        // data name of Scale_Y value
#endif
#ifndef PRICING_DATA_SHIFT_X
#define PRICING_DATA_SHIFT_X            "Shift_X"        // data name of Shift_Y value
#endif
#ifndef PRICING_DATA_SCALE_X
#define PRICING_DATA_SCALE_X            "Scale_X"        // data name of Scale_Y value
#endif
#ifndef PRICING_DATA_ISCONVERT_XY2
#define PRICING_DATA_ISCONVERT_XY2      "IsConvert_XY2"   // data name of IsConvert_XY2 flag
#endif
#ifndef PRICING_DATA_SHIFT_Y2
#define PRICING_DATA_SHIFT_Y2           "Shift_Y2"        // data name of Shift_Y value
#endif
#ifndef PRICING_DATA_SCALE_Y2
#define PRICING_DATA_SCALE_Y2           "Scale_Y2"        // data name of Scale_Y value
#endif
#ifndef PRICING_DATA_SHIFT_X2
#define PRICING_DATA_SHIFT_X2           "Shift_X2"        // data name of Shift_Y value
#endif
#ifndef PRICING_DATA_SCALE_X2
#define PRICING_DATA_SCALE_X2           "Scale_X2"        // data name of Scale_Y value
#endif
#ifndef PRICING_DATA_VALUEDATE
#define PRICING_DATA_VALUEDATE			"ValueDate"	// data name of value basedate
#endif
#ifndef PRICING_DATA_ISSPOTACCRUEDINTEREST
#define PRICING_DATA_ISSPOTACCRUEDINTEREST			"IsSpotAccruedInterest"	// data name of value basedate
#endif
#ifndef PRICING_DATA_DISCOUNTCURVE
#define PRICING_DATA_DISCOUNTCURVE         "DiscountCurve"        // data name of DiscountCurve
#endif
#ifndef PRICING_DATA_ISLEGBASECURRENCY
#define PRICING_DATA_ISLEGBASECURRENCY         "IsLegBaseCurrency"        // data name of IsLegBaseCurrency
#endif
#ifndef PRICING_DATA_FUNDINGCHANGEINFO
#define PRICING_DATA_FUNDINGCHANGEINFO				"FundingChangeInfo"		//  data name of FundingChangeInfo
#endif
#ifndef PRICING_DATA_ISSETUPPAYOFF
#define PRICING_DATA_ISSETUPPAYOFF		    "IsSetUpPayOff"		//  data name of issetuppayoff
#endif
//#ifndef PRICING_DATA_ISOUTPUT
//#define PRICING_DATA_ISOUTPUT				"isOutput"
//#endif
#ifndef PRICING_DATA_ISRESULTOUTPUT
#define PRICING_DATA_ISRESULTOUTPUT				"isResultOutput"
#endif
#ifndef PRICING_DATA_CFCALCSTARTDATE_LEG
#define PRICING_DATA_CFCALCSTARTDATE_LEG	"CFCalcStartDate_Leg"
#endif
#ifndef PRICING_DATA_CFCALCENDDATE_LEG
#define PRICING_DATA_CFCALCENDDATE_LEG	"CFCalcEndDate_Leg"
#endif
#ifndef PRICING_DATA_PAYMENTDATE_LEG
#define PRICING_DATA_PAYMENTDATE_LEG	"PaymentDate_Leg"
#endif
#ifndef PRICING_DATA_CASHLETVALUETIME_LEG
#define PRICING_DATA_CASHLETVALUETIME_LEG	"CashletValueTime_Leg"
#endif
#ifndef PRICING_DATA_CASHLETVALUE_LEG
#define PRICING_DATA_CASHLETVALUE_LEG	"CashletValue_Leg"
#endif
#ifndef PRICING_DATA_PVVALUE_LEG
#define PRICING_DATA_PVVALUE_LEG	"PVValue_Leg"
#endif
#ifndef PRICING_DATA_DF_LEG
#define PRICING_DATA_DF_LEG	"DF_Leg"
#endif
#ifndef PRICING_DATA_FRADF_LEG
#define PRICING_DATA_FRADF_LEG	"FRADF_Leg"
#endif
#ifndef PRICING_DATA_FIXINGDATE_LEG
#define PRICING_DATA_FIXINGDATE_LEG	"FixingDate_Leg"
#endif
#ifndef PRICING_DATA_FIXINGFLAG_LEG
#define PRICING_DATA_FIXINGFLAG_LEG	"FixingFlag_Leg"
#endif
#ifndef PRICING_DATA_FIXINGTIME_LEG
#define PRICING_DATA_FIXINGTIME_LEG	"FixingTime_Leg"
#endif
#ifndef PRICING_DATA_FIXEDRATE_LEG
#define PRICING_DATA_FIXEDRATE_LEG	"FixedRate_Leg"
#endif
#ifndef PRICING_DATA_GEARING_LEG
#define PRICING_DATA_GEARING_LEG	"Gearing_Leg"
#endif
#ifndef PRICING_DATA_FORWARD_LEG
#define PRICING_DATA_FORWARD_LEG	"Forward_Leg"
#endif
#ifndef PRICING_DATA_SPREAD_LEG
#define PRICING_DATA_SPREAD_LEG	"Spread_Leg"
#endif
#ifndef PRICING_DATA_STRIKE_LEG
#define PRICING_DATA_STRIKE_LEG	"Strike_Leg"
#endif
#ifndef PRICING_DATA_ACCRUALDAYS_LEG
#define PRICING_DATA_ACCRUALDAYS_LEG	"AccrualDays_Leg"
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONAL_LEG
#define PRICING_CALIBRATION_DATAOTIONAL_LEG	"Notional_Leg"
#endif
#ifndef PRICING_DATA_VOLATILITY_LEG
#define PRICING_DATA_VOLATILITY_LEG	"Volatility_Leg"
#endif
#ifndef PRICING_DATA_EXPIRYTERM_LEG
#define PRICING_DATA_EXPIRYTERM_LEG	"ExpiryTerm_Leg"
#endif
#ifndef PRICING_DATA_EXTRACFVALUETIME_LEG
#define PRICING_DATA_EXTRACFVALUETIME_LEG	"ExtraCFValueTime_Leg"
#endif
#ifndef PRICING_DATA_EXTRACFVALUE_LEG
#define PRICING_DATA_EXTRACFVALUE_LEG	"ExtraCFValue_Leg"
#endif
#ifndef PRICING_DATA_EXTRACFPVVALUE_LEG
#define PRICING_DATA_EXTRACFPVVALUE_LEG	"ExtraCFPVValue_Leg"
#endif
#ifndef PRICING_DATA_TRIGGERVALUETIME_LEG
#define PRICING_DATA_TRIGGERVALUETIME_LEG	"TriggerValueTime_Leg"
#endif
#ifndef PRICING_DATA_TRIGGERVALUE_LEG
#define PRICING_DATA_TRIGGERVALUE_LEG	"TriggerValue_Leg"
#endif
#ifndef PRICING_DATA_TRIGGERPVVALUE_LEG
#define PRICING_DATA_TRIGGERPVVALUE_LEG	"TriggerPVValue_Leg"
#endif
#ifndef PRICING_DATA_CALLVALUETIME_LEG
#define PRICING_DATA_CALLVALUETIME_LEG	"CallValueTime_Leg"
#endif
#ifndef PRICING_DATA_CALLVALUE_LEG
#define PRICING_DATA_CALLVALUE_LEG	"CallValue_Leg"
#endif
#ifndef PRICING_DATA_PV_LEG1
#define PRICING_DATA_PV_LEG1	"PV_Leg1"
#endif
#ifndef PRICING_DATA_PV_LEG2
#define PRICING_DATA_PV_LEG2	"PV_Leg2"
#endif
#ifndef PRICING_DATA_PV_LEG1_PREFIXING_ADJUSTMENT
#define PRICING_DATA_PV_LEG1_PREFIXING_ADJUSTMENT	"PV_Leg1_Prefixing_adjustment"
#endif
#ifndef PRICING_DATA_PV_LEG2_PREFIXING_ADJUSTMENT
#define PRICING_DATA_PV_LEG2_PREFIXING_ADJUSTMENT	"PV_Leg2_Prefixing_adjustment"
#endif
#ifndef PRICING_DATA_CURRENCY_LEG1
#define PRICING_DATA_CURRENCY_LEG1	"Currency_Leg1"
#endif
#ifndef PRICING_DATA_CURRENCY_LEG2
#define PRICING_DATA_CURRENCY_LEG2	"Currency_Leg2"
#endif
#ifndef PRICING_DATA_PVCURRENCY
#define PRICING_DATA_PVCURRENCY		"PVCurrency"
#endif
#ifndef PRICING_DATA_ISANNUITYCALC
#define PRICING_DATA_ISANNUITYCALC		"IsAnnuityCalc"
#endif
#ifndef PRICING_DATA_ANNUITYVALUE
#define PRICING_DATA_ANNUITYVALUE		"AnnuityValue"
#endif
#ifndef PRICING_DATA_ODDINDEXTYPE
#define PRICING_DATA_ODDINDEXTYPE		"OddIndexType"
#endif
#ifndef PRICING_DATA_TOLERANCEFORSPOTRATERATIOMETHOD
#define PRICING_DATA_TOLERANCEFORSPOTRATERATIOMETHOD		"ToleranceForSpotRateRatioMethod"
#endif
#ifndef PRICING_DATA_ISODDTRADE
#define PRICING_DATA_ISODDTRADE		"IsOddTrade"
#endif
#ifndef PRICING_DATA_TODAYFX_LEG1CCY
#define PRICING_DATA_TODAYFX_LEG1CCY	"TodayFX_Leg1CCY"
#endif
#ifndef PRICING_DATA_TODAYFX_LEG2CCY
#define PRICING_DATA_TODAYFX_LEG2CCY	"TodayFX_Leg2CCY"
#endif
#ifndef PRICING_DATA_DIRTYPRICE_LEG1CCY
#define PRICING_DATA_DIRTYPRICE_LEG1CCY		"DirtyPrice_Leg1CCY"
#endif
#ifndef PRICING_DATA_DIRTYPRICE_LEG2CCY
#define PRICING_DATA_DIRTYPRICE_LEG2CCY		"DirtyPrice_Leg2CCY"
#endif
#ifndef PRICING_DATA_ZEROCALC
#define PRICING_DATA_ZEROCALC            "ZeroCalc"
#endif
#ifndef PRICING_DATA_CONVEXITYADJUST_LEG
#define PRICING_DATA_CONVEXITYADJUST_LEG	"ConvexityAdjust_Leg"
#endif
#ifndef PRICING_DATA_CONVEXITYADJUSTVOL_LEG
#define PRICING_DATA_CONVEXITYADJUSTVOL_LEG	"ConvexityAdjustVol_Leg"
#endif
#ifndef PRICING_DATA_FIRSTNUMERAIREFORIRR
#define PRICING_DATA_FIRSTNUMERAIREFORIRR	"FirstNumeraireForIRR"
#endif
#ifndef PRICING_DATA_ANNUITYTERMS
#define PRICING_DATA_ANNUITYTERMS			"AnnuityTerms"
#endif
#ifndef PRICING_CALIBRATION_DATAOTIONALCF_LEG
#define PRICING_CALIBRATION_DATAOTIONALCF_LEG			"NotionalCF_Leg"
#endif
#ifndef PRICING_DATA_RENOTIONALFIXINGDATE_LEG
#define PRICING_DATA_RENOTIONALFIXINGDATE_LEG			"RenotionalFixingDate_Leg"
#endif
#ifndef PRICING_DATA_COMPOUNDING_STARTDATE
#define PRICING_DATA_COMPOUNDING_STARTDATE "CompoundingStartDate"
#endif
#ifndef PRICING_DATA_COMPOUNDING_ENDDATE
#define PRICING_DATA_COMPOUNDING_ENDDATE "CompoundingEndDate"
#endif
#ifndef PRICING_DATA_COMPOUNDING_FIXDATE
#define PRICING_DATA_COMPOUNDING_FIXDATE "CompoundingFixDate"
#endif
#ifndef PRICING_DATA_COMPOUNDING_FIXFLAG
#define PRICING_DATA_COMPOUNDING_FIXFLAG "CompoundingFixFlag"
#endif
#ifndef PRICING_DATA_COMPOUNDEDTERM
#define PRICING_DATA_COMPOUNDEDTERM "CompoundedTerm"
#endif
#ifndef PRICING_DATA_COMPOUNDEDRATE 
#define PRICING_DATA_COMPOUNDEDRATE "CompoundedRate"
#endif
#ifndef PRICING_DATA_COMPOUNDED_RATE_OUT
#define PRICING_DATA_COMPOUNDED_RATE_OUT "CompoundedRateOut"
#endif
#ifndef PRICING_DATA_ISCALCVANILLA
#define PRICING_DATA_ISCALCVANILLA "IsCalcVanilla"
#endif
#ifndef PRICING_DATA_YIELDPRONAME
#define PRICING_DATA_YIELDPRONAME "YieldProName"
#endif
// for EQ-Flows
#ifndef PRICING_DATA_ACCRUALTERM_LEG
#define PRICING_DATA_ACCRUALTERM_LEG	"AccrualTerm_Leg"
#endif
#ifndef PRICING_DATA_BOUNDARY1_LEG
#define PRICING_DATA_BOUNDARY1_LEG	"Boundary1_Leg"
#endif
#ifndef PRICING_DATA_BOUNDARY2_LEG
#define PRICING_DATA_BOUNDARY2_LEG	"Boundary2_Leg"
#endif
#ifndef PRICING_DATA_BOUNDARY3_LEG
#define PRICING_DATA_BOUNDARY3_LEG	"Boundary3_Leg"
#endif
#ifndef PRICING_DATA_BOUNDARY4_LEG
#define PRICING_DATA_BOUNDARY4_LEG	"Boundary4_Leg"
#endif
#ifndef PRICING_DATA_COUPONRATE0_LEG
#define PRICING_DATA_COUPONRATE0_LEG	"CouponRate0_Leg"
#endif
#ifndef PRICING_DATA_COUPONRATE1_LEG
#define PRICING_DATA_COUPONRATE1_LEG	"CouponRate1_Leg"
#endif
#ifndef PRICING_DATA_COUPONRATE2_LEG
#define PRICING_DATA_COUPONRATE2_LEG	"CouponRate2_Leg"
#endif
#ifndef PRICING_DATA_COUPONRATE3_LEG
#define PRICING_DATA_COUPONRATE3_LEG	"CouponRate3_Leg"
#endif
#ifndef PRICING_DATA_COUPONRATE4_LEG
#define PRICING_DATA_COUPONRATE4_LEG	"CouponRate4_Leg"
#endif
#ifndef DATAN_N_MV_IR_TODAYFX_SETTLEMENTTONONDELIVERABLE
#define DATAN_N_MV_IR_TODAYFX_SETTLEMENTTONONDELIVERABLE	"TodayFX_SettlementToNonDeliverable"
#endif
#ifndef PRICING_DATA_SETTLEMENTFIXINGDATE_LEG
#define PRICING_DATA_SETTLEMENTFIXINGDATE_LEG			"SettlementFixingDate_Leg"
#endif
#ifndef PRICING_DATA_SETTLEMENTADJUSTRATIO_LEG
#define PRICING_DATA_SETTLEMENTADJUSTRATIO_LEG			"SettlementAdjustRatio_Leg"
#endif

#define PAYOFFBASIS "PAYOFFBASIS"

//// FROTOTYPE ////
class AQLDate;
class AQLObject;
class AQLPriceDataManager;
class LAMathFXEntity;
class LAMathIndexEntity;
class LAMathPathEntity;
class LARatesNumeraireBase;
class LAPriceAccruedInterest;
class LAPolynomialBase;
class AQLDataDoubleMatrix;
class AQLDataBools;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief class that evaluates trade

*/
class LAPriceTradeValue : public AQLCoreValuation
{
public:
    // constructor	
	explicit LAPriceTradeValue(LAPriceAccruedInterest* pacc = 0);
    // destructor	
	virtual ~LAPriceTradeValue();
	// Check function for this class ID	
	virtual bool                isTypeOf(function_t id) const;
    // Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
    // Return this class type
	virtual function_t			getType() const;
	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
    // evaluation function
	virtual double              value(const AQLDate& basedate, 
										AQLObject& object,
									const AQLDataValuation& att) const;


	//Remove warning:C4512
	LAPriceTradeValue & operator=( const LAPriceTradeValue & ) { return *this; }

	/*!
		@brief cache class for performance up
	*/
	class LAPriceTradeValueDataProvider : public AQLDataProvider
	{
	public:
		virtual ~LAPriceTradeValueDataProvider(); 
		
		LAMathPathEntity*					pPath;		// path object
		std::set<LAMathIndexEntity*>		indexs;		// reference index vector
		LARatesNumeraireBase*				pNumeraire; // numeraire
		int								mcnum;		// mc num
		BoolVector						rcvpay;		// rcv(true) or pay(false) flag of each leg
		AQLDate							settledate;	// settle date
		double							settle;		// settle term from today
		double							baseterm;	// term from today to basedate
		bool							isstartable;// startable or not
		LAPricePayOff						payoff;		// payoff class
		AQLCoreFunctionHolder				acc;		// accrued interest function
		DoubleArray						expirytimes;	// call expiry times 
		DoubleArray						actiontimes;	// call action times
		bool							iscall;			// callable or not
		int								lsmcnum;		// lsmc num
		LAPolynomialBase*				poly;			// polynomial function
		AQLDataDoubleMatrix*				coefficient;	// coefficient
		AQLDataDoubleMatrix*				coefficient_rebate;	// coefficient
		std::vector<LAPolynomialBase*>	polies;			// polynomial functions
		std::vector<LAPolynomialBase*>	polies_rebate;		// polynomial functions
		DateVector						actiondates;	// call action dates
		bool							isoptionholder;	// option holder flag	
		std::vector<PayOffToolHolderVector> cashvec;    // cash vector
		DoubleMatrix					paytimes;       // payment timing
		int                             maxcfnum;       // maxcfnum
		AQLString						basecur;		// basecur
		AQLString						numerairecur;	// numerairecur
		AQLString						credit_ccy;		// credit currency
		bool							iscalcswap;		// calc flag for underlying swap
        bool                            isexcludeoutlier_y; //IsExcludeOutlier_Y flag
        bool                            isexcludeoutlier_x; //IsExcludeOutlier_X flag
        bool                            isexcludeoutlier_e; //IsExcludeOutlier_e flag
        double                          criteria_y;            //Criteria_Y value
        double                          criteria_x;            //Criteria_X value
        double                          criteria_e;            //Criteria_e value
		double                          svdTolerance;	//svdTolerance value
		bool                            isshift_y;      //IsShift_Y flag
        bool                            isshift_x;      //IsShift_X flag
		bool                            isscale_y;      //IsScale_Y flag
        bool                            isscale_x;      //IsScale_X flag
		AQLDataBools*                    isconvert_xy;   //isconvert_xy flag
		AQLDataDoubles*                  shift_y;        //shift_y value
		AQLDataDoubles*                  scale_y;        //scale_y value
		AQLDataDoubleMatrix*				shift_x;	    //shift_x value
		AQLDataDoubleMatrix*				scale_x;	    //scale_x value
		AQLDataBools*                    isconvert_xy_rebate;      //isconvert_xy_rebate flag
		AQLDataDoubles*                  shift_y_rebate;       //shift_y_rebate value
		AQLDataDoubles*                  scale_y_rebate;       //scale_y_rebate value
		AQLDataDoubleMatrix*				shift_x_rebate;	    //shift_x_rebate value
		AQLDataDoubleMatrix*				scale_x_rebate;	    //scale_x_rebate value

		AQLDate							valueDate;		// value date
		DoubleArray						basisgrid_payoff;	// grids of yield spread for payoff
		DoubleArray						basisspread_payoff;	// yield spreads for payoff(corresponding to basisgrid_payoff)
		AQLString						basisname_payoff;	// name of spreads for payoff

	};

	virtual double calcFeeValueExo(AQLObject &tradeEntity, LAPriceTradeValueDataProvider* dataProvider) const;

protected:
    // copy constructor
    LAPriceTradeValue(const LAPriceTradeValue& v);	

    // evaluation function
	virtual double              value(const AQLDate& basedate, 
										AQLObject& object, AQLDataProvider* dp,
										unsigned int startpathnum = 0) const;


	// get reference indexs
	std::set<LAMathIndexEntity*>
								getReferenceIndex(AQLObject& trade) const;
	// set up dataProvider
	AQLDataProvider*					setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const;	
	// value by backward
	virtual double				value_backward(const DoubleMatrix& time,
											const DoubleMatrix& cf,
											std::vector<std::pair<unsigned int, AQLDate> >& triggerhit,
											const DoubleArray& rebate,
											const DoubleMatrix& explanatory,
											const AQLDataProvider* dp,
											DoubleVector &callval,
											BoolVector* afterjudge = 0,
											BoolVector* judge = 0,
											DoubleVector* explainedvar = 0) const; 

	// calc ValueTerm ratio
	double calcValueTermRatio(const LAPriceTradeValueDataProvider &dataProvider) const;
	// get FXEntity
	const LAMathFXEntity &getFXEntity(AQLObject &object) const;

	LAPriceAccruedInterest*		mpAcc;// < accrued interest calulatetor
	void clearLSMCPointers(LAPriceTradeValueDataProvider &dataProvider) const; // clear LSMC pointers
private:
	// create new cache class
	virtual	AQLDataProvider*			createNewDataProvider() const;	
};

