/*
* @brief			Class the defines the Schedule Output
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/
#include "EnrichedSchedule.h"

namespace etrading
{

	EnrichedSchedule::EnrichedSchedule() : fixingDates_(DateVector()),
									fixingEndDates_(DateVector()),
									paymentDates_(DateVector()),
									accrualStartDates_(DateVector()),
									accrualEndDates_(DateVector()),
									accrualYearFractions_(DoubleVector()),
									isIrregularStub_(false),
									spreads_(DoubleVector())
	{};

	EnrichedSchedule::EnrichedSchedule(const DateVector& fixingDates,
								const DateVector& fixingEndDates,
								const DateVector& paymentDates,
								const DateVector& accrualStartDates,
								const DateVector& accrualEndDates,
								const DoubleVector& accrualYearFractions,
								const bool&	isIrregularStub,
								const DoubleVector&	spreads) : fixingDates_(fixingDates), 
																fixingEndDates_(fixingEndDates), 
																paymentDates_(paymentDates), 
																accrualStartDates_(accrualStartDates),
																accrualEndDates_(accrualEndDates), 
																accrualYearFractions_(accrualYearFractions), 
																isIrregularStub_(isIrregularStub), 
																spreads_(spreads)
	{}

	const bool EnrichedSchedule::isVariableSpread() const
	{
		if (spreads_.empty())
		{
			return false;
		}

		double epsilon = 1e-20;
		double firstSpread = spreads_[0];

		for (size_t i = 1; i < spreads_.size(); ++i)
		{
			auto diff = firstSpread - spreads_[i];
			if (std::abs(diff) > epsilon)
			{
				return true;
			}
		}
		return false;
	}

	std::shared_ptr<EnrichedSchedule> EnrichedSchedule::clone()
	{
		EnrichedSchedule temp(*this);
		return std::make_shared<EnrichedSchedule>(temp);
	}

}

