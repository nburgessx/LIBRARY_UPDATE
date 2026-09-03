#pragma once

#include <boost/math/special_functions/fpclassify.hpp> 
#include "SwapValidation.h"
#include "ScheduleValidation.h"
#include "ParameterValidation.h"
#include "CoreEnumerations.h"

namespace etrading
{

    class CoreCashflow
    {
    public:
		CoreCashflow(const AQLDate& fixingDate, 
					const AQLDate& accrualStartDate, 
					const AQLDate& accrualEndDate, 
					const AQLDate& paymentDate, 
					const double& accrualYearFraction,
					const double& notional,
					const double& leverage,
					const double& strikeRate,
					const double& spread);

        virtual ~CoreCashflow() {}

		CoreCashflow(const CoreCashflow & rhs);

		std::shared_ptr<CoreCashflow> clone();

		// Getter
		const AQLDate& fixingDate() const { return fixingDate_; };
		const AQLDate& accrualStartDate() const { return accrualStartDate_; };
		const AQLDate& accrualEndDate() const { return accrualEndDate_; };
		const AQLDate& paymentDate() const { return paymentDate_; };
		const double accrualYearFraction() const { return accrualYearFraction_; };

		const double notional() const { return notional_; };
		const double leverage() const { return leverage_; };
		const double strikeRate() const { return strikeRate_; };
		const double spread() const { return spread_; };

	private:

		AQLDate fixingDate_;
		AQLDate accrualStartDate_;
		AQLDate accrualEndDate_;
		AQLDate paymentDate_;
		double accrualYearFraction_;

		double notional_;
		double leverage_;
		double strikeRate_;
		double spread_;


    };

}
