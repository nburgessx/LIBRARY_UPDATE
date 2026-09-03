//
// LACurveForwardRateHelpers.h
// *** This file used to be called "LAMathCurveFuncUti1ity.h" ***
//
#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#ifndef WINDOWS
#if defined(WIN32) || defined(WIN64)
#define WINDOWS
#endif
#endif

// External Includes
#include <memory>
#include <utility>
#include <vector>
#include <tuple>

// Internal Includes
#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "AQLString.h"
#include "LACurvePricingObject.h"
#include "LATime.h"

// TODO - Try to remove this file. It contains lots of #defines that clash with external binaries
#include "ConstantDeclarations.h"

namespace etrading
{

    class LACurveForwardRateHelpers
    {
    public:

        // Normal = Geometric, Average = Arithmetic Averaging, Flat and Simple the same.
        enum COMPOUND_TYPE { COMPOUND_NORMAL, COMPOUND_FLAT, COMPOUND_SIMPLE, AVERAGE };

        static const char* YIELD_CURVE_NAME_PREFIX;

        static const char* YIELD_CURVE_PRO_NAME_PREFIX;

        // Get the rate using ACT/365 daycount for smooth rates. Note ACT/365_ISDA creates leap-year irregularities
        static double getRate(const AQLDate&  fromdate, AQLString term, AQLDataInstance* dataInstance, const AQLString& curveid,
            AQLString ratetype, AQLString frequency, AQLString daycount, AQLString slidingrule,
            AQLString calendar, AQLString interpolation, AQLString foreCurveName = STD,
            AQLString dfCurveName = STD, bool isFWDInter = true, const AQLString* roll_convention = NULL);

        static double getParRate(DateVector& datevec, AQLDataInstance* dataInstance, const AQLString& curveid,
            AQLString daycount, AQLString interpolation, AQLString foreCurveName = STD,
            AQLString dfCurveName = STD, bool isFWDInter = true);

        // Get the par rate with fixed and floating date schedules and fixing overrides
        //
        // Note: 'floatSpread' is in Basis Points, the optional 'useFwdData' boolean if true will use forward data points directly 
        // ... and if set to false will imply forward rates from discount factors.
        //
        static double getParRate(DateVector& fixedAccrualDates, DateVector& fixedPaymentDates, DateVector& floatFixingDates,
            DateVector& floatAccrualDates, DateVector& floatPaymentDates, AQLDataInstance* dataInstance, const AQLString& curveid,
            AQLString fixedDaycount, AQLString floatDaycount, AQLString interpolation, AQLString foreCurveName,
            AQLString dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
            double lastFixing, double floatSpread = 0.0, bool useFwdData = false, bool isOIS = false, AQLString oisCompoundingType = "",
            AQLString calendar = "", AQLString rollConvention = "", AQLString slidingRule = "");

        // Get the Swap PV
        static double getSwapPV(bool& isFixedRatePayerSwap, double& notional, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
            DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
            AQLDataInstance* dataInstance, const AQLString& curveid, double& fixedRate, AQLString fixedDaycount,
            double& floatSpreadInBasisPoints, AQLString floatDaycount, AQLString interpolation, AQLString foreCurveName,
            AQLString dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
            double lastFixing, bool useFwdData = false, bool isOIS = false, AQLString compoundingMethod = "", AQLString floatCalendar = "",
            AQLString floatRollConv = "", AQLString	slidingRule = "");

        // Get the Swap DV01
        static double getSwapDV01(bool isFixedRatePayerSwap, double notional, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
            DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
            AQLDataInstance* dataInstance, const AQLString& curveid, double& fixedRate, AQLString fixedDaycount,
            double& floatSpreadInBasisPoints, AQLString floatDaycount, AQLString interpolation, AQLString foreCurveName,
            AQLString dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
            double lastFixing, bool isOIS = false, AQLString compoundingMethod = "", AQLString floatCalendar = "",
            AQLString floatRollConv = "", AQLString	slidingRule = "");

        // Get the Swap PV01
        static double getSwapPV01(bool& isFixedRatePayerSwap, double& notional, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
            AQLDataInstance* dataInstance, const AQLString& curveid, AQLString fixedDaycount, AQLString interpolation, AQLString foreCurveName,
            AQLString dfCurveName);

        // Get the Asset Swap Spread using Par-Par Methodology
        static double getAssetSwapSpread(const double& bondPrice, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
            DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
            AQLDataInstance* dataInstance, const AQLString& curveid, double& fixedRate, AQLString fixedDaycount,
            AQLString floatDaycount, AQLString interpolation, AQLString foreCurveName, AQLString dfCurveName,
            bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing, double lastFixing,
            bool isCleanPrice = true, const AQLDate& settlementDate = AQLDate());

        static double getParRate(AQLString term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString frequency, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, AQLString foreCurveName = STD,
            AQLString dfCurveName = STD, bool isFWDInter = true, const AQLString* roll_convention = NULL);

        static double getParRate(const AQLDate& fromdate, AQLString term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString frequency, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, AQLString foreCurveName = STD,
            AQLString dfCurveName = STD, bool isFWDInter = true, const AQLString* roll_convention = NULL);

        static double getParRate(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLDate& fromDate, const AQLDate& toDate, const AQLDate* firstStubDate,
            const AQLDate* lastStubDate, const int* pday, AQLString freq, AQLString daycount, AQLString slidingrule, AQLString calendar,
            AQLString interpolation, AQLString foreCurveName = STD, AQLString dfCurveName = STD, bool isFWDInter = true);

        static double getForwardRate(const AQLDate& fromdate, double term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString frequency, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, AQLString curveName = STD, bool isFWDInter = true);

		// Function to imply the forward rate from a curve
        static DoubleArray getMultiForwardRate(const DateVector& fromdate, double term, AQLDataInstance* dataInstance, const AQLString& curveid,
            AQLString frequency, AQLString daycount, AQLString slidingrule, AQLString calendar, AQLString interpolation,
            AQLString curveName = STD, bool isFWDInter = true, bool useFwdData = false );

        static double getForwardRate(const AQLDate& fromdate, const AQLDate& todate, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString frequency, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, AQLString curveName = STD, bool isFWDInter = true);

		// Function to imply the forward rate from a curve
        static DoubleArray getMultiForwardRate(const DateVector& fromdate, const DateVector& todate, AQLDataInstance* dataInstance, const AQLString& curveid,
            AQLString frequency, AQLString daycount, AQLString slidingrule, AQLString calendar, AQLString interpolation, AQLString curveName = STD,
			bool isFWDInter = true, bool useFwdData = false ); // useFwdData: false = imply forwards from DiscountFactors, true = use forward data directly

        // Function to get the curve frequency
        static AQLString getCurveFrequency(const AQLString& curveCollection, const AQLString& curveName) { return "NOT_IMPLEMENTED"; }; // CurveCollection also known as CurveID

        // Function to get forward rates from discount factors. No todate required since this is implied from the curve frequency
        static DoubleArray getMultiForwardRatesUsingCurveFrequency(const DateVector& fromdate, AQLString& curveFrequency, AQLDataInstance* dataInstance, const AQLString& curveid,
            AQLString frequency, AQLString daycount, AQLString slidingrule, AQLString calendar, AQLString interpolation, AQLString curveName = STD,
			bool isFWDInter = true, bool useFwdData = false); // useFwdData: false = imply forwards from DiscountFactors, true = use forward data directly

        static double getForwardRate(const AQLDate& fromdate, AQLString term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString frequency, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, AQLString curveName = STD, bool isFWDInter = true);

		// Function to imply the forward rate from a curve
        static DoubleArray getMultiForwardRate(const DateVector& fromdate, AQLString term, AQLDataInstance* dataInstance, const AQLString& curveid,
            AQLString frequency, AQLString daycount, AQLString slidingrule, AQLString calendar, AQLString interpolation, AQLString curveName = STD,
			bool isFWDInter = true );

        static double getZeroRate(double term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString frequency,
            AQLString daycount, AQLString interpolation, AQLString curveName = STD, bool isFWDInter = true);

        static DoubleArray getMultiZeroRate(const DoubleArray& term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString frequency,
            AQLString daycount, AQLString interpolation, AQLString curveName = STD, bool isFWDInter = true);

        static double getZeroRate(AQLString term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString frequency,
            AQLString daycount, AQLString slidingrule, AQLString calendar, AQLString interpolation,
            AQLString curveName = STD, bool isFWDInter = true);

        static DoubleArray getMultiZeroRate(AQLStringVector term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString frequency,
            AQLString daycount, AQLString slidingrule, AQLString calendar, AQLString interpolation,
            AQLString curveName = STD, bool isFWDInter = true);

        static double getDF(const AQLDate& fromdate, AQLString term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, bool isbasisflag = false,
            AQLString curveName = STD);

        static DoubleArray getMultiDF(const DateVector& fromdate, AQLString term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, bool isbasisflag = false,
            AQLString curveName = STD);

        static double getDF(const AQLDate& fromdate, double term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, bool isbasisflag = false,
            AQLString curveName = STD);

        static DoubleArray getMultiDF(const DateVector& fromdate, const DoubleArray& term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, bool isbasisflag = false,
            AQLString curveName = STD);

        static double getDF(const AQLDate& fromdate, const AQLDate& todate, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, bool isbasisflag = false,
            AQLString curveName = STD);

        static DoubleArray getMultiDF(const DateVector& fromdate, const DateVector& todate, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, bool isbasisflag = false,
            AQLString curveName = STD);

        // Function to get spot discount factors. No fromdate required since this is the curve asof date
        static DoubleArray getMultiSpotDiscountFactors(const DateVector& todate, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, bool isbasisflag = false,
            AQLString curveName = STD);

        static double getDF(AQLString term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, bool isbasisflag = false,
            AQLString curveName = STD);

        static DoubleArray getMultiDF(AQLStringVector term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount,
            AQLString slidingrule, AQLString calendar, AQLString interpolation, bool isbasisflag = false,
            AQLString curveName = STD);

        static double getDF(double term, AQLDataInstance* dataInstance, const AQLString& curvid, AQLString daycount, AQLString interpolation, bool isbasisflag = false,
            AQLString curveName = STD);

        static DoubleArray getMultiDF(const DoubleArray& term, AQLDataInstance* dataInstance, const AQLString& curvid, AQLString daycount,
            AQLString interpolation, bool isbasisflag = false, AQLString curveName = STD);

        static double getBasisDF(double term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount, AQLString interpolation);

        static DoubleArray getMultiBasisDF(const DoubleArray& term, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount, AQLString interpolation);

        static double getGridRate(AQLDataInstance* dataInstance, const AQLString& curveid, AQLString currency, AQLString index, AQLString grid);

        static double getAnnuity(const AQLDate& start, const AQLDate& end, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString data_frequency,
            AQLString slidingrule, AQLString calendar, const AQLDate* firstStubDate, const AQLDate* lastStubDate,
            const int* pday, AQLString daycount, AQLString interpolation, AQLString curveName = STD);

        static double getAnnuity(const DateVector& datevec, AQLDataInstance* dataInstance, const AQLString& curveid, AQLString daycount, AQLString interpolation,
            AQLString curveName = STD);

        static double getBasisSwapValue(AQLDataInstance* dataInstance, const AQLDate& valueDate, const AQLDate& startDate, AQLString& term, const  AQLString& arbFreeCurveID,
            const AQLString& forecastCurveID, const AQLString& discountCurveID, double basis, bool isPrincipal, AQLString& frequency,
            const AQLString& daycount, const AQLString& slidingrule, const AQLString& calendar, double firstFixingRate = 0.,
            bool isEOMRoll = false, bool isFRN = false);

        static double getCurBasisSwapValue(AQLDataInstance* dataInstance, const AQLDate& valueDate, const AQLDate& startDate, const AQLString& term, const  AQLString& arbFreeCurveID,
            const AQLString& discountCurveID, const AQLString& dolArbFreeCurveID, const AQLString& dolForecastCurveID,
            const AQLString& dolDiscountCurveID, const AQLString& frequency, const AQLString& daycount, const AQLString& slidingrule,
            const AQLString& calendar, bool isEOMRoll = false);

        static LACurvePricingObject& getYieldCurveForCurveID(AQLDataInstance* dataInstance, const AQLString& curveid);
        static void setCalendarForCurveID(LACurvePricingObject& yc, const AQLString& calendar);
        static bool setUpForwardDayCount(AQLDataInstance *dataInstance, const AQLString &curveid, const AQLString &curveName, LACurvePricingObject &yc);
        static bool setUpForwardDayCount(AQLDataInstance *dataInstance, const AQLString &curveid, const AQLString &curveName, AQLPriceDataDayCount &dc);

        static double compound( AQLDataInstance* dataInstance,
								const AQLString& curveID,
								const AQLString& forecastCurveName,
								const AQLDate& start_date,
								const AQLDate& end_date,
								const double spread,
								const AQLString& frequency_,
								const bool is_start_roll,
								const AQLString& roll_convention_,
								const AQLString& calendar_,
								const AQLString& sliding_rule_,
								const AQLString& day_count_,
								const AQLString& interpolation_,
								const AQLString& compound_type_,
								const AQLDate* first_odd = NULL,
								const AQLDate* last_odd = NULL,
								const bool usePDay = false );

        //static void clearCalendarForCurveMap(bool isAll, bool isLock);


        //      @brief Function to get the fixed and floating schedules needed for par rate calculations
        //
        //      Pass into this function the following empty DateVector arguments, which are placeholders 
        //      and will populated / overwritten with the correct date schedule results.
        //
        //      @param [in]     AQLDate          effectiveDate                   The swap start or effective date
        //      @param [in]     AQLDate          maturityDate                    The swap end or maturity date
        //      @param [in]     AQLString        fixedFrequency                  Fixed coupon frequency
        //      @param [in]     AQLString        fixedAccrualBusDayAdjustment    Fixed accrual business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        //      @param [in]     AQLString        fixedAccrualCalendar            Fixed accrual calendar
        //      @param [in]     AQLString        fixedPaymentBusDayAdjustment    Fixed payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        //      @param [in]     AQLString        fixedPaymentCalendar            Fixed payment calendar  
        //      @param [in]     AQLString        fixedPaymentLag                 Fixed payment lag
        //      @param [in]     AQLString*       fixedStubType                   Fixed stub type; ShortStart, ShortEnd, LongStart, LongEnd
        //      @param [in]     AQLDate*         fixedFirstStubDate              Fixed first stub date
        //      @param [in]     AQLDate*         fixedLastStubDate               Fixed last stub date
        //      @param [in]     int*            fixedRollDay                    Fixed roll day, an integer representing the day of the month
        //      @param [in]     bool            fixedIsStartRoll,               Are fixed coupons rolling from the start day i.e. ShortEnd stub? True or False
        //      @param [in]     bool            fixedIsEomRoll,                 Are fixed coupons rolling on month end? True or False
        //      @param [in]     AQLString*       fixedRollConvention             Fixed roll convention: IMM, EOM, Start, End or NULL
        //      @param [in]     AQLString        floatFrequency                  Floating coupon frequency
        //      @param [in]     AQLString        floatFixingBusDayAdjustment     Floating payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        //      @param [in]     AQLString        floatFixingCalendar             Floating payment calendar 
        //      @param [in]     AQLString        floatFixingLag                  Floating fixing lag
        //      @param [in]     AQLString        floatAccrualBusDayAdjustment    Floating accrual business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        //      @param [in]     AQLString        floatAccrualCalendar            Floating accrual calendar
        //      @param [in]     AQLString        floatPaymentBusDayAdjustment    Floating payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        //      @param [in]     AQLString        floatPaymentCalendar            Floating payment calendar
        //      @param [in]     AQLString        floatPaymentLag                 Floating payment lag
        //      @param [in]     AQLString*       floatStubType                   Floating stub type; ShortStart, ShortEnd, LongStart, LongEnd
        //      @param [in]     AQLDate*         floatFirstStubDate              Floating first stub date
        //      @param [in]     AQLDate*         floatLastStubDate               Floating last stub date
        //      @param [in]     int*            floatRollDay                    Floating roll day, an integer representing the day of the month
        //      @param [in]     bool            floatIsStartRoll                Are floating coupons rolling from the start day i.e. ShortEnd stub? True or False          
        //      @param [in]     bool            floatIsEomRoll                  Are floating coupons rolling on month end? True or False
        //      @param [in]     AQLString*       floatRollConvention             Floating roll convention: IMM, EOM, Start, End or NULL
        //      @param [out]    DateVector      fixedAccrualDates               ( placeholder for the result )
        //      @param [out]    DateVector      fixedPaymentDates               ( placeholder for the result )
        //      @param [out]    DateVector      floatFixingDates                ( placeholder for the result )
        //      @param [out]    DateVector      floatAccrualDates               ( placeholder for the result )
        //      @param [out]    DateVector      floatPaymentDates               ( placeholder for the result )
        //      @param [in]     AQLString		fixedPaymentFreq                Fixed payment frequency
        //      @param [in]     AQLString		floatPaymentFreq                Float payment frequency
        //      @param [in]	    AQLString		fixingAdvanceOrArrears          Flag to indicate the fixing is advance or arrears
        //
        static void generateSwapSchedule(AQLDate          effectiveDate,
            AQLDate          maturityDate,
            AQLString        fixedFrequency,
            AQLString        fixedAccrualBusinessDayAdjustment,
            AQLString        fixedAccrualCalendar,
            AQLString        fixedPaymentBusinessDayAdjustment,
            AQLString        fixedPaymentCalendar,
            AQLString        fixedPaymentLag,
            AQLString*       fixedStubType,
            AQLDate*         fixedFirstStubDate,
            AQLDate*         fixedLastStubDate,
            int*            fixedRollDay,
            bool            fixedIsStartRoll,
            bool            fixedIsEomRoll,
            AQLString*       fixedRollConvention,
            AQLString        floatFrequency,
            AQLString        floatFixingBusinessDayAdjustment,
            AQLString        floatFixingCalendar,
            AQLString        floatFixingLag,
            AQLString        floatAccrualBusinessDayAdjustment,
            AQLString        floatAccrualCalendar,
            AQLString        floatPaymentBusinessDayAdjustment,
            AQLString        floatPaymentCalendar,
            AQLString        floatPaymentLag,
            AQLString*       floatStubType,
            AQLDate*         floatFirstStubDate,
            AQLDate*         floatLastStubDate,
            int*            floatRollDay,
            bool            floatIsStartRoll,
            bool            floatIsEomRoll,
            AQLString*       floatRollConvention,
            DateVector&     fixedAccrualDates,        // This parameter is a placeholder and will be overwritten with the result dates
            DateVector&     fixedPaymentDates,        // This parameter is a placeholder and will be overwritten with the result dates
            DateVector&     floatFixingDates,         // This parameter is a placeholder and will be overwritten with the result dates
            DateVector&     floatAccrualDates,        // This parameter is a placeholder and will be overwritten with the result dates
            DateVector&     floatPaymentDates,      // This parameter is a placeholder and will be overwritten with the result dates
            AQLString		  fixedPaymentFreq,
            AQLString		  floatPaymentFreq,
            const AQLString& fixingAdvanceOrArrears = AQLString("advance"),
            const bool      isAssetSwap = false,
            AQLDate          issueDate = AQLDate());

        //      @brief Function to get the fixed schedule needed for par rate calculations
        //
        //      Pass into this function the following empty DateVector arguments, which are placeholders 
        //      and will populated / overwritten with the correct date schedule results.
        //
        //      @param [in]     AQLDate          effectiveDate                   The swap start or effective date
        //      @param [in]     AQLDate          maturityDate                    The swap end or maturity date
        //      @param [in]     AQLString        fixedFrequency                  Fixed coupon frequency
        //      @param [in]     AQLString        fixedAccrualBusDayAdjustment    Fixed accrual business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        //      @param [in]     AQLString        fixedAccrualCalendar            Fixed accrual calendar
        //      @param [in]     AQLString        fixedPaymentBusDayAdjustment    Fixed payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        //      @param [in]     AQLString        fixedPaymentCalendar            Fixed payment calendar  
        //      @param [in]     AQLString        fixedPaymentLag                 Fixed payment lag
        //      @param [in]     AQLString*       fixedStubType                   Fixed stub type; ShortStart, ShortEnd, LongStart, LongEnd
        //      @param [in]     AQLDate*         fixedFirstStubDate              Fixed first stub date
        //      @param [in]     AQLDate*         fixedLastStubDate               Fixed last stub date
        //      @param [in]     int*            fixedRollDay                    Fixed roll day, an integer representing the day of the month
        //      @param [in]     bool            fixedIsStartRoll,               Are fixed coupons rolling from the start day i.e. ShortEnd stub? True or False
        //      @param [in]     bool            fixedIsEomRoll,                 Are fixed coupons rolling on month end? True or False
        //      @param [in]     AQLString*       fixedRollConvention             Fixed roll convention: IMM, EOM, Start, End or NULL
        //      @param [out]    DateVector      fixedAccrualDates               ( placeholder for the result )
        //      @param [out]    DateVector      fixedPaymentDates               ( placeholder for the result )
        //      @param [in]     AQLString        fixedPaymentFreq                Fixed payment frequency
        //
        static void generateFixedLegSchedule(AQLDate          effectiveDate,
            AQLDate          maturityDate,
            AQLString        fixedFrequency,
            AQLString        fixedAccrualBusinessDayAdjustment,
            AQLString        fixedAccrualCalendar,
            AQLString        fixedPaymentBusinessDayAdjustment,
            AQLString        fixedPaymentCalendar,
            AQLString        fixedPaymentLag,
            AQLString*       fixedStubType,
            AQLDate*         fixedFirstStubDate,
            AQLDate*         fixedLastStubDate,
            int*            fixedRollDay,
            bool            fixedIsStartRoll,
            bool            fixedIsEomRoll,
            AQLString*       fixedRollConvention,
            DateVector&     fixedAccrualDates,
            DateVector&     fixedPaymentDates,
            AQLString		  fixedPaymentFreq = AQLString());

        //      @brief Function to get the floating schedule needed for par rate calculations
        //
        //      Pass into this function the following empty DateVector arguments, which are placeholders 
        //      and will populated / overwritten with the correct date schedule results.
        //
        //      @param [in]     AQLDate          effectiveDate                   The swap start or effective date
        //      @param [in]     AQLDate          maturityDate                    The swap end or maturity date
        //      @param [in]     AQLString        floatFrequency                  Floating coupon frequency
        //      @param [in]     AQLString        floatFixingBusDayAdjustment     Floating payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        //      @param [in]     AQLString        floatFixingCalendar             Floating payment calendar 
        //      @param [in]     AQLString        floatFixingLag                  Floating fixing lag
        //      @param [in]     AQLString        floatAccrualBusDayAdjustment    Floating accrual business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        //      @param [in]     AQLString        floatAccrualCalendar            Floating accrual calendar
        //      @param [in]     AQLString        floatPaymentBusDayAdjustment    Floating payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        //      @param [in]     AQLString        floatPaymentCalendar            Floating payment calendar
        //      @param [in]     AQLString        floatPaymentLag                 Floating payment lag
        //      @param [in]     AQLString*       floatStubType                   Floating stub type; ShortStart, ShortEnd, LongStart, LongEnd
        //      @param [in]     AQLDate*         floatFirstStubDate              Floating first stub date
        //      @param [in]     AQLDate*         floatLastStubDate               Floating last stub date
        //      @param [in]     int*            floatRollDay                    Floating roll day, an integer representing the day of the month
        //      @param [in]     bool            floatIsStartRoll                Are floating coupons rolling from the start day i.e. ShortEnd stub? True or False          
        //      @param [in]     bool            floatIsEomRoll                  Are floating coupons rolling on month end? True or False
        //      @param [in]     AQLString*       floatRollConvention             Floating roll convention: IMM, EOM, Start, End or NULL
        //      @param [out]    DateVector      fixedAccrualDates               ( placeholder for the result )
        //      @param [out]    DateVector      fixedPaymentDates               ( placeholder for the result )
        //      @param [out]    DateVector      floatFixingDates                ( placeholder for the result )
        //      @param [out]    DateVector      floatAccrualDates               ( placeholder for the result )
        //      @param [out]    DateVector      floatPaymentDates               ( placeholder for the result )
        //      @param [in]     AQLString		floatPaymentFreq                Float payment frequency
        //      @param [in]	    AQLString&		fixingAdvanceOrArrears          Flag to indicate the fixing is advance or arrears
        //      @param [in]     bool			removeExtraDay					True to remove the extra fixing date and payment date. Default to False for backward compatibility, as all the core functions expect fixing dates & payment dates having the same size as accrual days
        //
        static void generateFloatLegSchedule(AQLDate          effectiveDate,
            AQLDate          maturityDate,
            AQLString        floatFrequency,
            AQLString        floatFixingBusinessDayAdjustment,
            AQLString        floatFixingCalendar,
            AQLString        floatFixingLag,
            AQLString        floatAccrualBusinessDayAdjustment,
            AQLString        floatAccrualCalendar,
            AQLString        floatPaymentBusinessDayAdjustment,
            AQLString        floatPaymentCalendar,
            AQLString        floatPaymentLag,
            AQLString*       floatStubType,
            AQLDate*         floatFirstStubDate,
            AQLDate*         floatLastStubDate,
            int*            floatRollDay,
            bool            floatIsStartRoll,
            bool            floatIsEomRoll,
            AQLString*       floatRollConvention,
            DateVector&     floatFixingDates,         // This parameter is a placeholder and will be overwritten with the result dates
            DateVector&     floatAccrualDates,        // This parameter is a placeholder and will be overwritten with the result dates
            DateVector&     floatPaymentDates,       // This parameter is a placeholder and will be overwritten with the result dates
            AQLString		  floatPaymentFreq = AQLString(),
            const AQLString& fixingAdvanceOrArrears = AQLString("advance"),
            bool			  removeExtraDay = false);

        /* @brief			helper method that validates and generates accrual start and end dates from a single combined vector of accrual dates
        *  @param [out]		accrualStartDates		    Accrual Start Dates
        *  @param [out]		accrualEndDates		        Accrual End Dates
        *  @param [in]		combinedAccrualDates        Combined Accrual Dates
        */
        static void validateAndGenerateAccrualStartAndEndDates(DateVector& accrualStartDates,
            DateVector& accrualEndDates,
            const DateVector& combinedAccrualDates);


        /* @brief Function to get the Accrual Schedule and Payment Schedule for fixed/float leg
        *
        *  @param [out]   accrualDates               ( placeholder for the result )
        *  @param [out]   paymentDates               ( placeholder for the result )
        *  @param [in]    effectiveDate              The swap start or effective date
        *  @param [in]    maturityDate               The swap end or maturity date
        *  @param [in]    accrualFrequency           Accrual frequency
        *  @param [in]    acrualBusDayAdjustment     Accrual business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        *  @param [in]    accrualCalendar            Accrual calendar
        *  @param [in]    paymentFreq                Payment frequency
        *  @param [in]    paymentBusDayAdjustment    Payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        *  @param [in]    paymentCalendar            Payment calendar
        *  @param [in]    paymentLag                 Payment lag
        *  @param [in]    stubType                   Stub type; ShortStart, ShortEnd, LongStart, LongEnd
        *  @param [in]    firstStubDate              First stub date
        *  @param [in]    lastStubDate               Last stub date
        *  @param [in]    rollDay                    Roll day, an integer representing the day of the month
        *  @param [in]    isStartRoll                Are coupons rolling from the start day i.e. ShortEnd stub? True or False
        *  @param [in]    isEomRoll                  Are coupons rolling on month end? True or False
        *  @param [in]    rollConvention             Roll convention: IMM, EOM, Start, End or NULL
        *  @param [in]	  fixingAdvanceOrArrears	 Flag to indicate the fixing is advance or arrears
        *  @param [in]    removeExtraPaymentDay		 True to remove the extra payment date at the front. Default to False for backward compatibility, as all the core functions expect payment dates having the same size as accrual days
        */
        static void generateAccrualAndPaymentSchedule(DateVector&     accrualDates,
            DateVector&     paymentDates,
            AQLDate			effectiveDate,
            AQLDate          maturityDate,
            AQLString        accrualFrequency,
            AQLString        accrualBusinessDayAdjustment,
            AQLString        accrualCalendar,
            AQLString		paymentFrequency,
            AQLString        paymentBusinessDayAdjustment,
            AQLString        paymentCalendar,
            AQLString        paymentLag,
            AQLString*       stubType,
            AQLDate*         firstStubDate,
            AQLDate*         lastStubDate,
            int*            rollDay,
            bool            isStartRoll,
            bool            isEomRoll,
            AQLString*       rollConvention,
            const AQLString& fixingAdvanceOrArrears = AQLString("advance"),
            bool			removeExtraPaymentDay = false);


        /*      @brief Function to get the floag leg Fixing Schedule
        *
        *      @param [in]  accrualDates					The swap leg accrual dates
        *      @param [in]  fixingBusinessDayAdjustment     Floating fixing business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
        *      @param [in]  fixingCalendar					Floating fixing calendar
        *      @param [in]  fixingLag						Float fixing lag
        *	   @param [in]	fixingAdvanceOrArrears			Flag to indicate the fixing is advance or arrears
        *      @param [in]  removeExtraFixingDay			True to remove the extra fixing date. Default to False for backward compatibility, as all the core functions expect fixing dates having the same size as accrual days
        *      @Return	floatFixingDates
        */
        static DateVector getFixingSchedule(const DateVector& accrualDates,
            AQLString  fixingBusinessDayAdjustment,
            AQLString  fixingCalendar,
            AQLString  fixingLag,
            const AQLString& fixingAdvanceOrArrears = AQLString("advance"),
            bool removeExtraFixingDay = false);

        /*
        * @brief	Generate the fixing schedule for a floating leg
        */
        static void generateFixingSchedule(DateVector&     fixingDates,
            AQLDate          effectiveDate,
            AQLDate          maturityDate,
            AQLString        frequency,
            AQLString        accrualRollConvention,
            AQLString        accrualCalendar,
            AQLString        fixingRollConvention,
            AQLString        fixingCalendar,
            AQLString        fixingLag,
            AQLString*       stubType,
            AQLDate*         firstStubDate,
            AQLDate*         lastStubDate,
            int*            rollDay,
            bool            isStartRoll,
            bool            isEomRoll,
            AQLString*       rollConvention,
            const AQLString&	fixingAdvanceOrArrears = AQLString("advance"));

        /* @brief			Update the accrual frequency if it is greater than the payment frequency
        * @param [in]		paymentFreq			Payment freqquency
        * @return True if it is fixing in advance
        */
        static bool isFixingInAdvance(const AQLString& fixingAdvanceOrArrears);

        /*
        * @brief	Calculate stub rate of a defined stub period
        */
        static double getStubRate(AQLDataInstance* dataInstance,
            const DateVector& fixingDates,
            const AQLStringVector& curveNames,
            const AQLStringVector& curveTenors,
            const DoubleVector& tenorCurveFixings,
            const AQLString& curveid,
            const AQLString& stubType,
            const AQLString& interpolation,
            const AQLString& dateCount,
            const AQLString& calendar,
            const AQLString& busDayAdj,
            bool  useNearbyCurve,
            bool  isFwdInter,
            bool  useFwdData,
            const AQLString& toleranceTenor,
            const AQLString& useCurveName);

        /*		This function allows for direct access to the underlying numbers being interpolated		*/
        static std::tuple<std::vector<double>, std::vector<double>> getXY(AQLDataInstance* dataInstance, const AQLString& curveCollection);

    private:

        static void setUpYieldCurveConvention(const AQLString &foreCurveName, const AQLString &inter, const AQLString &dc, const AQLString &sld,
            const AQLString &cal, const AQLString &freq, const AQLString &curveid, LACurvePricingObject &yc);

        static bool getForwardConvention(AQLDataInstance *dataInstance, const AQLString &curveid, const AQLString &curveName, AQLPriceDataDayCount &dc, AQLPriceDataSlidingRule &sld, AQLPriceDataCalendar &cal, AQLString &accessory);

        LACurveForwardRateHelpers(void);
        ~LACurveForwardRateHelpers(void);
        LACurveForwardRateHelpers(const LACurveForwardRateHelpers &rhs);
        LACurveForwardRateHelpers &operator=(const LACurveForwardRateHelpers &rhs);
    };

}
