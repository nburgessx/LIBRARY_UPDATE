/*
 * @brief			Class the defines the Constant Maturity Swap Cashflow 
 *
 * @Created:		27 March 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "CMSCashflow.h"
#include "LADateScheduleHelpers.h"

namespace etrading
{

	CMSCashflow::CMSCashflow() : FloatCashflow(),
								parRate1_( std::numeric_limits<double>::quiet_NaN() ),
								parRate2_( std::numeric_limits<double>::quiet_NaN() ),
								multiplier1_( std::numeric_limits<double>::quiet_NaN() ),
								multiplier2_( std::numeric_limits<double>::quiet_NaN() ) 
    {}
    
    CMSCashflow::CMSCashflow(const PayReceiveEnum& payReceive, double spread, 
							const LADate& fixingDate, const LADate& accrualStartDate, const LADate& accrualEndDate, int accrualDays, double accrualYearFraction, const LADate& paymentDate, 
							double notional, double leverage, const FrequencyEnum& zeroCouponSwapPaymentFreq, const CashflowTypeEnum& cashflowType)
				: FloatCashflow( payReceive, spread, fixingDate, accrualStartDate, accrualEndDate, accrualDays, accrualYearFraction, paymentDate, notional, leverage, 1.0 /*couponMultiplier*/, zeroCouponSwapPaymentFreq, cashflowType, CashFlowBespokeInfo()),
				  parRate1_(std::numeric_limits<double>::quiet_NaN()), 
				  parRate2_(std::numeric_limits<double>::quiet_NaN()),
				  multiplier1_(std::numeric_limits<double>::quiet_NaN()),
				  multiplier2_(std::numeric_limits<double>::quiet_NaN())
    {}

    CashflowPtr CMSCashflow::clone()
    {
        CashflowPtr cf = CashflowPtr(new CMSCashflow(*this));
        return cf;
    }

	void CMSCashflow::setParRate1( double parRate )
	{
		parRate1_ = parRate;
	}
	void CMSCashflow::setParRate2( double parRate )
	{
		parRate2_ = parRate;
	}
	void CMSCashflow::setMultiplier1( double multiplier )
	{
		multiplier1_ = multiplier;
	}
	void CMSCashflow::setMultiplier2( double multiplier )
	{
		multiplier2_ = multiplier;
	}

	double CMSCashflow::getParRate1() const
	{
		return parRate1_;
	}
	
	double CMSCashflow::getParRate2() const
	{
		return parRate2_;
	}
   
	
	double CMSCashflow::getMultiplier1() const
	{
		return multiplier1_;
	}
	
	double CMSCashflow::getMultiplier2() const
	{
		return multiplier2_;
	}

}
