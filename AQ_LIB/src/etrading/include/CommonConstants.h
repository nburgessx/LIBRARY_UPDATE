#pragma once

#include <string>

namespace etrading
{
	namespace AQOBJ_KEY
	{
		extern const std::string AQOBJ_OBJECT_COUNTER_DELIMITER;
	}

    namespace OBJECT_KEY
    {
        extern const std::string OBJECT_TYPE;
        extern const std::string TRADE_TYPE;
        extern const std::string MODEL_TYPE;
    }

    namespace IRS_KEY
    {
        // Instruments related
        extern const std::string TRADE_TYPE;
        extern const std::string TRADE_ID;
        extern const std::string NOTIONAL;
        extern const std::string AMORTIZATION;
        extern const std::string AMORT_FREQUENCY;
        extern const std::string NOTIONAL_EXCHANGE;
		extern const std::string EFFECTIVE_DATE;
        extern const std::string MATURITY_DATE;
        extern const std::string PAY_RECEIVE;
        //keep it for backward compatibility with me function example sheets
        extern const std::string PAYER_RECEIVER;
        
        extern const std::string IS_EOM_ROLL;
        extern const std::string COMPOUND_METHOD;
        extern const std::string CURRENCY;
        extern const std::string VALUATION_CURRENCY;
        extern const std::string LEG_TYPE;
        extern const std::string LEVERAGE;
		extern const std::string COUPONMULTIPLIER;
        extern const std::string SCHEDULE_TYPE;

        //For xccy swap:
        extern const std::string FX_RATE;
        extern const std::string NOTIONAL_FX_RESET_LEG;
        extern const std::string MTM;

        //For zero coupon swap:
        extern const std::string FV_NOTIONAL; 

        extern const std::string FIXED_RATE;
        extern const std::string FIXED_FREQUENCY;
        extern const std::string FIXED_DAYCOUNT;
        extern const std::string FIXED_BUSINESSDAYADJUSTMENT;
        extern const std::string FIXED_CALENDAR;
        extern const std::string FIXED_ACCRUALBUSINESSDAYADJUSTMENT;
        extern const std::string FIXED_ACCRUALCALENDAR;
        extern const std::string FIXED_ACCRUALDAYCOUNT;
        extern const std::string FIXED_ACCRUALFREQUENCY;
        extern const std::string FIXED_PAYMENTBUSINESSDAYADJUSTMENT;
        extern const std::string FIXED_PAYMENTCALENDAR;
        extern const std::string FIXED_PAYMENTFREQUENCY;
        extern const std::string FIXED_FIRSTSTUBDATE;
        extern const std::string FIXED_LASTSTUBDATE;
        extern const std::string FIXED_ROLLDAY;
        extern const std::string FIXED_PAYMENTLAG;
        extern const std::string FIXED_STUBTYPE;

        extern const std::string FLOAT_RATE;
        extern const std::string FLOAT_FREQUENCY;
        extern const std::string FLOAT_DAYCOUNT;
        extern const std::string FLOAT_BUSINESSDAYADJUSTMENT;
        extern const std::string FLOAT_CALENDAR;
        extern const std::string FLOAT_FIXINGBUSINESSDAYADJUSTMENT;
        extern const std::string FLOAT_FIXINGCALENDAR;
        extern const std::string FLOAT_FIXINGADVANCEORARREAR;
        extern const std::string FLOAT_ACCRUALBUSINESSDAYADJUSTMENT;
        extern const std::string FLOAT_ACCRUALCALENDAR;
        extern const std::string FLOAT_ACCRUALDAYCOUNT;
        extern const std::string FLOAT_ACCRUALFREQUENCY;
        extern const std::string FLOAT_PAYMENTBUSINESSDAYADJUSTMENT;
        extern const std::string FLOAT_PAYMENTCALENDAR;
        extern const std::string FLOAT_PAYMENTFREQUENCY;
        extern const std::string FLOAT_FIRSTSTUBDATE;
        extern const std::string FLOAT_LASTSTUBDATE;
        extern const std::string FLOAT_ROLLDAY;
        extern const std::string FLOAT_PAYMENTLAG;
        extern const std::string FLOAT_FIXINGLAG;
        extern const std::string FLOAT_FIRSTFIXINGLAG;
        extern const std::string FLOAT_STUBTYPE;
        extern const std::string FLOAT_FIRSTFIXING;
        extern const std::string FLOAT_LASTFIXING;
        extern const std::string FLOAT_SPREAD;
        extern const std::string FLOAT_FIRSTSTUBCURVEINDEX;
        extern const std::string FLOAT_LASTSTUBCURVEINDEX;

        //the following keys will only be used for functions accepting either fixed or float leg
        extern const std::string RATE;
        extern const std::string FREQUENCY;
        extern const std::string DAYCOUNT;
        extern const std::string BUSINESSDAYADJUSTMENT;
        extern const std::string CALENDAR;
        extern const std::string FIXINGBUSINESSDAYADJUSTMENT;
        extern const std::string FIXINGCALENDAR;
        extern const std::string ACCRUALBUSINESSDAYADJUSTMENT;
        extern const std::string ACCRUALCALENDAR;
        extern const std::string PAYMENTBUSINESSDAYADJUSTMENT;
        extern const std::string PAYMENTCALENDAR;
        extern const std::string FIRSTSTUBDATE;
        extern const std::string LASTSTUBDATE;
        extern const std::string ROLLDAY;
        extern const std::string PAYMENTLAG;
        extern const std::string FIXINGLAG;
        extern const std::string FIRSTFIXINGLAG;
        extern const std::string STUBTYPE;
        extern const std::string FIRSTFIXING;
        extern const std::string LASTFIXING;
        extern const std::string ACCRUALFREQUENCY;
        extern const std::string PAYMENTFREQUENCY;
        extern const std::string FIXINGADVANCEORARREAR;
        extern const std::string ACCRUALDAYCOUNT;
        extern const std::string FIRSTSTUBCURVEINDEX;
        extern const std::string LASTSTUBCURVEINDEX;
        extern const std::string FXFIXINGLAG;
        extern const std::string FXFIXINGBUSINESSDAYADJUSTMENT;
        extern const std::string FXFIXINGCALENDAR;

		//This is to allow user to change isFwdInter to true or false for STDCruve
        extern const std::string IS_FWD_INTER;

		// Used to specify whether coupons are credit risky
		extern const std::string PAYMENT_TRIGGER;

		// Specifies the calculation method for inflation swaps.
		extern const std::string INFLATION_RESET_TYPE;

		//FRA key
        extern const std::string STRIKE_RATE;
		extern const std::string FRA_STYLE;

	}

	namespace CASHFLOW_KEY
    {
        extern const std::string ACCRUAL_START;
        extern const std::string ACCRUAL_END;
        extern const std::string FIXING_DATE;
        extern const std::string PAYMENT_DATE;
    }

	namespace FEE_KEY
    {
        extern const std::string AMOUNT;
    }

	namespace CDS_KEY
    {
		extern const std::string CDS_SPREAD;
        extern const std::string HAZARD_RATE;
        extern const std::string RECOVERY_RATE;
	}

	namespace CMS_KEY
	{
		extern const std::string CMS_GENERATOR_NAME1;		// The SwapGenerator used to build the underlying swap index
		extern const std::string CMS_INDEX_MATURITY1;		// The maturity tenor of the underlying swap index
		extern const std::string CMS_INDEX_MULTIPLIER1;		// A multiplier factor on the swap index par-rate

		extern const std::string CMS_GENERATOR_NAME2;
		extern const std::string CMS_INDEX_MATURITY2;
		extern const std::string CMS_INDEX_MULTIPLIER2;
	}

	namespace TRS_KEY
	{
		extern const std::string BOND_NAME;
		extern const std::string ASSET_PERFORMANCE;
	}

    namespace ASSET_SWAP_KEY
    {
        extern const std::string IS_CLEAN_PRICE;
        extern const std::string ISSUE_DATE;
    }

    namespace MARKET_KEY
    {
        extern const std::string CURVE_COLLECTION;
        extern const std::string FORECAST_CURVE;
        extern const std::string DISCOUNT_CURVE;
        extern const std::string MARKET_TYPE_BASISSWAP;

        extern const std::string CURVE_TYPE_SWAP;
        extern const std::string CURVE_TYPE_OIS;
		extern const std::string CURVE_TYPE_ARR;
        extern const std::string CURVE_TYPE_BASIS;
		extern const std::string CURVE_TYPE_TENORBASIS;
        extern const std::string CURVE_TYPE_XCCYBASIS;
		extern const std::string CURVE_TYPE_FWDFXCONST;
		extern const std::string CURVE_TYPE_FX;

        extern const std::string LEG1FORECAST;
        extern const std::string LEG2FORECAST;
        extern const std::string LEG1DISCOUNT;
        extern const std::string LEG2DISCOUNT;
    }

    namespace TRADE_TYPE_KEY
    {
        extern const std::string TRADETYPE_VANILLA_IRS;
        extern const std::string TRADETYPE_OIS;
    }

    namespace RISK_KEY
    {
        extern const std::string DELTA_LADDER;
        extern const std::string DELTA_FLATSHIFT;
    }

    namespace PRICING_PARAMS
    {
        extern const std::string INTERPOLATION;
        extern const std::string COMPOUND_METHOD;
    }

    namespace SWAP_STUB
    {
        extern const std::string STUB_TOLERANCE;
        extern const std::string USE_CURVE_INDEX;
    }

    namespace SWAP_EXPRESSION_KEY
    {
        extern const std::string PAY_RECEIVE1;
        extern const std::string PAY_RECEIVE2;
        extern const std::string NOTIONAL1;
        extern const std::string NOTIONAL2;
        extern const std::string RATE_OR_SPREAD1;
        extern const std::string RATE_OR_SPREAD2;
        extern const std::string FIRSTFIXING1;
        extern const std::string LASTFIXING1;
        extern const std::string FIRSTFIXING2;
        extern const std::string LASTFIXING2;
		extern const std::string ROLLDAY; // this is used for the AQObj FX Curve
        extern const std::string IS_FWD_INTER1;
        extern const std::string IS_FWD_INTER2;
    }

    namespace BOND_KEY
    {
        extern const std::string AQOBJ_BOND_DESCRIPTION_LVB;  // AQObj SERIALIZATION LABEL FOR BOND DESCRIPTION LVB
        extern const std::string AQOBJ_BOND_SCHEDULE_LVB;     // AQObj SERIALIZATION LABEL FOR BOND SCHEDULE LVB

        extern const std::string BOND_DESCRIPTION;
        extern const std::string BOND_TYPE;
        extern const std::string BOND_QUOTE_CONVENTION;
        extern const std::string CALCULATION_TYPE;
        extern const std::string CURRENCY;
        extern const std::string ISIN;
        extern const std::string YIELD_TYPE;
        extern const std::string IS_CLEAN_PRICE;
        extern const std::string COUPON;
        extern const std::string FACE_VALUE;
        extern const std::string ISSUE_DATE;
        extern const std::string ACCRUAL_START_DATE;
        extern const std::string FIRST_COUPON_DATE;
        extern const std::string LAST_COUPON_DATE;
        extern const std::string ISSUE_PRICE;
        extern const std::string TAX_RATE;
        extern const std::string YIELD_FREQUENCY;
		extern const std::string EX_DIVIDEND_TENOR;
		extern const std::string EX_DIVIDEND_BUSINESSDAYADJUSTMENT;
		extern const std::string QUOTED_MARGIN; // For Floating Bond
	}

	namespace CURVEGENERATOR_CURVEPROPERTIES_KEY
	{
		extern const std::string CURVE_TYPE;
		extern const std::string FREQUENCY;
		extern const std::string CURVE_INDEX_FREQUENCY;
		extern const std::string STATIC_DATA_TABLE;
		extern const std::string INDEX_NAME;
		extern const std::string CURRENCY;
		extern const std::string INTERPOLATION;
		extern const std::string YIELDGEN_INTERPOLATION;
		extern const std::string BASIS_INTERPOLATION;
		extern const std::string FUTURE_INTERPOLATION;
		extern const std::string FRA_INTERPOLATION;
		extern const std::string DAYCOUNT;
		extern const std::string IS_FRA_USE;
		extern const std::string IS_FUTURE_USE;
		extern const std::string IS_FWD_BASIS;
		extern const std::string IS_FWD_FX;
		extern const std::string IS_RENOTIONAL_ADJUST;
		extern const std::string IS_XCCY_MARKED_TO_MARKET;
		extern const std::string DF_CURVE_NAME;
		extern const std::string GENERATE_FORWARDS_FROM_SWAPS_ONLY;
        extern const std::string FIXING_SOURCE;
		extern const std::string INTERPOLATION_JOIN_DATE;
        extern const std::string ALWAYS_CALC_JOIN_DATE;
        extern const std::string FAST_REBUILD;
        extern const std::string IS_SWAP_TENOR_ADJUST;
		extern const std::string USE_TENOR_BASIS;
		extern const std::string TENOR_SWAP_NAME;
        extern const std::string JACOBIAN_BUILD_FREQUENCY;
        extern const std::string JACOBIAN_SHIFT_TYPE;
        extern const std::string JACOBIAN_RISK_TYPE;
		extern const std::string JACOBIAN_GRADIENT_SHIFT_SIZE;
		extern const std::string JACOBIAN_MARKET_DATA_SHIFT_SIZE;
	}

	namespace CURVEGENERATOR_OIS_KEY
	{
		extern const std::string CALENDAR;
		extern const std::string DAYCOUNT;
		extern const std::string FREQUENCY;
		extern const std::string RESET_LAG;
		extern const std::string SLIDING_RULE;
		extern const std::string GENERATE_METHOD;
		extern const std::string FIRST_RATE;
		extern const std::string SHORT_TERM_CONVENTION;
		extern const std::string EPSILON;
		extern const std::string MAX_LOOP;
		extern const std::string SMOOTH_SHORT_END;
		extern const std::string COMPOUNDING_METHOD;
		extern const std::string LONG_TERM_CONVENTION;
		extern const std::string LONG_TERM;
		extern const std::string LONG_TERM_GENERATE_METHOD;
		extern const std::string LONG_TERM_GENERATE_METHOD_ARROIS;
        extern const std::string MEAN_REVERSION;
		extern const std::string IS_EOM_ROLL;
		extern const std::string EOM_DAY;
	}

	namespace CURVEGENERATOR_LIBOROISBASIS_KEY
	{
		extern const std::string CALENDAR;
		extern const std::string DAYCOUNT;
		extern const std::string FREQUENCY;
		extern const std::string SLIDING_RULE;
		extern const std::string CALENDAR_ARROIS;
		extern const std::string DAYCOUNT_ARROIS;
		extern const std::string FREQUENCY_ARROIS;
		extern const std::string SLIDING_RULE_ARROIS;
	}

	namespace CURVEGENERATOR_SWAPS_KEY
	{
		extern const std::string CALENDAR;
		extern const std::string DAYCOUNT;
		extern const std::string DAYCOUNT_FIX;
		extern const std::string DAYCOUNT_FLOAT;
		extern const std::string FREQUENCY;
		extern const std::string FREQUENCY_FIX;
        extern const std::string FREQUENCY_FIX_1Y;
        extern const std::string FREQUENCY_FIX_2Y;
        extern const std::string FREQUENCY_FIX_3Y;
        extern const std::string FREQUENCY_FIX_4Y;
        extern const std::string FREQUENCY_FIX_5Y;
		extern const std::string FREQUENCY_FLOAT;
        extern const std::string FREQUENCY_FLOAT_1Y;
        extern const std::string FREQUENCY_FLOAT_2Y;
        extern const std::string FREQUENCY_FLOAT_3Y;
        extern const std::string FREQUENCY_FLOAT_4Y;
        extern const std::string FREQUENCY_FLOAT_5Y;
		extern const std::string INTERPOLATION;
		extern const std::string RESET_LAG;
		extern const std::string SLIDING_RULE;
		extern const std::string IS_NEWTON_RAPHSON;
		extern const std::string IS_SIMULTANEOUS_EQ;
		extern const std::string EPSILON;
		extern const std::string MAX_LOOP;
		extern const std::string IS_EOM_ROLL;
		extern const std::string EOM_DAY;
        extern const std::string BASE_FREQUENCY_FLOAT;

		extern const std::string CALENDAR_ARROIS;
		extern const std::string SLIDING_RULE_ARROIS;
		extern const std::string DAYCOUNT_FIX_ARROIS;
		extern const std::string FREQUENCY_FIX_ARROIS;

	}

	namespace CURVEGENERATOR_MONEYMARKET_KEY
	{
		extern const std::string CALENDAR;
		extern const std::string DAYCOUNT;
		extern const std::string SLIDING_RULE;
	}
	
	namespace CURVEGENERATOR_LIBORFIXINGS_KEY
	{
		extern const std::string CALENDAR;
		extern const std::string DAYCOUNT;
		extern const std::string FREQUENCY;
		extern const std::string RESET_LAG;
		extern const std::string SLIDING_RULE;
		extern const std::string IS_EOM_ROLL;
		extern const std::string EOM_DAY;
	}
	
	namespace CURVEGENERATOR_FRAS_KEY
	{
		extern const std::string CALENDAR;
		extern const std::string DAYCOUNT;
		extern const std::string RESET_LAG;
		extern const std::string SLIDING_RULE;
        extern const std::string IS_EOM_ROLL;
		extern const std::string APPLY_TENSION;
		extern const std::string TENSION_GAP;
		extern const std::string SMOOTH_SHORT_END;
	}

	namespace CURVEGENERATOR_FUTURES_KEY
	{
		extern const std::string CALENDAR;
		extern const std::string DAYCOUNT;
		extern const std::string RESET_LAG;
		extern const std::string SLIDING_RULE;
		extern const std::string MEAN_REVERSION;
		extern const std::string USE_CONVEX_ADJUSTMENT; // Legacy Parameter meaning convexityQuotedAsPrice
		extern const std::string CONVEXITY_QUOTE_TYPE;	// Alias for USE_CONVEX_ADJUSTMENT
		extern const std::string APPLY_TENSION;
		extern const std::string TENSION_GAP;
		extern const std::string SMOOTH_SHORT_END;
		extern const std::string SERIAL_CALC_TYPE;
		extern const std::string INSTRUMENT_TYPE;
		extern const std::string INTERPOLATION_JOIN_DATE;
	}

	namespace CURVEGENERATOR_BASISSWAPS_KEY
	{
		extern const std::string MARKET_NAME;
        extern const std::string ADJUST_VALUE_INTERPOLATION;
		extern const std::string IS_LEG1_SPREAD;
		extern const std::string LEG1_CASHLET_CALENDAR;
		extern const std::string LEG1_CASHLET_DAYCOUNT;
		extern const std::string LEG1_CASHLET_FREQUENCY;
		extern const std::string LEG1_CASHLET_FREQUENCY_COMPOUND;
		extern const std::string LEG1_CASHLET_SLIDING_RULE;
		extern const std::string LEG1_CASHLET_SPOT_LAG;
		extern const std::string LEG1_FORECAST;
		extern const std::string LEG1_DISCOUNT;
		extern const std::string LEG1_INDEX_ACCESSARY;
		extern const std::string LEG1_INDEX_DAYCOUNT;
		extern const std::string LEG1_INDEX_FIXING_CALENDAR;
		extern const std::string LEG1_INDEX_FREQUENCY;
		extern const std::string LEG1_INDEX_PAYMENT_CALENDAR;
		extern const std::string LEG1_INDEX_RESET_LAG;
		extern const std::string LEG1_INDEX_SLIDING_RULE;

		extern const std::string LEG2_CASHLET_CALENDAR;
		extern const std::string LEG2_CASHLET_DAYCOUNT;
		extern const std::string LEG2_CASHLET_FREQUENCY;
		extern const std::string LEG2_CASHLET_FREQUENCY_COMPOUND;
		extern const std::string LEG2_CASHLET_SLIDING_RULE;
		extern const std::string LEG2_CASHLET_SPOT_LAG;
		extern const std::string LEG2_FORECAST;
		extern const std::string LEG2_DISCOUNT;
		extern const std::string LEG2_INDEX_ACCESSARY;
		extern const std::string LEG2_INDEX_DAYCOUNT;
		extern const std::string LEG2_INDEX_FIXING_CALENDAR;
		extern const std::string LEG2_INDEX_FREQUENCY;
		extern const std::string LEG2_INDEX_PAYMENT_CALENDAR;
		extern const std::string LEG2_INDEX_RESET_LAG;
		extern const std::string LEG2_INDEX_SLIDING_RULE;

		extern const std::string TARGET;
		extern const std::string IS_SIMULTANEOUS_EQ;
		extern const std::string FWD_INTERPOLATION;
		extern const std::string IS_SAME_GRID_INDEX;
		extern const std::string SPOT_RATE_TERM;
		extern const std::string IS_YIELD_SPREAD_CALC;
		extern const std::string EPSILON;
		extern const std::string MAX_LOOP;
		extern const std::string IS_FWD_INTER;
		extern const std::string IS_EOM_ROLL;
		extern const std::string EOM_DAY;
	}

	namespace CURVEGENERATOR_FXFWDS_KEY
	{
		extern const std::string CALENDAR;
		extern const std::string RESET_LAG;
		extern const std::string SLIDING_RULE;
		extern const std::string IS_PRICE_CCY;
		extern const std::string IS_DOMESTIC_CURRENCY;
		extern const std::string IS_RATIO;
		extern const std::string IS_FX_OUTRIGHT;
		extern const std::string IS_EOM_ROLL;
		extern const std::string EOM_DAY;
		extern const std::string DENOMINATOR;
		extern const std::string PIPSIZE;
		extern const std::string FREQUENCY;
		extern const std::string FIXEDRATEXCCYSTARTTENOR;
	}

	namespace CURVEGENERATOR_FWDFXCONST_KEY
	{
		extern const std::string LEG1_FORECAST;
		extern const std::string LEG1_DISCOUNT;
		extern const std::string LEG2_FORECAST;
		extern const std::string LEG2_DISCOUNT;
		extern const std::string TARGET;
	}

	namespace GENERATOR_COMPONENTS
	{
		extern const std::string KEY_CURVEPROPERTIES;
		extern const std::string KEY_MARKETDATAPROPERTIES;
		extern const std::string KEY_OIS;
		extern const std::string KEY_OISFIXINGS;		
		extern const std::string KEY_LIBOROISBASISSPREADS;
		extern const std::string KEY_LIBOROISBASIS;
		extern const std::string KEY_LIBORFIXINGS;
		extern const std::string KEY_SWAPS;
		extern const std::string KEY_MONEYMARKET;
		extern const std::string KEY_FRAS;
		extern const std::string KEY_FUTURES;
		extern const std::string KEY_BASISSWAPS;
		extern const std::string KEY_XCCYSWAPS;
		extern const std::string KEY_FXFWDS;
		extern const std::string KEY_FXSPOTS;
		extern const std::string KEY_FWDFXCONST;
		extern const std::string KEY_FORWARDADJUSTMENTS;
	}

	// These are the keys which are permitted in the CREDIT_MODEL LVB
	namespace CREDITMODEL_MODEL_PROPERTIES_KEY
	{
		extern const std::string ASOF_DATE;
		extern const std::string SPOT_LAG;
		extern const std::string SPOT_BUSINESSDAY_ADJUSTMENT;
		extern const std::string SPOT_CALENDAR;
		extern const std::string ACCRUAL_START_DATE;
		extern const std::string IMM_REFERENCE_DATE;
		extern const std::string CURRENCY;
		extern const std::string RECOVERY_RATE;
		extern const std::string INCLUDE_ACCRUED_INTEREST;
		extern const std::string BOND_HAS_RISKY_ACCRUED_INTEREST;
		extern const std::string SWAP_GENERATOR;
		extern const std::string CREDIT_INDEX;
		extern const std::string INTERPOLATION;
		extern const std::string EXTRAPOLATION;
		extern const std::string CDS_CURVE_COLLECTION;
		extern const std::string BOND_CURVE_COLLECTION;
		extern const std::string BOND_DISCOUNT_CURVE;
	}

	namespace SABR_MARKETDATA_PROPERTIES_KEY
	{
		extern const std::string MARKET_DATA_TYPE;
		extern const std::string STRIKE_ADJUST_AMOUNT;
		extern const std::string VALUE_TYPE;
	}

	namespace SABR_MODEL_PROPERTIES_KEY
	{
		extern const std::string AS_OF_DATE;
		extern const std::string CURVE_COLLECTION;
		extern const std::string SWAP_GENERATOR;
		extern const std::string CALENDAR;
		extern const std::string DAYCOUNT;
		extern const std::string BUSINESSDAY_ADJUSTMENT;
		extern const std::string APPROX_METHOD;
		extern const std::string CALIBRATION_METHOD;
		extern const std::string TARGET;
		extern const std::string VOL_TYPE;
		extern const std::string SHIFT;
		extern const std::string SOLVER_EPSILON;
		extern const std::string ALPHA_FROM_ATM_VOL;
		
		extern const std::string ALPHA_PARAM;
		extern const std::string BETA_PARAM;
		extern const std::string NU_PARAM;
		extern const std::string RHO_PARAM;

		extern const std::string ATMF_SWAPRATES;
		extern const std::string ANNUITY;

	}

	// These are the keys which are permitted in the CREDIT_BASKET_MODEL LVB
	namespace CREDITBASKETMODEL_MODEL_PROPERTIES_KEY
	{
		extern const std::string ASOF_DATE;
		extern const std::string BASKET_TYPE;
		extern const std::string NTH_TO_DEFAULT;
	}

	// These are the keys which are permitted in the BOND_STATICDATA LVB
	namespace BONDGENERATOR_STATICDATA_KEY
	{
		extern const std::string BOND_TYPE;
		extern const std::string CURRENCY;
		extern const std::string YIELD_TYPE;
		extern const std::string IS_CLEAN_PRICE;
		extern const std::string BOND_QUOTE_CONVENTION;
	}

	// These are the keys which are permitted in the BOND_SCHEDULE LVB
	namespace BONDGENERATOR_SCHEDULE_KEY
	{
		extern const std::string PAY_RECEIVE;
		extern const std::string NOTIONAL_EXCHANGE;
		extern const std::string FACE_VALUE;
		extern const std::string FREQUENCY;
		extern const std::string ACCRUAL_BUSINESSDAY_ADJUSTMENT;
		extern const std::string ACCRUAL_CALENDAR;
		extern const std::string PAYMENT_BUSINESSDAY_ADJUSTMENT;
		extern const std::string PAYMENT_CALENDAR;
		extern const std::string PAYMENT_LAG;
		extern const std::string DAYCOUNT;
		extern const std::string CALCULATION_TYPE;
        extern const std::string YIELD_FREQUENCY;
        extern const std::string TAX_RATE;
	}

	// The BondExpression LVB contains keys which are specific to the concrete bond.
	// These keys are used to further customize the bond, further to the settings provided by the BondGenerator.
	namespace BONDGENERATOR_EXPRESSION_KEY
	{
		extern const std::string BOND_DESCRIPTION;
		extern const std::string ISIN;
		extern const std::string ISSUE_DATE;
		extern const std::string FIRST_COUPON_DATE;
		extern const std::string LAST_COUPON_DATE;
		extern const std::string MATURITY_DATE;
		extern const std::string COUPON;
		extern const std::string ROLLDAY;
		extern const std::string ACCRUAL_START_DATE;
        extern const std::string ISSUE_PRICE;
        extern const std::string FACE_VALUE;          // Duplicate Key - Intentional to allow generator override - expression LVB takes priority over bond generator
        extern const std::string FREQUENCY;           // Duplicate Key - Intentional to allow generator override - expression LVB takes priority over bond generator
        extern const std::string DAYCOUNT;            // Duplicate Key - Intentional to allow generator override - expression LVB takes priority over bond generator
        extern const std::string YIELD_FREQUENCY;     // Duplicate Key - Intentional to allow generator override - expression LVB takes priority over bond generator
        extern const std::string TAX_RATE;            // Duplicate Key - Intentional to allow generator override - expression LVB takes priority over bond generator
		extern const std::string QUOTED_MARGIN;
	}

	namespace BONDGENERATOR_COMPONENTS
	{
		extern const std::string KEY_BOND_STATICDATA;
		extern const std::string KEY_BOND_SCHEDULE;
	}

	// These are the keys which are permitted in the BONDCURVE PROPERTIES block
	namespace BONDCURVE_PROPERTIES_KEY
	{
		extern const std::string SETTLEMENT_DATE;
		extern const std::string YIElD_CALCULATION_TYPE;
		extern const std::string YIELD_QUOTE_IN_PERCENT;
		extern const std::string INTERPOLATION;
		extern const std::string EXTRAPOLATION;
	}
	
	// These are the keys which are permitted in the BONDSPREADCURVE PROPERTIES block
	namespace BONDSPREADCURVE_PROPERTIES_KEY
	{
		extern const std::string SPREAD;
		extern const std::string BENCHMARK_BOND_CURVE;
	}

	namespace INTERPOLATION_KEYS
	{
		extern const std::string PIECEWISE_CONSTANT;
		extern const std::string FLAT;
	}

	namespace OPTION_KEYS
	{
        extern const std::string TRADE_TYPE;
		extern const std::string AS_OF_DATE;
        extern const std::string LONG_SHORT;
		extern const std::string OPTION_STYLE;
        extern const std::string CALL_PUT;
		extern const std::string STRIKE;
		extern const std::string VOL_DATA_SOURCE;
		extern const std::string VOL_TYPE;
		extern const std::string VOLATILITY;
		extern const std::string EFFECTIVE_DATE;
        extern const std::string EXPIRY;
		extern const std::string DAYCOUNT;
		extern const std::string SPOT_LAG; 		//The lag from curve's asOfDate to valuationDate
		extern const std::string SPOT_BUSINESSDAYADJUSTMENT; 		
		extern const std::string SPOT_CALENDAR; 		
		extern const std::string DELIVERY_DATE; //Underlying delivery date
		extern const std::string BOND_NAME; // Underlying Bond name
		extern const std::string FUTURE_NAME; // Underlying Future name
	}

    namespace VOLATILITY_KEYS
    {
        extern const std::string VOL_DATA_SOURCE;   // SWAPTION_VOL, CAP_VOL, HISTORICAL_VOL etc
		extern const std::string VOL_TYPE;          // NORMAL, LOG-NORMAL, SHIFTED-LOGNORMAL
		extern const std::string VOLATILITY;        // VOLATILITY %
		extern const std::string SHIFT_SIZE;        // SHIFT SIZE WHEN USING LOG-NORMAL VOL
    }

	namespace SWAPTION_KEYS
	{
        extern const std::string TRADE_TYPE;
		extern const std::string OPTION_STYLE;
		extern const std::string LONG_SHORT;
        extern const std::string PAYER_RECEIVER;
		extern const std::string TENOR_DESCRIPTION;
		extern const std::string EFFECTIVE_DATE;
        extern const std::string EXPIRY;
        extern const std::string SETTLEMENT_TYPE;
        extern const std::string CURRENCY;
        extern const std::string NOTIONAL;
        extern const std::string LEVERAGE;
        extern const std::string STRIKE;
        extern const std::string SWAP_GENERATOR;
		extern const std::string SWAP_START;
		extern const std::string SWAP_END;
		extern const std::string NOTIFICATION_DAYS;
		extern const std::string DAYCOUNT;
        extern const std::string BUSINESSDAYADJUSTMENT;
        extern const std::string CALENDAR;
        extern const std::string FEE;
        extern const std::string FEE_DATE;
        extern const std::string FEE_PAY_RECEIVE;
	}

	namespace CREDIT_OPTION_KEYS
	{
		extern const std::string PAYER_RECEIVER;
		extern const std::string STRIKE;
		extern const std::string EXPIRY;
		extern const std::string CDS_START_DATE;
		extern const std::string CDS_MATURITY_DATE;
		extern const std::string CDS_COUPON;
		extern const std::string VOLATILITY;
		extern const std::string FORWARD_SPREAD;
		extern const std::string OPTION_VALUE;
		extern const std::string SCALE_STRIKE_BY_SURVIVAL_PROBABILITY;	// Controls whether the adjusted strike is scaled by survival probability. O'Kane sets this to true.
		extern const std::string NUMBER_OF_UNDERLIERS;					// How many underliers in the credit index. Default is 125
		extern const std::string NUMBER_OF_REALIZED_DEFAULTS;			// How many underliers have defaulted since the credit option began
		extern const std::string DEFAULT_SETTLEMENT_AMOUNT;				// An override allowing the user to specify the payout on defaulted underliers,
																		// if they choose to exercise the credit option
	}

	namespace VALUATION_SETTING_KEYS
	{
		//Support Swap price with specific valuation date, e.g. pricing pv and spread for assetSwap
		extern const std::string VALUATION_DATE;
		extern const std::string SETTLEMENT_DATE;
		extern const std::string CURVE_COLLECTION;
        extern const std::string CURVE_OBJECT;
        extern const std::string CREDIT_MODEL;
        extern const std::string FX_SPOT;
		extern const std::string FX_AS_OF_DATE_RATE;
		extern const std::string VOLATILITY_MODEL;
		extern const std::string PRICING_MODEL;
		extern const std::string CONVEXITY_METHOD;

		extern const std::string FLOAT_BOND_CURRENT_COUPON;		// The annualized coupon rate for the next floating bond coupon
		extern const std::string FLOAT_BOND_ASSUMED_RATE;		// The average underlying index level for projected coupons
		extern const std::string FLOAT_BOND_INDEX_TO_NEXT_COUPON;	// The index discount rate for the very next floating bond coupon
		extern const std::string FLOAT_BOND_QUOTED_MARGIN;		// The floating bond spread over the underlying index
	}

	namespace MONTE_CARLO_PROPERTIES_KEYS
	{
		extern const std::string RANDOM_NUMBER_GENERATOR;
		extern const std::string DISTRIBUTION;
		extern const std::string NUMBER_OF_PATHS;
		extern const std::string ANTITHETIC_SAMPLING;
		extern const std::string EVALUATE_IN_PARALLEL;
		extern const std::string NUMBER_OF_ASSETS;
	}

	namespace STRUCTURED_CREDIT_CSV_KEYS
	{
		extern const std::string EXPOSURE_AT_DEFAULT;	// Total value the bank is exposed to when a loan defaults.
		extern const std::string RISK_WEIGHT;			// Represents the proportion of capital required to be set aside as a result of providing the loan
		extern const std::string KIRB;					// Exposure weighted average capital charge calculated using Internal Ratings Based approach
		extern const std::string MATURITY;				// Maturity of the loan
		extern const std::string MATURITY_MONTH;		// Maturity of the loan in months
		extern const std::string PEAD;					// Exposure at default
		extern const std::string LOAN_TYPE;				// 0 = Amortizing, 1 = Bullet
		extern const std::string RATE;					// Loan annual interest rate
		extern const std::string FREQ;					// Loan interest payment frequency
		extern const std::string PD_REGULATORY;			// Regulatory Probability of Default
		extern const std::string PDP;					// Proportion Probability of Default
		extern const std::string LGD;					// Loss Given Default
	}

	namespace STRUCTURED_CREDIT_KEYS
	{
		extern const std::string TRIGGER_NAME_PRO_RATA_SEQUENTIAL_VALE;
		extern const std::string TRIGGER_NAME_PRO_RATA_SEQUENTIAL_VALE_CUMULATIVE;
		extern const std::string TRIGGER_NAME_REINVEST;
		extern const std::string TRIGGER_NAME_REINVEST_VALE;
	}

	namespace TRANCHE_KEYS
	{
		extern const std::string SENIORITY_RANK;
		extern const std::string TRANCHE_SIZE;
		extern const std::string TRANCHE_PAY_TYPE;
		extern const std::string COUPON_TYPE;
		extern const std::string COUPON_RATE;
		extern const std::string COUPON_FLOOR;
		extern const std::string COUPON_FREQUENCY;
		extern const std::string CURVE_COLLECTION;
		extern const std::string RESET_CURVE;
		extern const std::string DISCOUNT_MARGIN;
		extern const std::string FIXING_TABLE;
	}

	namespace SYNTHETIC_EXCESS_SPREAD_KEYS
	{
		extern const std::string PERIODS;
		extern const std::string RESET_FREQUENCY;
		extern const std::string USE_OR_LOSE;
	}

	namespace  CASHFLOW_ENGINE_KEYS
	{
		extern const std::string KIRB;
		extern const std::string ELGD;
		extern const std::string REGULATORY_CAPITAL_MATURITY;
		extern const std::string DIVERSITY_METRIC;
		extern const std::string SUPERVISORY_TYPE;
		extern const std::string POOL_TYPE;
		extern const std::string EFFECTIVE_DATE;
		extern const std::string MATURITY_DATE;
		extern const std::string REINVESTMENT_END_DATE;
		extern const std::string REINVESTMENT_END_PERIOD;
	}

    namespace OBJECT_POOL_KEYS
    {
        extern const std::string TERMS;
        extern const std::string DISCOUNT_FACTORS;
    }

    namespace CURVE_ENGINE_KEYS
    {
        extern const std::string OIS_CURVE_CONVENTIONS;
        extern const std::string SWAP_CURVE_CONVENTIONS;
        extern const std::string TENOR_BASIS_CURVE_CONVENTIONS;
        extern const std::string XCCY_BASIS_CURVE_CONVENTIONS;
        extern const std::string DAYCOUNT;
        extern const std::string DAYCOUNT_FLOAT;
        extern const std::string LEG1_DAYCOUNT;
        extern const std::string LEG2_DAYCOUNT;
        extern const std::string TARGET;
        extern const std::string LEG1_FORECAST_RATES;
        extern const std::string LEG1_DISCOUNT_FACTORS;
    }

	// These keys may appear in the FixingTable PameterLVB
	namespace FIXING_TABLE_KEYS
	{
		extern const std::string TABLE_TYPE;

		// RATE Fixing parameters
		extern const std::string CURRENCY;
		extern const std::string TENOR;

		// Additional parameters for INFLATION
		extern const std::string INFLATION_INDEX;

		// FX parameters
		extern const std::string CURRENCY_PAIR;
	}

	// These keys used to serialize / deserialise a fixing table object
	namespace FIXING_TABLE_SERIALIZATION_KEYS
	{
		extern const std::string FIXING_TABLE;
		extern const std::string FIXING_TABLE_PARAMETERS;
		extern const std::string FIXING_TABLE_PARAMETERLVB;
		extern const std::string PARAMETER_NAME;
		extern const std::string PARAMETER_VALUE;
	}

	// These are the keys which are permitted in the INFLATION_CURVE LVB
	namespace INFLATION_CURVE_PROPERTIES_KEY
	{
		extern const std::string ASOF_DATE;
		extern const std::string SPOT_LAG;
		extern const std::string SPOT_BUSINESSDAY_ADJUSTMENT;
		extern const std::string SPOT_CALENDAR;
		extern const std::string CURRENCY;
		extern const std::string SWAP_GENERATOR;
		extern const std::string ANNUAL_INTERPOLATION;
		extern const std::string INFLATION_INDEX;
		extern const std::string CURVE_COLLECTION;
		extern const std::string FIXING_TABLE;
	}

}
