#include "CapFloorPricer.h"
#include "CapFloorCashflow.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "ExceptionMacros.h"

namespace etrading
{

    CapFloorPricer::CapFloorPricer(const std::shared_ptr<Option>& option, const std::shared_ptr<Volatility>& volProvider, const std::shared_ptr<RateProvider>& rateProvider) 
		: option_(option), volProvider_(volProvider), rateProvider_(rateProvider)
	
	{
		MLIB_REQUIRE(volProvider->asOfDate() == rateProvider->asOfDate(), "The asOfDate of Volatility and Curve must be the same");
		MLIB_REQUIRE(option->scheduleParameters()->currency() == volProvider->currency(), "The Currency of Volatility and Option must be the same");

		createCashflows();
	}

    CapFloorPricer::CapFloorPricer( const CapFloorPricer& rhs) : option_(rhs.option_),  volProvider_(rhs.volProvider_), rateProvider_(rhs.rateProvider_)
	{}

	void CapFloorPricer::createCashflows()
	{
		const auto schParams = option_->scheduleParameters();
		const auto schOutput = option_->enrichedSchedule();

		MLIB_REQUIRE(!schOutput->fixingDates().empty(), "Cap/Floor Option has no expiryDates(fixingDates)");

		const size_t cashflowSize = schOutput->accrualStartDates().size();
		const CallOrPutEnum callPut = option_->callPut();
		const auto volType = volProvider_->volType();
		const double vol = volProvider_->vol();

		const double notional = schParams->notional();
		const double leverage = schParams->leverage();
		const double strikeRate = schParams->fixedRateOrStrike();
		const double spread = schParams->spread();

		const CCY ccy = schParams->currency();
		const CCY valuationCcy = schParams->valuationCurrency();

		const DoubleVector discountFactors = rateProvider_->discountFactors(schParams, schOutput->paymentDates());

		// DFs is from expiryDt to paymentDate
		const DoubleVector floatRates = rateProvider_->liborRates(schParams, schOutput);

		MLIB_REQUIRE(cashflowSize == discountFactors.size() && cashflowSize == floatRates.size(), "Number of of accrualStartDates, discountFactors, FloatRates not matched");

		//Is it right to use DF's dayCount?
		const auto expiryYearFractionDayCount = ACT_365_DAYCOUNT; // toDayCountEnum(getDiscountFactorDayCount().getCString());

		const LADate valuationDate = getDateFromTenor(boost::assign::list_of(volProvider_->asOfDate()), schParams->spotLag().c_str(),
													toString(schParams->spotBusinessDayAdj()).c_str(), schParams->spotCalendar().c_str(), "")[0];
		for (size_t i = 0; i < cashflowSize; ++i)
		{
			const LADate fixingDate = schOutput->fixingDates()[i];
			const LADate accrualStartDate = schOutput->accrualStartDates()[i];
			const LADate accrualEndDate = schOutput->accrualEndDates()[i];
			const LADate paymentDate = schOutput->paymentDates()[i];
			const double yearFraction = schOutput->accrualYearFractions()[i];

			const CoreCashflow coreCf(fixingDate, accrualStartDate, accrualEndDate, paymentDate, yearFraction, notional, leverage, strikeRate, spread);
			
			const double expiryYearFraction = getYearFraction(valuationDate, fixingDate, expiryYearFractionDayCount, false);

			//From valuationDate to paymentDt 
			const double paymentYearFraction = getYearFraction(valuationDate, paymentDate, expiryYearFractionDayCount, false);

			const CapFloorCashflow capFloorCf(std::make_shared<CoreCashflow>(coreCf), callPut, volType, vol, expiryYearFraction, paymentYearFraction, floatRates[i], discountFactors[i], ccy, valuationCcy);

			cashflows_.emplace_back(std::make_shared<CapFloorCashflow>(capFloorCf));
		}
	}

	const double CapFloorPricer::pv() const
	{
		double optionPV = 0.;

		for (size_t i = 0; i < cashflows_.size(); ++i)
		{
			auto& cf = cashflows_[i];
			optionPV += cf->couponPV();
		}

		return optionPV;
	}

	const BlackScholesGreeks CapFloorPricer::calculateGreeks(const GreekTypeEnum& greekType, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump) const
	{

		double deltaSpot = 0.0;
		double gamma = 0.0;
		double vega = 0.0;
		double theta = 0.0;

		const auto expectedSize = cashflows_.size();

		for (size_t i = 0; i < expectedSize; ++i)
		{
			auto& cf = cashflows_[i];

			BlackScholesGreeks subGreeks = cf->greeks(greekType, deltaBump, gammaBump, vegaBump, thetaBump);

			deltaSpot += subGreeks.deltaSpot;
			gamma += subGreeks.gamma;
			vega += subGreeks.vega;
			theta += subGreeks.theta;
		}

		BlackScholesGreeks greeks;

		greeks.deltaSpot = deltaSpot;
		greeks.gamma = gamma;
		greeks.vega = vega;
		greeks.theta = theta;

		return greeks;
	}

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> CapFloorPricer::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			EXPIRY_DATE_HEADER
			, ACCRUAL_START_HEADER
			, ACCRUAL_END_HEADER
			, ACCRUAL_DAYS_HEADER
			, ACCRUAL_YEAR_FRACTIONS_HEADER
			, PAYMENT_DATE_HEADER
			, NOTIONAL_HEADER
			, LEVERAGE_HEADER
			, STRIKE_HEADER
			, VOL_HEADER
			, FLOAT_SPREAD_HEADER
			, FLOAT_RATE_HEADER
			, COUPON_HEADER
			, DISCOUNT_FACTOR_HEADER
			, COUPON_PV_HEADER

		};

		return expectedList;

	}

	const AnyTypeMatrix CapFloorPricer::view(const bool& showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList) const
	{

		MLIB_REQUIRE(!cashflows_.empty(), "Option has no cashflow to display");

		const auto nanValue = std::numeric_limits<double>::quiet_NaN();

		auto allowColumns = allowedColumns();

		const bool includeFixingDate = includeCashflowColumn(EXPIRY_DATE_HEADER, columnList, allowColumns);
		const bool includeAccrualStart = includeCashflowColumn(ACCRUAL_START_HEADER, columnList, allowColumns);
		const bool includeAccrualEnd = includeCashflowColumn(ACCRUAL_END_HEADER, columnList, allowColumns);
		const bool includeAccrualDays = includeCashflowColumn(ACCRUAL_DAYS_HEADER, columnList, allowColumns);
		const bool includeAccrualYearFraction = includeCashflowColumn(ACCRUAL_YEAR_FRACTIONS_HEADER, columnList, allowColumns);
		const bool includePaymentDate = includeCashflowColumn(PAYMENT_DATE_HEADER, columnList, allowColumns);

		const bool includeNotional = includeCashflowColumn(NOTIONAL_HEADER, columnList, allowColumns);
		const bool includeLeverage = includeCashflowColumn(LEVERAGE_HEADER, columnList, allowColumns);

		const bool includeStrike = includeCashflowColumn(STRIKE_HEADER, columnList, allowColumns);
		const bool includeVol = includeCashflowColumn(VOL_HEADER, columnList, allowColumns);

		const bool includeSpread = includeCashflowColumn(FLOAT_SPREAD_HEADER, columnList, allowColumns);
		const bool includeFloatate = includeCashflowColumn(FLOAT_RATE_HEADER, columnList, allowColumns);
		const bool includeCoupon = includeCashflowColumn(COUPON_HEADER, columnList, allowColumns);
		const bool includeDF = includeCashflowColumn(DISCOUNT_FACTOR_HEADER, columnList, allowColumns);
		const bool includeCouponPV = includeCashflowColumn(COUPON_PV_HEADER, columnList, allowColumns);

		AnyTypeMatrix cashflowDisplayBlock;

		//-----Header-----
		if (showColumnHeaders)
		{
			AnyTypeVector headers;

			MLIB_PUSH_BACK_IF(headers, toString(EXPIRY_DATE_HEADER), includeFixingDate);

			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_START_HEADER), includeAccrualStart);
			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_END_HEADER), includeAccrualEnd);
			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_DAYS_HEADER), includeAccrualDays);
			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_YEAR_FRACTIONS_HEADER), includeAccrualYearFraction);
			MLIB_PUSH_BACK_IF(headers, toString(PAYMENT_DATE_HEADER), includePaymentDate);

			MLIB_PUSH_BACK_IF(headers, toString(NOTIONAL_HEADER), includeNotional);
			MLIB_PUSH_BACK_IF(headers, toString(LEVERAGE_HEADER),  includeLeverage);

			MLIB_PUSH_BACK_IF(headers, toString(STRIKE_HEADER), includeStrike);
			MLIB_PUSH_BACK_IF(headers, toString(VOL_HEADER), includeVol);

			MLIB_PUSH_BACK_IF(headers, toString(FLOAT_SPREAD_HEADER), includeSpread);
			MLIB_PUSH_BACK_IF(headers, toString(FLOAT_RATE_HEADER), includeFloatate);

			MLIB_PUSH_BACK_IF(headers, toString(COUPON_HEADER), includeCoupon);
			MLIB_PUSH_BACK_IF(headers, toString(DISCOUNT_FACTOR_HEADER), includeDF);
			MLIB_PUSH_BACK_IF(headers, toString(COUPON_PV_HEADER), includeCouponPV);

			cashflowDisplayBlock.push_back(headers);
		}

		//-----Body-----

		for (size_t i = 0; i < cashflows_.size(); ++i)
		{
			const auto& cashflow = cashflows_[i];

			AnyTypeVector body;

			MLIB_PUSH_BACK_IF(body, fromLADateToDouble(cashflow->coreCashflow()->fixingDate()), includeFixingDate);


			const auto& coreCashflow = cashflow->coreCashflow();
			const auto accrualStartDt = coreCashflow->accrualStartDate();
			const auto accrualEndDt = coreCashflow->accrualEndDate();
			const int accrualDays = getDays(accrualStartDt, accrualEndDt);

			MLIB_PUSH_BACK_IF(body, fromLADateToDouble(accrualStartDt), includeAccrualStart);
			MLIB_PUSH_BACK_IF(body, fromLADateToDouble(accrualEndDt), includeAccrualEnd);
			MLIB_PUSH_BACK_IF(body, accrualDays, includeAccrualDays);
			MLIB_PUSH_BACK_IF(body, coreCashflow->accrualYearFraction(), includeAccrualYearFraction);
			MLIB_PUSH_BACK_IF(body, fromLADateToDouble(coreCashflow->paymentDate()), includePaymentDate);

			MLIB_PUSH_BACK_IF(body, coreCashflow->notional(), includeNotional);
			MLIB_PUSH_BACK_IF(body, coreCashflow->leverage(), includeLeverage);

			MLIB_PUSH_BACK_IF(body, coreCashflow->strikeRate(), includeStrike);
			MLIB_PUSH_BACK_IF(body, cashflow->vol(), includeVol);

			MLIB_PUSH_BACK_IF(body, coreCashflow->spread(), includeSpread);
			MLIB_PUSH_BACK_IF(body, cashflow->floatOrSwapRate(), includeFloatate);

			const auto coupon = roundToNearest(cashflow->coupon(), cashflow->currency());
			MLIB_PUSH_BACK_IF(body, coupon, includeCoupon);

			MLIB_PUSH_BACK_IF(body, cashflow->discountFactor(), includeDF);

			const auto couponPV = roundToNearest(cashflow->couponPV(), cashflow->valuationCurrency());
			MLIB_PUSH_BACK_IF(body, couponPV, includeCouponPV);

			cashflowDisplayBlock.push_back(body);
		}

		MLIB_REQUIRE(!cashflowDisplayBlock.empty(), "Unable to display the cashflows. The resulted cashflow is empty");
		MLIB_REQUIRE(!cashflowDisplayBlock[0].empty(), "Unable to display the cashflows. The resulted cashflow has no columns");

		return cashflowDisplayBlock;
	}

	const AnyTypeMatrix CapFloorPricer::viewGreeks(const GreekTypeEnum& greekType, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump, const bool& showColumnHeaders) const
	{
		BlackScholesGreeks greeks = calculateGreeks(greekType, deltaBump, gammaBump, vegaBump, thetaBump);

		AnyTypeMatrix result;

		if (showColumnHeaders)
		{
			AnyTypeVector headers;
			headers.push_back((std::string)"Delta");
			headers.push_back((std::string)"Gamma");
			headers.push_back((std::string)"Vega");
			headers.push_back((std::string)"Theta");

			result.push_back(headers);
		}

		AnyTypeVector body;
		body.push_back(greeks.deltaSpot);
		body.push_back(greeks.gamma);
		body.push_back(greeks.vega);
		body.push_back(greeks.theta);

		result.push_back(body);

		return result;
	}





}


