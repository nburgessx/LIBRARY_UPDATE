#include "CapFloorCashflow.h"
#include "CapletFloorlet.h"

namespace etrading
{
	CapFloorCashflow::CapFloorCashflow(const std::shared_ptr<CoreCashflow>& coreCashflow,
									const CallOrPutEnum & callPut,
									const VolatilityTypeEnum& volType,
									const double& vol,
									const double& expiryYearFraction,
									const double& paymentYearFraction,
									const double& floatRate,
									const double& discountFactor, // DF from asOfDate to paymentDate
									const CCY& currency,
									const CCY& valuationCurrency)
					: OptionCashflow(coreCashflow, callPut, volType, vol, expiryYearFraction, paymentYearFraction, floatRate, discountFactor, currency, valuationCurrency)
    {}

	// Copy Constructor
	CapFloorCashflow::CapFloorCashflow(const CapFloorCashflow & rhs) : OptionCashflow(rhs)
	{}


	std::shared_ptr<OptionCashflow> CapFloorCashflow::clone()
    {
		std::shared_ptr<OptionCashflow> cf = std::shared_ptr<OptionCashflow>(new CapFloorCashflow(*this));
        return cf;
    }

	const double CapFloorCashflow::annuityFactor() const
	{
		// Formula: annuityFactor = tao * Notional  * discountFactor, where discountFactor is from asOfDate to paymentDate
		const double factor = accrualAmount() * discountFactor_;
		
		return factor;
	}

	const double CapFloorCashflow::accrualAmount() const
	{
		//tao * Notional 
		const double accrualAmount = coreCashflow_->notional() * coreCashflow_->leverage() * coreCashflow_->accrualYearFraction();

		return accrualAmount;
	}

	const double CapFloorCashflow::coupon() const
	{

		const auto cfEnum = (callPut_ == CALL_OPTION) ? CAPLET_OPTION : FLOORLET_OPTION;

		//Note, we do not multiple the discount factor here, as it's coupon not coupon pv
		const auto factor = accrualAmount();

		double coupon = CapletFloorlet::price(cfEnum, factor, floatOrSwapRate_, coreCashflow_->strikeRate(), vol_, expiryYearFraction_, 0.0, volType_); // shift = 0.0;

		return coupon;
	}

	const BlackScholesGreeks CapFloorCashflow::greeks(const GreekTypeEnum& greekType, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump) const
	{
		const auto cfEnum = (callPut_ == CALL_OPTION) ? CAPLET_OPTION : FLOORLET_OPTION;

		AnnuityTerm annuityTerm;

		annuityTerm.timeToPayment_ = paymentYearFraction_;
		annuityTerm.discountTerm_ = discountFactor_;
		annuityTerm.accrualTerm_ = accrualAmount();
		annuityTerm.annuityFactor_ = AnnuityTerm::annuityFactor(annuityTerm.accrualTerm_, discountFactor_);

		BlackScholesGreeks greek;

		switch (greekType)
		{
			case ANALYTICAL_GREEK_TYPE:
				greek = CapletFloorlet::greeksAnalytical(cfEnum, floatOrSwapRate_, coreCashflow_->strikeRate(), vol_, expiryYearFraction_, 0.0, annuityTerm, volType_); // shift = 0.0
				break;
			case NUMERICAL_GREEK_TYPE:
				greek = CapletFloorlet::greeksNumerical(cfEnum, floatOrSwapRate_, coreCashflow_->strikeRate(), vol_, expiryYearFraction_, 0.0, annuityTerm, volType_,
														deltaBump, gammaBump, vegaBump, thetaBump); // shift = 0.0
				break;
			default:
				throw LACoreInvalidData("#Error: Only Analytical or Numerical Greek is supported", __FILE__, __LINE__);
				break;
		}


		return greek;
	}


}

