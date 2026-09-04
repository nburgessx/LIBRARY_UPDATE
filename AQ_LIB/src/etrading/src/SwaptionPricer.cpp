#include "SwaptionPricer.h"
#include "EuropeanIRSwaption.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "SwapUtilities.h"
#include "ExceptionMacros.h"
#include "ParameterValidation.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQOUtilities.h"
#include "SettingsValidation.h"

namespace etrading
{

    SwaptionPricer::SwaptionPricer(const std::shared_ptr<SwaptionTrade>& swaption, const AQLStringMatrix& valuationSettingsLVB )
		: swaptionTrade_( swaption ), valuationSettingsLVB_( valuationSettingsLVB )
	{
		AQ_REQUIRE( swaptionTrade_ != nullptr, "Invalid swaption trade" );
		

        // Set the CurveCollection from the Valuation Settings - Compatible with CurveCollection or LWO CurveObject input
        // =======================================================================================================
        curveCollection_  = getLWOCurveCollectionFromValuationSettings( valuationSettingsLVB ).getCString();


        // Set the Volatility Provider
        // =======================================================================================================
        const AQLString volObjectName = getVolatilityModelFromValuationSettings( valuationSettingsLVB );
        volProvider_ = getVolatility( volObjectName.getCString() ); // LWO Utility method to get the volatility object pointer from the LWO Cache


        // Validation: Ensure that Curve and Vol Model have the same Valuation Date and Currency
        // =======================================================================================================
        AQ_REQUIRE( volProvider_->asOfDate() == getCurveAsOfDate(curveCollection_.c_str()), "Inconsistent Model Valuation Dates - The yield curve and the volatility model asOfDates do not match" )
        AQ_REQUIRE( volProvider_->currency() == toCCYEnum( getCurveCurrency(curveCollection_.c_str()).getCString()), "Inconsistent Model Currencies - The yield curve and the volatility model currencies do not match" )
        AQ_REQUIRE( swaptionTrade_->currency() == volProvider_->currency(), "Invalid Volatility Model - The currency of the volatility model does not match that of the swaption trade" );
	}

	// Copy Constructor
    SwaptionPricer::SwaptionPricer( const SwaptionPricer& rhs) :
        swaptionTrade_( rhs.swaptionTrade_ ),
        valuationSettingsLVB_( rhs.valuationSettingsLVB_ ),
        curveCollection_( rhs.curveCollection_ ),
        volProvider_( rhs.volProvider_ )
	{}

    // Helper Method to Calculate a Discount Factor
    double SwaptionPricer::discountFactor( const AQLDate& paymentDate ) const
    {
        const std::string discountFactorCurveIndex = "OIS";
        const std::vector<double> discountFactors = etrading::getCurveDiscountFactors( std::vector<AQLDate>(1, paymentDate), curveCollection_.c_str(), discountFactorCurveIndex.c_str() );
        AQ_REQUIRE( discountFactors.size() > 0, "Invalid Discount Factor(s) - Discount factor results are empty")
        return discountFactors[0];
    }

    // Helper Method to Calculate Black-Scholes Parameters
    SwaptionPricer::BlackScholesParameters SwaptionPricer::calcualateBlackScholesParameters() const 
    {
        // Declare Return Parameter Struct;
        BlackScholesParameters bs;

        // Underlying Swap
        auto underlyingSwap                 = swaptionTrade_->underlyingSwap();
        bs.payerReceiver_                   = swaptionTrade_->payerReceiver();

        // Par Rate
		bs.parRate_                         = underlyingSwap->parRate( valuationSettingsLVB_, {} ); // Empty FixingTableMap since all underlying swaps are forward starting i.e. no fixings

        // Calculate the Annuity
        bs.annuityWithNotional_             = calculateAnnuityWithNotional( underlyingSwap, bs.parRate_ );

		// Calculate time to expiry
		const AQLDate valuationDate                  = volProvider_->asOfDate();
		const AQLDate adjustedOptionExpiryDate       = swaptionTrade_->adjustedOptionExpiryDate(); // Option Expiry Adjusted for Notification Lag
        const std::string notificationLag           = swaptionTrade_->notificationDays();
        AQ_REQUIRE( adjustedOptionExpiryDate >= valuationDate, "The swaption has expired! - OptionExpiryDate: " + adjustedOptionExpiryDate.stringWithFormat("DD-MM-YYYY") + ", NotificationDays: " + notificationLag.c_str() )

		const DayCountEnum optionDayCount   = swaptionTrade_->optionDayCount();

        bs.expiryYearFraction_              = getYearFraction(valuationDate, adjustedOptionExpiryDate, optionDayCount, false);
        bs.strike_                          = swaptionTrade_->strike();
		
		// Obtain swaption volatility
		bs.volatilityType_                  = volProvider_->volType();
		bs.vol_                             = volProvider_->vol();

        // Shifted-Lognormal Vol - Set the ShiftSize parameter when using Shifted-LogNormal Volatility
        bs.shiftSize_ = 0.0;
        if ( volProvider_->volType() == SHIFTED_LOGNORMAL_VOLATILITY )
        {
            bs.shiftSize_ = volProvider_->shiftSize();
        }

        // Other Helpful Parameters
        bs.longShortIndicator_              = ( swaptionTrade_->longShort() == LONG_POSITION ) ? 1 : -1;
        bs.leverage_                        = swaptionTrade_->leverage();

        return bs;
    }

	/* @brief	Calculates the swaption PV.
	*           Uses the parameters supplied during object construction.
	*/
	const double SwaptionPricer::pv() const
	{
        // Calculate BlackScholes Parameters
        const BlackScholesParameters bs = calcualateBlackScholesParameters();

        // Calculate swaptionPV
		const double blackScholesPV     = EuropeanIRSwaption::price( bs.payerReceiver_, bs.annuityWithNotional_, bs.parRate_, bs.strike_, bs.vol_, bs.expiryYearFraction_, bs.shiftSize_, bs.volatilityType_ );
        const double swaptionPv         = blackScholesPV * bs.leverage_ * bs.longShortIndicator_;
		        
        // Swaption Fee
        const double feePV              = calculateFeePV();

        // Swaption PV + Fee PV
        return swaptionPv + feePV;
	}


     /* @brief	Calculates the swaption delta.
	*           Uses the parameters supplied during object construction.
	*/
	const double SwaptionPricer::impliedVol(const double & price ) const
	{
        // Calculate BlackScholes Parameters
        const BlackScholesParameters bs = calcualateBlackScholesParameters();

        // Construct Swaption 
        EuropeanIRSwaption swaption     = EuropeanIRSwaption( bs.payerReceiver_, bs.annuityWithNotional_, bs.parRate_, bs.strike_, bs.vol_, bs.expiryYearFraction_, bs.shiftSize_, bs.volatilityType_ );
        
        // Remove the Fee from the Swaption Price if Present
        const double feePV              = calculateFeePV();

        // Derive the Implied Vol
        const double impliedVol         = swaption.calculateImpliedVol( price - feePV );
		return impliedVol;
	}


    /* @brief	Calculates the swaption delta.
	*           Uses the parameters supplied during object construction.
	*/
	const double SwaptionPricer::delta() const
	{
        // Calculate BlackScholes Parameters
        const BlackScholesParameters bs = calcualateBlackScholesParameters();

        // Calculate swaption Delta
		const double blackScholesDelta  = EuropeanIRSwaption::delta( bs.payerReceiver_, bs.annuityWithNotional_, bs.parRate_, bs.strike_, bs.vol_, bs.expiryYearFraction_, bs.shiftSize_, bs.volatilityType_ );
        const double swaptionDelta      = blackScholesDelta * bs.leverage_ * bs.longShortIndicator_;
		
        // Calculate annuity Delta
        const double annuityDelta       = calculateAnnuityDelta( bs );

        // Calculate fee Delta
        const double feeDelta           = calculateFeeDelta();
        
        // Delta  = SwaptionDelta + FeeDelta
        return swaptionDelta + annuityDelta + feeDelta;
	}


    /* @brief	Calculates the swaption gamma.
	*           Uses the parameters supplied during object construction.
	*/
	const double SwaptionPricer::gamma() const
	{
        // Calculate BlackScholes Parameters
        const BlackScholesParameters bs = calcualateBlackScholesParameters();

        // Calculate swaption Gamma
		const double blackScholesGamma  = EuropeanIRSwaption::gamma( bs.payerReceiver_, bs.annuityWithNotional_, bs.parRate_, bs.strike_, bs.vol_, bs.expiryYearFraction_, bs.shiftSize_, bs.volatilityType_ );
        const double swaptionGamma      = blackScholesGamma * bs.leverage_ * bs.longShortIndicator_;
		
        // Calculate annuity Gamma
        const double annuityGamma       = calculateAnnuityGamma( bs );

        // Calculate fee Gamma
        const double feeGamma           = calculateFeeGamma();
        
        // Gamma  = SwaptionGamma + FeeGamma
        return swaptionGamma + annuityGamma + feeGamma;
	}


    /* @brief	Calculates the swaption vega.
	*           Uses the parameters supplied during object construction.
	*/
	const double SwaptionPricer::vega() const
	{
        // Calculate BlackScholes Parameters
        const BlackScholesParameters bs = calcualateBlackScholesParameters();

        // Calculate swaption Vega
		const double blackScholesVega   = EuropeanIRSwaption::vega( bs.payerReceiver_, bs.annuityWithNotional_, bs.parRate_, bs.strike_, bs.vol_, bs.expiryYearFraction_, bs.shiftSize_, bs.volatilityType_ );
        const double swaptionVega       = blackScholesVega * bs.leverage_ * bs.longShortIndicator_;
		
        return swaptionVega;
	}


    /* @brief	Calculates the swaption theta.
	*           Uses the parameters supplied during object construction.
	*/
	const double SwaptionPricer::theta() const
	{
        // Calculate BlackScholes Parameters
        const BlackScholesParameters bs = calcualateBlackScholesParameters();

        // Calculate swaption Vega
		const double blackScholesTheta  = EuropeanIRSwaption::theta( bs.payerReceiver_, bs.annuityWithNotional_, bs.parRate_, bs.strike_, bs.vol_, bs.expiryYearFraction_, bs.shiftSize_, bs.volatilityType_ );
        const double swaptionTheta      = blackScholesTheta * bs.leverage_ * bs.longShortIndicator_;
		
        // Calculate annuity Theta
        const double annuityTheta       = calculateAnnuityTheta( bs );

        // Calculate fee Theta
        const double feeTheta           = calculateFeeTheta();
        
        // Theta  = SwaptionTheta + FeeTheta
        return swaptionTheta + annuityTheta + feeTheta;
	}


    /* @brief	Calculates the Notional Scaled Annuity for Cash, Par-Yield and Physical Settlement Types
	*/
	double SwaptionPricer::calculateAnnuityWithNotional( const SwapPtr& underlyingSwap, const double parRate ) const
	{
		double annuityWithNotional = std::numeric_limits<double>::quiet_NaN();

		// Determine which is the Fixed Leg of the swap
		const ScheduleTypeEnum leg0Type = underlyingSwap->getLeg(0)->getType();
		const ScheduleTypeEnum leg1Type = underlyingSwap->getLeg(1)->getType();
		if ( ( leg0Type != FIXED_SCHEDULE_TYPE ) && ( leg1Type != FIXED_SCHEDULE_TYPE ) )
		{
			AQ_THROW( "Swaption underlying swap instrument does not contain a FIXED_SCHEDULE_TYPE leg.");
		}

		const size_t fixedLegIdx = ( leg0Type == FIXED_SCHEDULE_TYPE ) ? 0 : 1;

		const SettlementTypeEnum swaptionSettlementType = swaptionTrade_->settlementType();
		switch( swaptionSettlementType )
		{
			// Cash and Physical Settlement Prices are the Same
            // Note there are two cash settlement methods namely cash and cash Par-yield. Cash Par Yield prices differently having a different annuity measure.
            case PHYSICAL_SETTLEMENT:
			case CASH_PRICE_SETTLEMENT:
			{
				const AQLString& fixedLegName = underlyingSwap->getLeg( fixedLegIdx )->getLegName();
		
				annuityWithNotional = underlyingSwap->annuity( valuationSettingsLVB_, fixedLegName );
				break;
			}
			case CASH_PAR_YIELD_SETTLEMENT:
			{
				const double notional                   = swaptionTrade_->notional();
				const LegPtr fixedLeg                   = underlyingSwap->getLeg( fixedLegIdx );
				const SchedulePtr fixedSchedule         = fixedLeg->getSchedule();
				const FrequencyEnum accrualFrequency    = fixedSchedule->getAccrualFrequency();
				const size_t nCouponsPerYear            = convertFrequenyToCouponsPerYear( accrualFrequency );
				const double swapTenorInYears           = fixedSchedule->getYearFractionFromEffectiveToMaturityDates();

				// The Par Yield Cash Annuity is a market approximation, which uses the swap rate as a yield to discount the swap flows from swap end to swap start. 
                annuityWithNotional = EuropeanIRSwaption::cashAnnuity( notional, parRate, nCouponsPerYear, swapTenorInYears );
                
                // Note we must discount until the Trade Valuation Date, since the cash annuity is only discounted to the swap start date
                annuityWithNotional *= discountFactor( swaptionTrade_->adjustedOptionExpiryDate() );

				break;
			}
			default:
            {
				AQ_THROW("UnSupported Swaption SettlementType: " + toString( swaptionSettlementType ) );
            }
		}

		return annuityWithNotional;
	}

    // Helper Method to Calculate the Swaption Fee PV if any
    double SwaptionPricer::calculateFeePV() const
    {
        double feePV = 0.0;
        const double feePayRecIndicator = swaptionTrade_->feePayReceiveIndicator();
        const double fee                = swaptionTrade_->fee() * feePayRecIndicator;
        
        if ( !AQ_IS_EQUAL_ZERO( fee ) ) 
        {
            // Fees in the past have zero PV
            const AQLDate feeDate        = swaptionTrade_->feeDate();
            double feeDiscountFactor    = 0.0;
            
            const AQLDate valuationDate  = volProvider_->asOfDate();
            if ( feeDate >= valuationDate )
            {
                // Get Fee Discount Factor
                feeDiscountFactor       = discountFactor( feeDate );
            }
            
            feePV = fee * feeDiscountFactor;
        }

        return feePV;
    }

    // Calculate Fee Delta, if any
    double SwaptionPricer::calculateFeeDelta() const
    {
        double feeDelta                         = 0.0;
        const double basisPointShiftSize        = -0.0001;  // Note: Negative Shift is Market Convention

        const double fee                        = swaptionTrade_->fee();

        if ( !AQ_IS_EQUAL_ZERO( fee ) )
        {
            // Calculate BlackScholes Parameters
            const AQLDate valuationDate          = volProvider_->asOfDate();
		    const AQLDate feeDate                = swaptionTrade_->feeDate();
            const DayCountEnum optionDayCount   = swaptionTrade_->optionDayCount();
            const double feeYearFraction        = getYearFraction( valuationDate, feeDate, optionDayCount, false );

            const double feePV                  = calculateFeePV();
            // PV( fee )    = cashflow * discFactor 
            //              = cashflow * exp( - z * t )
            // Therefore 
            // Delta( fee ) = cashflow * - t * exp( - z * t ) * bumpSize 
            //              = - PV( fee ) * t * bumpSize
            feeDelta                            = feePV * -1.0 * feeYearFraction * basisPointShiftSize;
        }

        return feeDelta;
    }

    // Calculate Fee Gamma, if any
    double SwaptionPricer::calculateFeeGamma() const
    {
        double feeGamma                         = 0.0;
        const double basisPointShiftSize        = -0.0001;  // Note: Negative Shift is Market Convention

        const double fee                        = swaptionTrade_->fee();

        if ( !AQ_IS_EQUAL_ZERO( fee ) )
        {
            // Calculate BlackScholes Parameters
            const AQLDate valuationDate          = volProvider_->asOfDate();
		    const AQLDate feeDate                = swaptionTrade_->feeDate();
            const DayCountEnum optionDayCount   = swaptionTrade_->optionDayCount();
            const double feeYearFraction        = getYearFraction( valuationDate, feeDate, optionDayCount, false );

            const double feePV                  = calculateFeePV();
            // PV( fee )    = cashflow * discFactor 
            //              = cashflow * exp( - z * t )
            // Therefore
            // Gamma( fee ) = cashflow * ( - t ) * ( - t ) * exp( - z * t ) * bumpSize * bumpSize 
            //              = PV( fee ) * t * t * bumpSize * bumpSize
            // Note: Gamma is Delta of Delta and hence our result has square of shiftSize
            feeGamma                            = feePV * feeYearFraction * feeYearFraction * basisPointShiftSize * basisPointShiftSize;
        }

        return feeGamma;
    }

    // Calculate Fee Theta, if any
    double SwaptionPricer::calculateFeeTheta() const
    {
        double feeTheta                         = 0.0;
        const double feePayRecIndicator         = swaptionTrade_->feePayReceiveIndicator();
        const double fee                        = swaptionTrade_->fee() * feePayRecIndicator;
        
        if ( !AQ_IS_EQUAL_ZERO( fee ) ) 
        {
            // Fees in the past have zero PV
            const AQLDate feeDate                = swaptionTrade_->feeDate();
            
            const AQLDate valuationDate          = volProvider_->asOfDate();
            if ( feeDate >= valuationDate )
            {
                // Discount Factor on Valuation Date
                double todayDiscountFactor       = discountFactor( feeDate );
                
                // Discount Factor on Valuation Date + 1
                AQLDate todayPlusOne             = valuationDate;
                todayPlusOne.addDays(1);
                
                double oneDayDiscountFactor     = discountFactor( todayPlusOne );
                AQ_REQUIRE ( AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( oneDayDiscountFactor), "Unable to calculate Theta - Discount Factor cannot be zero or negative" )
                
                double tomorrowDiscountFactor   = todayDiscountFactor / oneDayDiscountFactor;
                feeTheta                        = fee * ( tomorrowDiscountFactor - todayDiscountFactor );
            }
            else
            {
                // fee is in the past therefore return zero
                return 0.0;
            }
        }

        return feeTheta;
    }

    // Calculate Annuity Delta
    double SwaptionPricer::calculateAnnuityDelta( const BlackScholesParameters & bs ) const
    {
        // Calculate Annuity Delta: -Duration(Fixed) * Notional * Annuity(Fixed) * UnitSwaptionPayoff * DeltaShiftSize ( -1 Bps )
        double unitSwaptionPayoff       = EuropeanIRSwaption::price( bs.payerReceiver_, 1.0, bs.parRate_, bs.strike_, bs.vol_, bs.expiryYearFraction_, bs.shiftSize_, bs.volatilityType_ );
        unitSwaptionPayoff              = unitSwaptionPayoff * bs.leverage_ * bs.longShortIndicator_;
        
        //TODO: Improve approximation of duration term; approx error roughly USD 3 per USD 10MM trade notional for ATM Swaptions
        const double swapTenor          = getYearFraction( swaptionTrade_->swapStartDate(), swaptionTrade_->swapMaturityDate(), etrading::E30_360_DAYCOUNT, false );
        const double duration           = ( swapTenor * 0.5 ) * std::pow( 1 + bs.parRate_, -1.0 * ( swapTenor * 0.5 ) );
        
        const double deltaShiftSize     = -0.0001; // Negative 1 bps shift
        const double annuityDelta       = duration * bs.annuityWithNotional_ * unitSwaptionPayoff * deltaShiftSize;

        return annuityDelta;
    }

    // Calculate Annuity Gamma
    double SwaptionPricer::calculateAnnuityGamma( const BlackScholesParameters & bs ) const
    {
        // Calculate Annuity Gamma: Duration(Fixed) * Duration(Fixed) * Notional * Annuity(Fixed) * UnitSwaptionPayoff * GammaShiftSize ( -1 Bps )
        double unitSwaptionPayoff       = EuropeanIRSwaption::price( bs.payerReceiver_, 1.0 , bs.parRate_, bs.strike_, bs.vol_, bs.expiryYearFraction_, bs.shiftSize_, bs.volatilityType_ );
        unitSwaptionPayoff              = unitSwaptionPayoff * bs.leverage_ * bs.longShortIndicator_;
        
        //TODO: Improve approximation of duration term; approx error roughly USD 0.10 per USD 10MM trade notional for ATM Swaptions
        const double swapTenor          = getYearFraction( swaptionTrade_->swapStartDate(), swaptionTrade_->swapMaturityDate(), etrading::E30_360_DAYCOUNT, false );
        const double duration           = ( swapTenor * 0.5 ) * std::pow( 1 + bs.parRate_, -1.0 * ( swapTenor * 0.5 ) );
        
        const double gammaShiftSize     = -0.0001; // Negative 1 bps shift
        const double annuityGamma       = duration * duration * bs.annuityWithNotional_ * unitSwaptionPayoff * gammaShiftSize * gammaShiftSize;

        return annuityGamma;
    }

    // Calculate Annuity Theta
    double SwaptionPricer::calculateAnnuityTheta( const BlackScholesParameters & bs ) const
    {
        // TODO:
        return 0.0;
    }

}


