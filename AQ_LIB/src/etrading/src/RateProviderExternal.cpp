/*
* @brief			Class the provide the libor rates and discount factors from external source like BB
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/
#include "RateProviderExternal.h"

namespace etrading
{

	RateProviderExternal::RateProviderExternal(const LADate& asOfDate, const DoubleVector& dfs, const DoubleVector& liborRates) : RateProvider("", {} ), asOfDate_(asOfDate), discountFactors_(dfs), liborRates_(liborRates)
	{}

	RateProviderExternal::RateProviderExternal(const RateProviderExternal& rhs) : RateProvider(rhs), asOfDate_(rhs.asOfDate_), discountFactors_(rhs.discountFactors_), liborRates_(rhs.liborRates_)
	{}

	const LADate RateProviderExternal::asOfDate() const
	{
		return asOfDate_;
	}

	const DoubleVector RateProviderExternal::discountFactors(const std::shared_ptr<ScheduleParameters>& schParams, const DateVector& paymentDates) const
	{
		return discountFactors_;
	}

	const DoubleVector RateProviderExternal::liborRates(const std::shared_ptr<ScheduleParameters>& schParams, const std::shared_ptr<EnrichedSchedule>& schOutput) const
	{
		return liborRates_;
	}

}

