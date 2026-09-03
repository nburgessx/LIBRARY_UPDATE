#pragma once

#include "LACoreTemplateType.h"
#include "CoreEnumerations.h"
#include "LabelValueBlock.h"
#include "SingleCurveObject.h"


namespace etrading
{

	struct FxFwd
	{
		double mid;
		double bid;
		double ask;
		bool isOutright;

		void setMidFromBidAsk()
		{
			mid = (bid + ask ) * 0.5;
		}
	};

	struct FxSpotRate
	{
		double mid;
		double bid;
		double ask;

		void setMidFromBidAsk()
		{
			mid = (bid + ask) * 0.5;
		}
	};

	struct CurveFxFwdDataProvider
	{
		LADate asOfDate;
		LADate spotDate;

		//base - first ccy in the currencyPair, term - second ccy in the currencyPair
		std::string baseCurveCollection;
		std::string termCurveCollection;
		std::string baseDiscountCurveIndex;
		std::string termDiscountCurveIndex;
		double pipSize;

		std::string calendar;
		std::string businessDayAdjustment;
		std::string rollDayInput;
		DayCountEnum dayCount;

		FxSpotRate fxSpot;

	};

	/* @brief			Populate the fx forward bid/ask points and bid/ask outrights based on the quoted spot rate and fx forward rate
	*  @param [in]		fxSpot	    Spot fx rate
	*  @param [in]		fxFwd	    Forward fx
	*  @param [in]		pipSize				Swap point pip size
	*  @param [in]		settleEarlyThanSpot	True to indicate settleDate earlier than spotDate
	*  @return			Return a vector of forward fx rates/swap points
	*/
	std::vector<FxFwd> populateFxForwardPointsAndOutrights(const FxSpotRate& fxSpot, const FxFwd& fxFwd, const double pipSize, const bool settleEarlyThanSpot);

	/* @brief			Calculate forward forward fx rates (fxSwap rates)
    *  @param [in]		nearLegFwdFxBidAsks Near leg fwd fx bidAsk points and outrights
    *  @param [in]		farLegFwdFxBidAsks	Far leg fwd bidAsk points and outrights
    *  @param [in]		pipSize				Swap point pip size
	*  @return			Return a vector of forward forward fx rates/swap points
    */
	std::vector<FxFwd> calculateFxForwardForwards(const std::vector<FxFwd>& nearLegFwdFxBidAsks, const std::vector<FxFwd>& farLegFwdFxBidAsks, const double pipSize);

	/* @brief			Calculate a matrix of forward forward fx rates (fxSwap rates)
    *  @param [in]		nearLegFwdFxBidAskMatrix	A matrix of Near leg fwd fx bidAsk points and outrights
    *  @param [in]		nearLegFwdFxBidAskMatrix	A matrix of Far leg fwd bidAsk points and outrights
    *  @param [in]		pipSize						Swap point pip size
    *  @param [in]		columnList					Column header names to show specified columns. Default to empty list showing all columns.
    *  @return			Return a matrix of forward forward fx rates/swap points
    */
	std::vector<std::vector<FxFwd>> populateFxForwardForwards(const std::vector<std::vector<FxFwd>>& nearLegFwdFxBidAskMatrix, const std::vector<std::vector<FxFwd>>& farLegFwdFxBidAskMatrix, const double pipSize);

	/* @brief			Output fx forward rates based on fxPriceEnum
	*  @param [in]		fxFwd		    Fx forwards
	*  @param [in]		columnList		Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Return a vector of forward fx rates/swap points based on the fxPriceEnum
	*/
	AnyTypeVector outputFxPriceBody(const std::vector<FxFwd>& fxFwd, const std::unordered_set<FXPriceEnum, EnumClassHash>& columnList);

	/* @brief			Output fx forward/swap 
    *  @param [in]		columnList	Column header names to show specified columns. Default to empty list showing all columns.
    *  @return			Return a vector of forward fx rates/swap
    */
	AnyTypeMatrix outputFxPrice(const std::vector<std::vector<FxFwd>>& fxFwd, bool includeHeader, const std::unordered_set<FXPriceEnum, EnumClassHash>& columnList);


	LADate getDateFromTenorWithFxConvention(const LADate& fromDate, const std::string& tenor, const std::string& calendar, const std::string& businessDayAdjustment, const std::string& rollDayInput);

	LADate fromSettleDateOrTenorToDate(const std::string& settlementDateOrTenor, const LADate& asOfDate, const LADate& spotDate, const std::string& calendar, const std::string& businessDayAdjustment, const std::string& rollDayInput);

	//This is to control the sign of the calculation of fx forward points/ rates
   bool isSettleEarlierThanSpotDate(const std::string& settleDateOrTenor, const LADate& settleDate, const LADate& spotDate);

   // Return a matrix of fxForwardRates where each vector include: bidSwapPoints, askSwapPoints, bidOutright, askOutright
   std::vector<std::vector<FxFwd>> calculateFxForwardsFromDiscountCurves(const std::vector<std::string>& settlementDatesOrTenors, const CurveFxFwdDataProvider& curveFxFwdDataProvider, const double xccySwapRateBumpSize = 0.0);

   /* @brief			populate fx fwd conventions
   *  @param [in]		curveFxConventionsLVB	curveFxConventionsLVB
   *  @param [inout]	calendar		calendar
   *  @param [inout]	businessDayAdj	business day adjustment
   *  @param [inout]	rollDayInput	rollDayInput
   */
   void populateFxFwdConventions(const LabelValueBlock& curveFxConventionsLVB, std::string& calendar, std::string& businessDayAdjustment, std::string& rollDayInput);

   CurveFxFwdDataProvider populateCurveFxFwdDataProvider(const std::string& baseCurveCollection,
														const std::string& baseCurveIndex,
														const std::string& termCurveCollection,
														const std::string& termCurveIndex,
														const double fxSpotRate,
														const double pipSize);

   CurveFxFwdDataProvider populateCurveFxFwdDataProvider(const std::shared_ptr<etrading::SingleCurveObject> xccyCurveObject);

   //Get fxSwap based on the nearLeg/farLeg settleDatesOrTenors (which are implied from base/term discount curves)
   std::vector<std::vector<FxFwd>> calculateFxSwapFromDiscountCurves(const std::vector<std::string>& nearLegSettlementDatesOrTenors,
																   const std::vector<std::string>& farLegSettlementDatesOrTenors,
																   const CurveFxFwdDataProvider& curveFxFwdDataProvider,
																   bool outputFarLeg);

   //BaseCcy is the first ccy in the currecy pair, TermCcy is the 2nd ccy in the currency pair
   double fxRateFromSpotToAsOfDate(const double spotFXRate, const LADate& spotFXDate, const LAString& baseCurveCollection, const LAString& baseDiscountCurveIndex, const LAString& termCurveCollection, const LAString& termDiscountCurveIndex);

   //BaseCcy is the first ccy in the currecy pair, TermCcy is the 2nd ccy in the currency pair
   double fxRateFromAsOfDateToSpot(const double asOfDateFXRate, const LADate& spotFXDate, const LAString& baseCurveCollection, const LAString& baseDiscountCurveIndex, const LAString& termCurveCollection, const LAString& termDiscountCurveIndex);

}


