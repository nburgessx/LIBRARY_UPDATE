#include "InflationSchedule.h"
#include "InflationCashflow.h"
#include "AQLDateScheduleHelpers.h"

#include "CurveUtilities.h"

namespace etrading
{

	InflationSchedule::InflationSchedule( const std::string& instanceName) : Schedule(instanceName )
	{
        scheduleType_ = INFLATION_SCHEDULE_TYPE;
    }

	InflationSchedule::InflationSchedule( const LabelValueBlock& scheduleLVB, const std::string& instanceName) : Schedule( instanceName )
	{
		//----------
		const std::string inputLVB = "scheduleLVB";

   		scheduleType_ = INFLATION_SCHEDULE_TYPE;
        
        // Append the Schedule Type to the parameter LVB
        inputParameters_ = LabelValueBlock( inputParameters_, IRS_KEY::SCHEDULE_TYPE, toString(scheduleType_) );

		payerReceiver_                          = toPayReceiveEnum( scheduleLVB.getCompulsoryValueAsAQLStringFromKeys(IRS_KEY::PAY_RECEIVE, IRS_KEY::PAYER_RECEIVER, inputLVB).getCString() );

		leverage_                               =  scheduleLVB.getOptionalValueAsDouble( IRS_KEY::LEVERAGE, 1. );

        accrualEndDateOrTenor_                  = scheduleLVB.getCompulsoryValueAsAQLString( IRS_KEY::MATURITY_DATE,  inputLVB );

		AQLString fixedBusinessDayAdjustment     = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys( boost::assign::list_of( IRS_KEY::FIXED_BUSINESSDAYADJUSTMENT)(IRS_KEY::BUSINESSDAYADJUSTMENT) );
        AQLString fixedCalendar	                = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys( boost::assign::list_of( IRS_KEY::FIXED_CALENDAR)(IRS_KEY::CALENDAR) );
		AQLString fixedLegFreq	                = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys( boost::assign::list_of( IRS_KEY::FIXED_FREQUENCY)(IRS_KEY::FREQUENCY) );
        AQLString fixedDayCount	                = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys( boost::assign::list_of( IRS_KEY::FIXED_DAYCOUNT)(IRS_KEY::DAYCOUNT) );

        accrualbusinessDayAdj_	                = toBusinessDayAdjustmentEnum( scheduleLVB.getOptionalValueAsAQLStringFromKeys( IRS_KEY::FIXED_ACCRUALBUSINESSDAYADJUSTMENT, IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT, fixedBusinessDayAdjustment).getCString() );
        accrualCalendar_		                = scheduleLVB.getOptionalValueAsAQLStringFromKeys( IRS_KEY::FIXED_ACCRUALCALENDAR, IRS_KEY::ACCRUALCALENDAR,	fixedCalendar);
        accrualFrequency_		                = toFrequencyEnum( scheduleLVB.getOptionalValueAsAQLStringFromKeys( IRS_KEY::FIXED_ACCRUALFREQUENCY,	IRS_KEY::ACCRUALFREQUENCY, fixedLegFreq).getCString());
		accrualDaycount_		                = toDayCountEnum( scheduleLVB.getOptionalValueAsAQLStringFromKeys( IRS_KEY::FIXED_ACCRUALDAYCOUNT, IRS_KEY::ACCRUALDAYCOUNT,	fixedDayCount).getCString());

        auto defaultBusinessDayAdj              = fixedBusinessDayAdjustment.size() != 0 ? fixedBusinessDayAdjustment : toString(accrualbusinessDayAdj_).c_str();
        auto defaultCalendar                    = fixedCalendar.size() != 0 ? fixedCalendar : accrualCalendar_;
        auto defaultFrequency                   = fixedLegFreq.size() != 0 ? fixedLegFreq : toString(accrualFrequency_).c_str();

		paymentbusinessDayAdj_	                = toBusinessDayAdjustmentEnum( scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_PAYMENTBUSINESSDAYADJUSTMENT, IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT,  defaultBusinessDayAdj).getCString());
        paymentCalendar_		                = scheduleLVB.getOptionalValueAsAQLStringFromKeys( IRS_KEY::FIXED_PAYMENTCALENDAR, IRS_KEY::PAYMENTCALENDAR, defaultCalendar );
        paymentFrequency_		                = toFrequencyEnum( scheduleLVB.getOptionalValueAsAQLStringFromKeys( IRS_KEY::FIXED_PAYMENTFREQUENCY, IRS_KEY::PAYMENTFREQUENCY, defaultFrequency ).getCString());
       
        populateNotionalAndPaymentFreqEnum( scheduleLVB );
		
        rollDayInput_	                        = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_ROLLDAY,							IRS_KEY::ROLLDAY );
        payLag_					                = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_PAYMENTLAG,							IRS_KEY::PAYMENTLAG,					"0D");
        stubType_			                    = toStubTypeEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_STUBTYPE,			IRS_KEY::STUBTYPE).getCString());

		fixingbusinessDayAdj_					= toBusinessDayAdjustmentEnum( scheduleLVB.getOptionalValueAsAQLString( IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj ).getCString());
		fixingCalendar_							= scheduleLVB.getOptionalValueAsAQLString( IRS_KEY::FIXINGCALENDAR, defaultCalendar);
		fixLag_									= scheduleLVB.getOptionalValueAsAQLString( IRS_KEY::FIXINGLAG, "0D");
		firstFixLag_							= scheduleLVB.getOptionalValueAsAQLString( IRS_KEY::FIRSTFIXINGLAG, fixLag_ );
		fixingAdvanceOrArrears_					= scheduleLVB.getOptionalValueAsAQLString( IRS_KEY::FIXINGADVANCEORARREAR, "advance" );

		inflationResetType_						= toInflationResetTypeEnum( scheduleLVB.getCompulsoryValueAsString(IRS_KEY::INFLATION_RESET_TYPE) );

        //fxFixing
        fxFixingLag_                            = scheduleLVB.getOptionalValueAsAQLString( IRS_KEY::FXFIXINGLAG, "0D" );
		fxFixingbusinessDayAdj_                 = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsAQLString(IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj).getCString() );
        fxFixingCalendar_                       = scheduleLVB.getOptionalValueAsAQLString(IRS_KEY::FXFIXINGCALENDAR, defaultCalendar);

		AQ_REQUIRE( accrualFrequency_ == AT_MATURITY_FREQUENCY, "Only AT_MATURITY inflation accrual is currently supported." );

        populateAccrualStartDates( scheduleLVB );

		calculateScheduleDates();
		
		createCashflows();

	}

	SchedulePtr InflationSchedule::clone()
	{
		SchedulePtr sch = SchedulePtr(new InflationSchedule(*this));
		return sch;
	}

	InflationSchedule::InflationSchedule(const InflationSchedule& rhs) : Schedule(rhs)
	{};


	// @brief Calculate a one-period Inflation schedule
	void InflationSchedule::calculateScheduleDates()
	{
		effectiveDate_			= etrading::stringToDate(accrualStartDate_, "#Error: Invalid 'EffectiveDate'.");
		unadjustedMaturityDate_ = validateMaturityDate(getEffectiveDate(), accrualEndDateOrTenor_);

		AQLDate accrualStart = effectiveDate_;
		AQLDate accrualEnd	= unadjustedMaturityDate_;

		std::vector<AQLDate> accrualDates;
		accrualDates.push_back( accrualStart );
		accrualDates.push_back( accrualEnd );

		accrualStartDates_.push_back( accrualStart );
		accrualEndDates_.push_back( accrualEnd );

		double bondActualYearFraction = std::numeric_limits<double>::quiet_NaN();

		bool INCLUDE_LAST_PAYMENT_DATE = false;
		double tmpYearFraction = getYearFraction( accrualStart, accrualEnd, accrualDaycount_, INCLUDE_LAST_PAYMENT_DATE );

		accrualYearFractions_.push_back(tmpYearFraction);

		// Days between accrual start and end dates, not business days
		int days = getDays(accrualStart, accrualEnd);
		accrualDays_.push_back(days);

		fixingDates_ = validateAndGenerateFixingSchedule( accrualDates,
														  toString( fixingbusinessDayAdj_ ).c_str(),
														 fixingCalendar_,
														 fixLag_,
														 fixingAdvanceOrArrears_, true /* includeLastExtraFixingDate */ );

		switch ( inflationResetType_ )
		{
		case INFLATION_RESET_TYPE_MONTHLY_INTERPOLATION:
		{
			// For monthly interpolation, follow the convention that the fixing day is the first of the month
			for ( AQLDate& fixingDate : fixingDates_ )
			{
				fixingDate.setDay(1);
			}
			break;
		}
		default:
			AQ_THROW( "Unsupported InflationResetType '" + toString( inflationResetType_ ) + "' Only MonthlyInterpolation is allowed.");
		}

		// TODO: Should we apply payLag *after* businessDayAdjustment?
		AQLDate paymentDate = AQLDateScheduleHelpers::getDate( accrualEnd, payLag_ , toString( paymentbusinessDayAdj_ ), paymentCalendar_ );
		paymentDates_.push_back( paymentDate );

	}

    void InflationSchedule::populateNotionalAndPaymentFreqEnum( const LabelValueBlock& scheduleLVB )
    {
		Schedule::populateNotionalAndPaymentFreqEnum();

		if (scheduleLVB.size() == 0)
		{
			return;
		}

		notional_ = scheduleLVB.getOptionalValueAsDoubleFromKeys( IRS_KEY::NOTIONAL, BOND_KEY::FACE_VALUE, std::numeric_limits<double>::quiet_NaN() ); 

		AQ_THROW_IF( boost::math::isnan(notional_), "Notional is a mandatory field for InflationSchedule" );

    }

    void InflationSchedule::createCashflows() 
    {
        auto cashflowSize	= accrualStartDates_.size();
		auto fixingsSize	= fixingDates_.size();
		AQ_REQUIRE( fixingsSize == ( cashflowSize + 1 ), "Wrong number of fixings. Require a start and end fixing for each cashflow." );

        std::vector<double> cashflowNotionals = getCashflowNotionals( cashflowSize ); 

        for (size_t i=0; i < cashflowSize; ++i)
		{
			auto cashflowType = (i == cashflowSize - 1) ? NORMAL_LAST_CASHFLOW_TYPE : NORMAL_CASHFLOW_TYPE;

            double cashflowNotional = cashflowNotionals[i];

			CashflowPtr cf = CashflowPtr( new InflationCashflow( payerReceiver_, fixingDates_[0], accrualStartDates_[i], accrualEndDates_[i], accrualDays_[i], accrualYearFractions_[i], paymentDates_[i], cashflowNotional, leverage_, paymentFreqEnum_, cashflowType));

			cf->setFixingEndDate( fixingDates_[i+1] );

            cashflows_.push_back(cf);
		}
    }

	// Specify the columns which may appear in the cashflow table display
	std::unordered_set<CashflowHeaderEnum,EnumClassHash> InflationSchedule::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			FIXING_DATE_HEADER
			,FIXING_END_DATE_HEADER
			,ACCRUAL_START_HEADER
			,ACCRUAL_END_HEADER
			,ACCRUAL_DAYS_HEADER
			,ACCRUAL_YEAR_FRACTIONS_HEADER
			,PAYMENT_DATE_HEADER

			,NOTIONAL_HEADER
			,NOTIONAL_EXCHANGE_HEADER
			,LEVERAGE_HEADER
		};

		return expectedList;
	}

	/* @brief	Specifies the inflation index level at the start and end of the schedule
	*  @param	baseIndex	The base / reference inflation level at the start of the inflation schedule
	*  @param	resetIndex	The reset inflation level at the end of the inflation schedule
	*/
	void InflationSchedule::setInflationIndex(const double baseIndex, const double resetIndex)
	{
		auto cashflowSize = accrualStartDates_.size();

		for (size_t i = 0; i < cashflowSize; ++i)
		{
			CashflowPtr curCashflow = cashflows_[i];

			// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
			std::shared_ptr<InflationCashflow> inflationCashflow = std::static_pointer_cast<InflationCashflow>( curCashflow );

			inflationCashflow->setBaseIndex(baseIndex);
			inflationCashflow->setResetIndex(resetIndex);
		}
	}

	// @brief Returns the inflationResetType: e.g. MonthlyInterpolation, DailyInterpolation
	InflationResetTypeEnum InflationSchedule::getInflationResetType() const
	{
		return inflationResetType_;
	}

}
