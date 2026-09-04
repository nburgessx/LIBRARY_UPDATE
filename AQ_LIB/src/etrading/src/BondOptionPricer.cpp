#include "BondOptionPricer.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "AQOUtilities.h"
#include "SettingsValidation.h"

namespace etrading
{

    BondOptionPricer::BondOptionPricer(const std::shared_ptr<BondOption>& bondOption, const LabelValueBlock& valuationSettingsLVB, const double& discountRate, const DayCountEnum& discountDayCount) : bondOption_(bondOption), discountDayCount_(discountDayCount)
	{
		const std::string volObjectName = getVolatilityModelFromValuationSettings(valuationSettingsLVB).getCString();
		volProvider_ = getVolatility(volObjectName); // LWO Utility method to get the volatility object pointer from the LWO Cache

		AQ_REQUIRE(bondOption_ != nullptr, "Invalid Bond Optione trade.");
		AQ_REQUIRE(bondOption_->getCurrency() == volProvider_->currency(), "The Currency of Volatility and Option must be the same" );

		AQ_REQUIRE(volProvider_ != nullptr, "Invalid Vol Model.");
		AQ_REQUIRE(volProvider_->volType() == LOGNORMAL_VOLATILITY , "Bond Option/Bond Future Option - only Lognormal Volatility is supported.");

		asOfDate_ = volProvider_->asOfDate();
		AQ_REQUIRE(asOfDate_ != AQLDate(), "Invalid AsOfDate.");

		// Default to Settlement Date
		valuationDate_ = valuationSettingsLVB.getCompulsoryValueAsDate(VALUATION_SETTING_KEYS::VALUATION_DATE, "ValuationSettings");

		bondSettlementDate_ = valuationSettingsLVB.getOptionalValueAsDate("BondSettlementDate", valuationDate_);

		pricingModel_ = toPricingModelEnum(valuationSettingsLVB.getCompulsoryValueAsString(VALUATION_SETTING_KEYS::PRICING_MODEL, "ValuationSettings"));

		AQ_REQUIRE(pricingModel_ == BLACK_MODEL, "Bond Option/Bond Future Option - only Black Model is supported.");

		// T in Option formula, is from asOfDate to ExpiryDate, using option dayCount
		timeToExpiry_ = getYearFraction(asOfDate_, bondOption_->getExpiryDate(), bondOption_->getDayCount());

		// TimeToDelivery in Discount Factor is from valuationDate to DeliveryDate, using discountingDayCount
		// The dayCount used in discounting can be different from the option dayCount.
		timeInDiscounting_ = getYearFraction(valuationDate_, bondOption_->getUnderlyingDeliveryDate(), discountDayCount);
	}

	// Copy Constructor
    BondOptionPricer::BondOptionPricer( const BondOptionPricer& rhs) : bondOption_( rhs.bondOption_), volProvider_( rhs.volProvider_ ), asOfDate_(rhs.asOfDate_), bondSettlementDate_(rhs.bondSettlementDate_), valuationDate_(rhs.valuationDate_), discountRate_(rhs.discountRate_), discountDayCount_(rhs.discountDayCount_), timeToExpiry_(rhs.timeToExpiry_), timeInDiscounting_(rhs.timeInDiscounting_), repoRateTime_(rhs.repoRateTime_)
	{}

	//Option's discount factor from valuationDate to deliveryDate
	double BondOptionPricer::getDiscountFactor() const
	{
		// discount factor of the option: P(0,T)
		const double df = exp(-1.0 * discountRate_ * timeInDiscounting_);

		return df;

	}

	/* @brief	Calculates the Option PV, based on bond forward price
	*           Uses the parameters supplied during object construction.
	*/
	double BondOptionPricer::forwardOptionPrice(const double& bondFwdPrice) const
	{

		// For now we assume shift is zero
		const double shift = 0.0;

		double strikePrice = bondOption_->getStrike();

		double pv = BlackModelUtils::blackPrice(volProvider_->volType(), bondOption_->getCallPut(), bondFwdPrice, strikePrice, volProvider_->vol(), timeToExpiry_, shift, getDiscountFactor());
		
        // Default and assume a Long option position unless a Short position explicitly specified. This is consistent with market practice
        const int longShortIndicator = bondOption_->getLongShort() == SHORT_POSITION ? -1 : 1;

		//scale
        pv *= bondOption_->getNotional() * bondOption_->getLeverage() * longShortIndicator;

		return pv;
	}

	/* @brief	Calculates the Bond Option PV, based on bond spot price, repo rate, and repoDayCount. Assuming that spot bond clean price is following Black dynamtics
	*           Uses the parameters supplied during object construction.
	*/
	double BondOptionPricer::spotOptionPrice(const double& bondPrice, const double& repoRate, const DayCountEnum& repoDayCount) const
	{

		auto underlyingBond = bondOption_->getUnderlying();

		AQ_REQUIRE(underlyingBond != nullptr, "The Underlying must be a Bond object, i.e. BondName cannot be empty.");

		const double bondFwdPrice = underlyingBond->forwardPrice(bondPrice, bondSettlementDate_, bondOption_->getUnderlyingDeliveryDate(), repoRate, repoDayCount);

		// Black model: C = P(0,T)*[F*N(d1) - K*N(d2)], d1 = [ln(F/K) + 0.5 * vol^2 * T] / (vol *sqrt(T)), d2 = d1 - vol * sqrt(T).

		// Note: We can either use 1) forwardOptionPrice() which is based on bondForwardPrice, or 2) back out the carry with dividend q and call the price function based on bondSpotPrice. Both give the same price.

		// double price = forwardOptionPrice(bondFwdPrice);

		double price = spotOptionPrice(bondPrice, bondFwdPrice, discountRate_, timeToExpiry_, timeInDiscounting_);

		return price;
	}

	//	Helper function to calculate the Bond Option PV, based on bond spot price, repo rate, and repoDayCount. Assuming that spot bond clean price is following Black dynamtics
	double BondOptionPricer::spotOptionPrice(const double& bondPrice, const double& bondFwdPrice, const double& discountRate, const double& timeToExpiry, const double& timeToExpiryInDF) const
	{

		// Cost-of-carry b = r - q,  where r is the continoursly compounded risk free rate, q is the dividend rate.
		// Because F = P * exp[(r-q)*T], where f is the foward CLEAN price, P is the spot bond clean price, 
		// so we can back out cost-of-carry b = r - q = 1/T * ln (F/P):

		double b = 1.0 / timeToExpiryInDF * std::log(bondFwdPrice / bondPrice);

		double pValue = BlackScholes::price(bondOption_->getCallPut(), bondPrice, bondOption_->getStrike(), volProvider_->vol(), timeToExpiry, timeToExpiryInDF, discountRate, b, 0.0); // For now we assume shift is zero

		// Default and assume a Long option position unless a Short position explicitly specified. This is consistent with market practice
		const int longShortIndicator = bondOption_->getLongShort() == SHORT_POSITION ? -1 : 1;

		//scale
		pValue *= bondOption_->getNotional() * bondOption_->getLeverage() * longShortIndicator;

		return pValue;

	}


	BlackScholesGreeks BondOptionPricer::forwardOptionGreeks(const GreekTypeEnum& greekType, const double& bondFwdPrice, const NumericalGreekBump & greekBump) const
	{

		AnnuityTerm annuityTerm;

		annuityTerm.timeToPayment_ = timeInDiscounting_;
		annuityTerm.discountTerm_ = getDiscountFactor();
		annuityTerm.accrualTerm_ = bondOption_->getLeverage();
		annuityTerm.annuityFactor_ = AnnuityTerm::annuityFactor(annuityTerm.accrualTerm_, annuityTerm.discountTerm_);

		BlackScholesGreeks greeks;

		const auto callPut = bondOption_->getCallPut();
		const double strike = bondOption_->getStrike();
		const VolatilityTypeEnum volType = volProvider_->volType();
		const double vol = volProvider_->vol();

		// For bond's greek, we assume constant DiscountFactor
		const bool constantDF = true;
		
		const double shift = 0.0;

		switch (greekType)
		{
		case ANALYTICAL_GREEK_TYPE:
		{
			greeks = BlackModelUtils::blackGreeksAnalytical(volType, callPut, bondFwdPrice, strike, vol, timeToExpiry_, shift, annuityTerm, constantDF);

			break;
		}
		case NUMERICAL_GREEK_TYPE:
		{
			greeks = BlackModelUtils::blackGreeksNumerical(volType, callPut, bondFwdPrice, strike, vol, timeToExpiry_, shift, annuityTerm, greekBump, constantDF);

			break;
		}
		default:
			throw AQLCoreInvalidData("#Error: Only Analytical or Numerical Greek is supported", __FILE__, __LINE__);
			break;
		}

		//remove scaling on delta and gamma for bonds
		//remove negative sign for delta, for bonds 

		const double bpsFactor = 0.0001;

		greeks.deltaSpot = greeks.deltaSpot / bpsFactor;
		greeks.deltaSpot *= -1.0;

		greeks.gamma = greeks.gamma / (bpsFactor * bpsFactor);

		return greeks;
	}



	BlackScholesGreeks BondOptionPricer::spotOptionGreeks(const GreekTypeEnum& greekType, const double& bondSpotPrice, const double& repoRate, const DayCountEnum& repoDayCount, const NumericalGreekBump & greekBump) const
	{

		BlackScholesGreeks greeks;

		const auto callPut = bondOption_->getCallPut();
		const double strike = bondOption_->getStrike();
		const VolatilityTypeEnum volType = volProvider_->volType();
		const double vol = volProvider_->vol();

		// For bond's greek, we assume constant DiscountFactor
		const bool constantDF = true;

		const double shift = 0.0;

		// F = P * exp[(r-q)*T], where f is the foward price, P is the spot bond clean price, r is the continoursly compounded risk free rate, q is the dividend rate.
		// => q = r - 1/T * ln (F/P)

		auto underlyingBond = bondOption_->getUnderlying();

		// forward price at option's underlying delivery date
		const double bondFwdPrice = underlyingBond->forwardPrice(bondSpotPrice, bondSettlementDate_, bondOption_->getUnderlyingDeliveryDate(), repoRate, repoDayCount);

		// Cost-of-carry b = r - q,  where r is the continoursly compounded risk free rate, q is the dividend rate.
		// Because F = P * exp[(r-q)*T], where f is the foward CLEAN price, P is the spot bond clean price, 
		// so we can back out cost-of-carry b = r - q = 1/T * ln (F/P):
		double b = 1.0 / timeInDiscounting_  * std::log(bondFwdPrice / bondSpotPrice);
		
		switch (greekType)
		{
		case ANALYTICAL_GREEK_TYPE:
		{
			//TODO Yongyan
			throw AQLCoreInvalidData("#Error: Only Numerical Greek is supported", __FILE__, __LINE__);

			//// Note: Delta, Gamma, and Theta cannot be used the BlackScholes formula directly as the carry (r-q) is changing when bondSpotPrice change
			////throw AQLCoreInvalidData("#Error: Bond Option Analytical Greek is not supported", __FILE__, __LINE__);

			//greeks = BlackScholes::calculatePriceAndGreeks(callPut,
			//												bondSpotPrice,
			//												strike,
			//												vol,
			//												timeToExpiry_,
			//												timeInDiscounting_,
			//												discountRate_,
			//												b,        // cost-of-carry b = r - d
			//												shift);    // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes

			// Note: Delta and Gamma cannot be used the BlackScholes one directly as the carry is changing when bondSpotPrice change!
			// *** Derivation of new Delta': 
			// For Call: 
			// 1) C = exp(-q*T) *P*N(d1) - exp(-r*T)*K*N(d2), and q is function of P => dC/dP = exp (-q*T) * dq/dP * P * N(d1) + exp (-q*T) * N(d1). 
			// 2) Since F=P*exp(r-q)*T) => q = r - 1/T*ln(F/P) => dq/dP = 1/(P*T) 
			// Put 2) into 1), we have dC/dP =  exp (-q*T) * N(d1) * (1/T + 1), so Delta'  = Delta * (1/T + 1)

			// For Put, the derivation of new Delta is similar, 
			// dPut/dP = (-1.0) exp (-q*T) * N(-d1) * (1/T + 1), so Delta'  = Delta * (1/T + 1)

			//greeks.deltaSpot = greeks.deltaSpot * (1.0 / timeInDiscounting + 1.0);

			break;

		}
		case NUMERICAL_GREEK_TYPE:
		{
			// Note: Delta and Gamma cannot be used the BlackScholes one directly as the carry is changing when bondSpotPrice change!
			greeks.deltaSpot = spotOptionDeltaNumerical(bondSpotPrice, repoRate, repoDayCount, greekBump.deltaBump);
			greeks.gamma = spotOptionGammaNumerical(bondSpotPrice, repoRate, repoDayCount, greekBump.gammaBump);

			// We cannot use BlackScholes THETA directly as we are not bumping the timeInDiscounting
			greeks.theta = spotOptionThetaNumerical(bondSpotPrice, repoRate, repoDayCount, greekBump.thetaBump);
			
			greeks.vega = BlackScholes::numericalVega(callPut, bondSpotPrice, strike, vol, timeToExpiry_, timeInDiscounting_, discountRate_, b, shift, greekBump.vegaBump);
			
			// We cannot use BlackScholes RHO directly as we are not bumping the carry b = r - d
			greeks.rho = spotOptionRhoNumerical(bondSpotPrice, repoRate, repoDayCount, greekBump.thetaBump);

			//remove negative sign for delta, for bonds 
			greeks.deltaSpot *= -1.0;

			break;
		}
		default:
			throw AQLCoreInvalidData("#Error: Only Analytical or Numerical Greek is supported", __FILE__, __LINE__);
			break;
		}


		return greeks;
	}

	// Calculate the Numerical Delta 
	double BondOptionPricer::spotOptionDeltaNumerical(const double& bondSpotPrice, const double& repoRate, const DayCountEnum& repoDayCount, const double& bump) const
	{

		const double priceDownBump = spotOptionPrice(bondSpotPrice - bump, repoRate, repoDayCount);
		const double priceUpBump = spotOptionPrice(bondSpotPrice + bump, repoRate, repoDayCount);

		double deltaNumerical = (priceDownBump - priceUpBump) / (2 * bump);

		return deltaNumerical;
	}

	// Calculate the Numerical Gamma 
	double BondOptionPricer::spotOptionGammaNumerical( const double& bondSpotPrice, const double& repoRate, const DayCountEnum& repoDayCount, const double& bump) const
	{

		const double priceNoBump = spotOptionPrice(bondSpotPrice, repoRate, repoDayCount);
		const double priceDownBump = spotOptionPrice(bondSpotPrice - bump, repoRate, repoDayCount);
		const double priceUpBump = spotOptionPrice(bondSpotPrice + bump, repoRate, repoDayCount);

		double gammaNumerical = (priceUpBump - 2 * priceNoBump + priceDownBump) / (bump * bump);

		return gammaNumerical;
	}

	// Calculate the Numerical Theta 
	double BondOptionPricer::spotOptionThetaNumerical(const double& bondSpotPrice, const double& repoRate, const DayCountEnum& repoDayCount, const double& bump) const
	{

		auto underlyingBond = bondOption_->getUnderlying();
		AQ_REQUIRE(underlyingBond != nullptr, "The Underlying must be a Bond object, i.e. BondName cannot be empty.");

		const double bondFwdPrice = underlyingBond->forwardPrice(bondSpotPrice, bondSettlementDate_, bondOption_->getUnderlyingDeliveryDate(), repoRate, repoDayCount);

		const double priceNoBump = spotOptionPrice(bondSpotPrice, bondFwdPrice, discountRate_, timeToExpiry_, timeInDiscounting_);
		const double priceDownBump = spotOptionPrice(bondSpotPrice, bondFwdPrice, discountRate_, timeToExpiry_ - bump, timeInDiscounting_);

		const double thetaNumerical = priceDownBump - priceNoBump;

		return thetaNumerical;
	}

	// Calculate the Numerical Rho, by bumping both discountRate
	double BondOptionPricer::spotOptionRhoNumerical(const double& bondSpotPrice, const double& repoRate, const DayCountEnum& repoDayCount, const double& bump) const
	{
		auto underlyingBond = bondOption_->getUnderlying();
		AQ_REQUIRE(underlyingBond != nullptr, "The Underlying must be a Bond object, i.e. BondName cannot be empty.");

		const double bondFwdPrice = underlyingBond->forwardPrice(bondSpotPrice, bondSettlementDate_, bondOption_->getUnderlyingDeliveryDate(), repoRate, repoDayCount);
		
		const double priceNoBump = spotOptionPrice(bondSpotPrice, bondFwdPrice, discountRate_, timeToExpiry_, timeInDiscounting_);
		const double priceUpBump = spotOptionPrice(bondSpotPrice, bondFwdPrice, discountRate_ + bump, timeToExpiry_, timeInDiscounting_);

		const double rhoNumerical = (priceUpBump - priceNoBump) / bump ;

		return rhoNumerical;
	}


}


