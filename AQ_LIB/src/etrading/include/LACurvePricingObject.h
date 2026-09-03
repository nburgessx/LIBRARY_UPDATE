//
//  LACurvePricingObject.h
// This file used to be called LAMathYie1dCurve.h
#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LAObject.h"
#include "LACoreAutoPtr.h"
#include "LAPriceDataConvention.h"
#include "LAFunctionBase.h"
#include "LAModelDynamicsCurve.h"
 
// TODO: Remove these defines; this is bad practice!!!

#define ENTITY_IRYIELDCURVE  17

#ifndef IR_CALIBRATION_DATA_DAYCOUNT
#define IR_CALIBRATION_DATA_DAYCOUNT			"DayCount"			// Data Name of DayCount
#endif
#ifndef	IR_CALIBRATION_DATA_DAYCOUNT_FLOAT	
#define	IR_CALIBRATION_DATA_DAYCOUNT_FLOAT	"DayCountFloat"
#endif
#ifndef IR_CALIBRATION_DATA_ISFUTUREUSE
#define IR_CALIBRATION_DATA_ISFUTUREUSE		"IsFutureUse"		// Data Name of IsFutureUse
#endif
#ifndef IR_CALIBRATION_DATA_CURRENCY
#define IR_CALIBRATION_DATA_CURRENCY			"Currency"			// Data Name of Currency
#endif
#ifndef IR_CALIBRATION_DATA_INTERPOLATIONYG
#define IR_CALIBRATION_DATA_INTERPOLATIONYG	"YieldGenInterpolation"	// Data Name of YieldGenInterpolation
#endif
#ifndef IR_CALIBRATION_DATA_INTERPOLATIONFW
#define IR_CALIBRATION_DATA_INTERPOLATIONFW	"FutureInterpolation"	// Data Name of FutureInterpolation
#endif
#ifndef IR_CALIBRATION_DATA_FREQUENCY
#define IR_CALIBRATION_DATA_FREQUENCY			"Frequency"			// Data Name of Frequency
#endif
#ifndef IR_CALIBRATION_DATA_FREQUENCY_FLOAT
#define IR_CALIBRATION_DATA_FREQUENCY_FLOAT	"FrequencyFloat"			// Data Name of Frequency In Floating Leg
#endif
#ifndef IR_CALIBRATION_DATA_FREQUENCY_COMPOUND
#define IR_CALIBRATION_DATA_FREQUENCY_COMPOUND	"FrequencyCompound"			// Data Name of Frequency for Compounding
#endif
#ifndef IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT
#define IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT	"BaseFrequencyFloat"			// Data Name of Base Frequency In Floating Leg
#endif
#ifndef IR_CALIBRATION_DATA_BASEFREQUENCY_FIX
#define IR_CALIBRATION_DATA_BASEFREQUENCY_FIX	"BaseFrequencyFix"			// Data Name of Base Frequency In Fixed Leg
#endif
#ifndef IR_CALIBRATION_DATA_YIELDDATA
#define IR_CALIBRATION_DATA_YIELDDATA			"YieldData"			// Data Name of YieldData
#endif
#ifndef IR_CALIBRATION_DATA_FORWARDRATES
#define IR_CALIBRATION_DATA_FORWARDRATES		"ForwardRates"	// Data Name of ForwardRates
#endif
#ifndef IR_CALIBRATION_DATA_DFS
#define IR_CALIBRATION_DATA_DFS				"DiscountFactors"	// Data Name of DiscountFactors
#endif
#ifndef IR_CALIBRATION_DATA_DFS2
#define IR_CALIBRATION_DATA_DFS2				"DiscountFactors2"	// Data Name of DiscountFactors2
#endif
#ifndef IR_CALIBRATION_DATA_DFSVALUETOINTERPOLATE
#define IR_CALIBRATION_DATA_DFSVALUETOINTERPOLATE		"DFValueToIntepolate"	// Data Name of DFValueToIntepolate
#endif
#ifndef IR_CALIBRATION_DATA_DFS2VALUETOINTERPOLATE
#define IR_CALIBRATION_DATA_DFS2VALUETOINTERPOLATE	"DF2ValueToIntepolate"	// Data Name of DF2ValueToIntepolate
#endif
#ifndef IR_CALIBRATION_DATA_FIXINGCALENDAR
#define IR_CALIBRATION_DATA_FIXINGCALENDAR	 "FixingCalendar"		//  data name of FixingCalendar
#endif
#ifndef IR_CALIBRATION_DATA_PAYMENTCALENDAR
#define IR_CALIBRATION_DATA_PAYMENTCALENDAR	 "PaymentCalendar"		//  data name of PaymentCalendar
#endif
#ifndef IR_CALIBRATION_DATA_ISFRAUSE
#define IR_CALIBRATION_DATA_ISFRAUSE		"IsFRAUse"		// Data Name of IsFutureUse
#endif
#ifndef IR_CALIBRATION_DATA_ACCESSARY
#define IR_CALIBRATION_DATA_ACCESSARY			"Accessary"			// Data Name of Accessary
#endif
#ifndef IR_CALIBRATION_DATA_LOBASISNAME
#define IR_CALIBRATION_DATA_LOBASISNAME		"LOBasisName"		// Data Name of Libor-OIS Basis Swap Market Name
#endif
#ifndef IR_CALIBRATION_DATA_INVERSE_ENGINE_JACOBIAN
#define IR_CALIBRATION_DATA_INVERSE_ENGINE_JACOBIAN		"InverseEngineJacobian"		// Inversed engine Jacobian gradient at the final solution
#endif
#ifndef IR_CALIBRATION_DATA_ENGINE_JACOBIAN
#define IR_CALIBRATION_DATA_ENGINE_JACOBIAN		"EngineJacobian"		// Engine Jacobian gradient at the final solution
#endif
#ifndef IR_CALIBRATION_DATA_ENGINE_JACOBIAN_LABEL
#define IR_CALIBRATION_DATA_ENGINE_JACOBIAN_LABEL		"EngineJacobianLabel"		// Jacobian matrix labels at the final solution
#endif
#ifndef IR_CALIBRATION_DATA_INVERSE_CURVE_JACOBIAN
#define IR_CALIBRATION_DATA_INVERSE_CURVE_JACOBIAN		"InverseCurveJacobian"		// Inversed curve Jacobian gradient at the final solution
#endif
#ifndef IR_CALIBRATION_DATA_CURVE_JACOBIAN
#define IR_CALIBRATION_DATA_CURVE_JACOBIAN		"CurveJacobian"		// Curve Jacobian gradient at the final solution
#endif
#ifndef IR_CALIBRATION_DATA_JACOBIAN_STATE_VARIABLES
#define IR_CALIBRATION_DATA_JACOBIAN_STATE_VARIABLES	"JacobianStateVariables"		// Curve solver state variables at the final solution
#endif
#ifndef IR_CALIBRATION_DATA_BUILTBYCURVEENGINE
#define IR_CALIBRATION_DATA_BUILTBYCURVEENGINE	"BuiltByCurveEngine"		// Was curve built by curve engine?
#endif


#define	EPS		1.0e-5	// rate eps


//curve type
#ifndef OIS
#define OIS	"OIS"
#endif
#ifndef STD
#define STD	"STD"
#endif
#ifndef DUMMY
#define DUMMY	"DUMMY"
#endif
#ifndef FIXEDRATE
#define FIXEDRATE	"FIXEDRATE"
#endif

//frequency
#ifndef SIMPLE
#define SIMPLE		"SIMPLE"
#endif
#ifndef CONTINUOUS
#define CONTINUOUS	"CONTINUOUS"
#endif
#ifndef ANNUAL
#define ANNUAL		"ANNUAL"
#endif
#ifndef SEMI_ANNUAL
#define SEMI_ANNUAL	"SEMI-ANNUAL"
#endif
#ifndef QUARTERLY
#define QUARTERLY	"QUARTERLY"
#endif
#ifndef MONTHLY
#define MONTHLY		"MONTHLY"
#endif
#ifndef LUNAR
#define LUNAR		"LUNAR"
#endif

// roll convention
#ifndef ROLLCONV_NORMAL 
#define ROLLCONV_NORMAL "NORMAL"
#endif
#ifndef ROLLCONV_EOM 
#define ROLLCONV_EOM "EOM"
#endif
#ifndef ROLLCONV_LUNAR 
#define ROLLCONV_LUNAR "LUNAR"
#endif

// Default values
#ifndef DEF_VAL_NO_CHANGE
#define DEF_VAL_NO_CHANGE "NO_CHANGE"
#endif
#ifndef DEF_VAL_BLANK
#define DEF_VAL_BLANK ""
#endif

#ifndef IR_NO_DATA
#define IR_NO_DATA "NO_DATA"
#endif

class LADataInstance;
class LADate;
class LADataDoubles;
class LADataDate;
class LAInterpolationBase;
class LACoreProcedure;
class LADataString;
class LADataStrings;
class LAPriceDataCalendar;
class LAPriceDataInterpolation;
class LADataMultiReference;
class LADataReference;
class LAPriceDataSlidingRule;

class LADataBool;


namespace etrading
{

    // This sub class is in this file under etrading namespace
    class LAMathPathYieldCurve;

    //=================== YIELD CURVE ==================================
    class LACurvePricingObject : public LAObject
    {
    public:
        
        // LIFECYCLE
        //////////////////////////////////////////////////////////////////////

		// constructor
		LACurvePricingObject();

        // constructor
        LACurvePricingObject(LADataInstance* dataInstance, const LAString *const pCurveType = 0);
        
        // copy constructor
        LACurvePricingObject(const LACurvePricingObject& curve);
        
        // destructor
        virtual ~LACurvePricingObject();

        //  QUERY
        //////////////////////////////////////////////////////////////////////

        // get EntityType
        virtual object_t	getType(void) const;
        
        // this object have specified object or not
        virtual bool		isTypeOf(object_t id) const;
        
        //	get name
        const LADataString&	getName() const;
        
        //	get name
        LADataString&		getName();
        
        //	get Interpolation
        const LAPriceDataInterpolation&  getInterpolation() const;
        
        //	get Interpolation
        LAPriceDataInterpolation& getInterpolation();
        
        //	get Interpolation
        const LAPriceDataInterpolation& getInterpolation(const LAString &curveType) const;
        
        //	get Interpolation
        LAPriceDataInterpolation& getInterpolation(const LAString &curveType);

        // get DayCountConvension
        const LAPriceDataDayCount& getDayCount(void) const;
        
        // get DayCountConvension
        LAPriceDataDayCount& getDayCount(void);
        
        // get YieldData
        const LADataReference& getYieldData() const;
        
        // get YieldData
        LADataReference& getYieldData();
        
        // get Calendar
        const LAPriceDataCalendar& getCalendar() const;
        
        // get Calendar
        LAPriceDataCalendar& getCalendar();
        
        // get SlidingRule
        const LAPriceDataSlidingRule& getSlidingRule() const;
        
        // get SlidingRule
        LAPriceDataSlidingRule& getSlidingRule();
        
        // get Frequency
        const LADataString& getFrequency() const;
        
        // get Frequency
        LADataString& getFrequency();

        // get forward rates, day count fractions, DFs at starts, DFs at ends as tuple
        void analyzeLeg(const DateVector& inDates,
                        const LAString& inForeCurveName,
                        const LAString& inDFCurveName,
                        bool isFWDInter,
                        DoubleArray& rates,
                        DoubleArray& terms,
                        DoubleArray& dfs_start,
                        DoubleArray& dfs_end);

        //	get ZeroRate from fromDate to toDate
        double				getZeroRate(const LADate& fromDate, const LADate& toDate, bool isFWDInter = true, bool useFwdData = false) const; // useFwdData defaults to false for backward compatibility purposes only
        
        //	get BasisZeroRate from fromDate to toDate
        double				getBasisZeroRate(const LADate& fromDate, const LADate& toDate) const;
        //	get ZeroRate for specified term from fromDate 
        
        double				getZeroRate(const LADate& fromDate, const double term, bool isFWDInter = true, bool useFwdData = false) const; // useFwdData defaults to false for backward compatibility purposes only
        //	get BasisZeroRate for specified term from fromDate 
        
        double				getBasisZeroRate(const LADate& fromDate, const double term) const;
        //	get ZeroRate for specified term from AsOfDate 
        
        double				getZeroRate(const double term, bool isFWDInter = true) const;
        //	get BasisZeroRate for specified term from AsOfDate 
       
        double				getBasisZeroRate(const double term) const;
        //	get ZeroRate for specified term from fromDate 
        
        double				getZeroRate(const LADate& fromDate, const LAString& term_str, bool isFWDInter = true) const;
        //	get BasisZeroRate for specified term from fromDate 
        
        double				getBasisZeroRate(const LADate& fromDate, const LAString& term_str) const;
        //	get ZeroRate for specified term from AsOfDate 
        
        double				getZeroRate(const LAString& term_str, bool isFWDInter = true) const;
        
        //	get BasisZeroRate for specified term from AsOfDate 
        double				getBasisZeroRate(const LAString& term_str) const;
        
        //	get Annuity from DateVector 
        double				getAnnuity(const DateVector& Dates) const;

        //	get Annuity from AccrualDates, PaymentDates and Daycount fraction, optional deductAccruedInterest and 
        // isCleanPrice parameters are for bond / asset swap calculations
        double				getAnnuity(const DateVector& accrualDates,
                                       const DateVector& paymentDates,
                                       const LAPriceDataDayCount& daycount,
                                       const LADate settlementDate = LADate(),
                                       const bool deductAccruedInterest = false,
                                       const bool isCleanPrice = true) const;

        //	get ParRate for specified term from AsOfDate 
        double				getParRate(const LAString& term_str, LAString foreCurveName = STD, LAString dfCurveName = STD, bool isFWDInter = true, const LAString* roll_convention = NULL);

        //	get ParRate from DateVector 
        double				getParRate(const DateVector& dates, const DateVector& dates_float, LAString foreCurveName = STD, LAString dfCurveName = STD, bool isFWDInter = true);

        //	get ParRate from DateVector with First and Last Fixing Overrides
        double				getParRate(const DateVector&   fixedAccrualDates,
                                       const DateVector&   fixedPaymentDates,
                                       const DateVector&   floatFixingDates,
                                       const DateVector&   floatAccrualDates,
                                       const DateVector&   floatPaymentDates,
                                       bool                useFirstFixing,
                                       double              firstFixing,
                                       bool                useLastFixing,
                                       double              lastFixing,
                                       LAString            fixedDaycount,
                                       LAString            floatDaycount,
                                       LAString            foreCurveName = STD,
                                       LAString            dfCurveName = STD,
                                       bool                isFWDInter = true,
                                       double              floatSpread = 0.0, // Floating Libor Spread in Basis Points
                                       bool                useFwdData = false, // UseFwdData: False (default) imply forwards from discount factors, True = Use forward data directly ... set to flase for backward compatibility
                                       bool				isOIS = false,
                                       LAString			oisCompoundingType = "",
                                       LAString			calendar = "",
                                       LAString			interpolation = "",
                                       LAString			rollConvention = "",
                                       LAString			slidingRule = "");


        //	get Par Spread of compound swaps from DateVector with First and Last Fixing Overrides
        double				getParCompoundSpread(const DateVector& dates,
                                                 const DateVector& dates_Cmp,
                                                 LAString foreCurveName,
                                                 LAString foreCurveName_Cmp,
                                                 LAString dfCurveName,
                                                 bool isStraight,
                                                 bool isFWDInter);

        //	get Par Basis of CCSs from DateVector with First and Last Fixing Overrides
        double				getParBasis(const DateVector& dates,
                                        const DateVector& dates_USD,
                                        LAString modifiedDiscountName,
                                        LAString foreCurveName,
                                        LAString foreCurveName_USD,
                                        LAString dfCurveName_USD,
                                        bool isMarkToMarket,
                                        bool isFWDInter);

        //	get SwapPV from DateVector with First and Last Fixing Overrides
        double				getSwapPV(const bool&          isFixedRatePayerSwap,
                                      const double&        notional,
                                      const double&        fixedRate,
                                      const double&        floatSpreadInBasisPoints,
                                      const DateVector&    fixedAccrualDates,
                                      const DateVector&    fixedPaymentDates,
                                      const DateVector&    floatFixingDates,
                                      const DateVector&    floatAccrualDates,
                                      const DateVector&    floatPaymentDates,
                                      bool                 useFirstFixing,
                                      double               firstFixing,
                                      bool                 useLastFixing,
                                      double               lastFixing,
                                      LAString             fixedDaycount,
                                      LAString             floatDaycount,
                                      LAString             foreCurveName = STD,
                                      LAString             dfCurveName = STD,
                                      bool                 isFWDInter = true,
                                      bool                 useFwdData = false, // UseFwdData: False (default) imply forwards from discount factors, True = Use forward data directly ... set to flase for backward compatibility
                                      bool				   isOIS = false,
                                      LAString			   oisCompoundingType = "",
                                      LAString			   calendar = "",
                                      LAString			   inputInterpolation = "",
                                      LAString			   rollConvention = "",
                                      LAString			   slidingRule = "");

        //	get SwapDuration from DateVector with First and Last Fixing Overrides
        double				getSwapDuration(bool                 isModifiedDuration,
                                            bool                 isFixedRatePayerSwap,
                                            double               notional,
                                            double               fixedRate,
                                            double               floatSpreadInBasisPoints,
                                            const DateVector&    fixedAccrualDates,
                                            const DateVector&    fixedPaymentDates,
                                            const DateVector&    floatFixingDates,
                                            const DateVector&    floatAccrualDates,
                                            const DateVector&    floatPaymentDates,
                                            bool                 useFirstFixing,
                                            double               firstFixing,
                                            bool                 useLastFixing,
                                            double               lastFixing,
                                            LAString             fixedDaycount,
                                            LAString             floatDaycount,
                                            LAString             foreCurveName = STD,
                                            LAString             dfCurveName = STD,
                                            bool                 isFWDInter = true,
                                            bool				  isOIS = false,
                                            LAString			  oisCompoundingType = "",
                                            LAString			  calendar = "",
                                            LAString			  inputInterpolation = "",
                                            LAString			  rollConvention = "",
                                            LAString			  slidingRule = "");

        //	get SwapDV01 from DateVector with First and Last Fixing Overrides
        double				getSwapDV01(bool                 isFixedRatePayerSwap,
                                        double               notional,
                                        double               fixedRate,
                                        double               floatSpreadInBasisPoints,
                                        const DateVector&    fixedAccrualDates,
                                        const DateVector&    fixedPaymentDates,
                                        const DateVector&    floatFixingDates,
                                        const DateVector&    floatAccrualDates,
                                        const DateVector&    floatPaymentDates,
                                        bool                 useFirstFixing,
                                        double               firstFixing,
                                        bool                 useLastFixing,
                                        double               lastFixing,
                                        LAString             fixedDaycount,
                                        LAString             floatDaycount,
                                        LAString             foreCurveName = STD,
                                        LAString             dfCurveName = STD,
                                        bool                 isFWDInter = true,
                                        bool				  isOIS = false,
                                        LAString			  oisCompoundingType = "",
                                        LAString			  calendar = "",
                                        LAString			  inputInterpolation = "",
                                        LAString			  rollConvention = "",
                                        LAString			  slidingRule = "");

        //	get SwapPV01 from DateVector with First and Last Fixing Overrides
        double				getSwapPV01(const bool&        isFixedRatePayerSwap,
                                        const double&      notional,
                                        const DateVector&  fixedAccrualDates,
                                        const DateVector&  fixedPaymentDates,
                                        LAString           fixedDaycount,
                                        LAString           foreCurveName = STD,
                                        LAString           dfCurveName = STD);

        //	get SwapPV01 from DateVector with First and Last Fixing Overrides
        double	     getAssetSwapSpread(const double& bondPrice,
                                        const double& fixedRate,
                                        const DateVector& fixedAccrualDates,
                                        const DateVector& fixedPaymentDates,
                                        const DateVector& floatFixingDates,
                                        const DateVector& floatAccrualDates,
                                        const DateVector& floatPaymentDates,
                                        bool useFirstFixing,
                                        double firstFixing,
                                        bool useLastFixing,
                                        double lastFixing,
                                        LAString fixedDaycount,
                                        LAString floatDaycount,
                                        LAString foreCurveName = STD,
                                        LAString dfCurveName = STD,
                                        bool isFWDInter = true,
                                        bool isCleanPrice = true,
                                        const LADate& settlementDate = LADate());

        //	get ParRate by DateCalcUtility
        double				getParRate(const LADate& fromDate, const LADate& toDate,
                                       const LADate* firstStubDate = NULL,
                                       const LADate* lastStubDate = NULL,
                                       const int* pday = NULL, LAString foreCurveName = STD,
                                       LAString dfCurveName = STD, bool isFWDInter = true,
                                       const LAString* roll_convention = NULL);
        
        // get ParRate by DateCalcUtility
        double				getParRate(const LADate& fromDate, const LAString& term_str,
                                       const LADate* firstStubDate = NULL,
                                       const LADate* lastStubDate = NULL,
                                       const int* pday = NULL, LAString foreCurveName = STD,
                                       LAString dfCurveName = STD, bool isFWDInter = true,
                                       const LAString* roll_convention = NULL);

        // Calculate stub rate of a defined stub period
        double				getStubRate(const DateVector& fixingDates,
                                        const LAStringVector& curveNames,
                                        const LAStringVector& curveTenors,
                                        const DoubleVector& tenorCurveFixings,
                                        const LAString& curveid,
                                        const LAString& stubType,
                                        const LAString& dateCount,
                                        const LAString& calendar,
                                        const LAString& busDayAdj,
                                        bool  useNearbyCurve,
                                        bool  useGivenFixings,
                                        bool  isFwdInter,
                                        bool  useFwdData,
                                        const LAString& toleranceTenor,
                                        const LAString& useCurveName);

		/* @brief		Calculate the curve discount factor between fromDate and toDate.
		*  @param[in]	fromDate	The date from which to start discounting
		*  @param[in]	toDate		Calculate the discount factor to this date
		*  @returns	The calculated discount factor.
		*/
        double				getDF(const LADate& fromDate, const LADate& toDate) const;

		/* @brief		Calculate the basis curve discount factor between fromDate and toDate.
		*  @param[in]	fromDate	The date from which to start discounting
		*  @param[in]	toDate		Calculate the discount factor to this date
		*  @returns	The calculated discount factor.
		*/
        double				getBasisDF(const LADate& fromDate, const LADate& toDate) const;
        
        //	get DF for specified term from fromDate 
        double				getDF(const LADate& fromDate, const double& term) const;
        
        //	get BasisDF for specified term from fromDate 
        double				getBasisDF(const LADate& fromDate, const double& term) const;
        
        //	get DF for specified term from AsOfDate 
        //	double				getDF(const double term) const;
        //	get DF for specified term from AsOfDate 
        
        double				getDF(const double term, const DayCount* pdc = NULL) const;
        
        //	get BasisDF for specified term from AsOfDate 
        double				getBasisDF(const double term, const DayCount* pdc = NULL) const;

        //	get DF for specified term from fromDate 
        //double			getDF(const LADate& fromDate, const LAString& term_str) const;

        //	get DF for specified term from fromDate 
        double				getDF(const LADate& fromDate, const LAString& term_str, LAString bdc = DEF_VAL_NO_CHANGE, LAString cal = DEF_VAL_BLANK) const; // Optional Fields required for a bug fix
        
        //	get BasisDF for specified term from fromDate 
        double				getBasisDF(const LADate& fromDate, const LAString& term_str) const;
        
        //	get DF for specified term from AsOfDate 
        double				getDF(const LAString& fromDate, LAString bdc = DEF_VAL_NO_CHANGE, LAString cal = DEF_VAL_BLANK) const; // Optional Fields required for a bug fix
        
        //	get BasisDF for specified term from AsOfDate 
        double				getBasisDF(const LAString& term_str) const;
        
        //	get raw DFs and terms
        void				getRawDFs(DoubleArray& terms, DoubleArray& dfs) const;

        const LAInterpolationBase   &getDFInterpolation(const LAString *pCurveType = 0) const;
        const LAInterpolationBase   &getForwardsInterpolation(const LAString *pCurveType = 0) const;
        const LAInterpolationBase   &getBasisDFInterpolation() const;

        //	clone this class
        LAObject*			clone() const;

        double				getCurve(const double term) const;
        
        LAMathPathYieldCurve	getCurve(double t, DayCount dc) const;

        //  OPERATION 

            //	set Interpolation
        void				setInterpolation(LAInterpolationBase* a,
            const LAString& name);
        // set Interpolation
        void				setInterpolation(const LAString& name);


        // remove specified Data. If Data is nothing, do nothing.
        virtual void        remove(const LAString& dataName);
        
        // Initialize this Object.
        virtual void		reset(void);
        
        //	Note: this is curveIndex
        void				setCurveType(const LAString &curveType);
        
        //	 
        void				setBasisCurveType(const LAString &basisCurveType);
        
        // !
        void				setCurveNameAndInterpolation(const LAString &curveName, const LAString &interpolation);

		//	Note: this is curveIndex
		LAString			getCurveType(void) const;

        static const RateConvention setRC(LAString Freq);

        // get DFInterpolationMethod
        const LADataString& getDFInterpolationMethod() const;
        
        // get DFInterpolationMethod
        LADataString& getDFInterpolationMethod();
        
        // get DF2InterpolationMethod
        const LADataString& getDF2InterpolationMethod() const;
        
        // get DF2InterpolationMethod
        LADataString& getDF2InterpolationMethod();

        static unsigned int setSpanFromFrequency(const LAString& freq);

        double getBasisSwapValue(const LADate& valueDate, const LADate& startDate, const LAString& term, double basis,
            bool isPrincipal, const LAString& frequency, const LAPriceDataDayCount& daycount,
            const LAString& forecastCurveID, const LAString& discountCurveID, double firstFixingRate,
            bool isEOMRoll, bool isFRN);

        double getCurBasisSwapValue(const LACurvePricingObject& yc, const LADate& valueDate, const LADate& startDate,
            const LAString& term, const LAString& frequency, const LAPriceDataDayCount& daycount,
            const LAString& forecastCurveID, const LAString& discountCurveID, bool isEOMRoll,
            double firstFixingAmount = 1., double firstFixingRate = 0.);

        // get base forward rate
        void getBaseForwardRate(const LAString &curveType, DoubleArray &terms, DoubleMatrix &termsMat, DoubleArray &taus, DoubleArray &rates, double &joinDateAsDouble, std::vector<DateVector> *pDateMat_out = 0, std::vector<DateVector> *pDateMat_in = 0, const bool & useForwardData = false) const;

        // get base forward rate
        void getBaseForwardRate(const LAString &curveType, DoubleArray &terms, DoubleMatrix &termsMat, DoubleArray &taus, DoubleArray &rates, std::vector<DateVector> *pDateMat_out = 0, std::vector<DateVector> *pDateMat_in = 0, const bool & useForwardData = false) const;

        // get daycount adjusted forward rate
        void getAdjForwardRate(const LAString &curveType, const LAPriceDataDayCount &dc, DoubleArray &terms, DoubleMatrix &termsMat, DoubleArray &taus, DoubleArray &rates, std::vector<DateVector> *pDateMat_in = 0) const;

        // get forward interpolation
        const LAInterpolationBase   &getFWDInterpolation(const LAString *pCurveType = 0, const bool & useFwdData = false) const;

        // get DayCountConvension
        const LAPriceDataDayCount& getDayCount(const LAString &curveType) const;
        
        // get DayCountConvension
        LAPriceDataDayCount&		getDayCount(const LAString &curveType);
        
        // get DayCountConvension from yield data object
        const LAPriceDataDayCount&
            getDayCountFromYieldData(const LAString &curveType) const;

        // get convension from yield curve data object
        void				getCurveConvention(LAString& freq, LAPriceDataCalendar& cal, LAPriceDataSlidingRule& sld, LAPriceDataDayCount& dc, LAString& accessary, const LAString& curveName) const;

        // get convension from yield curve data object
        void				getCurveConvention(LAString& freq, LAPriceDataCalendar& cal, LAPriceDataSlidingRule& sld, LAPriceDataDayCount& dc, LAString& accessary, const LAString& curveName);

        virtual bool operator==(const LACurvePricingObject& a) const;

        /* @brief			Function to calculate the swap price and risk. This function has been written to centralize the calculation of these
        *                   parameters and remove the previous multiple duplication of swap calculations.
        *
        *  @param [in]      bool                  isFixedRatePayerSwap; True = Payer Swap, False = Receiver Swap
        *  @param [in]      double                notional
        *  @param [in]      double                fixedRate
        *  @param [in]      double                floatSpreadInBasisPoints
        *  @param [in]      DateVector            fixedAccrualDates
        *  @param [in]      DateVector            fixedPaymentDates
        *  @param [in]      DateVector            floatFixingDates
        *  @param [in]      DateVector            floatAccrualDates
        *  @param [in]      DateVector            floatPaymentDates
        *  @param [in]      bool                  useFirstFixing
        *  @param [in]      double                firstFixing
        *  @param [in]      bool                  useLastFixing
        *  @param [in]      double                lastFixing
        *  @param [in]      LAString              fixedDaycount
        *  @param [in]      LAString              floatDaycount
        *  @param [in]      LAString              foreCurveName; forecast curve name, defaults to STD
        *  @param [in]      LAString              dfCurveName; discount curve name, defaults OIS
        *  @param [in]      bool                  isFWDInter, This flag controls how forwards are calculated; True = interpolate forwards, False = imply forwards from discount factors
        *  @param [in]		bool				  isOIS, Is the forecasting curve an OIS Curve?: True or False
        *  @param [in]		LAString			  oisCompoundingType, Ois Curve Compounding Method
        *  @param [in]		LAString			  calendar
        *  @param [in]		LAString			  interpolation
        *  @param [in]		LAString			  rollConvention
        *  @param [in]		LAString			  slidingRule
        *
        */
        void calculateSwapPriceAndRisk(const bool             isFixedRatePayerSwap,
                                       const double           notional,
                                       const double           fixedRate,
                                       const double           floatSpreadInBasisPoints,
                                       const DateVector &     fixedAccrualDates,
                                       const DateVector &     fixedPaymentDates,
                                       const DateVector &     floatFixingDates,
                                       const DateVector &     floatAccrualDates,
                                       const DateVector &     floatPaymentDates,
                                       const bool             useFirstFixing,
                                       const double           firstFixing,
                                       const bool             useLastFixing,
                                       const double           lastFixing,
                                       const LAString &       fixedDaycount,
                                       const LAString &       floatDaycount,
                                       const LAString &       foreCurveName = STD,
                                       const LAString &       dfCurveName = OIS,
                                       const bool             isFWDInter = true,
                                       const bool		       isOIS = false,
                                       const LAString &	   oisCompoundingType = "",
                                       const LAString &	   calendar = "",
                                       const LAString &	   inputInterpolation = "",
                                       const LAString &	   rollConvention = "",
                                       const LAString &	   slidingRule = "");

        // Remove data of a given curve
        void deleteCurveDataByCurveName(const LAString& curveName);

    protected:
        
        // copy object		 
        virtual LAObject&	copy(const LAObject& e);
        
        // delete DF interpolations
        void delDFInterpolations();
        
        // delete FWD interpolations
        void delFWDInterpolations();
        
        // delete daycount
        void delDayCounts();

        // delete DF interpolations of the given curve
        void delDFInterpolationsByCurve(const LAString& curve);
        
        // delete FWD interpolations of the given curve
        void delFWDInterpolationsByCurve(const LAString& curve);
        
        // delete daycount of the given curve
        void delDayCountsByCurve(const LAString& curve);

        // setUpInterpolation
        void setUpInterpolation() const;
        
        // setValuesToInterpolator
        void setValuesToInterpolator(const LAString& curveType, const DoubleArray &index, const DoubleArray &value) const;
        
        // setValuesToInterpolator
        void setValuesToInterpolator(LAPriceDataInterpolation* pInter, const LAString& curveType, const DoubleArray &index, const DoubleArray &value) const;
        
        //	get CurveTypeInterpolation
        LAPriceDataInterpolation& getCurveTypeInterpolation(const LAString &curveType) const;

        //	add DF interpolation
        void addDFInterpolation(const LAString &curveType) const;

        //	add FWD interpolation
        void addFWDInterpolation(const LAString &curveType) const;
        
        // get yield data version
        int	getYieldDataVersion(void) const;

        LAString	mCurveSuffix;		// CurveAttribute suffix
        LAString    mCurveType;		    // curve index
        LAString*   mpBasisCurveType;	// basis curve type
        
        mutable std::map<LAString, LAPriceDataInterpolation *> mDFInterMap;
        mutable std::map<LAString, LAPriceDataInterpolation *> mFWDInterMap;

    private:

        // Set data by name
        LADataHolder&		add(const LAString& name);
        mutable  std::map<LAString, int>		mCurveVersionMap; // cuveVersion map
        mutable std::map<LAString, LAPriceDataDayCount *> mDayCountMap; // daycount map

        LADataHolder*		mpName;				// Name				 (DATA_STRING)
        LADataHolder*		mpInter;			// Interpolation		 (DATA_INTERPOLATION)
        LADataHolder*		mpDayCount;			// DayCount Convension (DATA_DAYCOUNT) 
        LADataHolder*		mpCalendar;			// Calendar			 (DATA_CALENDAR) 
        LADataHolder*		mpSlidingRule;		// SlidingRule		 (DATA_SLIDINGRULE) 
        LADataHolder*		mpFreq;				// Frequency			 (DATA_STRING)
        LADataHolder*		mpYieldData;		// YieldData			 (DATA_REFERENCE)
        LADataHolder*		mpValueToInterp;	// Value To Interpolate(DATA_STRING)
        LADataHolder*		mpValueToInterp2;	// Value To Interpolate2(DATA_STRING)

        // Swap Calculations
        double              swapPV_;
        double              swapPVExcludingSpread_;
        double              fixedLegPV_;
        double              floatLegPV_;
        double              floatLegPVExcludingSpread_;
        double              spreadPV_;
        double              AnnuityFixed_;          // note this annuity parameter is scaled by the notional
        double              AnnuityFloat_;          // note this annuity parameter is scaled by the notional
        double              swapParRateMarket_;
        double              swapParRateTrade_;
        double              macaulaysDurationSwapExcludingSpread_;
        double              macaulaysDurationSpread_;
        double              modifiedDurationSwapExcludingSpread_;
        double              modifiedDurationSpread_;
        double              swapPV01_;
        double              swapDV01_;

    };


    class LAMathPathYieldCurve : public LARatesPathElementCurve
    {
    public:
        
        // Default constructor
        LAMathPathYieldCurve(const LACurvePricingObject* yc, double t, DayCount dc);
        
        // Destructor
        ~LAMathPathYieldCurve();

        // Make copy(clone) of this class
        virtual LARatesPathElementBase*		clone() const;

        const LACurvePricingObject*			    getYC(void) const { return mpYC; }
        const DayCount					    getDC(void) const { return mDC; }
        virtual double					    getP(double T) const;


        double							getForward(double T) const// initial curve
        {
            double ret = 0.0;
            if (0.0 == T)
                ret = getF(0.0001, 0.0001);
            else
                ret = getF(T, 0.0001);
            return ret;
        }

        virtual bool					operator==(const LAMathPathYieldCurve& a) const;

    private:
        const LACurvePricingObject*	mpYC;		// pointer to LACurvePricingObject class
        DayCount mDC;                       // day count
    };

}