#include "ProtectionSchedule.h"
#include "ProtectionCashflow.h"
#include "CurveUtilities.h"

namespace etrading
{

	ProtectionSchedule::ProtectionSchedule(const std::string& instanceName) : Schedule(instanceName)
	{
        scheduleType_ = PROTECTION_SCHEDULE_TYPE;
    }

	ProtectionSchedule::ProtectionSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : Schedule( instanceName )
	{
		//----------
		const std::string inputLVB = "scheduleLVB";

   		scheduleType_ = PROTECTION_SCHEDULE_TYPE;
        
        // Create new LVB with Schedule Type
        inputParameters_ = LabelValueBlock( inputParameters_, IRS_KEY::SCHEDULE_TYPE, toString(scheduleType_) );

		payerReceiver_                          = toPayReceiveEnum(scheduleLVB.getCompulsoryValueAsLAStringFromKeys(IRS_KEY::PAY_RECEIVE, IRS_KEY::PAYER_RECEIVER, inputLVB).getCString());

		leverage_                               =  scheduleLVB.getOptionalValueAsDouble(IRS_KEY::LEVERAGE, 1.);

        accrualEndDateOrTenor_                  = scheduleLVB.getCompulsoryValueAsLAString( IRS_KEY::MATURITY_DATE,  inputLVB );

		LAString fixedBusinessDayAdjustment     = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys( boost::assign::list_of(IRS_KEY::FIXED_BUSINESSDAYADJUSTMENT)(IRS_KEY::BUSINESSDAYADJUSTMENT) );
        LAString fixedCalendar	                = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys( boost::assign::list_of(IRS_KEY::FIXED_CALENDAR)(IRS_KEY::CALENDAR) );
		LAString fixedLegFreq	                = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys( boost::assign::list_of(IRS_KEY::FIXED_FREQUENCY)(IRS_KEY::FREQUENCY) );
        LAString fixedDayCount	                = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys( boost::assign::list_of(IRS_KEY::FIXED_DAYCOUNT)(IRS_KEY::DAYCOUNT) );

        accrualbusinessDayAdj_	                = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALBUSINESSDAYADJUSTMENT,		IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT,	fixedBusinessDayAdjustment).getCString());
        accrualCalendar_		                = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALCALENDAR,					IRS_KEY::ACCRUALCALENDAR,				fixedCalendar);
        accrualFrequency_		                = toFrequencyEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALFREQUENCY,					IRS_KEY::ACCRUALFREQUENCY,				fixedLegFreq).getCString());
		accrualDaycount_		                = toDayCountEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALDAYCOUNT,					IRS_KEY::ACCRUALDAYCOUNT,				fixedDayCount).getCString());

        auto defaultBusinessDayAdj              = fixedBusinessDayAdjustment.size() != 0 ? fixedBusinessDayAdjustment : toString(accrualbusinessDayAdj_).c_str();
        auto defaultCalendar                    = fixedCalendar.size() != 0 ? fixedCalendar : accrualCalendar_;
        auto defaultFrequency                   = fixedLegFreq.size() != 0 ? fixedLegFreq : toString(accrualFrequency_).c_str();

		paymentbusinessDayAdj_	                = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTBUSINESSDAYADJUSTMENT,		IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT,	defaultBusinessDayAdj).getCString());
        paymentCalendar_		                = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTCALENDAR,					IRS_KEY::PAYMENTCALENDAR,				defaultCalendar );
        paymentFrequency_		                = toFrequencyEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTFREQUENCY,					IRS_KEY::PAYMENTFREQUENCY,				defaultFrequency).getCString());
       
        populateNotionalAndPaymentFreqEnum(scheduleLVB);
		
        // Allowable Stub LVB Keys
        std::vector<std::string> firstStubKeys;
        firstStubKeys.push_back( IRS_KEY::FIXED_FIRSTSTUBDATE );
        firstStubKeys.push_back( IRS_KEY::FIRSTSTUBDATE );
        firstStubKeys.push_back( BOND_KEY::FIRST_COUPON_DATE );

        std::vector<std::string> lastStubKeys;
        lastStubKeys.push_back( IRS_KEY::FIXED_LASTSTUBDATE );
        lastStubKeys.push_back( IRS_KEY::LASTSTUBDATE );
        lastStubKeys.push_back( BOND_KEY::LAST_COUPON_DATE );

		firstStub_				                = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys( firstStubKeys );
        lastStub_		                        = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys( lastStubKeys );

        rollDayInput_	                        = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ROLLDAY,							IRS_KEY::ROLLDAY );
        payLag_					                = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTLAG,							IRS_KEY::PAYMENTLAG,					"0D");
        stubType_			                    = toStubTypeEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_STUBTYPE,			IRS_KEY::STUBTYPE).getCString());

        //fxFixing
        fxFixingLag_                            = scheduleLVB.getOptionalValueAsLAString( IRS_KEY::FXFIXINGLAG, "0D" );
		fxFixingbusinessDayAdj_                 = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAString(IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj).getCString() );
        fxFixingCalendar_                       = scheduleLVB.getOptionalValueAsLAString(IRS_KEY::FXFIXINGCALENDAR, defaultCalendar);

        populateAccrualStartDates(scheduleLVB);

        //Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates, fixingDates
		Schedule::calculateScheduleDates();

		//Populate the cashflows based on schedule inputs
		createCashflows();

	}

    void ProtectionSchedule::populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB)
    {
		Schedule::populateNotionalAndPaymentFreqEnum();

		if (scheduleLVB.size() == 0)
		{
			return;
		}

		notional_ = scheduleLVB.getOptionalValueAsDoubleFromKeys( IRS_KEY::NOTIONAL, BOND_KEY::FACE_VALUE, std::numeric_limits<double>::quiet_NaN() ); 

		if (boost::math::isnan(notional_))
        {
        	throw LACoreInvalidData( "#Error: Notional is a mandatory field for ProtectionSchedule", __FILE__, __LINE__ );
        }

    }

    void ProtectionSchedule::createCashflows() 
    {
		// Use the first accrualStartDate as the paymentDate
        auto paymentDate = accrualStartDates_.at(0);
        auto cashflowSize = accrualStartDates_.size();

        std::vector<double> cashflowNotionals = getCashflowNotionals( cashflowSize ); 

        for (size_t i=0; i < cashflowSize; ++i)
		{
			auto cashflowType = (i == cashflowSize - 1) ? NORMAL_LAST_CASHFLOW_TYPE : NORMAL_CASHFLOW_TYPE;

            double cashflowNotional = cashflowNotionals[i];

			CashflowPtr cf = CashflowPtr( new ProtectionCashflow( payerReceiver_, accrualStartDates_[i], accrualEndDates_[i], accrualDays_[i], accrualYearFractions_[i], paymentDates_[i], cashflowNotional, leverage_, paymentFreqEnum_, cashflowType));
            cashflows_.push_back(cf);
		}
    }

    SchedulePtr ProtectionSchedule::clone()
    {
        SchedulePtr sch = SchedulePtr( new ProtectionSchedule( *this ));
        return sch;
    }

	ProtectionSchedule::ProtectionSchedule(const ProtectionSchedule& rhs) : Schedule( rhs )
	{};


	/* @brief Updates the cashflow survival / default probabilities using the hazard rate and recovery rate parameters
	*
	* @param[in]	asOfDate				The valuation date of the leg
	* @param[in]	hazardRate				The CDS hazard rate parameter, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	includeAccruedInterest	Not used for the Protection Schedule / Cashflows
	*/
	void ProtectionSchedule::setSurvivalProbabilitiesUsingHazardRate( const LADate& asOfDate, const double hazardRate, const double recoveryRate, const bool includeAccruedInterest )
	{
		size_t cashflowSize = cashflows_.size();

		double prevSurvivalProbability = 1.0;
		double survivalProbability = 1.0;
		LADate prevPaymentDate = asOfDate;

		// Calculate and set survival / default probabilities for each cashflow
		for (size_t i=0; i<cashflowSize; i++ )
		{
			CashflowPtr curCashflow = cashflows_[i];

			// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
			std::shared_ptr<ProtectionCashflow> protectionCashflow = std::static_pointer_cast<ProtectionCashflow>( curCashflow );
			protectionCashflow->setRecoveryRate( recoveryRate );

			const LADate& paymentDate = paymentDates_[i];

			double paymentYearFraction = getYearFraction(prevPaymentDate, paymentDate, accrualDaycount_, false);	

			const double survivalFactor = exp( -hazardRate * paymentYearFraction );
			survivalProbability *= survivalFactor;
			protectionCashflow->setSurvivalProbability( survivalProbability );

			double marginalDefaultProbability = prevSurvivalProbability - survivalProbability;
			protectionCashflow->setMarginalDefaultProbability( marginalDefaultProbability );

			prevSurvivalProbability = survivalProbability;
			prevPaymentDate = paymentDate;
		}	
	}

	/* @brief Updates the cashflow survival / default probabilities using the provided credit model.
	*
	* @param[in]	asOfDate				The valuation date of the leg
	* @param[in]	creditModel				The calibrated credit model
	*/
	void ProtectionSchedule::setSurvivalProbabilitiesUsingCreditModel( const LADate& asOfDate, const CreditModel& creditModel )
	{
		const double recoveryRate = creditModel.getRecoveryRate();
		double prevSurvivalProbability = creditModel.getSurvivalProbability( effectiveDate_ );

		// Calculate and set survival / default probabilities for each cashflow
		size_t cashflowSize = cashflows_.size();
		for (size_t i=0; i<cashflowSize; i++ )
		{
			CashflowPtr curCashflow = cashflows_[i];

			// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
			std::shared_ptr<ProtectionCashflow> protectionCashflow = std::static_pointer_cast<ProtectionCashflow>( curCashflow );
			protectionCashflow->setRecoveryRate( recoveryRate );

			const LADate& paymentDate = paymentDates_[i];
			const double survivalProbability = creditModel.getSurvivalProbability( paymentDate );
			protectionCashflow->setSurvivalProbability( survivalProbability );

			double marginalDefaultProbability = prevSurvivalProbability - survivalProbability;
			protectionCashflow->setMarginalDefaultProbability( marginalDefaultProbability );

			prevSurvivalProbability = survivalProbability;
		}	
	}

    const DataSchema ProtectionSchedule::generateCashflowSchema(const std::string& schemaName) const
    {
		return Schedule::generateCashflowSchema( schemaName );
    }

    std::map<std::string, std::vector<std::string>> ProtectionSchedule::getCashflowDataMap() const
    {
		return Schedule::getCashflowDataMap();
    }

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> ProtectionSchedule::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			ACCRUAL_START_HEADER
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

}
