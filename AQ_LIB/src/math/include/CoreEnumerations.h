//
// @Description: This file as has all the essential enums for handling objects

#pragma once


#include <string>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <unordered_set>

#include "AQLString.h"

namespace etrading
{

	// This function takes a const char and returns a new std::string which is trimmed and upper case
	std::string trim_to_upper(const char* inputStr);

	// This function takes a std::string and returns a new std::string which is trimmed and upper case
	std::string trim_to_upper(const std::string & inputStr);

	// Template to convert enums to AQLString calling using the underlying toString (std::string) methods
    template<class T>
    AQLString toAQLString( const T enumValue )
    {
        return toString(enumValue).c_str();
    }

    // the reason for '_' is that these variable names are invalid in C++ without them
    enum CurveTenorEnum
    {
		NONE_CURVE_TENOR,
		CURVE_TENOR_1D,
        CURVE_TENOR_1M,
        CURVE_TENOR_3M,
        CURVE_TENOR_6M,
        CURVE_TENOR_12M
    };
     std::string toString( const CurveTenorEnum enumValue );
     CurveTenorEnum toCurveTenorEnum( const std::string& enumString );
     double toYearFraction( const CurveTenorEnum enumValue, const bool isLeapYear = false );
    
    // Additional Helper Methods
    std::string toFrequencyFromCurveTenor(const std::string& curveTenor );
    
    enum MarketQuoteTypeEnum
    {
        FRA_QUOTE,
        CENTRALBANK_SWAP,
        IRS_SWAP,
        IR_FUTURES,
        FXRATES
    };
     std::string toString( const MarketQuoteTypeEnum enumValue );

    enum SwapQuoteTypeEnum
    {
        OIS_SWAPTYPE,
        STD_SWAPTYPE,
        BASIS_SWAPTYPE
    };
	 std::string toString( const SwapQuoteTypeEnum enumValue );
	 SwapQuoteTypeEnum toSwapQuoteTypeEnum(const std::string& enumString); 

    enum CentralBankTypeEnum
    {
        BOE_CENTRAL_BANK,
        ECB_CENTRAL_BANK,
        FED_CENTRAL_BANK,
        BOJ_CENTRAL_BANK
    };
	 std::string toString( const CentralBankTypeEnum enumValue );
	 CentralBankTypeEnum toCentralBankTypeEnum(const std::string& enumString); 

    enum FRAPeriodEnum
    {
        _3M_FRAPERIOD,
        _6M_FRAPERIOD
    };
     CurveTenorEnum toCurveTenorEnum( const FRAPeriodEnum fraEnum );
	 FRAPeriodEnum toFRAPeriodEnum( const CurveTenorEnum curveTenorEnum  );


    // Extrapolation Type (we need the _EXTRAPOLATION thing to distinguish it from other enums)
    enum ExtrapolationTypeEnum
    {
        CONSTANT_EXTRAPOLATION,
        LINEAR_EXTRAPOLATION
    };
     std::string toString( const ExtrapolationTypeEnum enumValue );
     ExtrapolationTypeEnum toExtrapolationTypeEnum( const std::string& enumString );

    enum DayCountEnum
    {
        NONE_DAYCOUNT,
        ACT_360_DAYCOUNT,
        ACT_365_DAYCOUNT,
		N30_360_DAYCOUNT,			// 30/360
        E30_360_DAYCOUNT,			// 30E/360, 30/360 ISMA
        E30_360_ISDA_DAYCOUNT,		// 30E/360 ISDA, 30/360 German
        ACT_ACT_DAYCOUNT,			// same as ACT_365_ISDA
        ACT_365_FJ_DAYCOUNT,		// ACT/365 FIXED JAPAN, Other names: Actual/365 No Leap year, NL 365, NL/365
		ONE_DAYCOUNT,				// 1/1
		HALF_DAYCOUNT,				// 1/2
		QUARTER_DAYCOUNT			// 1/4
    };
     std::string toString( const DayCountEnum enumValue );
     DayCountEnum toDayCountEnum( const std::string& enumString );

    // CompoundingMethodEnum is usually used for OIS but it is possible to use this for bespoke swaps
    // e.g. an annually paying swap but compounding 3m; note that a non-OIS curve CAN have no compounding method applied to it
    // but that is not necessarily the case. If no compounding method is relevant to the curve, simply setting any method will do
    enum CompoundingMethodEnum
    {
        NONE_COMPOUNDING_METHOD,
        GEOMETRIC_COMPOUNDING_METHOD,
        ARITHMETIC_COMPOUNDING_METHOD,
        FLAT_COMPOUNDING_METHOD,
        SIMPLE_COMPOUNDING_METHOD
    };
     std::string toString( const CompoundingMethodEnum enumValue );
     CompoundingMethodEnum toCompoundingMethodEnum( const std::string& enumString );

	// for OIS this is simply ignored because we have to compound daily
    enum CompoundingFrequencyEnum
    {
		NONE_COMPOUNDING,
		ANNUAL_COMPOUNDING,
        SEMI_ANNUAL_COMPOUNDING,
        QUARTERLY_COMPOUNDING,
        MONTHLY_COMPOUNDING,
        LUNAR_COMPOUNDING,
        WEEKLY_COMPOUNDING,
        SIMPLE_COMPOUNDING,
        CONTINUOUS_COMPOUNDING
    };
     std::string toString( const CompoundingFrequencyEnum enumValue );
     CompoundingFrequencyEnum toCompoundingFrequencyEnum( const std::string& enumString );
     std::string toTermString( const CompoundingFrequencyEnum enumValue );
    
    enum StubTypeEnum
    {
        NONE_STUBTYPE,
        SHORT_START_STUBTYPE,
        LONG_START_STUBTYPE,
        SHORT_END_STUBTYPE,
        LONG_END_STUBTYPE
    };
     std::string toString( const StubTypeEnum enumValue );
     StubTypeEnum toStubTypeEnum( const std::string& enumString );

    enum CurveTypeEnum
    {
        NONE_CURVETYPE,
		OIS_CURVETYPE,
		ARR_CURVETYPE,
		SWAP_CURVETYPE,
        TENORBASIS_CURVETYPE,
        XCCYBASIS_CURVETYPE,
        FWDFXCONST_CURVETYPE,
	    FX_CURVETYPE,
		GLOBAL_CURVETYPE
    };
    std::string toString( const CurveTypeEnum enumValue );
    CurveTypeEnum toCurveTypeEnum( const std::string& enumString );

	enum CurveCalibrationTypeEnum
	{
        NONE_CURVE_CALIBRATION,
		SINGLE_CURVE_CALIBRATION,
		DUAL_CURVE_CALIBRATION,
		MULTI_CURVE_CALIBRATION // AKA GLOBAL_CURVE_CALIBRATION
    };
	std::string toString( const CurveCalibrationTypeEnum enumValue );
    CurveCalibrationTypeEnum toCurveCalibrationTypeEnum( const std::string& enumString );


    enum BusinessDayAdjustmentEnum
    {
        NO_CHANGE,
		NONE_BUSINESS_DAY_ADJ,
        FOLLOWING,
        MOD_FOLLOWING,
        PRECEDING,
        MOD_PRECEDING,
    };
     std::string toString( const BusinessDayAdjustmentEnum enumValue );
     BusinessDayAdjustmentEnum toBusinessDayAdjustmentEnum( const std::string& enumString );

    enum CachedObjectEnum
    {
        TABLE,
        MATRIX,
        CURVE_DEPRECATED,
        CURVE_BUILD_PROPERTIES,         // not really cached on its own, just for DataSchema in StaticStructureStore
	    SWAP_OBJECT,                    // SWAP is taken by #define SWAP "SWAP" (AQLTemplate...)
        SCHEDULE,         
	    LEG,
        BOND,
        SWAP_GENERATOR,
        FREE_OBJECT,
        EXAMPLE_STAND_ALONE,
        EXAMPLE_BASE,
        FIXING_TABLE,
        CURVE_DATA,
        FEE_SCHEDULE,                   // not really cached on its own, just for DataSchema
        LEG_STATIC_DATA,                // not really cached on its own, just for DataSchema
        LEG_GENERATOR,                  // not really cached on its own, just for DataSchema
        BESPOKE_SCHEDULE_PROPERTIES,    // not really cached on its own, just for DataSchema
		CURVE_GENERATOR,
		CURVE_MARKETDATA,
		CURVE,
		MULTICURVE,
		FX_CURVE,
		BOND_GENERATOR,
		OPTION,
		VOLATILITY,
		CREDIT_MODEL,
		CREDIT_BASKET_MODEL,
		BOND_CURVE,
		INFLATION_CURVE,
		SABR_MODEL,
		SABR_MARKETDATA
	};
     std::string toString( const CachedObjectEnum enumValue );
     CachedObjectEnum toCachedObjectEnum( const std::string& enumString );

	enum BondGeneratorEnum
	{
		NONE_BONDGENERATOR,
		BOND_STATICDATA,
		BOND_SCHEDULE
	};
	 std::string toString( const BondGeneratorEnum enumValue );
	 BondGeneratorEnum toBondGeneratorEnum( const std::string& enumString );

	enum CurveGeneratorEnum
	{
		NONE_CURVEGENERATOR,
		CURVE_PROPERTIES,
		MONEY_MARKET_CONVENTIONS,
		LIBOR_FIXING_CONVENTIONS,
		FRA_CONVENTIONS,
		FUTURES_CONVENTIONS,
		SWAP_CONVENTIONS,
		OIS_CONVENTIONS,
		LIBOR_OIS_BASIS_CONVENTIONS,
		BASIS_SWAP_CONVENTIONS,
		XCCY_BASIS_CONVENTIONS,
		FXFWD_CONVENTIONS,
		FWDFXCONST_CONVENTIONS
	};
	 std::string toString( const CurveGeneratorEnum enumValue );
	 CurveGeneratorEnum toCurveGeneratorEnum( const std::string& enumString );

	enum CurveMarketDataEnum
	{
		NONE_MARKETDATA,
		MARKETDATA_PROPERTIES,
		LIBOR_FIXING_TABLE,
		FRA_MARKETDATA,
		FUTURES_MARKETDATA,
		SWAP_MARKETDATA,
		OIS_MARKETDATA,
		OIS_FIXING_TABLE,
		LIBOR_OIS_BASISSPREAD_MARKETDATA,
		BASIS_SWAP_MARKETDATA,
		XCCY_SWAP_MARKETDATA,
		FXFWD_MARKETDATA,
		FXFWD_BIDASK_MARKETDATA,
        FXSPOT_MARKETDATA,
        FXSPOT_BIDASK_MARKETDATA,
		FORWARD_ADJUSTMENTS_MARKETDATA
	};
	 std::string toString( const CurveMarketDataEnum enumValue );
	 CurveMarketDataEnum toCurveMarketDataEnum( const std::string& enumString );

	enum CreditModelEnum
	{
		NONE_CREDITMODEL,
		MODEL_PROPERTIES,
		CDS_MARKETDATA,
		BOND_MARKETDATA,
		CREDIT_MODELS
	};
	 std::string toString( const CreditModelEnum enumValue );
	 CreditModelEnum toCreditModelEnum( const std::string& enumString );

	enum CreditBasketTypeEnum
	{
		NONE_BASKET_TYPE,
		HOMOGENEOUS_LOSS,
		INHOMOGENEOUS_LOSS
	};
	 std::string toString( const CreditBasketTypeEnum enumValue );
	 CreditBasketTypeEnum toCreditBasketTypeEnum( const std::string& enumString );

	 enum BondCurveEnum
	{
		BONDCURVE_NONE,
		BONDCURVE_PROPERTIES,
		BONDCURVE_MARKETDATA
	};
	 std::string toString( const BondCurveEnum enumValue );
	 BondCurveEnum toBondCurveEnum( const std::string& enumString );

	 // Distinguishes a BondCurve built directly from bond quotes from one built as a spread over a benchmark BondCurve.
	 // NOTE: not named BOND_CURVE - that identifier is already CachedObjectEnum::BOND_CURVE (the AQObj registration tag) above.
	 enum BondCurveTypeEnum
	{
		BONDCURVE_TYPE_OUTRIGHT,
		BONDCURVE_TYPE_SPREAD
	};
	 std::string toString( const BondCurveTypeEnum enumValue );
	 BondCurveTypeEnum toBondCurveTypeEnum( const std::string& enumString );

	 // Interpolation/extrapolation choices for a BondCurve's yield pillars (or, for a BOND_SPREAD_CURVE, its spread nodes)
	 // NOTE: not named FLAT/LINEAR - ConstantDeclarations.h (models) #defines a bare LINEAR macro, which would corrupt that token.
	 enum BondCurveInterpolationEnum
	{
		BONDCURVE_FLAT,
		BONDCURVE_LINEAR
	};
	 std::string toString( const BondCurveInterpolationEnum enumValue );
	 BondCurveInterpolationEnum toBondCurveInterpolationEnum( const std::string& enumString );

	 enum InflationCurveEnum
	 {
		 INFLATIONCURVE_NONE,
		 INFLATIONCURVE_PROPERTIES,
		 INFLATIONCURVE_CPI_FIRST_YEAR,
		 INFLATIONCURVE_ZC_INFLATIONSWAPS,
		 INFLATIONCURVE_SEASONALITY
	 };
	 std::string toString( const InflationCurveEnum enumValue );
	 InflationCurveEnum toInflationCurveEnum( const std::string& enumString );


	 enum InflationResetTypeEnum
	 {
		 INFLATION_RESET_TYPE_NONE,
		 INFLATION_RESET_TYPE_MONTHLY_INTERPOLATION,	// Payoff depends directly on the monthly Index Level, typically with a 2 or 3 month lag
		 INFLATION_RESET_TYPE_DAILY_INTERPOLATION		// Payoff depends on the reference number, obtained by interpolating the index level
	 };
	 std::string toString( const InflationResetTypeEnum enumValue );
	 InflationResetTypeEnum toInflationResetTypeEnum( const std::string& enumString );


    // VARIANT: this does not mean anything: it means it could be any of the previous enums (to allow for containers of mixed type) mixed
    enum ContainedTypeEnum
    {
        INTEGER_VALUE,
        DOUBLE_VALUE,
        DATE_VALUE,
        STRING_VALUE,
        BOOL_VALUE,
        VARIANT_VALUE,
        EMPTY_VALUE
    };
     std::string toString( const ContainedTypeEnum enumValue );
     ContainedTypeEnum toContainedTypeEnum( const std::string& enumString );

    enum InterpolationEnum
    {
        RIGHT_CONTINUOUS_INTERPOLATION,		// ( x[i-1], x[i]   ]
        LEFT_CONTINUOUS_INTERPOLATION,		// [ x[i],	 x[i+1] )
        STEP_INTERPOLATION,
        LINEAR_INTERPOLATION,
		LINEAR_WITH_FLAT_EXTRAPOLATION,
        MONOTONE_CONVEX_INTERPOLATION,
        SPLINE_INTERPOLATION,
		MONOTONESPLINE_INTERPOLATION,
		MONOTONEPARABOLIC_INTERPOLATION,
        NATURAL_SPLINE_INTERPOLATION,
        CLAMPED_SPLINE_INTERPOLATION,
        PARABOLIC_INTERPOLATION,
        LINEARSPLINE_INTERPOLATION,
		LINEARMONOTONESPLINE_INTERPOLATION,
		LINEARMONOTONEPARABOLIC_INTERPOLATION,
        CONSTRAINED_SPLINE_INTERPOLATION,
		NELSON_SIEGEL_INTERPOLATION,
		SVENSSON_INTERPOLATION,
		POLYNOMIAL_INTERPOLATION,
		LOG_LINEAR_INTERPOLATION
    };
     std::string toString( const InterpolationEnum enumValue );
     InterpolationEnum toInterpolationEnum( const std::string& enumString );
	 bool isHybridInterpolation( const InterpolationEnum enumValue );
	 bool isHybridInterpolation( const std::string& enumString );

    enum CCY
    {
		/* Null Case */
		NO_CCY,
        
		/* Majors */
		AUD,			// Australia Dollar
        EUR,			// Europe Euro
        GBP,			// United Kingdom Pound
        JPY,			// Japan Yen
		NZD,			// New Zealand Dollar
        USD,			// United States Dollar

		/* Minors */
		AED,			// United Arab Emirates Dirham
		ARS,			// Argentina Peso
		BRL,			// Brazil Real
		CAD,			// Canada Dollar
		CHF,			// Swiss Franc
		CNH,			// China Yuan (HK Offshore)
		CNY,			// China Yuan (Onshore)
		CLP,			// Chile Peso
		COP,			// Colombia Peso
		CZK,			// Czech Koruna
		DKK,			// Danish Krone
		EGP,			// Egyptian Pound
		HKD,			// Hong Kong Dollar
		HRK,			// Croatia Kuna
		HUF,			// Hungary Forint
		INR,			// India Rupee
		IDR,			// Indonesia Rupiah
		ILS,			// Israel Shekel
		KRW,			// South Korea Won
		MAD,			// Morocco Dirham
		MXN,			// Mexico Peso
		MYR,			// Malaysia Ringgit
		NGN,			// Nigeria Naira
		NOK,			// Norway Krone
		OMR,			// Oman Rial
		PHP,			// Philippines Peso
		PLN,			// Poland Zloty
		QAR,			// Qatar Riyal
		RON,			// Romania Leu
		RSD,			// Serbia Dinar
		RUB,			// Russia Ruble
		SAR,			// Saudi Arabia Riyal
        SEK,			// Sweden Krone
        SGD,			// Singapore Dollar
		TWD,			// Taiwan Dollar
		THB,			// Thailand Baht
		TRY,			// Turkey Lira
		UAH,			// Ukraine Hryvnia
		VND,			// Vietnam Dong
		ZAR				// South Africa Rand
    };
    std::string toString( const CCY ccyEnum );
    CCY toCCYEnum( const std::string& enumString );

    // TODO: get rid of this namespace by using C++11's  'enum class ' instead of namespace protection of names
    namespace serialize
    {

        // TODO: add XML, ProtoBuf, etc.
        enum SerializationMethodEnum
        {
            JSON
        };

        enum SerializationTargetEnum
        {
            STRING,
            FILE
        };

    }

    enum NotionalExchangeEnum
    {
        NONE_NE, // No exchange
        START_NE, // exchanges on start(effective) date and intermediate dates
        END_NE,   // exchanges on intermediate dates and end(maturity) date
        START_AND_END_NE // exchanges on start date, intermediate dates, and end date
    };
     std::string toString( const NotionalExchangeEnum enumValue );
     NotionalExchangeEnum toNotionalExchangeEnum( const std::string& enumString );

 
    enum ScheduleTypeEnum
    {
        NONE_SCHEDULE_TYPE,
		FIXED_SCHEDULE_TYPE,
        FLOAT_SCHEDULE_TYPE,
        FEE_SCHEDULE_TYPE,
		FRA_SCHEDULE_TYPE,
		PREMIUM_SCHEDULE_TYPE,
		PROTECTION_SCHEDULE_TYPE,
		CMS_SCHEDULE_TYPE,
		INFLATION_SCHEDULE_TYPE,
		SWAPSCHEDULE_FIXEDBOND, // Swap fixed leg as the fixed bond proxy
		SWAPSCHEDULE_FLOATBOND, // Swap float leg as the float bond proxy
		BONDSCHEDULE_FIXEDBOND, //internal used for BOND
		BONDSCHEDULE_FLOATBOND,
	};
     std::string toString( const ScheduleTypeEnum enumValue );
     ScheduleTypeEnum toScheduleTypeEnum( const std::string& enumString );

	enum BespokeScheduleTypeEnum
	{
		// The following fields are used internally (not exposed to users) to identify a bespoke schedule:
		NONE_BESPOKE_SCHEDULE,
		BESPOKE_SCHEDULE_WITH_PROPERTIES, // *** Cashflow without daycount and forcaseCurve column
		BESPOKE_SCHEDULE, // *** Cashflow with daycount and forcaseCurve column
	};
	 std::string toString(const BespokeScheduleTypeEnum enumValue);
	 BespokeScheduleTypeEnum toBespokeScheduleTypeEnum(const std::string& enumString);

    enum SwapTypeEnum
    {
        VANILLA_SWAP,
        CROSS_CURRENCY_SWAP,
        ZERO_COUPON_SWAP,
        XCCY_ZERO_COUPON_SWAP,
		CREDIT_DEFAULT_SWAP,
		CONSTANT_MATURITY_SWAP,
		TOTAL_RETURN_SWAP,
		ZERO_COUPON_INFLATION_SWAP
    };
     std::string toString( const SwapTypeEnum enumValue );
     SwapTypeEnum toSwapTypeEnum( const std::string& enumString );

	enum PaymentTriggerEnum
	{
		PAY_ALWAYS,
		PAY_ON_SURVIVAL,
		PAY_ON_DEFAULT
	};
	 std::string toString( const PaymentTriggerEnum enumValue );
     PaymentTriggerEnum toPaymentTriggerEnum( const std::string& enumString );

	enum TRSAssetPerformanceEnum
	{
		NO_PERFORMANCE,					// Do not pay the asset performance
		PAY_AT_MATURITY_PERFORMANCE,	// Pay the asset performance at maturity of the TRS
		PAY_EACH_COUPON_PERFORMANCE,	// Pay the asset performance at each coupon payment date
		RESTRIKE_NOTIONAL_PERFORMANCE	// Include the asset performance by resetting the notional.
	};
     std::string toString( const TRSAssetPerformanceEnum enumValue );
     TRSAssetPerformanceEnum toTRSAssetPerformanceEnum( const std::string& enumString );

    enum FileTypeEnum
    {
        JSON,
        TEXT
    };
     std::string toString( const FileTypeEnum enumValue );
     FileTypeEnum toFileTypeEnum( const std::string& enumString );

    
    enum FrequencyEnum
    {
        NONE_FREQUENCY,
        ANNUAL_FREQUENCY,
        SEMI_ANNUAL_FREQUENCY,
        QUARTERLY_FREQUENCY,
        MONTHLY_FREQUENCY,
        WEEKLY_FREQUENCY,
        DAILY_FREQUENCY,
        AT_MATURITY_FREQUENCY
    };
     std::string toString( const FrequencyEnum enumValue );
     FrequencyEnum toFrequencyEnum( const std::string& enumString );
    
    // Additional Helper Methods
     FrequencyEnum toFrequencyEnumFromCurveTenorEnum( const CurveTenorEnum curveTenorEnum );
     CurveTenorEnum fromFrequencyEnumtoCurveTenorEnum( const FrequencyEnum frequencyEnum );
    
    enum ProductEnum
    {
        NONE_PRODUCT,
        SWAP_PRODUCT,
        BOND_PRODUCT
    };
     std::string toString( const ProductEnum enumValue );
     ProductEnum toProductEnum( const std::string& enumString );

    enum CashflowHeaderEnum
    {
        //For Schedule:
        FIXING_DATE_HEADER,
		FIXING_END_DATE_HEADER,
		ACCRUAL_START_HEADER,
        ACCRUAL_END_HEADER,
        ACCRUAL_DAYS_HEADER,
        ACCRUAL_YEAR_FRACTIONS_HEADER,
        TRUE_YIELD_YEAR_FRACTIONS_HEADER,
		EX_DIVIDEND_DATE_HEADER,
		PAYMENT_DATE_HEADER,

        //For Fixed/Float Cashflow:
        NOTIONAL_HEADER,
        NOTIONAL_EXCHANGE_HEADER,
        LEVERAGE_HEADER,
		COUPON_MULTIPLIER_HEADER,
        FIXED_RATE_HEADER,
        FLOAT_SPREAD_HEADER,
        FLOAT_RATE_HEADER,
		CONVEXITY_HEADER, // for libor in arrear
		UNADJUSTED_FLOAT_RATE_HEADER, // for libor in arrear
		FX_FIXING_DATE_HEADER,
        FX_RATE_HEADER,
        COUPON_HEADER,
        DISCOUNT_FACTOR_HEADER,
        COUPON_PV_HEADER,

        //For Fee Cashflow:
        AMOUNT_HEADER,
        PAY_RECEIVE_HEADER,

        //For FRA Cashflow:
        STRIKE_RATE_HEADER,

		// For Credit Default Swap Cashflows:
		CDS_SPREAD_HEADER,
		SURVIVAL_PROBABILITY_HEADER,
		MARGINAL_DEFAULT_PROBABILITY_HEADER,
		PREMIUM_COUPON_HEADER,
		RISKY_COUPON_HEADER,
		ACCRUAL_ON_DEFAULT_COUPON_HEADER,

		//For Option Cashflow:
		EXPIRY_DATE_HEADER,
		STRIKE_HEADER,
		VOL_HEADER,

		// Bespoke cashflow's extra headers
		ACCRUAL_DAYCOUNT_HEADER,
		FIXING_BUSINESSDAYADJUSTMENT_HEADER,
		FIXING_CALENDAR_HEADER,
		FORECAST_CURVE_HEADER

	};

	// Helper functor to calculate the hash of an enum class.
	// Use this if you wish to use an enum as the key in an unordered_set or unordered_map.
	// https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
	//
	// NOTE: Be careful to test if you intend to use negative enums
	// i.e. enums which do not start at 0.
	// Testing shows this /should/ work: the enum value is mapped to SIZE_MAX + ( negative value )
	struct EnumClassHash
	{
		template <typename T>
		std::size_t operator()(T t) const
		{
			return static_cast<std::size_t>(t);
		}
	};

	 std::string toUpperString(const CashflowHeaderEnum enumValue);
	 std::string toString( const CashflowHeaderEnum enumValue );
	 CashflowHeaderEnum toCashflowHeaderEnum( const std::string& enumString );
     std::unordered_set<CashflowHeaderEnum,EnumClassHash> toCashflowHeaderEnumSet( const std::vector<std::string>& enumStrings );

	enum FXPriceEnum
    {
        NONE_FXPRICE,       //Including BID, ASK, and MID outrights and swappoints
        OUTRIGHT_FXPRICE,   //Including BID, ASK, and MID outrights
        POINTS_FXPRICE,     //Including BID, ASK, and MID swap points
        BID_FXPRICE,        //Including BID's outrights and swappoints
        ASK_FXPRICE,        //Including ASK's outrights and swappoints
        MID_FXPRICE,        //Including MID's outrights and swappoints
        BID_OUTRIGHT_FXPRICE,
        ASK_OUTRIGHT_FXPRICE,
        MID_OUTRIGHT_FXPRICE,
        BID_POINTS_FXPRICE,
        ASK_POINTS_FXPRICE,
        MID_POINTS_FXPRICE
    };
     std::string toString( const FXPriceEnum enumValue );
     FXPriceEnum toFXPriceEnum( const std::string& enumString );
	 std::unordered_set<FXPriceEnum, EnumClassHash> toFXPriceEnumSet( const std::vector<std::string>& enumStrings );

    enum CallOrPutEnum
    {
		NONE_OPTION,
		CALL_OPTION,
        PUT_OPTION,
    };
     std::string toString( const CallOrPutEnum enumValue );
     CallOrPutEnum toCallOrPutEnum( const std::string& enumString );

	enum CapFloorEnum
	{
		CAP_OPTION,
		FLOOR_OPTION
	};
	 std::string toString(const CapFloorEnum enumValue);
	 CapFloorEnum toCapFloorEnum(const std::string& enumString);

    enum CapletFloorletEnum
	{
		CAPLET_OPTION,
		FLOORLET_OPTION
	};
	 std::string toString(const CapletFloorletEnum enumValue);
	 CapletFloorletEnum toCapletFloorletEnum(const std::string& enumString);

	enum BooleanEnum
    {
        NONE_BOOL,
        TRUE_BOOL,
        FALSE_BOOL
    };
     std::string toString( const BooleanEnum enumValue );
     BooleanEnum toBooleanEnum( const std::string& enumString );
     bool toBoolean( const BooleanEnum enumValue );
    
	 bool toBooleanFromString( const std::string& enumString );
     BooleanEnum toBooleanEnumFromBool( const bool& boolValue );

    enum StateVariableEnum
    {
		STATE_VARIABLE_NONE,
		STATE_VARIABLE_ZERO_RATE,
        STATE_VARIABLE_ZERO_RATE_TIMES_TIME,
        STATE_VARIABLE_LOG_DF,
        STATE_VARIABLE_DF,
		STATE_VARIABLE_FORWARD_RATE
    };
     std::string toString( const StateVariableEnum enumValue );
     StateVariableEnum toStateVariableEnum( const std::string& enumString );

	 enum FuturesTypeEnum
	 {
		 FUTURES_TYPE_NONE, 
		 
		 // DF Approach: Firstly build regular futures (Mar, Jun, Sept, Dec for USD)
		 // Secondly backout the startDF/endDF of the serial futures (months not in regular futures).
		 SERIAL_FUTURES_BY_DF,
		 
		 // MCM Adjusted Rate Approach: Use Rate (default value) and adjust the future rate based on the adjusted future end dates.
		 SERIAL_FUTURES_BY_RATE,		
		 
		 // Use Standard Contiguous FUTURE routine
		 CONTIGUOUS_FUTURES				
	 };
	 std::string toString(const FuturesTypeEnum enumValue);
	 FuturesTypeEnum toFuturesTypeEnum(const std::string& enumString);

	enum PayReceiveEnum
    {
        NONE_PAYRECEIVE_ENUM,
        PAY_PAYRECEIVE_ENUM,
        RECEIVE_PAYRECEIVE_ENUM
    };
     std::string toString( const PayReceiveEnum enumValue );
     PayReceiveEnum toPayReceiveEnum( const std::string& enumString );
	 PayReceiveEnum flipPayReceive(const PayReceiveEnum& payRec );
    

    enum PayerReceiverSwaptionEnum
    {
        NONE_SWAPTION,
        PAYER_SWAPTION,
        RECEIVER_SWAPTION
    };
     std::string toString( const PayerReceiverSwaptionEnum enumValue );
     PayerReceiverSwaptionEnum toPayerReceiverSwaptionEnum( const std::string& enumString );
    
	enum OptionStyleEnum
	{
		NONE_STYLE_OPTION,
		EUROPEAN_OPTION,
		BERMUDAN_OPTION,
		AMERICAN_OPTION
	};
	 std::string toString( const OptionStyleEnum enumValue );
     OptionStyleEnum toOptionStyleEnum( const std::string& enumString );
    

    enum VolatilityTypeEnum
    {
        NONE_VOLATILITY,
        LOGNORMAL_VOLATILITY,
        SHIFTED_LOGNORMAL_VOLATILITY,
        NORMAL_VOLATILITY
    };
     std::string toString( const VolatilityTypeEnum enumValue );
     VolatilityTypeEnum toVolatilityTypeEnum( const std::string& enumString );

	enum ConvexityMethodEnum
	{
		NONE_CONVEXITY, //No convexity is applied, turn off the convexity adjustment
		HULL_APPROX_CONVEXITY,
		STANDARD_CONVEXITY		// TODO: Fix Bad Enum, STANDARD = Linear Rate Model by Antoon Pelsser
	};
	 std::string toString(const ConvexityMethodEnum enumValue);
	 ConvexityMethodEnum toConvexityMethodEnum(const std::string& enumString);

	enum PricingModelEnum
	{
		NONE_MODEL,
		BLACK_MODEL,
		BLACK_SCHOLES_MODEL
	};
	 std::string toString(const PricingModelEnum enumValue);
	 PricingModelEnum toPricingModelEnum(const std::string& enumString);

	enum SettlementTypeEnum
	{
		NONE_SETTLEMENT,
		CASH_PRICE_SETTLEMENT,
		CASH_PAR_YIELD_SETTLEMENT,
		PHYSICAL_SETTLEMENT
	};
	 std::string toString( const SettlementTypeEnum enumValue );
     SettlementTypeEnum toSettlementTypeEnum( const std::string& enumString );


	enum FraStyleEnum
	{
		NONE_FRA_STYLE,
		REGULAR_FRA_STYLE,
		BROKEN_DATED_FRA_STYLE
	};
	 std::string toString(const FraStyleEnum enumValue);
	 FraStyleEnum toFraStyleEnum(const std::string& enumString);

	enum CashflowTypeEnum
	{
		NORMAL_CASHFLOW_TYPE, //normal cashflow
		FIRST_NOTIONAL_EXCHANGE_CASHFLOW_TYPE, //special cashflow for first notional exchange
		NORMAL_LAST_CASHFLOW_TYPE //normal cashflow but it's the last cashflow
	};
	 std::string toString(const CashflowTypeEnum enumValue);
	 CashflowTypeEnum toCashflowTypeEnum(const std::string& enumString);

	enum GreekTypeEnum
	{
		NONE_GREEK_TYPE, 
		ANALYTICAL_GREEK_TYPE, 
		NUMERICAL_GREEK_TYPE,
		AAD_GREEK_TYPE
	};
	 std::string toString(const GreekTypeEnum enumValue);
	 GreekTypeEnum toGreekTypeEnum(const std::string& enumString);

    enum VolatilityDataSourceEnum
	{
        NONE_VOLDATA,
        SWAPTION_VOLDATA, 
		CAP_VOLDATA,
        FLOOR_VOLDATA,
        HISTORICAL_VOLDATA,
        MANUALINPUT_VOLDATA
	};
	 std::string toString(const VolatilityDataSourceEnum enumValue);
	 VolatilityDataSourceEnum toVolatilityDataSourceEnum(const std::string& enumString);

    enum OptionTradeTypeEnum
	{
        NONE_TRADE_TYPE,
        CAP_FLOOR_TRADE,
		EUROPEAN_SWAPTION_TRADE,
        BOND_OPTION_TRADE
	};
	 std::string toString(const OptionTradeTypeEnum enumValue);
	 OptionTradeTypeEnum toOptionTradeTypeEnum(const std::string& enumString);

    enum LongShortPositionEnum
	{
        NONE_POSITION,
        LONG_POSITION,
        SHORT_POSITION
	};
	 std::string toString(const LongShortPositionEnum enumValue);
	 LongShortPositionEnum toLongShortPositionEnum(const std::string& enumString);

	enum RandomNumberGeneratorEnum
	{
		NONE_GENERATOR,
		MERSENNE_TWISTER_GENERATOR,
		SOBOL_GENERATOR
	};
	 std::string toString(const RandomNumberGeneratorEnum enumValue);
	 RandomNumberGeneratorEnum toRandomNumberGeneratorEnum(const std::string& enumString);

	enum DistributionEnum
	{
		NONE_DISTRIBUTION,
		UNIFORM_DISTRIBUTION,
		NORMAL_DISTRIBUTION
	};
	 std::string toString(const DistributionEnum enumValue);
	 DistributionEnum toDistributionEnum(const std::string& enumString);

	enum FixingTypeEnum
	{
		IN_ADVANCE_FIXING,
		IN_ARREARS_FIXING,
		ARBITRARY_FIXING // Internally used, when fixing date is between accrualStart and accrualEnd
	};
	 std::string toString(const FixingTypeEnum enumValue);
	 FixingTypeEnum toFixingTypeEnum(const std::string& enumString);

	 enum FixingTableEnum
	 {
		 INTEREST_RATE_FIXING_TABLE,
		 INFLATION_FIXING_TABLE,
		 FX_FIXING_TABLE
	 };
	 std::string toString( const FixingTableEnum enumValue );
	 FixingTableEnum toFixingTableEnum( const std::string& enumString );


    enum OISLongTermInstrumentsEnum
	{
		NONE_OIS_LONGTERM_INSTRUMENTS,
		LIBOROIS_OIS_LONGTERM_INSTRUMENTS
	};
	 std::string toString(const OISLongTermInstrumentsEnum enumValue);
	 OISLongTermInstrumentsEnum toOISLongTermInstrumentsEnum(const std::string& enumString);

	 enum OISBasisInstrumentSwapTypeEnum
	 {
		 NONE_OISBASIS_INSTRUMENT_SWAP_TYPE,
		 OIS_OISBASIS_INSTRUMENT_SWAP_TYPE,
		 LIBOR_OISBASIS_INSTRUMENT_SWAP_TYPE
	 };
	 std::string toString(const OISBasisInstrumentSwapTypeEnum enumValue);
	 OISBasisInstrumentSwapTypeEnum toOISBasisInstrumentSwapTypeEnum(const std::string& enumString);

	 enum OISLongTermCompoundingEnum
	 {
		 NONE_OIS_LONGTERM_COMPOUNDING,
		 DAILY_AVERAGES_OIS_LONGTERM_COMPOUNDING
	 };
	 std::string toString(const OISLongTermCompoundingEnum enumValue);
	 OISLongTermCompoundingEnum toOISLongTermCompoundingEnum(const std::string& enumString);

	 enum OISMidTermInstrumentsEnum
	 {
		 NONE_OIS_SHORTTERM_INSTRUMENTS,
		 CENTRAL_BANK_SWAP,
		 FED_FUND_FUTURE,
		 ONE_MONTH_FUTURE,
		 THREE_MONTH_FUTURE
	 };
	 std::string toString(const OISMidTermInstrumentsEnum enumValue);
	 OISMidTermInstrumentsEnum toOISMidTermInstrumentsEnum(const std::string& enumString);

    enum OISCompoundingEnum
	{
		NONE_OIS_COMPOUNDING,
		ARITHMETIC_OIS_COMPOUNDING,
        GEOMETRIC_OIS_COMPOUNDING
	};
	 std::string toString(const OISCompoundingEnum enumValue);
	 OISCompoundingEnum toOISCompoundingEnum(const std::string& enumString);

	enum LoanTypeEnum
	{
		AMORTIZING_LOAN_TYPE = 0,   // Set AMORTIZING to 0 and BULLET to 1, to match the convention used by MGEN.
		BULLET_LOAN_TYPE = 1,
		NONE_LOAN_TYPE
	};
	 std::string toString(const LoanTypeEnum enumValue);
	 LoanTypeEnum toLoanTypeEnum(const std::string& enumString);

	enum SupervisoryTypeEnum
	{
		NONE_SUPERVISORY_TYPE,
		LEGACY_SUPERVISORY_TYPE,
		IRBA_SUPERVISORY_TYPE
	};
	 std::string toString(const SupervisoryTypeEnum enumValue);
	 SupervisoryTypeEnum toSupervisoryTypeEnum(const std::string& enumString);

	enum PoolTypeEnum
	{
		NONE_POOL_TYPE,
		WHOLESALE_POOL_TYPE,
		RETAIL_POOL_TYPE
	};
	 std::string toString(const PoolTypeEnum enumValue);
	 PoolTypeEnum toPoolTypeEnum(const std::string& enumString);

	enum RankTypeEnum
	{
		NONE_RANK_TYPE,
		SENIOR_RANK_TYPE,
		NON_SENIOR_RANK_TYPE
	};
	 std::string toString(const RankTypeEnum enumValue);
	 RankTypeEnum toRankTypeEnum(const std::string& enumString);

	enum TrancheTypeEnum
	{
		NONE_TRANCHE_TYPE,
		PRO_RATA_TRANCHE_TYPE,
		SEQUENTIAL_TRANCHE_TYPE
	};
	 std::string toString(const TrancheTypeEnum enumValue);
	 TrancheTypeEnum toTranchTypeEnum(const std::string& enumString);

	 enum TrancheCouponTypeEnum
	 {
		 NONE_COUPON_TYPE,
		 FIXED_COUPON_TYPE,
		 FLOAT_COUPON_TYPE
	 };
	 std::string toString( const TrancheCouponTypeEnum enumValue );
	 TrancheCouponTypeEnum toTrancheCouponTypeEnum( const std::string& enumString );

	 enum OverridePayTypeEnum
	 {
		 NONE_OVERRIDE_PAY_TYPE,
		 INACTIVE_OVERRIDE_PAY_TYPE,
		 SEQUENTIAL_OVERRIDE_PAY_TYPE
	 };

	enum SecIrDataProviderranularityTypeEnum
	{
		NONE_SPECIFIED_GRANULAR_TYPE,
		GRANULAR_TYPE,
		NON_GRANULAR_TYPE
	};

	 std::string toString(const SecIrDataProviderranularityTypeEnum enumValue);
	 SecIrDataProviderranularityTypeEnum toSecIrDataProviderranularityTypeEnum(const std::string& enumString);

    enum ShiftTypeEnum
    {
        NONE_SHIFT_TYPE,
        FLAT_SHIFT_TYPE,
        PERTURBED_SHIFT_TYPE
    };

    std::string toString( const ShiftTypeEnum enumValue );
    ShiftTypeEnum toShiftTypeEnum( const std::string & enumString );

    enum RiskTypeEnum
    {
        NONE_RISK_TYPE,
        DISCOUNT_FACTOR_RISK_TYPE,
        FORWARD_RATE_RISK_TYPE,
        COMPOUND_RATE_RISK_TYPE
    };

    std::string toString( const RiskTypeEnum enumValue );
    RiskTypeEnum toRiskTypeEnum( const std::string & enumString );

    enum BuildFrequencyEnum
    {
        NEVER_BUILD_FREQUENCY,
        ALWAYS_BUILD_FREQUENCY,
        ONCE_BUILD_FREQUENCY,
        TOLERANCE_BUILD_FREQUENCY
    };

    std::string toString( const BuildFrequencyEnum enumValue );
    BuildFrequencyEnum toBuildFrequencyEnum( const std::string & enumString );

	enum TurnOfYearAdjustmentTypeEnum
    {
        NO_TURN_ADJUSTMENT,
        RELATIVE_TURN_ADJUSTMENT,
        ABSOLUTE_TURN_ADJUSTMENT
    };

    std::string toString( const TurnOfYearAdjustmentTypeEnum enumValue );
    TurnOfYearAdjustmentTypeEnum toTurnOfYearAdjustmentTypeEnum( const std::string & enumString );


	enum SabrModelEnum
	{
		SABR_MODEL_PROPERTIES,
		SABR_MODEL_PARAMETERS,
		SABR_MODEL_MKTDATA,
		SABR_MODEL_VOL_MKTDATA
	};
	std::string toString(const SabrModelEnum enumValue);
	SabrModelEnum toSabrModelEnum(const std::string& enumString);

	enum SabrMarketDataEnum
	{
		SABR_MKTDATA_PROPERTIES,
		SABR_MKTDATA
	};
	std::string toString(const SabrMarketDataEnum enumValue);
	SabrMarketDataEnum toSabrMarketDataEnum(const std::string& enumString);

	enum SabrMarketDataTypeEnum
	{
		SABR_MKTDATA_VOL,
		SABR_MKTDATA_SWAPRATE,
		SABR_MKTDATA_ANNUITY,
		SABR_MKTDATA_PARAM
	};
	std::string toString(const SabrMarketDataTypeEnum enumValue);
	SabrMarketDataTypeEnum toSabrMarketDataTypeEnum(const std::string& enumString);

	enum SabrMarketDataValueTypeEnum
	{
		ABSOLUTE_SABR_VALUETYPE,
		RELATIVE_SABR_VALUETYPE
	};
	std::string toString(const SabrMarketDataValueTypeEnum enumValue);
	SabrMarketDataValueTypeEnum toSabrMarketDataValueTypeEnum(const std::string& enumString);

	enum SabrParamEnum
	{
		ALPHA_SABR,
		BETA_SABR,
		RHO_SABR,
		NU_SABR
	};
	std::string toString(const SabrParamEnum enumValue);
	SabrParamEnum toSabrParamEnum(const std::string& enumString);


	enum SABRCalibrationTargetEnum
	{
		SABR_CALIB_VOLATILITY,
		SABR_CALIB_PREMIUM
	};
	std::string toString(const SABRCalibrationTargetEnum enumValue);
	SABRCalibrationTargetEnum toSABRCalibrationTargetEnum(const std::string& enumString);

	enum SABRApproxMethodEnum
	{
		HAGAN_APPROX_METHOD,
		ANTONOV_APPROX_METHOD
	};
	std::string toString(const SABRApproxMethodEnum enumValue);
	SABRApproxMethodEnum toSABRApproxMethodEnum(const std::string& enumString);


}
