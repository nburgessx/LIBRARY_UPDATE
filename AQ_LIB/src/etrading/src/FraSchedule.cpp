#include "FraSchedule.h"
#include "FraCashflow.h"
#include "SwapValidation.h"
#include "CurveUtilities.h"
#include "CurveValidation.h"
#include "ExceptionMacros.h"

namespace etrading
{

	FraSchedule::FraSchedule(const std::string& instanceName) : Schedule(instanceName), floatRate_(std::numeric_limits<double>::quiet_NaN()), strikeRate_(std::numeric_limits<double>::quiet_NaN())
	{
        scheduleType_ = FRA_SCHEDULE_TYPE;
    }

	FraSchedule::FraSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : Schedule(instanceName)
	{
		const std::string inputLVB = "scheduleLVB";
        scheduleType_ = FRA_SCHEDULE_TYPE;
		inputParameters_ = LabelValueBlock( scheduleLVB, IRS_KEY::SCHEDULE_TYPE, toString(scheduleType_) );

		std::string payRec	= scheduleLVB.getCompulsoryValueAsLAString(IRS_KEY::PAY_RECEIVE, inputLVB).getCString();
		//buyer means pay fixed
		if (same(payRec, "BUY"))
		{
			payerReceiver_ = PAY_PAYRECEIVE_ENUM;
		} 
		//seller means receive fixed
		else if (same(payRec, "SELL"))
		{
			payerReceiver_ = RECEIVE_PAYRECEIVE_ENUM;
		}
		else
		{
			payerReceiver_ = toPayReceiveEnum(payRec);
		}

		leverage_ =  scheduleLVB.getOptionalValueAsDouble(IRS_KEY::LEVERAGE, 1.);

		strikeRate_	= scheduleLVB.getCompulsoryValueAsDoubleFromKeys( IRS_KEY::STRIKE_RATE, IRS_KEY::FIXED_RATE, inputLVB );

		accrualEndDateOrTenor_	= scheduleLVB.getCompulsoryValueAsLAString( IRS_KEY::MATURITY_DATE,  inputLVB );
		
   		accrualbusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getCompulsoryValueAsLAStringFromMultipleKeys(boost::assign::list_of (IRS_KEY::BUSINESSDAYADJUSTMENT)
																											   (IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT)
																											   (IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT)
																											   (IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT),  inputLVB ).getCString());

        accrualCalendar_	= scheduleLVB.getCompulsoryValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::CALENDAR)(IRS_KEY::ACCRUALCALENDAR)(IRS_KEY::FLOAT_CALENDAR)(IRS_KEY::FLOAT_ACCRUALCALENDAR),  inputLVB );
        accrualDaycount_	= toDayCountEnum(scheduleLVB.getCompulsoryValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::DAYCOUNT)(IRS_KEY::ACCRUALDAYCOUNT)(IRS_KEY::FLOAT_DAYCOUNT)(IRS_KEY::FLOAT_ACCRUALDAYCOUNT),  inputLVB ).getCString());
	    accrualFrequency_	= toFrequencyEnum(scheduleLVB.getCompulsoryValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FREQUENCY)(IRS_KEY::ACCRUALFREQUENCY)(IRS_KEY::FLOAT_FREQUENCY)(IRS_KEY::FLOAT_ACCRUALFREQUENCY),  inputLVB ).getCString());

		notional_= scheduleLVB.getCompulsoryValueAsDouble(IRS_KEY::NOTIONAL);
        notionalExchangeEnum_	= toNotionalExchangeEnum(scheduleLVB.getOptionalValueAsLAString(IRS_KEY::NOTIONAL_EXCHANGE, "NONE").getCString());
        MLIB_REQUIRE( notionalExchangeEnum_ == NONE_NE,    "FRA's NotionalExchange has to be 'NONE'" );

		fixingbusinessDayAdj_   = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT,		IRS_KEY::FLOAT_FIXINGBUSINESSDAYADJUSTMENT, toString(accrualbusinessDayAdj_).c_str() ).getCString());
        fixingCalendar_         = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXINGCALENDAR,					IRS_KEY::FLOAT_FIXINGCALENDAR, accrualCalendar_);
		fixLag_					= scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXINGLAG,						IRS_KEY::FLOAT_FIXINGLAG, "0D");
        firstFixLag_            = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FLOAT_FIRSTFIXINGLAG,				IRS_KEY::FIRSTFIXINGLAG, fixLag_);
		fixingAdvanceOrArrears_	= scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXINGADVANCEORARREAR,			IRS_KEY::FLOAT_FIXINGADVANCEORARREAR, "advance");

		firstStub_		        = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIRSTSTUBDATE,					IRS_KEY::FLOAT_FIRSTSTUBDATE);
        lastStub_	            = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::LASTSTUBDATE,						IRS_KEY::FLOAT_LASTSTUBDATE);
        rollDayInput_	        = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::ROLLDAY,							IRS_KEY::FLOAT_ROLLDAY);
        stubType_			    = toStubTypeEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::STUBTYPE,			IRS_KEY::FLOAT_STUBTYPE).getCString());

        paymentFrequency_           = accrualFrequency_;
        paymentbusinessDayAdj_      = accrualbusinessDayAdj_;
        paymentCalendar_            = accrualCalendar_;

        populateAccrualStartDates(scheduleLVB);

		MLIB_REQUIRE(firstStub_.size() == 0, "User cannot set FirstStubDate for FRA");
		MLIB_REQUIRE(lastStub_.size() == 0, "User cannot set LastStubDate for FRA");
		MLIB_REQUIRE(stubType_ == NONE_STUBTYPE, "User cannot set StubType for FRA");

		// 'lastStub_' is set to be the same as effectiveDate
		lastStub_ = accrualStartDate_;

        //Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates, fixingDates
		calculateScheduleDates();
		// Fra's payment date is the same as accrualStartDate
		paymentDates_ = accrualStartDates_;

        //Populate the cashflows based on schedule inputs
        createCashflows(); 
		
		//This flag is to control stubRate calculation: if it is REGULAR, the trade's schedule is treated as regular, i.e. no stub rate calculation will be used. 
		fraStyle_ = toFraStyleEnum(scheduleLVB.getCompulsoryValueAsLAString(IRS_KEY::FRA_STYLE, inputLVB).getCString());

		// If it is REGULAR, the trade's schedule is treated as regular, i.e. no stub rate calculation will be used. Only check irregular stub for BROKEN_DATED Fra
		if (fraStyle_ == BROKEN_DATED_FRA_STYLE)
		{
			// Determine if the Stubs are Irregular Stub
			determineIsIrregularStub();
		}

    }

    void FraSchedule::createCashflows() 
    {
		auto cashflowSize = accrualStartDates_.size();

        std::vector<double> cashflowNotionals = getCashflowNotionals(cashflowSize); 

        for (size_t i=0; i < cashflowSize; ++i)
		{
			CashflowPtr cf = CashflowPtr(new FraCashflow(payerReceiver_, strikeRate_, getFixingDate(i), accrualStartDates_[i], accrualEndDates_[i], accrualDays_[i], accrualYearFractions_[i], paymentDates_[i], cashflowNotionals[i], leverage_));
            
            cashflows_.push_back(cf);
		}
    }

    SchedulePtr FraSchedule::clone()
    {
        SchedulePtr sch = SchedulePtr(new FraSchedule(*this));
        return sch;
    }

	FraSchedule::FraSchedule(const FraSchedule& rhs) : Schedule(rhs), strikeRate_(rhs.strikeRate_), floatRate_(rhs.floatRate_)
	{}

	double FraSchedule::getFixedRate() const
	{
		return strikeRate_;
	}

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> FraSchedule::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			ACCRUAL_START_HEADER
			,ACCRUAL_END_HEADER
			,ACCRUAL_DAYS_HEADER
			,ACCRUAL_YEAR_FRACTIONS_HEADER
			,PAYMENT_DATE_HEADER

			,NOTIONAL_HEADER
			,STRIKE_RATE_HEADER
		};

		return expectedList;
	};


}
