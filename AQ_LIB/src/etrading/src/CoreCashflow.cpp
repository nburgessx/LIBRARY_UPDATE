/*
* @brief			Class the defines the Date Cashflow
* @Created:			05 Feb 2018
* @Author:			Yongyan Zheng
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/
#include "CoreCashflow.h"

namespace etrading
{

	CoreCashflow::CoreCashflow(const LADate& fixingDate, 
		const LADate& accrualStartDate, 
		const LADate& accrualEndDate, 
		const LADate& paymentDate, 
		const double& accrualYearFraction,
		const double& notional,
		const double& leverage,
		const double& strikeRate,
		const double& spread) : fixingDate_(fixingDate), 
								accrualStartDate_(accrualStartDate), 
								accrualEndDate_(accrualEndDate), 
								paymentDate_(paymentDate), 
								accrualYearFraction_(accrualYearFraction),
								notional_(notional),
								leverage_(leverage),
								strikeRate_(strikeRate),
								spread_(spread)
		{}

	CoreCashflow::CoreCashflow(const CoreCashflow & rhs) : fixingDate_(rhs.fixingDate_), 
														accrualStartDate_(rhs.accrualStartDate_), 
														accrualEndDate_(rhs.accrualEndDate_), 
														paymentDate_(rhs.paymentDate_), 
														accrualYearFraction_(rhs.accrualYearFraction_),
														notional_(rhs.notional_),
														leverage_(rhs.leverage_),
														strikeRate_(rhs.strikeRate_),
														spread_(rhs.spread_)
	{}

	/*
	* The clone() method invokes the CoreCashflow copy constructor. Note that we deliberately do not provide a copy constructor,
	* and instead use the compiler default. We do this because there are no pointer members that
	* need careful copying. More subtley, we want the string members variables of this class to deep copy (rather than shallow copy
	* with an increased reference count). This allows us to use the cloned CoreCashflow instance from a different thread.
	*/
	std::shared_ptr<CoreCashflow> CoreCashflow::clone()
    {
		CoreCashflow temp(*this);
		return std::make_shared<CoreCashflow>(temp);
    }


}

