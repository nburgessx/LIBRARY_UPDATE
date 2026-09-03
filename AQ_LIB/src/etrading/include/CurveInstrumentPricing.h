//
//  CurveInstrumentPricing.h
//  This file was previosuly called YieldCurvePricing.h
//
#pragma once

#include "AQLObject.h"
#include "LACurvePricingObject.h"
using namespace etrading;


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
class etrading::LAMathPathYieldCurve;

struct StubRateAndFixingDate
{
	AQLDate fixingDate_;
	double stubRate_;
};


/*! 
    @brief Class of CurveInstrumentPricing
*/
class CurveInstrumentPricing 
{
public:
	
	// constructor
	CurveInstrumentPricing() {}

	// destructor
	~CurveInstrumentPricing() {}
    
    //	get ParRate 
	static double getParRate( DateVector& fixedAccrualDates, DateVector& fixedPaymentDates, DateVector& floatFixingDates,
                              DateVector& floatAccrualDates, DateVector& floatPaymentDates, AQLDataInstance* dataInstance, const AQLString& curveid,
                              AQLString fixedDaycount, AQLString floatDaycount, AQLString interpolation, AQLString foreCurveName,
                              AQLString dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
                              double lastFixing, double floatSpread = 0.0, bool useFwdData = false, bool isOIS = false, AQLString oisCompoundingType = "", 
							  AQLString calendar = "", AQLString rollConvention = "", AQLString slidingRule = "" );

    // Get swap PV
	static double getSwapPV( bool& isFixedRatePayerSwap, double& notional, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
                             DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
                             AQLDataInstance* dataInstance, const AQLString& curveid, double& fixedRate, const AQLString& fixedDaycount,
                             double& floatSpreadInBasisPoints, const AQLString& floatDaycount, const AQLString& interpolation, const AQLString& foreCurveName,
                             const AQLString& dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
                             double lastFixing, bool useFwdData = false, bool isOIS = false, const AQLString& compoundingMethod = "", const AQLString& floatCalendar = "", 
							 const AQLString& floatRollConv = "", const AQLString& slidingRule = "", const AQLString& floatFreq = "");

	// Get Swap DV01
    static double getSwapDV01( bool isFixedRatePayerSwap, double notional, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
                               DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
                               AQLDataInstance* dataInstance, const AQLString& curveid, double& fixedRate, AQLString fixedDaycount,
                               double& floatSpreadInBasisPoints, AQLString floatDaycount, AQLString interpolation, AQLString foreCurveName,
                               AQLString dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
                               double lastFixing, bool isOIS = false, AQLString compoundingMethod = "", AQLString floatCalendar = "", 
							   AQLString floatRollConv = "", AQLString	slidingRule = "", const AQLString& floatFrequency = "");

	// Get the Asset Swap Spread using Par-Par Methodology
    static double getAssetSwapSpread( const double& bondPrice, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates, 
                                      DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
                                      AQLDataInstance* dataInstance, const AQLString& curveid, double& fixedRate, AQLString fixedDaycount, 
                                      AQLString floatDaycount, AQLString interpolation, AQLString foreCurveName, AQLString dfCurveName,
                                      bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing, double lastFixing,
                                      bool isCleanPrice = true, const AQLDate& settlementDate = AQLDate() );

    // get StubRate
    static StubRateAndFixingDate getStubRate( const DateVector& fixingDates,
											  const AQLStringVector& curveNames,
											  const AQLStringVector& curveTenors,
											  const DoubleVector& tenorCurveFixings,
											  const AQLString& curveid,									
											  const AQLString& stubType,
											  const AQLString& interpolation,
											  const AQLString& dateCount,
											  const AQLString& calendar,
											  const AQLString& busDayAdj,
											  const AQLString& rollConvention,
											  bool  useNearbyCurve,
											  bool  isFwdInter,
											  bool  useFwdData,
											  const AQLString& toleranceTenor,
											  const AQLString& useCurveName,
											  const AQLString& indexFrequency,
											  bool isRegularSwapSchedule = false);

	//Get Stub rate given fixingStart and fixingEnd dates
	static double getStubRateFromFixingStartEnd(const AQLDate& fixingDate,
										        const AQLDate& fixingEndDate,
										        const AQLStringVector& curveNames,
										        const AQLStringVector& curveTenors,
										        const DoubleVector& tenorCurveFixings,
										        const AQLString& curveid,
										        const AQLString& dateCount,
										        const AQLString& calendar,
										        const AQLString& busDayAdj,
										        bool  useNearbyCurve,
										        bool  isFwdInter,
										        bool  useFwdData,
										        const AQLString& toleranceTenor,
										        const AQLString& useCurveName);


};

