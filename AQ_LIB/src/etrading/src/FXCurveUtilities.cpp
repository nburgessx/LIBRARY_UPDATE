/*
 * @brief			Helper methods used for Schedule
 * @Created:		17 March 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "FXCurveUtilities.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "LACurveForwardRateHelpers.h"
#include "LAPriceDataCalendar.h"

#include "LAMathDefine.h"
#include "LACurvePricingObject.h"
#include "LADataBasics.h"
#include "LACoreComponentManager.h"

#include "LAPriceDataSlidingRule.h"
#include "LADataReference.h"
#include "LADateScheduleHelpers.h"
#include <algorithm>
#include "CurveBuildDefaults.h"
#include "LWOUtilities.h"

#include "LADateHelpers.h"
#include "LACompoundingFunc.h"
#include "LAMathInterpolationUtilities.h"

#include "CurveMarketData.h"
#include "ExceptionMacros.h"
#include "LADefinitions.h"
#include "CurveResultsContainer.h"


namespace etrading
{

	/* @brief			Helper method to calculate fx swap point sign
    */
	 int getFxSwapPointSign(const double spotFxBidRate, const double spotFxAskRate)
	 {
		 int swapPointSignIndicator = (spotFxBidRate <= spotFxAskRate) ? 1 : -1;
		 return swapPointSignIndicator;
	 }

	/* @brief			Helper method to calculate fx forward points given the forward outrights
    */
	 FxFwd getFxForwardRatesFromOutrightToPoint(const FxSpotRate& fxSpot, const FxFwd& fxFwdRate, const double pipSize, const bool settleEarlyThanSpot)
	 {
                
		// FXForwardOutright = FxSpotOutright +/- swapPoints * pipSize 
		// => swapPoints = (FXForwardOutright - FxSpotOutright) / pipSize * swapPointSign

		int swapPointSign =getFxSwapPointSign(fxSpot.bid, fxSpot.ask);
		if (settleEarlyThanSpot)
		{
			swapPointSign *= -1;
		}
		double factor = swapPointSign / pipSize;

		FxFwd fxPoint;

		fxPoint.isOutright = false;
		fxPoint.mid = (fxFwdRate.mid - fxSpot.mid) * factor;
		fxPoint.bid = (fxFwdRate.bid - fxSpot.bid) * factor;
		fxPoint.ask = (fxFwdRate.ask - fxSpot.ask) * factor;

		return fxPoint;
	 }

	/* @brief			Helper method to calculate fx forward outrights given the forward points
    */
	 FxFwd getFxForwardRatesFromPointsToOutright(const FxSpotRate& fxSpot, const FxFwd& fxSwapPoint, const double pipSize, const bool settleEarlyThanSpot)
	 {
		// FXForwardOutright = FxSpotOutright +/- swapPoints * pipSize 

		int swapPointSign =getFxSwapPointSign(fxSpot.bid, fxSpot.ask);
		if (settleEarlyThanSpot)
		{
			swapPointSign *= -1;
		}

		double factor = swapPointSign * pipSize;

		// outright = spot + point/factor
		FxFwd fxRate;
		fxRate.isOutright = true;
		fxRate.mid = fxSpot.mid + fxSwapPoint.mid * factor;
		fxRate.bid = fxSpot.bid + fxSwapPoint.bid * factor;
		fxRate.ask = fxSpot.ask + fxSwapPoint.ask * factor;
		return fxRate;
	 }

	 LADate getDateFromTenorWithFxConvention(const LADate& fromDate, const std::string& tenor, const std::string& calendar, const std::string& businessDayAdjustment, const std::string& rollDayInput)
	 {
		 auto rollConvention = getRollConvection(LADate(), LADate(), rollDayInput.c_str());

		 LADate toDate = getDateFromTenor(boost::assign::list_of(fromDate),
			 tenor.c_str(),
			 businessDayAdjustment.c_str(),
			 calendar.c_str(),
			 rollConvention)[0];

		 return toDate;

	 }

	 bool isSpotTenor(const std::string& settlementDateOrTenor)
	 {
		 return boost::iequals(settlementDateOrTenor, "Spot") || boost::iequals(settlementDateOrTenor, "SP");
	 }

	 bool isSettleEarlierThanSpotDate(const std::string& settleDateOrTenor, const LADate& settleDate, const LADate& spotDate) 
	 {
		 return (settleDate < spotDate);
	 }


	 LADate fromSettleDateOrTenorToDate(const std::string& settlementDateOrTenor, const LADate& asOfDate, const LADate& spotDate, const std::string& calendar, const std::string& businessDayAdjustment, const std::string& rollDayInput)
	 {
		 //Decide if the settlementDatesOrTenors are in tenor format by checking the first element
		 bool isSettleTenor = isMaturityDateTenor(settlementDateOrTenor.c_str());
		 LADate settleDate;
		 if (isSettleTenor)
		 {
			 if (isSpotTenor(settlementDateOrTenor))
			 {
				 settleDate = spotDate;
			 }
			 else
			 {
				 settleDate = getDateFromTenorWithFxConvention(asOfDate, settlementDateOrTenor, calendar, businessDayAdjustment, rollDayInput);
			 }
			 //settleDate starts from spot date of the settle tenor later than spot tenor
			 if (settleDate > spotDate)
			 {
				 settleDate = getDateFromTenorWithFxConvention(spotDate, settlementDateOrTenor, calendar, businessDayAdjustment, rollDayInput);
			 }
		 }
		 else
		 {
			 settleDate = stringToDate(settlementDateOrTenor.c_str(), "#Error: Invalid 'SettleDate'.");
		 }
		 return settleDate;
	 }

	 std::vector<LADate> fromSettleDatesOrTenorsToDates(const std::vector<std::string>& settlementDatesOrTenors, const LADate& asOfDate, const LADate& spotDate, const std::string& calendar, const std::string& businessDayAdjustment, const std::string& rollDayInput)
	 {
		 //Populate settleDates
		 size_t expectedSize = settlementDatesOrTenors.size();
		 std::vector<LADate> settlementDates(expectedSize);

		 for (size_t i = 0; i < expectedSize; ++i)
		 {
			 LADate settleDate = fromSettleDateOrTenorToDate(settlementDatesOrTenors[i], asOfDate, spotDate, calendar, businessDayAdjustment, rollDayInput);
			 settlementDates[i] = settleDate;
		 }

		 return settlementDates;
	 }


	/* @brief			Populate the fx forward bid/ask points and bid/ask outrights based on the quoted spot rate and fx forward rate
    *  @param [in]		fxSpot	    Spot fx rate
    *  @param [in]		fxFwd	    Forward fx 
    *  @param [in]		pipSize				Swap point pip size
    *  @param [in]		settleEarlyThanSpot	True to indicate settleDate earlier than spotDate
    *  @return			Return a vector of forward fx rates/swap points
    */
    std::vector<FxFwd> populateFxForwardPointsAndOutrights(const FxSpotRate& fxSpot, const FxFwd& fxFwd, const double pipSize, const bool settleEarlyThanSpot)
    {

		FxFwd fxPoint;
		FxFwd fxFwdRate;

		if (fxFwd.isOutright)
		{
			fxFwdRate = fxFwd;
			fxPoint = getFxForwardRatesFromOutrightToPoint(fxSpot, fxFwdRate, pipSize, settleEarlyThanSpot);
		} 
		//quoted fxFwdRate is in point
		else
		{
			fxPoint = fxFwd;
			fxFwdRate = getFxForwardRatesFromPointsToOutright(fxSpot, fxPoint, pipSize, settleEarlyThanSpot);
		} 

        std::vector<FxFwd> forwardFXs (2);

		forwardFXs[0] = fxPoint;
		forwardFXs[1] = fxFwdRate;

        return forwardFXs;
     }


	/* @brief			Calculate forward forward fx rates (fxSwap rates)
    *  @param [in]		nearLegFwdFxBidAsks Near leg fwd fx bidAsk points and outrights
    *  @param [in]		farLegFwdFxBidAsks	Far leg fwd bidAsk points and outrights
    *  @param [in]		pipSize				Swap point pip size
    *  @return			Return a vector of forward forward fx rates/swap points
    */
	std::vector<FxFwd> calculateFxForwardForwards(const std::vector<FxFwd>& nearLegFwdFxBidAsks, const std::vector<FxFwd>& farLegFwdFxBidAsks, const double pipSize)
    {

		std::vector<FxFwd> fwdFwdFx(2);

		if (farLegFwdFxBidAsks.size() != 0)
		{
			if (farLegFwdFxBidAsks.size() != nearLegFwdFxBidAsks.size())
			{
				throw LACoreInvalidData( "#Error: farLegFwdFxBids and nearLegFwdFxAsks must have same columns: bidPoints, askPoints, bidOutright, askOutright" , __FILE__, __LINE__ );
			}

			double farLegFwdFxPointsBid = farLegFwdFxBidAsks[0].bid;
			double farLegFwdFxPointsAsk = farLegFwdFxBidAsks[0].ask;

			double nearLegFwdFxPointsBid = nearLegFwdFxBidAsks[0].bid;
			double nearLegFwdFxPointsAsk = nearLegFwdFxBidAsks[0].ask;
			double nearLegFwdFxOutrightBid = nearLegFwdFxBidAsks[1].bid;
			double nearLegFwdFxOutrightAsk = nearLegFwdFxBidAsks[1].ask;

			// 1) FwdFwdPoint Formula:
			// fwdFwdFxPointsBid = farLegFwdFxPointsBid - nearLegFwdFxPointsAsk * pipSize
			// fwdFwdFxPointsAsk = farLegFwdFxPointsAsk - nearLegFwdFxPointsBid * pipSize

			double fwdFwdFxPointsBid = farLegFwdFxPointsBid - nearLegFwdFxPointsAsk;
			double fwdFwdFxPointsAsk = farLegFwdFxPointsAsk - nearLegFwdFxPointsBid;

			// 2) FwdFwdOutright Formula:

			// FXForwardOutright = FxSpotOutright +/- swapPoints * pipSize
			int swapPointSign =getFxSwapPointSign(nearLegFwdFxOutrightBid, nearLegFwdFxOutrightAsk);
			double factor = swapPointSign * pipSize;

			double fwdFwdFxOutrightBid = nearLegFwdFxOutrightBid + fwdFwdFxPointsBid * factor;
			double fwdFwdFxOutrightAsk = nearLegFwdFxOutrightAsk + fwdFwdFxPointsAsk * factor;

			FxFwd fxFwdPoint;
			fxFwdPoint.isOutright = false;
			fxFwdPoint.bid = fwdFwdFxPointsBid;
			fxFwdPoint.ask = fwdFwdFxPointsAsk;
			fxFwdPoint.setMidFromBidAsk();
	
			FxFwd fxFwdRate;
			fxFwdRate.isOutright = true;
			fxFwdRate.bid = fwdFwdFxOutrightBid;
			fxFwdRate.ask = fwdFwdFxOutrightAsk;
			fxFwdRate.setMidFromBidAsk();
			
			fwdFwdFx[0] = fxFwdPoint;
			fwdFwdFx[1] = fxFwdRate;
		}
		else
		{
			//To be consistent with BB, the near leg point is N/A
			FxFwd fxFwdPoint;
			fxFwdPoint.isOutright = false;
			fxFwdPoint.bid = std::numeric_limits<double>::quiet_NaN();
			fxFwdPoint.ask = std::numeric_limits<double>::quiet_NaN();
			fxFwdPoint.mid = std::numeric_limits<double>::quiet_NaN();

			FxFwd fxFwdRate;
			fxFwdRate.isOutright = true;
			fxFwdRate.bid = nearLegFwdFxBidAsks[1].bid;
			fxFwdRate.ask = nearLegFwdFxBidAsks[1].ask;
			fxFwdRate.setMidFromBidAsk();

			fwdFwdFx[0] = fxFwdPoint;
			fwdFwdFx[1] = fxFwdRate;

		}

		return fwdFwdFx;

    }

	/* @brief			Calculate a matrix of forward forward fx rates (fxSwap rates)
    *  @param [in]		nearLegFwdFxBidAskMatrix	A matrix of Near leg fwd fx bidAsk points and outrights
    *  @param [in]		nearLegFwdFxBidAskMatrix	A matrix of Far leg fwd bidAsk points and outrights
    *  @param [in]		pipSize						Swap point pip size
    *  @param [in]		columnList					Column header names to show specified columns. Default to empty list showing all columns.
    *  @return			Return a matrix of forward forward fx rates/swap points
    */
	std::vector<std::vector<FxFwd>> populateFxForwardForwards(const std::vector<std::vector<FxFwd>>& nearLegFwdFxBidAskMatrix, const std::vector<std::vector<FxFwd>>& farLegFwdFxBidAskMatrix, const double pipSize)
    {

		bool outputFarLeg = (farLegFwdFxBidAskMatrix.size() != 0);

        if (outputFarLeg && nearLegFwdFxBidAskMatrix.size() != farLegFwdFxBidAskMatrix.size())
        {
            throw LACoreInvalidData( "#Error: nearLegFwdFxBidAskMatrix and farLegFwdFxBidAskMatrix must have same number of rows" , __FILE__, __LINE__ );
        }

		std::vector<std::vector<FxFwd>> fwdFwdFxRateMatrix;

		for (size_t i = 0; i< nearLegFwdFxBidAskMatrix.size(); ++i)
		{
			auto farLegFwdFxRates = outputFarLeg ? farLegFwdFxBidAskMatrix[i] : std::vector<FxFwd>();
			
			auto fwdFwdFxRates = calculateFxForwardForwards(nearLegFwdFxBidAskMatrix[i], farLegFwdFxRates, pipSize);

			fwdFwdFxRateMatrix.push_back(fwdFwdFxRates);
		}
        
        return fwdFwdFxRateMatrix;
    }

    /* @brief			Output fx forward rates based on fxPriceEnum
    *  @param [in]		fxFwd		    Fx forwards
    *  @param [in]		columnList		Column header names to show specified columns. Default to empty list showing all columns.
    *  @return			Return a vector of forward fx rates/swap points based on the fxPriceEnum
    */
	AnyTypeVector  outputFxPriceBody(const std::vector<FxFwd>& fxFwd, const std::unordered_set<FXPriceEnum, EnumClassHash>& columnList)
    {

		if (fxFwd.size() < 2)
		{
            throw LACoreInvalidData( "fwdFxRates vector must have at least 2 items: point and outright.", __FILE__, __LINE__ );
		}

		FxFwd fxFwdPoint = fxFwd[0];
		FxFwd fxFwdRate = fxFwd[1];

		AnyTypeVector outputs;

		MLIB_PUSH_BACK_IF(outputs, fxFwdPoint.bid, includeFXPriceColumn(BID_POINTS_FXPRICE, columnList) );
		MLIB_PUSH_BACK_IF(outputs, fxFwdPoint.ask, includeFXPriceColumn(ASK_POINTS_FXPRICE, columnList) );

		MLIB_PUSH_BACK_IF(outputs, fxFwdRate.bid, includeFXPriceColumn(BID_OUTRIGHT_FXPRICE, columnList) );
		MLIB_PUSH_BACK_IF(outputs, fxFwdRate.ask, includeFXPriceColumn(ASK_OUTRIGHT_FXPRICE, columnList) );

		MLIB_PUSH_BACK_IF(outputs, fxFwdPoint.mid, includeFXPriceColumn(MID_POINTS_FXPRICE, columnList) );
		MLIB_PUSH_BACK_IF(outputs, fxFwdRate.mid, includeFXPriceColumn(MID_OUTRIGHT_FXPRICE, columnList) );

		return outputs;
     }

	/* @brief			Output fx forward/swap 
    *  @param [in]		columnList	Column header names to show specified columns. Default to empty list showing all columns.
    *  @return			Return a vector of forward fx rates/swap
    */
	AnyTypeMatrix outputFxPrice(const std::vector<std::vector<FxFwd>>& fxFwd, bool includeHeader, const std::unordered_set<FXPriceEnum, EnumClassHash>& columnList)
	{
		size_t expectedSize = fxFwd.size();

		AnyTypeMatrix body(expectedSize);

		for (size_t i = 0; i < expectedSize; ++i)
		{
			body[i] = outputFxPriceBody(fxFwd[i], columnList);
		}

		if( includeHeader )
		{
			AnyTypeMatrix result;
            AnyTypeVector headers;
			
			MLIB_PUSH_BACK_IF(headers, toString(BID_POINTS_FXPRICE), includeFXPriceColumn(BID_POINTS_FXPRICE, columnList) );
			MLIB_PUSH_BACK_IF(headers, toString(ASK_POINTS_FXPRICE), includeFXPriceColumn(ASK_POINTS_FXPRICE, columnList) );

			MLIB_PUSH_BACK_IF(headers, toString(BID_OUTRIGHT_FXPRICE), includeFXPriceColumn(BID_OUTRIGHT_FXPRICE, columnList) );
			MLIB_PUSH_BACK_IF(headers, toString(ASK_OUTRIGHT_FXPRICE), includeFXPriceColumn(ASK_OUTRIGHT_FXPRICE, columnList) );

			MLIB_PUSH_BACK_IF(headers, toString(MID_POINTS_FXPRICE), includeFXPriceColumn(MID_POINTS_FXPRICE, columnList) );
			MLIB_PUSH_BACK_IF(headers, toString(MID_OUTRIGHT_FXPRICE), includeFXPriceColumn(MID_OUTRIGHT_FXPRICE, columnList) );

			result.push_back(headers);

			//merge headers and body
	        result.insert(result.end(), body.begin(), body.end());

			return result;
		}
		else
		{
			return body;
		}
	}

	// Return a matrix of fxForwardRates where each vector include: bidSwapPoints, askSwapPoints, bidOutright, askOutright
	std::vector<std::vector<FxFwd>> calculateFxForwardsFromDiscountCurves(const std::vector<std::string>& settlementDatesOrTenors, const CurveFxFwdDataProvider& curveFxInfo, const double xccySwapRateBumpSize)
	{

		LAString baseCollection(curveFxInfo.baseCurveCollection.c_str());
		LAString termCollection(curveFxInfo.termCurveCollection.c_str());
		LAString baseDfCurve(curveFxInfo.baseDiscountCurveIndex.c_str());
		LAString termDfCurve(curveFxInfo.termDiscountCurveIndex.c_str());

		//Check if the required DF curves have been built:
		getCurveStaticDataTableName(baseCollection, baseDfCurve);
		getCurveStaticDataTableName(termCollection, termDfCurve);

		//Populate settleDates
		std::vector<LADate> settlementDates = fromSettleDatesOrTenorsToDates(settlementDatesOrTenors, curveFxInfo.asOfDate, curveFxInfo.spotDate, curveFxInfo.calendar, curveFxInfo.businessDayAdjustment, curveFxInfo.rollDayInput);

		//DF(spotDate, settleDate), base - first ccy in the currencyPair, term - second ccy in the currencyPair
		auto baseDiscountFactors = getCurveDiscountFactors(curveFxInfo.spotDate, settlementDates, baseCollection, baseDfCurve);
		auto termDiscountFactors = getCurveDiscountFactors(curveFxInfo.spotDate, settlementDates, termCollection, termDfCurve);

		//Populate allForwardFXs
		size_t expectedSize = settlementDatesOrTenors.size();
		std::vector<std::vector<FxFwd>> allForwardFXs(expectedSize);

		double epsilon = 1e-20;

		for (size_t i = 0; i < expectedSize; ++i)
		{
			const LADate& settleDate = settlementDates[i];

			double baseDF = baseDiscountFactors[i];
			double termDF = termDiscountFactors[i];

			bool isSettleTenor = isMaturityDateTenor(settlementDatesOrTenors[i].c_str());

			if (settleDate == curveFxInfo.spotDate && isSettleTenor &&	!isSpotTenor(settlementDatesOrTenors[i]))
			{
				//DF(asOf, settleDate)
				baseDF = getCurveDiscountFactors(boost::assign::list_of(settleDate), baseCollection, baseDfCurve)[0];
				termDF = getCurveDiscountFactors(boost::assign::list_of(settleDate), termCollection, termDfCurve)[0];
			}

			if (std::abs(termDF) <= epsilon)
			{
				throw LACoreInvalidData((boost::format("#Error: Cannot imply FxForward from discount curves as term discount factor is smaller than \"%s\"") % epsilon).str().c_str(), __FILE__, __LINE__);
			}

			double bidTermDF = termDF, askTermDF = termDF;

			if (!MLIB_IS_EQUAL_ZERO(xccySwapRateBumpSize))
			{
				//Analytical formula to calculate new DF' when swapRate is bumped:
				//Formula: fxFwdRate_usdrub = fxSpot_usdrub * (1+r_rub * yf_rub)/(1+r_usd *yf_usd) = fxSpot_usdrub * DF_usd/DF_rub
				// Assuming 1/(1+r*yf) = DF, we have :
				// 1) When tenor <=1Y, DF = 1/(1+ r * yf) => r' - r = (1/DF' - 1/DF)/yf 
				//=> DF' = 1/(yf * rateBump + 1/DF), where r is the xccySwapRate, DF is the xccy discount factor, yf is the year fraction from spotDate to settleDate
				// 2) When tenor > 1Y, DF = 1/(1+ r)^yf => r' - r = (DF')^(-1/yf) - (DF)^(-1/yf)
				//=> DF' = (rateBump + (DF)^(-1/yf))^(-yf)

				double yf = getYearFraction(curveFxInfo.spotDate, settleDate, curveFxInfo.dayCount);

				int years = settleDate.yearOfEra() - curveFxInfo.spotDate.yearOfEra();
				if (years <= 1)
				{
					double temp = 1.0 / termDF;
					bidTermDF = 1.0 / (temp + yf * (-xccySwapRateBumpSize));
					askTermDF = 1.0 / (temp + yf * xccySwapRateBumpSize);

					double ask = 1.0 / (yf * xccySwapRateBumpSize + 1.0 / termDF);
					double bid = 1.0 / (yf *  (-xccySwapRateBumpSize) + 1.0 / termDF);
				}
				{

					double temp = std::pow(termDF, -1.0 / yf);
					bidTermDF = std::pow((-xccySwapRateBumpSize + temp), -yf);
					askTermDF = std::pow((xccySwapRateBumpSize + temp), -yf);
				}
			}

			FxFwd fxFwdRate;
			fxFwdRate.isOutright = true;
			fxFwdRate.mid = curveFxInfo.fxSpot.mid * baseDF / termDF;;
			fxFwdRate.bid = curveFxInfo.fxSpot.bid * baseDF / bidTermDF;
			fxFwdRate.ask = curveFxInfo.fxSpot.ask * baseDF / askTermDF;

			//When using DF to calculate the fxFwd rate, the rate is in outright
			bool settleEarly = isSettleEarlierThanSpotDate(settlementDatesOrTenors[i], settleDate, curveFxInfo.spotDate);
			auto forwardFXs = populateFxForwardPointsAndOutrights(curveFxInfo.fxSpot, fxFwdRate, curveFxInfo.pipSize, settleEarly);

			allForwardFXs[i] = forwardFXs;

		}

		return allForwardFXs;


	}


	//FxConvention getFxConvention(const LabelValueBlock& curveFxConventionsLVB)
	/* @brief			populate fx fwd conventions
	*  @param [in]		curveFxConventionsLVB	curveFxConventionsLVB
	*  @param [inout]	calendar		calendar
	*  @param [inout]	businessDayAdj	business day adjustment
	*  @param [inout]	rollDayInput	rollDayInput
	*/
	void populateFxFwdConventions(const LabelValueBlock& curveFxConventionsLVB, std::string& calendar, std::string& businessDayAdjustment, std::string& rollDayInput)
	{
		calendar = curveFxConventionsLVB.getCompulsoryValue(CURVEGENERATOR_FXFWDS_KEY::CALENDAR);
		businessDayAdjustment = curveFxConventionsLVB.getCompulsoryValue(CURVEGENERATOR_FXFWDS_KEY::SLIDING_RULE);

		//TODO: Is EOM_DAY the right one to use?
		rollDayInput = curveFxConventionsLVB.getOptionalValue(CURVEGENERATOR_FXFWDS_KEY::EOM_DAY, "");
	}

	LabelValueBlock getFxConventionLVBFromFwdFxConstConv(const std::string& xccyCurveCollection, const LabelValueBlock& fwdFxConstConvLVB, const std::string& forecastCurveKey)
	{

		LabelValueBlock curveFxConventionsLVB;
		auto forecastCurve = fwdFxConstConvLVB.getCompulsoryValueAsLAString(forecastCurveKey);
		if (forecastCurve.findString(MULTI_STATIC_DATA_DELIMITER) != -1)
		{
			forecastCurve = forecastCurve.toToken(MULTI_STATIC_DATA_DELIMITER)[1];
		}
		auto xccyCurveTypeUSDCSA = toCurveTypeEnum(getCurveType(xccyCurveCollection, getCurveStaticDataTableName(xccyCurveCollection, forecastCurve)).getCString());
		if (xccyCurveTypeUSDCSA == XCCYBASIS_CURVETYPE)
		{
			curveFxConventionsLVB = CurveResultsContainer::getInstance().getCurveResults(xccyCurveCollection, forecastCurve.getCString())->curveConventionsAndMarketData()->xccyBasisCurveData()->fxFwdConvLVB_;;
		}

		return curveFxConventionsLVB;
	}


	CurveFxFwdDataProvider populateCurveFxFwdDataProvider(const std::string& baseCurveCollection,
														const std::string& baseCurveIndex,
														const std::string& termCurveCollection,
														const std::string& termCurveIndex, 
														const double fxSpotRate,
														const double pipSize)
	{

		//base - first ccy in the currencyPair, term - second ccy in the currencyPair

		auto baseCurveType = toCurveTypeEnum(getCurveType(baseCurveCollection, getCurveStaticDataTableName(baseCurveCollection, baseCurveIndex)).getCString());
		auto termCurveType = toCurveTypeEnum(getCurveType(termCurveCollection, getCurveStaticDataTableName(termCurveCollection, termCurveIndex)).getCString());

		std::string xccyCurveCollection, xccyCurveIndex;
		CurveTypeEnum xccyCurveType;

		//If the term/ 2nd Currency in the currency pair is xccyCurve, use it for the fxFwdConvention; if not try the base/1st currency in the currency pair
		if(termCurveType == XCCYBASIS_CURVETYPE || termCurveType == FWDFXCONST_CURVETYPE)
		{
			xccyCurveCollection = termCurveCollection;
			xccyCurveIndex = termCurveIndex;
			xccyCurveType = termCurveType;
		}
		else if (baseCurveType == XCCYBASIS_CURVETYPE || baseCurveType == FWDFXCONST_CURVETYPE)
		{
			xccyCurveCollection = baseCurveCollection;
			xccyCurveIndex = baseCurveIndex;
			xccyCurveType = baseCurveType;
		}
		else
		{
			throw LACoreInvalidData("#Error: either baseCurve or termCurve must be XccyBasis or FxConst Curve", __FILE__, __LINE__);
		}

		LADate curveAsOfDate = getCurveAsOfDate(xccyCurveCollection);

		MLIB_REQUIRE(isEnabledCurveResults() && doesExistCurveResultsConventionsAndMarketData(xccyCurveCollection, xccyCurveIndex), "Curve Results have been Disabled");

		LabelValueBlock curveFxConventionsLVB;
		auto curveConvMarketData = CurveResultsContainer::getInstance().getCurveResults(xccyCurveCollection, xccyCurveIndex)->curveConventionsAndMarketData();
		if (xccyCurveType == XCCYBASIS_CURVETYPE)
		{
			curveFxConventionsLVB = curveConvMarketData->xccyBasisCurveData()->fxFwdConvLVB_;
		}
		else if (xccyCurveType == FWDFXCONST_CURVETYPE)
		{
			auto fwdFxConstConvLVB = curveConvMarketData->fxFwdConstantCurveData()->fwdfxconstConvLVB_;
			if (same(fwdFxConstConvLVB.getCompulsoryValue(CURVEGENERATOR_FWDFXCONST_KEY::TARGET), "Leg2Discount"))
			{

				curveFxConventionsLVB = getFxConventionLVBFromFwdFxConstConv(xccyCurveCollection, fwdFxConstConvLVB, CURVEGENERATOR_FWDFXCONST_KEY::LEG2_FORECAST);
				if (curveFxConventionsLVB.size() == 0)
				{
					//Use curve cullection of diff leg
					auto curveCollectionToUse = same(xccyCurveCollection, baseCurveCollection) ? termCurveCollection : baseCurveCollection;
					curveFxConventionsLVB = getFxConventionLVBFromFwdFxConstConv(curveCollectionToUse, fwdFxConstConvLVB, CURVEGENERATOR_FWDFXCONST_KEY::LEG1_FORECAST);
				}
			}
			else if(same(fwdFxConstConvLVB.getCompulsoryValue(CURVEGENERATOR_FWDFXCONST_KEY::TARGET), "Leg1Discount"))
			{
				curveFxConventionsLVB = getFxConventionLVBFromFwdFxConstConv(xccyCurveCollection, fwdFxConstConvLVB, CURVEGENERATOR_FWDFXCONST_KEY::LEG1_FORECAST);
				if (curveFxConventionsLVB.size() == 0)
				{
					//Use curve cullection of diff leg
					auto curveCollectionToUse = same(xccyCurveCollection, baseCurveCollection) ? termCurveCollection : baseCurveCollection;
					curveFxConventionsLVB = getFxConventionLVBFromFwdFxConstConv(curveCollectionToUse, fwdFxConstConvLVB, CURVEGENERATOR_FWDFXCONST_KEY::LEG2_FORECAST);
				}
			}
			else 
			{
				MLIB_THROW("#Error: FWDFXCONST's Target must be either Leg2Discount or Leg1Discount");
			}
		}
				
		DayCountEnum dayCount = toDayCountEnum(getDiscountFactorDayCount().getCString());

		const std::string resetLag = curveFxConventionsLVB.getCompulsoryValue(CURVEGENERATOR_FXFWDS_KEY::RESET_LAG);

		std::string calendar, businessDayAdjustment, rollDayInput;
		populateFxFwdConventions(curveFxConventionsLVB, calendar, businessDayAdjustment, rollDayInput);

		const LADate spotDate = getDateFromTenorWithFxConvention(curveAsOfDate, resetLag, calendar, businessDayAdjustment, rollDayInput);

		CurveFxFwdDataProvider xccyCurveFxInfo;

		xccyCurveFxInfo.asOfDate = curveAsOfDate;
		xccyCurveFxInfo.spotDate = spotDate;

		xccyCurveFxInfo.calendar = calendar;
		xccyCurveFxInfo.businessDayAdjustment = businessDayAdjustment;
		xccyCurveFxInfo.rollDayInput = rollDayInput;

		xccyCurveFxInfo.dayCount = dayCount;

		FxSpotRate fxSpot;
		fxSpot.bid = fxSpotRate;
		fxSpot.mid = fxSpotRate;
		fxSpot.ask = fxSpotRate;
		xccyCurveFxInfo.fxSpot = fxSpot;

		xccyCurveFxInfo.baseCurveCollection = baseCurveCollection;
		xccyCurveFxInfo.termCurveCollection = termCurveCollection;
		xccyCurveFxInfo.baseDiscountCurveIndex = baseCurveIndex;
		xccyCurveFxInfo.termDiscountCurveIndex = termCurveIndex;
		xccyCurveFxInfo.pipSize = pipSize;

		return xccyCurveFxInfo;
	}

	CurveFxFwdDataProvider populateCurveFxFwdDataProvider(const std::shared_ptr<SingleCurveObject> xccyCurveObject)
	{

		LAString domensticDiscountCurve = xccyCurveObject->getCurveIndexName();
		if (domensticDiscountCurve.findString(MULTI_STATIC_DATA_DELIMITER) != -1)
		{
			domensticDiscountCurve = domensticDiscountCurve.toToken(MULTI_STATIC_DATA_DELIMITER)[0];
		}
		auto domensticCurveType = toCurveTypeEnum(getCurveType(xccyCurveObject->getDomesticCurveCollection(), getCurveStaticDataTableName(xccyCurveObject->getDomesticCurveCollection(), domensticDiscountCurve)).getCString());

		MLIB_REQUIRE(domensticCurveType == XCCYBASIS_CURVETYPE, "This method only supports curve object with XccyBasis curve type.");

		const LabelValueBlock curveFxConventionsLVB = xccyCurveObject->getCurveGeneratorObj()->toLabelValueBlock(toString(FXFWD_CONVENTIONS));
		const LabelValueBlock curveXccySwapConventionsLVB = xccyCurveObject->getCurveGeneratorObj()->toLabelValueBlock(toString(XCCY_BASIS_CONVENTIONS));

		LAString foreignDiscountCurve = "";

		//We use getDiscountFactorDayCount() as all curve DFs in CurveUtilities.cpp are using ACt/365.
		DayCountEnum dayCount = toDayCountEnum(getDiscountFactorDayCount().getCString());
	
		if (same(curveXccySwapConventionsLVB.getCompulsoryValue(CURVEGENERATOR_BASISSWAPS_KEY::TARGET), "Leg2Discount"))
		{
			foreignDiscountCurve = curveXccySwapConventionsLVB.getCompulsoryValue(CURVEGENERATOR_BASISSWAPS_KEY::LEG1_DISCOUNT);
		}
		else
		{
			foreignDiscountCurve = curveXccySwapConventionsLVB.getCompulsoryValue(CURVEGENERATOR_BASISSWAPS_KEY::LEG2_DISCOUNT);
		}
		if (foreignDiscountCurve.findString(MULTI_STATIC_DATA_DELIMITER) != -1)
		{
			foreignDiscountCurve = foreignDiscountCurve.toToken(MULTI_STATIC_DATA_DELIMITER)[1];
		}

		LADate curveAsOfDate = getCurveAsOfDate(xccyCurveObject->getDomesticCurveCollection());
		const std::string resetLag = curveFxConventionsLVB.getCompulsoryValue(CURVEGENERATOR_FXFWDS_KEY::RESET_LAG);

		std::string calendar, businessDayAdjustment, rollDayInput;
		populateFxFwdConventions(curveFxConventionsLVB, calendar, businessDayAdjustment, rollDayInput);

		const LADate spotDate = getDateFromTenorWithFxConvention(curveAsOfDate, resetLag, calendar, businessDayAdjustment, rollDayInput);

		auto fxSpotMatrix = xccyCurveObject->getCurveMarketDataObj()->toLAStringMatrix(toString(FXSPOT_MARKETDATA));

		const int marketDataRateColumn = CurveMarketData::findMarketDataRateColumnNumber_ForAllInstrumentTypes(FXSPOT_MARKETDATA);

		MLIB_REQUIRE(fxSpotMatrix.size() > 0 && marketDataRateColumn >= 0, "Invalid Market Data: Not enough columns in the FxSpot market data block.");

		double fxSpotRate = fxSpotMatrix[0][marketDataRateColumn].getDoubleValue();

		//e.g. RUB - domestic, USD - foreign
		const bool isFxSpotFromForeignToDomestic = curveFxConventionsLVB.getCompulsoryValueAsBool(CURVEGENERATOR_FXFWDS_KEY::IS_DOMESTIC_CURRENCY);

		double pipSize = curveFxConventionsLVB.getCompulsoryValueAsDouble(CURVEGENERATOR_FXFWDS_KEY::PIPSIZE);

		CurveFxFwdDataProvider xccyCurveFxInfo;

		xccyCurveFxInfo.asOfDate = curveAsOfDate;
		xccyCurveFxInfo.spotDate = spotDate;

		xccyCurveFxInfo.calendar = calendar;
		xccyCurveFxInfo.businessDayAdjustment = businessDayAdjustment;
		xccyCurveFxInfo.rollDayInput = rollDayInput;

		xccyCurveFxInfo.dayCount = dayCount;

		FxSpotRate fxSpot;
		fxSpot.bid = fxSpotRate;
		fxSpot.mid = fxSpotRate;
		fxSpot.ask = fxSpotRate;
		xccyCurveFxInfo.fxSpot = fxSpot;

		//base - first ccy in the currencyPair, term - second ccy in the currencyPair
		if (isFxSpotFromForeignToDomestic)
		{
			xccyCurveFxInfo.baseCurveCollection = xccyCurveObject->getForeignCurveCollection();
			xccyCurveFxInfo.termCurveCollection = xccyCurveObject->getDomesticCurveCollection();
			xccyCurveFxInfo.baseDiscountCurveIndex = foreignDiscountCurve.getCString();
			xccyCurveFxInfo.termDiscountCurveIndex = domensticDiscountCurve.getCString();
			xccyCurveFxInfo.pipSize = 1.0 / pipSize;
		}
		else
		{
			xccyCurveFxInfo.baseCurveCollection = xccyCurveObject->getDomesticCurveCollection();
			xccyCurveFxInfo.termCurveCollection = xccyCurveObject->getForeignCurveCollection();
			xccyCurveFxInfo.baseDiscountCurveIndex = domensticDiscountCurve.getCString();
			xccyCurveFxInfo.termDiscountCurveIndex = foreignDiscountCurve.getCString();
			xccyCurveFxInfo.pipSize = pipSize;
		}

		return xccyCurveFxInfo;
	}


	//Get fxSwap based on the nearLeg/farLeg settleDatesOrTenors (which are implied from base/term discount curves)
	std::vector<std::vector<FxFwd>> calculateFxSwapFromDiscountCurves(const std::vector<std::string>& nearLegSettlementDatesOrTenors,
																	const std::vector<std::string>& farLegSettlementDatesOrTenors,
																	const CurveFxFwdDataProvider& curveFxInfo,
																	bool outputFarLeg)
	{
		std::vector<std::vector<FxFwd>> nearLegFwdFxBidAskMatrix;
		std::vector<std::vector<FxFwd>> farLegFwdFxBidAskMatrix;
		if (outputFarLeg)
		{
			nearLegFwdFxBidAskMatrix = calculateFxForwardsFromDiscountCurves(nearLegSettlementDatesOrTenors, curveFxInfo);
			farLegFwdFxBidAskMatrix = calculateFxForwardsFromDiscountCurves(farLegSettlementDatesOrTenors, curveFxInfo);
		}
		else
		{
			nearLegFwdFxBidAskMatrix = calculateFxForwardsFromDiscountCurves(nearLegSettlementDatesOrTenors, curveFxInfo);
		}

		auto forwardForwardFxRates = populateFxForwardForwards(nearLegFwdFxBidAskMatrix, farLegFwdFxBidAskMatrix, curveFxInfo.pipSize);

		return forwardForwardFxRates;

	}

	double fxRateFromSpotToAsOfDate(const double spotFXRate, const LADate& spotFXDate, const LAString& baseCurveCollection, const LAString& baseCurveIndex, const LAString& termCurveCollection, const LAString& termCurveIndex)
	{
		//SPOT_FX(EUR/USD) = ASOF_FX(EUR / USD) * DF( EUR_USDCSA, 0, SPOTDATE ) / DF( USD_USDCSA, 0, SPOTDATE)
		//=>  ASOF_FX(EUR / USD)  = SPOT_FX(EUR/USD) * DF( USD_USDCSA, 0, SPOTDATE) / DF( EUR_USDCSA, 0, SPOTDATE )

		//validate the DF curves have been built, and get he asOfDate from the curves
		getCurveStaticDataTableName(baseCurveCollection, baseCurveIndex);
		getCurveStaticDataTableName(termCurveCollection, termCurveIndex);

		auto baseDF = etrading::getCurveDiscountFactors(boost::assign::list_of(spotFXDate), baseCurveCollection, baseCurveIndex)[0];
		auto termDF = etrading::getCurveDiscountFactors(boost::assign::list_of(spotFXDate), termCurveCollection, termCurveIndex)[0];

		MLIB_REQUIRE(!MLIB_IS_EQUAL_ZERO(baseDF), "base ccy DF as denominator cannot be zero.");

		//fx rate is from base to term ccy
		const double asOfDateFxRate = spotFXRate * termDF / baseDF;

		return asOfDateFxRate;
	}

	double fxRateFromAsOfDateToSpot(const double asOfDateFXRate, const LADate& spotFXDate, const LAString& baseCurveCollection, const LAString& baseCurveIndex, const LAString& termCurveCollection, const LAString& termCurveIndex)
	{
		//SPOT_FX(EUR/USD) = ASOF_FX(EUR / USD) * DF( EUR_USDCSA, 0, SPOTDATE ) / DF( USD_USDCSA, 0, SPOTDATE)

		//validate the DF curves have been built, and get he asOfDate from the curves
		getCurveStaticDataTableName(baseCurveCollection, baseCurveIndex);
		getCurveStaticDataTableName(termCurveCollection, termCurveIndex);

		auto baseDF = etrading::getCurveDiscountFactors(boost::assign::list_of(spotFXDate), baseCurveCollection, baseCurveIndex)[0];
		auto termDF = etrading::getCurveDiscountFactors(boost::assign::list_of(spotFXDate), termCurveCollection, termCurveIndex)[0];

		MLIB_REQUIRE(!MLIB_IS_EQUAL_ZERO(termDF), "term ccy DF as denominator cannot be zero.");

		//fx rate is from base to term ccy
		const double spotFX = asOfDateFXRate * baseDF / termDF;

		return spotFX;
	}


}