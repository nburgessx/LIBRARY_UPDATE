/*
* @brief			Class the defines the Bond Option Pricer
*					Given a Swaption Trade and market data, allows the calculation of Bond Option PV and greeks
* @Created:			07 Aug 2018
* @Author:			Yongyan Zheng
* @Department:		Quant Research & Analytics
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/
#pragma once

#include "BondOption.h"
#include "Volatility.h"
#include "LabelValueBlock.h"
#include "BlackScholes.h"
#include "CoreEnumerations.h"
#include "BlackUtils.h"

namespace etrading
{
    class BondOptionPricer 
    {
    public:
		// Constructors
		BondOptionPricer( const std::shared_ptr<BondOption>& bondOption, const LabelValueBlock& valuationSettingsLVB, const double& discountRate, const DayCountEnum& discountDayCount);
        BondOptionPricer( const BondOptionPricer& rhs );
		virtual ~BondOptionPricer() {}

		/* @brief	Calculates the Option PV, based on bond forward price
		*           Uses the parameters supplied during object construction.
		*/
		double forwardOptionPrice(const double& bondFwdPrice) const;

		/* @brief	Calculates the Option PV, based on bond spot price, repo rate, and repoDayCount. Assuming that spot bond clean price is following Black dynamtics
		*           Uses the parameters supplied during object construction.
		*/
		double spotOptionPrice(const double& bondPrice, const double& repoRate, const DayCountEnum& repoDayCount) const;

		BlackScholesGreeks forwardOptionGreeks(const GreekTypeEnum& greekType, const double& bondFwdPrice, const NumericalGreekBump & greekBump) const;
		BlackScholesGreeks spotOptionGreeks(const GreekTypeEnum& greekType, const double& bondFwdPrice, const double& repoRate, const DayCountEnum& repoDayCount, const NumericalGreekBump & greekBump) const;

	private:

		std::shared_ptr<BondOption> bondOption_;
		std::shared_ptr<Volatility> volProvider_;

		LADate asOfDate_;
		LADate valuationDate_; 
		LADate bondSettlementDate_;

		double discountRate_;
		DayCountEnum discountDayCount_;

		PricingModelEnum pricingModel_;

		// TimeToExpiry from asOfDate to ExpiryDate, using option dayCount
		double timeToExpiry_;

		// TimeToDelivery in Discount Factor is from valuationDate to DeliveryDate, using discountingDayCount
		double timeInDiscounting_;
		double repoRateTime_;

		//	Helper function to calculate the Bond Option PV, based on bond spot price, repo rate, and repoDayCount. Assuming that spot bond clean price is following Black dynamtics
		double spotOptionPrice(const double& bondPrice, const double& bondFwdPrice, const double& discountRate, const double& timeToExpiry, const double& timeToExpiryInDF) const;

		double spotOptionDeltaNumerical(const double& bondSpotPrice, const double& repoRate, const DayCountEnum& repoDayCount, const double& bump) const;
		double spotOptionGammaNumerical(const double& bondSpotPrice, const double& repoRate, const DayCountEnum& repoDayCount, const double& bump) const;
		double spotOptionThetaNumerical(const double& bondSpotPrice, const double& repoRate, const DayCountEnum& repoDayCount, const double& bump) const;
		double spotOptionRhoNumerical(const double& bondSpotPrice, const double& repoRate, const DayCountEnum& repoDayCount, const double& bump) const;

		//Helper function: Option's discount factor from valuationDate to deliveryDate
		double getDiscountFactor() const;

	};

}


