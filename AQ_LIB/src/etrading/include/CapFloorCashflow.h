#pragma once

#include "OptionCashflow.h"

namespace etrading
{
    class CapFloorCashflow : public OptionCashflow
	{

    public:
		CapFloorCashflow(const std::shared_ptr<CoreCashflow>& coreCashflow, 
						const CallOrPutEnum & callPut, 
						const VolatilityTypeEnum& volType, 
						const double& vol, 
						const double& expiryYearFraction,
						const double& paymentYearFraction,
						const double& floatRate,
						const double& discountFactor,
						const CCY& currency,
						const CCY& valuationCurrency);

		virtual ~CapFloorCashflow() {}

		CapFloorCashflow(const CapFloorCashflow & rhs);

		//Override
		std::shared_ptr<OptionCashflow> clone();

		//Override
		const double coupon() const;

		//Override, each cashflow is a caplet/floorlet
		const BlackScholesGreeks greeks(const GreekTypeEnum& greekType, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump) const;

	private:
		
		const double accrualAmount() const;
		const double annuityFactor() const;
	
    };


}
