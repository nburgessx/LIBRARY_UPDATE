/*
* @brief			Base Class the defines the Option cashflow
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/
#include "OptionCashflow.h"

namespace etrading
{
  
    OptionCashflow::OptionCashflow(const std::shared_ptr<CoreCashflow>& coreCashflow,
								const CallOrPutEnum & callPut,
								const VolatilityTypeEnum & volType,
								const double& vol,
								const double& expiryYearFraction,
								const double& paymentYearFraction,
								const double& floatOrSwapRate,
								const double& discountFactor,
								const CCY& currency,
								const CCY& valuationCurrency)

		: coreCashflow_(coreCashflow), 
		callPut_(callPut), 
		volType_(volType), 
		vol_(vol), 
		expiryYearFraction_(expiryYearFraction), 
		paymentYearFraction_(paymentYearFraction),
		floatOrSwapRate_(floatOrSwapRate),
		discountFactor_(discountFactor),
		currency_(currency),
		valuationCurrency_(valuationCurrency)
	{}

	OptionCashflow::OptionCashflow(const OptionCashflow & rhs) 
		: callPut_(rhs.callPut_), 
		volType_(rhs.volType_),
		vol_(rhs.vol_),
		expiryYearFraction_(rhs.expiryYearFraction_),
		paymentYearFraction_(rhs.paymentYearFraction_),
		floatOrSwapRate_(rhs.floatOrSwapRate_),
		discountFactor_(rhs.discountFactor_),
		currency_(rhs.currency_),
		valuationCurrency_(rhs.valuationCurrency_)
	{
		if (rhs.coreCashflow_ != nullptr)
		{
			coreCashflow_ = rhs.coreCashflow_->clone();
		}
	}

	const double OptionCashflow::couponPV() const
	{
		double cpv = coupon() * discountFactor_;

        return cpv;
	}

	const BlackScholesGreeks OptionCashflow::greeks(const GreekTypeEnum& greekType, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump) const
	{
		throw LACoreInvalidData("#Error: greeks() is not supported", __FILE__, __LINE__);
	}

}

