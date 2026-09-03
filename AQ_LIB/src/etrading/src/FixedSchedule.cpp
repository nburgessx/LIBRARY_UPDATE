/*
 * @brief			Class the defines the fixed leg schedule 
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "FixedSchedule.h"
#include "FixedCashflow.h"
#include "CurveUtilities.h"
#include "BondUtilities.h"

namespace etrading
{

	FixedSchedule::FixedSchedule(const std::string& instanceName) : Schedule(instanceName), fixedRate_(0)
	{
        scheduleType_ = FIXED_SCHEDULE_TYPE;
    }

	void FixedSchedule::initialize(const LabelValueBlock& scheduleLVB)
	{

		const std::string inputLVB = "scheduleLVB";
		inputParameters_ = scheduleLVB;

        // Add Schedule Type to LVB
        inputParameters_ = LabelValueBlock( inputParameters_, IRS_KEY::SCHEDULE_TYPE, toString(scheduleType_) );

		payerReceiver_ = toPayReceiveEnum(scheduleLVB.getCompulsoryValueAsLAStringFromKeys(IRS_KEY::PAY_RECEIVE, IRS_KEY::PAYER_RECEIVER, inputLVB).getCString());

		amortization_ = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::AMORTIZATION);
		amortFrequency_ = scheduleLVB.getOptionalValueAsUnsignedInt(IRS_KEY::AMORT_FREQUENCY);

		notionalExchangeEnum_ = toNotionalExchangeEnum(scheduleLVB.getOptionalValueAsLAString(IRS_KEY::NOTIONAL_EXCHANGE, "NONE").getCString());
		leverage_ = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::LEVERAGE, 1.);
		couponMultiplier_ = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::COUPONMULTIPLIER, 1.);

		fixedRate_ = scheduleLVB.getCompulsoryValueAsDoubleFromKeys(IRS_KEY::FIXED_RATE, BOND_KEY::COUPON, inputLVB);

		accrualEndDateOrTenor_ = scheduleLVB.getCompulsoryValueAsLAString(IRS_KEY::MATURITY_DATE, inputLVB);

		LAString fixedBusinessDayAdjustment = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FIXED_BUSINESSDAYADJUSTMENT)(IRS_KEY::BUSINESSDAYADJUSTMENT));
		LAString fixedCalendar = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FIXED_CALENDAR)(IRS_KEY::CALENDAR));
		LAString fixedLegFreq = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FIXED_FREQUENCY)(IRS_KEY::FREQUENCY));
		LAString fixedDayCount = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FIXED_DAYCOUNT)(IRS_KEY::DAYCOUNT));

		accrualbusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALBUSINESSDAYADJUSTMENT, IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT, fixedBusinessDayAdjustment).getCString());
		accrualCalendar_ = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALCALENDAR, IRS_KEY::ACCRUALCALENDAR, fixedCalendar);
		accrualFrequency_ = toFrequencyEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALFREQUENCY, IRS_KEY::ACCRUALFREQUENCY, fixedLegFreq).getCString());
		accrualDaycount_ = toDayCountEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ACCRUALDAYCOUNT, IRS_KEY::ACCRUALDAYCOUNT, fixedDayCount).getCString());

		auto defaultBusinessDayAdj = fixedBusinessDayAdjustment.size() != 0 ? fixedBusinessDayAdjustment : toString(accrualbusinessDayAdj_).c_str();
		auto defaultCalendar = fixedCalendar.size() != 0 ? fixedCalendar : accrualCalendar_;
		auto defaultFrequency = fixedLegFreq.size() != 0 ? fixedLegFreq : toString(accrualFrequency_).c_str();

		paymentbusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTBUSINESSDAYADJUSTMENT, IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj).getCString());
		paymentCalendar_ = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTCALENDAR, IRS_KEY::PAYMENTCALENDAR, defaultCalendar);
		paymentFrequency_ = toFrequencyEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTFREQUENCY, IRS_KEY::PAYMENTFREQUENCY, defaultFrequency).getCString());

		populateNotionalAndPaymentFreqEnum(scheduleLVB);

		// Allowable Stub LVB Keys
		std::vector<std::string> firstStubKeys;
		firstStubKeys.push_back(IRS_KEY::FIXED_FIRSTSTUBDATE);
		firstStubKeys.push_back(IRS_KEY::FIRSTSTUBDATE);
		firstStubKeys.push_back(BOND_KEY::FIRST_COUPON_DATE);

		std::vector<std::string> lastStubKeys;
		lastStubKeys.push_back(IRS_KEY::FIXED_LASTSTUBDATE);
		lastStubKeys.push_back(IRS_KEY::LASTSTUBDATE);
		lastStubKeys.push_back(BOND_KEY::LAST_COUPON_DATE);

		firstStub_ = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(firstStubKeys);
		lastStub_ = scheduleLVB.getOptionalValueAsLAStringFromMultipleKeys(lastStubKeys);

		rollDayInput_ = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_ROLLDAY, IRS_KEY::ROLLDAY);
		payLag_ = scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_PAYMENTLAG, IRS_KEY::PAYMENTLAG, "0D");
		stubType_ = toStubTypeEnum(scheduleLVB.getOptionalValueAsLAStringFromKeys(IRS_KEY::FIXED_STUBTYPE, IRS_KEY::STUBTYPE).getCString());

		//fxFixing
		fxFixingLag_ = scheduleLVB.getOptionalValueAsLAString(IRS_KEY::FXFIXINGLAG, "0D");
		fxFixingbusinessDayAdj_ = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsLAString(IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj).getCString());
		fxFixingCalendar_ = scheduleLVB.getOptionalValueAsLAString(IRS_KEY::FXFIXINGCALENDAR, defaultCalendar);

		populateAccrualStartDates(scheduleLVB);

	}


	FixedSchedule::FixedSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : Schedule(instanceName)
	{
		scheduleType_ = FIXED_SCHEDULE_TYPE;

		initialize(scheduleLVB);
   
        //Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates, fixingDates
		Schedule::calculateScheduleDates();

        //Populate the cashflows based on schedule inputs
        createCashflows(); 
	}

    void FixedSchedule::populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB)
    {
		Schedule::populateNotionalAndPaymentFreqEnum();

		if (scheduleLVB.size() == 0)
		{
			return;
		}

		notional_                               = scheduleLVB.getOptionalValueAsDoubleFromKeys( IRS_KEY::NOTIONAL, BOND_KEY::FACE_VALUE, std::numeric_limits<double>::quiet_NaN() ); 
		futureValueNotional_                    = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::FV_NOTIONAL, std::numeric_limits<double>::quiet_NaN());

		//When FV Notional is provided, Notional will be calculated from the FutureNotional 
		if (!boost::math::isnan(futureValueNotional_))
		{
			if (paymentFreqEnum_ != AT_MATURITY_FREQUENCY) 
			{
        		throw LACoreInvalidData( "#Error: When FVNotional is specified, the payment frequency must be 'AT MATURITY'", __FILE__, __LINE__ );
			}

			if (!boost::math::isnan(notional_) )
			{
	    		throw LACoreInvalidData( "#Error: Please provide either FVNotional or Notional for FixedSchedule, but not both", __FILE__, __LINE__ );
			}

			notional_ = futureValueNotional_;

		}
		// When FV Notional is not provided, notinoal is a mandatory field
		else if (boost::math::isnan(notional_))
        {
        	throw LACoreInvalidData( "#Error: Notional is a mandatory field for FixedSchedule", __FILE__, __LINE__ );
        }

    }

    void FixedSchedule::createUpfrontCashflow(const LADate& paymentDate, double leverage) 
    {
        if (notionalExchangeEnum_ == START_NE || notionalExchangeEnum_ == START_AND_END_NE)
        {
            auto nanDoubleValue = std::numeric_limits<double>::quiet_NaN();

            upfrontCashflow_= CashflowPtr(new FixedCashflow(payerReceiver_, nanDoubleValue, LADate(), LADate(), 0, nanDoubleValue, paymentDate, nanDoubleValue, leverage, 1.0 /*couponMultiplier*/, paymentFreqEnum_, FIRST_NOTIONAL_EXCHANGE_CASHFLOW_TYPE, CashFlowBespokeInfo()));
            upfrontCashflow_->setFwdFxRate(nanDoubleValue);
    	}
    }

 

    void FixedSchedule::createCashflows() 
    {
        //Use the first accrualStartDate as the paymentDate
        auto paymentDate = accrualStartDates_.at(0);
        createUpfrontCashflow(paymentDate, leverage_);

        auto cashflowSize = accrualStartDates_.size();

        std::vector<double> cashflowNotionals = getCashflowNotionals(cashflowSize); 

        for (size_t i=0; i < cashflowSize; ++i)
		{
			auto cashflowType = (i == cashflowSize - 1) ? NORMAL_LAST_CASHFLOW_TYPE : NORMAL_CASHFLOW_TYPE;

            double cashflowNotional = cashflowNotionals[i];

			CashFlowBespokeInfo bespokeInfo;
			bespokeInfo.accrualDaycount = accrualDaycount_;

			CashflowPtr cf = CashflowPtr(new FixedCashflow(payerReceiver_, fixedRate_, accrualStartDates_[i], accrualEndDates_[i], accrualDays_[i], accrualYearFractions_[i], paymentDates_[i], cashflowNotional, leverage_, couponMultiplier_, paymentFreqEnum_, cashflowType, bespokeInfo));

            cashflows_.push_back(cf);
		}

        // Update notional exchanges based on notionals of cashflows
        updateNotionalExchange();

    }

	//Bespoke schedule
	FixedSchedule::FixedSchedule(const std::string& instanceName, const LabelValueBlock& bespokeScheduleProperties, const std::vector<LabelValueBlock>& cashflowLVBs, const BespokeScheduleTypeEnum& bespokeScheduleType) : Schedule(instanceName, bespokeScheduleProperties, cashflowLVBs, bespokeScheduleType)
    {
		createBespokeCashflows(cashflowLVBs);

		// Update notional exchanges based on notionals of cashflows
		updateNotionalExchange();

	}

    SchedulePtr FixedSchedule::clone()
    {
        SchedulePtr sch = SchedulePtr(new FixedSchedule(*this));
        return sch;
    }

	FixedSchedule::FixedSchedule(const FixedSchedule& rhs) : Schedule(rhs), fixedRate_(rhs.fixedRate_)
	{};

	void FixedSchedule::createBespokeCashflows(const std::vector<LabelValueBlock>& cashflowLVBs) 
	{

	    const std::string inputLVB = "cashflowLVB";

        if (cashflowLVBs.size() ==0 )
        {
    		throw LACoreInvalidData( "#Error: The cashflows of bespokeSchedule need to be provided", __FILE__, __LINE__ );
        }

        //1) Create a upfront cashflow for the notionalExchange is START or START_AND_END

        //Use the first accrualStartDate as the paymentDate
		auto firstCashflowLVB = cashflowLVBs.at(0);
		
		LADate paymentDate	= firstCashflowLVB.getCompulsoryValueAsDate(CASHFLOW_KEY::ACCRUAL_START, inputLVB );
		double leverage = firstCashflowLVB.getCompulsoryValueAsDouble( IRS_KEY::LEVERAGE,  inputLVB );
        createUpfrontCashflow(paymentDate, leverage);

		//2) Create the normal cashflows
		auto cashflowSize = cashflowLVBs.size();

		DayCountEnum accrualDaycount = accrualDaycount_;


        for (size_t i=0; i < cashflowLVBs.size(); ++i)
		{
            LabelValueBlock cashflowLVB = cashflowLVBs[i];

			if (bespokeScheduleType_ == BESPOKE_SCHEDULE)
			{
				accrualDaycount = toDayCountEnum(cashflowLVB.getCompulsoryValueAsString(IRS_KEY::ACCRUALDAYCOUNT, inputLVB));
			}

            LADate accrualStart	= cashflowLVB.getCompulsoryValueAsDate( CASHFLOW_KEY::ACCRUAL_START, inputLVB );
			LADate accrualEnd = cashflowLVB.getCompulsoryValueAsDate( CASHFLOW_KEY::ACCRUAL_END,  inputLVB );
			if (accrualStart > accrualEnd) 
			{
				throw LACoreInvalidData( "#Error: Accrual Start Date cannot be later than Accrual End Date", __FILE__, __LINE__ );
			}

            LADate paymentDate = cashflowLVB.getCompulsoryValueAsDate( CASHFLOW_KEY::PAYMENT_DATE,  inputLVB );

			double notional = cashflowLVB.getCompulsoryValueAsDouble( IRS_KEY::NOTIONAL,  inputLVB );
			double leverage = cashflowLVB.getCompulsoryValueAsDouble( IRS_KEY::LEVERAGE,  inputLVB );
			double couponMultiplier = cashflowLVB.getOptionalValueAsDouble(IRS_KEY::COUPONMULTIPLIER, 1.0);
			double fixedRate = cashflowLVB.getCompulsoryValueAsDouble( IRS_KEY::FIXED_RATE,  inputLVB );
			double accrualYearFractions = getYearFraction(accrualStart, accrualEnd, accrualDaycount, false);
			int accrualDays = getDays(accrualStart, accrualEnd);

			auto cashflowType = (i == cashflowSize - 1) ? NORMAL_LAST_CASHFLOW_TYPE : NORMAL_CASHFLOW_TYPE;

			CashFlowBespokeInfo bespokeInfo;
			bespokeInfo.accrualDaycount = accrualDaycount;

            CashflowPtr cf = CashflowPtr(new FixedCashflow(payerReceiver_, fixedRate, accrualStart, accrualEnd, accrualDays, accrualYearFractions, paymentDate, notional, leverage, couponMultiplier, paymentFreqEnum_, cashflowType, bespokeInfo));

            cashflows_.push_back(cf);

            //Update the schedule outputs
            accrualStartDates_.push_back(accrualStart);
		    accrualEndDates_.push_back(accrualEnd);
		    accrualYearFractions_.push_back(accrualYearFractions);
		    accrualDays_.push_back(accrualDays);
		    paymentDates_.push_back(paymentDate);

		}
	}

	double FixedSchedule::getFixedRate() const
	{
		return fixedRate_;
	}

    const std::string FixedSchedule::getCashflowSchemaName() const
    {
		return toString(bespokeScheduleType_);
    }

    const DataSchema FixedSchedule::generateCashflowSchema(const std::string& schemaName) const
    {
        if (!isBespoke())
        {
            return Schedule::generateCashflowSchema(schemaName);
        }
        std::string schName = (schemaName.size()!=0) ? schemaName : getDataSchemaName();
		const int NUM_OF_COLUMNS = 8;
	    const DataSchema  dynamicSchema(    schName, NUM_OF_COLUMNS,
                                            boost::assign::list_of(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE),
		                                    boost::assign::list_of(toString(ACCRUAL_START_HEADER))(toString(ACCRUAL_END_HEADER))(toString(PAYMENT_DATE_HEADER))(toString(NOTIONAL_HEADER))(toString(LEVERAGE_HEADER))(toString(COUPON_MULTIPLIER_HEADER))(toString(FIXED_RATE_HEADER))
																(toString(ACCRUAL_DAYCOUNT_HEADER)) );
        return dynamicSchema;
    }

    std::map<std::string, std::vector<std::string>> FixedSchedule::getCashflowDataMap() const
    {
        std::map<std::string, std::vector<std::string>> dataMap;
        if (isBespoke())
        {
			auto expectedSize = cashflows_.size();

			std::vector<std::string> accrualStartVec(expectedSize);
            std::vector<std::string> accrualEndVec(expectedSize);
            std::vector<std::string> paymentDateVec(expectedSize);
            std::vector<std::string> notionalVec(expectedSize);
            std::vector<std::string> leverageVec(expectedSize);
			std::vector<std::string> couponMultiplierVec(expectedSize);
			std::vector<std::string> fixedRateVec(expectedSize);
			std::vector<std::string> accrualDaycountVec(expectedSize);

            for (size_t i=0; i < cashflows_.size(); ++i)
            {
   	            auto cf = cashflows_[i];	
				accrualStartVec[i] = cf->getAccrualStartDate().stringWithFormat().getCString();
				accrualEndVec[i] = cf->getAccrualEndDate().stringWithFormat().getCString();
				paymentDateVec[i] = cf->getPaymentDate().stringWithFormat().getCString();
				notionalVec[i] = boost::lexical_cast<std::string>(cf->getNotional());
				leverageVec[i] = boost::lexical_cast<std::string>(cf->getLeverage());
				couponMultiplierVec[i] = boost::lexical_cast<std::string>(cf->getCouponMultiplier());
				fixedRateVec[i] = boost::lexical_cast<std::string>(cf->getFixedRate());
				accrualDaycountVec[i] = toString(cf->bespokeInfo().accrualDaycount);
			}

			dataMap[toString(ACCRUAL_START_HEADER)] = accrualStartVec;
			dataMap[toString(ACCRUAL_END_HEADER)] = accrualEndVec;
			dataMap[toString(PAYMENT_DATE_HEADER)] = paymentDateVec;
			dataMap[toString(NOTIONAL_HEADER)] = notionalVec;
			dataMap[toString(LEVERAGE_HEADER)] = leverageVec;
			dataMap[toString(COUPON_MULTIPLIER_HEADER)] = couponMultiplierVec;
			dataMap[toString(FIXED_RATE_HEADER)] = fixedRateVec;
			dataMap[toString(ACCRUAL_DAYCOUNT_HEADER)] = accrualDaycountVec;

        }

        return dataMap;
    }

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> FixedSchedule::allowedColumns() const
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
			,FIXED_RATE_HEADER

			// Bespoked cashflow fields
			, ACCRUAL_DAYCOUNT_HEADER

		};

		return expectedList;
	};


}
