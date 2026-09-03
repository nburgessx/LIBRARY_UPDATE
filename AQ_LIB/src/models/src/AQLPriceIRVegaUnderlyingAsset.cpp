#include <algorithm>

#include "AQLDataBasics.h"
#include "AQLDataReference.h"
#include "AQLDataVector.h"
#include "AQLObjectPool.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLMathDefine.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLMathDateCalculations.h"
#include "AQLMathJamshidianSwaption.h"
#include "AQLMathSwaptionVolUtility.h"
#include "AQLPriceIRVegaUnderlyingAsset.h"

AQLPriceOriginalIRSABRUnderlyingAsset::AQLPriceOriginalIRSABRUnderlyingAsset(
		const AQLDate& baseDate, const AQLString& currency, const AQLString& underlyingName, AQLDataInstance* dataInstance) :
	mDataInstance(dataInstance), mBaseDate(baseDate)
{
	const AQLString calibInfoName = AQLString(currency).toUpper() + "_IRSABRCalibInfoEntity_" + underlyingName;
	AQLObject& calibInfo = dataInstance->getObjectPool().getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
	AQLObject* convs[2];
	convs[0] = &dynamic_cast<AQLDataReference&>(calibInfo.getData("CapConvID", ISNOTNULL).get()).get().get();
	convs[1] = &dynamic_cast<AQLDataReference&>(calibInfo.getData("SwaptionConvID", ISNOTNULL).get()).get().get();
	for (auto i : { 0, 1 })
	{
		mConvIDs[i] = dynamic_cast<const AQLDataString&>(convs[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
		mSlidingRules[i] =
			&dynamic_cast<const AQLPriceDataSlidingRule&>(convs[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		mFixingCalendars[i] =
			&dynamic_cast<const AQLPriceDataCalendar&>(convs[i]->getData(PRICING_DATA_FIXINGCALENDAR, ISNOTNULL).get());
	}

	const AQLString curveSetID = calibInfoName + "_CurveSetID";
	mCurveEntity = &dataInstance->getObjectPool().getObject(curveSetID, ENCHKTYPE_ISDEFINED).get();
	mCurveID = dynamic_cast<const AQLDataString&>(mCurveEntity->getData(PRICING_DATA_CURVEID, ISDEFINED).get()).get();
	mDFName = dynamic_cast<const AQLDataString&>(mCurveEntity->getData(CALIBRATION_DATA_DISCOUNTCURVENAME, ISDEFINED).get()).get();
}

std::pair<AQLString, bool>
AQLPriceOriginalIRSABRUnderlyingAsset::getCurveName(const AQLString& underlying) const
{
	int y, m, d, w;
	AQLMathDateCalculations::termStrtoYMDW(underlying, y, m, d, w);

	// According to AQLMathSwaptionVolUtility::calibrateSABRATMFix,
	// 1M, 3M, 6M vols are caplet vols
	bool isCap;
	AQLString forName;
	switch (y * 12 + m)
	{
	case 1:
		isCap = true;
		forName = dynamic_cast<const AQLDataString&>(mCurveEntity->getData(CALIBRATION_DATA_1MLCURVENAME, ISNOTNULL).get()).get();
		break;
	case 3:
		isCap = true;
		forName = dynamic_cast<const AQLDataString&>(mCurveEntity->getData(CALIBRATION_DATA_3MLCURVENAME, ISNOTNULL).get()).get();
		break;
	case 6:
		isCap = true;
		forName = dynamic_cast<const AQLDataString&>(mCurveEntity->getData(CALIBRATION_DATA_6MLCURVENAME, ISNOTNULL).get()).get();
		break;
	default:
		isCap = false;
		forName = dynamic_cast<const AQLDataString&>(mCurveEntity->getData(CALIBRATION_DATA_SWAPRATELCURVENAME, ISNOTNULL).get()).get();
		break;
	}

	return std::make_pair(std::move(forName), isCap);
}

std::pair<double, double>
AQLPriceOriginalIRSABRUnderlyingAsset::getTerms(const AQLString& opt, const AQLString& underlying, bool isCap) const
{
	const std::size_t i = isCap ? 0 : 1;

	const AQLPriceDataDayCount dc_act(ACT_365_ISDA);
	const AQLDate optDate = AQLMathDateCalculations::getDate(
		mBaseDate, opt, *mSlidingRules[i], mFixingCalendars[i], true);
	const double optTerm = dc_act.getTerm(mBaseDate, optDate, true);

	int y, m, d, w;
	AQLMathDateCalculations::termStrtoYMDW(underlying, y, m, d, w);
	return std::make_pair(optTerm, y + m / 12.0);
}

double
AQLPriceOriginalIRSABRUnderlyingAsset::getForward(
	const AQLString& opt, const AQLString& underlying, const std::pair<AQLString, bool>& curveName) const
{
	const std::size_t i = curveName.second ? 0 : 1;

	const AQLPriceDataDayCount dc_act(ACT_365_ISDA);
	const AQLDate optDate = AQLMathDateCalculations::getDate(
		mBaseDate, opt, *mSlidingRules[i], mFixingCalendars[i], true);
	return AQLMathSwaptionVolUtility::getForward(
		mDataInstance, optDate, underlying, mCurveID, mConvIDs[i], curveName.first, mDFName);
}

const AQLString&
AQLPriceOriginalIRSABRUnderlyingAsset::getCurveID() const
{
	return mCurveID;
}

AQLPriceAnotherUnderlyingAsset::AQLPriceAnotherUnderlyingAsset(const AQLDate& baseDate, const AQLString& curveID, AQLObject& info) :
	mDataInstance(info.getDataInstance()), mCurveID(curveID), mBaseDate(baseDate),
	mCurveTypeKeys(&dynamic_cast<const AQLDataStrings&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCURVETYPEKEYS).get()).get()),
	mCurveTypes(&dynamic_cast<const AQLDataStrings&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCURVETYPES).get()).get()),
	mSwaptionFrequency(dynamic_cast<const AQLDataString&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONFREQUENCY).get()).get())
{
	mSpotLags[0] = dynamic_cast<const AQLDataString&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCAPSPOTLAG).get()).get();
	mSpotLags[1] = dynamic_cast<const AQLDataString&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONSPOTLAG).get()).get();
	mDayCounts[0] = &dynamic_cast<const AQLPriceDataDayCount&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCAPDAYCOUNT).get());
	mDayCounts[1] = &dynamic_cast<const AQLPriceDataDayCount&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONDAYCOUNT).get());
	mSlidingRules[0] = &dynamic_cast<const AQLPriceDataSlidingRule&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCAPSLIDINGRULE).get());
	mSlidingRules[1] = &dynamic_cast<const AQLPriceDataSlidingRule&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONSLIDINGRULE).get());
	mFixingCalendars[0] = &dynamic_cast<const AQLPriceDataCalendar&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCAPFIXINGCALENDAR).get());
	mFixingCalendars[1] = &dynamic_cast<const AQLPriceDataCalendar&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONFIXINGCALENDAR).get());
	mPaymentCalendars[0] = &dynamic_cast<const AQLPriceDataCalendar&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCAPPAYMENTCALENDAR).get());
	mPaymentCalendars[1] = &dynamic_cast<const AQLPriceDataCalendar&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONPAYMENTCALENDAR).get());
	mDFName = getDFName();
}

// Deduces ("JPY3MLFORECAST", true /*is_cap*/) from "3M"
std::pair<AQLString, bool>
AQLPriceAnotherUnderlyingAsset::getCurveName(const AQLString& underlying) const
{
	AQLStringVector::const_iterator i;

	// Caplet?
	bool isCap = true;
	AQLString keyU = underlying;
	keyU.toUpper();
	keyU += "LNAME";
	i = std::find_if(mCurveTypeKeys->begin(), mCurveTypeKeys->end(),
		[&keyU](AQLString curveTypeKey) -> bool
		{
			curveTypeKey.toUpper();
			return curveTypeKey == keyU;
		});
	if (i == mCurveTypeKeys->end())
	{
		// Matches to "SwapRateName_1Y"?
		const AQLString keyU = "SWAPRATENAME_" + AQLString(underlying).toUpper();
		i = std::find_if(mCurveTypeKeys->begin(), mCurveTypeKeys->end(),
			[&keyU](AQLString curveTypeKey) -> bool
			{
				curveTypeKey.toUpper();
				return curveTypeKey == keyU;
			});
		if (i == mCurveTypeKeys->end())
		{
			i = std::find_if(mCurveTypeKeys->begin(), mCurveTypeKeys->end(),
				[](AQLString curveTypeKey) -> bool
				{
					curveTypeKey.toUpper();
					return curveTypeKey == "SWAPRATENAME";
				});
		}
		isCap = false;
	}
	return std::make_pair((*mCurveTypes)[i - mCurveTypeKeys->begin()], isCap);
}

double
AQLPriceAnotherUnderlyingAsset::getForward(
	const AQLString& opt, const AQLString& underlying, const std::pair<AQLString, bool>& curveName) const
{
	const std::size_t i = curveName.second ? 0 : 1;

	const AQLDate optDate = AQLMathDateCalculations::getDate(
		mBaseDate, opt, *mSlidingRules[i], mFixingCalendars[i], true);

	// This logic is based on the implementation of
	// AQLMathSwaptionVolUtility::getForward

	AQLPriceDataSlidingRule fol;
	fol.convertFromString(FOL);
	AQLDate tmpDate = AQLMathDateCalculations::getDate(
		optDate, mSpotLags[i], fol, mFixingCalendars[i], true);
	AQLDate endDate = AQLMathDateCalculations::getDate(
		tmpDate, underlying, *mSlidingRules[i], mPaymentCalendars[i], true);
	if (curveName.second)
	{
		DateVector dates;
		dates.push_back(tmpDate);
		dates.push_back(endDate);
		return AQLMathCurveFuncUtility::getParRate(
			dates, mDataInstance, mCurveID, mDayCounts[i]->convertToString(),
			"SPLINE", curveName.first, getDFName());
	}
	else
	{
		return AQLMathCurveFuncUtility::getParRate(
			mDataInstance, mCurveID, tmpDate, endDate, nullptr, nullptr, nullptr,
			mSwaptionFrequency, mDayCounts[i]->convertToString(),
			mSlidingRules[i]->convertToString(),
			mPaymentCalendars[i]->convertToString(), "SPLINE", curveName.first, getDFName());
	}
}

const AQLString&
AQLPriceAnotherUnderlyingAsset::getDFName() const
{
	const auto i = std::find_if(mCurveTypeKeys->begin(), mCurveTypeKeys->end(),
		[](AQLString curveTypeKey) -> bool
		{
			curveTypeKey.toUpper();
			return curveTypeKey == "DFNAME";
		});
	return (*mCurveTypes)[i - mCurveTypeKeys->begin()];
}
