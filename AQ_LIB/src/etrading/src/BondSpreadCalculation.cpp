/*
* @brief			Bond Utilities for the LWO Swap object related classes
* @Created:		24th January 2017
* @Author:			Nicholas Burgess
* @Department:		MHI Quant Research and Analytics
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/


#include "BondSpreadCalculation.h"

#include "ExceptionMacros.h"
#include "FloatBondLeg.h"
#include "Solvers.h"
#include "SwapCalculation.h"
#include "SettingsValidation.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>


namespace etrading
{

	void populateValuationSettings(ValuationSettings& fixedValSetting, ValuationSettings& floatValSetting, const LegPtr& fixedLeg, const LegPtr& floatLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const bool includeAccruedInterest)
	{
		//Handle fxAsOfDate in cross currency
		double asOfDateFxRate = 1.0;
		if (fixedLeg->getStaticData()->getCurrency() != floatLeg->getStaticData()->getCurrency())
		{
			auto valuationCcy = toCCYEnum(valuationSettingsLVB.getOptionalValueAsLAString(IRS_KEY::VALUATION_CURRENCY).getCString());
			asOfDateFxRate = calculateAsOfDateFxRate(valuationSettingsLVB, valuationCcy, fixedLeg, floatLeg);
		}
		LabelValueBlock valSettingLVB(valuationSettingsLVB, VALUATION_SETTING_KEYS::FX_AS_OF_DATE_RATE, boost::lexical_cast<std::string>(asOfDateFxRate));

		// *** We need to exclude the accrued interest when solving the swap rate, because the target swapPV excludes accrued interest
		fixedValSetting = ValuationSettings(valSettingLVB, fixingTableNames, fixedLeg->getLegName(), includeAccruedInterest);
		floatValSetting = ValuationSettings(valSettingLVB, fixingTableNames, floatLeg->getLegName(), false/* includeAccruedInterest */);

	}

	//Helper function for calculateBondZSpread()
	double bondPvWithZSpread(const BondPtr& bond, const DoubleVector& zeroRates, const DoubleVector& yearFractions, const size_t& firstActiveCashflowIndex, const bool& continuouslyCompounding, const bool& includeCouponRate, const double& spread)
	{
		auto schedule = bond->getSchedule();
		auto cashflows = schedule->getAllCashflows();
		const size_t cashflowSize = schedule->getAllCashflows().size();

		double pv = 0.0;

		//Assuming spread is in bips
		const double spreadInDecimal = spread * 0.0001;

		const double simpleYearFraction = convertBondFrequencyToYearFraction(bond->getBondYieldParameters().couponFrequency_);

		//When changing the spread, we update the new discount factor
		for (size_t i = firstActiveCashflowIndex; i < cashflowSize; ++i)
		{
			auto cf = cashflows[i];

			// For zero coupon cashflow, pv is always zero, so we skip such cashflow as changing spread has no impact
			const bool isZeroCoupon = ( ( cf->getPaymentFreqEnum() == AT_MATURITY_FREQUENCY ) && ! cf->isLastCashflow() );
			
			CashflowData cashflowData;
			if (i == firstActiveCashflowIndex)
			{
				cashflowData.includeCouponRate = includeCouponRate;
			}

			if (!isZeroCoupon)
			{ 
				// year fraction from settleDate to paymentDate
				const double t = yearFractions[i];

				const double zeroRate = zeroRates[i];

				const double newZeroRate = zeroRate + spreadInDecimal;

				double newDF = 0.0;
				if (continuouslyCompounding)
				{
					//Formula: newDF = exp(- (zeroRate + spread) * t)
					newDF = std::exp((-1.0)* newZeroRate * t);
				}
				else
				{
					// Formula: DF = 1 / (1 + (zeroRate + spread)*simpleYearFraction) ^ (t/simpleYearFraction), this formula is from paper "Explaining the Lehman Brothers Option Adjusted Spread of a Corporate Bond"
					newDF = std::pow((1.0 + newZeroRate * simpleYearFraction), (-1.0) * (t / simpleYearFraction));
				}

				cashflowData.discountFactor = newDF;

				pv += cf->getCouponPv( cashflowData );
			}

		}

		// Convert the Bond PV into the Dirty Bond Price in Percent
		const double faceValue = schedule->getNotional();
		double dirtyPrice = pv / faceValue * 100.0;

		return dirtyPrice;
	}



	/* @brief			function to calculate the Bond Z-Spread from discount factor input.
	*                   This function accepts zeroRates as input to allow us to test the result against different systems.
	*  @param [in]		bondDirtyPrice		Bond Dirty Price
	*  @param [in]		bond				Pointer to the Bond Object
	*  @param [in]		zeroRates			External ZeroRates
	*  @param [in]		settleDate	        Bond Settlement Date
	*  @param [in]		continuouslyCompounding	        True to calculate z-spread using continouslyCompounding
	*  @return			returns the Z-Spread
	*/
	double calculateBondZSpread(const double& bondDirtyPrice, const std::shared_ptr< Bond >& bond, const DoubleVector& zeroRates, const LADate& settleDate, const bool& continuouslyCompounding)
	{
		// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
		const std::shared_ptr<BondSchedule>& schedule = std::static_pointer_cast<BondSchedule>(bond->getSchedule());

		const auto allCashflows = schedule->getAllCashflows();

		size_t cashflowSize = allCashflows.size();

		//Get DFs from the discount curve
		auto paymentDates = schedule->getAllPaymentDates();

		MLIB_REQUIRE(cashflowSize == zeroRates.size() && cashflowSize == paymentDates.size(), "Cashflows, DiscountFactors, and paymentDates should have the same size.");

		//Discount factor's year fraction: If it's continuouslyCompounding, CURVE's convention is used; otherwise, BOND's convention is used:
		const auto dfDayCount = continuouslyCompounding ? toDayCountEnum(getDiscountFactorDayCount().getCString()) : bond->getSchedule()->getAccrualDaycount();
		
		// Get the Bond's active cashflow index (first bond coupon that is not in the past)
		const size_t firstActiveCashflowIndex = getBondActiveCashflowIndex(settleDate, paymentDates);

		// Newton-Raphson Solver Settings
		double initialGuessForSpread = 0.0;
		const double tolerance = 1e-8;
		const unsigned int maxIterations = 1000;
		const double shiftSize = 0.0000001;

		double targetPV = bondDirtyPrice;

		// One-dimensional objective function used by the solver:
		// This lambda function captures the settlementDate and yieldCalcType as fixed parameters.
		// The spead is the variable which the solver will adjust in order to obtain the targetPV.

		// year fractions from settleDate to paymentDates
		DoubleVector yearFractions(cashflowSize);

		for (size_t i = 0; i < cashflowSize; ++i)
		{
			// year fraction from settleDate to paymentDate
			const double paymentYearFraction = (i >= firstActiveCashflowIndex) ? getYearFraction(settleDate, paymentDates[i], dfDayCount, false) : 0.0;

			yearFractions[i] = paymentYearFraction;
		}

		auto firstActiveCF = allCashflows.at(firstActiveCashflowIndex);

		bool includeCouponRate = schedule->getFirstActiveCashflowIncludeCouponRate(firstActiveCF, settleDate);

		auto function = [bond, zeroRates, yearFractions, firstActiveCashflowIndex, continuouslyCompounding, includeCouponRate, targetPV](const double spread)
		{
			return bondPvWithZSpread(bond, zeroRates, yearFractions, firstActiveCashflowIndex, continuouslyCompounding, includeCouponRate, spread);
		};

		// Solver Results Contain: Solution, nInterations and Jacobian
		const double spread = solvers::newtonRaphson(function, targetPV, initialGuessForSpread, tolerance, maxIterations, shiftSize).solution;

		return spread;
	}



	/* @brief			function to calculate the Bond Z-Spread
	*  @param [in]		bondDirtyPrice		Bond Dirty Price
	*  @param [in]		bond			    Pointer to the Bond Object
	*  @param [in]		curveCollection	    The name of the curve collection set
	*  @param [in]		forecastCurve       The forecast curve name
	*  @param [in]		settleDate	        Bond Settlement Date
	*  @param [in]		continuouslyCompounding	        True to calculate z-spread using continouslyCompounding
	*  @return			returns the Z-Spread
	*/
	double calculateBondZSpread(const double& bondDirtyPrice, const std::shared_ptr< Bond >& bond, const std::string& curveCollection, const std::string& forecastCurve, const LADate& settleDate, const bool& continuouslyCompounding)
	{

		// Throw proper exception when the curve has not been built
		getCurveStaticDataTableName(curveCollection.c_str(), forecastCurve.c_str());

		//Get DFs between curve's asOfDate to paymentDate from the forecast curve
		auto paymentDates = bond->getSchedule()->getAllPaymentDates();

		// Get the Bond's active cashflow index (first bond coupon that is not in the past)
		const size_t firstActiveCashflowIndex = getBondActiveCashflowIndex(settleDate, paymentDates);

		// Get the DF from the forecastCurve (normally libor) curve, the DF is from curve's settleDate to paymentDate, as seen at asOfDate
		const auto discountFactors = getCurveDiscountFactorsFromBaseDate(firstActiveCashflowIndex, settleDate, paymentDates, curveCollection.c_str(), forecastCurve.c_str());

		const size_t cashflowSize = paymentDates.size();

		//Discount factor's year fraction: If it's continuouslyCompounding, CURVE's convention is used; otherwise, BOND's convention is used:
		const auto dfDayCount = continuouslyCompounding ? toDayCountEnum(getDiscountFactorDayCount().getCString()) : bond->getSchedule()->getAccrualDaycount();
		
		const double simpleYearFraction = convertBondFrequencyToYearFraction(bond->getBondYieldParameters().couponFrequency_);


		DoubleVector zeroRates(cashflowSize);
		for (size_t i = 0; i < cashflowSize; ++i)
		{
			if (i >= firstActiveCashflowIndex)
			{
				const LADate paymentDt = paymentDates[i];

				const double df = discountFactors[i];

				if (settleDate != paymentDt)
				{
					const double t = getYearFraction(settleDate, paymentDt, dfDayCount, false);

					if (continuouslyCompounding)
					{
						// Continuously compounding: DF(0,t) = exp(-zeroRate * t) => zeroRate = -1/t*ln(DF):
						zeroRates[i] = (-1.0) / t * log(df);
					}
					else
					{
						// Discrete compounding: DF = 1 / (1 + (zeroRate + spread)*simpleYearFraction) ^ (t/simpleYearFraction), 
						// => zeroRate = (DF ^(- simpleYearFraction/t) - 1) / simpleYearFraction
						zeroRates[i] = (std::pow(df, (-1.0 * simpleYearFraction / t)) - 1.0) / simpleYearFraction;
					}
				}
				else
				{
					zeroRates[i] = 0.0;
				}
			}
			else
			{
				zeroRates[i] = 0.0;
			}
		}

		const double spread = calculateBondZSpread(bondDirtyPrice, bond, zeroRates, settleDate, continuouslyCompounding);

		return spread;
	}


	double calculateAssetSwapSpreadParParFromBondCleanPrice(const double& bondCleanPrice, const LegPtr& fixedLeg, const LegPtr& floatLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{

		const double Par = 100.0;

		// parParAdjustment has the same sign of swap's floatLeg:
		const int floatLegSign = floatLeg->getSchedule()->getPayRecIndicator();

		// Formula: parParAdjustment = (Par - BondCleanPrice)/Par * FaceValue * sign
		const double absNotional = std::fabs(fixedLeg->getSchedule()->getNotional());
		const double parParAdjustment = (Par - bondCleanPrice) / Par * absNotional * floatLegSign;

		// ASW spread is the swap float leg's spread based on swapPV as parParAdjustment
		// When calculating ASW spread, we always use clean swap pv to solve the spread (excluded interest), in Bloomberg, the target value is Premium as parparAdjustment. 
		ValuationSettings fixedVal;
		ValuationSettings floatVal;
		populateValuationSettings(fixedVal, floatVal, fixedLeg, floatLeg, valuationSettingsLVB, fixingTableNames, false);

		DataProvider fixedDataProvider(fixedVal);
		DataProvider floatDataProvider(floatVal);

		const double aswSpread = calculateFloatSpreadFromTargetSwapPV(fixedDataProvider, floatDataProvider, fixedLeg, floatLeg, parParAdjustment);

		return aswSpread;
	}

	double calculateBondCleanPriceFromAssetSwapSpread(const double& assetSwapParParSpread, const SwapPtr& origSwap, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{

		// parParAdjustment has the same sign of swap's floatLeg
		// parParAdjustment = (Par - BondCleanPrice)/Par * FaceValue * floatLegSign => BondCleanPrice = Par - Par * parParAdjustment/ (FactValue * floatLegSign), here parParAdjustment equals to the assetSwapPV when the assetSwapParParSpread is applied to the swap.
		// So, BondCleanPrice = Par - Par * assetSwapPV/ (FactValue * floatLegSign)

		const double Par = 100.0;

		auto swapFixedLeg = getFixedLeg(origSwap);
		auto swapFloatLeg = getFloatLeg(origSwap);

		// 1) Validate Bond's MaturityDate, Coupon, DayCount, Frequency are matching Swap's Fixed Leg
		bool isFixedFloatSwap = (swapFixedLeg->getType() == etrading::SWAPSCHEDULE_FIXEDBOND && swapFloatLeg->getType() == etrading::FLOAT_SCHEDULE_TYPE);

        MLIB_REQUIRE( isFixedFloatSwap, "Invalid Asset Swap Set-Up - Asset Swaps must be contain a FixedBond and Float Leg" )

		//Assume bond and swap's fixed leg has the same notional
		const double absNotional = std::fabs(swapFixedLeg->getSchedule()->getNotional());

		// Clean price (exclude accrued interest)
		ValuationSettings fixedVal;
		ValuationSettings floatVal;
		populateValuationSettings(fixedVal, floatVal, swapFixedLeg, swapFloatLeg, valuationSettingsLVB, fixingTableNames, false);

		DataProvider fixedDataProvider(fixedVal);
		DataProvider floatDataProvider(floatVal);

		//Override the float spread so that the cashflow compound rate can use the given spread
		floatDataProvider.setFloatSpreadOverride(assetSwapParParSpread);

		//fixed leg pv
		double assetSwapPV = swapFixedLeg->pv(fixedDataProvider);
		
		// fixed leg pv  + float leg pv
		assetSwapPV += swapFloatLeg->pv(floatDataProvider);

		MLIB_REQUIRE( !MLIB_IS_EQUAL_ZERO(absNotional), "Bond Notional must not be zero" )

		// BondCleanPrice = Par - Par * assetSwapPV/ (FactValue * floatLegSign)
		const double bondCleanPrice = Par - Par * assetSwapPV / (absNotional * swapFloatLeg->getSchedule()->getPayRecIndicator());

		return bondCleanPrice;
	}

	// Calculate the Floating Bond asset swap's FixedEqvCoupon from AssetSwapSpread - by solving the swap fixed leg's swap rate so that the swap PV zero
	double calculateFixedEqvCouponFromFloatingBondAssetSwapSpread(const double& assetSwapParParSpread, const SwapPtr& origSwap, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{

		auto fixedLeg = getFixedLeg(origSwap);
		auto floatLeg = getFloatLeg(origSwap);

		bool isFixedFloatSwap = (fixedLeg->getType() == etrading::FIXED_SCHEDULE_TYPE && floatLeg->getType() == etrading::SWAPSCHEDULE_FLOATBOND);

		MLIB_REQUIRE(isFixedFloatSwap, "Invalid Asset Swap Set-Up - Asset Swaps must be contain a Fixed Leg and Float Bond Leg")

		ValuationSettings fixedVal;
		ValuationSettings floatVal;
		populateValuationSettings(fixedVal, floatVal, fixedLeg, floatLeg, valuationSettingsLVB, fixingTableNames, false);

		DataProvider dataProviderFixedLeg(fixedVal);
		DataProvider dataProviderFloatLeg(floatVal);

		dataProviderFloatLeg.setFloatSpreadOverride(assetSwapParParSpread);

		// Solve the swapRate given swapPV zero, when provide the parparAdjustment
		const double targetSwapPV = 0.0;
		const double swapRate = calculateSwapRateFromTargetSwapPV(dataProviderFixedLeg, dataProviderFloatLeg, fixedLeg, floatLeg, targetSwapPV);

		return swapRate;
	}

	// Calculate the Floating Bond Clean Price from AssetSwapSpread, so that the swap PV matches parParAjustment, given the solved FixedEqvCoupon and float leg's spread as quotedMargin
	double calculateBondCleanPriceFromFloatingBondAssetSwapSpread(const double& assetSwapParParSpread, const SwapPtr& origSwap, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{
		// Step 1) Solve the swap fixed leg's swap rate so that the swap PV zero, given the ASWSpread in the float leg's spread
		const double fixedEqvCoupon = calculateFixedEqvCouponFromFloatingBondAssetSwapSpread(assetSwapParParSpread, origSwap, valuationSettingsLVB, fixingTableNames);

		// Step 2) Calculate the Floating Bond Clean Price from AssetSwapSpread, so that the swap PV matches parParAjustment, given the solved FixedEqvCoupon and float leg's spread as quotedMargin
		auto swapFixedLeg = getFixedLeg(origSwap);
		auto swapFloatLeg = getFloatLeg(origSwap);

		ValuationSettings fixedVal;
		ValuationSettings floatVal;
		// Clean price (exclude accrued interest)
		populateValuationSettings(fixedVal, floatVal, swapFixedLeg, swapFloatLeg, valuationSettingsLVB, fixingTableNames, false);

		DataProvider fixedDataProvider(fixedVal);
		DataProvider floatDataProvider(floatVal);


		fixedDataProvider.setCompoundRateOverride(fixedEqvCoupon);

		// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
		const std::shared_ptr<FloatBondLegSchedule>& bondFloatSchedule = std::static_pointer_cast<FloatBondLegSchedule>(swapFloatLeg->getSchedule());
		//Override the float spread 
		floatDataProvider.setFloatSpreadOverride(bondFloatSchedule->getQuotedMargin());

		// Formula: parParAdjustment = (Par - BondCleanPrice)/Par * FaceValue * sign,  parParAdjustment has the same sign of swap's FIXED leg:
		// => BondCleanPrice = Par - Par * assetSwapPV/ (FactValue * fixedLegSign)

		//fixed leg pv
		double assetSwapPV = swapFixedLeg->pv(fixedDataProvider);

		// fixed leg pv  + float leg pv
		assetSwapPV += swapFloatLeg->pv(floatDataProvider);

		const int fixedLegSign = swapFixedLeg->getSchedule()->getPayRecIndicator();

		const double Par = 100.0;
		//Assume bond and swap's fixed leg has the same notional
		const double absNotional = std::fabs(swapFixedLeg->getSchedule()->getNotional());
		MLIB_REQUIRE(!MLIB_IS_EQUAL_ZERO(absNotional), "Bond Notional must not be zero")

		const double bondCleanPrice = Par - Par * assetSwapPV / (absNotional * swapFixedLeg->getSchedule()->getPayRecIndicator());
		return bondCleanPrice;
	}

	// Calculate the Floating Bond asset swap's FixedEqvCoupon - the swap fixed leg's swap rate so that the swap PV (excluding accrued interest) matches the bond's parParAdjustment
	double calculateFloatingBondAssetSwapFixedEqvCoupon(const double& bondCleanPrice, const LegPtr& fixedLeg, const LegPtr& floatLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{

		// Solve for the swap fixed leg's fixedRate, use the parparAdjustment as the swap pv.
		const double Par = 100.0;

		// Formula: parParAdjustment = (Par - BondCleanPrice)/Par * FaceValue * sign,  parParAdjustment has the same sign of swap's FIXED leg:
		const int fixedLegSign = fixedLeg->getSchedule()->getPayRecIndicator();
		const double parParAdjustment = (Par - bondCleanPrice) / Par * std::fabs(floatLeg->getSchedule()->getNotional()) * fixedLegSign;

		// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
		const std::shared_ptr<FloatBondLegSchedule>& bondFloatSchedule = std::static_pointer_cast<FloatBondLegSchedule>(floatLeg->getSchedule());
		const double floatSpreadOverride = bondFloatSchedule->getQuotedMargin();

		// *** We need to exclude the accrued interest when solving the swap rate, because the target swapPV excludes accrued interest
		ValuationSettings fixedVal;
		ValuationSettings floatVal;
		populateValuationSettings(fixedVal, floatVal, fixedLeg, floatLeg, valuationSettingsLVB, fixingTableNames, false);

		DataProvider dataProviderFixedLeg(fixedVal);
		DataProvider dataProviderFloatLeg(floatVal);

		dataProviderFloatLeg.setFloatSpreadOverride(floatSpreadOverride);

		const double swapRate = calculateSwapRateFromTargetSwapPV(dataProviderFixedLeg, dataProviderFloatLeg, fixedLeg, floatLeg, parParAdjustment);

		return swapRate;
	}

	// Calculate the Floating Bond asset swap spread based on the given FixedEqvCoupon
	double calculateFloatingBondAssetSwapSpreadFromFixedEqvCoupon(const double& fixedRate, const LegPtr& fixedLeg, const LegPtr& floatLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{

		// ASW spread is the swap float leg's spread based on swapPV as parParAdjustment
		// When calculating ASW spread, we always use clean swap pv to solve the spread (excluded interest), in Bloomberg, the target value is Premium as parparAdjustment. 
		ValuationSettings fixedVal;
		ValuationSettings floatVal;
		populateValuationSettings(fixedVal, floatVal, fixedLeg, floatLeg, valuationSettingsLVB, fixingTableNames, false);

		DataProvider dataProviderFixedLeg(fixedVal);
		DataProvider dataProviderFloatLeg(floatVal);

		dataProviderFixedLeg.setCompoundRateOverride(fixedRate);

		// ASW spread is the swap float leg's spread based on swapPV (excluding accrued interest) as ZERO
		const double SWAP_TARGET_PV = 0.0;
		const double aswSpread = calculateFloatSpreadFromTargetSwapPV(dataProviderFixedLeg, dataProviderFloatLeg, fixedLeg, floatLeg, SWAP_TARGET_PV);

		return aswSpread;
	}


	// Calculate the Floating Bond asset swap spread
	double calculateFloatingBondAssetSwapSpread(const double& bondCleanPrice, const LegPtr& origFixedLeg, const LegPtr& origFloatLeg, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{

		/***
		For an asset swap, we always RECEIVE cashflows from the bond, so the PAY leg of the swap has to match the bond (what we receive)
		To calculate the ASW spread on a floating bond, the swap's floating leg will match the floating bond. Two steps to find the ASW spread:
		1) Solve for the swap fixed leg's fixedRate, use the parparAdjustment as the target swap pv (excluding accrued interest).
		2) Solve for the ASW spread: Use the fixedRate calculated from step 1), and use ZERO as the target swap pv (excluding accrued interest) to solve for swap float leg's spread. This spread is the ASW spread.
		***/

		// Step 1): Solve for the swap fixed leg's fixedRate, use the parparAdjustment as the swap pv.
		const double swapRate = calculateFloatingBondAssetSwapFixedEqvCoupon(bondCleanPrice, origFixedLeg, origFloatLeg, valuationSettingsLVB, fixingTableNames);

		// Step 2): Solve for the ASW spread : Use the fixedRate calculated from step 1), and use ZERO as the swap pv to solve for swap float leg's spread. This spread is the ASW spread.
		const double aswSpread = calculateFloatingBondAssetSwapSpreadFromFixedEqvCoupon(swapRate, origFixedLeg, origFloatLeg, valuationSettingsLVB, fixingTableNames);

		return aswSpread;
	}

	//Helper function to solve either fixed rate or float spread so that the swap PV match the targetSwapPV
	double calculateSwapPvByChangingSpread(DataProvider& spreadDataProvider, const double& refLegPV, const LegPtr& spreadLeg, const double& spread)
	{

		setSwapLegSpreadOrFixedRate(spreadDataProvider, spreadLeg->getType(), spread);

		double spreadLegPV = spreadLeg->pv(spreadDataProvider, false/* nativeCurrency */, false/* updateCurveData */);

		double totalPV = spreadLegPV + refLegPV;
		return totalPV;
	}

	// Helper function to solve the spread of the float leg, given target SwapPV
	double calculateFloatSpreadFromTargetSwapPV(DataProvider& dataProviderRefLeg, DataProvider& dataProviderSpreadLeg, const LegPtr& refLeg, const LegPtr& spreadLeg, const double& targetSwapPV)
	{

		double refLegPV = refLeg->pv(dataProviderRefLeg);

		double spreadLegPV = spreadLeg->pv(dataProviderSpreadLeg);

		double swapPVWithoutAswSpread = refLegPV + spreadLegPV;

		double floatAnnuityWithSign = spreadLeg->annuityWithNotional(dataProviderSpreadLeg) * spreadLeg->getSchedule()->getPayRecIndicator();

		MLIB_REQUIRE(floatAnnuityWithSign != 0, "Unable to calculate the Asset Swap Spread - The float leg specified has a zero PV and/or zero float Annuity value.")

		// Newton-Raphson Solver Settings
		// Initial Guess for Spread using Analytical formula: for Par Par Swap Spread we require: SwapPV = SwapPVWithoutAswSpread + aswSpread * floatAnnuityWithSign,
		double initialGuessForSpread = (targetSwapPV - swapPVWithoutAswSpread) / floatAnnuityWithSign * 10000.0;

		// Reference leg's notional is the same as the bond's notional
		const double absNotional = std::fabs(refLeg->getSchedule()->getNotional());

		const double tolerance = 1e-12 * absNotional; // *** Important *** Normalize the tolerance by the trade notional, otherwise tolerance is not fixed.

		const unsigned int maxIterations = 1000;
		const double shiftSize = 0.0000001;

		// One-dimensional objective function used by the solver:
		// This lambda function captures the settlementDate and yieldCalcType as fixed parameters.
		// The spead is the variable which the solver will adjust in order to obtain the targetSwapPV.
		auto function = [&](const double spread)
		{
			return calculateSwapPvByChangingSpread(dataProviderSpreadLeg, refLegPV, spreadLeg, spread);
		};

		// Solver Results Contain: Solution, nInterations and Jacobian
		const auto result = solvers::newtonRaphson(function, targetSwapPV, initialGuessForSpread, tolerance, maxIterations, shiftSize);

		const double spread = result.solution;

		return spread;
	}

	// Here float leg is the reference leg
	double calculateSwapRateFromTargetSwapPV(DataProvider& dataProviderFixedLeg, DataProvider& dataProviderFloatLeg, const LegPtr& fixedLeg, const LegPtr& floatLeg, const double& targetSwapPV)
	{
		// Step 1): Solve for the swap fixed leg's fixedRate, use the parparAdjustment as the swap pv.

		// Formula: parParAdjustment = (Par - BondCleanPrice)/Par * FaceValue * sign,  parParAdjustment has the same sign of swap's FIXED leg:
		const int fixedLegSign = fixedLeg->getSchedule()->getPayRecIndicator();


		/*** 1) SwapRate Formula:
		parParAdjustment	= swapPV_NO_ACCRUED_INT
		= fixedPV_NO_ACCRUED_INT + floatPV_NO_ACCRUED_INT
		= (swapRate*fixedLegAnnuity - swapRate* sumOfFixedLegAccrualYearFractions) + floatPV_NO_ACCRUED_INT
		=> swapRate			= (parParAdjustment - floatPV_NO_ACCRUED_INT)/ (fixedLegAnnuity - sumOfFixedLegAccrualYearFractions)
		***/

		// 1.1) numerator: (parParAdjustment - floatPV_NO_ACCRUED_INT)

		const double floatLegPVNoAccruedInterest = floatLeg->pv(dataProviderFloatLeg);

		const double numerator = targetSwapPV - floatLegPVNoAccruedInterest;

		// 1.2) denominator: fixedLegAnnuity - sumOfFixedLegAccrualYearFractions
		const double fixedLegAnnuity = fixedLeg->annuityWithNotional(dataProviderFixedLeg) * fixedLegSign;

		// Set the fixRate to 1, so that we can get the sum of the accrued year fractions
		dataProviderFixedLeg.setCompoundRateOverride(1.0);

		fixedLeg->initializeDataProvider(dataProviderFixedLeg);
		const double sumOfFixedLegAccruedYearFractions = fixedLeg->getSchedule()->calculateAccruedInterest(dataProviderFixedLeg, false/* nativeCurrency */);

		const double denominator = (fixedLegAnnuity - sumOfFixedLegAccruedYearFractions) * fixedLeg->getSchedule()->getPayRecIndicator();

		// 1.3) swapRate = (parParAdjustment - floatPV_NO_ACCRUED_INT) / (fixedLegAnnuity - sumOfFixedLegAccrualYearFractions)
		const double initialSwapRate = numerator / denominator;

		// Newton-Raphson Solver Settings

		// Scale to Bps as calculateSwapPvByChangingSpread() function is commonly used by fixedLeg's swapRate solving and floatLeg's spread solving
		double initialGuessForSpreadInBps = initialSwapRate * 10000.0;

		// Reference leg's notional is the same as the bond's notional
		const double absNotional = std::fabs(floatLeg->getSchedule()->getNotional());
		const double tolerance = 1e-12 * absNotional; // *** Important *** Normalize the tolerance by the trade notional, otherwise tolerance is not fixed.

		const unsigned int maxIterations = 1000;
		const double shiftSize = 0.0000001;


		// One-dimensional objective function used by the solver:
		// This lambda function captures the settlementDate and yieldCalcType as fixed parameters.
		// The spead is the variable which the solver will adjust in order to obtain the targetSwapPV.
		auto function = [&](const double fixedRate)
		{
			return calculateSwapPvByChangingSpread(dataProviderFixedLeg, floatLegPVNoAccruedInterest, fixedLeg, fixedRate);
		};

		// Solver Results Contain: Solution, nInterations and Jacobian
		double swapRate = solvers::newtonRaphson(function, targetSwapPV, initialGuessForSpreadInBps, tolerance, maxIterations, shiftSize).solution;

		//scale back from Bps to raw number
		swapRate *= 0.0001;

		return swapRate;

	}


}