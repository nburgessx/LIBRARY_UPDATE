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
		CoreCashflow(const LADate& fixingDate, 
					const LADate& accrualStartDate, 
					const LADate& accrualEndDate, 
					const LADate& paymentDate, 
					const double& accrualYearFraction,
					const double& notional,
					const double& leverage,
					const double& strikeRate,
					const double& spread);

        virtual ~CoreCashflow() {}

		CoreCashflow(const CoreCashflow & rhs);

		std::shared_ptr<CoreCashflow> clone();

		// Getter
		const LADate& fixingDate() const { return fixingDate_; };
		const LADate& accrualStartDate() const { return accrualStartDate_; };
		const LADate& accrualEndDate() const { return accrualEndDate_; };
		const LADate& paymentDate() const { return paymentDate_; };
		const double accrualYearFraction() const { return accrualYearFraction_; };

		const double notional() const { return notional_; };
		const double leverage() const { return leverage_; };
		const double strikeRate() const { return strikeRate_; };
		const double spread() const { return spread_; };

	private:

		LADate fixingDate_;
		LADate accrualStartDate_;
		LADate accrualEndDate_;
		LADate paymentDate_;
		double accrualYearFraction_;

		double notional_;
		double leverage_;
		double strikeRate_;
		double spread_;


    };

}
