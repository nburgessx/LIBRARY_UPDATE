		/*
 * @brief			Class the defines the schedule 
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "BondSchedule.h"
#include "ParameterValidation.h"
#include "SwapValidation.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "LACurvePricingObject.h"
#include "LACurveForwardRateHelpers.h"
#include "BondAccrualPeriods.h"
#include "BondUtilities.h"
#include "SwapUtilities.h"
#include "FixedBondCashflow.h"
#include <string> 

namespace etrading
{

	BondSchedule::BondSchedule(const std::string& instanceName) : Schedule(instanceName),
		bondCalculationType_(NO_CALCULATION_TYPE),
		issueDate_(""),
		issuePrice_(std::numeric_limits<double>::quiet_NaN()),
		taxRate_(std::numeric_limits<double>::quiet_NaN()),
		yieldFrequency_(NONE_FREQUENCY),
		bondTrueYieldYearFractions_(DoubleVector()),
		exDividendTenor_(""),
		exDividendBusinessDayAdj_(NONE_BUSINESS_DAY_ADJ),
		exDividendDates_(DateVector()) //calculated dates based on exDividendDays 
	{}

	BondSchedule::BondSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : Schedule(instanceName)
	{}

	void BondSchedule::populateAccrualStartDates(const LabelValueBlock& scheduleLVB)
	{
		const std::string inputLVB = "scheduleLVB";

		//A bond always have an issue date
		issueDate_ = scheduleLVB.getCompulsoryValueAsLAString(BOND_KEY::ISSUE_DATE, inputLVB);

		//A zero coupon bond has no accrual start date. If accrual started date is specified then use it; otherwise use issue date
		accrualStartDate_ = scheduleLVB.getOptionalValueAsLAString(BOND_KEY::ACCRUAL_START_DATE, issueDate_);
	}

	void BondSchedule::populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB)
	{
		Schedule::populateNotionalAndPaymentFreqEnum();

		if (scheduleLVB.size() == 0)
		{
			return;
		}

		notional_ = scheduleLVB.getOptionalValueAsDoubleFromKeys(IRS_KEY::NOTIONAL, BOND_KEY::FACE_VALUE, std::numeric_limits<double>::quiet_NaN());

		if (boost::math::isnan(notional_))
		{
			throw LACoreInvalidData("#Error: Notional is a mandatory field for PremiumSchedule", __FILE__, __LINE__);
		}
	}

	void BondSchedule::populateExDividendDates()
	{
		const size_t expectedSize = accrualStartDates_.size();
		exDividendDates_.reserve(expectedSize);

		for (size_t i = 0; i < expectedSize; ++i)
		{
			LADate exDividendDate = LADate();
			if (!exDividendTenor_.empty())
			{
				exDividendDate = getDateFromTenor(boost::assign::list_of(paymentDates_[i]), exDividendTenor_.c_str(), toString(exDividendBusinessDayAdj_).c_str(), accrualCalendar_, "")[0];
			}

			exDividendDates_.push_back(exDividendDate);
		}

	}


	void BondSchedule::calculateScheduleDates()
	{
		Schedule::calculateScheduleDates();

		const size_t expectedSize = accrualStartDates_.size();

		accrualYearFractions_.clear(); // Do not use the accrualYearFractions calculated from the base class Schedule

		accrualYearFractions_.reserve(expectedSize);
		bondTrueYieldYearFractions_.reserve(expectedSize);

		for (size_t i = 0; i < expectedSize; ++i)
		{
			LADate accrualStart = accrualStartDates_[i];
			LADate accrualEnd = accrualEndDates_[i];

			double bondActualYearFraction = std::numeric_limits<double>::quiet_NaN();

			bool firstCashflow = (i == 0);
			bool lastCashflow = (i == expectedSize - 1);
			LADate paymentDate = paymentDates_[i];
			double tmpYearFraction = calculateBondCashflowYearFraction(firstCashflow, lastCashflow);

			accrualYearFractions_.push_back(tmpYearFraction);

			//Calculate bond actual/adjusted year fraction, with consideration of accrual business adjustment 
			bondActualYearFraction = calculateBondTrueYieldYearFraction(i, accrualStart, accrualEnd, paymentDate);

			bondTrueYieldYearFractions_.push_back(bondActualYearFraction);
		}

		populateExDividendDates();

	}

	double BondSchedule::getFinalCashflowNotionalExchange(const double& notional) const
	{
		double notionaltoUse = getBondNotionalExchangeAtMaturity(bondCalculationType_, getEffectiveDate(), getMaturityDate(), notional, issuePrice_, taxRate_, getPaymentDates());
		return notionaltoUse;
	}

	BondSchedule::BondSchedule(const BondSchedule& rhs) : Schedule(rhs),
		bondCalculationType_(rhs.bondCalculationType_),
		issueDate_(rhs.issueDate_.getCString()),
		issuePrice_(rhs.issuePrice_),
		taxRate_(rhs.taxRate_),
		yieldFrequency_(rhs.yieldFrequency_),
		bondTrueYieldYearFractions_(rhs.bondTrueYieldYearFractions_),
		exDividendTenor_(rhs.exDividendTenor_),
		exDividendBusinessDayAdj_(rhs.exDividendBusinessDayAdj_),
		exDividendDates_(rhs.exDividendDates_)
	{}

	// Return BondActiveCouponDates struct
	// The adjustedFirstActivePaymentDate has business day adjustment if the yield is TRUE yield, otherwise it is the same as firstActivePaymentDate.
	// If it is not the first cashflow, the priorDate is the payment date before the first active coupon (payment) date
	// If it is the first cashflow, 1) when isPriorDtWithFullCouponPeriod is true (used in bond yield calculation), then priorDate is the prior date with full coupon period from the first active payment date. 
	//                              2) when isPriorDtWithFullCouponPeriod is false (used in bondAccruedInterest calculation), then priorDate is first accrualStartDate (effective date).
	BondActiveCouponDates BondSchedule::getBondFirstActiveCouponDates(const LADate& settlementDate, bool isPriorDtWithFullCouponPeriod) const
	{
		// Get the Active Coupon Date
		auto paymentDates = getPaymentDates();
		auto firstActivePaymentDateAndIndex = getActiveCashflowDateAndIndex(settlementDate, paymentDates, paymentDates);
		const LADate firstActivePaymentDate = firstActivePaymentDateAndIndex.first;
		size_t firstActivePaymentIndex = firstActivePaymentDateAndIndex.second;

		bool isFirstCashflow = (firstActivePaymentIndex == 0);

		LADate priorFirstActivePaymentDate = LADate();

		if (isFirstCashflow)
		{
			if (isPriorDtWithFullCouponPeriod)
			{
				//Same as firstPriorVirtualCouponDate_, when there is no stub
				priorFirstActivePaymentDate = getBondPriorVirtualPaymentDate(firstActivePaymentDate);
			}
			else
			{
				//Same as first AccrualStartDate, when there is shortStart/longStart stub in the first cashflow
				priorFirstActivePaymentDate = getAccrualStartDates().front();
			}
		}
		else
		{
			//Same as preultimate payment date, when there is shortEnd/longEnd stub in the last cashflow
			priorFirstActivePaymentDate = paymentDates.at(firstActivePaymentIndex - 1);
		}

		BondActiveCouponDates couponDates;

		couponDates.firstActiveCashflowIndex_ = firstActivePaymentIndex;
		couponDates.firstActiveCouponDate_ = firstActivePaymentDate;
		couponDates.priorFirstActiveCouponDate_ = priorFirstActivePaymentDate;

		couponDates.adjustedFirstActiveCouponDate_ = accrualEndDates_[firstActivePaymentIndex];;

		const LADate firstPriorVirtualPaymentDate = getBondPriorVirtualPaymentDate(firstActivePaymentDate);
		const LADate secondPriorVirtualPaymentDate = getBondPriorVirtualPaymentDate(firstPriorVirtualPaymentDate);

		couponDates.firstPriorVirtualCouponDate_ = firstPriorVirtualPaymentDate;
		couponDates.secondPriorVirtualCouponDate_ = secondPriorVirtualPaymentDate;

		couponDates.firstActiveCashflowExDividendDate_ = exDividendDates_[firstActivePaymentIndex];

		couponDates.isSettleDateExdividend_ = excludeCouponInterest(settlementDate, couponDates.firstActiveCashflowExDividendDate_);

		return couponDates;
	}


	StubTypeEnum BondSchedule::getBondStubTypeEnum(bool isFrontStub) const
	{
		// Front Stub
		if (isFrontStub)
		{
			StubTypeEnum frontStubType = NONE_STUBTYPE;
			if (stubType_ != NONE_STUBTYPE)
			{
				frontStubType = stubType_;
				if (frontStubType == SHORT_END_STUBTYPE || frontStubType == LONG_END_STUBTYPE)
				{
					return NONE_STUBTYPE;
				}
			}
			else
			{
				auto firstPaymentDate = paymentDates_.front();
				LADate firstPriorVirtualPaymentDate = getBondPriorVirtualPaymentDate(firstPaymentDate);
				LADate secondPriorVirtualPaymentDate = getBondPriorVirtualPaymentDate(firstPriorVirtualPaymentDate);

				auto effectiveDate = getEffectiveDate();
				if (effectiveDate > firstPriorVirtualPaymentDate)
				{
					frontStubType = SHORT_START_STUBTYPE;
				}
				else if (effectiveDate < firstPriorVirtualPaymentDate && effectiveDate > secondPriorVirtualPaymentDate)
				{
					frontStubType = LONG_START_STUBTYPE;
				}
				else
				{
					return NONE_STUBTYPE;
				}
			}
			return frontStubType;


		}
		else
		{
			// End Stub
			StubTypeEnum endStubType = NONE_STUBTYPE;
			if (stubType_ != NONE_STUBTYPE)
			{
				endStubType = stubType_;
				if (endStubType == SHORT_START_STUBTYPE || endStubType == LONG_START_STUBTYPE)
				{
					return NONE_STUBTYPE;
				}
			}
			else
			{
				auto paymentDateSize = paymentDates_.size();
				if (paymentDateSize <= 2)
				{
					return NONE_STUBTYPE;
				}

				auto preultimatePaymentDate = paymentDates_[paymentDateSize - 2];
				auto lastPaymentDate = paymentDates_.back();
				LADate priorToLastPaymentDate = getBondPriorVirtualPaymentDate(lastPaymentDate);

				if (preultimatePaymentDate > priorToLastPaymentDate)
				{
					endStubType = SHORT_END_STUBTYPE;
				}
				else if (preultimatePaymentDate < priorToLastPaymentDate)
				{
					endStubType = LONG_END_STUBTYPE;
				}
				else
				{
					return NONE_STUBTYPE;
				}
			}

			return endStubType;
		}

	}

	double BondSchedule::calculateBondAccruedInterest(const LADate& settlementDate, const BondYieldParameters& bondYieldParameters) const
	{

		auto activeCouponDates = getBondFirstActiveCouponDates(settlementDate, false);

		double accruedInterestYearFraction = etrading::calculateBondAccruedInterestYearFraction(settlementDate,
			activeCouponDates,
			bondYieldParameters);


		// Calculate the accrued interest
		const double faceValue = getNotional();
		const double couponRate = getFixedRate();
		const double annualizationFactor = convertBondFrequencyToYearFraction(accrualFrequency_);

		const double annualizedCouponRate = couponRate * annualizationFactor;

		const double accruedInterestFactor = 100.0 * annualizedCouponRate * accruedInterestYearFraction;

		const double roundedAccruedInterestFactor = roundBondAccruedInterestFactor(accruedInterestFactor, bondCalculationType_, settlementDate);

		const double accruedInterest = (faceValue / 100.0) * roundedAccruedInterestFactor;

		return accruedInterest;
	}

	int BondSchedule::calculateBondAccruedInterestDays(const LADate& settlementDate, const BondYieldParameters& bondYieldParameters) const
	{
		auto activeCouponDates = getBondFirstActiveCouponDates(settlementDate, false);

		auto result = etrading::calculateBondAccruedInterestDays(settlementDate, activeCouponDates, bondYieldParameters);
		const int accruedInterestDays = (unsigned int)roundToDecimal(result, 0);

		return accruedInterestDays;
	}

	// Virtual payment date with full coupon period before the given paymentDate
	LADate BondSchedule::getBondPriorVirtualPaymentDate(const LADate& paymentDate) const
	{
		//Get tenor like '-1Y', '-
		auto tenor = getFrequencyTenor(accrualFrequency_);
		auto rollConvention = getRollConvection(rollDayInput_);

		std::stringstream ss;
		ss << "-" << tenor;
		LAString minusTenor = ss.str().c_str();

		LADate priorVirtualCouponDate = getDateFromTenor(boost::assign::list_of(paymentDate),
			minusTenor,
			toString(paymentbusinessDayAdj_).c_str(),
			paymentCalendar_,
			rollConvention)[0];
		return priorVirtualCouponDate;
	}


	double  BondSchedule::calculateBondCashflowYearFraction(bool firstCashflow, bool lastCashflow) const
	{

		auto bondCalculationType = getBondCalculationType();

		double normalYearFraction = convertBondFrequencyToYearFraction(accrualFrequency_);

		double resultYearFraction = normalYearFraction;

		StubTypeEnum frontStubType = getBondStubTypeEnum(true);
		StubTypeEnum endStubType = getBondStubTypeEnum(false);
		auto dayCountEnum = getAccrualDaycount();

		if (firstCashflow && frontStubType != NONE_STUBTYPE)
		{
			// Handle Front Stub
			auto firstPaymentDate = paymentDates_.front();
			auto firstAccrualStartDate = getEffectiveDate();

			switch (frontStubType)
			{
			case SHORT_START_STUBTYPE:
			{
				LADate firstPriorVirtualPaymentDate = getBondPriorVirtualPaymentDate(firstPaymentDate);

				//fullCouponPeriodDays is the full coupon periods between 1stPriorVirtualDt and first payment date
				auto fullCouponPeriodDays = getBondFullCouponPeriodDays(firstPriorVirtualPaymentDate, firstPaymentDate, dayCountEnum, accrualFrequency_);

				if (isJapaneseGovenmentBond(bondCalculationType))
				{
					//For JGP, BOJ convention is used: yearFraction = 1 - days/fullCouponPeriodDays, where days is from 1stPriorVirtualDt to firstAccrualStartDt
					auto days = getBondActualCouponPeriodDays(firstPriorVirtualPaymentDate, firstAccrualStartDate, dayCountEnum);

					//Bonds that are issued prior to 3/1/2001 are given one extra day in the first coupon period, so days outside first coupon period is MINUS 1 day
					if (isBondJGBIssuedPriorToMarketConventionChangeDate(bondCalculationType, issueDate_))
					{
						days--;
					}

					resultYearFraction = 1 - days * (1.0) / fullCouponPeriodDays;
				}
				else
				{
					//shortYearFraction = (firstPaymentDate-firstAccrualStartDt) / fullCouponPeriodDays
					auto days1 = getBondActualCouponPeriodDays(firstAccrualStartDate, firstPaymentDate, dayCountEnum);

					resultYearFraction = days1 * (1.0) / fullCouponPeriodDays;
				}

				//scale by the normalYearfaction, e.g. 0.5, 1 
				resultYearFraction *= normalYearFraction;

				break;
			}
			case LONG_START_STUBTYPE:
			{

				LADate firstPriorVirtualPaymentDate = getBondPriorVirtualPaymentDate(firstPaymentDate);
				LADate secondPriorVirtualPaymentDate = getBondPriorVirtualPaymentDate(firstPriorVirtualPaymentDate);

				//1) shortYearFraction = (1stPriorVirtualDt - firstAccrualStartDt)/fullCouponPeriodDays1, where fullCouponPeriodDays1 is the full coupon period between 2stPriorVirtualDt and 1stPriorVirtualDt         
				auto days1 = getBondActualCouponPeriodDays(firstAccrualStartDate, firstPriorVirtualPaymentDate, dayCountEnum);
				if (isBondJGBIssuedPriorToMarketConventionChangeDate(bondCalculationType, issueDate_))
				{
					days1++;
				}

				double fullCouponPeriodDays1 = getLongStartStubFirstFullCouponDays(firstPaymentDate, firstPriorVirtualPaymentDate, secondPriorVirtualPaymentDate, bondCalculationType, dayCountEnum, accrualFrequency_);

				double shortYearFraction = days1 * (1.0) / fullCouponPeriodDays1; //to double

				// 2) totalYearFraction =  shortYearFraction + normalCouponYearFraction, , where normalCouponYearFraction is the full coupon period between 1stPriorVirtualDt and 1stCouponDt;
				resultYearFraction = shortYearFraction + 1;

				//scale by the normalYearfaction, e.g. 0.5, 1
				resultYearFraction *= normalYearFraction;

				break;
			}
			default:
				throw ETradingException((boost::format("#Error: For Bond StubType value '%i' invalid, must be 'NONE', 'SHORT_START' or 'LONG_START'.") % frontStubType).str());
				break;
			} // end of switch

		}
		else if (lastCashflow && endStubType != NONE_STUBTYPE)
		{
			// Handle End Stub
			auto lastPaymentDate = paymentDates_.back();
			auto paymentDateSize = paymentDates_.size();
			auto preultimatePaymentDate = paymentDates_[paymentDateSize - 2];

			switch (endStubType)
			{
			case SHORT_END_STUBTYPE:
			{
				LADate firstPriorVirtualPaymentDate = getBondPriorVirtualPaymentDate(lastPaymentDate);

				auto days1 = getBondActualCouponPeriodDays(preultimatePaymentDate, lastPaymentDate, dayCountEnum);

				auto fullCouponPeriodDays = getBondFullCouponPeriodDays(firstPriorVirtualPaymentDate, lastPaymentDate, dayCountEnum, accrualFrequency_);

				//shortYearFraction = (LastPaymentDate-preultimatePaymentDate) / lastFullCouponPeriod     
				resultYearFraction = days1 * 1.0 / fullCouponPeriodDays; //to double

				//scale by the normalYearfaction, e.g. 0.5, 1
				resultYearFraction *= normalYearFraction;

				break;
			}
			case LONG_END_STUBTYPE:
			{
				LADate firstPriorVirtualPaymentDate = getBondPriorVirtualPaymentDate(lastPaymentDate);
				LADate secondPriorVirtualPaymentDate = getBondPriorVirtualPaymentDate(firstPriorVirtualPaymentDate);

				//1) shortYearFraction = (1stPriorVirtual-preultimatePaymentDate) / fullCouponPeriodDays1, where fullCouponPeriodDays1 is the full coupon period between 2stPriorVirtualDt and 1stPriorVirtualDt         
				auto days1 = getBondActualCouponPeriodDays(preultimatePaymentDate, firstPriorVirtualPaymentDate, dayCountEnum);
				auto fullCouponPeriodDays1 = getBondFullCouponPeriodDays(secondPriorVirtualPaymentDate, firstPriorVirtualPaymentDate, dayCountEnum, accrualFrequency_);
				double shortYearFraction = days1 * 1.0 / fullCouponPeriodDays1; //to double

				//2) totalYearFraction =  shortYearFraction + normalCouponYearFraction, where fullCouponPeriodDays2 is the full coupon period between 1stPriorVirtual and lastPaymentDt
				resultYearFraction = shortYearFraction + 1;

				//scale by the normalYearfaction, e.g. 0.5, 1
				resultYearFraction *= normalYearFraction;

				break;
			}
			default:
				throw ETradingException((boost::format("#Error: For Bond StubType value '%i' invalid, must be 'NONE', 'SHORT_START' or 'LONG_START'.") % endStubType).str());
				break;
			} // end of switch
		}

		return resultYearFraction;

	}

	//This is used when the yield calculation type is chosen as TRUE_YIELD
	double  BondSchedule::calculateBondTrueYieldYearFraction(size_t cashflowIndex, const LADate& accrualStart, const LADate& accrualEnd, const LADate& paymentDate) const
	{
		double trueYieldYearFraction = 0.0;

		LADate previousPaymentDate;

		if (cashflowIndex == 0)
		{
			previousPaymentDate = (getUpfrontCashflow() != nullptr) ? getUpfrontCashflow()->getPaymentDate() : accrualStart;
		}
		else
		{
			previousPaymentDate = paymentDates_[cashflowIndex - 1];

		}

		auto dayCountEnum = getAccrualDaycount();

		auto adjustedDays = getBondActualCouponPeriodDays(accrualStart, accrualEnd, dayCountEnum);
		auto unAdjustedPaymentDays = getBondFullCouponPeriodDays(previousPaymentDate, paymentDate, dayCountEnum, accrualFrequency_);

		trueYieldYearFraction = adjustedDays * 1.0 / unAdjustedPaymentDays;

		//scale by the normalYearfaction, e.g. 0.5, 1 
		double normalYearFraction = convertBondFrequencyToYearFraction(accrualFrequency_);
		trueYieldYearFraction *= normalYearFraction;

		return trueYieldYearFraction;
	}

	/* @brief Private helper method which stores the input discount factors and float rates in the dataProvider
	*  @param[out] dataProvider				The DataProvider to update
	*  @param[in] calculatedDiscountFactors	The input discount factors to set in the data provider
	*  @param[in] floatRates				The input float rates
	*  @param[in] activeCouponDates			Specifies the coupon dates which bracket the settlement date.	
	*/
	void BondSchedule::setDiscountFactorsAndFloatRatesInDataProvider( DataProvider& dataProvider, const DoubleVector& calculatedDiscountFactors, const std::vector< FloatRateData >& floatRates, const BondActiveCouponDates& activeCouponDates ) const
	{
		// Handle upfront cashflow
		DoubleVector discountFactors = calculatedDiscountFactors;
		if (getUpfrontCashflow() != nullptr)
		{
			// get it from the first element
			auto df = discountFactors.front();

			// Set DF for the upfrontCashflow
			dataProvider.setUpfrontDiscountFactor(df);

			// DFs excluding the first one that belongs to upfrontCashflow
			discountFactors.erase(discountFactors.begin());
		}

		// Set floatRates and DFs for the non-upfront cashflows
		size_t cashflowSize = getCashflowSize();
		if (cashflowSize != discountFactors.size())
		{
			throw LACoreInvalidData("#Error: DiscountFactors and Cashflows should have the same size", __FILE__, __LINE__);
		}

		bool hasFloatRates = (floatRates.size() != 0);

		if (hasFloatRates && cashflowSize != floatRates.size())
		{
			throw LACoreInvalidData("#Error: FloatRates and Cashflows should have the same size", __FILE__, __LINE__);
		}

		dataProvider.setDiscountFactors(discountFactors);
		if (hasFloatRates)
		{
			dataProvider.setFloatRates(floatRates);
		}

		updateIncludeCouponRates(dataProvider, activeCouponDates.firstActiveCashflowIndex_);

	}


	void BondSchedule::initializeDataProviderWithYieldData(DataProvider& dataProvider, const BondYieldParameters & bondYieldParameters, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr, const std::vector< FloatRateData >& floatRates) const
	{

		// These are the first active coupon dates
		const LADate settlementDate = dataProvider.getValuationSettings().getValuationDate();

		//activeCouponDatesPtr is populated when solving yields, as the activeCouponDate won't change
		auto activeCouponDates = (activeCouponDatesPtr != nullptr) ? *(activeCouponDatesPtr.get()) : getBondFirstActiveCouponDates(settlementDate, true);

		// Get DFs from the discount curve
		auto tempPaymentDatesWithfrontNotionalExchange = getPaymentDates();
		auto adjustedYearFractions = bondTrueYieldYearFractions_;

		// Handle upfront cashflow
		if (getUpfrontCashflow() != nullptr)
		{
			//add it to the first element
			tempPaymentDatesWithfrontNotionalExchange.insert(tempPaymentDatesWithfrontNotionalExchange.begin(), getUpfrontCashflow()->getPaymentDate());
			adjustedYearFractions.insert(adjustedYearFractions.begin(), 1.0);
		}

		auto dayCount = getAccrualDaycount();

		DoubleVector discountFactors = calculateDiscountFactorsFromBondYield( bondYieldParameters,
																			  settlementDate,
																			  activeCouponDates,
																			  tempPaymentDatesWithfrontNotionalExchange,
																			  yield,
																			  dayCount,
																			  yieldCalcType,
																			  adjustedYearFractions );

		setDiscountFactorsAndFloatRatesInDataProvider( dataProvider, discountFactors, floatRates, activeCouponDates );

	}

	/* @brief: Initializes the dataProvider using discount factors calculated using yield points obtained from a BondCurve.
	*
	* @param[out]	dataProvider			The dataProvider to initialize
	* @param[in]	bondYieldParameters		Specifies static bond data such as calculationType, coupon payment frequency etc
	* @param[in]	bondCurve				The bond curve
	* @param[in]	activeCouponDatesPtr	Specifies the coupon dates which bracket the settlement date.
	* @param[in]	floatRates				Optional float rates
	*/
	void BondSchedule::initializeDataProviderWithBondCurve( DataProvider& dataProvider, const BondYieldParameters & bondYieldParameters, const BondCurve& bondCurve, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr, const std::vector< FloatRateData >& floatRates ) const
	{
		// These are the first active coupon dates
		const LADate settlementDate = dataProvider.getValuationSettings().getValuationDate();

		//activeCouponDatesPtr is populated when solving yields, as the activeCouponDate won't change
		auto activeCouponDates = (activeCouponDatesPtr != nullptr) ? *(activeCouponDatesPtr.get()) : getBondFirstActiveCouponDates(settlementDate, true);

		// Get DFs from the discount curve
		auto tempPaymentDatesWithfrontNotionalExchange = getPaymentDates();
		auto adjustedYearFractions = bondTrueYieldYearFractions_;

		// Handle upfront cashflow
		if (getUpfrontCashflow() != nullptr)
		{
			//add it to the first element
			tempPaymentDatesWithfrontNotionalExchange.insert(tempPaymentDatesWithfrontNotionalExchange.begin(), getUpfrontCashflow()->getPaymentDate());
			adjustedYearFractions.insert(adjustedYearFractions.begin(), 1.0);
		}

		auto dayCount = getAccrualDaycount();

		DoubleVector discountFactors = calculateDiscountFactorsFromBondCurve( bondYieldParameters,
																			  settlementDate,
																			  activeCouponDates,
																			  tempPaymentDatesWithfrontNotionalExchange,
																			  bondCurve,
																			  dayCount,
																			  adjustedYearFractions );

		setDiscountFactorsAndFloatRatesInDataProvider( dataProvider, discountFactors, floatRates, activeCouponDates );
	}


	LADate BondSchedule::getMaturityDate() const
	{
		return getUnadjustedMaturityDate();
	}

	//Helper function to initialize the member variables
	void BondSchedule::initialize(const LabelValueBlock& scheduleLVB)
	{
		const std::string inputLVB = "scheduleLVB";
		inputParameters_ = LabelValueBlock( scheduleLVB, IRS_KEY::SCHEDULE_TYPE, toString(scheduleType_) );

		payerReceiver_ = toPayReceiveEnum(scheduleLVB.getCompulsoryValueAsLAStringFromKeys(IRS_KEY::PAY_RECEIVE, IRS_KEY::PAYER_RECEIVER, inputLVB).getCString());

		amortization_ = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::AMORTIZATION);
		amortFrequency_ = scheduleLVB.getOptionalValueAsUnsignedInt(IRS_KEY::AMORT_FREQUENCY);

		notionalExchangeEnum_ = toNotionalExchangeEnum(scheduleLVB.getOptionalValueAsLAString(IRS_KEY::NOTIONAL_EXCHANGE, "NONE").getCString());

		leverage_ = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::LEVERAGE, 1.);

		accrualEndDateOrTenor_ = scheduleLVB.getCompulsoryValueAsLAString(IRS_KEY::MATURITY_DATE, inputLVB);

		LAString fixedBusinessDayAdjustment = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FIXED_BUSINESSDAYADJUSTMENT)(IRS_KEY::BUSINESSDAYADJUSTMENT));
		LAString fixedCalendar = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FIXED_CALENDAR)(IRS_KEY::CALENDAR));
		LAString fixedLegFreq = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FIXED_FREQUENCY)(IRS_KEY::FREQUENCY));
		LAString fixedDayCount = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FIXED_DAYCOUNT)(IRS_KEY::DAYCOUNT));

		accrualbusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALBUSINESSDAYADJUSTMENT, IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT, fixedBusinessDayAdjustment).getCString());
		accrualCalendar_ = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALCALENDAR, IRS_KEY::ACCRUALCALENDAR, fixedCalendar);
		accrualFrequency_ = toFrequencyEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALFREQUENCY, IRS_KEY::ACCRUALFREQUENCY, fixedLegFreq).getCString());
		accrualDaycount_ = toDayCountEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALDAYCOUNT, IRS_KEY::ACCRUALDAYCOUNT, fixedDayCount).getCString());

		auto defaultBusinessDayAdj = fixedBusinessDayAdjustment.size() != 0 ? fixedBusinessDayAdjustment : toString(accrualbusinessDayAdj_).c_str();
		auto defaultCalendar = fixedCalendar.size() != 0 ? fixedCalendar : accrualCalendar_;
		auto defaultFrequency = fixedLegFreq.size() != 0 ? fixedLegFreq : toString(accrualFrequency_).c_str();

		paymentbusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTBUSINESSDAYADJUSTMENT, IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj).getCString());
		paymentCalendar_ = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTCALENDAR, IRS_KEY::PAYMENTCALENDAR, defaultCalendar);
		paymentFrequency_ = toFrequencyEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTFREQUENCY, IRS_KEY::PAYMENTFREQUENCY, defaultFrequency).getCString());

		populateNotionalAndPaymentFreqEnum(scheduleLVB);

		// Allowable Stub LVB Keys
		std::vector<std::string> firstStubKeys;
		firstStubKeys.push_back(IRS_KEY::FIXED_FIRSTSTUBDATE);
		firstStubKeys.push_back(IRS_KEY::FIRSTSTUBDATE);
		firstStubKeys.push_back(BOND_KEY::FIRST_COUPON_DATE);

		std::vector<std::string> lastStubKeys;
		lastStubKeys.push_back(IRS_KEY::FIXED_LASTSTUBDATE);
		lastStubKeys.push_back(IRS_KEY::LASTSTUBDATE);
		lastStubKeys.push_back(BOND_KEY::LAST_COUPON_DATE);

		firstStub_ = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(firstStubKeys);
		lastStub_ = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(lastStubKeys);

		rollDayInput_ = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ROLLDAY, IRS_KEY::ROLLDAY);
		payLag_ = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTLAG, IRS_KEY::PAYMENTLAG, "0D");
		stubType_ = toStubTypeEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_STUBTYPE, IRS_KEY::STUBTYPE).getCString());

		//fxFixing
		fxFixingLag_ = scheduleLVB.getOptionalValueAsLAString(IRS_KEY::FXFIXINGLAG, "0D");
		fxFixingbusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAString(IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj).getCString());
		fxFixingCalendar_ = scheduleLVB.getOptionalValueAsLAString(IRS_KEY::FXFIXINGCALENDAR, defaultCalendar);

		// Bond Calculation Type
		bondCalculationType_ = toBondCalculationTypeEnum(scheduleLVB.getCompulsoryValueAsLAString(BOND_KEY::CALCULATION_TYPE, inputLVB).getCString());

		issuePrice_ = scheduleLVB.getOptionalValueAsDouble(BOND_KEY::ISSUE_PRICE, std::numeric_limits<double>::quiet_NaN());
		taxRate_ = scheduleLVB.getOptionalValueAsDouble(BOND_KEY::TAX_RATE, std::numeric_limits<double>::quiet_NaN());
		yieldFrequency_ = toFrequencyEnum(scheduleLVB.getOptionalValueAsLAString(BOND_KEY::YIELD_FREQUENCY, toString(accrualFrequency_).c_str()).getCString());

		// Populate exDividend tenor and busAdj:
		exDividendTenor_ = scheduleLVB.getOptionalValueAsString(BOND_KEY::EX_DIVIDEND_TENOR, "");
		exDividendBusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsString(BOND_KEY::EX_DIVIDEND_BUSINESSDAYADJUSTMENT, ""));

		if (!exDividendTenor_.empty())
		{
			//If the tenor is input by the user, we need to add negative sign so that exDividend is before couponDate
			std::stringstream mySS;
			mySS << "-" << exDividendTenor_;
			exDividendTenor_ = mySS.str();
		}

		switch (bondCalculationType_)
		{
		case TYPE23_AUSTRALIAN_GOVERNMENT_BONDS:

			//Australia Gov Bonds: Begin to trade ex-dividend 7 CALENDAR days prior to the coupon date.
			//Which means exDividendDate = couponDate - 7CalendarDays.  
			if (exDividendTenor_.empty())
			{
				exDividendTenor_ = "-7D";
			}
			if (exDividendBusinessDayAdj_ == NONE_BUSINESS_DAY_ADJ)
			{
				exDividendBusinessDayAdj_ = NO_CHANGE;
			}
			break;
		case TYPE26_UK_GILT:

			//UK Gilt: Begin to trade ex-dividend "at the close of" 7 BUSINESS days prior to the coupon date. 
			//Which means exDividendDate = couponDate - 7BusinessDays + 1. 
			if (exDividendTenor_.empty())
			{
				exDividendTenor_ = "-6D";
			}
			if (exDividendBusinessDayAdj_ == NONE_BUSINESS_DAY_ADJ)
			{
				exDividendBusinessDayAdj_ = MOD_FOLLOWING;
			}
			break;
		default:
			// Do nothing
			break;
		}

	}


	//Override
	void BondSchedule::initializeDataProviderWithCurveData(DataProvider& dataProvider, const LAString& discountCurve, const std::vector< FloatRateData >& floatRates) const
	{
		Schedule::initializeDataProviderWithCurveData(dataProvider, discountCurve, floatRates);

		// No that we cannot use the swap's getFirstNonpastDateIndex(), because:
		// When settleDate equals paymentDate, for swap this cashflow is the first active cashflow; but for bond this cashflow is in the past. 
		auto firstActiveCashflowInfo = getActiveCashflowDateAndIndex(dataProvider.getValuationSettings().getValuationDate(), getPaymentDates(), getPaymentDates());
		const size_t firstActiveCashflowIndex = firstActiveCashflowInfo.second;

		updateIncludeCouponRates(dataProvider, firstActiveCashflowIndex);
	}

	//When settlementDate >= exDividendDate, this method will set the firstActiveCashflow's includeCouponRate to false; otherwise it will be true
	void BondSchedule::updateIncludeCouponRates(DataProvider& dataProvider, const size_t& firstActiveCashflowIndex) const
	{

		const size_t cashflowSize = cashflows_.size();

		const LADate settleDate = dataProvider.getValuationSettings().getValuationDate();

		BoolVector includeCouponRates(cashflowSize, true);

		auto firstActiveCF = getCashflow(firstActiveCashflowIndex);
		bool includeCouponRate = getFirstActiveCashflowIncludeCouponRate(firstActiveCF, settleDate);
		includeCouponRates[firstActiveCashflowIndex] = includeCouponRate;

		dataProvider.setIncludeCouponRates(includeCouponRates);

	}

	bool BondSchedule::getFirstActiveCashflowIncludeCouponRate(const CashflowPtr& firstActiveCashflow, const LADate& settleDate) const
	{
		if (exDividendTenor_.empty())
		{
			return true;
		}

		// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
		const std::shared_ptr<FixedBondCashflow>& firstActiveCF = std::static_pointer_cast<FixedBondCashflow>(firstActiveCashflow);


		const bool includeCouponRate = !excludeCouponInterest(settleDate, firstActiveCF->getBondExDividendDate());

		return includeCouponRate;
	}

	std::unordered_set<CashflowHeaderEnum, EnumClassHash> BondSchedule::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum, EnumClassHash> expectedList
		{
			ACCRUAL_START_HEADER
			,ACCRUAL_END_HEADER
			,ACCRUAL_YEAR_FRACTIONS_HEADER
			,EX_DIVIDEND_DATE_HEADER
			,PAYMENT_DATE_HEADER

			,NOTIONAL_HEADER
			,NOTIONAL_EXCHANGE_HEADER
			,LEVERAGE_HEADER
		};

		return expectedList;
	}

	void BondSchedule::populateHeaderAndBody(AnyTypeVector& headers, std::vector<AnyTypeVector>& bodyBlock, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList) const
	{

		auto allowColumns = allowedColumns();

		bool includeAccrualStart = includeCashflowColumn(ACCRUAL_START_HEADER, columnList, allowColumns);
		bool includeAccrualEnd = includeCashflowColumn(ACCRUAL_END_HEADER, columnList, allowColumns);
		bool includeAccrualYearFraction = includeCashflowColumn(ACCRUAL_YEAR_FRACTIONS_HEADER, columnList, allowColumns);
		bool includeTrueYieldAccrualYearFraction = includeCashflowColumn(TRUE_YIELD_YEAR_FRACTIONS_HEADER, columnList, allowColumns);
		bool includeExDividendDate = includeCashflowColumn(EX_DIVIDEND_DATE_HEADER, columnList, allowColumns);
		bool includePaymentDate = includeCashflowColumn(PAYMENT_DATE_HEADER, columnList, allowColumns);
		bool includeNotional = includeCashflowColumn(NOTIONAL_HEADER, columnList, allowColumns);
		bool includeNotionalExchange = includeCashflowColumn(NOTIONAL_EXCHANGE_HEADER, columnList, allowColumns);
		bool includeLeverage = includeCashflowColumn(LEVERAGE_HEADER, columnList, allowColumns);

		auto allCashflows = getAllCashflows();
		size_t cashflowSize = allCashflows.size();

		MLIB_REQUIRE(cashflowSize != 0, "Cashflows cannot be empty.");

		bool hasBondExDividendDate = (!exDividendTenor_.empty());

		//1) Headers
		if (showColumnHeaders)
		{
			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_START_HEADER), includeAccrualStart);
			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_END_HEADER), includeAccrualEnd);
			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_YEAR_FRACTIONS_HEADER), includeAccrualYearFraction);
			MLIB_PUSH_BACK_IF(headers, toString(TRUE_YIELD_YEAR_FRACTIONS_HEADER), includeTrueYieldAccrualYearFraction);
			MLIB_PUSH_BACK_IF(headers, toString(EX_DIVIDEND_DATE_HEADER), hasBondExDividendDate && includeExDividendDate);
			MLIB_PUSH_BACK_IF(headers, toString(PAYMENT_DATE_HEADER), includePaymentDate);
			MLIB_PUSH_BACK_IF(headers, toString(NOTIONAL_HEADER), includeNotional);
			MLIB_PUSH_BACK_IF(headers, toString(NOTIONAL_EXCHANGE_HEADER), includeNotionalExchange);
			MLIB_PUSH_BACK_IF(headers, toString(LEVERAGE_HEADER), includeLeverage);

		}

		const size_t headerColumnSize = headers.size();

		//2) Body
		for (size_t i = 0; i < cashflowSize; ++i)
		{
			AnyTypeVector body;
			if (headerColumnSize > 0)
			{
				body.reserve(headerColumnSize);
			}

			auto cashflow = allCashflows[i];

			// bool isUpfrontCf = cashflow->isUpfrontCashflow(); <--- Unused Variable

			MLIB_PUSH_BACK_IF(body, fromLADateToDouble(cashflow->getAccrualStartDate()), includeAccrualStart);
			MLIB_PUSH_BACK_IF(body, fromLADateToDouble(cashflow->getAccrualEndDate()), includeAccrualEnd);
			MLIB_PUSH_BACK_IF(body, cashflow->getAccrualYearFraction(), includeAccrualYearFraction);

			MLIB_PUSH_BACK_IF(body, fromLADateToDouble(cashflow->getPaymentDate()), includePaymentDate);

			MLIB_PUSH_BACK_IF(body, cashflow->getBondTrueYieldYearFraction(), includeTrueYieldAccrualYearFraction);
			MLIB_PUSH_BACK_IF(body, fromLADateToDouble(cashflow->getBondExDividendDate()), hasBondExDividendDate && includeExDividendDate);

			MLIB_PUSH_BACK_IF(body, cashflow->getNotional(), includeNotional);

			MLIB_PUSH_BACK_IF(body, cashflow->getNotionalExchange(), includeNotionalExchange);
			MLIB_PUSH_BACK_IF(body, cashflow->getLeverage(), includeLeverage);

			bodyBlock.push_back(body);
		}

	}


}
