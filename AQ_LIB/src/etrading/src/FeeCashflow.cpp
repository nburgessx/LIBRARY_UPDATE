#include "FeeCashflow.h"
#include "AQLDateScheduleHelpers.h"
#include "Variant.h"
#include "SwapValidation.h"

namespace etrading
{

	FeeCashflow::FeeCashflow(const PayReceiveEnum& payReceive, const AQLDate& paymentDate, double amount) 
		: amount_(amount), 
        Cashflow(payReceive, 
        AQLDate(), //fixingDate
        AQLDate(), //accrualStartDate
        AQLDate(), //accrualEndDate
        0, //accrualDays
        0, //accrualYearFraction
        paymentDate, 
        0, //notional
        1.0, //leverage
		1.0, //couponMultiplier
        NONE_FREQUENCY, //PaymentFrequencyEnum
		NORMAL_CASHFLOW_TYPE,
		CashFlowBespokeInfo()) 
    {}

    CashflowPtr FeeCashflow::clone()
    {
        CashflowPtr cf = CashflowPtr(new FeeCashflow(*this));
        return cf;
    }

    void FeeCashflow::flipPayerReceiver()
    {
        payReceive_ = flipPayReceive(getPayReceive());
    }

	double FeeCashflow::getCoupon( const CashflowData& CashflowData ) const 
	{
        double coupon = amount_;
        if (coupon != 0)
        {
            coupon = coupon * getIndicatorFromPayRec(payReceive_);
        }

		return coupon;
	}

	double FeeCashflow::getAmount() const 
	{
		return amount_;
	}

}

