#include "CommonConstants.h"

namespace etrading
{
	namespace AQO_KEY
	{
		const std::string AQO_OBJECT_COUNTER_DELIMITER			= ":";
	}

    namespace OBJECT_KEY
    {
        const std::string OBJECT_TYPE                           = "OBJECTTYPE";
        const std::string TRADE_TYPE                            = "TRADETYPE";
        const std::string MODEL_TYPE                            = "MODELTYPE";
    }

    namespace IRS_KEY
    {
        // Instruments related
        const std::string TRADE_TYPE			                = "TRADETYPE";
        const std::string TRADE_ID					            = "TRADEID";
        const std::string NOTIONAL				                = "NOTIONAL";
        const std::string AMORTIZATION				            = "AMORTIZATION";
        const std::string AMORT_FREQUENCY   	                = "AMORTFREQUENCY";
        const std::string NOTIONAL_EXCHANGE		                = "NOTIONALEXCHANGE";
        const std::string EFFECTIVE_DATE		                = "EFFECTIVEDATE";
        const std::string MATURITY_DATE			                = "MATURITYDATE";

        const std::string PAY_RECEIVE		                    = "PAYRECEIVE";
        //keep it for backward compatibility with me example sheets
        const std::string PAYER_RECEIVER		                = "PAYERRECEIVER";

        const std::string IS_EOM_ROLL			                = "ISEOMROLL";
        const std::string COMPOUND_METHOD		                = "COMPOUNDMETHOD";
        const std::string CURRENCY								= "CURRENCY";
        const std::string VALUATION_CURRENCY					= "VALUATIONCURRENCY";
        const std::string LEG_TYPE								= "LEGTYPE";
        const std::string LEVERAGE								= "LEVERAGE";
		const std::string COUPONMULTIPLIER						= "COUPONMULTIPLIER";
        const std::string SCHEDULE_TYPE							= "SCHEDULETYPE";

        //xccy swap:
        const std::string FX_RATE							    = "FXRATE";
        const std::string NOTIONAL_FX_RESET_LEG				    = "NOTIONALFXRESETLEG";
        const std::string MTM					                = "MTM";

        //For zero coupon swap:
        //Future Value Notional
        const std::string FV_NOTIONAL					        = "FVNOTIONAL";

		const std::string FIXED_RATE			                = "FIXEDRATE";
        const std::string FIXED_FREQUENCY		                = "FIXEDFREQUENCY";
        const std::string FIXED_DAYCOUNT		                = "FIXEDDAYCOUNT";
        const std::string FIXED_BUSINESSDAYADJUSTMENT	        = "FIXEDBUSINESSDAYADJUSTMENT";
        const std::string FIXED_CALENDAR		                = "FIXEDCALENDAR";
        const std::string FIXED_ACCRUALBUSINESSDAYADJUSTMENT	= "FIXEDACCRUALBUSINESSDAYADJUSTMENT";
        const std::string FIXED_ACCRUALCALENDAR		            = "FIXEDACCRUALCALENDAR";
        const std::string FIXED_ACCRUALDAYCOUNT		            = "FIXEDACCRUALDAYCOUNT";
        const std::string FIXED_ACCRUALFREQUENCY		        = "FIXEDACCRUALFREQUENCY";
        const std::string FIXED_PAYMENTBUSINESSDAYADJUSTMENT	= "FIXEDPAYMENTBUSINESSDAYADJUSTMENT";
        const std::string FIXED_PAYMENTCALENDAR		            = "FIXEDPAYMENTCALENDAR";
        const std::string FIXED_PAYMENTFREQUENCY                = "FIXEDPAYMENTFREQUENCY";
        const std::string FIXED_FIRSTSTUBDATE	                = "FIXEDFIRSTSTUBDATE";
        const std::string FIXED_LASTSTUBDATE	                = "FIXEDLASTSTUBDATE";
        const std::string FIXED_ROLLDAY			                = "FIXEDROLLDAY";
        const std::string FIXED_PAYMENTLAG		                = "FIXEDPAYMENTLAG";
        const std::string FIXED_STUBTYPE		                = "FIXEDSTUBTYPE";

        const std::string FLOAT_RATE			                = "FLOATRATE";
        const std::string FLOAT_FREQUENCY		                = "FLOATFREQUENCY";
        const std::string FLOAT_DAYCOUNT		                = "FLOATDAYCOUNT";
        const std::string FLOAT_BUSINESSDAYADJUSTMENT	        = "FLOATBUSINESSDAYADJUSTMENT";
        const std::string FLOAT_CALENDAR		                = "FLOATCALENDAR";
        const std::string FLOAT_FIXINGBUSINESSDAYADJUSTMENT	    = "FLOATFIXINGBUSINESSDAYADJUSTMENT";
        const std::string FLOAT_FIXINGCALENDAR		            = "FLOATFIXINGCALENDAR";
        const std::string FLOAT_FIXINGADVANCEORARREAR			= "FLOATFIXINGADVANCEORARREAR";
        const std::string FLOAT_ACCRUALBUSINESSDAYADJUSTMENT	= "FLOATACCRUALBUSINESSDAYADJUSTMENT";
        const std::string FLOAT_ACCRUALCALENDAR		            = "FLOATACCRUALCALENDAR";
        const std::string FLOAT_ACCRUALDAYCOUNT		            = "FLOATACCRUALDAYCOUNT";
        const std::string FLOAT_ACCRUALFREQUENCY		        = "FLOATACCRUALFREQUENCY";
        const std::string FLOAT_PAYMENTBUSINESSDAYADJUSTMENT	= "FLOATPAYMENTBUSINESSDAYADJUSTMENT";
        const std::string FLOAT_PAYMENTCALENDAR		            = "FLOATPAYMENTCALENDAR";
        const std::string FLOAT_PAYMENTFREQUENCY		        = "FLOATPAYMENTFREQUENCY";
        const std::string FLOAT_FIRSTSTUBDATE	                = "FLOATFIRSTSTUBDATE";
        const std::string FLOAT_LASTSTUBDATE	                = "FLOATLASTSTUBDATE";
        const std::string FLOAT_ROLLDAY			                = "FLOATROLLDAY";
        const std::string FLOAT_PAYMENTLAG		                = "FLOATPAYMENTLAG";
        const std::string FLOAT_FIXINGLAG		                = "FLOATFIXINGLAG";
        const std::string FLOAT_FIRSTFIXINGLAG		            = "FLOATFIRSTFIXINGLAG";
        const std::string FLOAT_STUBTYPE		                = "FLOATSTUBTYPE";
        const std::string FLOAT_FIRSTFIXING		                = "FLOATFIRSTFIXING";
        const std::string FLOAT_LASTFIXING		                = "FLOATLASTFIXING";
        const std::string FLOAT_SPREAD			                = "FLOATSPREAD";
        const std::string FLOAT_FIRSTSTUBCURVEINDEX				= "FLOATFIRSTSTUBCURVEINDEX";
        const std::string FLOAT_LASTSTUBCURVEINDEX				= "FLOATLASTSTUBCURVEINDEX";

        //the following keys will only be used for functions accepting either fixed or float leg
        const std::string RATE			                        = "RATE";
        const std::string FREQUENCY		                        = "FREQUENCY";
        const std::string DAYCOUNT		                        = "DAYCOUNT";
        const std::string BUSINESSDAYADJUSTMENT	                = "BUSINESSDAYADJUSTMENT";
        const std::string CALENDAR		                        = "CALENDAR";
        const std::string FIXINGBUSINESSDAYADJUSTMENT	        = "FIXINGBUSINESSDAYADJUSTMENT";
        const std::string FIXINGCALENDAR		                = "FIXINGCALENDAR";
        const std::string ACCRUALBUSINESSDAYADJUSTMENT	        = "ACCRUALBUSINESSDAYADJUSTMENT";
        const std::string ACCRUALCALENDAR		                = "ACCRUALCALENDAR";
        const std::string PAYMENTBUSINESSDAYADJUSTMENT	        = "PAYMENTBUSINESSDAYADJUSTMENT";
        const std::string PAYMENTCALENDAR		                = "PAYMENTCALENDAR";
        const std::string FIRSTSTUBDATE	                        = "FIRSTSTUBDATE";
        const std::string LASTSTUBDATE	                        = "LASTSTUBDATE";
        const std::string ROLLDAY			                    = "ROLLDAY";
        const std::string PAYMENTLAG		                    = "PAYMENTLAG";
        const std::string FIXINGLAG		                        = "FIXINGLAG";
        const std::string FIRSTFIXINGLAG		                = "FIRSTFIXINGLAG";
        const std::string STUBTYPE		                        = "STUBTYPE";
        const std::string FIRSTFIXING		                    = "FIRSTFIXING";
        const std::string LASTFIXING		                    = "LASTFIXING";
        const std::string ACCRUALFREQUENCY				        = "ACCRUALFREQUENCY";
        const std::string PAYMENTFREQUENCY				        = "PAYMENTFREQUENCY";
        const std::string FIXINGADVANCEORARREAR			        = "FIXINGADVANCEORARREAR";
        const std::string ACCRUALDAYCOUNT		                = "ACCRUALDAYCOUNT";
        const std::string FIRSTSTUBCURVEINDEX			        = "FIRSTSTUBCURVEINDEX";
        const std::string LASTSTUBCURVEINDEX			        = "LASTSTUBCURVEINDEX";
        const std::string FXFIXINGLAG		                    = "FXFIXINGLAG";
        const std::string FXFIXINGBUSINESSDAYADJUSTMENT	        = "FXFIXINGBUSINESSDAYADJUSTMENT";
        const std::string FXFIXINGCALENDAR		                = "FXFIXINGCALENDAR";

		//This is to allow user to change isFwdInter to true or false for STDCurve
		const std::string IS_FWD_INTER							= "ISFWDINTER";

		// Used to specify whether coupons are credit risky
		const std::string PAYMENT_TRIGGER						= "PAYMENTTRIGGER";

		// Specifies the calculation method for inflation swaps.
		const std::string INFLATION_RESET_TYPE					= "INFLATIONRESETTYPE";

		//FRA key
		const std::string STRIKE_RATE							= "STRIKERATE";
		const std::string FRA_STYLE								= "FRASTYLE";

    }

	namespace CASHFLOW_KEY
    {
        const std::string ACCRUAL_START					        = "ACCRUALSTART";
        const std::string ACCRUAL_END					        = "ACCRUALEND";
        const std::string FIXING_DATE					        = "FIXINGDATE";
        const std::string PAYMENT_DATE					        = "PAYMENTDATE";
    }

	namespace FEE_KEY
    {
        const std::string AMOUNT 					            = "AMOUNT";
    }

	namespace CDS_KEY
    {
		const std::string CDS_SPREAD                            = "CDSSPREAD";
        const std::string HAZARD_RATE                           = "HAZARDRATE";
        const std::string RECOVERY_RATE                         = "RECOVERYRATE";
	}

	namespace CMS_KEY
	{
		const std::string CMS_GENERATOR_NAME1              = "CMSGENERATOR1";
		const std::string CMS_INDEX_MATURITY1              = "CMSINDEXMATURITY1";
		const std::string CMS_INDEX_MULTIPLIER1            = "CMSINDEXMULTIPLIER1";

		const std::string CMS_GENERATOR_NAME2              = "CMSGENERATOR2";
		const std::string CMS_INDEX_MATURITY2              = "CMSINDEXMATURITY2";
		const std::string CMS_INDEX_MULTIPLIER2            = "CMSINDEXMULTIPLIER2";
	}

	namespace TRS_KEY
	{
		const std::string BOND_NAME							= "BONDNAME";
		const std::string ASSET_PERFORMANCE					= "ASSETPERFORMANCE";
	}

    namespace ASSET_SWAP_KEY
    {
        const std::string IS_CLEAN_PRICE			            = "ISCLEANPRICE";
        const std::string ISSUE_DATE			                = "ISSUEDATE";
    }

    namespace MARKET_KEY
    {
        const std::string CURVE_COLLECTION		                = "CURVECOLLECTION";
        const std::string FORECAST_CURVE		                = "FORECASTCURVE";
        const std::string DISCOUNT_CURVE		                = "DISCOUNTCURVE";
        const std::string MARKET_TYPE_BASISSWAP	                = "BASISSWAP";

        const std::string CURVE_TYPE_SWAP		                = "SWAP";
        const std::string CURVE_TYPE_OIS			            = "OIS";
		const std::string CURVE_TYPE_ARR						= "ARR";
        const std::string CURVE_TYPE_BASIS		                = "BASIS";
		const std::string CURVE_TYPE_TENORBASIS		            = "TENORBASIS";
        const std::string CURVE_TYPE_XCCYBASIS	                = "XCCYBASIS";
		const std::string CURVE_TYPE_FWDFXCONST	                = "FWDFXCONST";
		const std::string CURVE_TYPE_FX	                		= "FX";
    }

    namespace TRADE_TYPE_KEY
    {
        const std::string TRADETYPE_VANILLA_IRS	                = "VANILLAIRS";
        const std::string TRADETYPE_OIS			                = "OIS";
    }

    namespace RISK_KEY
    {
        const std::string DELTA_LADDER			                = "DELTALADDER";
        const std::string DELTA_FLATSHIFT		                = "FLATSHIFT";
    }

    namespace PRICING_PARAMS
    {
        const std::string INTERPOLATION		                    = "INTERPOLATION";
        const std::string COMPOUND_METHOD	                    = "COMPOUNDMETHOD";
    }

    namespace SWAP_STUB
    {
        const std::string STUB_TOLERANCE	                    = "STUBTOLERANCE";
        const std::string USE_CURVE_INDEX	                    = "USECURVEINDEX";
    }

    namespace SWAP_EXPRESSION_KEY
    {
        const std::string PAY_RECEIVE1                          = "PAYRECEIVE1";
        const std::string PAY_RECEIVE2                          = "PAYRECEIVE2";
        const std::string CURRENCY1                             = "CURRENCY1";
        const std::string CURRENCY2                             = "CURRENCY2";
        const std::string NOTIONAL1                             = "NOTIONAL1";
        const std::string NOTIONAL2                             = "NOTIONAL2";
        const std::string RATE_OR_SPREAD1                       = "RATEORSPREAD1";
        const std::string RATE_OR_SPREAD2                       = "RATEORSPREAD2";
        const std::string FIRSTFIXING1                          = "FIRSTFIXING1";
        const std::string LASTFIXING1                           = "LASTFIXING1";
        const std::string FIRSTFIXING2                          = "FIRSTFIXING2";
        const std::string LASTFIXING2                           = "LASTFIXING2";
        const std::string IS_FWD_INTER1                         = "ISFWDINTER1";
        const std::string IS_FWD_INTER2                         = "ISFWDINTER2";
    }

    namespace BOND_KEY
    {
        const std::string AQO_BOND_DESCRIPTION_LVB        = "BOND_DESCRIPTION_PARAMETERS";  // AQO SERIALIZATION LABEL FOR BOND DESCRIPTION LVB
        const std::string AQO_BOND_SCHEDULE_LVB           = "BOND_SCHEDULE_PARAMETERS";     // AQO SERIALIZATION LABEL FOR BOND SCHEDULE LVB


        const std::string BOND_DESCRIPTION                = "BONDDESCRIPTION";
        const std::string BOND_TYPE                       = "BONDTYPE";
        const std::string BOND_QUOTE_CONVENTION           = "BONDQUOTECONVENTION";
        const std::string CALCULATION_TYPE                = "CALCULATIONTYPE";
        const std::string CURRENCY                        = "CURRENCY";
        const std::string ISIN                            = "ISIN";
        const std::string YIELD_TYPE                      = "YIELDTYPE";
        const std::string IS_CLEAN_PRICE                  = "ISCLEANPRICE";
        const std::string COUPON                          = "COUPON";
        const std::string FACE_VALUE                      = "FACEVALUE";
        const std::string ISSUE_DATE                      = "ISSUEDATE";
        const std::string ACCRUAL_START_DATE              = "ACCRUALSTARTDATE";
        const std::string FIRST_COUPON_DATE               = "FIRSTCOUPONDATE";
        const std::string LAST_COUPON_DATE                = "LASTCOUPONDATE";
        const std::string ISSUE_PRICE                     = "ISSUEPRICE";
        const std::string TAX_RATE                        = "TAXRATE";
        const std::string YIELD_FREQUENCY                 = "YIELDFREQUENCY";
		const std::string EX_DIVIDEND_TENOR				  = "EXDIVIDENDTENOR";
		const std::string EX_DIVIDEND_BUSINESSDAYADJUSTMENT = "EXDIVIDENDBUSINESSDAYADJUSTMENT";
		const std::string QUOTED_MARGIN						= "QUOTEDMARGIN"; // For Floating Bond

	}

	namespace CURVEGENERATOR_CURVEPROPERTIES_KEY
	{
		const std::string CURVE_TYPE                      = "CURVETYPE";
		const std::string FREQUENCY                       = "FREQUENCY";
		const std::string CURVE_INDEX_FREQUENCY           = "CURVEINDEXFREQUENCY";
		const std::string STATIC_DATA_TABLE               = "STATICDATATABLE";
		const std::string INDEX_NAME                      = "INDEXNAME";
		const std::string CURRENCY                        = "CURRENCY";
		const std::string INTERPOLATION                   = "INTERPOLATION";
		const std::string YIELDGEN_INTERPOLATION          = "YIELDGEN.INTERPOLATION";
		const std::string BASIS_INTERPOLATION             = "BASIS.INTERPOLATION";
		const std::string FUTURE_INTERPOLATION            = "FUTURE.INTERPOLATION";
		const std::string FRA_INTERPOLATION               = "FRA.INTERPOLATION";
		const std::string DAYCOUNT                        = "DAYCOUNT";
		const std::string IS_FRA_USE                      = "ISFRAUSE";
		const std::string IS_FUTURE_USE                   = "ISFUTUREUSE";
		const std::string IS_FWD_BASIS                    = "ISFWDBASIS";
		const std::string IS_FWD_FX                       = "ISFWDFX";
		const std::string IS_RENOTIONAL_ADJUST            = "ISRENOTIONALADJUST";
		const std::string IS_XCCY_MARKED_TO_MARKET        = "ISXCCYMARKEDTOMARKET";
		const std::string DF_CURVE_NAME                   = "DFCURVENAME";
		const std::string GENERATE_FORWARDS_FROM_SWAPS_ONLY = "GENERATEFORWARDSFROMSWAPSONLY";
        const std::string FIXING_SOURCE                   = "FIXINGSOURCE";
		const std::string INTERPOLATION_JOIN_DATE		  = "INTERPOLATIONJOINDATE";
		const std::string ALWAYS_CALC_JOIN_DATE           = "ALWAYSCALCJOINDATE";
        const std::string FAST_REBUILD                    = "FASTREBUILD";
        const std::string IS_SWAP_TENOR_ADJUST            = "ISSWAPTENORADJUST";
		const std::string USE_TENOR_BASIS				  = "USETENORBASIS";	// Alias for IS_SWAP_TENOR_ADJUST
        const std::string TENOR_SWAP_NAME                 = "TENORSWAPNAME";
        const std::string JACOBIAN_BUILD_FREQUENCY        = "JACOBIANBUILDFREQUENCY";
        const std::string JACOBIAN_SHIFT_TYPE             = "JACOBIANSHIFTTYPE";
        const std::string JACOBIAN_RISK_TYPE              = "JACOBIANRISKTYPE";
		const std::string JACOBIAN_GRADIENT_SHIFT_SIZE	  = "JACOBIANGRADIENTSHIFTSIZE";
		const std::string JACOBIAN_MARKET_DATA_SHIFT_SIZE = "JACOBIANMARKETDATASHIFTSIZE";
	}

	namespace CURVEGENERATOR_OIS_KEY
	{
		const std::string CALENDAR                        = "CALENDAR";
		const std::string DAYCOUNT                        = "DAYCOUNT";
		const std::string FREQUENCY                       = "FREQUENCY";
		const std::string RESET_LAG                       = "RESETLAG";
		const std::string SLIDING_RULE                    = "SLIDINGRULE";
		const std::string GENERATE_METHOD                 = "GENERATEMETHOD";
		const std::string FIRST_RATE                      = "FIRSTRATE";
		const std::string SHORT_TERM_CONVENTION           = "SHORTTERMCONVENTION";
		const std::string EPSILON                         = "EPSILON";
		const std::string MAX_LOOP                        = "MAXLOOP";
		const std::string SMOOTH_SHORT_END                = "SMOOTHSHORTEND";
		const std::string COMPOUNDING_METHOD              = "COMPOUNDINGMETHOD";
		const std::string LONG_TERM_CONVENTION            = "LONGTERMCONVENTION";
		const std::string LONG_TERM                       = "LONGTERM";
		const std::string LONG_TERM_GENERATE_METHOD       = "LONGTERM.GENERATEMETHOD";
		const std::string LONG_TERM_GENERATE_METHOD_ARROIS = "LONGTERM.GENERATEMETHOD.ARROIS";
        const std::string MEAN_REVERSION                  = "MEANREVERSION"; // RFR FUTURES
		const std::string IS_EOM_ROLL                     = "ISEOMROLL";
		const std::string EOM_DAY                         = "EOMDAY";
	}

	namespace CURVEGENERATOR_LIBOROISBASIS_KEY
	{
		const std::string CALENDAR                        = "CALENDAR";
		const std::string DAYCOUNT                        = "DAYCOUNT";
		const std::string FREQUENCY                       = "FREQUENCY";
		const std::string SLIDING_RULE                    = "SLIDINGRULE";
		const std::string CALENDAR_ARROIS				  = "CALENDAR.ARROIS";
		const std::string DAYCOUNT_ARROIS                 = "DAYCOUNT.ARROIS";
		const std::string FREQUENCY_ARROIS                = "FREQUENCY.ARROIS";
		const std::string SLIDING_RULE_ARROIS             = "SLIDINGRULE.ARROIS";
	}

	namespace CURVEGENERATOR_SWAPS_KEY
	{
		const std::string CALENDAR                        = "CALENDAR";
		const std::string DAYCOUNT                        = "DAYCOUNT";
        const std::string DAYCOUNT_FIX                    = "DAYCOUNTFIX";
		const std::string DAYCOUNT_FLOAT                  = "DAYCOUNTFLOAT";
		const std::string FREQUENCY                       = "FREQUENCY";
        const std::string FREQUENCY_FLOAT                 = "FREQUENCYFLOAT";
        const std::string FREQUENCY_FLOAT_1Y              = "FREQUENCYFLOAT.1Y";
        const std::string FREQUENCY_FLOAT_2Y              = "FREQUENCYFLOAT.2Y";
        const std::string FREQUENCY_FLOAT_3Y              = "FREQUENCYFLOAT.3Y";
        const std::string FREQUENCY_FLOAT_4Y              = "FREQUENCYFLOAT.4Y";
        const std::string FREQUENCY_FLOAT_5Y              = "FREQUENCYFLOAT.5Y";
		const std::string FREQUENCY_FIX                   = "FREQUENCYFIX";
		const std::string FREQUENCY_FIX_1Y                = "FREQUENCYFIX.1Y";
        const std::string FREQUENCY_FIX_2Y                = "FREQUENCYFIX.2Y";
        const std::string FREQUENCY_FIX_3Y                = "FREQUENCYFIX.3Y";
        const std::string FREQUENCY_FIX_4Y                = "FREQUENCYFIX.4Y";
        const std::string FREQUENCY_FIX_5Y                = "FREQUENCYFIX.5Y";
		const std::string INTERPOLATION                   = "INTERPOLATION";
		const std::string RESET_LAG                       = "RESETLAG";
		const std::string SLIDING_RULE                    = "SLIDINGRULE";
		const std::string IS_NEWTON_RAPHSON               = "ISNEWTONRAPHSON";
		const std::string IS_SIMULTANEOUS_EQ              = "ISSIMULTANEOUSEQ";
		const std::string EPSILON                         = "EPSILON";
		const std::string MAX_LOOP                        = "MAXLOOP";
		const std::string IS_EOM_ROLL                     = "ISEOMROLL";
		const std::string EOM_DAY                         = "EOMDAY";
        const std::string BASE_FREQUENCY_FLOAT            = "BASEFREQUENCYFLOAT";

		const std::string CALENDAR_ARROIS				  = "CALENDAR.ARROIS";
		const std::string SLIDING_RULE_ARROIS             = "SLIDINGRULE.ARROIS";
		const std::string DAYCOUNT_FIX_ARROIS             = "DAYCOUNTFIX.ARROIS";
		const std::string FREQUENCY_FIX_ARROIS            = "FREQUENCYFIX.ARROIS";

	}
		
	namespace CURVEGENERATOR_MONEYMARKET_KEY
	{
		const std::string CALENDAR                        = "CALENDAR";
		const std::string DAYCOUNT                        = "DAYCOUNT";
		const std::string SLIDING_RULE                    = "SLIDINGRULE";
	}

	namespace CURVEGENERATOR_LIBORFIXINGS_KEY
	{
		const std::string CALENDAR                        = "CALENDAR";
		const std::string DAYCOUNT                        = "DAYCOUNT";
		const std::string FREQUENCY                       = "FREQUENCY";
		const std::string RESET_LAG                       = "RESETLAG";
		const std::string SLIDING_RULE                    = "SLIDINGRULE";
		const std::string IS_EOM_ROLL                     = "ISEOMROLL";
		const std::string EOM_DAY                         = "EOMDAY";
	}

	namespace CURVEGENERATOR_FRAS_KEY
	{
		const std::string CALENDAR                        = "CALENDAR";
		const std::string DAYCOUNT                        = "DAYCOUNT";
		const std::string RESET_LAG                       = "RESETLAG";
		const std::string SLIDING_RULE                    = "SLIDINGRULE";
        const std::string IS_EOM_ROLL                     = "ISEOMROLL";
		const std::string APPLY_TENSION                   = "APPLYTENSION";
		const std::string TENSION_GAP                     = "TENSIONGAP";
		const std::string SMOOTH_SHORT_END                = "SMOOTHSHORTEND";
		const std::string SERIAL_CALC_TYPE				  = "SERIALCALCTYPE";
		const std::string INSTRUMENT_TYPE				  = "INSTRUMENTYPE"; // Alias for SERIAL_CALC_TYPE
	}

	namespace CURVEGENERATOR_FUTURES_KEY
	{
		const std::string CALENDAR                        = "CALENDAR";
		const std::string DAYCOUNT                        = "DAYCOUNT";
		const std::string RESET_LAG                       = "RESETLAG";
		const std::string SLIDING_RULE                    = "SLIDINGRULE";
		const std::string MEAN_REVERSION                  = "MEANREVERSION";
		const std::string USE_CONVEX_ADJUSTMENT           = "USECONVEXADJUSTMENT"; // Legacy Parameter meaning ConvexityQuotedAsPrice
		const std::string CONVEXITY_QUOTE_TYPE			  = "CONVEXITYQUOTETYPE"; // Alias for USECONVEXADJUSTMENT
		const std::string APPLY_TENSION                   = "APPLYTENSION";
		const std::string TENSION_GAP                     = "TENSIONGAP";
		const std::string SMOOTH_SHORT_END                = "SMOOTHSHORTEND";
		const std::string SERIAL_CALC_TYPE				  = "SERIALCALCTYPE";
		const std::string INSTRUMENT_TYPE				  = "INSTRUMENTTYPE"; // Alias for SERIAL_CALC_TYPE
		const std::string INTERPOLATION_JOIN_DATE         = "INTERPOLATIONJOINDATE";
	}

	namespace CURVEGENERATOR_BASISSWAPS_KEY
	{
		const std::string MARKET_NAME                     = "MARKETNAME";
        const std::string ADJUST_VALUE_INTERPOLATION      = "ADJUSTVALUE.INTERPOLATION"; // For Xccy Basis

		const std::string IS_LEG1_SPREAD                  = "ISLEG1SPREAD";
		const std::string LEG1_CASHLET_CALENDAR           = "LEG1CASHLET.CALENDAR";
		const std::string LEG1_CASHLET_DAYCOUNT           = "LEG1CASHLET.DAYCOUNT";
		const std::string LEG1_CASHLET_FREQUENCY          = "LEG1CASHLET.FREQUENCY";
		const std::string LEG1_CASHLET_FREQUENCY_COMPOUND = "LEG1CASHLET.FREQUENCYCOMPOUND";
		const std::string LEG1_CASHLET_SLIDING_RULE       = "LEG1CASHLET.SLIDINGRULE";
		const std::string LEG1_CASHLET_SPOT_LAG           = "LEG1CASHLET.SPOTLAG";
		const std::string LEG1_FORECAST                   = "LEG1FORECAST";
		const std::string LEG1_DISCOUNT                   = "LEG1DISCOUNT";
		const std::string LEG1_INDEX_ACCESSARY            = "LEG1INDEX.ACCESSARY"; // Yes, this is a typo we have to support
		const std::string LEG1_INDEX_DAYCOUNT             = "LEG1INDEX.DAYCOUNT";
		const std::string LEG1_INDEX_FIXING_CALENDAR      = "LEG1INDEX.FIXING.CALENDAR";
		const std::string LEG1_INDEX_FREQUENCY            = "LEG1INDEX.FREQUENCY";
		const std::string LEG1_INDEX_PAYMENT_CALENDAR     = "LEG1INDEX.PAYMENT.CALENDAR";
		const std::string LEG1_INDEX_RESET_LAG            = "LEG1INDEX.RESETLAG";
		const std::string LEG1_INDEX_SLIDING_RULE         = "LEG1INDEX.SLIDINGRULE";

		const std::string LEG2_CASHLET_CALENDAR           = "LEG2CASHLET.CALENDAR";
		const std::string LEG2_CASHLET_DAYCOUNT           = "LEG2CASHLET.DAYCOUNT";
		const std::string LEG2_CASHLET_FREQUENCY          = "LEG2CASHLET.FREQUENCY";
		const std::string LEG2_CASHLET_FREQUENCY_COMPOUND = "LEG2CASHLET.FREQUENCYCOMPOUND";
		const std::string LEG2_CASHLET_SLIDING_RULE       = "LEG2CASHLET.SLIDINGRULE";
		const std::string LEG2_CASHLET_SPOT_LAG           = "LEG2CASHLET.SPOTLAG";
		const std::string LEG2_FORECAST                   = "LEG2FORECAST";
		const std::string LEG2_DISCOUNT                   = "LEG2DISCOUNT";
		const std::string LEG2_INDEX_ACCESSARY            = "LEG2INDEX.ACCESSARY";
		const std::string LEG2_INDEX_DAYCOUNT             = "LEG2INDEX.DAYCOUNT";
		const std::string LEG2_INDEX_FIXING_CALENDAR      = "LEG2INDEX.FIXING.CALENDAR";
		const std::string LEG2_INDEX_FREQUENCY            = "LEG2INDEX.FREQUENCY";
		const std::string LEG2_INDEX_PAYMENT_CALENDAR     = "LEG2INDEX.PAYMENT.CALENDAR";
		const std::string LEG2_INDEX_RESET_LAG            = "LEG2INDEX.RESETLAG";
		const std::string LEG2_INDEX_SLIDING_RULE         = "LEG2INDEX.SLIDINGRULE";

		const std::string TARGET                          = "TARGET";
		const std::string IS_SIMULTANEOUS_EQ              = "ISSIMULTANEOUSEQ";
		const std::string FWD_INTERPOLATION               = "FWDINTERPOLATION";
		const std::string IS_SAME_GRID_INDEX              = "ISSAMEGRIDINDEX";
		const std::string SPOT_RATE_TERM                  = "SPOTRATETERM";
		const std::string IS_YIELD_SPREAD_CALC            = "ISYIELDSPREADCALC";
		const std::string EPSILON                         = "EPSILON";
		const std::string MAX_LOOP                        = "MAXLOOP";
		const std::string IS_FWD_INTER                    = "ISFWDINTER";
		const std::string IS_EOM_ROLL                     = "ISEOMROLL";
		const std::string EOM_DAY                         = "EOMDAY";
	}

	namespace CURVEGENERATOR_FXFWDS_KEY
	{
		const std::string CALENDAR                        = "CALENDAR";
		const std::string RESET_LAG                       = "RESETLAG";
		const std::string SLIDING_RULE                    = "SLIDINGRULE";
		const std::string IS_PRICE_CCY                    = "ISPRICECCY";
		const std::string IS_DOMESTIC_CURRENCY            = "ISDOMESTICCURRENCY";
		const std::string IS_RATIO                        = "ISRATIO";
		const std::string IS_FX_OUTRIGHT				  = "ISFXOUTRIGHT";
		const std::string IS_EOM_ROLL                     = "ISEOMROLL";
		const std::string EOM_DAY                         = "EOMDAY";
		const std::string DENOMINATOR                     = "DENOMINATOR";
		const std::string PIPSIZE						  = "PIPSIZE";
		const std::string FREQUENCY                       = "FREQUENCY";
		const std::string FIXEDRATEXCCYSTARTTENOR		  = "FIXEDRATEXCCYSTARTTENOR";
	}

	namespace CURVEGENERATOR_FWDFXCONST_KEY
	{
		const std::string LEG1_FORECAST                   = "LEG1FORECAST";
		const std::string LEG1_DISCOUNT                   = "LEG1DISCOUNT";
		const std::string LEG2_FORECAST                   = "LEG2FORECAST";
		const std::string LEG2_DISCOUNT                   = "LEG2DISCOUNT";
		const std::string TARGET                          = "TARGET";
	}

	namespace GENERATOR_COMPONENTS
	{
		const std::string KEY_CURVEPROPERTIES			  = "CURVEPROPERTIES";
		const std::string KEY_MARKETDATAPROPERTIES		  = "MARKETDATAPROPERTIES";
		const std::string KEY_OIS						  = "OIS";
		const std::string KEY_OISFIXINGS				  = "OISFIXINGS";
		const std::string KEY_LIBOROISBASISSPREADS		  = "LIBOROISBASISSPREADS";
		const std::string KEY_LIBOROISBASIS				  = "LIBOROISBASIS";
		const std::string KEY_LIBORFIXINGS				  = "LIBORFIXINGS";
		const std::string KEY_SWAPS						  = "SWAPS";
		const std::string KEY_MONEYMARKET				  = "MONEYMARKET";
		const std::string KEY_FRAS						  = "FRAS";
		const std::string KEY_FUTURES					  = "FUTURES";
		const std::string KEY_BASISSWAPS				  = "BASISSWAPS";
		const std::string KEY_XCCYSWAPS					  = "XCCYSWAPS";
		const std::string KEY_FXFWDS					  = "FXFWDS";
		const std::string KEY_FXSPOTS					  = "FXSPOTS";
		const std::string KEY_FWDFXCONST				  = "FWDFXCONST";
		const std::string KEY_FORWARDADJUSTMENTS		  = "FORWARDADJUSTMENTS";
		
	}

	// These are the keys which are permitted in the CREDIT_MODEL LVB
	namespace CREDITMODEL_MODEL_PROPERTIES_KEY
	{
		const std::string ASOF_DATE						= "ASOFDATE";
		const std::string SPOT_LAG						= "SPOTLAG";
		const std::string SPOT_BUSINESSDAY_ADJUSTMENT	= "SPOTBUSINESSDAYADJUSTMENT";
		const std::string SPOT_CALENDAR					= "SPOTCALENDAR";
		const std::string ACCRUAL_START_DATE			= "ACCRUALSTARTDATE";
		const std::string IMM_REFERENCE_DATE			= "IMMREFERENCEDATE";
		const std::string CURRENCY						= "CURRENCY";
		const std::string RECOVERY_RATE					= "RECOVERYRATE";
		const std::string INCLUDE_ACCRUED_INTEREST		= "INCLUDEACCRUEDINTEREST";
		const std::string SWAP_GENERATOR				= "SWAPGENERATOR";
		const std::string CREDIT_INDEX					= "CREDITINDEX";
		const std::string INTERPOLATION					= "INTERPOLATION";
		const std::string EXTRAPOLATION					= "EXTRAPOLATION";
		const std::string CDS_CURVE_COLLECTION			= "CDSCURVECOLLECTION";
		const std::string BOND_CURVE_COLLECTION			= "BONDCURVECOLLECTION";
		const std::string BOND_DISCOUNT_CURVE			= "BONDDISCOUNTCURVE";
		const std::string BOND_HAS_RISKY_ACCRUED_INTEREST = "BONDHASRISKYACCRUEDINTEREST";
	}

	// These are the keys which are permitted in the CREDIT_BASKET_MODEL LVB
	namespace CREDITBASKETMODEL_MODEL_PROPERTIES_KEY
	{
		const std::string ASOF_DATE						= "ASOFDATE";
		const std::string BASKET_TYPE					= "BASKETTYPE";		// Homogeneous or Inhomogeneous loss
		const std::string NTH_TO_DEFAULT				= "NTHTODEFAULT";	// Enter 1 if First to default, etc
	}

	// These are the keys which are permitted in the BOND_STATICDATA LVB
	namespace BONDGENERATOR_STATICDATA_KEY
	{
		const std::string BOND_TYPE                       = "BONDTYPE";
		const std::string CURRENCY                        = "CURRENCY";
		const std::string YIELD_TYPE                      = "YIELDTYPE";
		const std::string IS_CLEAN_PRICE                  = "ISCLEANPRICE";
		const std::string BOND_QUOTE_CONVENTION           = "BONDQUOTECONVENTION";
	}

	// These are the keys which are permitted in the BOND_SCHEDULE LVB
	namespace BONDGENERATOR_SCHEDULE_KEY
	{
		const std::string PAY_RECEIVE                     = "PAYRECEIVE";
		const std::string NOTIONAL_EXCHANGE               = "NOTIONALEXCHANGE";
		const std::string FACE_VALUE                      = "FACEVALUE";
		const std::string FREQUENCY                       = "FREQUENCY";
		const std::string ACCRUAL_BUSINESSDAY_ADJUSTMENT  = "ACCRUALBUSINESSDAYADJUSTMENT";
		const std::string ACCRUAL_CALENDAR                = "ACCRUALCALENDAR";
		const std::string PAYMENT_BUSINESSDAY_ADJUSTMENT  = "PAYMENTBUSINESSDAYADJUSTMENT";
		const std::string PAYMENT_CALENDAR                = "PAYMENTCALENDAR";
		const std::string PAYMENT_LAG                     = "PAYMENTLAG";
		const std::string DAYCOUNT                        = "DAYCOUNT";
		const std::string CALCULATION_TYPE                = "CALCULATIONTYPE";
        const std::string YIELD_FREQUENCY                 = "YIELDFREQUENCY";
        const std::string TAX_RATE                        = "TAXRATE";
	}

	// The BondExpression LVB contains keys which are specific to the concrete bond.
	// These keys are used to further customize the bond, further to the settings provided by the BondGenerator.
	namespace BONDGENERATOR_EXPRESSION_KEY
	{
		const std::string BOND_DESCRIPTION                = "BONDDESCRIPTION";
		const std::string ISIN                            = "ISIN";
		const std::string ISSUE_DATE                      = "ISSUEDATE";
		const std::string FIRST_COUPON_DATE               = "FIRSTCOUPONDATE";
		const std::string LAST_COUPON_DATE                = "LASTCOUPONDATE";
		const std::string MATURITY_DATE                   = "MATURITYDATE";
		const std::string COUPON                          = "COUPON";
		const std::string ROLLDAY                         = "ROLLDAY";
		const std::string ACCRUAL_START_DATE              = "ACCRUALSTARTDATE";
        const std::string ISSUE_PRICE                     = "ISSUEPRICE";
        const std::string FACE_VALUE                      = "FACEVALUE";
        const std::string FREQUENCY                       = "FREQUENCY";
        const std::string DAYCOUNT                        = "DAYCOUNT";
        const std::string YIELD_FREQUENCY                 = "YIELDFREQUENCY";
        const std::string TAX_RATE                        = "TAXRATE";
		const std::string QUOTED_MARGIN                   = "QUOTEDMARGIN";
	}

	namespace BONDGENERATOR_COMPONENTS
	{
		const std::string KEY_BOND_STATICDATA      = "BOND_STATICDATA";
		const std::string KEY_BOND_SCHEDULE        = "BOND_SCHEDULE";
	}

	// These are the keys which are permitted in the BONDCURVE PROPERTIES block
	namespace BONDCURVE_PROPERTIES_KEY
	{
		const std::string SETTLEMENT_DATE				= "SETTLEMENTDATE";
		const std::string YIElD_CALCULATION_TYPE		= "YIELDCALCULATIONTYPE";
		const std::string YIELD_QUOTE_IN_PERCENT		= "YIELDQUOTEINPERCENT";
		const std::string INTERPOLATION					= "INTERPOLATION";
		const std::string EXTRAPOLATION					= "EXTRAPOLATION";
	}

	// These are the keys which are permitted in the BONDSPREADCURVE PROPERTIES block
	namespace BONDSPREADCURVE_PROPERTIES_KEY
	{
		const std::string SPREAD						= "SPREAD";
		const std::string BENCHMARK_BOND_CURVE			= "BENCHMARKBONDCURVE";
	}

	namespace INTERPOLATION_KEYS
	{
		const std::string PIECEWISE_CONSTANT			= "PIECEWISECONSTANT";
		const std::string FLAT							= "FLAT";
	}

	namespace OPTION_KEYS
	{
        const std::string TRADE_TYPE                    = "TRADETYPE";
		const std::string AS_OF_DATE                    = "ASOFDATE";
		const std::string OPTION_STYLE                  = "OPTIONSTYLE";
        const std::string LONG_SHORT                    = "LONGSHORT";
        const std::string CALL_PUT                      = "CALLPUT";
		const std::string STRIKE                        = "STRIKE";
        const std::string EFFECTIVE_DATE                = "EFFECTIVEDATE";
		const std::string EXPIRY                        = "EXPIRY";
		const std::string DAYCOUNT                      = "DAYCOUNT";
		const std::string SPOT_LAG                      = "SPOTLAG";
		const std::string SPOT_BUSINESSDAYADJUSTMENT    = "SPOTBUSINESSDAYADJUSTMENT";
		const std::string SPOT_CALENDAR                 = "SPOTCALENDAR";
        const std::string DELIVERY_DATE					= "DELIVERYDATE"; //Underlying delivery date
		const std::string BOND_NAME						= "BONDNAME"; // Underlying Bond name
		const std::string FUTURE_NAME					= "FUTURENAME"; // Underlying Future name
	}

    namespace VOLATILITY_KEYS
    {
        const std::string VOL_DATA_SOURCE               = "VOLDATASOURCE";
		const std::string VOL_TYPE                      = "VOLTYPE";
		const std::string VOLATILITY                    = "VOLATILITY";
		const std::string SHIFT_SIZE                    = "SHIFTSIZE";
    }

	namespace SWAPTION_KEYS
	{
        const std::string TRADE_TYPE                    = "TRADETYPE";
        const std::string OPTION_STYLE                  = "OPTIONSTYLE";
		const std::string LONG_SHORT                    = "LONGSHORT";
        const std::string PAYER_RECEIVER                = "PAYERRECEIVER";
		const std::string TENOR_DESCRIPTION             = "TENORDESCRIPTION";
		const std::string EFFECTIVE_DATE                = "EFFECTIVEDATE";
        const std::string EXPIRY                        = "EXPIRY";
        const std::string SETTLEMENT_TYPE               = "SETTLEMENTTYPE";
        const std::string CURRENCY                      = "CURRENCY";
        const std::string NOTIONAL                      = "NOTIONAL";
        const std::string LEVERAGE                      = "LEVERAGE";
        const std::string STRIKE                        = "STRIKE";
        const std::string SWAP_GENERATOR                = "SWAPGENERATOR";
		const std::string SWAP_START                    = "SWAPSTART";
		const std::string SWAP_END                      = "SWAPEND";
		const std::string NOTIFICATION_DAYS             = "NOTIFICATIONDAYS";
		const std::string DAYCOUNT                      = "DAYCOUNT";
        const std::string BUSINESSDAYADJUSTMENT         = "BUSINESSDAYADJUSTMENT";
        const std::string CALENDAR                      = "CALENDAR";
        const std::string FEE                           = "FEE";
        const std::string FEE_DATE                      = "FEEDATE";
        const std::string FEE_PAY_RECEIVE               = "FEEPAYRECEIVE";
	}

	namespace CREDIT_OPTION_KEYS
	{
		const std::string PAYER_RECEIVER				= "PAYERRECEIVER";
		const std::string STRIKE						= "STRIKE";
		const std::string EXPIRY						= "EXPIRY";
		const std::string CDS_START_DATE				= "CDSSTARTDATE";
		const std::string CDS_MATURITY_DATE				= "CDSMATURITYDATE";
		const std::string CDS_COUPON					= "CDSCOUPON";
		const std::string VOLATILITY					= "VOLATILITY";
		const std::string FORWARD_SPREAD				= "FORWARDSPREAD";
		const std::string OPTION_VALUE					= "OPTIONVALUE";
		const std::string SCALE_STRIKE_BY_SURVIVAL_PROBABILITY = "SCALESTRIKEBYSURVIVALPROBABILITY";
		const std::string NUMBER_OF_UNDERLIERS			= "NUMBEROFUNDERLIERS";
		const std::string NUMBER_OF_REALIZED_DEFAULTS	= "NUMBEROFREALIZEDDEFAULTS";
		const std::string DEFAULT_SETTLEMENT_AMOUNT		= "DEFAULTSETTLEMENTAMOUNT";
	}

	namespace VALUATION_SETTING_KEYS
	{
		const std::string VALUATION_DATE                = "VALUATIONDATE";
		const std::string SETTLEMENT_DATE               = "SETTLEMENTDATE";
		const std::string CURVE_COLLECTION              = "CURVECOLLECTION";
        const std::string CURVE_OBJECT                  = "CURVEOBJECT";
        const std::string CREDIT_MODEL                  = "CREDITMODEL";
        const std::string FX_SPOT                       = "FXSPOT";
		const std::string FX_AS_OF_DATE_RATE            = "FXASOFDATERATE";
		const std::string VOLATILITY_MODEL              = "VOLATILITYMODEL";
		const std::string PRICING_MODEL					= "PRICINGMODEL";
		const std::string CONVEXITY_METHOD				= "CONVEXITYMETHOD";
		const std::string FLOAT_BOND_CURRENT_COUPON		= "FLOATBONDCURRENTCOUPON";		// The annualized coupon rate for the next floating bond coupon
		const std::string FLOAT_BOND_ASSUMED_RATE		= "FLOATBONDASSUMEDRATE";		// The average underlying index level for projected coupons
		const std::string FLOAT_BOND_INDEX_TO_NEXT_COUPON = "FLOATBONDINDEXTONEXTCOUPON";	// The index discount rate for the very next floating bond coupon
		const std::string FLOAT_BOND_QUOTED_MARGIN		= "FLOATBONDQUOTEDMARGIN";		// The floating bond spread over the underlying index
	}

	namespace MONTE_CARLO_PROPERTIES_KEYS
	{
		const std::string RANDOM_NUMBER_GENERATOR		= "RANDOMNUMBERGENERATOR";
		const std::string DISTRIBUTION					= "DISTRIBUTION";
		const std::string NUMBER_OF_PATHS				= "NUMBEROFPATHS";
		const std::string ANTITHETIC_SAMPLING			= "ANTITHETICSAMPLING";
		const std::string EVALUATE_IN_PARALLEL			= "EVALUATEINPARALLEL";
		const std::string NUMBER_OF_ASSETS				= "NUMBEROFASSETS";
	}

	namespace STRUCTURED_CREDIT_CSV_KEYS
	{
		const std::string EXPOSURE_AT_DEFAULT			= "EXPOSUREATDEFAULT";	// Total value the bank is exposed to when a loan defaults.
		const std::string RISK_WEIGHT                   = "RW";					// Represents the proportion of capital required to be set aside as a result of providing the loan
		const std::string KIRB							= "KIRB";				// Exposure weighted average capital charge calculated using Internal Ratings Based approach
		const std::string MATURITY						= "MATURITY";			// Maturity of the loan
		const std::string MATURITY_MONTH                = "MATURITY_MONTH";		// Maturity of the loan in months
		const std::string PEAD                          = "PEAD";				// Exposure at default
		const std::string LOAN_TYPE                     = "LOAN_TYPE";			// 0 = Amortizing, 1 = Bullet
		const std::string RATE                          = "RATE";				// Loan annual interest rate
		const std::string FREQ                          = "FREQ";				// Loan interest payment frequency
		const std::string PD_REGULATORY					= "PD_REGULATORY";		// Regulatory Probability of Default
		const std::string PDP                           = "PDP";				// Proportion Probability of Default
		const std::string LGD                           = "LGD";				// Loss Given Default
	}

	namespace STRUCTURED_CREDIT_KEYS
	{
		const std::string TRIGGER_NAME_PRO_RATA_SEQUENTIAL_VALE				= "PR_SEQ_VALE";
		const std::string TRIGGER_NAME_PRO_RATA_SEQUENTIAL_VALE_CUMULATIVE	= "PR_SEQ_VALE_CUM";
		const std::string TRIGGER_NAME_REINVEST								= "REINVEST";
		const std::string TRIGGER_NAME_REINVEST_VALE						= "REINVEST_VALE";
	}

	namespace TRANCHE_KEYS
	{
		const std::string SENIORITY_RANK	= "SENIORITYRANK";
		const std::string TRANCHE_SIZE		= "TRANCHESIZE";
		const std::string TRANCHE_PAY_TYPE	= "TRANCHEPAYTYPE";
		const std::string COUPON_TYPE		= "COUPONTYPE";
		const std::string COUPON_RATE		= "COUPONRATE";
		const std::string COUPON_FLOOR		= "COUPONFLOOR";
		const std::string COUPON_FREQUENCY	= "COUPONFREQUENCY";
		const std::string CURVE_COLLECTION	= "CURVECOLLECTION";
		const std::string RESET_CURVE		= "RESETCURVE";
		const std::string DISCOUNT_MARGIN	= "DISCOUNTMARGIN";
		const std::string FIXING_TABLE		= "FIXINGTABLE";
	}

	namespace SYNTHETIC_EXCESS_SPREAD_KEYS
	{
		const std::string PERIODS			= "PERIODS";
		const std::string RESET_FREQUENCY	= "RESETFREQUENCY";
		const std::string USE_OR_LOSE		= "USEORLOSE";
	}

	namespace  CASHFLOW_ENGINE_KEYS
	{
		const std::string KIRB							= "KIRB";
		const std::string ELGD							= "ELGD";
		const std::string REGULATORY_CAPITAL_MATURITY	= "REGULATORYCAPITALMATURITY";
		const std::string DIVERSITY_METRIC				= "DIVERSITYMETRIC";
		const std::string SUPERVISORY_TYPE				= "SUPERVISORYTYPE";
		const std::string POOL_TYPE						= "POOLTYPE";
		const std::string EFFECTIVE_DATE				= "EFFECTIVEDATE";
		const std::string MATURITY_DATE					= "MATURITYDATE";
		const std::string REINVESTMENT_END_DATE			= "REINVESTMENTENDDATE";
		const std::string REINVESTMENT_END_PERIOD		= "REINVESTMENTENDPERIOD";
	}

    namespace OBJECT_POOL_KEYS
    {
        const std::string TERMS                         = "Terms";              // Case Sensitive
        const std::string DISCOUNT_FACTORS              = "DiscountFactors";    // Case Sensitive
    }

    namespace CURVE_ENGINE_KEYS
    {
        const std::string OIS_CURVE_CONVENTIONS         = "OIS_CURVE_CONVENTIONS";
        const std::string SWAP_CURVE_CONVENTIONS        = "SWAP_CURVE_CONVENTIONS";
        const std::string TENOR_BASIS_CURVE_CONVENTIONS = "TENOR_BASIS_CURVE_CONVENTIONS";
        const std::string XCCY_BASIS_CURVE_CONVENTIONS  = "XCCY_BASIS_CURVE_CONVENTIONS";
        const std::string DAYCOUNT			            = "DAYCOUNT";
        const std::string DAYCOUNT_FLOAT	            = "DAYCOUNTFLOAT";
        const std::string LEG1_DAYCOUNT                 = "LEG1CASHLET.DAYCOUNT";
        const std::string LEG2_DAYCOUNT                 = "LEG2CASHLET.DAYCOUNT";
        const std::string TARGET	                    = "TARGET";
        const std::string LEG1_FORECAST_RATES           = "LEG1FORECAST";
        const std::string LEG1_DISCOUNT_FACTORS         = "LEG1DISCOUNT";
    }

	// These keys may appear in the FixingTable PameterLVB
	namespace FIXING_TABLE_KEYS
	{
		const std::string TABLE_TYPE					= "TABLETYPE";

		// RATE Fixing parameters
		const std::string CURRENCY						= "CURRENCY";
		const std::string TENOR							= "TENOR";

		// Additional parameters for INFLATION
		const std::string INFLATION_INDEX				= "INFLATIONINDEX";

		// FX parameters
		const std::string CURRENCY_PAIR					= "CURRENCYPAIR";
	}

	// These keys used to serialize / deserialise a fixing table object
	namespace FIXING_TABLE_SERIALIZATION_KEYS
	{
		const std::string FIXING_TABLE					= "FIXING_TABLE";
		const std::string FIXING_TABLE_PARAMETERS		= "FIXING_TABLE_PARAMETERS";
		const std::string FIXING_TABLE_PARAMETERLVB		= "FIXING_TABLE_PARAMETERLVB";
		const std::string PARAMETER_NAME				= "PARAMETER_NAME";
		const std::string PARAMETER_VALUE				= "PARAMETER_VALUE";
	}

	// These are the keys which are permitted in the INFLATION CURVE LVB
	namespace INFLATION_CURVE_PROPERTIES_KEY
	{
		const std::string ASOF_DATE						= "ASOFDATE";
		const std::string SPOT_LAG						= "SPOTLAG";
		const std::string SPOT_BUSINESSDAY_ADJUSTMENT	= "SPOTBUSINESSDAYADJUSTMENT";
		const std::string SPOT_CALENDAR					= "SPOTCALENDAR";
		const std::string CURRENCY						= "CURRENCY";
		const std::string SWAP_GENERATOR				= "SWAPGENERATOR";
		const std::string ANNUAL_INTERPOLATION			= "ANNUALINTERPOLATION";
		const std::string INFLATION_INDEX				= "INFLATIONINDEX";
		const std::string CURVE_COLLECTION				= "CURVECOLLECTION";
		const std::string FIXING_TABLE					= "FIXINGTABLE";
	}

	namespace SABR_MARKETDATA_PROPERTIES_KEY
	{
		const std::string MARKET_DATA_TYPE = "MARKETDATATYPE";
		const std::string STRIKE_ADJUST_AMOUNT = "STRIKEADJUSTAMOUNT";
		const std::string VALUE_TYPE = "VALUETYPE";
	}

	namespace SABR_MODEL_PROPERTIES_KEY
	{

		const std::string AS_OF_DATE = "ASOFDATE";
		const std::string CURVE_COLLECTION = "CURVECOLLECTION";
		const std::string SWAP_GENERATOR = "SWAPGENERATOR";
		const std::string CALENDAR = "CALENDAR";
		const std::string DAYCOUNT = "DAYCOUNT";
		const std::string BUSINESSDAY_ADJUSTMENT = "BUSINESSDAYADJUSTMENT";
		const std::string APPROX_METHOD = "APPROXMETHOD";
		const std::string CALIBRATION_METHOD = "CALIBRATIONMETHOD";
		const std::string TARGET = "TARGET";
		const std::string VOL_TYPE = "VOLTYPE";
		const std::string SHIFT = "SHIFT";
		const std::string SOLVER_EPSILON = "SOLVEREPSILON";
		const std::string ALPHA_FROM_ATM_VOL = "ALPHAFROMATMVOL";

		const std::string ALPHA_PARAM = "ALPHA";
		const std::string BETA_PARAM = "BETA";
		const std::string NU_PARAM = "NU";
		const std::string RHO_PARAM = "RHO";

		const std::string ATMF_SWAPRATES = "ATMFSWAPRATES";
		const std::string ANNUITY = "ANNUITY";

	}


}

