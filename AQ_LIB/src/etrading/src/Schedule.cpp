#include "Schedule.h"
#include "ParameterValidation.h"
#include "SwapValidation.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "LACurvePricingObject.h"
#include "LACurveForwardRateHelpers.h"
#include "LADateScheduleHelpers.h"
#include "BondAccrualPeriods.h"
#include "BondUtilities.h"
#include "SwapUtilities.h"
#include <string> 

namespace etrading
{

	Schedule::Schedule(const std::string& instanceName) : IsLWOObject(instanceName, SCHEDULE),
        scheduleType_(NONE_SCHEDULE_TYPE),
		bespokeScheduleType_(NONE_BESPOKE_SCHEDULE),
		payerReceiver_(NONE_PAYRECEIVE_ENUM),
        notional_(std::numeric_limits<double>::quiet_NaN()), 
        amortization_(0), 
        amortFrequency_(0), 
        futureValueNotional_(std::numeric_limits<double>::quiet_NaN()),	
        paymentFreqEnum_(NONE_FREQUENCY),
        notionalExchangeEnum_(NONE_NE), 
		leverage_(1),
		couponMultiplier_(1),
		accrualStartDate_(""),
        accrualEndDateOrTenor_(""),		
		accrualbusinessDayAdj_(NONE_BUSINESS_DAY_ADJ),		
        accrualCalendar_(""),			
        accrualFrequency_(NONE_FREQUENCY),			
        accrualDaycount_(NONE_DAYCOUNT),			
		paymentbusinessDayAdj_(NONE_BUSINESS_DAY_ADJ),		
        paymentCalendar_(""),			
    	paymentFrequency_(NONE_FREQUENCY),			
		payLag_(""),						
		rollDayInput_(""),				
		stubType_(NONE_STUBTYPE),
	    firstStub_(""),					
        lastStub_(""),					
        fixingAdvanceOrArrears_(""),
		fixingbusinessDayAdj_(NONE_BUSINESS_DAY_ADJ),		
        fixingCalendar_(""),			
        fixLag_(""),
        firstFixLag_(""),
   		fixingDates_(DateVector()),
		fixingEndDates_(DateVector()),
		accrualStartDates_(DateVector()),
		accrualEndDates_(DateVector()),
		paymentDates_(DateVector()),
		accrualYearFractions_(DoubleVector()),
		accrualDays_(IntVector()),
		inputParameters_(LabelValueBlock()),
        upfrontCashflow_(),
        fxFixingLag_(""),
        fxFixingbusinessDayAdj_(NONE_BUSINESS_DAY_ADJ),
        fxFixingCalendar_(""),
        isIrregularStub_(false),
		effectiveDate_(LADate()),
		unadjustedMaturityDate_(LADate())
	{}

	Schedule::Schedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : IsLWOObject(instanceName, SCHEDULE)
	{
		const std::string inputLVB = "scheduleLVB";
		inputParameters_ = scheduleLVB;

		scheduleType_ = NONE_SCHEDULE_TYPE;
		bespokeScheduleType_ = NONE_BESPOKE_SCHEDULE;

        //Since ScheduleType field is provided by the user, then 
        inputParameters_ = LabelValueBlock( inputParameters_, IRS_KEY::SCHEDULE_TYPE, toString(scheduleType_) );

		payerReceiver_ = toPayReceiveEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::PAY_RECEIVE, IRS_KEY::PAYER_RECEIVER, "").getCString());
		notional_= scheduleLVB.getOptionalValueAsDouble(IRS_KEY::NOTIONAL, std::numeric_limits<double>::quiet_NaN());

        amortization_ = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::AMORTIZATION);
        amortFrequency_ = scheduleLVB.getOptionalValueAsUnsignedInt(IRS_KEY::AMORT_FREQUENCY);

        futureValueNotional_ = std::numeric_limits<double>::quiet_NaN();
        paymentFreqEnum_ = NONE_FREQUENCY;

		notionalExchangeEnum_	= toNotionalExchangeEnum(scheduleLVB.getOptionalValueAsLAString(IRS_KEY::NOTIONAL_EXCHANGE, "NONE").getCString());
		leverage_ = 1.;
		couponMultiplier_ = 1.;
		
		accrualEndDateOrTenor_ = scheduleLVB.getCompulsoryValueAsLAString( IRS_KEY::MATURITY_DATE,  inputLVB );

		accrualbusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getCompulsoryValueAsLAString( IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT, inputLVB ).getCString());
        accrualCalendar_ = scheduleLVB.getCompulsoryValueAsLAString( IRS_KEY::ACCRUALCALENDAR, inputLVB );
        accrualFrequency_ = toFrequencyEnum(scheduleLVB.getCompulsoryValueAsLAString( IRS_KEY::ACCRUALFREQUENCY, inputLVB ).getCString());
        accrualDaycount_ = toDayCountEnum(scheduleLVB.getCompulsoryValueAsLAString( IRS_KEY::ACCRUALDAYCOUNT, inputLVB ).getCString());

		paymentbusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getCompulsoryValueAsLAString( IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT ).getCString());
        paymentCalendar_ = scheduleLVB.getCompulsoryValueAsLAString( IRS_KEY::PAYMENTCALENDAR );
    	paymentFrequency_ = toFrequencyEnum(scheduleLVB.getOptionalValueAsLAString( IRS_KEY::PAYMENTFREQUENCY, toString(accrualFrequency_).c_str()).getCString());

		payLag_	= scheduleLVB.getOptionalValueAsLAString( IRS_KEY::PAYMENTLAG, "0D" );
        
		rollDayInput_ = scheduleLVB.getOptionalValueAsLAString( IRS_KEY::ROLLDAY );
		stubType_ = toStubTypeEnum(scheduleLVB.getOptionalValueAsLAString( IRS_KEY::STUBTYPE  ).getCString());
	    firstStub_ = scheduleLVB.getOptionalValueAsLAString( IRS_KEY::FIRSTSTUBDATE );
        lastStub_ = scheduleLVB.getOptionalValueAsLAString( IRS_KEY::LASTSTUBDATE  );
        fixingAdvanceOrArrears_	= scheduleLVB.getOptionalValueAsLAString( IRS_KEY::FIXINGADVANCEORARREAR, "advance" );

		//fixing
		fixingbusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAString( IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT).getCString());
        fixingCalendar_ = scheduleLVB.getOptionalValueAsLAString( IRS_KEY::FIXINGCALENDAR);
        fixLag_ = scheduleLVB.getOptionalValueAsLAString( IRS_KEY::FIXINGLAG, "0D" );
        firstFixLag_ = scheduleLVB.getOptionalValueAsLAString( IRS_KEY::FIRSTFIXINGLAG, fixLag_);

        //fxFixing
        fxFixingLag_              = scheduleLVB.getOptionalValueAsLAString( IRS_KEY::FXFIXINGLAG, "0D" );
		fxFixingbusinessDayAdj_   = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAString( IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT, toString(accrualbusinessDayAdj_).c_str()).getCString());
        fxFixingCalendar_         = scheduleLVB.getOptionalValueAsLAString( IRS_KEY::FXFIXINGCALENDAR, accrualCalendar_ );

        populateAccrualStartDates(scheduleLVB);

        //Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates, fixingDates
		calculateScheduleDates();

        //Populate the cashflows based on schedule inputs
        createCashflows(); 

        // Determine if the Stubs are Irregular Stub
        determineIsIrregularStub();

	}

    void Schedule::populateAccrualStartDates(const LabelValueBlock& scheduleLVB)
    {
		const std::string inputLVB = "scheduleLVB";
		accrualStartDate_ = scheduleLVB.getCompulsoryValueAsLAString(IRS_KEY::EFFECTIVE_DATE, inputLVB);
	}

    FrequencyEnum Schedule::getPaymentFreqEnum() const
    {
        return paymentFreqEnum_;
    }

    bool Schedule::isPaymentFreqEnumAtMaturity() const
    {
        return paymentFreqEnum_ == AT_MATURITY_FREQUENCY;
    }

	void Schedule::populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB)
    {
		// The leg can pay AT_MATURITY (in this case the coupon calculation follows the zero coupon swap logic), or pay at other frequencey (in this case the coupon calculation follows normal logic).
        paymentFreqEnum_ = paymentFrequency_;

        //Set paymentFreq: If the paymentFreq is 'AT_MATURITY', change it to be the same as accrual frequency, because the 'AT MATURITY' frequency will not impact the schedule, it only impact the coupon calculation
        if (isPaymentFreqEnumAtMaturity())
        {
            paymentFrequency_ =  accrualFrequency_;
        }

	}

    void Schedule::createUpfrontCashflow(const LADate& paymentDate, double leverage)
    {
    	throw LACoreInvalidData( "#Error: createUpfrontCashflow method not supported for this leg or product", __FILE__, __LINE__ );
    }

    void Schedule::createCashflows() 
    {
        auto cashflowSize = accrualStartDates_.size();

        std::vector<double> cashflowNotionals = getCashflowNotionals(cashflowSize); 

        for (size_t i=0; i < accrualStartDates_.size(); ++i)
		{
			auto cashflowType = (i == cashflowSize - 1) ? NORMAL_LAST_CASHFLOW_TYPE : NORMAL_CASHFLOW_TYPE;

            double cashflowNotional = cashflowNotionals[i];

			CashFlowBespokeInfo bespokeInfo;
			bespokeInfo.accrualDaycount = accrualDaycount_;
			bespokeInfo.forecastCurve = "";

			CashflowPtr cf = CashflowPtr(new Cashflow(payerReceiver_, getFixingDate(i), accrualStartDates_[i], accrualEndDates_[i], accrualDays_[i], accrualYearFractions_[i], paymentDates_[i], cashflowNotional, leverage_, couponMultiplier_, paymentFreqEnum_, cashflowType, bespokeInfo));
			cashflows_.push_back(cf);
		}
    }

	// cashflowLVBs will be updated by bespokeScheduleProperties
    Schedule::Schedule(const std::string& instanceName, const LabelValueBlock& bespokeScheduleProperties, const std::vector<LabelValueBlock>& cashflowLVBs, const BespokeScheduleTypeEnum& bespokeScheduleType)
        : IsLWOObject(instanceName, SCHEDULE),
		bespokeScheduleType_(bespokeScheduleType),
        accrualStartDate_(""),			
        accrualEndDateOrTenor_(""),		
		payLag_(""),						
	    firstStub_(""),					
        lastStub_(""),					
        fixLag_(""),
		firstFixLag_(""),
        upfrontCashflow_(),
        inputParameters_(bespokeScheduleProperties),
        amortization_(0), 
        amortFrequency_(0)
    {

  		const std::string inputLVB = "bespokeScheduleProperties";

		MLIB_REQUIRE(bespokeScheduleProperties.size() > 0, "bespokeScheduleProperties cannot be empty.");
		MLIB_REQUIRE(cashflowLVBs.size() > 0, "Cashflows cannot be empty.");

		switch (bespokeScheduleType)
		{
		case BESPOKE_SCHEDULE_WITH_PROPERTIES:
		{
			accrualDaycount_ = toDayCountEnum(bespokeScheduleProperties.getCompulsoryValueAsLAString(IRS_KEY::ACCRUALDAYCOUNT, inputLVB).getCString());
			stubType_ = toStubTypeEnum(bespokeScheduleProperties.getOptionalValueAsLAString(IRS_KEY::STUBTYPE).getCString());
			fixingAdvanceOrArrears_ = bespokeScheduleProperties.getOptionalValueAsLAString(IRS_KEY::FIXINGADVANCEORARREAR, "advance");

			accrualbusinessDayAdj_ = toBusinessDayAdjustmentEnum(bespokeScheduleProperties.getCompulsoryValueAsLAString(IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT).getCString());
			accrualCalendar_ = bespokeScheduleProperties.getCompulsoryValueAsLAString(IRS_KEY::ACCRUALCALENDAR);

			// 1) These fields are required to check if the schedule is regular or not: accrualFrequency_, accrualbusinessDayAdj_, accrualCalendar_, rollDayInput_
			// 2) These fields are mandatory fields for OIS Swap with bespoke schedule: accrualbusinessDayAdj_, accrualCalendar_, rollDayInput_
			accrualFrequency_ = toFrequencyEnum(bespokeScheduleProperties.getCompulsoryValueAsLAString(IRS_KEY::ACCRUALFREQUENCY, inputLVB).getCString());

			rollDayInput_ = bespokeScheduleProperties.getOptionalValueAsLAString(IRS_KEY::ROLLDAY);

			paymentFrequency_ = toFrequencyEnum(bespokeScheduleProperties.getOptionalValueAsLAString(IRS_KEY::PAYMENTFREQUENCY, toString(accrualFrequency_).c_str()).getCString());
			paymentbusinessDayAdj_ = accrualbusinessDayAdj_;
			paymentCalendar_ = accrualCalendar_;

			break;
		}
		case BESPOKE_SCHEDULE:
		{
			accrualDaycount_		= NONE_DAYCOUNT;
			stubType_				= NONE_STUBTYPE;
			fixingAdvanceOrArrears_ = "";

			accrualbusinessDayAdj_	= NONE_BUSINESS_DAY_ADJ;
			accrualCalendar_		= "";
			accrualFrequency_		= NONE_FREQUENCY;

			rollDayInput_			= "";

			paymentbusinessDayAdj_	= NONE_BUSINESS_DAY_ADJ;
			paymentCalendar_		= "";
			paymentFrequency_		= NONE_FREQUENCY;

			break;
		}
		default:
			throw ETradingException("#Error: bespokeScheduleType value invalid, must be 'BESPOKE_SCHEDULE_WITH_PROPERTIES' or 'BESPOKE_SCHEDULE'.");
			break;
		}


		// These are mandatory fields to populate a bespoke schedule:
		payerReceiver_ = toPayReceiveEnum(bespokeScheduleProperties.getCompulsoryValueAsLAStringFromKeys(IRS_KEY::PAY_RECEIVE, IRS_KEY::PAYER_RECEIVER, inputLVB).getCString());
		notionalExchangeEnum_ = toNotionalExchangeEnum(bespokeScheduleProperties.getOptionalValueAsLAString(IRS_KEY::NOTIONAL_EXCHANGE, "NONE").getCString());

		scheduleType_ = toScheduleTypeEnum(bespokeScheduleProperties.getCompulsoryValueAsString(IRS_KEY::SCHEDULE_TYPE, inputLVB));

		auto defaultFxFixingbusinessDayAdj = accrualbusinessDayAdj_;
		auto defaultFxFixingCalendar = accrualCalendar_;

		if (isFloatLeg(scheduleType_))
		{
			// fixingbusinessDayAdj_ is required to get the floatRates
			auto fixingbusinessDayAdjStr = bespokeScheduleProperties.getOptionalValueAsString(IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT, toString(accrualbusinessDayAdj_));
			MLIB_REQUIRE(!fixingbusinessDayAdjStr.empty(), "FixingBusinessDayAdjustment cannot be empty.");
			fixingbusinessDayAdj_ = toBusinessDayAdjustmentEnum(fixingbusinessDayAdjStr);

			fixingCalendar_ = bespokeScheduleProperties.getOptionalValueAsLAString(IRS_KEY::FIXINGCALENDAR, accrualCalendar_.getCString());
			MLIB_REQUIRE(fixingCalendar_.size() > 0, "FixingCalendar cannot be empty.");

			defaultFxFixingbusinessDayAdj = fixingbusinessDayAdj_;
			defaultFxFixingCalendar = fixingCalendar_;
		}

		// These optional fields are for xccy swap
		fxFixingbusinessDayAdj_ = toBusinessDayAdjustmentEnum(bespokeScheduleProperties.getOptionalValueAsLAString(IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT, toString(defaultFxFixingbusinessDayAdj).c_str()).getCString());
		fxFixingCalendar_ = bespokeScheduleProperties.getOptionalValueAsLAString(IRS_KEY::FXFIXINGCALENDAR, defaultFxFixingCalendar);
		fxFixingLag_ = bespokeScheduleProperties.getOptionalValueAsLAString(IRS_KEY::FXFIXINGLAG, "0D");

		// These optional fields are for Zero Coupon Swap
        futureValueNotional_        = bespokeScheduleProperties.getOptionalValueAsDouble(IRS_KEY::FV_NOTIONAL, std::numeric_limits<double>::quiet_NaN()); 			

		//Get first notional as the schedule initial notional, so that the notional exchange can have the correct value
		notional_ = cashflowLVBs.at(0).getOptionalValueAsDouble(IRS_KEY::NOTIONAL, std::numeric_limits<double>::quiet_NaN());
		MLIB_REQUIRE(!boost::math::isnan(notional_), "Notional cannot be empty.");

		populateNotionalAndPaymentFreqEnum();

	}

    SchedulePtr Schedule::clone()
    {
        SchedulePtr sch = SchedulePtr(new Schedule(*this));
        return sch;
    }

	/*
	 * Some subtlety in the Schedule copy constructor: We require the LAString member variables to deep copy (rather than shallow copy
	 * with an incremented reference count). We require this so that we can use the new Schedule instance on a different thread,
	 * without risking concurrent access to the LAString reference count. Hence we copy the LAStrings by invoking getCString()
	 * on the rhs instance, forcing the LAString to deep copy from a char*.
	 */
	Schedule::Schedule(const Schedule& rhs) : IsLWOObject(rhs.getRefToName().c_str(), SCHEDULE), 
		scheduleType_(rhs.scheduleType_),
		bespokeScheduleType_(rhs.bespokeScheduleType_),
		payerReceiver_(rhs.payerReceiver_),
		notional_(rhs.notional_), 
        amortization_(rhs.amortization_), 
        amortFrequency_(rhs.amortFrequency_), 
		futureValueNotional_(rhs.futureValueNotional_), 
		paymentFreqEnum_ (rhs.paymentFreqEnum_ ), 
        notionalExchangeEnum_(rhs.notionalExchangeEnum_), 
        leverage_(rhs.leverage_),
		couponMultiplier_(rhs.couponMultiplier_),
		accrualStartDate_(rhs.accrualStartDate_.getCString()),			
        accrualEndDateOrTenor_(rhs.accrualEndDateOrTenor_.getCString()),		
		accrualbusinessDayAdj_(rhs.accrualbusinessDayAdj_),		
        accrualCalendar_(rhs.accrualCalendar_.getCString()),			
        accrualFrequency_(rhs.accrualFrequency_),			
        accrualDaycount_(rhs.accrualDaycount_),			
		paymentbusinessDayAdj_(rhs.paymentbusinessDayAdj_),		
        paymentCalendar_(rhs.paymentCalendar_.getCString()),			
    	paymentFrequency_(rhs.paymentFrequency_),			
		payLag_(rhs.payLag_.getCString()),						
		rollDayInput_(rhs.rollDayInput_.getCString()),				
		stubType_(rhs.stubType_),					
	    firstStub_(rhs.firstStub_.getCString()),					
        lastStub_(rhs.lastStub_.getCString()),					
        fixingAdvanceOrArrears_(rhs.fixingAdvanceOrArrears_.getCString()),
		fixingbusinessDayAdj_(rhs.fixingbusinessDayAdj_),		
        fixingCalendar_(rhs.fixingCalendar_.getCString()),			
        fixLag_(rhs.fixLag_.getCString()),	
        firstFixLag_(rhs.firstFixLag_.getCString()),	
		fixingDates_(rhs.fixingDates_),
		fixingEndDates_(rhs.fixingEndDates_),
		accrualStartDates_(rhs.accrualStartDates_),
		accrualEndDates_(rhs.accrualEndDates_),
		paymentDates_(rhs.paymentDates_),
		accrualYearFractions_(rhs.accrualYearFractions_),
		accrualDays_(rhs.accrualDays_),
		inputParameters_(rhs.inputParameters_),
        fxFixingLag_(rhs.fxFixingLag_),	        
        fxFixingbusinessDayAdj_(rhs.fxFixingbusinessDayAdj_),
        fxFixingCalendar_(rhs.fxFixingCalendar_.getCString()),
        isIrregularStub_(rhs.isIrregularStub_),
		effectiveDate_(rhs.effectiveDate_),
		unadjustedMaturityDate_(rhs.unadjustedMaturityDate_)
	{
        
        upfrontCashflow_ = (rhs.upfrontCashflow_ != nullptr) ? rhs.upfrontCashflow_->clone() : CashflowPtr();

        for(size_t i =0 ; i < rhs.cashflows_.size(); ++i)
		{
            auto cf = rhs.cashflows_[i];
			if(cf != nullptr)
			{
				cashflows_.emplace_back(cf->clone()); 
			}
		}
	}

	//return all accrualDates with accrualStart and accrualEnd
	DateVector Schedule::generateAccrualAndPaymentSchedules()
	{
		//Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates 
		DateVector accrualDates;
		validateAndGenerateAccrualAndPaymentSchedules(accrualDates,
													paymentDates_,
													accrualStartDate_,
													accrualEndDateOrTenor_,
													toString(accrualFrequency_).c_str(),
													toString(accrualbusinessDayAdj_).c_str(),
													accrualCalendar_,
													toString(paymentFrequency_).c_str(),
													toString(paymentbusinessDayAdj_).c_str(),
													paymentCalendar_,
													payLag_,
													toString(stubType_).c_str(),
													firstStub_,
													lastStub_,
													rollDayInput_,
													fixingAdvanceOrArrears_);

		validateAndGenerateAccrualStartAndEndDates(accrualStartDates_, accrualEndDates_, accrualDates);

		return accrualDates;
	}

	void Schedule::calculateScheduleDates() 
	{
		effectiveDate_ = etrading::stringToDate(accrualStartDate_, "#Error: Invalid 'EffectiveDate'.");
		unadjustedMaturityDate_ = validateMaturityDate(getEffectiveDate(), accrualEndDateOrTenor_);

		//Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates 
		DateVector accrualDates = generateAccrualAndPaymentSchedules();

		size_t expectedSize = accrualDates.size() - 1;

        for (size_t i=0; i < expectedSize; ++i)
		{
			LADate accrualStart = accrualStartDates_[i];
			LADate accrualEnd = accrualEndDates_[i];

            double bondActualYearFraction = std::numeric_limits<double>::quiet_NaN();

            // Default: Year Fraction for SWAPS

            // Important Note: If compound interest is being applied then we accrue interest to the
			// payment date ( with no payment lag ) and not the accrual end date.
			bool INCLUDE_LAST_PAYMENT_DATE = false;		// i.e. do not include last payment date
			double tmpYearFraction = getYearFraction( accrualStart, accrualEnd, accrualDaycount_, INCLUDE_LAST_PAYMENT_DATE );

			accrualYearFractions_.push_back(tmpYearFraction);

			// Days between accrual start and end dates, not business days
			int days  = getDays(accrualStart, accrualEnd);
			accrualDays_.push_back(days);
		}

		if (fixingbusinessDayAdj_ != NONE_BUSINESS_DAY_ADJ && fixingCalendar_.size() != 0)
		{
			// 1) If firstFixLag is the not specified, or firstFixLag is the same as fixLag, use the fixLag to calculate fixingDates
            if (firstFixLag_.size() == 0 || fixLag_.toUpper() == firstFixLag_.toUpper()) 
            {
                fixingDates_ = validateAndGenerateFixingSchedule( accrualDates,
															      toString(fixingbusinessDayAdj_).c_str(),
															      fixingCalendar_,
															      fixLag_,
															      fixingAdvanceOrArrears_ );
            }
			// 2) If the firstFixLag is specified, use the firstFixLag to calculate the first cashflow's fixingDate, and use fixLag to calculate the rest cashflows
	        else
            {

                // First cashflow: accrualDates come in pairs ie a start date and an end date
                if ( accrualDates.size() >= 2 )
                {

                    //fixingDate of the first cashflow, based on the firstFixingLag
                    fixingDates_ = validateAndGenerateFixingSchedule( boost::assign::list_of( accrualDates.at(0))(accrualDates.at(1) ),
															          toString(fixingbusinessDayAdj_).c_str(),
															          fixingCalendar_,
															          firstFixLag_,
															          fixingAdvanceOrArrears_ );

                    //excluding the first cashflow's accrualStartDate & accrualEndDate
                    accrualDates.erase( accrualDates.begin(), accrualDates.begin() + 1 );
                }

                // For the rest cashflows: accrualDates come in pairs ie a start date and an end date
                if ( accrualDates.size() >= 2 )
                {
                    //fixingDates for the remaining cashflows
                    auto fixingDates2 = validateAndGenerateFixingSchedule( accrualDates,
															               toString(fixingbusinessDayAdj_).c_str(),
															               fixingCalendar_,
															               fixLag_,
															               fixingAdvanceOrArrears_);

                    fixingDates_.insert(fixingDates_.end(), fixingDates2.begin(), fixingDates2.end());
                }
    
            }
			
			fixingEndDates_ = LADateScheduleHelpers::getMultiDate( fixingDates_, getFrequencyTenor( getAccrualFrequency() ), toString( getFixingBusinessDayAdj() ).c_str(), getFixingCalendar(), nullptr); // rollconvention* = nullptr

    	}
	}

    DateVector Schedule::calculateFxFixingDates() 
    {
        DateVector fxFixingDates;
        fxFixingDates.reserve(accrualStartDates_.size());

        DateVector accrualDates = accrualStartDates_;
        accrualDates.push_back(accrualStartDates_.back());

        //*** In MLib Xccy curve builindg, fxFixingDate(i) is the same as paymentDate(i-1), i.e. same as accrualStartDate(i)
       fxFixingDates = validateAndGenerateFixingSchedule(   accrualDates,
															toString(fxFixingbusinessDayAdj_).c_str(),
															fxFixingCalendar_,
															fxFixingLag_,
															"advance" ); //We assume the fxFixing is always in advance

    
        return fxFixingDates;
    }

	/*  @brief		This is for calculation of OIS float rates.
					This function calculates fixingEndDates specifically for OIS Swaps.
					The OIS fixingEndDate is used for the OIS average rate calculation,
					which is different from the normal Libor fixingEndDate.
					It is calculated by applying a fixingLag to accrualEndDate.
		@returns	A vector of fixing end dates
	*/
    DateVector Schedule::calculateAndPopulateOISFixingEndDates()
    {
        DateVector fixingEndDates;
        fixingEndDates.reserve(accrualEndDates_.size());

        DateVector accrualDates = accrualEndDates_;
        accrualDates.push_back(accrualEndDates_.back());

        if (!etrading::LACurveForwardRateHelpers::isFixingInAdvance(fixingAdvanceOrArrears_))
        {
           throw LACoreInvalidData( "#Error: For OIS Swap, fixing in arrears is not supported.", __FILE__, __LINE__ );
        }

        fixingEndDates = validateAndGenerateFixingSchedule( accrualDates,
															toString(fixingbusinessDayAdj_).c_str(),
															fixingCalendar_,
															fixLag_,
															"advance" ); 

        // Update cashflows with the fixingEndDates
        for (size_t i=0; i < cashflows_.size(); ++i)
        {
            auto cashflow = cashflows_[i];
            auto fixingEndDt = fixingEndDates[i];
            cashflow->setFixingEndDate(fixingEndDt);
        }

        return fixingEndDates;

    }

	void Schedule::createBespokeCashflows(const std::vector<LabelValueBlock>& cashflowLVBs) 
	{
    	throw LACoreInvalidData( "#Error: createBespokeCashflows method not supported for this leg or product", __FILE__, __LINE__ );
	}

	double Schedule::getFinalCashflowNotionalExchange(const double& notional) const
	{
		return notional;
	}

    void Schedule::updateNotionalExchange()
    {
        //first cashflow
        if (upfrontCashflow_ != nullptr)
        {
			// notionalExchange (upfront)  = - notional(upfront) 
	         auto notionalExchange = getNotional() * (-1);
            upfrontCashflow_->setNotionalExchange(notionalExchange);
        } 

        if ( getNotionalExchangeEnum()  == NONE_NE) 
        {
           return;
        }

        size_t cashflowSize = getCashflowSize();
        bool isLastCashflowNotionalExchange = getNotionalExchangeEnum() == END_NE || getNotionalExchangeEnum() == START_AND_END_NE;
        for(size_t i=0; i < cashflowSize; ++i)
		{
            auto cf = getCashflow(i);
            auto notional = cf->getNotional();


            //final cashflow
            bool isFinalCashflow = (i == cashflowSize - 1);
            if (isFinalCashflow)
            {
                if (isLastCashflowNotionalExchange)
                {
					// notionalExchange (last)  = notional(last)
					const double notionalToUse = getFinalCashflowNotionalExchange(notional);
	                cf->setNotionalExchange(notionalToUse);
                }
                else
                {
                    cf->setNotionalExchange(0);
                }
            } 
            else
            {
				// notionalExchange(i)  = notional(i) * leverage(i) - notional(i+1) * leverage(i+1)
				auto nextCashflow = getCashflow( i + 1 );
                auto nextNotional = nextCashflow->getNotional();
                auto notionalExchange = notional - nextNotional;
                cf->setNotionalExchange(notionalExchange);
            }
		}
    }
	

	
	void Schedule::initializeDataProviderWithCurveData( DataProvider& dataProvider, const LAString& discountCurve, const std::vector<FloatRateData>& floatRates ) const
	{
		//Get DFs from the discount curve
		auto paymentDates = getPaymentDates();

		//Handle upfront cashflow
		if (getUpfrontCashflow() != nullptr)
		{
			//add it to the first element
			paymentDates.insert(paymentDates.begin(), getUpfrontCashflow()->getPaymentDate());
		}

		const LADate valuationDate = dataProvider.getValuationSettings().getValuationDate();
		size_t firstNonpastPaymentDateIndex = getFirstNonpastDateIndex(paymentDates, valuationDate);
		const LAString curveCollection = dataProvider.getValuationSettings().getCurveCollection().c_str();
		auto discountFactors = getCurveDiscountFactorsFromBaseDate(firstNonpastPaymentDateIndex, valuationDate, paymentDates, curveCollection, discountCurve);

		//Handle upfront cashflow
        if (getUpfrontCashflow() != nullptr)
		{
            //get it from the first element
            auto df = discountFactors.front();

            //Set DF for the upfrontCashflow
			dataProvider.setUpfrontDiscountFactor( df );

            //DFs excluding the first one that belongs to upfrontCashflow
            discountFactors.erase(discountFactors.begin());
    	}

        //Set floatRates and DFs for the non-upfront cashflows

        size_t cashflowSize = getCashflowSize();
        if (cashflowSize != discountFactors.size())
		{
			throw LACoreInvalidData( "#Error: DiscountFactors and Cashflows should have the same size", __FILE__, __LINE__ );
		}

        bool hasFloatRates = (floatRates.size() != 0);

        if (hasFloatRates && cashflowSize != floatRates.size())
		{
			throw LACoreInvalidData( "#Error: FloatRates and Cashflows should have the same size", __FILE__, __LINE__ );
		}

		dataProvider.setDiscountFactors( discountFactors );
		if ( hasFloatRates )
		{
			dataProvider.setFloatRates( floatRates );
		}
    }

	bool Schedule::hasAccruedInterest(const LADate& valuationDate) const
	{
		// If settleDate is specified and later than effectiveDate, then accruedInterest is included
		if (valuationDate > getEffectiveDate())
		{
			return true;
		}
		else
		{
			return false;
		}

	}

	double Schedule::calculateAccruedInterest( const DataProvider& dataProvider, bool nativeCurrencyPV ) const
    {
		double accruedInterest = 0.0;

		const LADate settleDate = dataProvider.getValuationSettings().getValuationDate();

		if (!hasAccruedInterest(settleDate)) 
		{
			// No accrued interest
			return accruedInterest;
		}

		// cashflows_ excludes the notional exchange upfront cashflow
		size_t cashflowSize = cashflows_.size();
		size_t firstNonpastPaymentDateIndex = getFirstNonpastDateIndex(paymentDates_, settleDate);

		for(size_t i = firstNonpastPaymentDateIndex; i < cashflowSize; ++i )
		{
			auto cashflow = cashflows_[i];
			CashflowData cashflowData = dataProvider.getCashflowDataExcludingUpfront( i );
			auto accrualStartDate = cashflow->getAccrualStartDate();

			if (accrualStartDate >= settleDate) 

			{
				// No accrued interest
				break;
			}

		    // Calculate accrued interest
			auto accrualEndDate = cashflow->getAccrualEndDate();
			auto paymentDate = cashflow->getPaymentDate();
			// include the whole coupon (where the coupon excludes notional exchange)
			if (accrualEndDate < settleDate)
			{
				//Notional Exchange already include leverage
				auto couponWithoutNotionalExchange = cashflow->getCoupon( cashflowData ) - cashflow->getNotionalExchange();

				accruedInterest += couponWithoutNotionalExchange;
			}
			// include a ratio of coupon (where the coupon excludes notional exchange)
			else 
			{
				//Notional Exchange already include leverage
				auto couponWithoutNotionalExchange = cashflow->getCoupon( cashflowData ) - cashflow->getNotionalExchange();

				// ratio = (settleDate - accrualStartDate)/ (accrualEndDate - accrualStart)

				bool DONT_INCLUDE_LAST_PAYMENT_DATE = false;
				double accruedPeriodYearFraction = getYearFraction(accrualStartDate, settleDate, accrualDaycount_, DONT_INCLUDE_LAST_PAYMENT_DATE);
				double fullPeriodYearFraction = getYearFraction(accrualStartDate, accrualEndDate, accrualDaycount_, DONT_INCLUDE_LAST_PAYMENT_DATE);

				double ratio = accruedPeriodYearFraction / fullPeriodYearFraction;

				accruedInterest += couponWithoutNotionalExchange * ratio;
			}
		}
		
		// Handle XCCY:
		if ( ! nativeCurrencyPV && dataProvider.getCurrency() != dataProvider.getValuationCurrency() )
        {
            accruedInterest *= dataProvider.getValuationSettings().getFXAsOfDateRate();
        }
			
		return accruedInterest;

	}


    LADate Schedule::getEffectiveDate() const
    {
		// For bespoke schedule, the accrualStartDate_ is not provided
		if (accrualStartDate_.size() == 0)
		{
			MLIB_REQUIRE(cashflows_.size() > 0, "Must have at least one cashflow.");
			LADate effectiveDt = cashflows_.front()->getAccrualStartDate();

			return effectiveDt;
		}

		return effectiveDate_;
    }

    
	LADate Schedule::getMaturityDate() const
    {
		LADate maturityDt = cashflows_.back()->getAccrualEndDate();

		return maturityDt;
    }

	LADate Schedule::getUnadjustedMaturityDate() const
    {
		// For bespoke schedule, the accrualEndDateOrTenor_ is not provided
		if (accrualEndDateOrTenor_.size() == 0)
		{
			MLIB_REQUIRE(cashflows_.size() > 0, "Must have at least one cashflow.");
			const LADate maturityDt = cashflows_.back()->getAccrualEndDate();

			return maturityDt;
		}

		return unadjustedMaturityDate_;
    }

	bool Schedule::isBespoke() const
	{
		return (bespokeScheduleType_ != NONE_BESPOKE_SCHEDULE);
	}

	LADate Schedule::getFixingDate(size_t i) const
	{
		LADate fixingDate;
		if (fixingDates_.size() > 0)
		{
			fixingDate = fixingDates_[i];
		}
		else
		{
			fixingDate = LADate();
		}
		return fixingDate;
	}

	LabelValueBlock Schedule::getInputParameters() const
	{
		return inputParameters_;
	}

	void Schedule::setInputParameters(const LabelValueBlock& inputParameters)
	{
		inputParameters_ = inputParameters;
	}

	double Schedule::getNotional() const
	{
		if (cashflows_.size() == 0)
		{
			throw LACoreInvalidData( "#Error: Cashflow has not been populated", __FILE__, __LINE__ );
		}
		return cashflows_[0]->getNotional();
	}

	double Schedule::getFutureValueNotional() const
	{
		return futureValueNotional_;
	}

    void Schedule::setNotional(double notional)
    {
        for( size_t i = 0; i < cashflows_.size(); i++ )
	    {			
		    auto cf = cashflows_[i];
		    cf->setNotional(notional);
        }

        //Need to update the notional exchanges when the Notional is changed.
        updateNotionalExchange();

    }

    NotionalExchangeEnum Schedule::getNotionalExchangeEnum() const
    {
        return notionalExchangeEnum_;
    }

    void Schedule::flipPayerReceiver()
    {
        payerReceiver_ = flipPayReceive(payerReceiver_);
        
        // Create New LVB with updated pay receive flag
        inputParameters_ = LabelValueBlock( inputParameters_, IRS_KEY::PAY_RECEIVE, toString(payerReceiver_) );

        if (upfrontCashflow_ != nullptr)
        {
            upfrontCashflow_->flipPayerReceiver();
        }
        for( size_t i = 0; i < cashflows_.size(); i++ )
	    {			
		    auto cf = cashflows_[i];
		    cf->flipPayerReceiver();
        }
    }

	PayReceiveEnum Schedule::getPayerReceiver() const
	{
		return payerReceiver_;
    }

   	int Schedule::getPayRecIndicator() const
	{
		return getIndicatorFromPayRec(payerReceiver_);
	}

	double Schedule::getLeverage() const
	{
		if (cashflows_.size() == 0)
		{
			throw LACoreInvalidData( "#Error: Cashflow has not been populated", __FILE__, __LINE__ );
		}
		return cashflows_[0]->getLeverage();
	}

	double Schedule::getCouponMultiplier() const
	{
		if (cashflows_.size() == 0)
		{
			throw LACoreInvalidData("#Error: Cashflow has not been populated", __FILE__, __LINE__);
		}
		return cashflows_[0]->getCouponMultiplier();
	}

    ScheduleTypeEnum Schedule::getScheduleType() const
	{
		return scheduleType_;
	}

	BespokeScheduleTypeEnum Schedule::getBespokeScheduleType() const
	{
		return bespokeScheduleType_;
	}

	LAString Schedule::getRollDayInput() const
	{
		return rollDayInput_;
	}	

	LAString Schedule::getAccrualCalendar() const
	{
		return accrualCalendar_;
	}	

	LAString Schedule::getFxFixingCalendar() const
	{
		return fxFixingCalendar_;
	}

	BusinessDayAdjustmentEnum Schedule::getAccrualbusinessDayAdj() const
	{
		return accrualbusinessDayAdj_;
	}	

	BusinessDayAdjustmentEnum Schedule::getFxFixingBusinessDayAdj() const
	{
		return fxFixingbusinessDayAdj_;
	}

	DayCountEnum Schedule::getAccrualDaycount() const
	{
		return accrualDaycount_;
	}	

    FrequencyEnum Schedule::getAccrualFrequency() const
    {
		return accrualFrequency_;
	}

    FrequencyEnum Schedule::getPaymentFrequency() const
    {
		return paymentFrequency_;
	}

	StubTypeEnum Schedule::getStubType() const
    {
		StubTypeEnum stubType = stubType_;
        if( firstStub_.size() != 0 )
        {
            stubType = SHORT_START_STUBTYPE;	// When firstStubDate is specified, we know if we are dealing with front stub
        }
        else if( lastStub_.size() != 0 )
        {
            stubType = SHORT_END_STUBTYPE;	 //When lastStubDate is specified, we know if we are dealing with end stub
        }
		return stubType;
	}

  	LAString Schedule::getFixingCalendar() const
	{
		return fixingCalendar_;
	}

	LAString Schedule::getFixLag() const
	{
		return fixLag_;
	}
	

	BusinessDayAdjustmentEnum Schedule::getFixingBusinessDayAdj() const
	{
		return fixingbusinessDayAdj_;
	}	

	FixingTypeEnum Schedule::getFixingAdvanceOrArrears() const
	{
		return toFixingTypeEnum(fixingAdvanceOrArrears_.getCString());
	}

	LAString Schedule::getFirstStub() const
	{
		return firstStub_;
	}	

    LAString Schedule::getLastStub() const
	{
		return lastStub_;
	}	

    //------Schedule outputs------

	DateVector Schedule::getFixingDates() const
	{
		return fixingDates_;
	}

	DateVector Schedule::getFixingEndDates() const
	{
		return fixingEndDates_;
	}
	
	//Payment dates excluding upfront cashflow
	DateVector Schedule::getPaymentDates() const
	{
		return paymentDates_;
	}	

	//Payment dates including upfront cashflow
	DateVector Schedule::getAllPaymentDates() const
	{
		if (getUpfrontCashflow() != nullptr)
		{
			DateVector allPaymentDates = paymentDates_;
			allPaymentDates.insert(allPaymentDates.begin(), getUpfrontCashflow()->getPaymentDate());
			return allPaymentDates;
		}
		return paymentDates_;
	}

	DateVector Schedule::getAccrualStartDates() const
	{
		return accrualStartDates_;
	}	
	
	DateVector Schedule::getAccrualEndDates() const
	{
		return accrualEndDates_;
	}	

	DoubleVector Schedule::getAccrualYearFractions() const
	{
		return accrualYearFractions_;
	}	
	
	IntVector Schedule::getAccrualDays() const
	{
		return accrualDays_;
	}	

	CashflowPtr Schedule::getCashflow(size_t i) const
	{
		return cashflows_[i];
	}

	size_t Schedule::getCashflowSize() const
	{
		return cashflows_.size();
	}

    std::vector<CashflowPtr> Schedule::getAllCashflows() const
    {
        auto cashflows = cashflows_;
        if (getUpfrontCashflow() != nullptr)
        {
            cashflows.insert(cashflows.begin(), getUpfrontCashflow());
        }

		return cashflows;
    }

    std::vector<CashflowPtr> Schedule::getAllCashflowsExcludingUpfrontNotional() const
    {
        auto cashflows = cashflows_;
        return cashflows;
    }

    std::vector<CashflowPtr> Schedule::getAllCashflowsIncludingUpfrontNotional() const
    {
        // Call the getAllCashflows method
        return Schedule::getAllCashflows();
    }

    CashflowPtr Schedule::getUpfrontCashflow() const
    {
		return upfrontCashflow_;
	}

    bool Schedule::isAccrualFreqLessThanPaymentFreq() const
    {
		MLIB_REQUIRE( (accrualFrequency_ != NONE_FREQUENCY && paymentFrequency_ != NONE_FREQUENCY), "accrualFreq and paymentFreq cannot be empty.");

        auto smaller = getFrequencyNumber(accrualFrequency_) < getFrequencyNumber(paymentFrequency_);
        return smaller;
    }

    bool Schedule::isVariableNotional() const
    {    
        double epsilon = 1e-20;
        double firstNotional = cashflows_.at(0)->getNotional(); 

        for (size_t i = 1; i < cashflows_.size(); ++i)
        {
            auto cashflow = cashflows_[i];
            auto diff = firstNotional - cashflow->getNotional();
            if (std::abs(diff) > epsilon)
            {
                return true;
            }
        }

        return false;
    }


    bool Schedule::isIrregularStub() const
    {
        return isIrregularStub_;
    }


    bool Schedule::isFrontStub() const
    {
		StubTypeEnum stubType = getStubType();

        if (stubType == NONE_STUBTYPE || stubType == SHORT_START_STUBTYPE || stubType == LONG_START_STUBTYPE)
	    {
	        return true;
	    }
	    else if ( stubType == SHORT_END_STUBTYPE || stubType == LONG_END_STUBTYPE)
	    {
		    return false;
	    }
	    else
	    {
		    throw LACoreInvalidData("#Error: Stub Type must be None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).", __FILE__, __LINE__ );
	    }
    }

    double Schedule::getYearFractionFromEffectiveToMaturityDates() const
    {
        double totalYearFraction = 0;
        for (size_t i = 0; i < accrualYearFractions_.size(); ++i)
        {
            totalYearFraction += accrualYearFractions_[i];
        }
        return totalYearFraction;
    }

	
  	void Schedule::updateCashflowsCompoundingCoupons( const DataProvider& dataProvider ) 
    {
		//Zero coupon scenario:
		if (paymentFreqEnum_ == AT_MATURITY_FREQUENCY)
		{
			// cashflows_ excludes the upfront cashflow
			size_t cashflowSize = cashflows_.size();
			std::vector<CashFlowInfo> prevCashflowsInfo;
			prevCashflowsInfo.reserve(cashflowSize);
				
			for (size_t i = 1; i < cashflowSize; ++i)
			{
				const auto& curCashflow = cashflows_[i];
				size_t j = i - 1;
				const auto& previousCashflow = cashflows_[j];
				CashflowData cashflowData = dataProvider.getCashflowDataExcludingUpfront(j);

				CashFlowInfo cfInfo;
				cfInfo.coupon = previousCashflow->getRegularCoupon( cashflowData );
				prevCashflowsInfo.push_back(cfInfo);
				
				if (prevCashflowsInfo.size() > 0)
				{
					curCashflow->setPreviousCashflowsInfo(prevCashflowsInfo);
				}
			}
		}
    }

    std::vector<double> Schedule::getCashflowNotionals(const size_t cashflowSize) 
    {
        std::vector<double> cashflowNotionals(cashflowSize, notional_);

        //If the notional is amortizing
        if (amortization_ != 0 && amortFrequency_ != 0)
        {
            unsigned int count = 0;
            double cashflowNotional = notional_;

            for (size_t i=0; i < cashflowSize; ++i)
		    {
                if (count >= amortFrequency_)
                {
                    cashflowNotional = cashflowNotional - amortization_;
                    count = 0;
                }

                cashflowNotionals[i] = cashflowNotional;
                count++;

            }
		}
      
        return cashflowNotionals;
    }

    //dummy methods, just to avoid downcasting
    double Schedule::getFixedRate() const
    {
    	throw LACoreInvalidData( "#Error: getFixedRate not supported", __FILE__, __LINE__ );
    }

    double Schedule::getSpread() const
    {
    	throw LACoreInvalidData( "#Error: getSpread not supported", __FILE__, __LINE__ );
    }

   	bool Schedule::isVariableSpread() const 
    {
    	throw LACoreInvalidData( "#Error: isVariableSpread method for floating spreads not supported on a fixed leg", __FILE__, __LINE__ );
    }

    //end of dummy methods

	std::unordered_set<CashflowHeaderEnum, EnumClassHash> Schedule::allowedColumns() const
	{
		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
		{
			FIXING_DATE_HEADER
			,ACCRUAL_START_HEADER
			,ACCRUAL_END_HEADER
			,ACCRUAL_DAYS_HEADER
			,ACCRUAL_YEAR_FRACTIONS_HEADER
			,PAYMENT_DATE_HEADER
		};

		return expectedList;
	};


	// headers and bodys are outputs
	void Schedule::populateHeaderAndBody(AnyTypeVector& headers, std::vector<AnyTypeVector>& bodyBlock, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList, const bool convertDatesToExcelFormat ) const
	{
		auto allowColumns = allowedColumns();

		bool includeFixingDate = includeCashflowColumn(FIXING_DATE_HEADER, columnList, allowColumns);
		bool includeFixingEndDate = includeCashflowColumn(FIXING_END_DATE_HEADER, columnList, allowColumns);

		bool includeAccrualStart = includeCashflowColumn(ACCRUAL_START_HEADER, columnList, allowColumns);
		bool includeAccrualEnd = includeCashflowColumn(ACCRUAL_END_HEADER, columnList, allowColumns);
		bool includeAccrualDays = includeCashflowColumn(ACCRUAL_DAYS_HEADER, columnList, allowColumns);
		bool includeAccrualYearFraction = includeCashflowColumn(ACCRUAL_YEAR_FRACTIONS_HEADER, columnList, allowColumns);

		bool includePaymentDate = includeCashflowColumn(PAYMENT_DATE_HEADER, columnList, allowColumns);

		bool includeAmount = includeCashflowColumn(AMOUNT_HEADER, columnList, allowColumns);
		bool includePayReceive = includeCashflowColumn(PAY_RECEIVE_HEADER, columnList, allowColumns);

		bool includeNotional = includeCashflowColumn(NOTIONAL_HEADER, columnList, allowColumns);
		bool includeNotionalExchange = includeCashflowColumn(NOTIONAL_EXCHANGE_HEADER, columnList, allowColumns);
		bool includeLeverage = includeCashflowColumn(LEVERAGE_HEADER, columnList, allowColumns);

		//For Premium Schedule
		bool includeCdsSpread = includeCashflowColumn(CDS_SPREAD_HEADER, columnList, allowColumns);

		bool includeFixedRate = includeCashflowColumn(FIXED_RATE_HEADER, columnList, allowColumns);

		// For FRA
		bool includeSrikeRate = includeCashflowColumn(STRIKE_RATE_HEADER, columnList, allowColumns);

		bool includeSpread = includeCashflowColumn(FLOAT_SPREAD_HEADER, columnList, allowColumns);

		// Bespoked cashflow fields, only include when they are specified by the searchSet
		bool includeCouponMultiplier = includeCashflowColumn(COUPON_MULTIPLIER_HEADER, columnList, allowColumns, false);
		bool includeAccrualDaycount = includeCashflowColumn(ACCRUAL_DAYCOUNT_HEADER, columnList, allowColumns, false);
		bool includeForecastCurve = includeCashflowColumn(FORECAST_CURVE_HEADER, columnList, allowColumns, false);

		auto allCashflows = getAllCashflows();
		size_t cashflowSize = allCashflows.size();

		MLIB_REQUIRE(cashflowSize != 0, "Cashflows cannot be empty.");

		//1) Headers
		if (showColumnHeaders)
		{
			//Float Schedule
			MLIB_PUSH_BACK_IF(headers, toString(FIXING_DATE_HEADER), includeFixingDate);
			MLIB_PUSH_BACK_IF(headers, toString(FIXING_END_DATE_HEADER), includeFixingEndDate);

			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_START_HEADER), includeAccrualStart);
			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_END_HEADER), includeAccrualEnd);
			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_DAYS_HEADER), includeAccrualDays);
			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_YEAR_FRACTIONS_HEADER), includeAccrualYearFraction);
			MLIB_PUSH_BACK_IF(headers, toString(PAYMENT_DATE_HEADER), includePaymentDate);

			MLIB_PUSH_BACK_IF(headers, toString(AMOUNT_HEADER), includeAmount);
			MLIB_PUSH_BACK_IF(headers, toString(PAY_RECEIVE_HEADER), includePayReceive);

			MLIB_PUSH_BACK_IF(headers, toString(NOTIONAL_HEADER), includeNotional);
			
			// For FRA Schedule
			MLIB_PUSH_BACK_IF(headers, toString(STRIKE_RATE_HEADER), includeSrikeRate);

			MLIB_PUSH_BACK_IF(headers, toString(NOTIONAL_EXCHANGE_HEADER), includeNotionalExchange);
			MLIB_PUSH_BACK_IF(headers, toString(LEVERAGE_HEADER), includeLeverage);

			//For Premium Schedule
			MLIB_PUSH_BACK_IF(headers, toString(CDS_SPREAD_HEADER), includeCdsSpread);

			MLIB_PUSH_BACK_IF(headers, toString(FIXED_RATE_HEADER), includeFixedRate);

			MLIB_PUSH_BACK_IF(headers, toString(COUPON_MULTIPLIER_HEADER), includeCouponMultiplier);
			MLIB_PUSH_BACK_IF(headers, toString(FLOAT_SPREAD_HEADER), includeSpread);
			MLIB_PUSH_BACK_IF(headers, toString(ACCRUAL_DAYCOUNT_HEADER), includeAccrualDaycount);
			MLIB_PUSH_BACK_IF(headers, toString(FORECAST_CURVE_HEADER), includeForecastCurve);

		}

		const size_t headerColumnSize = headers.size();

		//2) Body
		for (size_t i = 0; i < cashflowSize; ++i)
		{
			AnyTypeVector body;
			if (headerColumnSize > 0)
			{
				body.reserve(headerColumnSize);
			}

			auto cashflow = allCashflows[i];

			MLIB_PUSH_BACK_DATE_IF(body, cashflow->getFixingDate(), includeFixingDate, convertDatesToExcelFormat);
			MLIB_PUSH_BACK_DATE_IF(body, cashflow->getFixingEndDate(), includeFixingEndDate, convertDatesToExcelFormat);

			MLIB_PUSH_BACK_DATE_IF(body, cashflow->getAccrualStartDate(), includeAccrualStart, convertDatesToExcelFormat);
			MLIB_PUSH_BACK_DATE_IF(body, cashflow->getAccrualEndDate(), includeAccrualEnd, convertDatesToExcelFormat);

			MLIB_PUSH_BACK_IF(body, cashflow->isUpfrontCashflow() ? std::numeric_limits<double>::quiet_NaN() : cashflow->getAccrualDays(), includeAccrualDays);

			MLIB_PUSH_BACK_IF(body, cashflow->getAccrualYearFraction(), includeAccrualYearFraction);

			MLIB_PUSH_BACK_DATE_IF(body, cashflow->getPaymentDate(), includePaymentDate, convertDatesToExcelFormat);

			MLIB_PUSH_BACK_IF(body, cashflow->getAmount(), includeAmount);
			MLIB_PUSH_BACK_IF(body, toString(cashflow->getPayReceive()), includePayReceive);

			MLIB_PUSH_BACK_IF(body, cashflow->getNotional(), includeNotional);

			// For FRA
			MLIB_PUSH_BACK_IF(body, cashflow->getFixedRate(), includeSrikeRate);

			MLIB_PUSH_BACK_IF(body, cashflow->getNotionalExchange(), includeNotionalExchange);
			MLIB_PUSH_BACK_IF(body, cashflow->getLeverage(), includeLeverage);

			//For Premium Schedule
			MLIB_PUSH_BACK_IF(body, cashflow->getCdsSpread(), includeCdsSpread);

			MLIB_PUSH_BACK_IF(body, cashflow->getFixedRate(), includeFixedRate);

			MLIB_PUSH_BACK_IF(body, cashflow->getCouponMultiplier(), includeCouponMultiplier);
			MLIB_PUSH_BACK_IF(body, cashflow->getSpread(), includeSpread);

			// Bespoked cashflow fields

			auto bespokeInfo = cashflow->bespokeInfo();
			MLIB_PUSH_BACK_IF(body, toString(bespokeInfo.accrualDaycount), includeAccrualDaycount);
			MLIB_PUSH_BACK_IF(body, bespokeInfo.forecastCurve, includeForecastCurve);

			bodyBlock.push_back(body);
		}

	}

	std::pair<AnyTypeMatrix, AnyTypeMatrix> Schedule::view(bool showBespokeProperties, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList, const bool convertDatesToExcelFormat) const
	{

		const size_t MAX_SCHEDULE_CASHFLOW_COLUMNS = 30;
		const size_t maxRows = getAllCashflows().size();

		AnyTypeVector headers;
		headers.reserve(MAX_SCHEDULE_CASHFLOW_COLUMNS);

		AnyTypeMatrix bodyBlock;
		bodyBlock.reserve(maxRows);

		populateHeaderAndBody(headers, bodyBlock, showColumnHeaders, columnList, convertDatesToExcelFormat );

		// Validate the result size
		validateDisplayCashflowHeaderAndBody(showColumnHeaders, headers, bodyBlock);

        //3) Add header row to header matrix
        AnyTypeMatrix headerBlock;

        if( showColumnHeaders )
        {
			//Populate Schedule Properties for Bespoke Schedule
	        AnyTypeVector row;
			if (showBespokeProperties)
			{
                //Title of the ScheduleProperties display
				row.push_back((std::string)"ScheduleProperties");
				headerBlock.push_back(row);

                //BeskpokeScheduleProperties
                auto keys = inputParameters_.getKeys();
                auto values = inputParameters_.getValues();
                for(size_t i=0; i < keys.size(); ++i) 
                {
					row.clear();
					row.push_back(keys[i]);
					row.push_back(values[i]);
					headerBlock.push_back(row);
                }

                //Title of the Schedule display
			    row.clear();
			    row.push_back((std::string)"Schedule");
    			headerBlock.push_back(row);
            }
			
            headerBlock.push_back( headers );
        }

		return std::make_pair(headerBlock, bodyBlock);

	} 

    //this method is only for backward compatibility with me methods (none LWO)	
	LAStringMatrix Schedule::display(bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList, const bool convertDatesToExcelFormat) const
	{
		auto block = Schedule::view(false, showColumnHeaders, columnList, convertDatesToExcelFormat);
	    auto headerBlock = block.first;
	    auto bodyBlock = block.second;

        LAStringMatrix ret;
        LAStringVector row;

        if(showColumnHeaders)
        {
            for( size_t i = 0; i < headerBlock.size(); ++i )
            {
                row.clear();
                for( size_t j = 0; j < headerBlock[0].size(); ++j )
                {
                    std::stringstream s;
				    s << headerBlock[i][j];
                    row.push_back( s.str().c_str() );
                }

                ret.push_back( row );
            }
	    }

        double defaultLADate = fromLADateToDouble(LADate());
        auto nanValue = std::numeric_limits<double>::quiet_NaN();

        for( size_t i = 0; i < bodyBlock.size(); ++i )
        {
            row.clear();

            for( size_t j = 0; j < bodyBlock[0].size(); ++j )
            {

                auto rawValue = bodyBlock[i][j];

				//Allow Schedule's cashflow to contain string value
				if (rawValue.type() == typeid(std::string))
				{
					std::string value = boost::get< std::string >(rawValue);
					row.push_back(value.c_str());
				}
				else
				{
					double value = (rawValue.type() == typeid(double)) ? boost::get< double >(rawValue) : nanValue;

					bool isUpfrontCashflow = (i == 0 && getUpfrontCashflow() != nullptr);

					std::stringstream s;
					//1) Handle NaN value, by setting the result to empty string
					//2) Handle upfront cashflow, by setting LADate() value to empty string
					if (boost::math::isnan(value) || (isUpfrontCashflow && (value - defaultLADate == 0)))
					{
						s << std::string();
					}
					else
					{
						s << std::fixed << std::setprecision(20) << value;
					}
					row.push_back(s.str().c_str());
				}

            }
            ret.push_back( row );
	    }
		return ret;
	}

    const std::string Schedule::getDataSchemaName() const
    {
        auto schemaName = isBespoke()? toString(BESPOKE_SCHEDULE_PROPERTIES) : toString(SCHEDULE);
        return schemaName;
    }

    const std::string Schedule::getCashflowSchemaName() const
    {
        return "DUMMY_BESPOKE_CASHFLOW";
    }

    const DataSchema Schedule::generateDataSchema(const std::string& schemaName) const
    {
        std::string schName = (schemaName.size()!=0) ? schemaName : getDataSchemaName();
	    const DataSchema  dynamicSchema(    schName,2,
                                            boost::assign::list_of(STRING_VALUE)(VARIANT_VALUE),
		                                    boost::assign::list_of("VARIABLE_NAME")("VARIABLE_VALUE"));
        return dynamicSchema;
    }

    std::map<std::string, Variant> Schedule::getDataMap() const
    {
        std::map<std::string, Variant> dataMap;
        auto keys = inputParameters_.getKeys();
        auto values = inputParameters_.getValues();
        for(size_t i=0; i < keys.size(); ++i) 
        {
			dataMap[keys[i]] = values[i];
        }
        return dataMap;
    }

    const DataSchema Schedule::generateCashflowSchema(const std::string& schemaName) const
    {
    	throw LACoreInvalidData( "#Error: Bespoke cashflows not supported on base class Schedule", __FILE__, __LINE__ );
    }

    std::map<std::string, std::vector<std::string>> Schedule::getCashflowDataMap() const
    {
        std::map<std::string, std::vector<std::string>> dataMap;
        return dataMap;
    }

    const SchemaObject Schedule::toSchemaObject() const
	{
        SchemaObject schemaObject( SCHEDULE, getRefToName());
        addInputParametersToSchemaObject(schemaObject, getDataSchemaName());
		addBespokeCashflowsToSchemaObject(schemaObject, getCashflowSchemaName());

		return schemaObject;
	}

    void Schedule::addInputParametersToSchemaObject(SchemaObject& schemaObject, const std::string& schemaName) const
    {
        //Populate data with key->value pairs
        auto dataMap = getDataMap();
        if (dataMap.size()!=0)
        {
            schemaObject.addDataSchema(generateDataSchema(schemaName));
            schemaObject.setDataForSchemaWithMap(schemaName, dataMap);
        }
    }
    void Schedule::addBespokeCashflowsToSchemaObject(SchemaObject& schemaObject, const std::string& bespokeCashflowSchemaName) const
    {
		//Populate data with key->values pairs
		auto dataMapMultiValues = getCashflowDataMap();
		if (dataMapMultiValues.size() != 0)
		{
			schemaObject.addDataSchema(generateCashflowSchema(bespokeCashflowSchemaName));
			for(auto iter = dataMapMultiValues.begin(); iter != dataMapMultiValues.end(); iter++) 
			{
				schemaObject.setColumnData( bespokeCashflowSchemaName, iter->first, iter->second ); 
			}
		}
    }

    void Schedule::determineIsIrregularStub()
    {
        // Initialize to false
        isIrregularStub_ = false;

        MLIB_REQUIRE( !cashflows_.empty(), "Schedule Error - No cashflows to evaluate." )

        if (firstStub_.size() != 0 || lastStub_.size() != 0)
        {
            isIrregularStub_ = true;
            return;
        }

        LADate swapStart = getEffectiveDate();

        LADate unadjustSwapEnd = getUnadjustedMaturityDate();

        LAString frequency = getFrequencyString( getFrequencyTenor( accrualFrequency_) );

        // Maturities if quoted as a tenor are adjusted and when quoted as dates are unadjusted
        bool isMaturityDateAdjusted = etrading::isMaturityDateTenor(accrualEndDateOrTenor_);

        // get rollDay
        int* rollDayPtr = nullptr;
        int  rollDay = 0;
        LAString* rollConventionPtr = nullptr;
        LAString rollConvention = LAString("");
        if (rollDayInput_ != LAString("0") && rollDayInput_.size() != 0)
        {
            // Generate Fixed Leg Coupon Roll Conventions if the rollDayString is not empty or set to zero
            rollDayPtr = &rollDay;
            rollConventionPtr = &rollConvention;

            bool temp;
            etrading::validateAndPopulateRollDayConventions(rollDayInput_, swapStart, unadjustSwapEnd, &rollDayPtr, &rollConventionPtr, temp, temp);
        }

        // is it regular schedule?
        LAString rollConv = getRollConvection(swapStart, unadjustSwapEnd, rollDayInput_);
        if (rollDayPtr != nullptr)
        {
            rollDay = *rollDayPtr;
        }

        // Note: Irregular stub, pay attention to the not operator !!!
        isIrregularStub_ = !etrading::isRegularSwapSchedule(swapStart, unadjustSwapEnd, isMaturityDateAdjusted, frequency, toString(accrualbusinessDayAdj_).c_str(), accrualCalendar_, rollDay, rollConv);
        return ;

    }

}
