#include "TotalReturnSwap.h"
#include "CDSLeg.h"
#include "PremiumLeg.h"
#include "CoreEnumerations.h"
#include "SwapUtilities.h"
#include "SwapCalculation.h"
#include "LWOUtilities.h"


namespace etrading
{

	TotalReturnSwap::TotalReturnSwap( const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB) : Swap(instanceName, TOTAL_RETURN_SWAP, swapPropertiesLVB ) 
	{
		validateLegs( leg1, leg2 );

        //Single Currency
        if ( leg1->getStaticData()->getCurrency() != leg2->getStaticData()->getCurrency() )
        {
  		    MLIB_THROW( "Only single currency swaps are supported." );
        }

        if (boost::math::isnan( leg1->getSchedule()->getNotional()) || boost::math::isnan(leg2->getSchedule()->getNotional() ))
        {
    		MLIB_THROW( "Notional not provided." );
        }

        inputParameters_ = swapPropertiesLVB;
        
        addToLegCollection( leg1 );
		addToLegCollection( leg2 );

    }

    TotalReturnSwap::TotalReturnSwap( const TotalReturnSwap& rhs ) : Swap( rhs )
    {}

    std::shared_ptr<Swap> TotalReturnSwap::clone()
    {
        SwapPtr swap = SwapPtr( new TotalReturnSwap( *this ));
        return swap;
    }

	void TotalReturnSwap::validateCreditModel( const CreditModel& creditModel ) const
	{
		auto firstLeg = legs_.get(0);
		CCY firstLegCurrency = firstLeg->getStaticData()->getCurrency();

		if ( firstLegCurrency != creditModel.getCurrency() )
		{
			MLIB_THROW( "Leg currency '" + toString( firstLegCurrency ) + "' does not match Credit Model currency '" + toString( creditModel.getCurrency() ) + "'." );
		}		
	}

	/* @brief Calculates the total PV of all of the Credit Default Swap legs.
	*
	* @param[in]	creditModel				The calibrated credit model
	* @param[in]	fixingTableNames			A map of fixing table names, indexed by legName
	* @param[in]	legName					Optionally calculate the PV of the specified leg only.
	* @returns	The calculated PV value
	*/
	double TotalReturnSwap::pv( const std::string& creditModelName, const LabelValueBlock& fixingTableNames, const std::string& legName) const
	{
		CreditModelPtr creditModel = getCreditModel( creditModelName );
		validateCreditModel( *creditModel );

		double pv = 0.0;

        if ( legName.size() == 0 )
        {
			// Calc PV of all legs
			for ( size_t i = 0; i < legs_.size(); ++i )
			{
				auto leg = legs_.get( i );

				// if CMS leg, call the alternative PV method with convexity adj.
				// If standard leg (fixed etc) call the
				if ( leg->getType() == PREMIUM_SCHEDULE_TYPE )
				{
					// OK to use static_pointer_cast here because we know the leg type for sure (the ConstantMaturitySwap created it).
					std::shared_ptr<CDSLeg> cdsLeg = std::static_pointer_cast<CDSLeg> ( leg );
					pv += cdsLeg->pv( *creditModel );

				}
				else
				{
					// regular leg type
					DataProvider dataProvider(ValuationSettings(*creditModel, fixingTableNames));
					pv += leg->pv( dataProvider);
				}
			}
        }
		else
		{
			// Search for the specified leg and calculate the PV of that leg only.
			bool legFound = false;
			for (size_t i = 0; i < legs_.size(); ++i)
			{
				auto leg = legs_.get(i);
				if ( same( leg->getLegName(), legName.c_str() ))
				{
					legFound = true;

					if ( leg->getType() == PREMIUM_SCHEDULE_TYPE )
					{
						// OK to use static_pointer_cast here because we know the leg type for sure (the ConstantMaturitySwap created it).
						std::shared_ptr<CDSLeg> cdsLeg = std::static_pointer_cast<CDSLeg> ( leg );
						pv = cdsLeg->pv( *creditModel );
					}
					else
					{
						// Regular leg type
						DataProvider dataProvider(ValuationSettings(*creditModel, fixingTableNames));
						pv = leg->pv( dataProvider );
					}

					break;
				}
			}
			if ( ! legFound )
			{
				MLIB_THROW( "Specified legName '" + legName + "' does not exist in the TotalReturnSwap." );
			}
		}

		return pv;
	}


	/* @brief Calculates the par rate of the premium leg of the total return swap
	*
	* @param[in]	creditModel				The calibrated credit model
	* @param[in]	fixingTableNames			A map of fixing table names, indexed by legName
	* @returns		The par rate of the TRS premium leg	
	*/
	double TotalReturnSwap::parRate( const std::string& creditModelName, const LabelValueBlock& fixingTableNames ) const
	{
		CreditModelPtr creditModel = getCreditModel( creditModelName );

		validateCreditModel( *creditModel );

        if(legs_.size() < 2)
		{
			throw LACoreInvalidData( "#Error: Expecting two legs to be present on the Total Return Swap", __FILE__, __LINE__ );
		}

		LegPtr leg1 = legs_.get(0);
		LegPtr leg2 = legs_.get(1);

		LegPtr targetLeg  = leg1->getType() == PREMIUM_SCHEDULE_TYPE ? leg1 : leg2;
		LegPtr againstLeg = leg1->getType() == PREMIUM_SCHEDULE_TYPE ? leg2 : leg1;

		// Spread formula: pvSpreadLegWithZeroSpread + pvSpreadLegSpread + pvRefLeg = 0 => pvSpreadLegSpread = -1 * (pvSpreadLegWithZeroSpread + pvAgainstLeg)

		const double pvAgainstLeg = pv( creditModelName, fixingTableNames, againstLeg->getLegName().getCString());

		// Check that the specified leg is a premium leg
		auto premiumLeg = std::dynamic_pointer_cast<PremiumLeg>( targetLeg );
		if ( premiumLeg == nullptr )
		{
			MLIB_THROW( "Specified leg '" + targetLeg->getLegName() + "' is not a CDS Premium Leg." );
		}

		// This simplified parRate calculation using annuity is only valid if
		// the accrual frequency is the same as the payment frequency
		if ( premiumLeg->getSchedule()->isAccrualFreqLessThanPaymentFreq() || premiumLeg->getSchedule()->isPaymentFreqEnumAtMaturity() )
		{
			MLIB_THROW( "Par-rate calculation for TRS trade does not support compounding cashflows. ");
		}

		// NOTE: the RiskyAnnuityWithNotional() calculation will also update cashflows on the premioum leg, setting discount factors
		// and Survival / Default probabilities. We make use of this before invoking getNotionalExchangePv() below.
		// 
		const double riskyAnnuityWithSign = premiumLeg->RiskyAnnuityWithNotional( *creditModel ) * premiumLeg->getSchedule()->getPayRecIndicator();

		DataProvider premiumLegDataProvider(ValuationSettings(*creditModel, fixingTableNames));

		premiumLeg->initializeDataProvider( premiumLegDataProvider);

		// Check if notionalExchange, and calc PV
		double pvPremiumLegZeroSpread = 0.0;
		if ( premiumLeg->getSchedule()->getNotionalExchangeEnum() != NONE_NE )
        {
            //get all the cashflows including the upfrontCashflow
            auto cashflows = premiumLeg->getSchedule()->getAllCashflows();

            for( size_t i = 0; i < cashflows.size(); i++ )
		    {			
			    auto cf = cashflows[i];
			    pvPremiumLegZeroSpread += cf->getNotionalExchangePv( premiumLegDataProvider.getCashflowDataIncludingUpfront( i ) );
		    }
        }

		double pvTotal = -1.0 * ( pvPremiumLegZeroSpread + pvAgainstLeg );

		const double parRate = pvTotal / riskyAnnuityWithSign;
		return parRate;
	}

	/* @brief Calculates the par spread of the float leg of the total return swap
	*
	* @param[in]	creditModel				The calibrated credit model
	* @param[in]	fixingTableNames			A map of fixing table names, indexed by legName
	* @returns		The par spread of the TRS float leg	
	*/
	double TotalReturnSwap::spread( const std::string& creditModelName, const LabelValueBlock& fixingTableNames) const
	{
		CreditModelPtr creditModel = getCreditModel( creditModelName );
		validateCreditModel( *creditModel );

		if(legs_.size() < 2)
		{
			MLIB_THROW( "Expecting two legs to be present on the Total Return Swap" );
		}

		LegPtr leg1 = legs_.get(0);
		LegPtr leg2 = legs_.get(1);

		// Here we put the spread on the non-premium leg i.e. the TRS float leg.
		LegPtr spreadLeg = leg1->getType() == PREMIUM_SCHEDULE_TYPE ? leg2 : leg1;
		LegPtr againstLeg    = leg1->getType() == PREMIUM_SCHEDULE_TYPE ? leg1 : leg2;

		if (spreadLeg->getType() != FLOAT_SCHEDULE_TYPE )
		{
			MLIB_THROW( "Par-Spread calculation is only supported for a TRS with a Float leg." );
		}

		// This simplified parRate calculation using annuity is only valid if
		// the accrual frequency is the same as the payment frequency
		if (spreadLeg->getSchedule()->isAccrualFreqLessThanPaymentFreq() || spreadLeg->getSchedule()->isPaymentFreqEnumAtMaturity() )
		{
			MLIB_THROW( "Par-spread calculation for TRS trade does not support compounding cashflows. ");
		}

		// Spread formula: pvSpreadLegWithZeroSpread + pvSpreadLegSpread + pvRefLeg = 0 => pvSpreadLegSpread = -1 * (pvSpreadLegWithZeroSpread + pvRefLeg)

		// Check that the reference leg is a premium leg
		auto premiumLeg = std::dynamic_pointer_cast<PremiumLeg>( againstLeg );
		if ( premiumLeg == nullptr )
		{
			MLIB_THROW( "Specified leg '" + againstLeg->getLegName() + "' is not a CDS Premium Leg." );
		}

		double pvAgainstLeg = premiumLeg->pv( *creditModel );

		DataProvider spreadLegDataProvider(ValuationSettings(*creditModel, fixingTableNames));

		//Always set the spread to 0 for the calculation, if it is not a parSpread function the original spread will be added back in the end 
		setSwapLegSpreadOrFixedRate(spreadLegDataProvider, spreadLeg->getType(), 0.);

		// calc pv of spread (float) leg, including any notional exchange
		double pvSpreadLegZeroSpread = spreadLeg->pv( spreadLegDataProvider);

		double pvTotal = -1.0 * (pvSpreadLegZeroSpread + pvAgainstLeg );

		double annuityWithSign = spreadLeg->annuityWithNotional(spreadLegDataProvider) * spreadLeg->getSchedule()->getPayRecIndicator();

		double spread = pvTotal / annuityWithSign * 1.0e4; // return the spread in bps
			
		return spread;
	}

	/* @brief Calculates the annity of the specified Total Return Swap leg
	*
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName			    Mandatory, calculate the annuity of this leg.
	* @returns	The calculated annuity value
	*/
	double TotalReturnSwap::annuity( const std::string& creditModelName, const std::string& legName ) const
	{

		if ( legName.size() == 0 ) {
           throw LACoreInvalidData( "#Error: Leg name must be provided", __FILE__, __LINE__ );
        }

        if ( !legs_.exists( legName.c_str() )) 
        {
            throw LACoreInvalidData( "#Error: Leg name does not exist in the swap", __FILE__, __LINE__ );
        }

		CreditModelPtr creditModel = getCreditModel( creditModelName );
		validateCreditModel( *creditModel );

		double annuity = 0.0;

		for (size_t i = 0; i < legs_.size(); ++i)
        {
            auto leg = legs_.get(i);
            if (same(leg->getLegName(), legName.c_str() ))
            {

				// if CMS leg, call the alternative annuity method with convexity adj.
				// If standard leg (fixed etc) call the
				if ( leg->getType() == PREMIUM_SCHEDULE_TYPE )
				{
					// OK to use static_pointer_cast here because we know the leg type for sure (the ConstantMaturitySwap created it).
					std::shared_ptr<PremiumLeg> premiumLeg = std::static_pointer_cast<PremiumLeg> ( leg );
					annuity = premiumLeg->RiskyAnnuityWithNotional( *creditModel );
					break;
				}
				else
				{
					// regular leg type
					DataProvider dataProvider(ValuationSettings(*creditModel, {}));

					annuity = leg->annuityWithNotional(dataProvider);

					break;
				}
            }
        }

		return annuity;
	}

}

