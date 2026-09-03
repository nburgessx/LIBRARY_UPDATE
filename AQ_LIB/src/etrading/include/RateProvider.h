/*
* @brief			Class the provide the libor rates from forecastCurve and discount factors from discountCurve
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/
#pragma once

#include <string>
#include "LabelValueBlock.h"
#include "FixingTable.h"
#include "ScheduleValidation.h"

namespace etrading
{
	class RateProvider
	{
	public:

		RateProvider(const std::string& curveCollection, const std::shared_ptr<FixingTable>& fixingTable);
		RateProvider(const RateProvider& rhs);
		virtual ~RateProvider() {}

		//From payment dates to asOfDate of the curve
		virtual const DoubleVector discountFactors(const std::shared_ptr<ScheduleParameters>& schParams, const DateVector& paymentDates) const;
		virtual const DoubleVector liborRates(const std::shared_ptr<ScheduleParameters>& schParams, const std::shared_ptr<EnrichedSchedule>& schOutput) const;

		virtual const LADate asOfDate() const;

	private:

		std::string curveCollection_;
		std::shared_ptr<FixingTable> fixingTable_;

		const DoubleVector calculateOisFloatRates(size_t firstNonpastFixingDateIndex, const std::string& interpolation, const std::shared_ptr<ScheduleParameters>& schParams, const std::shared_ptr<EnrichedSchedule>& schOutput) const;

		const double calculateStubRate(const std::string& stubCurveIndex, const LAStringVector& curveIndices, const LAStringVector& curveTenors, const std::string& indexFrequency, const std::string& interpolation,
								 const std::shared_ptr<ScheduleParameters>& schParams, const std::shared_ptr<EnrichedSchedule>& schOutput) const;

	};

}
