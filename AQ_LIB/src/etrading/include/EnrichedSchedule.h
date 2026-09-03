/*
* @brief			Class the defines the Schedule Output
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/
#pragma once

#include "CoreEnumerations.h"
#include "LACoreTemplateType.h"

namespace etrading
{
	class EnrichedSchedule
	{
	public:

		EnrichedSchedule();
		EnrichedSchedule(const DateVector&		fixingDates,
						const DateVector&		fixingEndDates,
						const DateVector&		paymentDates,
						const DateVector&		accrualStartDates,
						const DateVector&		accrualEndDates,
						const DoubleVector&		accrualYearFractions,
						const bool&				isIrregularStub,
						const DoubleVector&		spreads);

		std::shared_ptr<EnrichedSchedule> clone();

		const bool isVariableSpread() const;

		//Getter
		const DateVector& fixingDates() const { return fixingDates_; };
		const DateVector& fixingEndDates() const { return fixingEndDates_; };
		const DateVector& paymentDates() const { return paymentDates_; };
		const DateVector& accrualStartDates() const { return accrualStartDates_; };
		const DateVector& accrualEndDates() const { return accrualEndDates_; };
		const DoubleVector accrualYearFractions() const { return accrualYearFractions_; };
		const bool isIrregularStub() const { return isIrregularStub_; };
		const DoubleVector spreads() const { return spreads_; };

	private:

		DateVector		fixingDates_; //Fixing start dates
		DateVector		fixingEndDates_;
		DateVector		paymentDates_;
		DateVector		accrualStartDates_;
		DateVector		accrualEndDates_;
		DoubleVector	accrualYearFractions_;
		bool			isIrregularStub_;
		DoubleVector	spreads_;

	};

}
