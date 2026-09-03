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

