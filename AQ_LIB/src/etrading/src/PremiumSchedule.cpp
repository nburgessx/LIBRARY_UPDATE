#include "AQObjUtilities.h"
#include "PremiumSchedule.h"
#include "PremiumCashflow.h"
#include "CurveUtilities.h"
#include "CurveValidation.h"
#include "AQLDateScheduleHelpers.h"
#include "SwapUtilities.h"
#include "ExceptionMacros.h"

#include <memory>

namespace etrading
{

	PremiumSchedule::PremiumSchedule(const std::string& instanceName) : Schedule(instanceName)
	{
        scheduleType_ = PREMIUM_SCHEDULE_TYPE;
    }

	PremiumSchedule::PremiumSchedule(const LabelValueBlock& scheduleLVB, const std::string& instanceName) : Schedule(instanceName)
	{

		//----------
		const std::string inputLVB = "scheduleLVB";

   		scheduleType_ = PREMIUM_SCHEDULE_TYPE;

        // Create new LVB with schedule type
        inputParameters_ = LabelValueBlock( inputParameters_, IRS_KEY::SCHEDULE_TYPE, toString(scheduleType_) );

		payerReceiver_                          = toPayReceiveEnum(scheduleLVB.getCompulsoryValueAsAQLStringFromKeys(IRS_KEY::PAY_RECEIVE, IRS_KEY::PAYER_RECEIVER, inputLVB).getCString());

		notionalExchangeEnum_                   = toNotionalExchangeEnum( scheduleLVB.getOptionalValueAsAQLString( IRS_KEY::NOTIONAL_EXCHANGE, "NONE" ).getCString() );

		leverage_                               =  scheduleLVB.getOptionalValueAsDouble(IRS_KEY::LEVERAGE, 1.);

        accrualEndDateOrTenor_                  = scheduleLVB.getCompulsoryValueAsAQLString( IRS_KEY::MATURITY_DATE,  inputLVB );

		AQLString fixedBusinessDayAdjustment     = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys( boost::assign::list_of(IRS_KEY::FIXED_BUSINESSDAYADJUSTMENT)(IRS_KEY::BUSINESSDAYADJUSTMENT) );
        AQLString fixedCalendar	                = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys( boost::assign::list_of(IRS_KEY::FIXED_CALENDAR)(IRS_KEY::CALENDAR) );
		AQLString fixedLegFreq	                = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys( boost::assign::list_of(IRS_KEY::FIXED_FREQUENCY)(IRS_KEY::FREQUENCY) );
        AQLString fixedDayCount	                = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys( boost::assign::list_of(IRS_KEY::FIXED_DAYCOUNT)(IRS_KEY::DAYCOUNT) );

        accrualbusinessDayAdj_	                = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_ACCRUALBUSINESSDAYADJUSTMENT,		IRS_KEY::ACCRUALBUSINESSDAYADJUSTMENT,	fixedBusinessDayAdjustment).getCString());
        accrualCalendar_		                = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_ACCRUALCALENDAR,					IRS_KEY::ACCRUALCALENDAR,				fixedCalendar);
        accrualFrequency_		                = toFrequencyEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_ACCRUALFREQUENCY,					IRS_KEY::ACCRUALFREQUENCY,				fixedLegFreq).getCString());
		accrualDaycount_		                = toDayCountEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_ACCRUALDAYCOUNT,					IRS_KEY::ACCRUALDAYCOUNT,				fixedDayCount).getCString());

        auto defaultBusinessDayAdj              = fixedBusinessDayAdjustment.size() != 0 ? fixedBusinessDayAdjustment : toString(accrualbusinessDayAdj_).c_str();
        auto defaultCalendar                    = fixedCalendar.size() != 0 ? fixedCalendar : accrualCalendar_;
        auto defaultFrequency                   = fixedLegFreq.size() != 0 ? fixedLegFreq : toString(accrualFrequency_).c_str();

		paymentbusinessDayAdj_	                = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_PAYMENTBUSINESSDAYADJUSTMENT,		IRS_KEY::PAYMENTBUSINESSDAYADJUSTMENT,	defaultBusinessDayAdj).getCString());
        paymentCalendar_		                = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_PAYMENTCALENDAR,					IRS_KEY::PAYMENTCALENDAR,				defaultCalendar );
        paymentFrequency_		                = toFrequencyEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_PAYMENTFREQUENCY,					IRS_KEY::PAYMENTFREQUENCY,				defaultFrequency).getCString());
       
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

		firstStub_				                = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys( firstStubKeys );
        lastStub_		                        = scheduleLVB.getOptionalValueAsAQLStringFromMultipleKeys( lastStubKeys );

        rollDayInput_	                        = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_ROLLDAY,							IRS_KEY::ROLLDAY );
        payLag_					                = scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_PAYMENTLAG,							IRS_KEY::PAYMENTLAG,					"0D");
        stubType_			                    = toStubTypeEnum(scheduleLVB.getOptionalValueAsAQLStringFromKeys(IRS_KEY::FIXED_STUBTYPE,			IRS_KEY::STUBTYPE).getCString());

        //fxFixing
        fxFixingLag_                            = scheduleLVB.getOptionalValueAsAQLString( IRS_KEY::FXFIXINGLAG, "0D" );
		fxFixingbusinessDayAdj_                 = toBusinessDayAdjustmentEnum(scheduleLVB.getOptionalValueAsAQLString(IRS_KEY::FXFIXINGBUSINESSDAYADJUSTMENT, defaultBusinessDayAdj).getCString() );
        fxFixingCalendar_                       = scheduleLVB.getOptionalValueAsAQLString(IRS_KEY::FXFIXINGCALENDAR, defaultCalendar);

        populateAccrualStartDates(scheduleLVB);

        //Given the effectiveDate (accrualStartDate) and maturityDate(accrualEndDateOrTenor), calculate the accrualStartDates, accrualEndDates, paymentDates, fixingDates
		Schedule::calculateScheduleDates();

		// Total Return Swap performance attributes
		assetPerformanceEnum_ = toTRSAssetPerformanceEnum( scheduleLVB.getOptionalValueAsString( TRS_KEY::ASSET_PERFORMANCE ) );
		AQ_REQUIRE( assetPerformanceEnum_ == NO_PERFORMANCE || assetPerformanceEnum_ == PAY_AT_MATURITY_PERFORMANCE, "TRS currently only supports an AssetPerformance value of 'None' or'PayAtMaturity'." );
		if ( assetPerformanceEnum_ != NO_PERFORMANCE )
		{
			bondBaseDirtyPriceFixing_ = scheduleLVB.getCompulsoryValueAsDouble( IRS_KEY::FIRSTFIXING );

			bondName_                 = scheduleLVB.getCompulsoryValueAsString( TRS_KEY::BOND_NAME, inputLVB );
			auto bond = getBond( bondName_ );
			const AQLDate bondMaturityDate = bond->getSchedule()->getMaturityDate();

			// Check that bond does not mature before the swap leg schedule.
			// Note that we cannot invoke getMaturityDate() because the schedule is not yet fully initialized
			const size_t nAccrualEndDates = accrualEndDates_.size();
			AQ_REQUIRE( nAccrualEndDates > 0, "TRS premium leg does not contain any accrual periods.");
			const AQLDate scheduleEndDate = accrualEndDates_[ nAccrualEndDates - 1];
			AQ_REQUIRE( bondMaturityDate >= scheduleEndDate, "Bond " + bondName_ + " has a maturity earlier than the TRS premium leg maturity." );

			const std::string cdsSpreadAsString = scheduleLVB.getOptionalValueAsString( CDS_KEY::CDS_SPREAD );
			if ( cdsSpreadAsString == "" )
			{
				// Set the coupon rate for the Premium Schedule equal to the bond coupon rate
				cdsSpread_ = bond->getSchedule()->getFixedRate();
			}
			else
			{
				// If CDS_SPREAD is provided, use this as an override to the bond coupon
				cdsSpread_ = scheduleLVB.getCompulsoryValueAsDouble( CDS_KEY::CDS_SPREAD, inputLVB );	
			}
		}
		else
		{
			cdsSpread_ = scheduleLVB.getCompulsoryValueAsDouble( CDS_KEY::CDS_SPREAD, inputLVB );	
		}

		//Populate the cashflows based on schedule inputs
		createCashflows();
	}

    SchedulePtr PremiumSchedule::clone()
    {
        SchedulePtr sch = SchedulePtr( new PremiumSchedule( *this ));
        return sch;
    }

	PremiumSchedule::PremiumSchedule(const PremiumSchedule& rhs) : Schedule( rhs )
	{};

    void PremiumSchedule::populateNotionalAndPaymentFreqEnum(const LabelValueBlock& scheduleLVB)
    {
		Schedule::populateNotionalAndPaymentFreqEnum();

		if (scheduleLVB.size() == 0)
		{
			return;
		}

		notional_ = scheduleLVB.getOptionalValueAsDoubleFromKeys( IRS_KEY::NOTIONAL, BOND_KEY::FACE_VALUE, std::numeric_limits<double>::quiet_NaN() ); 

		if (boost::math::isnan(notional_))
        {
        	throw AQLCoreInvalidData( "#Error: Notional is a mandatory field for PremiumSchedule", __FILE__, __LINE__ );
        }
    }

	void PremiumSchedule::createUpfrontCashflow( const AQLDate& paymentDate, double leverage ) 
    {
        if ( notionalExchangeEnum_ == START_NE || notionalExchangeEnum_ == START_AND_END_NE )
        {
            auto nanDoubleValue = std::numeric_limits<double>::quiet_NaN();

            upfrontCashflow_= CashflowPtr( new PremiumCashflow( payerReceiver_, nanDoubleValue, AQLDate(), AQLDate(), 0, nanDoubleValue, paymentDate, nanDoubleValue, leverage, paymentFreqEnum_, FIRST_NOTIONAL_EXCHANGE_CASHFLOW_TYPE));
            upfrontCashflow_->setFwdFxRate( nanDoubleValue );
    	}
    }

    void PremiumSchedule::createCashflows() 
    {
		// Use the first accrualStartDate as the paymentDate
        auto paymentDate = accrualStartDates_.at(0);
        createUpfrontCashflow( paymentDate, leverage_ );

        auto cashflowSize = accrualStartDates_.size();

        std::vector<double> cashflowNotionals = getCashflowNotionals( cashflowSize ); 

        for (size_t i=0; i < cashflowSize; ++i)
		{
			auto cashflowType = (i == cashflowSize - 1) ? NORMAL_LAST_CASHFLOW_TYPE : NORMAL_CASHFLOW_TYPE;

            double cashflowNotional = cashflowNotionals[i];

			CashflowPtr cf = CashflowPtr( new PremiumCashflow( payerReceiver_,
																cdsSpread_,
																accrualStartDates_[i],
																accrualEndDates_[i],
																accrualDays_[i],
																accrualYearFractions_[i],
																paymentDates_[i],
																cashflowNotional,
																leverage_,
																paymentFreqEnum_,
																cashflowType));
            cashflows_.push_back(cf);
		}
		
		// Update notional exchanges based on notionals of cashflows
        updateNotionalExchange();

    }

	double PremiumSchedule::getCDSSpread() const
	{
		return cdsSpread_;
	}

	void PremiumSchedule::setCDSSpread( double cdsSpread )
	{
		cdsSpread_ = cdsSpread;
	}


	/* @brief Updates the cashflow survival / default probabilities using the hazard rate and recovery rate parameters
	*
	* @param[in]	asOfDate				The valuation date of the leg
	* @param[in]	hazardRate				The CDS hazard rate parameter, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	includeAccruedInterest	Specifies whether cashflows should include the accruedInterest
	*/
	void PremiumSchedule::setSurvivalProbabilitiesUsingHazardRate( const AQLDate& asOfDate, const double hazardRate, const double recoveryRate, const bool includeAccruedInterest )
	{
		double prevSurvivalProbability = 1.0;
		double survivalProbability = 1.0;
		AQLDate prevPaymentDate = asOfDate;

		// Calculate and set survival / default probabilities for each cashflow
		size_t cashflowSize = cashflows_.size();
		for (size_t i=0; i<cashflowSize; i++ )
		{
			CashflowPtr curCashflow = cashflows_[i];

			// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
			std::shared_ptr<PremiumCashflow> premiumCashflow = std::static_pointer_cast<PremiumCashflow>( curCashflow );

			premiumCashflow->setIncludeAccruedInterest( includeAccruedInterest );

			const AQLDate& paymentDate = paymentDates_[i];
			double paymentYearFraction = getYearFraction(prevPaymentDate, paymentDate, accrualDaycount_, false);	

			const double survivalFactor = exp( -hazardRate * paymentYearFraction );
			survivalProbability *= survivalFactor;
			premiumCashflow->setSurvivalProbability( survivalProbability );

			double marginalDefaultProbability = prevSurvivalProbability - survivalProbability;
			premiumCashflow->setMarginalDefaultProbability( marginalDefaultProbability );

			premiumCashflow->setPerformance( 0.0 );

			prevSurvivalProbability = survivalProbability;
			prevPaymentDate = paymentDate;
		}
	}

	/* @brief Updates the cashflow survival / default probabilities using the provided credit model.
	*
	* @param[in]	asOfDate				The valuation date of the leg
	* @param[in]	creditModel				The calibrated credit model
	*/
	void PremiumSchedule::setSurvivalProbabilitiesUsingCreditModel( const AQLDate& asOfDate, const CreditModel& creditModel )
	{
		const bool includeAccruedInterest = creditModel.getIncludeAccruedInterest();

		// Calculate and set survival / default probabilities for each cashflow
		double prevSurvivalProbability = creditModel.getSurvivalProbability( effectiveDate_ );

		const size_t cashflowSize = cashflows_.size();
		for (size_t i=0; i<cashflowSize; i++ )
		{
			CashflowPtr curCashflow = cashflows_[i];

			// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
			std::shared_ptr<PremiumCashflow> premiumCashflow = std::static_pointer_cast<PremiumCashflow>( curCashflow );

			premiumCashflow->setIncludeAccruedInterest( includeAccruedInterest );

			const AQLDate& paymentDate = paymentDates_[i];
			const double survivalProbability = creditModel.getSurvivalProbability( paymentDate );
			premiumCashflow->setSurvivalProbability( survivalProbability );

			double marginalDefaultProbability = prevSurvivalProbability - survivalProbability;
			premiumCashflow->setMarginalDefaultProbability( marginalDefaultProbability );

			premiumCashflow->setPerformance( 0.0 );

			prevSurvivalProbability = survivalProbability;
		}
	}

	/* @brief	Get the Asset Performance calculation type
	*			i.e. wether the underlying asset performance should be included in the cashflow payments
	*/
	TRSAssetPerformanceEnum PremiumSchedule::getAssetPerformanceEnum() const
	{
		return assetPerformanceEnum_;
	}

	/* @brief Updates the Schedule cashflows to include the Total-Return-Swap performance of the underlying asset.
	*  @param[in]	creditModel		The credit model used to obtain survival and default probabilities
	*  @param[in]	paymentTrigger	Specifies whether the cashflows have credit risk: i.e. do they pay always, or pay on survival	
	*/
	void PremiumSchedule::updateTRSPerformance( const CreditModel& creditModel, const PaymentTriggerEnum& paymentTrigger )
	{
		switch ( assetPerformanceEnum_ )
		{
			case PAY_AT_MATURITY_PERFORMANCE:
			{
				auto bond = getBond( bondName_ );
				const double faceValue = bond->getSchedule()->getNotional();
				const double recoveryRate = creditModel.getRecoveryRate();
				const double recoveryAmount = recoveryRate * faceValue;

				const size_t cashflowSize = cashflows_.size();
				for (size_t i=0; i<cashflowSize; i++ )
				{
					CashflowPtr curCashflow = cashflows_[i];

					// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
					std::shared_ptr<PremiumCashflow> premiumCashflow = std::static_pointer_cast<PremiumCashflow>( curCashflow );


					// Calculate the asset performance in the event of a default
					const double performanceOnDefault = ( recoveryAmount - bondBaseDirtyPriceFixing_ ) / bondBaseDirtyPriceFixing_ * premiumCashflow->getMarginalDefaultProbability();
					double performance = performanceOnDefault;

					// Calculate the pull-to-par performance at maturity
					if ( premiumCashflow->isLastCashflow() )
					{
						const AQLDate forwardSettlementDate = getMaturityDate(); // *** TODO: Any lag on this date?
						
						double bondForwardDirtyPrice = 0.0;
						switch ( paymentTrigger )
						{
							case PAY_ON_SURVIVAL:
							{
								bondForwardDirtyPrice = bond->forwardDirtyPriceFromCreditModel( forwardSettlementDate, creditModel );
								break;
							}
							case PAY_ALWAYS:
							{
								const std::string curveCollection = creditModel.getBondCurveCollection();
								const std::string discountCurve = creditModel.getBondDiscountCurve();
								bondForwardDirtyPrice = bond->forwardDirtyPriceFromDiscountCurve( forwardSettlementDate, curveCollection, discountCurve );
								break;
							}
							default:
								AQ_THROW( "Unsupported PaymentTrigger type: " + toString( paymentTrigger ));
								break;
						}
						
						const double pullToParPerformance = ( bondForwardDirtyPrice - bondBaseDirtyPriceFixing_ ) / bondBaseDirtyPriceFixing_;
						performance += pullToParPerformance;
					}
					premiumCashflow->setPerformance( performance );
				}
				break;
			}
			case NO_PERFORMANCE:
			{
				const size_t cashflowSize = cashflows_.size();
				for (size_t i=0; i<cashflowSize; i++ )
				{
					CashflowPtr curCashflow = cashflows_[i];

					// OK to use static_pointer_cast here because we know the cashflow type for sure (the schedule created it).
					std::shared_ptr<PremiumCashflow> premiumCashflow = std::static_pointer_cast<PremiumCashflow>( curCashflow );
					premiumCashflow->setPerformance( 0.0 );
				}
				break;
			}
			default:
				AQ_THROW( "Unsupported TRS Performance type: " + toString( assetPerformanceEnum_ ));
		}
	}


    const DataSchema PremiumSchedule::generateCashflowSchema(const std::string& schemaName) const
    {
		return Schedule::generateCashflowSchema( schemaName );
    }

    std::map<std::string, std::vector<std::string>> PremiumSchedule::getCashflowDataMap() const
    {
		return Schedule::getCashflowDataMap();
    }

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> PremiumSchedule::allowedColumns() const
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
			,CDS_SPREAD_HEADER
		};

		return expectedList;
	}


}
