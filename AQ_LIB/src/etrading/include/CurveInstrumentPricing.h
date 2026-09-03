//
//  CurveInstrumentPricing.h
//  This file was previosuly called YieldCurvePricing.h
//
#pragma once

#include "LAObject.h"
#include "LACurvePricingObject.h"
using namespace etrading;


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
class etrading::LAMathPathYieldCurve;

struct StubRateAndFixingDate
{
	LADate fixingDate_;
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
                              DateVector& floatAccrualDates, DateVector& floatPaymentDates, LADataInstance* dataInstance, const LAString& curveid,
                              LAString fixedDaycount, LAString floatDaycount, LAString interpolation, LAString foreCurveName,
                              LAString dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
                              double lastFixing, double floatSpread = 0.0, bool useFwdData = false, bool isOIS = false, LAString oisCompoundingType = "", 
							  LAString calendar = "", LAString rollConvention = "", LAString slidingRule = "" );

    // Get swap PV
	static double getSwapPV( bool& isFixedRatePayerSwap, double& notional, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
                             DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
                             LADataInstance* dataInstance, const LAString& curveid, double& fixedRate, const LAString& fixedDaycount,
                             double& floatSpreadInBasisPoints, const LAString& floatDaycount, const LAString& interpolation, const LAString& foreCurveName,
                             const LAString& dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
                             double lastFixing, bool useFwdData = false, bool isOIS = false, const LAString& compoundingMethod = "", const LAString& floatCalendar = "", 
							 const LAString& floatRollConv = "", const LAString& slidingRule = "", const LAString& floatFreq = "");

	// Get Swap DV01
    static double getSwapDV01( bool isFixedRatePayerSwap, double notional, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates,
                               DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
                               LADataInstance* dataInstance, const LAString& curveid, double& fixedRate, LAString fixedDaycount,
                               double& floatSpreadInBasisPoints, LAString floatDaycount, LAString interpolation, LAString foreCurveName,
                               LAString dfCurveName, bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing,
                               double lastFixing, bool isOIS = false, LAString compoundingMethod = "", LAString floatCalendar = "", 
							   LAString floatRollConv = "", LAString	slidingRule = "", const LAString& floatFrequency = "");

	// Get the Asset Swap Spread using Par-Par Methodology
    static double getAssetSwapSpread( const double& bondPrice, DateVector& fixedAccrualDates, DateVector& fixedPaymentDates, 
                                      DateVector& floatFixingDates, DateVector& floatAccrualDates, DateVector& floatPaymentDates,
                                      LADataInstance* dataInstance, const LAString& curveid, double& fixedRate, LAString fixedDaycount, 
                                      LAString floatDaycount, LAString interpolation, LAString foreCurveName, LAString dfCurveName,
                                      bool isFWDInter, bool useFirstFixing, double firstFixing, bool useLastFixing, double lastFixing,
                                      bool isCleanPrice = true, const LADate& settlementDate = LADate() );

    // get StubRate
    static StubRateAndFixingDate getStubRate( const DateVector& fixingDates,
											  const LAStringVector& curveNames,
											  const LAStringVector& curveTenors,
											  const DoubleVector& tenorCurveFixings,
											  const LAString& curveid,									
											  const LAString& stubType,
											  const LAString& interpolation,
											  const LAString& dateCount,
											  const LAString& calendar,
											  const LAString& busDayAdj,
											  const LAString& rollConvention,
											  bool  useNearbyCurve,
											  bool  isFwdInter,
											  bool  useFwdData,
											  const LAString& toleranceTenor,
											  const LAString& useCurveName,
											  const LAString& indexFrequency,
											  bool isRegularSwapSchedule = false);

	//Get Stub rate given fixingStart and fixingEnd dates
	static double getStubRateFromFixingStartEnd(const LADate& fixingDate,
										        const LADate& fixingEndDate,
										        const LAStringVector& curveNames,
										        const LAStringVector& curveTenors,
										        const DoubleVector& tenorCurveFixings,
										        const LAString& curveid,
										        const LAString& dateCount,
										        const LAString& calendar,
										        const LAString& busDayAdj,
										        bool  useNearbyCurve,
										        bool  isFwdInter,
										        bool  useFwdData,
										        const LAString& toleranceTenor,
										        const LAString& useCurveName);


};

