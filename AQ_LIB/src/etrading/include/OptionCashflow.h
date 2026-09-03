/*
* @brief			Base Class the defines the Option cashflow
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/
#pragma once

#include <boost/math/special_functions/fpclassify.hpp> 
#include "CoreEnumerations.h"
#include "CoreCashflow.h"
#include "BlackScholes.h"

namespace etrading
{

    class OptionCashflow
    {
    public:

		OptionCashflow(const std::shared_ptr<CoreCashflow>& coreCashflow,
					const CallOrPutEnum & callPut,
					const VolatilityTypeEnum& volType,
					const double& vol,
					const double& expiryYearFraction, 
					const double& paymentYearFraction,
					const double& floatOrSwapRate,
					const double& discountFactor,
					const CCY& currency,
					const CCY& valuationCurrency);

		virtual ~OptionCashflow() {}

		OptionCashflow(const OptionCashflow & rhs);

		virtual std::shared_ptr<OptionCashflow> clone() = 0;

		virtual const double coupon() const = 0;

		const double couponPV() const;

		//Getter
		const CallOrPutEnum callPut() const	{ return callPut_; };
		const VolatilityTypeEnum volType() const { return volType_; };
		const double vol() const { return vol_; };
		const double expiryYearFraction() const	{ return expiryYearFraction_; };
		const double paymentYearFraction() const { return paymentYearFraction_; };
		const double floatOrSwapRate() const	{ return floatOrSwapRate_; };
		const double discountFactor() const { return discountFactor_; };
		const CCY currency() const { return currency_; };
		const CCY valuationCurrency() const { return valuationCurrency_; };

		const std::shared_ptr<CoreCashflow>& coreCashflow() const { return coreCashflow_; };

		//Useful for cap/floor, where each cashflow is a caplet/floorlet
		virtual const BlackScholesGreeks greeks(const GreekTypeEnum& greekType, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump) const;


	protected:

		std::shared_ptr<CoreCashflow> coreCashflow_;

		CallOrPutEnum callPut_;
		VolatilityTypeEnum volType_;
		double vol_;
		double expiryYearFraction_;
		double paymentYearFraction_;
		double floatOrSwapRate_;
		double discountFactor_; //DF from paymentDate to asOfDate
		CCY currency_;
		CCY valuationCurrency_;

    };

}
