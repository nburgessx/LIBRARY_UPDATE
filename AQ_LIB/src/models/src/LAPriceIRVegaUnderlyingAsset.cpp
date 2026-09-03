#include <algorithm>

#include "LADataBasics.h"
#include "LADataReference.h"
#include "LADataVector.h"
#include "LAObjectPool.h"
#include "LADataInstance.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataSlidingRule.h"
#include "LAMathDefine.h"
#include "LAMathCurveFuncUtility.h"
#include "LAMathDateCalculations.h"
#include "LAMathJamshidianSwaption.h"
#include "LAMathSwaptionVolUtility.h"
#include "LAPriceIRVegaUnderlyingAsset.h"

LAPriceOriginalIRSABRUnderlyingAsset::LAPriceOriginalIRSABRUnderlyingAsset(
		const LADate& baseDate, const LAString& currency, const LAString& underlyingName, LADataInstance* dataInstance) :
	mDataInstance(dataInstance), mBaseDate(baseDate)
{
	const LAString calibInfoName = LAString(currency).toUpper() + "_IRSABRCalibInfoEntity_" + underlyingName;
	LAObject& calibInfo = dataInstance->getObjectPool().getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
	LAObject* convs[2];
	convs[0] = &dynamic_cast<LADataReference&>(calibInfo.getData("CapConvID", ISNOTNULL).get()).get().get();
	convs[1] = &dynamic_cast<LADataReference&>(calibInfo.getData("SwaptionConvID", ISNOTNULL).get()).get().get();
	for (auto i : { 0, 1 })
	{
		mConvIDs[i] = dynamic_cast<const LADataString&>(convs[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
		mSlidingRules[i] =
			&dynamic_cast<const LAPriceDataSlidingRule&>(convs[i]->getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get());
		mFixingCalendars[i] =
			&dynamic_cast<const LAPriceDataCalendar&>(convs[i]->getData(PRICING_DATA_FIXINGCALENDAR, ISNOTNULL).get());
	}

	const LAString curveSetID = calibInfoName + "_CurveSetID";
	mCurveEntity = &dataInstance->getObjectPool().getObject(curveSetID, ENCHKTYPE_ISDEFINED).get();
	mCurveID = dynamic_cast<const LADataString&>(mCurveEntity->getData(PRICING_DATA_CURVEID, ISDEFINED).get()).get();
	mDFName = dynamic_cast<const LADataString&>(mCurveEntity->getData(CALIBRATION_DATA_DISCOUNTCURVENAME, ISDEFINED).get()).get();
}

std::pair<LAString, bool>
LAPriceOriginalIRSABRUnderlyingAsset::getCurveName(const LAString& underlying) const
{
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(underlying, y, m, d, w);

	// According to LAMathSwaptionVolUtility::calibrateSABRATMFix,
	// 1M, 3M, 6M vols are caplet vols
	bool isCap;
	LAString forName;
	switch (y * 12 + m)
	{
	case 1:
		isCap = true;
		forName = dynamic_cast<const LADataString&>(mCurveEntity->getData(CALIBRATION_DATA_1MLCURVENAME, ISNOTNULL).get()).get();
		break;
	case 3:
		isCap = true;
		forName = dynamic_cast<const LADataString&>(mCurveEntity->getData(CALIBRATION_DATA_3MLCURVENAME, ISNOTNULL).get()).get();
		break;
	case 6:
		isCap = true;
		forName = dynamic_cast<const LADataString&>(mCurveEntity->getData(CALIBRATION_DATA_6MLCURVENAME, ISNOTNULL).get()).get();
		break;
	default:
		isCap = false;
		forName = dynamic_cast<const LADataString&>(mCurveEntity->getData(CALIBRATION_DATA_SWAPRATELCURVENAME, ISNOTNULL).get()).get();
		break;
	}

	return std::make_pair(std::move(forName), isCap);
}

std::pair<double, double>
LAPriceOriginalIRSABRUnderlyingAsset::getTerms(const LAString& opt, const LAString& underlying, bool isCap) const
{
	const std::size_t i = isCap ? 0 : 1;

	const LAPriceDataDayCount dc_act(ACT_365_ISDA);
	const LADate optDate = LAMathDateCalculations::getDate(
		mBaseDate, opt, *mSlidingRules[i], mFixingCalendars[i], true);
	const double optTerm = dc_act.getTerm(mBaseDate, optDate, true);

	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(underlying, y, m, d, w);
	return std::make_pair(optTerm, y + m / 12.0);
}

double
LAPriceOriginalIRSABRUnderlyingAsset::getForward(
	const LAString& opt, const LAString& underlying, const std::pair<LAString, bool>& curveName) const
{
	const std::size_t i = curveName.second ? 0 : 1;

	const LAPriceDataDayCount dc_act(ACT_365_ISDA);
	const LADate optDate = LAMathDateCalculations::getDate(
		mBaseDate, opt, *mSlidingRules[i], mFixingCalendars[i], true);
	return LAMathSwaptionVolUtility::getForward(
		mDataInstance, optDate, underlying, mCurveID, mConvIDs[i], curveName.first, mDFName);
}

const LAString&
LAPriceOriginalIRSABRUnderlyingAsset::getCurveID() const
{
	return mCurveID;
}

LAPriceAnotherUnderlyingAsset::LAPriceAnotherUnderlyingAsset(const LADate& baseDate, const LAString& curveID, LAObject& info) :
	mDataInstance(info.getDataInstance()), mCurveID(curveID), mBaseDate(baseDate),
	mCurveTypeKeys(&dynamic_cast<const LADataStrings&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCURVETYPEKEYS).get()).get()),
	mCurveTypes(&dynamic_cast<const LADataStrings&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCURVETYPES).get()).get()),
	mSwaptionFrequency(dynamic_cast<const LADataString&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONFREQUENCY).get()).get())
{
	mSpotLags[0] = dynamic_cast<const LADataString&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCAPSPOTLAG).get()).get();
	mSpotLags[1] = dynamic_cast<const LADataString&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONSPOTLAG).get()).get();
	mDayCounts[0] = &dynamic_cast<const LAPriceDataDayCount&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCAPDAYCOUNT).get());
	mDayCounts[1] = &dynamic_cast<const LAPriceDataDayCount&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONDAYCOUNT).get());
	mSlidingRules[0] = &dynamic_cast<const LAPriceDataSlidingRule&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCAPSLIDINGRULE).get());
	mSlidingRules[1] = &dynamic_cast<const LAPriceDataSlidingRule&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONSLIDINGRULE).get());
	mFixingCalendars[0] = &dynamic_cast<const LAPriceDataCalendar&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCAPFIXINGCALENDAR).get());
	mFixingCalendars[1] = &dynamic_cast<const LAPriceDataCalendar&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONFIXINGCALENDAR).get());
	mPaymentCalendars[0] = &dynamic_cast<const LAPriceDataCalendar&>(
		info.getData(PRICING_DATA_VEGACONVERTEDCAPPAYMENTCALENDAR).get());
	mPaymentCalendars[1] = &dynamic_cast<const LAPriceDataCalendar&>(
		info.getData(PRICING_DATA_VEGACONVERTEDSWAPTIONPAYMENTCALENDAR).get());
	mDFName = getDFName();
}

// Deduces ("JPY3MLFORECAST", true /*is_cap*/) from "3M"
std::pair<LAString, bool>
LAPriceAnotherUnderlyingAsset::getCurveName(const LAString& underlying) const
{
	LAStringVector::const_iterator i;

	// Caplet?
	bool isCap = true;
	LAString keyU = underlying;
	keyU.toUpper();
	keyU += "LNAME";
	i = std::find_if(mCurveTypeKeys->begin(), mCurveTypeKeys->end(),
		[&keyU](LAString curveTypeKey) -> bool
		{
			curveTypeKey.toUpper();
			return curveTypeKey == keyU;
		});
	if (i == mCurveTypeKeys->end())
	{
		// Matches to "SwapRateName_1Y"?
		const LAString keyU = "SWAPRATENAME_" + LAString(underlying).toUpper();
		i = std::find_if(mCurveTypeKeys->begin(), mCurveTypeKeys->end(),
			[&keyU](LAString curveTypeKey) -> bool
			{
				curveTypeKey.toUpper();
				return curveTypeKey == keyU;
			});
		if (i == mCurveTypeKeys->end())
		{
			i = std::find_if(mCurveTypeKeys->begin(), mCurveTypeKeys->end(),
				[](LAString curveTypeKey) -> bool
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
LAPriceAnotherUnderlyingAsset::getForward(
	const LAString& opt, const LAString& underlying, const std::pair<LAString, bool>& curveName) const
{
	const std::size_t i = curveName.second ? 0 : 1;

	const LADate optDate = LAMathDateCalculations::getDate(
		mBaseDate, opt, *mSlidingRules[i], mFixingCalendars[i], true);

	// This logic is based on the implementation of
	// LAMathSwaptionVolUtility::getForward

	LAPriceDataSlidingRule fol;
	fol.convertFromString(FOL);
	LADate tmpDate = LAMathDateCalculations::getDate(
		optDate, mSpotLags[i], fol, mFixingCalendars[i], true);
	LADate endDate = LAMathDateCalculations::getDate(
		tmpDate, underlying, *mSlidingRules[i], mPaymentCalendars[i], true);
	if (curveName.second)
	{
		DateVector dates;
		dates.push_back(tmpDate);
		dates.push_back(endDate);
		return LAMathCurveFuncUtility::getParRate(
			dates, mDataInstance, mCurveID, mDayCounts[i]->convertToString(),
			"SPLINE", curveName.first, getDFName());
	}
	else
	{
		return LAMathCurveFuncUtility::getParRate(
			mDataInstance, mCurveID, tmpDate, endDate, nullptr, nullptr, nullptr,
			mSwaptionFrequency, mDayCounts[i]->convertToString(),
			mSlidingRules[i]->convertToString(),
			mPaymentCalendars[i]->convertToString(), "SPLINE", curveName.first, getDFName());
	}
}

const LAString&
LAPriceAnotherUnderlyingAsset::getDFName() const
{
	const auto i = std::find_if(mCurveTypeKeys->begin(), mCurveTypeKeys->end(),
		[](LAString curveTypeKey) -> bool
		{
			curveTypeKey.toUpper();
			return curveTypeKey == "DFNAME";
		});
	return (*mCurveTypes)[i - mCurveTypeKeys->begin()];
}
