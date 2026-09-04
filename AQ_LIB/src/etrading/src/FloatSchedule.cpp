#include "FloatSchedule.h"
#include "FloatCashflow.h"
#include "SwapValidation.h"
#include "CurveValidation.h"
#include "AQLDateScheduleHelpers.h"

namespace etrading
{

	FloatSchedule::FloatSchedule(const std::string& instanceName) : Schedule(instanceName), spread_(0)
	{
        scheduleType_ = FLOAT_SCHEDULE_TYPE;
    }

	FloatSchedule::FloatSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : Schedule(instanceName)
	{
		// Initialise the FloatSchedule from the supplied label-value block
		initialise( scheduleLVB );

        //Populate the cashflows based on schedule inputs
        createCashflows();

        // Determine if the Stubs are Irregular Stub
        determineIsIrregularStub();
    }

	// @brief: A helper method which initialises the FloatSchedule from a label-value block.
	void FloatSchedule::initialise( const LabelValueBlock& scheduleLVB )
	{
		const std::string inputLVB = "scheduleLVB";
		inputParameters_ = scheduleLVB;

    	scheduleType_ = FLOAT_SCHEDULE_TYPE;

        // Add Schedule_Type to LVB
        inputParameters_ = LabelValueBlock( inputParameters_, IRS_KEY::SCHEDULE_TYPE, toString(scheduleType_) );

		payerReceiver_  = toPayReceiveEnum(scheduleLVB.getCompulsoryValueAsAQLStringFromKeys(IRS_KEY::PAY_RECEIVE, IRS_KEY::PAYER_RECEIVER, inputLVB).getCString());

        amortization_ = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::AMORTIZATION);
        amortFrequency_ = scheduleLVB.getOptionalValueAsUnsignedInt(IRS_KEY::AMORT_FREQUENCY);

        notionalExchangeEnum_	= toNotionalExchangeEnum(scheduleLVB.getOptionalValueAsAQLString(IRS_KEY::NOTIONAL_EXCHANGE, "NONE").getCString());
		leverage_		  = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::LEVERAGE, 1.);
		couponMultiplier_ = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::COUPONMULTIPLIER, 1.);
		
        spread_ = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::FLOAT_SPREAD, 0.);
        
        accrualEndDateOrTenor_ = scheduleLVB.getCompulsoryValueAsAQLString( IRS_KEY::MATURITY_DATE,  inputLVB );

   		AQLString floatBusinessDayAdjustment = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT)(IRS_KEY::BUSINESSDAYADJUSTMENT));
        AQLString floatCalendar	= scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FLOAT_CALENDAR)(IRS_KEY::CALENDAR));
		AQLString floatLegFreq	= scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FLOAT_FREQUENCY)(IRS_KEY::FREQUENCY));
        AQLString floatDayCount	= scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys(boost::assign::list_of(IRS_KEY::FLOAT_DAYCOUNT)(IRS_KEY::DAYCOUNT));

		accrualbusinessDayAdj_  = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT,		IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT, floatBusinessDayAdjustment).getCString());
        accrualCalendar_	    = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_ACCRUALCALENDAR,					IRS_KEY::ACCRUALCALENDAR, floatCalendar);
	    accrualFrequency_		= toFrequencyEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_ACCRUALFREQUENCY,					IRS_KEY::ACCRUALFREQUENCY, floatLegFreq ).getCString());
		accrualDaycount_		= toDayCountEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_ACCRUALDAYCOUNT,					IRS_KEY::ACCRUALDAYCOUNT, floatDayCount ).getCString());

        auto defaultBusinessDayAdj = floatBusinessDayAdjustment.size() != 0 ? floatBusinessDayAdjustment : toString(accrualbusinessDayAdj_).c_str();
        auto defaultCalendar = floatCalendar.size() != 0 ? floatCalendar : accrualCalendar_;
        auto defaultFrequency = floatLegFreq.size() != 0 ? floatLegFreq : toString(accrualFrequency_).c_str();
  	
		paymentbusinessDayAdj_  = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_PAYMENTBUSINESSDAYADJUSTMENT,		IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj).getCString() );
        paymentCalendar_	    = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_PAYMENTCALENDAR,					IRS_KEY::PAYMENTCALENDAR, defaultCalendar );
        paymentFrequency_		= toFrequencyEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_PAYMENTFREQUENCY,					IRS_KEY::PAYMENTFREQUENCY, defaultFrequency).getCString());
       
        populateNotionalAndPaymentFreqEnum(scheduleLVB);

        firstStub_		        = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_FIRSTSTUBDATE,						IRS_KEY::FIRSTSTUBDATE);
        lastStub_	            = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_LASTSTUBDATE,						IRS_KEY::LASTSTUBDATE);
        rollDayInput_	        = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_ROLLDAY,							IRS_KEY::ROLLDAY);
        payLag_					= scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_PAYMENTLAG,							IRS_KEY::PAYMENTLAG, "0D");
        stubType_			    = toStubTypeEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_STUBTYPE,			IRS_KEY::STUBTYPE).getCString());
	   
		fixingbusinessDayAdj_   = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_FIXINGBUSINESSDAYADJUSTMENT,		IRS_KEY::FIXINGBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj ).getCString());
        fixingCalendar_         = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_FIXINGCALENDAR,						IRS_KEY::FIXINGCALENDAR, defaultCalendar);
		fixLag_					= scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_FIXINGLAG,							IRS_KEY::FIXINGLAG, "0D");
        firstFixLag_            = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_FIRSTFIXINGLAG,						IRS_KEY::FIRSTFIXINGLAG, fixLag_);
		fixingAdvanceOrArrears_	= scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FLOAT_FIXINGADVANCEORARREAR,				IRS_KEY::FIXINGADVANCEORARREAR, "advance");

        //fxFixing
        fxFixingLag_ = scheduleLVB.getOptionalValueAsAQLString( IRS_KEY::FXFIXINGLAG, "0D" );
		fxFixingbusinessDayAdj_   = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsAQLString(IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj ).getCString());
        fxFixingCalendar_         = scheduleLVB.getOptionalValueAsAQLString(IRS_KEY::FXFIXINGCALENDAR, defaultCalendar);

        populateAccrualStartDates(scheduleLVB);

        //Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates, fixingDates
		Schedule::calculateScheduleDates();
	}

	void FloatSchedule::populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB)
    {
		Schedule::populateNotionalAndPaymentFreqEnum();

		if (scheduleLVB.size() == 0)
		{
			return;
		}

		//Notional is always mandatory for Float leg
		notional_= scheduleLVB.getCompulsoryValueAsDouble(IRS_KEY::NOTIONAL);

		futureValueNotional_ = scheduleLVB.getOptionalValueAsDouble(IRS_KEY::FV_NOTIONAL, std::numeric_limits<double>::quiet_NaN());

		//FutureNotional is for Zero Coupon Swap
        if (!boost::math::isnan(futureValueNotional_))
        {
        	throw AQLCoreInvalidData( "#Error: Float leg does not support FVNotional", __FILE__, __LINE__ );
        }
	
	}

    void FloatSchedule::createUpfrontCashflow(const AQLDate& paymentDate, double leverage) 
    {
        if (notionalExchangeEnum_ == START_NE || notionalExchangeEnum_ == START_AND_END_NE)
		{
            auto nanDoubleValue = std::numeric_limits<double>::quiet_NaN();

			upfrontCashflow_= CashflowPtr(new FloatCashflow(payerReceiver_, nanDoubleValue, AQLDate(), AQLDate(), AQLDate(), 0, nanDoubleValue, paymentDate, nanDoubleValue, leverage, 1.0 /*couponMultiplier*/, paymentFreqEnum_, FIRST_NOTIONAL_EXCHANGE_CASHFLOW_TYPE, CashFlowBespokeInfo()));
            upfrontCashflow_->setFwdFxRate(nanDoubleValue);
    	}
    }

    void FloatSchedule::createCashflows() 
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
			bespokeInfo.forecastCurve = "";

			CashflowPtr cf = CashflowPtr(new FloatCashflow(payerReceiver_, spread_, getFixingDate(i), accrualStartDates_[i], accrualEndDates_[i], accrualDays_[i], accrualYearFractions_[i], paymentDates_[i], cashflowNotional, leverage_, couponMultiplier_, paymentFreqEnum_, cashflowType, bespokeInfo));
            
			cf->setFixingEndDate(fixingEndDates_[i]);
			
            cashflows_.push_back(cf);
		}

        // Update notional exchanges based on notionals of cashflows
        updateNotionalExchange();
   

    }

	//Bespoke schedule
	FloatSchedule::FloatSchedule(const std::string& instanceName, const LabelValueBlock& bespokeScheduleProperties, const std::vector<LabelValueBlock>& cashflowLVBs, const BespokeScheduleTypeEnum& bespokeScheduleType) : Schedule(instanceName, bespokeScheduleProperties, cashflowLVBs, bespokeScheduleType)
    {
		createBespokeCashflows(cashflowLVBs);

		// Update notional exchanges based on notionals of cashflows
		updateNotionalExchange();

		if (bespokeScheduleType == BESPOKE_SCHEDULE_WITH_PROPERTIES)
		{
			//Need to set it to true, because we don't have information on firstFixingDate & lastFixingDate, 
			// so we cannot determine the Irregular stub using normal determineIsIrregularStub().
			isIrregularStub_ = true;
		}
	}

    SchedulePtr FloatSchedule::clone()
    {
        SchedulePtr sch = SchedulePtr(new FloatSchedule(*this));
        return sch;
    }

	FloatSchedule::FloatSchedule(const FloatSchedule& rhs) : Schedule(rhs), spread_(rhs.spread_)
	{}

  void FloatSchedule::updateCashflowsCompoundingCoupons( const DataProvider& dataProvider )
    {
        size_t cashflowSize = cashflows_.size();
        std::vector<CashFlowInfo> prevCashflowsInfo;
        prevCashflowsInfo.reserve(cashflowSize);

		// Note that for leg with paymentFreqEnum as 'AT_MATURITY', the internal paymentFrequency equals to accrualFrequency
        if (paymentFreqEnum_ == AT_MATURITY_FREQUENCY)
        {
            for(size_t i=1; i < cashflowSize; ++i)
            {
				// NOTE: cashflows_ excludes the upfront notional exchange cashflow
                const auto& curCashflow = cashflows_[i];
				const size_t j = i - 1;
				const auto& prevCashflow = cashflows_[j];
				CashflowData cashflowData = dataProvider.getCashflowDataExcludingUpfront( j );
				
				CashFlowInfo cfInfo;
                cfInfo.coupon = prevCashflow->getRegularCoupon( cashflowData );
                cfInfo.spreadAmount = prevCashflow->getSpreadAmount( cashflowData );
                cfInfo.floatRate = cashflowData.floatRateData.resetRate;
                prevCashflowsInfo.push_back(cfInfo); 

                if (prevCashflowsInfo.size() > 0 )
                {
                    curCashflow->setPreviousCashflowsInfo( prevCashflowsInfo );
                }
             }
        }
        //For paymentFreqEnum is AT_MATURITY, isAccrualFreqLessThanPaymentFreq() is always false, as the payment freq will be populated by accrual freq
        //For cashflows with same payment date, we need to include the previous coupon in the compound rate
        else if (getBespokeScheduleType() == BESPOKE_SCHEDULE || isAccrualFreqLessThanPaymentFreq())
        {
            for(size_t i=1; i < cashflowSize; ++i)
            {
                const auto& curCashflow = getCashflow(i);
				const auto& curPaymentDate = curCashflow->getPaymentDate();

				size_t j = i - 1;

				// NOTE: cashflows_ excludes the upfront notional exchange cashflow
                const auto& prevCashflow = cashflows_[j];
                if ( curPaymentDate == prevCashflow->getPaymentDate())
                {
					CashflowData cashflowData = dataProvider.getCashflowDataExcludingUpfront( j );
					CashFlowInfo cfInfo;
	                cfInfo.coupon = prevCashflow->getRegularCoupon( cashflowData );
                    cfInfo.spreadAmount  = prevCashflow->getSpreadAmount( cashflowData );
	                cfInfo.floatRate = cashflowData.floatRateData.resetRate;
                    prevCashflowsInfo.push_back(cfInfo);
                } 
                //previous cashflow's payment date < current cashflow's payment date
                else
                {
					prevCashflowsInfo.clear();

					//This is tempory function until the compounding calculation move to leg level
					prevCashflow->setCashflowTypeToLast();
                }

                if (prevCashflowsInfo.size() > 0 )
                {
                    curCashflow->setPreviousCashflowsInfo(prevCashflowsInfo);
                }
            } //end of for loop
        } 
    }

	void FloatSchedule::createBespokeCashflows(const std::vector<LabelValueBlock>& cashflowLVBs) 
	{

		const std::string inputLVB = "cashflowLVB";

        //1) Create a upfront cashflow for the notionalExchange is START or START_AND_END


        //Use the first accrualStartDate as the paymentDate
		auto firstCashflowLVB = cashflowLVBs.at(0);

        AQLDate paymentDate	= firstCashflowLVB.getCompulsoryValueAsDate(CASHFLOW_KEY::ACCRUAL_START, inputLVB );
		double leverage = firstCashflowLVB.getCompulsoryValueAsDouble( IRS_KEY::LEVERAGE,  inputLVB );
        createUpfrontCashflow(paymentDate, leverage);

		//2) Create the normal cashflows
		auto cashflowSize = cashflowLVBs.size();
			
		DayCountEnum accrualDaycount = accrualDaycount_;

		std::string forecastCurve = "";

        for (size_t i=0; i < cashflowSize; ++i)
		{
			LabelValueBlock cashflowLVB = cashflowLVBs[i];

			if (bespokeScheduleType_ == BESPOKE_SCHEDULE)
			{
				accrualDaycount = toDayCountEnum(cashflowLVB.getCompulsoryValueAsString(IRS_KEY::ACCRUALDAYCOUNT, inputLVB));
				forecastCurve = cashflowLVB.getCompulsoryValueAsString(MARKET_KEY::FORECAST_CURVE, inputLVB);
			}
			
			AQLDate accrualStart	= cashflowLVB.getCompulsoryValueAsDate( CASHFLOW_KEY::ACCRUAL_START, inputLVB );
			AQLDate accrualEnd = cashflowLVB.getCompulsoryValueAsDate( CASHFLOW_KEY::ACCRUAL_END,  inputLVB );
			if (accrualStart > accrualEnd) 
			{
				throw AQLCoreInvalidData( "#Error: Accrual Start Date cannot be later than Accrual End Date", __FILE__, __LINE__ );
			}

			AQLDate fixingDate	= cashflowLVB.getOptionalValueAsDate( CASHFLOW_KEY::FIXING_DATE, accrualStart);
			AQLDate paymentDate = cashflowLVB.getCompulsoryValueAsDate(CASHFLOW_KEY::PAYMENT_DATE, inputLVB);

            if (fixingDate > paymentDate) 
			{
				throw AQLCoreInvalidData( "#Error: Fixing Date cannot be later than Payment Date", __FILE__, __LINE__ );
			}

			double notional			= cashflowLVB.getCompulsoryValueAsDouble( IRS_KEY::NOTIONAL,  inputLVB );
			double leverage			= cashflowLVB.getCompulsoryValueAsDouble( IRS_KEY::LEVERAGE,  inputLVB );
			double couponMultiplier	= cashflowLVB.getOptionalValueAsDouble(IRS_KEY::COUPONMULTIPLIER, 1.0);
			double floatSpread		= cashflowLVB.getCompulsoryValueAsDouble( IRS_KEY::FLOAT_SPREAD,  inputLVB );
			double accrualYearFractions = getYearFraction(accrualStart, accrualEnd, accrualDaycount, false);
			int accrualDays = getDays(accrualStart, accrualEnd);

			auto cashflowType = (i == cashflowSize - 1) ? NORMAL_LAST_CASHFLOW_TYPE : NORMAL_CASHFLOW_TYPE;

			CashFlowBespokeInfo bespokeInfo;
			bespokeInfo.accrualDaycount = accrualDaycount;
			bespokeInfo.forecastCurve = forecastCurve;

            CashflowPtr cf = CashflowPtr(new FloatCashflow(payerReceiver_, floatSpread, fixingDate, accrualStart, accrualEnd, accrualDays, accrualYearFractions, paymentDate, notional, leverage, couponMultiplier, paymentFreqEnum_, cashflowType, bespokeInfo));

			cashflows_.push_back(cf);

            //Update the schedule outputs
			fixingDates_.push_back(fixingDate);
			accrualStartDates_.push_back(accrualStart);
			accrualEndDates_.push_back(accrualEnd);
			accrualYearFractions_.push_back(accrualYearFractions);
			accrualDays_.push_back(accrualDays);
			paymentDates_.push_back(paymentDate);
		}

		//Only populate fixingEndDates for the bespoke schedule with Schedule properties 
		if (bespokeScheduleType_ == BESPOKE_SCHEDULE_WITH_PROPERTIES)
		{
			fixingEndDates_ = AQLDateScheduleHelpers::getMultiDate( fixingDates_, getFrequencyTenor(accrualFrequency_), toString(fixingbusinessDayAdj_).c_str(), fixingCalendar_, nullptr); // rollconvention* = nullptr
		}
		
	}

	double FloatSchedule::getSpread() const
	{
		return spread_;
	}

    bool FloatSchedule::isVariableSpread() const
    {    
        double epsilon = 1e-20;
        double firstSpread = cashflows_.at(0)->getSpread();
     
        for (size_t i = 1; i < cashflows_.size(); ++i)
        {
            auto cashflow = cashflows_[i];
            auto diff = firstSpread - cashflow->getSpread();
            if (std::abs(diff) > epsilon)
            {
                return true;
            }
        }

        return false;
    }


    const std::string FloatSchedule::getCashflowSchemaName() const
    {
		return toString(bespokeScheduleType_);
	}

    const DataSchema FloatSchedule::generateCashflowSchema(const std::string& schemaName) const
    {
        if (!isBespoke())
        {
            return Schedule::generateCashflowSchema(schemaName);
        }
        std::string schName = (schemaName.size()!=0) ? schemaName : getDataSchemaName();
		const int NUM_OF_COLUMNS = 10;
		const DataSchema  dynamicSchema(    schName, NUM_OF_COLUMNS,
                                            boost::assign::list_of(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE)(STRING_VALUE),
		                                    boost::assign::list_of(toString(FIXING_DATE_HEADER))(toString(ACCRUAL_START_HEADER))(toString(ACCRUAL_END_HEADER))(toString(PAYMENT_DATE_HEADER))(toString(NOTIONAL_HEADER))(toString(LEVERAGE_HEADER))(toString(COUPON_MULTIPLIER_HEADER))(toString(FLOAT_SPREAD_HEADER))
																(toString(ACCRUAL_DAYCOUNT_HEADER))(toString(FORECAST_CURVE_HEADER)) );
        return dynamicSchema;
    }

   std::map<std::string, std::vector<std::string>> FloatSchedule::getCashflowDataMap() const
    {
        std::map<std::string, std::vector<std::string>> dataMap;

        if (isBespoke())
        {
			auto expectedSize  = cashflows_.size();

            std::vector<std::string> fixingDateVec(expectedSize);
			std::vector<std::string> accrualStartVec(expectedSize);
            std::vector<std::string> accrualEndVec(expectedSize);
            std::vector<std::string> paymentDateVec(expectedSize);
            std::vector<std::string> notionalVec(expectedSize);
            std::vector<std::string> leverageVec(expectedSize);
			std::vector<std::string> couponMultiplierVec(expectedSize);
			std::vector<std::string> floatSpreadVec(expectedSize);
			std::vector<std::string> accrualDaycountVec(expectedSize);
			std::vector<std::string> forecastCurveVec(expectedSize);

			for (size_t i=0; i < cashflows_.size(); ++i)
            {
   	            auto cf = cashflows_[i];	

				fixingDateVec[i] = cf->getFixingDate().stringWithFormat().getCString();
				accrualStartVec[i] = cf->getAccrualStartDate().stringWithFormat().getCString();
				accrualEndVec[i] = cf->getAccrualEndDate().stringWithFormat().getCString();
				paymentDateVec[i] = cf->getPaymentDate().stringWithFormat().getCString();
				notionalVec[i] = boost::lexical_cast<std::string>(cf->getNotional());
				leverageVec[i] = boost::lexical_cast<std::string>(cf->getLeverage());
				couponMultiplierVec[i] = boost::lexical_cast<std::string>(cf->getCouponMultiplier());
				floatSpreadVec[i] = boost::lexical_cast<std::string>(cf->getSpread());

				accrualDaycountVec[i] = toString(cf->bespokeInfo().accrualDaycount);
				forecastCurveVec[i] = cf->bespokeInfo().forecastCurve;
			}

            dataMap[toString(FIXING_DATE_HEADER)] = fixingDateVec;
            dataMap[toString(ACCRUAL_START_HEADER)] = accrualStartVec;
            dataMap[toString(ACCRUAL_END_HEADER)] = accrualEndVec;
            dataMap[toString(PAYMENT_DATE_HEADER)] = paymentDateVec;
            dataMap[toString(NOTIONAL_HEADER)] = notionalVec;
            dataMap[toString(LEVERAGE_HEADER)] = leverageVec;
			dataMap[toString(COUPON_MULTIPLIER_HEADER)] = couponMultiplierVec;
			dataMap[toString(FLOAT_SPREAD_HEADER)] = floatSpreadVec;
			dataMap[toString(ACCRUAL_DAYCOUNT_HEADER)] = accrualDaycountVec;
			dataMap[toString(FORECAST_CURVE_HEADER)] = forecastCurveVec;
		}
        return dataMap;
    }

   std::unordered_set<CashflowHeaderEnum,EnumClassHash> FloatSchedule::allowedColumns() const
   {
	   std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
	   {
		   FIXING_DATE_HEADER
			,ACCRUAL_START_HEADER
			,ACCRUAL_END_HEADER
			,ACCRUAL_DAYS_HEADER
			,ACCRUAL_YEAR_FRACTIONS_HEADER
			,PAYMENT_DATE_HEADER
			,NOTIONAL_HEADER
			,NOTIONAL_EXCHANGE_HEADER
			,LEVERAGE_HEADER
			,FLOAT_SPREAD_HEADER
			,COUPON_MULTIPLIER_HEADER

		   // Bespoked cashflow fields
			, ACCRUAL_DAYCOUNT_HEADER
			, FORECAST_CURVE_HEADER
	   };

	   return expectedList;
   };



}
