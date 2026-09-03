#pragma once

#include <string>
#include "RateProvider.h"

namespace etrading
{
	class RateProviderExternal : public RateProvider
	{
	public:
		
		RateProviderExternal(const LADate& asOfDate, const DoubleVector& dfs, const DoubleVector& liborRates);
		RateProviderExternal(const RateProviderExternal& rhs);
		virtual ~RateProviderExternal() {}

		//Override
		const DoubleVector discountFactors(const std::shared_ptr<ScheduleParameters>& schParams, const DateVector& paymentDates) const;
		//Override
		const DoubleVector liborRates(const std::shared_ptr<ScheduleParameters>& schParams, const std::shared_ptr<EnrichedSchedule>& schOutput) const;
		//Override
		const LADate asOfDate() const;

	private:

		LADate asOfDate_;
		DoubleVector discountFactors_;
		DoubleVector liborRates_;

	};

}
