/*! 
	@file
    @brief  Class declaration to represent YieldCurve.

*/
//  2007, AlgoQuantHub..

#ifndef LAMathYieldCurve_h
#define LAMathYieldCurve_h

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLObject.h"
#include "AQLCoreAutoPtr.h"
#include "AQLPriceDataConvention.h"
#include "AQLFunctionBase.h"

#include "LAModelDynamicsCurve.h"
//#define STD     "STD"
//#define FIXEDRATE     "FIXEDRATE"
 
//// DEFINES ////
// LAMathYieldCurve Entitiy id
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


class AQLDataInstance;
class AQLDate;
class AQLDataDoubles;
class AQLDataDate;
class AQLInterpolationBase;
class AQLCoreProcedure;
class AQLDataString;
class AQLDataStrings;
class AQLPriceDataCalendar;
class AQLPriceDataInterpolation;
class AQLDataMultiReference;
class AQLDataReference;
class AQLPriceDataSlidingRule;

class AQLDataBool;
class LAMathPathYieldCurve;
//=================== YIELD CURVE ==================================
/*! 
    @brief Class of YieldCurve
*/
class LAMathYieldCurve : public AQLObject
{
public:
// LIFECYCLE
	// constructor
	LAMathYieldCurve(AQLDataInstance* dataInstance, const AQLString *const pCurveType = 0);
	// copy constructor
	LAMathYieldCurve(const LAMathYieldCurve& curve);
	// destructor
	virtual ~LAMathYieldCurve();

//  QUERY
	// get EntityType
	virtual object_t	getType(void) const;
    // this object have specified object or not
	virtual bool		isTypeOf(object_t id) const;
	//	get name
	const AQLDataString&	getName() const;
	//	get name
	AQLDataString&		getName() ;
	//	get Interpolation
	const AQLPriceDataInterpolation&
						getInterpolation() const;
	//	get Interpolation
	AQLPriceDataInterpolation&
						getInterpolation();	
	//	get Interpolation
	const AQLPriceDataInterpolation&
						getInterpolation(const AQLString &curveType) const;
	//	get Interpolation
	AQLPriceDataInterpolation&
						getInterpolation(const AQLString &curveType);	

	// get DayCountConvension
	const AQLPriceDataDayCount&
						getDayCount(void) const;
    // get DayCountConvension
	AQLPriceDataDayCount&		getDayCount(void);
	// get YieldData
	const AQLDataReference&
						getYieldData() const;
    // get YieldData
	AQLDataReference&
						getYieldData();
    // get Calendar
	const AQLPriceDataCalendar&
						getCalendar() const;
	// get Calendar
	AQLPriceDataCalendar&
						getCalendar();
    // get SlidingRule
	const AQLPriceDataSlidingRule&
						getSlidingRule() const;
    // get SlidingRule
	AQLPriceDataSlidingRule&
						getSlidingRule();
	// get Frequency
	const AQLDataString&
						getFrequency() const;
	// get Frequency
	AQLDataString&
						getFrequency();

	// get forward rates, day count fractions, DFs at starts, DFs at ends as tuple
	void analyzeLeg(const DateVector& inDates,
					const AQLString& inForeCurveName,
					const AQLString& inDFCurveName,
					bool isFWDInter,
					DoubleArray& rates,
					DoubleArray& terms,
					DoubleArray& dfs_start,
					DoubleArray& dfs_end);

	//	get ZeroRate from fromDate to toDate
	double				getZeroRate(const AQLDate& fromDate, const AQLDate& toDate, bool isFWDInter = true, bool useFwdData = false) const; // useFwdData defaults to false for backward compatibility purposes only
	//	get BasisZeroRate from fromDate to toDate
	double				getBasisZeroRate(const AQLDate& fromDate, const AQLDate& toDate) const;
	//	get ZeroRate for specified term from fromDate 
	double				getZeroRate(const AQLDate& fromDate,	const double term, bool isFWDInter = true, bool useFwdData = false) const; // useFwdData defaults to false for backward compatibility purposes only
	//	get BasisZeroRate for specified term from fromDate 
	double				getBasisZeroRate(const AQLDate& fromDate, const double term) const;
	//	get ZeroRate for specified term from AsOfDate 
	double				getZeroRate(const double term, bool isFWDInter = true) const;
	//	get BasisZeroRate for specified term from AsOfDate 
	double				getBasisZeroRate(const double term) const;
	//	get ZeroRate for specified term from fromDate 
	double				getZeroRate(const AQLDate& fromDate,	const AQLString& term_str, bool isFWDInter = true) const; 
	//	get BasisZeroRate for specified term from fromDate 
	double				getBasisZeroRate(const AQLDate& fromDate, const AQLString& term_str) const;
	//	get ZeroRate for specified term from AsOfDate 
	double				getZeroRate(const AQLString& term_str, bool isFWDInter = true) const;
	//	get BasisZeroRate for specified term from AsOfDate 
	double				getBasisZeroRate(const AQLString& term_str) const;
	//	get Annuity from DateVector 
	double				getAnnuity(const DateVector& Dates) const;
    
    //	get Annuity from AccrualDates, PaymentDates and Daycount fraction, optional deductAccruedInterest and 
    // isCleanPrice parameters are for bond / asset swap calculations
	double				getAnnuity( const DateVector& accrualDates, 
                                    const DateVector& paymentDates,
                                    const AQLPriceDataDayCount& daycount, 
                                    const AQLDate settlementDate = AQLDate(),
                                    const bool deductAccruedInterest = false,
                                    const bool isCleanPrice = true ) const;

    //	get ParRate for specified term from AsOfDate 
	double				getParRate(const AQLString& term_str, AQLString foreCurveName = STD, 
								   AQLString dfCurveName = STD, bool isFWDInter = true, const AQLString* roll_convention=NULL);
	
    //	get ParRate from DateVector 
	double				getParRate(const DateVector& dates, const DateVector& dates_float, AQLString foreCurveName = STD, 
								   AQLString dfCurveName = STD, bool isFWDInter = true);

    //	get ParRate from DateVector with First and Last Fixing Overrides
	double				getParRate( const DateVector&   fixedAccrualDates,
                                    const DateVector&   fixedPaymentDates, 
                                    const DateVector&   floatFixingDates,
                                    const DateVector&   floatAccrualDates,
                                    const DateVector&   floatPaymentDates,
                                    bool                useFirstFixing,
                                    double              firstFixing,
                                    bool                useLastFixing,
                                    double              lastFixing,
                                    AQLString            fixedDaycount,
                                    AQLString            floatDaycount, 
                                    AQLString            foreCurveName = STD,
                                    AQLString            dfCurveName = STD,
                                    bool                isFWDInter = true,
                                    double              floatSpread = 0.0, // Floating Libor Spread in Basis Points
                                    bool                useFwdData = false, // UseFwdData: False (default) imply forwards from discount factors, True = Use forward data directly ... set to flase for backward compatibility
									bool				isOIS = false,
									AQLString			oisCompoundingType = "",
							 		AQLString			calendar = "",
									AQLString			interpolation = "",
									AQLString			rollConvention = "",
									AQLString			slidingRule = "");


	//	get Par Spread of compound swaps from DateVector with First and Last Fixing Overrides
	double				getParCompoundSpread(const DateVector& dates,
											 const DateVector& dates_Cmp,
											 AQLString foreCurveName,
											 AQLString foreCurveName_Cmp,
											 AQLString dfCurveName,
											 bool isStraight,
											 bool isFWDInter);

	//	get Par Basis of CCSs from DateVector with First and Last Fixing Overrides
	double				getParBasis(const DateVector& dates,
									const DateVector& dates_USD,
									AQLString modifiedDiscountName,
									AQLString foreCurveName,
									AQLString foreCurveName_USD,
									AQLString dfCurveName_USD,
									bool isMarkToMarket,
									bool isFWDInter);

    //	get SwapPV from DateVector with First and Last Fixing Overrides
	double				getSwapPV( const bool&          isFixedRatePayerSwap,
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
                                   AQLString             fixedDaycount,
                                   AQLString             floatDaycount,
                                   AQLString             foreCurveName = STD,
                                   AQLString             dfCurveName = STD,
                                   bool                 isFWDInter = true,
                                    bool                useFwdData = false, // UseFwdData: False (default) imply forwards from discount factors, True = Use forward data directly ... set to flase for backward compatibility
								   bool					isOIS = false, 
								   AQLString				oisCompoundingType = "", 
								   AQLString				calendar = "", 
								   AQLString				inputInterpolation = "", 
								   AQLString				rollConvention = "", 
								   AQLString				slidingRule = "");

    //	get SwapDuration from DateVector with First and Last Fixing Overrides
	double				getSwapDuration( bool                 isModifiedDuration,
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
                                         AQLString             fixedDaycount,
                                         AQLString             floatDaycount,
                                         AQLString             foreCurveName = STD,
                                         AQLString             dfCurveName = STD,
                                         bool                 isFWDInter = true,
								         bool				  isOIS = false, 
								         AQLString			  oisCompoundingType = "", 
								         AQLString			  calendar = "", 
								         AQLString			  inputInterpolation = "", 
								         AQLString			  rollConvention = "", 
								         AQLString			  slidingRule = "" );

    //	get SwapDV01 from DateVector with First and Last Fixing Overrides
	double				getSwapDV01( bool                 isFixedRatePayerSwap,
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
                                     AQLString             fixedDaycount,
                                     AQLString             floatDaycount,
                                     AQLString             foreCurveName = STD,
                                     AQLString             dfCurveName = STD,
                                     bool                 isFWDInter = true,
								     bool				  isOIS = false, 
								     AQLString			  oisCompoundingType = "", 
								     AQLString			  calendar = "", 
								     AQLString			  inputInterpolation = "", 
								     AQLString			  rollConvention = "", 
								     AQLString			  slidingRule = "");

    //	get SwapPV01 from DateVector with First and Last Fixing Overrides
	double				getSwapPV01( const bool&        isFixedRatePayerSwap,
                                     const double&      notional,
                                     const DateVector&  fixedAccrualDates,
                                     const DateVector&  fixedPaymentDates,
                                     AQLString           fixedDaycount,
                                     AQLString           foreCurveName = STD,
                                     AQLString           dfCurveName = STD );

    //	get SwapPV01 from DateVector with First and Last Fixing Overrides
	double	     getAssetSwapSpread( const double& bondPrice,
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
                                     AQLString fixedDaycount,
                                     AQLString floatDaycount,
                                     AQLString foreCurveName = STD,
                                     AQLString dfCurveName = STD,
                                     bool isFWDInter = true,
                                     bool isCleanPrice = true,
                                     const AQLDate& settlementDate = AQLDate() );

	//	get ParRate by DateCalcUtility
	double				getParRate(const AQLDate& fromDate, const AQLDate& toDate, 
								   const AQLDate* firstStubDate = NULL,
								   const AQLDate* lastStubDate = NULL,
								   const int* pday = NULL, AQLString foreCurveName = STD, 
								   AQLString dfCurveName = STD, bool isFWDInter = true,
                                   const AQLString* roll_convention=NULL);
	// get ParRate by DateCalcUtility
	double				getParRate(const AQLDate& fromDate, const AQLString& term_str, 
								   const AQLDate* firstStubDate = NULL,
								   const AQLDate* lastStubDate = NULL,
								   const int* pday = NULL, AQLString foreCurveName = STD, 
								   AQLString dfCurveName = STD, bool isFWDInter = true,
                                   const AQLString* roll_convention=NULL);

	// Calculate stub rate of a defined stub period
	double				getStubRate(const DateVector& fixingDates,
										const AQLStringVector& curveNames,
										const AQLStringVector& curveTenors,
										const DoubleVector& tenorCurveFixings,
										const AQLString& curveid,									
										const AQLString& stubType,
										const AQLString& dateCount,
										const AQLString& calendar,
										const AQLString& busDayAdj,
										bool  useNearbyCurve,
										bool  useGivenFixings,
										bool  isFwdInter,
										bool  useFwdData,
										const AQLString& toleranceTenor,
										const AQLString& useCurveName);

	//	get DF from fromDate to toDate
	double				getDF(const AQLDate& fromDate, const AQLDate& toDate) const;
	
	//	get BasisDF from fromDate to toDate
	double				getBasisDF(const AQLDate& fromDate, const AQLDate& toDate) const;
	//	get DF for specified term from fromDate 
	double				getDF(const AQLDate& fromDate, const double& term) const;
	//	get BasisDF for specified term from fromDate 
	double				getBasisDF(const AQLDate& fromDate, const double& term) const;
	//	get DF for specified term from AsOfDate 
//	double				getDF(const double term) const;
	//	get DF for specified term from AsOfDate 
	double				getDF(const double term, const DayCount* pdc = NULL) const;
	//	get BasisDF for specified term from AsOfDate 
	double				getBasisDF(const double term, const DayCount* pdc = NULL) const;

	//	get DF for specified term from fromDate 
	//double			getDF(const AQLDate& fromDate, const AQLString& term_str) const;

	//	get DF for specified term from fromDate 
	double				getDF(const AQLDate& fromDate, const AQLString& term_str, AQLString bdc = DEF_VAL_NO_CHANGE , AQLString cal = DEF_VAL_BLANK ) const; // Optional Fields required for a bug fix
	//	get BasisDF for specified term from fromDate 
	double				getBasisDF(const AQLDate& fromDate, const AQLString& term_str) const;
	//	get DF for specified term from AsOfDate 
	double				getDF(const AQLString& fromDate, AQLString bdc = DEF_VAL_NO_CHANGE, AQLString cal = DEF_VAL_BLANK ) const; // Optional Fields required for a bug fix
	//	get BasisDF for specified term from AsOfDate 
	double				getBasisDF(const AQLString& term_str) const;
	//	get raw DFs and terms
	void				getRawDFs(DoubleArray& terms, DoubleArray& dfs) const;

	const AQLInterpolationBase   &getDFInterpolation(const AQLString *pCurveType = 0) const;
    const AQLInterpolationBase   &getForwardsInterpolation(const AQLString *pCurveType = 0) const;
	const AQLInterpolationBase   &getBasisDFInterpolation() const;

	//	clone this class
	AQLObject*			clone() const;// %%% COVARIANT RETURN %%%
	
	double				getCurve(const double term) const;
	LAMathPathYieldCurve	getCurve(double t, DayCount dc) const;

//  OPERATION 

	//	set Interpolation
	void				setInterpolation(AQLInterpolationBase* a, 
											const AQLString& name);
	// set Interpolation
	void				setInterpolation(const AQLString& name);


	// remove specified Data. If Data is nothing, do nothing.
    virtual void        remove(const AQLString& dataName);
	// Initialize this Object.
	virtual void		reset(void);
	//	 
	void				setCurveType(const AQLString &curveType);
	//	 
	void				setBasisCurveType(const AQLString &basisCurveType);
	// !
	void				setCurveNameAndInterpolation(const AQLString &curveName, const AQLString &interpolation);

	AQLString			getCurveType(void) const;
	
	static const RateConvention
						setRC(AQLString Freq);


	// get DFInterpolationMethod
	const AQLDataString&
						getDFInterpolationMethod() const;
	// get DFInterpolationMethod
	AQLDataString&
						getDFInterpolationMethod();
	// get DF2InterpolationMethod
	const AQLDataString&
						getDF2InterpolationMethod() const;
	// get DF2InterpolationMethod
	AQLDataString&
						getDF2InterpolationMethod();


	static unsigned int setSpanFromFrequency(const AQLString& freq );

	double getBasisSwapValue(const AQLDate& valueDate, const AQLDate& startDate, const AQLString& term, double basis, 
							 bool isPrincipal, const AQLString& frequency, const AQLPriceDataDayCount& daycount, 
							 const AQLString& forecastCurveID, const AQLString& discountCurveID, double firstFixingRate, 
							 bool isEOMRoll, bool isFRN);

	double getCurBasisSwapValue(const LAMathYieldCurve& yc, const AQLDate& valueDate, const AQLDate& startDate, 
								const AQLString& term, const AQLString& frequency, const AQLPriceDataDayCount& daycount, 
								const AQLString& forecastCurveID, const AQLString& discountCurveID, bool isEOMRoll, 
								double firstFixingAmount = 1., double firstFixingRate = 0.);

	// get base forward rate
	void getBaseForwardRate(const AQLString &curveType, DoubleArray &terms, DoubleMatrix &termsMat, DoubleArray &taus, DoubleArray &rates, double &joinDateAsDouble, std::vector<DateVector> *pDateMat_out = 0, std::vector<DateVector> *pDateMat_in = 0, const bool & useForwardData = false) const;

	// get base forward rate
	void getBaseForwardRate(const AQLString &curveType, DoubleArray &terms, DoubleMatrix &termsMat, DoubleArray &taus, DoubleArray &rates, std::vector<DateVector> *pDateMat_out = 0, std::vector<DateVector> *pDateMat_in = 0, const bool & useForwardData = false) const;

    // get daycount adjusted forward rate
	void getAdjForwardRate(const AQLString &curveType, const AQLPriceDataDayCount &dc, DoubleArray &terms, DoubleMatrix &termsMat, DoubleArray &taus, DoubleArray &rates, std::vector<DateVector> *pDateMat_in = 0) const;

	// get forward interpolation
    const AQLInterpolationBase   &getFWDInterpolation(const AQLString *pCurveType = 0, const bool & useFwdData = false) const;
    
	// get DayCountConvension
	const AQLPriceDataDayCount&
						getDayCount(const AQLString &curveType) const;
    // get DayCountConvension
	AQLPriceDataDayCount&		getDayCount(const AQLString &curveType);
	// get DayCountConvension from yield data object
	const AQLPriceDataDayCount&
						getDayCountFromYieldData(const AQLString &curveType) const;

	// get convension from yield curve data object
	void				getCurveConvention(AQLString& freq, AQLPriceDataCalendar& cal, AQLPriceDataSlidingRule& sld,
										   AQLPriceDataDayCount& dc, AQLString& accessary, const AQLString& curveName) const;

	// get convension from yield curve data object
	void				getCurveConvention(AQLString& freq, AQLPriceDataCalendar& cal, AQLPriceDataSlidingRule& sld,
										   AQLPriceDataDayCount& dc, AQLString& accessary, const AQLString& curveName);

	virtual bool operator==(const LAMathYieldCurve& a) const;

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
    *  @param [in]      AQLString              fixedDaycount
    *  @param [in]      AQLString              floatDaycount
    *  @param [in]      AQLString              foreCurveName; forecast curve name, defaults to STD
    *  @param [in]      AQLString              dfCurveName; discount curve name, defaults OIS
    *  @param [in]      bool                  isFWDInter, This flag controls how forwards are calculated; True = interpolate forwards, False = imply forwards from discount factors         
	*  @param [in]		bool				  isOIS, Is the forecasting curve an OIS Curve?: True or False
	*  @param [in]		AQLString			  oisCompoundingType, Ois Curve Compounding Method
	*  @param [in]		AQLString			  calendar
	*  @param [in]		AQLString			  interpolation
	*  @param [in]		AQLString			  rollConvention
	*  @param [in]		AQLString			  slidingRule
    *
	*/
    void calculateSwapPriceAndRisk( const bool             isFixedRatePayerSwap,      
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
                                    const AQLString &       fixedDaycount,             
                                    const AQLString &       floatDaycount,             
                                    const AQLString &       foreCurveName           = STD,       
                                    const AQLString &       dfCurveName             = OIS,
                                    const bool             isFWDInter              = true,         
								    const bool		       isOIS                   = false,           
								    const AQLString &	   oisCompoundingType      = "", 
								    const AQLString &	   calendar                = "",           
								    const AQLString &	   inputInterpolation      = "", 
								    const AQLString &	   rollConvention          = "",     
								    const AQLString &	   slidingRule             = "" );

	// Remove data of a given curve
	void deleteCurveDataByCurveName(const AQLString& curveName);

protected:
	// copy object		 
	virtual AQLObject&	copy(const AQLObject& e);
	// delete DF interpolations
	void delDFInterpolations();
	// delete FWD interpolations
	void delFWDInterpolations();
	// delete daycount
	void delDayCounts();

	// delete DF interpolations of the given curve
	void delDFInterpolationsByCurve(const AQLString& curve);
	// delete FWD interpolations of the given curve
	void delFWDInterpolationsByCurve(const AQLString& curve);
	// delete daycount of the given curve
	void delDayCountsByCurve(const AQLString& curve);

	// setUpInterpolation
	void setUpInterpolation() const;
    // setValuesToInterpolator
    void setValuesToInterpolator( const AQLString& curveType, const DoubleArray &index, const DoubleArray &value ) const;
    // setValuesToInterpolator
    void setValuesToInterpolator( AQLPriceDataInterpolation* pInter, const AQLString& curveType, const DoubleArray &index, const DoubleArray &value ) const;
    //	get CurveTypeInterpolation
	AQLPriceDataInterpolation& getCurveTypeInterpolation(const AQLString &curveType) const;
	//	add DF interpolation
	void addDFInterpolation(const AQLString &curveType) const;

	//	add FWD interpolation
	void addFWDInterpolation(const AQLString &curveType) const;
	// get yield data version
	int	getYieldDataVersion(void) const;
	
	AQLString	mCurveSuffix;		// CurveAttribute suffix
	AQLString    mCurveType;		// curve Type
	AQLString*   mpBasisCurveType;	// basis curve type
	mutable std::map<AQLString, AQLPriceDataInterpolation *> mDFInterMap;
	mutable std::map<AQLString, AQLPriceDataInterpolation *> mFWDInterMap;
		
private:
	
	// Set data by name
	AQLDataHolder&		add(const AQLString& name);
	mutable  std::map<AQLString, int>		mCurveVersionMap; // cuveVersion map
	mutable std::map<AQLString, AQLPriceDataDayCount *> mDayCountMap; // daycount map

	AQLDataHolder*		mpName;				// Name				 (DATA_STRING)
	AQLDataHolder*		mpInter;			// Interpolation		 (DATA_INTERPOLATION)
	AQLDataHolder*		mpDayCount;			// DayCount Convension (DATA_DAYCOUNT) 
	AQLDataHolder*		mpCalendar;			// Calendar			 (DATA_CALENDAR) 
	AQLDataHolder*		mpSlidingRule;		// SlidingRule		 (DATA_SLIDINGRULE) 
	AQLDataHolder*		mpFreq;				// Frequency			 (DATA_STRING)
	AQLDataHolder*		mpYieldData;		// YieldData			 (DATA_REFERENCE)
	AQLDataHolder*		mpValueToInterp;	// Value To Interpolate(DATA_STRING)
	AQLDataHolder*		mpValueToInterp2;	// Value To Interpolate2(DATA_STRING)

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
	LAMathPathYieldCurve(const LAMathYieldCurve* yc, double t, DayCount dc);
	// Destructor
	~LAMathPathYieldCurve();
		
										// Make copy(clone) of this class
	virtual LARatesPathElementBase*		clone() const;// %%% COVARIANT RETURN %%%
										//======================================
										// get value
	/*!
		@return yield curve object
	*/
	const LAMathYieldCurve*			getYC(void) const {return mpYC;}

	/*!
		@return value
	*/
//	virtual const void*				get(void) const {return &mpYC;}
		
	/*!
		@return day count
	*/
	const DayCount					getDC(void) const {return mDC;}

	virtual double					getP(double T) const;
	
/*	virtual void					set(const void* pdata) 
									{	
										const LAMathYieldCurve* pdata_YC = reinterpret_cast<const LAMathYieldCurve*>(pdata); 
										//const SCALARARRAY* p_array = reinterpret_cast<const SCALARARRAY*>(pdata); 
										//int i, j;
										//for (i = p_array->size() - 1, j = mData_L.size() - 1; j >= 0; j--, i--)
										mpYC = pdata_YC;
									}*/

//	virtual void					set(const LARatesPathElementBase& a);
	double							getForward(double T) const// initial curve
									{
										double ret = 0.0;
										if(0.0 == T)
											ret = getF(0.0001,0.0001);
										else
											ret = getF(T,0.0001);
										return ret;
									}

	virtual bool					operator==(const LAMathPathYieldCurve& a) const;
		
private:
	const LAMathYieldCurve*	mpYC;		// pointer to LAMathYieldCurve class
	DayCount mDC;// day count
};
#endif
