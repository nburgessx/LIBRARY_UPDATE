/*!
	@file LAPriceVegaUnderlyingAsset.h
	@brief Defines three types: LAPriceIRVegaUnderlyingAsset, LAPriceOriginalIRSABRUnderlyingAsset and LAPriceAnotherUnderlyingAsset.
*/

#ifndef LAPriceIRVegaUnderlyingAsset_h
#define LAPriceIRVegaUnderlyingAsset_h

#include "LADate.h"
#include "LAString.h"
#include <utility>

class LAObject;
class LADataInstance;
class LAPriceDataDayCount;
class LAPriceDataCalendar;
class LAPriceDataSlidingRule;

// An abstract class an object of which represents interate rates of one currency.
class LAPriceIRVegaUnderlyingAsset
{
public:
	virtual ~LAPriceIRVegaUnderlyingAsset()
	{}

	// Deduces ("JPY3MLFORECAST", true /*is_cap*/) from "3M".
	//
	// @param underlying
	//     A string representation of the length of the underlying asset (caplet or swap)
	//
	// @return
	//     A pair of the curve name and the "capletness"
	virtual std::pair<LAString, bool> getCurveName(const LAString& underlying) const = 0;

	// Calculates a forward rate.
	//
	// @param opt
	//     A string representation of the option maturity
	// @param underlying
	//     A string representation of the length of the underlying asset (caplet or swap)
	// @param curveName
	//     The corresponding return value of getCurveName
	//
	// @return
	//     The forward rate (0.01 means 1%)
	virtual double getForward(
		const LAString& opt, const LAString& underlying, const std::pair<LAString, bool>& curveName) const = 0;
};

// An implementation of LAPriceIRVegaUnderlyingAsset
// for already set-up calib-info entities.
class LAPriceOriginalIRSABRUnderlyingAsset : public LAPriceIRVegaUnderlyingAsset
{
	LADataInstance* mDataInstance;
	LADate mBaseDate;
	LAString mConvIDs[2];
	const LAPriceDataSlidingRule* mSlidingRules[2];
	const LAPriceDataCalendar* mFixingCalendars[2];
	const LAObject* mCurveEntity;
	LAString mCurveID;
	LAString mDFName;

public:
	// Creates an object from calib-info entities.
	//
	// @param baseDate
	//     The base date
	// @param currency
	//     The currency
	// @param underlyingName
	//     The underlying name of IR vegas (such as "tibor")
	// @param dataInstance
	//     The dataInstance, that shall not be 0
	LAPriceOriginalIRSABRUnderlyingAsset(
		const LADate& baseDate, const LAString& currency, const LAString& underlyingName, LADataInstance* dataInstance);

	std::pair<LAString, bool> getCurveName(const LAString& underlying) const;

	// Calculates the option term and the caplet/swap length in years.
	//
	// @param opt
	//     A string representation of the option maturity
	// @param underlying
	//     A string representation of the length of the underlying asset (caplet or swap)
	// @param isCap
	//     curveName.second where curveName is the corresponding return value of getCurveName
	//
	// @return
	//     A pair of the option term and the caplet/swap length in years
	std::pair<double, double> getTerms(const LAString& opt, const LAString& underlying, bool isCap) const;

	double getForward(const LAString& opt, const LAString& underlying, const std::pair<LAString, bool>& curveName) const;

	// Retrieves the curve ID (the name of the "DATA" object for the yield concerned)
	//
	// @return
	//     The curve ID
	const LAString& getCurveID() const;
};

// Data names of risk entities referenced by LAPriceAnotherUnderlyingAsset STARTS HERE

#ifndef PRICING_DATA_VEGACONVERTEDSHIFTVAL
#define PRICING_DATA_VEGACONVERTEDSHIFTVAL		"VegaConvertedShiftVal"		// data name of VegaConvertedShiftVal
#endif
#ifndef PRICING_DATA_VEGACONVERTEDUNDERLYING
#define PRICING_DATA_VEGACONVERTEDUNDERLYING		"VegaConvertedUnderlying"		// data name of VegaConvertedUnderlying
#endif
#ifndef PRICING_DATA_VEGACONVERTEDCAPLETTENOR
#define PRICING_DATA_VEGACONVERTEDCAPLETTENOR		"VegaConvertedCapletTenor"		// data name of VegaConvertedCapletTenor
#endif
#ifndef PRICING_DATA_VEGACONVERTEDRATIOUPPERBOUND
#define PRICING_DATA_VEGACONVERTEDRATIOUPPERBOUND		"VegaConvertedRatioUpperBound"		// data name of VegaConvertedRatioUpperBound
#endif
#ifndef PRICING_DATA_VEGACONVERTEDCURVETYPEKEYS
#define PRICING_DATA_VEGACONVERTEDCURVETYPEKEYS		"VegaConvertedCurveTypeKeys"		// data name of VegaConvertedCurveTypeKeys
#endif
#ifndef PRICING_DATA_VEGACONVERTEDCURVETYPES
#define PRICING_DATA_VEGACONVERTEDCURVETYPES		"VegaConvertedCurveTypes"		// data name of VegaConvertedCurveTypes
#endif
#ifndef PRICING_DATA_VEGACONVERTEDCAPSPOTLAG
#define PRICING_DATA_VEGACONVERTEDCAPSPOTLAG		"VegaConvertedCapSpotLag"		// data name of VegaConvertedCapSpotLag
#endif
#ifndef PRICING_DATA_VEGACONVERTEDCAPDAYCOUNT
#define PRICING_DATA_VEGACONVERTEDCAPDAYCOUNT		"VegaConvertedCapDayCount"		// data name of VegaConvertedCapDayCount
#endif
#ifndef PRICING_DATA_VEGACONVERTEDCAPSLIDINGRULE
#define PRICING_DATA_VEGACONVERTEDCAPSLIDINGRULE		"VegaConvertedCapSlidingRule"		// data name of VegaConvertedCapSlidingRule
#endif
#ifndef PRICING_DATA_VEGACONVERTEDCAPPAYMENTCALENDAR
#define PRICING_DATA_VEGACONVERTEDCAPPAYMENTCALENDAR		"VegaConvertedCapPaymentCalendar"		// data name of VegaConvertedCapPaymentCalendar
#endif
#ifndef PRICING_DATA_VEGACONVERTEDCAPFIXINGCALENDAR
#define PRICING_DATA_VEGACONVERTEDCAPFIXINGCALENDAR		"VegaConvertedCapFixingCalendar"		// data name of VegaConvertedCapFixingCalendar
#endif
#ifndef PRICING_DATA_VEGACONVERTEDSWAPTIONSPOTLAG
#define PRICING_DATA_VEGACONVERTEDSWAPTIONSPOTLAG		"VegaConvertedSwaptionSpotLag"		// data name of VegaConvertedSwaptionSpotLag
#endif
#ifndef PRICING_DATA_VEGACONVERTEDSWAPTIONDAYCOUNT
#define PRICING_DATA_VEGACONVERTEDSWAPTIONDAYCOUNT		"VegaConvertedSwaptionDayCount"		// data name of VegaConvertedSwaptionDayCount
#endif
#ifndef PRICING_DATA_VEGACONVERTEDSWAPTIONSLIDINGRULE
#define PRICING_DATA_VEGACONVERTEDSWAPTIONSLIDINGRULE		"VegaConvertedSwaptionSlidingRule"		// data name of VegaConvertedSwaptionSlidingRule
#endif
#ifndef PRICING_DATA_VEGACONVERTEDSWAPTIONPAYMENTCALENDAR
#define PRICING_DATA_VEGACONVERTEDSWAPTIONPAYMENTCALENDAR		"VegaConvertedSwaptionPaymentCalendar"		// data name of VegaConvertedSwaptionPaymentCalendar
#endif
#ifndef PRICING_DATA_VEGACONVERTEDSWAPTIONFIXINGCALENDAR
#define PRICING_DATA_VEGACONVERTEDSWAPTIONFIXINGCALENDAR		"VegaConvertedSwaptionFixingCalendar"		// data name of VegaConvertedSwaptionFixingCalendar
#endif
#ifndef PRICING_DATA_VEGACONVERTEDSWAPTIONFREQUENCY
#define PRICING_DATA_VEGACONVERTEDSWAPTIONFREQUENCY		"VegaConvertedSwaptionFrequency"		// data name of VegaConvertedSwaptionFrequency
#endif

// Data names of risk entities referenced by LAPriceAnotherUnderlyingAsset ENDS HERE

// An implementation of LAPriceIRVegaUnderlyingAsset
// for arbitrary underlying assets after risk entities.
class LAPriceAnotherUnderlyingAsset : public LAPriceIRVegaUnderlyingAsset
{
	LADataInstance* mDataInstance;
	LAString mCurveID;
	LADate mBaseDate;
	const LAStringVector* mCurveTypeKeys;
	const LAStringVector* mCurveTypes;
	LAString mSpotLags[2];
	const LAPriceDataDayCount* mDayCounts[2];
	const LAPriceDataSlidingRule* mSlidingRules[2];
	const LAPriceDataCalendar* mFixingCalendars[2];
	const LAPriceDataCalendar* mPaymentCalendars[2];
	LAString mSwaptionFrequency;
	LAString mDFName;

public:
	// Creates an object from a risk object.
	//
	// @param baseDate
	//     The base date
	// @param curveID
	//     The curve ID (the name of the "DATA" object for the yield concerned)
	// @param info
	//     The risk object
	LAPriceAnotherUnderlyingAsset(const LADate& baseDate, const LAString& curveID, LAObject& info);

	std::pair<LAString, bool> getCurveName(const LAString& underlying) const;

	double getForward(const LAString& opt, const LAString& underlying, const std::pair<LAString, bool>& curveName) const;

private:
	const LAString& getDFName() const;
};

#endif
