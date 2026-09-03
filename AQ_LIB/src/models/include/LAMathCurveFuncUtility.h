#ifndef LAMathCurveFuncUtility_h
#define LAMathCurveFuncUtility_h

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#ifndef WINDOWS
#if defined(WIN32) || defined(WIN64)
#define WINDOWS
#endif
#endif



#include <memory>
#include <utility>
#include <vector>
#include <tuple>
#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "LAString.h"
#include "LAMathYieldCurve.h"
#include "ConstantDeclarations.h"
#include "LATime.h"
//////////////////////////

//date method
class LAMathCurveFuncUtility
{
public:
	
	// Normal = Geometric, Average = Arithmetic Averaging, Flat and Simple the same.
	enum COMPOUND_TYPE {COMPOUND_NORMAL, COMPOUND_FLAT, COMPOUND_SIMPLE, AVERAGE};

	static const char* YIELD_CURVE_NAME_PREFIX;

	static const char* YIELD_CURVE_PRO_NAME_PREFIX;

	static double getRate(const LADate&  fromdate, LAString term, LADataInstance* dataInstance, const LAString& curveid, 
						  LAString ratetype, LAString frequency, LAString daycount, LAString slidingrule, 
						  LAString calendar, LAString interpolation, LAString foreCurveName = STD, 
						  LAString dfCurveName = STD, bool isFWDInter = true, const LAString* roll_convention=NULL);

	static double getParRate(DateVector& datevec, LADataInstance* dataInstance, const LAString& curveid, 
						   LAString daycount, LAString interpolation, LAString foreCurveName = STD, 
						   LAString dfCurveName = STD, bool isFWDInter = true);

    // Get the par rate with fixed and floating date schedules and fixing overrides
    //
    // Note: 'floatSpread' is in Basis Points, the optional 'useFwdData' boolean if true will use forward data points directly 
    // ... and if set to false will imply forward rates from discount factors.
    //
    static double getParRate( DateVector& fixedAccrualDates, DateVector& fixedPaymentDates, DateVector& floatFixingDates,
                              DateVector& floatAccrualDates, DateVector& floatPaymentDates, LADataInstance* dataInstance, const LAString& curveid,
                              LAString fixedDaycount, LAString floatDaycount, LAString interpolation, LAString foreCurveName,
                              LAString dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
                              double lastFixing, double floatSpread = 0.0, bool useFwdData = false, bool isOIS = false, LAString oisCompoundingType = "", 
							  LAString calendar = "", LAString rollConvention = "", LAString slidingRule = "" );

    // Get the Swap PV
    static double getSwapPV( bool& isFixedRatePayerSwap, double& notional, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
                             DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
                             LADataInstance* dataInstance, const LAString& curveid, double& fixedRate, LAString fixedDaycount,
                             double& floatSpreadInBasisPoints, LAString floatDaycount, LAString interpolation, LAString foreCurveName,
                             LAString dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
                             double lastFixing, bool useFwdData = false, bool isOIS = false, LAString compoundingMethod = "", LAString floatCalendar = "", 
							 LAString floatRollConv = "", LAString	slidingRule = "");

    // Get the Swap DV01
    static double getSwapDV01( bool isFixedRatePayerSwap, double notional, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
                               DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
                               LADataInstance* dataInstance, const LAString& curveid, double& fixedRate, LAString fixedDaycount,
                               double& floatSpreadInBasisPoints, LAString floatDaycount, LAString interpolation, LAString foreCurveName,
                               LAString dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
                               double lastFixing, bool isOIS = false, LAString compoundingMethod = "", LAString floatCalendar = "", 
							   LAString floatRollConv = "", LAString	slidingRule = "");

    // Get the Swap PV01
    static double getSwapPV01( bool& isFixedRatePayerSwap, double& notional, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
                               LADataInstance* dataInstance, const LAString& curveid, LAString fixedDaycount, LAString interpolation, LAString foreCurveName,
                               LAString dfCurveName );

    // Get the Asset Swap Spread using Par-Par Methodology
    static double getAssetSwapSpread( const double& bondPrice, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates, 
                                      DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
                                      LADataInstance* dataInstance, const LAString& curveid, double& fixedRate, LAString fixedDaycount, 
                                      LAString floatDaycount, LAString interpolation, LAString foreCurveName, LAString dfCurveName,
                                      bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing, double lastFixing,
                                      bool isCleanPrice = true, const LADate& settlementDate = LADate() );

	static double getParRate(LAString term, LADataInstance* dataInstance, const LAString& curveid, LAString frequency, LAString daycount, 
							LAString slidingrule, LAString calendar, LAString interpolation, LAString foreCurveName = STD, 
							LAString dfCurveName = STD, bool isFWDInter = true, const LAString* roll_convention=NULL);

	static double getParRate(const LADate& fromdate, LAString term, LADataInstance* dataInstance, const LAString& curveid, LAString frequency, LAString daycount, 
							 LAString slidingrule, LAString calendar, LAString interpolation, LAString foreCurveName = STD, 
							 LAString dfCurveName = STD, bool isFWDInter = true, const LAString* roll_convention=NULL);

	static double getParRate(LADataInstance* dataInstance, const LAString& curveid, const LADate& fromDate, const LADate& toDate, const LADate* firstStubDate, 
							 const LADate* lastStubDate, const int* pday, LAString freq, LAString daycount, LAString slidingrule, LAString calendar, 
							 LAString interpolation, LAString foreCurveName = STD, LAString dfCurveName = STD, bool isFWDInter = true);

	static double getForwardRate(const LADate& fromdate, double term, LADataInstance* dataInstance, const LAString& curveid, LAString frequency, LAString daycount, 
							LAString slidingrule, LAString calendar, LAString interpolation, LAString curveName = STD, bool isFWDInter = true);

	static DoubleArray getMultiForwardRate(const DateVector& fromdate, double term, LADataInstance* dataInstance, const LAString& curveid, 
							LAString frequency, LAString daycount, LAString slidingrule, LAString calendar,LAString interpolation, 
							LAString curveName = STD, bool isFWDInter = true, bool useFwdData = false );

	static double getForwardRate(const LADate& fromdate, const LADate& todate, LADataInstance* dataInstance, const LAString& curveid, LAString frequency, LAString daycount, 
							LAString slidingrule, LAString calendar, LAString interpolation, LAString curveName = STD, bool isFWDInter = true);

	static DoubleArray getMultiForwardRate(const DateVector& fromdate, const DateVector& todate, LADataInstance* dataInstance, const LAString& curveid,
							LAString frequency, LAString daycount, LAString slidingrule, LAString calendar, LAString interpolation, 
							LAString curveName = STD, bool isFWDInter = true, bool useFwdData = false); // useFwdData: false = imply forwards from DiscountFactors, true = use forward data directly
	
    // Function to get the curve frequency
    static LAString getCurveFrequency( const LAString& curveCollection, const LAString& curveName ); // CurveCollection also known as CurveID

    // Function to get forward rates from discount factors. No todate required since this is implied from the curve frequency
    static DoubleArray getMultiForwardRatesUsingCurveFrequency(const DateVector& fromdate, LAString& curveFrequency, LADataInstance* dataInstance, const LAString& curveid,
							LAString frequency, LAString daycount, LAString slidingrule, LAString calendar, LAString interpolation, 
							LAString curveName = STD, bool isFWDInter = true, bool useFwdData = false); // useFwdData: false = imply forwards from DiscountFactors, true = use forward data directly

	static double getForwardRate(const LADate& fromdate, LAString term, LADataInstance* dataInstance, const LAString& curveid, LAString frequency, LAString daycount, 
							LAString slidingrule, LAString calendar, LAString interpolation, LAString curveName = STD, bool isFWDInter = true);

	static DoubleArray getMultiForwardRate(const DateVector& fromdate, LAString term, LADataInstance* dataInstance, const LAString& curveid, 
							LAString frequency, LAString daycount, LAString slidingrule, LAString calendar, LAString interpolation, 
							LAString curveName = STD, bool isFWDInter = true);

	static double getZeroRate(double term, LADataInstance* dataInstance, const LAString& curveid, LAString frequency, 
							LAString daycount, LAString interpolation, LAString curveName = STD, bool isFWDInter = true);

	static DoubleArray getMultiZeroRate(const DoubleArray& term, LADataInstance* dataInstance, const LAString& curveid, LAString frequency, 
							LAString daycount, LAString interpolation, LAString curveName = STD, bool isFWDInter = true);

	static double getZeroRate(LAString term, LADataInstance* dataInstance, const LAString& curveid, LAString frequency,
							LAString daycount, LAString slidingrule, LAString calendar, LAString interpolation, 
							LAString curveName = STD, bool isFWDInter = true);

	static DoubleArray getMultiZeroRate(LAStringVector term, LADataInstance* dataInstance, const LAString& curveid, LAString frequency,
							LAString daycount, LAString slidingrule, LAString calendar, LAString interpolation, 
							LAString curveName = STD, bool isFWDInter = true);

	static double getDF(const LADate& fromdate, LAString term, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, 
                            LAString slidingrule, LAString calendar, LAString interpolation, bool isbasisflag=false, 
							LAString curveName = STD);

	static DoubleArray getMultiDF(const DateVector& fromdate, LAString term, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, 
                            LAString slidingrule, LAString calendar, LAString interpolation, bool isbasisflag=false, 
							LAString curveName = STD);

	static double getDF(const LADate& fromdate, double term, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, 
						LAString slidingrule, LAString calendar, LAString interpolation, bool isbasisflag=false, 
						LAString curveName = STD);

	static DoubleArray getMultiDF(const DateVector& fromdate, const DoubleArray& term, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, 
							LAString slidingrule, LAString calendar, LAString interpolation, bool isbasisflag=false, 
							LAString curveName = STD);
	
	static double getDF(const LADate& fromdate, const LADate& todate, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, 
							LAString slidingrule, LAString calendar, LAString interpolation, bool isbasisflag=false, 
							LAString curveName = STD);

	static DoubleArray getMultiDF(const DateVector& fromdate, const DateVector& todate, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, 
							LAString slidingrule, LAString calendar, LAString interpolation, bool isbasisflag=false, 
							LAString curveName = STD);
	    
    // Function to get spot discount factors. No fromdate required since this is the curve asof date
    static DoubleArray getMultiSpotDiscountFactors( const DateVector& todate, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, 
							LAString slidingrule, LAString calendar, LAString interpolation, bool isbasisflag=false, 
							LAString curveName = STD);

	static double getDF(LAString term, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, 
							LAString slidingrule, LAString calendar, LAString interpolation, bool isbasisflag=false, 
							LAString curveName = STD);

	static DoubleArray getMultiDF(LAStringVector term, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, 
							LAString slidingrule, LAString calendar, LAString interpolation, bool isbasisflag=false, 
							LAString curveName = STD);

	static double getDF(double term, LADataInstance* dataInstance, const LAString& curvid, LAString daycount, LAString interpolation, bool isbasisflag=false, 
						LAString curveName = STD);

	static DoubleArray getMultiDF(const DoubleArray& term, LADataInstance* dataInstance, const LAString& curvid, LAString daycount, 
							LAString interpolation, bool isbasisflag=false, LAString curveName = STD);
	
	static double getBasisDF(double term, LADataInstance* dataInstance, const LAString& curveid,LAString daycount, LAString interpolation);

	static DoubleArray getMultiBasisDF(const DoubleArray& term, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, LAString interpolation);

	static double getGridRate(LADataInstance* dataInstance, const LAString& curveid, LAString currency, LAString index, LAString grid);

	static double getAnnuity(const LADate& start, const LADate& end, LADataInstance* dataInstance, const LAString& curveid, LAString data_frequency,
							 LAString slidingrule, LAString calendar,	const LADate* firstStubDate, const LADate* lastStubDate,
							 const int* pday, LAString daycount, LAString interpolation, LAString curveName = STD);

 	static double getAnnuity(const DateVector& datevec, LADataInstance* dataInstance, const LAString& curveid, LAString daycount, LAString interpolation, 
							 LAString curveName = STD);

   static double getBasisSwapValue(LADataInstance* dataInstance, const LADate& valueDate, const LADate& startDate, LAString& term, const  LAString& arbFreeCurveID,
                                    const LAString& forecastCurveID, const LAString& discountCurveID, double basis, bool isPrincipal, LAString& frequency, 
									const LAString& daycount, const LAString& slidingrule, const LAString& calendar, double firstFixingRate=0., 
									bool isEOMRoll = false, bool isFRN = false);

	static double getCurBasisSwapValue(LADataInstance* dataInstance, const LADate& valueDate, const LADate& startDate,const LAString& term, const  LAString& arbFreeCurveID,
									   const LAString& discountCurveID, const LAString& dolArbFreeCurveID, const LAString& dolForecastCurveID, 
									   const LAString& dolDiscountCurveID, const LAString& frequency, const LAString& daycount, const LAString& slidingrule, 
									   const LAString& calendar, bool isEOMRoll = false);

	static  LAMathYieldCurve& getYieldCurveForCurveID(LADataInstance* dataInstance, const LAString& curveid);
	static void setCalendarForCurveID(LAMathYieldCurve& yc, const LAString& calendar);
	static bool setUpForwardDayCount(LADataInstance *dataInstance, const LAString &curveid, const LAString &curveName, LAMathYieldCurve &yc);
	static bool setUpForwardDayCount(LADataInstance *dataInstance, const LAString &curveid, const LAString &curveName, LAPriceDataDayCount &dc);

	static double compound(LADataInstance* dataInstance,
                           const LAString& curveID,
                           const LAString& forecastCurveName,
                           const LADate& start_date,
                           const LADate& end_date,
                           const double spread,
                           const LAString& frequency_,
                           const bool is_start_roll,
                           const LAString& roll_convention_,
                           const LAString& calendar_,
                           const LAString& sliding_rule_,
                           const LAString& day_count_,
                           const LAString& interpolation_,
                           const LAString& compound_type_,
                           const LADate* first_odd = NULL,
                           const LADate* last_odd = NULL,
						   const bool usePDay=false);

	//static void clearCalendarForCurveMap(bool isAll, bool isLock);


    //      @brief Function to get the fixed and floating schedules needed for par rate calculations
    //
    //      Pass into this function the following empty DateVector arguments, which are placeholders 
    //      and will populated / overwritten with the correct date schedule results.
    //
    //      @param [in]     LADate          effectiveDate                   The swap start or effective date
    //      @param [in]     LADate          maturityDate                    The swap end or maturity date
    //      @param [in]     LAString        fixedFrequency                  Fixed coupon frequency
    //      @param [in]     LAString        fixedAccrualBusDayAdjustment    Fixed accrual business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    //      @param [in]     LAString        fixedAccrualCalendar            Fixed accrual calendar
    //      @param [in]     LAString        fixedPaymentBusDayAdjustment    Fixed payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    //      @param [in]     LAString        fixedPaymentCalendar            Fixed payment calendar  
    //      @param [in]     LAString        fixedPaymentLag                 Fixed payment lag
    //      @param [in]     LAString*       fixedStubType                   Fixed stub type; ShortStart, ShortEnd, LongStart, LongEnd
    //      @param [in]     LADate*         fixedFirstStubDate              Fixed first stub date
    //      @param [in]     LADate*         fixedLastStubDate               Fixed last stub date
    //      @param [in]     int*            fixedRollDay                    Fixed roll day, an integer representing the day of the month
    //      @param [in]     bool            fixedIsStartRoll,               Are fixed coupons rolling from the start day i.e. ShortEnd stub? True or False
    //      @param [in]     bool            fixedIsEomRoll,                 Are fixed coupons rolling on month end? True or False
    //      @param [in]     LAString*       fixedRollConvention             Fixed roll convention: IMM, EOM, Start, End or NULL
    //      @param [in]     LAString        floatFrequency                  Floating coupon frequency
    //      @param [in]     LAString        floatFixingBusDayAdjustment     Floating payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    //      @param [in]     LAString        floatFixingCalendar             Floating payment calendar 
    //      @param [in]     LAString        floatFixingLag                  Floating fixing lag
    //      @param [in]     LAString        floatAccrualBusDayAdjustment    Floating accrual business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    //      @param [in]     LAString        floatAccrualCalendar            Floating accrual calendar
    //      @param [in]     LAString        floatPaymentBusDayAdjustment    Floating payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    //      @param [in]     LAString        floatPaymentCalendar            Floating payment calendar
    //      @param [in]     LAString        floatPaymentLag                 Floating payment lag
    //      @param [in]     LAString*       floatStubType                   Floating stub type; ShortStart, ShortEnd, LongStart, LongEnd
    //      @param [in]     LADate*         floatFirstStubDate              Floating first stub date
    //      @param [in]     LADate*         floatLastStubDate               Floating last stub date
    //      @param [in]     int*            floatRollDay                    Floating roll day, an integer representing the day of the month
    //      @param [in]     bool            floatIsStartRoll                Are floating coupons rolling from the start day i.e. ShortEnd stub? True or False          
    //      @param [in]     bool            floatIsEomRoll                  Are floating coupons rolling on month end? True or False
    //      @param [in]     LAString*       floatRollConvention             Floating roll convention: IMM, EOM, Start, End or NULL
    //      @param [out]    DateVector      fixedAccrualDates               ( placeholder for the result )
    //      @param [out]    DateVector      fixedPaymentDates               ( placeholder for the result )
    //      @param [out]    DateVector      floatFixingDates                ( placeholder for the result )
    //      @param [out]    DateVector      floatAccrualDates               ( placeholder for the result )
    //      @param [out]    DateVector      floatPaymentDates               ( placeholder for the result )
    //      @param [in]     LAString		fixedPaymentFreq                Fixed payment frequency
    //      @param [in]     LAString		floatPaymentFreq                Float payment frequency
	//      @param [in]	    LAString		fixingAdvanceOrArrears          Flag to indicate the fixing is advance or arrears
    //
    static void generateSwapSchedule( LADate          effectiveDate,
                                      LADate          maturityDate,
                                      LAString        fixedFrequency,
                                      LAString        fixedAccrualBusinessDayAdjustment,
                                      LAString        fixedAccrualCalendar,
                                      LAString        fixedPaymentBusinessDayAdjustment,
                                      LAString        fixedPaymentCalendar,
                                      LAString        fixedPaymentLag,
                                      LAString*       fixedStubType,
                                      LADate*         fixedFirstStubDate,
                                      LADate*         fixedLastStubDate,
                                      int*            fixedRollDay,
                                      bool            fixedIsStartRoll,
                                      bool            fixedIsEomRoll,
                                      LAString*       fixedRollConvention,
                                      LAString        floatFrequency,
                                      LAString        floatFixingBusinessDayAdjustment,
                                      LAString        floatFixingCalendar,
                                      LAString        floatFixingLag,
                                      LAString        floatAccrualBusinessDayAdjustment,
                                      LAString        floatAccrualCalendar,
                                      LAString        floatPaymentBusinessDayAdjustment,
                                      LAString        floatPaymentCalendar,
                                      LAString        floatPaymentLag,
                                      LAString*       floatStubType,
                                      LADate*         floatFirstStubDate,
                                      LADate*         floatLastStubDate,
                                      int*            floatRollDay,
                                      bool            floatIsStartRoll,
                                      bool            floatIsEomRoll,
                                      LAString*       floatRollConvention,
                                      DateVector&     fixedAccrualDates,        // This parameter is a placeholder and will be overwritten with the result dates
                                      DateVector&     fixedPaymentDates,        // This parameter is a placeholder and will be overwritten with the result dates
                                      DateVector&     floatFixingDates,         // This parameter is a placeholder and will be overwritten with the result dates
                                      DateVector&     floatAccrualDates,        // This parameter is a placeholder and will be overwritten with the result dates
                                      DateVector&     floatPaymentDates,      // This parameter is a placeholder and will be overwritten with the result dates
									  LAString		  fixedPaymentFreq,
									  LAString		  floatPaymentFreq,
									  const LAString& fixingAdvanceOrArrears = LAString("advance"),
                                      const bool      isAssetSwap = false,
                                      LADate          issueDate = LADate() );

    //      @brief Function to get the fixed schedule needed for par rate calculations
    //
    //      Pass into this function the following empty DateVector arguments, which are placeholders 
    //      and will populated / overwritten with the correct date schedule results.
    //
    //      @param [in]     LADate          effectiveDate                   The swap start or effective date
    //      @param [in]     LADate          maturityDate                    The swap end or maturity date
    //      @param [in]     LAString        fixedFrequency                  Fixed coupon frequency
    //      @param [in]     LAString        fixedAccrualBusDayAdjustment    Fixed accrual business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    //      @param [in]     LAString        fixedAccrualCalendar            Fixed accrual calendar
    //      @param [in]     LAString        fixedPaymentBusDayAdjustment    Fixed payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    //      @param [in]     LAString        fixedPaymentCalendar            Fixed payment calendar  
    //      @param [in]     LAString        fixedPaymentLag                 Fixed payment lag
    //      @param [in]     LAString*       fixedStubType                   Fixed stub type; ShortStart, ShortEnd, LongStart, LongEnd
    //      @param [in]     LADate*         fixedFirstStubDate              Fixed first stub date
    //      @param [in]     LADate*         fixedLastStubDate               Fixed last stub date
    //      @param [in]     int*            fixedRollDay                    Fixed roll day, an integer representing the day of the month
    //      @param [in]     bool            fixedIsStartRoll,               Are fixed coupons rolling from the start day i.e. ShortEnd stub? True or False
    //      @param [in]     bool            fixedIsEomRoll,                 Are fixed coupons rolling on month end? True or False
    //      @param [in]     LAString*       fixedRollConvention             Fixed roll convention: IMM, EOM, Start, End or NULL
    //      @param [out]    DateVector      fixedAccrualDates               ( placeholder for the result )
    //      @param [out]    DateVector      fixedPaymentDates               ( placeholder for the result )
    //      @param [in]     LAString        fixedPaymentFreq                Fixed payment frequency
    //
    static void generateFixedLegSchedule( LADate          effectiveDate,
                                          LADate          maturityDate,
                                          LAString        fixedFrequency,
                                          LAString        fixedAccrualBusinessDayAdjustment,
                                          LAString        fixedAccrualCalendar,
                                          LAString        fixedPaymentBusinessDayAdjustment,
                                          LAString        fixedPaymentCalendar,
                                          LAString        fixedPaymentLag,
                                          LAString*       fixedStubType,
                                          LADate*         fixedFirstStubDate,
                                          LADate*         fixedLastStubDate,
                                          int*            fixedRollDay,
                                          bool            fixedIsStartRoll,
                                          bool            fixedIsEomRoll,
                                          LAString*       fixedRollConvention,
                                          DateVector&     fixedAccrualDates,
                                          DateVector&     fixedPaymentDates,
										  LAString		  fixedPaymentFreq=LAString());

    //      @brief Function to get the floating schedule needed for par rate calculations
    //
    //      Pass into this function the following empty DateVector arguments, which are placeholders 
    //      and will populated / overwritten with the correct date schedule results.
    //
    //      @param [in]     LADate          effectiveDate                   The swap start or effective date
    //      @param [in]     LADate          maturityDate                    The swap end or maturity date
    //      @param [in]     LAString        floatFrequency                  Floating coupon frequency
    //      @param [in]     LAString        floatFixingBusDayAdjustment     Floating payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    //      @param [in]     LAString        floatFixingCalendar             Floating payment calendar 
    //      @param [in]     LAString        floatFixingLag                  Floating fixing lag
    //      @param [in]     LAString        floatAccrualBusDayAdjustment    Floating accrual business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    //      @param [in]     LAString        floatAccrualCalendar            Floating accrual calendar
    //      @param [in]     LAString        floatPaymentBusDayAdjustment    Floating payment business day adjustment, Modified_Following, Following, Preceding, Modified_Preceding or No_Change
    //      @param [in]     LAString        floatPaymentCalendar            Floating payment calendar
    //      @param [in]     LAString        floatPaymentLag                 Floating payment lag
    //      @param [in]     LAString*       floatStubType                   Floating stub type; ShortStart, ShortEnd, LongStart, LongEnd
    //      @param [in]     LADate*         floatFirstStubDate              Floating first stub date
    //      @param [in]     LADate*         floatLastStubDate               Floating last stub date
    //      @param [in]     int*            floatRollDay                    Floating roll day, an integer representing the day of the month
    //      @param [in]     bool            floatIsStartRoll                Are floating coupons rolling from the start day i.e. ShortEnd stub? True or False          
    //      @param [in]     bool            floatIsEomRoll                  Are floating coupons rolling on month end? True or False
    //      @param [in]     LAString*       floatRollConvention             Floating roll convention: IMM, EOM, Start, End or NULL
    //      @param [out]    DateVector      fixedAccrualDates               ( placeholder for the result )
    //      @param [out]    DateVector      fixedPaymentDates               ( placeholder for the result )
    //      @param [out]    DateVector      floatFixingDates                ( placeholder for the result )
    //      @param [out]    DateVector      floatAccrualDates               ( placeholder for the result )
    //      @param [out]    DateVector      floatPaymentDates               ( placeholder for the result )
    //      @param [in]     LAString		floatPaymentFreq                Float payment frequency
	//      @param [in]	    LAString&		fixingAdvanceOrArrears          Flag to indicate the fixing is advance or arrears
	//      @param [in]     bool			removeExtraDay					True to remove the extra fixing date and payment date. Default to False for backward compatibility, as all the core functions expect fixing dates & payment dates having the same size as accrual days
    //
    static void generateFloatLegSchedule( LADate          effectiveDate,
                                          LADate          maturityDate,
                                          LAString        floatFrequency,
                                          LAString        floatFixingBusinessDayAdjustment,
                                          LAString        floatFixingCalendar,
                                          LAString        floatFixingLag,
                                          LAString        floatAccrualBusinessDayAdjustment,
                                          LAString        floatAccrualCalendar,
                                          LAString        floatPaymentBusinessDayAdjustment,
                                          LAString        floatPaymentCalendar,
                                          LAString        floatPaymentLag,
                                          LAString*       floatStubType,
                                          LADate*         floatFirstStubDate,
                                          LADate*         floatLastStubDate,
                                          int*            floatRollDay,
                                          bool            floatIsStartRoll,
                                          bool            floatIsEomRoll,
                                          LAString*       floatRollConvention,
                                          DateVector&     floatFixingDates,         // This parameter is a placeholder and will be overwritten with the result dates
                                          DateVector&     floatAccrualDates,        // This parameter is a placeholder and will be overwritten with the result dates
                                          DateVector&     floatPaymentDates,       // This parameter is a placeholder and will be overwritten with the result dates
										  LAString		  floatPaymentFreq=LAString(),
										  const LAString& fixingAdvanceOrArrears=LAString("advance"),
										  bool			  removeExtraDay=false);
	
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
												LADate			effectiveDate,
												LADate          maturityDate,
												LAString        accrualFrequency,
												LAString        accrualBusinessDayAdjustment,
												LAString        accrualCalendar,
												LAString		paymentFrequency,
												LAString        paymentBusinessDayAdjustment,
												LAString        paymentCalendar,
												LAString        paymentLag,
												LAString*       stubType,
												LADate*         firstStubDate,
												LADate*         lastStubDate,
												int*            rollDay,
												bool            isStartRoll,
												bool            isEomRoll,
												LAString*       rollConvention,
												const LAString& fixingAdvanceOrArrears=LAString("advance"),
												bool			removeExtraPaymentDay=false);


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
											LAString  fixingBusinessDayAdjustment,
											LAString  fixingCalendar,
											LAString  fixingLag,
											const LAString& fixingAdvanceOrArrears=LAString("advance"),
											bool removeExtraFixingDay=false);

	/*
	* @brief	Generate the fixing schedule for a floating leg
	*/
	static void generateFixingSchedule( DateVector&     fixingDates,
										LADate          effectiveDate,
										LADate          maturityDate,
										LAString        frequency,
										LAString        accrualRollConvention,
										LAString        accrualCalendar,
										LAString        fixingRollConvention,
										LAString        fixingCalendar,
										LAString        fixingLag,
										LAString*       stubType,
										LADate*         firstStubDate,
										LADate*         lastStubDate,
										int*            rollDay,
										bool            isStartRoll,
										bool            isEomRoll,
										LAString*       rollConvention,
										const LAString&	fixingAdvanceOrArrears=LAString("advance"));

    /* @brief			Update the accrual frequency if it is greater than the payment frequency
    * @param [in]		paymentFreq			Payment freqquency
    * @return True if it is fixing in advance
    */
    static bool isFixingInAdvance(const LAString& fixingAdvanceOrArrears);

	/*
	* @brief	Calculate stub rate of a defined stub period
	*/
	static double getStubRate( LADataInstance* dataInstance,
									const DateVector& fixingDates,
									const LAStringVector& curveNames,
									const LAStringVector& curveTenors,
									const DoubleVector& tenorCurveFixings,
									const LAString& curveid,									
									const LAString& stubType,
									const LAString& interpolation,
									const LAString& dateCount,
									const LAString& calendar,
									const LAString& busDayAdj,
									bool  useNearbyCurve,									
									bool  isFwdInter,
									bool  useFwdData,
									const LAString& toleranceTenor,
									const LAString& useCurveName);

	/*		This function allows for direct access to the underlying numbers being interpolated		*/
	static std::tuple<std::vector<double>,std::vector<double>> LAMathCurveFuncUtility::getXY(LADataInstance* dataInstance, const LAString& curveCollection);

private:

	static void setUpYieldCurveConvention(const LAString &foreCurveName, const LAString &inter, const LAString &dc, const LAString &sld, 
													const LAString &cal, const LAString &freq, const LAString &curveid, LAMathYieldCurve &yc);

	static bool getForwardConvention(LADataInstance *dataInstance, const LAString &curveid, const LAString &curveName, LAPriceDataDayCount &dc, LAPriceDataSlidingRule &sld, LAPriceDataCalendar &cal, LAString &accessory);

	LAMathCurveFuncUtility(void);
	~LAMathCurveFuncUtility(void);
	LAMathCurveFuncUtility(const LAMathCurveFuncUtility &rhs);
	LAMathCurveFuncUtility &operator=(const LAMathCurveFuncUtility &rhs);

};
#endif 
