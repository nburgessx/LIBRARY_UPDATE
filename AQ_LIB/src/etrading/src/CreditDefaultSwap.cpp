#include "CreditDefaultSwap.h"
#include "PremiumLeg.h"
#include "CoreEnumerations.h"
#include "SwapUtilities.h"
#include "Solvers.h"
#include "SettingsValidation.h"
#include "SimpsonsRuleIntegration.h"
#include "RandomNumberGenerator.h"
#include "MonteCarloEngine.h"
#include "SurvivalPath.h"
#include "NormalDistribution.h"
#include "ql/math/integrals/gaussianquadratures.hpp"
#include "Swap.h"

#include <omp.h>


namespace etrading
{
	namespace
	{
		/* @brief		Private helper function which identifies the type of input leg, and assigns either the premiumLeg / protectionLeg parameter
		*
		*  @param[in]	leg				The input leg.
		*  @param[out]	premiumLeg		On output, initialized to the premiumLeg, if found
		*  @param[out]  protectionLeg	On output, initialized to the protectionLeg, if found
		*/
		void identifyCdsLeg( const std::shared_ptr<Leg>& leg, std::shared_ptr<CDSLeg>& premiumLeg, std::shared_ptr<CDSLeg>& protectionLeg )
		{
			std::shared_ptr<CDSLeg> cdsLeg = std::dynamic_pointer_cast<CDSLeg> ( leg );
			if ( cdsLeg == nullptr )
			{
				// skip this leg, it is not a CDS leg
				return;
			}
			const ScheduleTypeEnum legType = cdsLeg->getType();
			switch( legType )
			{
			case PREMIUM_SCHEDULE_TYPE:
				premiumLeg = cdsLeg;
				break;

			case PROTECTION_SCHEDULE_TYPE:
				protectionLeg = cdsLeg;
				break;

			default:
				AQ_THROW( "Unsupported CDS leg type " + toString( legType ));
			}
		}

		/* @brief	Helper utility function which iniitalizes the dataProvider for each CDS leg.
		*			It also updates the survival probabilities / accruedInterest flag for each cashflow in the legs.
		*  @param[in]	creditModel					A calibrated credit model, used for obtaining survival probabilities
		*  @param[in]	premiumLeg					The CDS premium leg
		*  @param[in]	protectionLeg				The CDS protection leg
		*  @param[out]	dataProviderPremiumLeg		On output: The initialized dataProvider for the premium leg
		*  @param[out]	dataProviderProtectionLeg	On output: The initialized dataProvider for the protection leg
		*  @param[out]	curveCollection				On output: The curveCollection containing the discount curve
		*  @param[out]	discountCurve				On output: The discount curve used for discounting CDS leg cashflows
		*/
		void initializeDataProvidersAndLegs( const CreditModel& creditModel,
											const std::shared_ptr<CDSLeg>& premiumLeg,
											const std::shared_ptr<CDSLeg>& protectionLeg,
											DataProvider& dataProviderPremiumLeg,
											DataProvider& dataProviderProtectionLeg,
											std::string& curveCollection,
											std::string& discountCurve )
		{
			if ( premiumLeg != nullptr )
			{
				const LADate& asOfDate = dataProviderPremiumLeg.getValuationSettings().getValuationDate();
				premiumLeg->setSurvivalProbabilitiesUsingCreditModel( asOfDate, creditModel ); // Update survival probabilities, accrued interest flag
				premiumLeg->initializeDataProvider( dataProviderPremiumLeg );
				curveCollection = dataProviderPremiumLeg.getValuationSettings().getCurveCollection();
				discountCurve   = premiumLeg->getStaticData()->getDiscountCurve().getCString();
			}

			if ( protectionLeg != nullptr )
			{
				const LADate& asOfDate = dataProviderProtectionLeg.getValuationSettings().getValuationDate();
				protectionLeg->setSurvivalProbabilitiesUsingCreditModel( asOfDate, creditModel ); // Update survival probabilities, accrued interest flag
				protectionLeg->initializeDataProvider( dataProviderProtectionLeg );
				curveCollection = dataProviderProtectionLeg.getValuationSettings().getCurveCollection();
				discountCurve   = protectionLeg->getStaticData()->getDiscountCurve().getCString();
			}
		}

		/* @brief	Helper function to calculate a vector of implied stopping dates from survival probabilities.
		*			The function also calculates the discount factor on each stopping date, if required.
		*
		* @param[out]	stoppingDates	Output: This vector will be populated with stopping dates
		* @param[out]	discountFactors	Output: This vector will be populated with the discount factor on each stopping date
		* @param[in]	survivalProbabilities	A vector of survival probabilities from which stopping dates can be implied
		* @param[in]	creditModel				A calibrated credit model
		* @param[in]	curveCollection			The curveCollection containing the discount curve
		* @param[in]	discountCurve			The discount curve to use			
		* @param[in]	payDefaultCashflowsOnNextCouponDate	If true, any cashflows on default are paid on the next coupon date.
		*													if false, any cashflows are paid on the stopping date
		* @param[in]	runInParallel			Whether to iterate over loops in parallel
		*/
		void calculateStoppingDatesAndDiscountFactorsFromSurvivalProbabilties(  std::vector<LADate>& stoppingDates,
																				DoubleVector& discountFactors,
																				const DoubleVector& survivalProbabilities,
																				const CreditModel& creditModel,
																				const std::string& curveCollection,
																				const std::string& discountCurve,
																				const bool payDefaultCashflowsOnNextCouponDate,
																				const bool runInParallel )
		{
			const size_t numPaths = survivalProbabilities.size();
			stoppingDates.resize( numPaths );
			discountFactors.resize (numPaths );
			
			if ( runInParallel )
			{
				#pragma omp parallel for num_threads( omp_get_max_threads() )
				for (int i=0; i<(int) numPaths; i++)
				{
					const LADate stoppingDate = creditModel.getImpliedSurvivalDate( survivalProbabilities[i] );
					stoppingDates[i] = stoppingDate;
				}
			}
			else
			{
				for (size_t i=0; i<numPaths; i++)
				{
					const LADate stoppingDate = creditModel.getImpliedSurvivalDate( survivalProbabilities[i] );
					stoppingDates[i] = stoppingDate;
				}
			}

			// Pre-calculate discount factors at the stopping date, if required
			if ( ! payDefaultCashflowsOnNextCouponDate )
			{
				discountFactors = getCurveDiscountFactors( creditModel.getAsOfDate(), stoppingDates, curveCollection.c_str(), discountCurve.c_str() );
			}
		}
	}

	/* @brief		Private helper method which assigns premiumLeg / protectionLeg pointers for the credit default swap.
	*				If legName is provided, then search for that leg and assign one of premiumLeg / protectionLeg.
	*
	*  @param[in]	legName			Optional: Specify the legName to search for within the CDS. If legName is an empty string, assign both premiunmLeg and protectionLeg parameters
	*  @param[out]	premiumLeg		On output, initialized to the premiumLeg, if found
	*  @param[out]  protectionLeg	On output, initialized to the protectionLeg, if found
	*/
	void CreditDefaultSwap::identifyCdsLegsUsingLegNameifProvided( const LAString& legName, std::shared_ptr<CDSLeg>& premiumLeg, std::shared_ptr<CDSLeg>& protectionLeg ) const
	{
		if ( legName.size() == 0 )
		{
			// Search for a premium AND protection Leg, and initialize premiumLeg / protectionLeg variables
			for (size_t i = 0; i < legs_.size(); ++i)
			{
				auto leg = legs_.get(i);
				identifyCdsLeg( leg, premiumLeg, protectionLeg );
			}
			AQ_REQUIRE( premiumLeg != nullptr, "Missing Premium Leg." );
			AQ_REQUIRE( protectionLeg != nullptr, "Missing Protection Leg." );
		}
		else
		{
			// Search for the specified leg
			for (size_t i = 0; i < legs_.size(); ++i)
			{
				auto leg = legs_.get(i);
				if ( same( leg->getLegName(), legName ))
				{
					identifyCdsLeg( leg, premiumLeg, protectionLeg );
				}
			}
			// Require at least one of premiumLeg or protectionLeg to be set
			AQ_REQUIRE( premiumLeg != nullptr || protectionLeg != nullptr, "Specified legName '" + legName + "' does not exist in the CreditDefaultSwap.");
		}
	}

	CreditDefaultSwap::CreditDefaultSwap( const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB) : Swap(instanceName, CREDIT_DEFAULT_SWAP, swapPropertiesLVB) 
	{
		auto cdsLeg1 = std::dynamic_pointer_cast<CDSLeg>( leg1 );
		if ( cdsLeg1 == nullptr )
		{
			AQ_THROW( "Swap leg 1 '" + leg1->getLegName() + "' is not a CDS Leg. ");
		}

		auto cdsLeg2 = std::dynamic_pointer_cast<CDSLeg>( leg2 );
		if ( cdsLeg2 == nullptr )
		{
			AQ_THROW( "Swap leg 2 '" + leg1->getLegName() + "' is not a CDS Leg. ");
		}
		
		validateLegs(leg1, leg2);

        //Single Currency
        if ( leg1->getStaticData()->getCurrency() != leg2->getStaticData()->getCurrency() )
        {
  		    AQ_THROW( "It is not a single currency Swap" );
        }

        if (boost::math::isnan(leg1->getSchedule()->getNotional()) || boost::math::isnan(leg2->getSchedule()->getNotional()))
        {
    		AQ_THROW( "Please provide Notional" );
        }

        inputParameters_ = swapPropertiesLVB;
        
        addToLegCollection(leg1);
		addToLegCollection(leg2);

    }

    CreditDefaultSwap::CreditDefaultSwap(const CreditDefaultSwap& rhs) : Swap(rhs)
    {}

    std::shared_ptr<Swap> CreditDefaultSwap::clone()
    {
        SwapPtr swap = SwapPtr(new CreditDefaultSwap(*this));
        return swap;
    }

	void CreditDefaultSwap::validateCreditModel( const CreditModel& creditModel ) const
	{
		auto firstLeg = legs_.get(0);
		CCY firstLegCurrency = firstLeg->getStaticData()->getCurrency();

		if ( firstLegCurrency != creditModel.getCurrency() )
		{
			AQ_THROW( "Leg currency '" + toString( firstLegCurrency ) + "' does not match Credit Model currency '" + toString( creditModel.getCurrency() ) + "'." );
		}		
	}

	/* @brief Calculates the total PV of all of the Credit Default Swap legs.
	*
	* @param[in]	valuationSettingsLVB			The collection containing the OIS curve for discounting
	* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	legName					Optionally calculate the PV of the specified leg only.
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the PV
	* @returns	The calculated PV value
	*/
	double CreditDefaultSwap::pvFromHazardRate( const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const LAString& legName, const bool includeAccruedInterest ) const
	{
		double pv = 0.0;

        if (legName.size() == 0)
        {
			// Calc PV of all legs
			for ( size_t i = 0; i < legs_.size(); ++i )
			{
				auto leg = legs_.get( i );

				DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, {}, leg->getLegName()));

				// OK to use static_pointer_cast here because we know the leg type for sure (the CreditDefaultSwap created it).
				std::shared_ptr<CDSLeg> cdsLeg = std::static_pointer_cast<CDSLeg> ( leg );
				pv += cdsLeg->pvFromHazardRate(dataProvider, hazardRate, recoveryRate, includeAccruedInterest );
			}
        }
		else
		{
			// Search for the specified leg and calculate the PV of that leg only.
			bool legFound = false;
			for (size_t i = 0; i < legs_.size(); ++i)
			{
				auto leg = legs_.get(i);
				if ( same( leg->getLegName(), legName ))
				{
					legFound = true;

					DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, {}, leg->getLegName()));

					// OK to use static_pointer_cast here because we know the leg type for sure (the CreditDefaultSwap created it).
					std::shared_ptr<CDSLeg> cdsLeg = std::static_pointer_cast<CDSLeg> ( leg );
					pv = cdsLeg->pvFromHazardRate(dataProvider, hazardRate, recoveryRate, includeAccruedInterest );

					break;
				}
			}
			if ( ! legFound )
			{
				AQ_THROW("Specified legName '" + legName + "' does not exist in the CreditDefaultSwap." );
			}
		}

		return pv;
	}

	/* @brief Calculates the total PV of all of the Credit Default Swap legs.
	*
	* @param[in]	creditModel				The calibrated credit model
	* @param[in]	legName					Optionally calculate the PV of the specified leg only.
	* @returns	The calculated PV value
	*/
	double CreditDefaultSwap::pv( const CreditModel& creditModel, const LAString& legName ) const
	{
		validateCreditModel( creditModel );
		const std::string curveCollection = creditModel.getCDSCurveCollection();

		double pv = 0.0;

        if (legName.size() == 0)
        {
			// Calc PV of all legs
			for ( size_t i = 0; i < legs_.size(); ++i )
			{
				auto leg = legs_.get( i );
				
				// Identify the Leg Type: We need to allow mixed leg types i.e. credit and rates legs
				const ScheduleTypeEnum legType = leg->getType();
				const bool isCreditLeg = ( legType == ScheduleTypeEnum::PREMIUM_SCHEDULE_TYPE || legType == ScheduleTypeEnum::PROTECTION_SCHEDULE_TYPE );

				if ( isCreditLeg )
				{
					// OK to use static_pointer_cast here because we know the leg type for sure (the CreditDefaultSwap created it).
					std::shared_ptr<CDSLeg> cdsLeg = std::static_pointer_cast<CDSLeg> ( leg );
					pv += cdsLeg->pv( creditModel );
				}
				else
				{
					DataProvider dataProvider( ValuationSettings(curveCollection,true) );
					pv += leg->pv( dataProvider, false );
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
				if ( same( leg->getLegName(), legName ) )
				{
					legFound = true;

					// Identify the Leg Type: We need to allow mixed leg types i.e. credit and rates legs
					const ScheduleTypeEnum legType = leg->getType();
					const bool isCreditLeg = ( legType == ScheduleTypeEnum::PREMIUM_SCHEDULE_TYPE || legType == ScheduleTypeEnum::PROTECTION_SCHEDULE_TYPE );

					if ( isCreditLeg )
					{
						// OK to use static_pointer_cast here because we know the leg type for sure (the CreditDefaultSwap created it).
						std::shared_ptr<CDSLeg> cdsLeg = std::static_pointer_cast<CDSLeg> ( leg );
						pv = cdsLeg->pv( creditModel );
					}
					else
					{
						DataProvider dataProvider( ValuationSettings(curveCollection,true) );
						pv = leg->pv( dataProvider, false );
					}

					break;
				}
			}
			if ( ! legFound )
			{
				AQ_THROW("Specified legName '" + legName + "' does not exist in the CreditDefaultSwap." );
			}
		}

		return pv;
	}

	/* @brief	Calculates the total PV of all the Credit Default Swap Legs, by integrating the payoff over survivial probability.
	*			The integration over survival probability is equivalent to an integration over survival time, with an appropriate change of variable.
	*			The PV of coupon payments in each leg is calculated by assuming they are paid out with certainty, up to the default time.
	*			When default occurs after the maturity of the CDS there is no more protection payout.
	*
	* @param[in]	creditModel							The calibrated credit model
	* @param[in]	legName								Optionally calculate the PV of the specified leg only.
	* @param[in]	numberOfIntegrationPoints			Specifies the number of (x,y) points to use in the numerical integration
	* @param[in]	evaluateInParallel					When TRUE, evaluate loops in parallel, where possible.
	* @param[in]	payDefaultCashflowsOnNextCouponDate	When default occurs, whether to pay out the protection and accrued interest immediately, or wait to the next coupon date.
	*													TRUE means wait to the next coupon date. This flag is used to match the PV by integration to the analytic PV formula.
	* @returns	The calculated PV value
	*/
	double CreditDefaultSwap::pvByIntegration( const CreditModel& creditModel, const LAString& legName, const size_t numberOfIntegrationPoints, const bool evaluateInParallel, const bool payDefaultCashflowsOnNextCouponDate ) const
	{ 
		std::shared_ptr<CDSLeg> premiumLeg;
		std::shared_ptr<CDSLeg> protectionLeg;
		identifyCdsLegsUsingLegNameifProvided( legName, premiumLeg, protectionLeg );
		
		// Initialize data providers in preparation for calculating discount factors
		std::string curveCollection;
		std::string discountCurve;
		DataProvider dataProviderPremiumLeg( ValuationSettings(creditModel, {}) );
		DataProvider dataProviderProtectionLeg( ValuationSettings(creditModel, {}) );
		initializeDataProvidersAndLegs( creditModel, premiumLeg, protectionLeg, dataProviderPremiumLeg, dataProviderProtectionLeg, curveCollection, discountCurve );

		const LADate effectiveDate = premiumLeg == nullptr ? protectionLeg->getSchedule()->getEffectiveDate() : premiumLeg->getSchedule()->getEffectiveDate();
		const LADate maturityDate  = premiumLeg == nullptr ? protectionLeg->getSchedule()->getMaturityDate() : premiumLeg->getSchedule()->getMaturityDate();
		
		const double maxSurvivalProb      = creditModel.getSurvivalProbability( effectiveDate );
		const double maturitySurvivalProb = creditModel.getSurvivalProbability( maturityDate );
		const double minSurvivalProb = 0.0;

		DoubleVector survivalProbabilities;
		survivalProbabilities.reserve( numberOfIntegrationPoints );
		DoubleVector presentValues;
		presentValues.reserve( numberOfIntegrationPoints );

		for ( size_t i=0; i<numberOfIntegrationPoints; i++ )
		{
			const double survivalProbability = maturitySurvivalProb + ( maxSurvivalProb - maturitySurvivalProb ) * (double) i / (double) (numberOfIntegrationPoints-1); 
			survivalProbabilities.push_back( survivalProbability );
		}

		// Pre-calculate the stopping dates implied by the simulated survival probabilities
		// Note numPaths may not be equal to numSamples if we are using antithetic sampling
		const size_t numPaths = survivalProbabilities.size();
		std::vector<LADate> stoppingDates( numPaths );
		DoubleVector discountFactorsAtStoppingDates( numPaths, std::numeric_limits<double>::quiet_NaN() );
		calculateStoppingDatesAndDiscountFactorsFromSurvivalProbabilties( stoppingDates,
																		  discountFactorsAtStoppingDates,
																		  survivalProbabilities,
																		  creditModel,
																		  curveCollection,
																		  discountCurve,
																		  payDefaultCashflowsOnNextCouponDate,
																		  evaluateInParallel );

		// Calculate the PV at selected node points, i.e. the integrand function
		// X axis is survival probability, y axis is CDS PV
		for ( size_t i=0; i<numberOfIntegrationPoints; i++ )
		{
			const LADate& stoppingDate = stoppingDates[i];
			const double discountFactorAtStoppingDate = discountFactorsAtStoppingDates[i];
			const double pvPremiumLeg    = premiumLeg == nullptr ? 0.0 : premiumLeg->riskFreePVtoStoppingDate( dataProviderPremiumLeg, creditModel, stoppingDate, discountFactorAtStoppingDate, payDefaultCashflowsOnNextCouponDate );
			const double pvProtectionLeg = protectionLeg == nullptr ? 0.0 : protectionLeg->riskFreePVtoStoppingDate( dataProviderProtectionLeg, creditModel, stoppingDate, discountFactorAtStoppingDate, payDefaultCashflowsOnNextCouponDate );

			const double pvToStoppingDate = pvPremiumLeg + pvProtectionLeg;
			presentValues.push_back( pvToStoppingDate );
		}

        // Initialize the integrand Target Function Class
		InterpolationEnum interpolationType = etrading::LINEAR_INTERPOLATION;
        SimpsonsRule::TargetFunction target( survivalProbabilities, presentValues, interpolationType );
        
        // Initialize Integration Settings
        SimpsonsRule::IntegrationLimits integrationLimits;
        integrationLimits.lowerBound  = maturitySurvivalProb;
        integrationLimits.upperBound  = maxSurvivalProb;
        integrationLimits.nSteps      = numberOfIntegrationPoints - 1;

        // Numerically Integrate
        SimpsonsRuleIntegrand integrand( target );
        const double integralBeforeMaturity = integrand.integrate( integrationLimits );

		// "Constant" part of the integral, where the default time occurs after maturity of the CDS
		const double pvToMaturityPremiumLegOnly = ( premiumLeg == nullptr ) ? 0.0 : premiumLeg->riskFreePVtoStoppingDate( dataProviderPremiumLeg, creditModel, maturityDate );
		const double integralAfterMaturity = pvToMaturityPremiumLegOnly * ( maturitySurvivalProb - minSurvivalProb );

		const double expectedPV = integralBeforeMaturity + integralAfterMaturity;
		return expectedPV;
	}

	/* @brief	Calculates the total PV of all the Credit Default Swap Legs, by a monte-carlo simulation over survivial probability.
	*			The monte-carlo over survival probability is equivalent to a monte-carlo over survival time, with an appropriate change of variable.
	*			The PV of coupon payments in each leg is calculated by assuming they are paid out with certainty, up to the default time.
	*			When default occurs after the maturity of the CDS there is no more protection payout.
	*
	* @param[in]	creditModel							The calibrated credit model
	* @param[in]	legName								Optionally calculate the PV of the specified leg only.
	* @param[in]	mcParametersLVB						A label value block specifying Monte-Carlo / Random number generator parameters. 
	* @param[in]	payDefaultCashflowsOnNextCouponDate	When default occurs, whether to pay out the protection and accrued interest immediately, or wait to the next coupon date.
	*													TRUE means wait to the next coupon date. This flag is used to match the PV by integration to the analytic PV formula.
	* @param[out]	standardError						Returns the Monte-Carlo standard error in the result.
	*
	* @returns	The calculated PV value
	*/
	double CreditDefaultSwap::pvByMonteCarlo( const CreditModel& creditModel, const LAString& legName, const LabelValueBlock& mcParametersLVB, const bool payDefaultCashflowsOnNextCouponDate, double& standardError ) const
	{		
		std::shared_ptr<CDSLeg> premiumLeg;
		std::shared_ptr<CDSLeg> protectionLeg;
		identifyCdsLegsUsingLegNameifProvided( legName, premiumLeg, protectionLeg );

		// Initialize data providers in preparation for calculating discount factors
		std::string curveCollection;
		std::string discountCurve;
		DataProvider dataProviderPremiumLeg( ValuationSettings(creditModel, {}) );
		DataProvider dataProviderProtectionLeg( ValuationSettings(creditModel, {}) );
		initializeDataProvidersAndLegs( creditModel, premiumLeg, protectionLeg, dataProviderPremiumLeg, dataProviderProtectionLeg, curveCollection, discountCurve );

		const LADate effectiveDate = premiumLeg == nullptr ? protectionLeg->getSchedule()->getEffectiveDate() : premiumLeg->getSchedule()->getEffectiveDate();
		const LADate maturityDate  = premiumLeg == nullptr ? protectionLeg->getSchedule()->getMaturityDate() : premiumLeg->getSchedule()->getMaturityDate();
		const double pvToMaturityPremiumLegOnly = premiumLeg == nullptr ? 0.0 : premiumLeg->riskFreePVtoStoppingDate( dataProviderPremiumLeg, creditModel, maturityDate );

		const double maxSurvivalProb      = creditModel.getSurvivalProbability( effectiveDate );
		const double maturitySurvivalProb = creditModel.getSurvivalProbability( maturityDate );
		const double minSurvivalProb      = 0.0;

		// The random number generator used by the Monte-Carlo simulation
		// Here we run the simulation for the period before maturity of the CDS,
		// i.e. the survival probability is between maxSurvivalProbability and maturitySurvivalProbability
		RandomNumberGenerator generator = RandomNumberGenerator::buildUniformGenerator( mcParametersLVB, maturitySurvivalProb, maxSurvivalProb );


		const DistributionEnum distributionEnum	= generator.getDistribution();
		if ( distributionEnum != UNIFORM_DISTRIBUTION )
		{
			AQ_THROW( "CreditDefaultSwap Pricing requires a UNIFORM random number generator");
		}

		const int numberOfSamples			= generator.getNumberOfSamples();
		const bool evaluatePathsInParallel	= generator.getEvaluatePathsInParallel();
		
		// The random number generator is not thread safe. So fetch all the random numbers up front
		const DoubleVector survivalProbabilities = generator.getRandomNumbers( numberOfSamples );

		// Pre-calculate the stopping dates implied by the simulated survival probabilities
		// Note numPaths may not be equal to numSamples if we are using antithetic sampling
		const size_t numPaths = survivalProbabilities.size();
		std::vector<LADate> stoppingDates( numPaths );
		DoubleVector discountFactorsAtStoppingDates( numPaths, std::numeric_limits<double>::quiet_NaN() );
		calculateStoppingDatesAndDiscountFactorsFromSurvivalProbabilties( stoppingDates,
																		  discountFactorsAtStoppingDates,
																		  survivalProbabilities,
																		  creditModel,
																		  curveCollection,
																		  discountCurve,
																		  payDefaultCashflowsOnNextCouponDate,
																		  evaluatePathsInParallel );
		// Generate the set of paths
		std::vector<SurvivalPath> paths( numPaths );
		for (size_t i=0; i<numPaths; i++)
		{
			const double survivalProbability = survivalProbabilities[i];
			const LADate stoppingDate = stoppingDates[i];
			const double discountFactorAtStoppingDate = discountFactorsAtStoppingDates[i];
			paths[i] =  SurvivalPath( survivalProbability, stoppingDate, discountFactorAtStoppingDate );
		}

		/* @brief		The Monte-Carlo payoff function
		*  @param[in]	survivalProbability	The input survival probability
		*
		*  @returns		The PV of the path, assuming the underlying index defaults at the date corresponding to the input survivalProbability.
		*/
		auto payoffFunction = [&] ( const SurvivalPath& path ) -> double
		{
			// Calculate the PV of premium and protection coupons assuming default occurs on the stopping date
			const LADate stoppingDate = path.getStoppingDate();
			const double discountFactorAtStoppingDate = path.getDiscountFactorAtStoppingDate();

			const double pvPremiumLeg = premiumLeg == nullptr ? 0.0 : premiumLeg->riskFreePVtoStoppingDate( dataProviderPremiumLeg, creditModel, stoppingDate, discountFactorAtStoppingDate, payDefaultCashflowsOnNextCouponDate );
			const double pvProtectionLeg = protectionLeg == nullptr ? 0.0 : protectionLeg->riskFreePVtoStoppingDate( dataProviderProtectionLeg, creditModel, stoppingDate, discountFactorAtStoppingDate, payDefaultCashflowsOnNextCouponDate );
			const double pvToStoppingDate = pvPremiumLeg + pvProtectionLeg;
			return pvToStoppingDate;
		};

		// Monte Carlo for the period before maturity of the CDS
		MonteCarloEngine<SurvivalPath> monteCarloEngine( payoffFunction, evaluatePathsInParallel );
		monteCarloEngine.runSimulation( paths );
		
		const MonteCarloStatistics& monteCarloStatistics = monteCarloEngine.getMonteCarloStatistics();
		double pv = monteCarloStatistics.getMean();

		const double probabilityScalingFactor = ( maxSurvivalProb - maturitySurvivalProb );
		pv *= probabilityScalingFactor;

		standardError = monteCarloStatistics.getStandardError();
		standardError *= probabilityScalingFactor;

		// Contribution from the period after maturity of the CDS
		const double pvContributionFromDefaultAfterMaturity = pvToMaturityPremiumLegOnly * ( maturitySurvivalProb - minSurvivalProb );
		pv += pvContributionFromDefaultAfterMaturity;

		return pv;
	}

	/* @brief Calculate the PV of a CDS instrument which pays the specified fixedCoupon
	*
	* @param[in]	creditModel		The calibrated credit model
	* @param[in]	parSpread		The fair-value par-spread of the CDS instrument
	* @param[in]	fixedCoupon		The coupon rate which standardises the CDS instrument, e.g. 100bps
	* @param[in]	assumeFlatCurve	If true, the ISDA flat curve approximation is used to calculate the risky annuity factor.
	*								If false, the calibrated term structure of hazard rates is used.
	*/
	double CreditDefaultSwap::pvFromSpread( const CreditModel& creditModel, const double parSpread, const double fixedCoupon, const bool assumeFlatCurve ) const
	{
		const LAString premiumLegName = getLeg(0)->getLegName();

		double calculatedRiskyAnnuity = 0.0;
		if ( assumeFlatCurve )
		{
			const double recoveryRate = creditModel.getRecoveryRate();
			AQ_REQUIRE((recoveryRate >= 0 && recoveryRate < 1.0), "Require the recovery rate to be positive and strictly less than 1.0");

			const double hazardRateAtFlatSpread = parSpread / (1.0 - recoveryRate );

			LabelValueBlock valuationSettingsLVB( VALUATION_SETTING_KEYS::CURVE_COLLECTION, creditModel.getCDSCurveCollection().c_str() );

			calculatedRiskyAnnuity = riskyAnnuityFromHazardRate( valuationSettingsLVB, hazardRateAtFlatSpread, recoveryRate, premiumLegName, creditModel.getIncludeAccruedInterest() );
		}
		else
		{
			calculatedRiskyAnnuity = riskyAnnuity( creditModel, premiumLegName );
		}

		const double pv = ( parSpread - fixedCoupon ) * calculatedRiskyAnnuity;
		return pv;
	}

	/* @brief	Models the CDS spread as log-normal process. S_t = S0 exp( -0.5 vol^2 t + vol sqrt(T) )
	*			Given a normal variate taken from a standard normal distribution, returns the corresponding spread value
	*
	* @param[in]	meanSpreadEstimate	The mean of the log-normal process
	* @param[in]	normalVariateSample	A sample taken from a standard normal distribution
	* @param[in]	drift				The log-normal process drift term
	* @param[in]	volatilityTime		The volatility scaling factor multiplied by the normal variate
	*/
	double CreditDefaultSwap::logNormalSpreadProcess( const double meanSpreadEstimate, const double normalVariateSample, const double drift, const double volatilityTime ) const
	{
		const double spreadSample = meanSpreadEstimate * std::exp( drift + volatilityTime * normalVariateSample );
		return spreadSample;
	}

	/* @brief	Calculates the expected value of a forward CDS, assuming the spread follows a log-normal process
	*
	* @param[in]	creditModel			The calibrated credit model
	* @param[in]	meanSpreadEstimate	A sample taken from a standard normal distribution
	* @param[in]	drift				The log-normal process drift term
	* @param[in]	volatilityTime		The volatility scaling factor multiplied by the normal variate
	* @param[in]	fixedCoupon			The coupon rate which standardises the CDS instrument, e.g. 100bps
	*/
	double CreditDefaultSwap::calculateCDSIndexExpectedValue( const CreditModel& creditModel, const double meanSpreadEstimate, const double drift, const double volatilityTime, const double fixedCoupon ) const
	{
		const bool assumeFlatCurve = true;

		// Calculate the value of the CDS assuming the spread is a log normal process
		auto integrandFunction = [&]( const double integrationVariable ) -> double
		{
			const double spreadSample = logNormalSpreadProcess( meanSpreadEstimate, integrationVariable, drift, volatilityTime );

			const double cdsPV = pvFromSpread( creditModel, spreadSample, fixedCoupon, assumeFlatCurve );
			const double densityFunction = standardNormalDistributionPDF( integrationVariable );

			return cdsPV * densityFunction;
		};

		// Use Gauss-Hermite integrand because the integrand is of the form
		// f(x) exp( -x^2 ), and f(x) is a smooth function
		const size_t nIntegrationPoints = 64;
		QuantLib::GaussHermiteIntegration integrator(nIntegrationPoints);

		const double cdsExpectedValue = integrator( integrandFunction );
		return cdsExpectedValue;
	}

	/* @brief	Calibrates the spread log-normal process so that it reproduces the price of a forward CDS contract
	*			The calibration parameter is the mean of the log-normal distribution
	* @param[in]	creditModel			The calibrated credit model
	* @param[in]	targetForwardPrice	The forward spread to match
	* @param[in]	drift				The log-normal process drift term
	* @param[in]	volatilityTime		The volatility scaling factor which multiplies normal variate
	* @param[in]	forwardCoupon		The coupon rate which standardises the CDS instrument, e.g. 100bps
	*/
	double CreditDefaultSwap::calibrateLogNormalMeanSpread( const CreditModel& creditModel, const double targetForwardPrice, const double drift, const double volatilityTime, const double fixedCoupon ) const
	{
		// Newton-Raphson Solver Settings
		const double initialMeanSpreadEstimate = 0.005; // TODO: Think of something better
		const double tolerance = 1.0e-10;
		const unsigned int maxIterations = 20;
		const double shiftSize = 0.00001;

		// One-dimensional objective function used by the solver:
		// This lambda function captures the valuationSettingsLVB and recoveryRate as fixed parameters.
		// The inputHazardRate is the variable which the solver will adjust in order to obtain the targeCdsSpread.
		auto function = [&]( const double inputMeanSpreadEstimate ) -> double
		{
			return calculateCDSIndexExpectedValue( creditModel, inputMeanSpreadEstimate, drift, volatilityTime, fixedCoupon );
		};

		// Solver Results Contain: Solution, nInterations and Jacobian
		const solvers::SolverResults solverResults = solvers::newtonRaphson( function, targetForwardPrice, initialMeanSpreadEstimate, tolerance, maxIterations, shiftSize );
		const double meanSpread = solverResults.solution;

		return meanSpread;
	}


	/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
	*
	* @param[in]	dataProvider			The data provider to hold the valuation settings
	* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	legName					The Premium leg to use when calculating the risky annuity. A mandatory parameter.
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
	* @returns	The risky annuity
	*/
	double CreditDefaultSwap::riskyAnnuityFromHazardRate( const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const LAString& legName, const bool includeAccruedInterest ) const
	{
		double riskyAnnuity = 0.0;

		LAString validatedPremiumLegName = validateLegName( legName, PREMIUM_SCHEDULE_TYPE, legs_ );

		auto leg = legs_.findLegByName( validatedPremiumLegName );

		// Check that the specified leg is a premium leg
		auto premiumLeg = std::dynamic_pointer_cast<PremiumLeg>( leg );
		if ( premiumLeg == nullptr )
		{
			AQ_THROW( "Specified leg '" + validatedPremiumLegName + "' is not a CDS Premium Leg." );
		}

		DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, {}, premiumLeg->getLegName()));
		
		riskyAnnuity = premiumLeg->RiskyAnnuityWithNotional(dataProvider, hazardRate, recoveryRate, includeAccruedInterest );

		return riskyAnnuity;
	}

	/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
	*
	* @param[in]	creditModel				The calibrated credit model
	* @param[in]	legName					The Premium leg to use when calculating the risky annuity. A mandatory parameter.
	* @returns	The risky annuity
	*/
	double CreditDefaultSwap::riskyAnnuity( const CreditModel& creditModel, const LAString& legName ) const
	{
		validateCreditModel( creditModel );

		double riskyAnnuity = 0.0;

		LAString validatedPremiumLegName = validateLegName( legName, PREMIUM_SCHEDULE_TYPE, legs_ );

		auto leg = legs_.findLegByName( validatedPremiumLegName );

		// Check that the specified leg is a premium leg
		auto premiumLeg = std::dynamic_pointer_cast<PremiumLeg>( leg );
		if ( premiumLeg == nullptr )
		{
			AQ_THROW( "Specified leg '" + validatedPremiumLegName + "' is not a CDS Premium Leg." );
		}
		
		riskyAnnuity = premiumLeg->RiskyAnnuityWithNotional( creditModel );

		return riskyAnnuity;
	}

	/* @brief[in]	Computes the accrued year fraction from the previous coupon date to the specified date
	*				Used in accrued interest calculations.
	*
	* @param[in]	creditModel		The calibrated credit model
	* @param[in]	toDate			The date to which we wish to calculate the year fraction
	* @param[in]	legName			The Premium leg name
	* @returns: The year fraction
	*/
	double CreditDefaultSwap::accruedYearFraction( const CreditModel& creditModel, const LADate& toDate, const LAString& legName ) const
	{
		validateCreditModel(creditModel);

		double accruedYearFraction = 0.0;

		LAString validatedPremiumLegName = validateLegName(legName, PREMIUM_SCHEDULE_TYPE, legs_);

		auto leg = legs_.findLegByName(validatedPremiumLegName);

		// Check that the specified leg is a premium leg
		auto premiumLeg = std::dynamic_pointer_cast<PremiumLeg>(leg);
		if (premiumLeg == nullptr)
		{
			AQ_THROW("Specified leg '" + validatedPremiumLegName + "' is not a CDS Premium Leg.");
		}

		accruedYearFraction = premiumLeg->accruedYearFraction( creditModel, toDate );

		return accruedYearFraction;
	}

	/* @brief[in]	Computes the accrued interest from the previous coupon date to the specified date
	*				i.e. this corresponds to the amount of premium coupon that is accrued
	*
	* @param[in]	creditModel		The calibrated credit model
	* @param[in]	toDate			The date to which we wish to calculate the accrued interest for
	* @param[in]	legName			The Premium leg name
	* @returns: The accrued interest
	*/
	double CreditDefaultSwap::accruedInterest( const CreditModel& creditModel, const LADate& toDate, const LAString& legName ) const
	{
		validateCreditModel(creditModel);

		double accruedInterest = 0.0;

		LAString validatedPremiumLegName = validateLegName(legName, PREMIUM_SCHEDULE_TYPE, legs_);

		auto leg = legs_.findLegByName(validatedPremiumLegName);

		// Check that the specified leg is a premium leg
		auto premiumLeg = std::dynamic_pointer_cast<PremiumLeg>(leg);
		if (premiumLeg == nullptr)
		{
			AQ_THROW("Specified leg '" + validatedPremiumLegName + "' is not a CDS Premium Leg.");
		}

		accruedInterest = premiumLeg->accruedInterest(creditModel, toDate );

		return accruedInterest;
	}



	/* @brief Calculates the par spread of the specified Credit Default Swap. The internal implementation method
	*
	* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
	* @returns	The CDS par spread
	*/
	double CreditDefaultSwap::parSpreadFromHazardRate_impl( const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const LAString& premiumLegName, const LAString& protectionLegName, const bool includeAccruedInterest ) const
	{
		const double protectionLegPV = pvFromHazardRate( valuationSettingsLVB, hazardRate, recoveryRate, protectionLegName, includeAccruedInterest );

		const double riskyAnnuity = riskyAnnuityFromHazardRate(valuationSettingsLVB, hazardRate, recoveryRate, premiumLegName, includeAccruedInterest );

		const double parSpread = protectionLegPV / riskyAnnuity;
		return parSpread;
	}

	/* @brief Calculates the par spread of the specified Credit Default Swap.
	*
	* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
	* @returns	The CDS par spread
	*/
	double CreditDefaultSwap::parSpreadFromHazardRate( const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const LAString& premiumLegName, const LAString& protectionLegName, const bool includeAccruedInterest ) const
	{
		LAString validatedPremiumLegName    = validateLegName( premiumLegName, PREMIUM_SCHEDULE_TYPE, legs_ );
		LAString validatedProtectionLegName = validateLegName( protectionLegName, PROTECTION_SCHEDULE_TYPE, legs_ );

		const double parSpread = parSpreadFromHazardRate_impl( valuationSettingsLVB, hazardRate, recoveryRate, validatedPremiumLegName, validatedProtectionLegName, includeAccruedInterest );
		return parSpread;
	}

	/* @brief Calculates the par spread of the specified Credit Default Swap.
	*
	* @param[in]	creditModel				The calibrated credit model
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @returns	The CDS par spread
	*/
	double CreditDefaultSwap::parSpread( const CreditModel& creditModel, const LAString& premiumLegName, const LAString& protectionLegName ) const
	{
		validateCreditModel( creditModel );

		LAString validatedPremiumLegName    = validateLegName( premiumLegName, PREMIUM_SCHEDULE_TYPE, legs_ );
		LAString validatedProtectionLegName = validateLegName( protectionLegName, PROTECTION_SCHEDULE_TYPE, legs_ );

		std::string curveCollection = creditModel.getCDSCurveCollection();
		const double parSpread = parSpread_impl( curveCollection.c_str(), creditModel, validatedPremiumLegName, validatedProtectionLegName );
		return parSpread;
	}

	/* @brief Calculates the par spread of the specified Credit Default Swap using the Credit Model. The internal implementation method
	*
	* @param[in]	curveCollection			The curve collection name
	* @param[in]	creditModel				The calibrated credit modelt
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @returns	The CDS par spread
	*/
	double CreditDefaultSwap::parSpread_impl( const LAString& curveCollection, const CreditModel& creditModel, const LAString& premiumLegName, const LAString& protectionLegName ) const
	{
		const double protectionLegPV = pv( creditModel, protectionLegName );

		const double annuity = riskyAnnuity( creditModel, premiumLegName );

		const double parSpread = protectionLegPV / annuity;
		return parSpread;
	}

	/* @brief Solves for the hazard rate, given the specified CDS par spread
	*
	* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	parSpread				The CDS par spread ( as a decimal )
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
	* @returns	The CDS hazard rate
	*/
	double CreditDefaultSwap::hazardRateFromParSpread( const LabelValueBlock& valuationSettingsLVB, const double targetCdsSpread, const double recoveryRate, const LAString& premiumLegName, const LAString& protectionLegName, const bool includeAccruedInterest ) const
	{
		LAString validatedPremiumLegName    = validateLegName( premiumLegName, PREMIUM_SCHEDULE_TYPE, legs_ );
		LAString validatedProtectionLegName = validateLegName( protectionLegName, PROTECTION_SCHEDULE_TYPE, legs_ );

		// Newton-Raphson Solver Settings
        const double initialGuessForHazardRate = targetCdsSpread / ( 1 - recoveryRate );
        const double tolerance            = 1.0e-10;
        const unsigned int maxIterations  = 1000;
        const double shiftSize            = 0.0000001;

		// One-dimensional objective function used by the solver:
		// This lambda function captures the valuationSettingsLVB and recoveryRate as fixed parameters.
		// The inputHazardRate is the variable which the solver will adjust in order to obtain the targeCdsSpread.
		auto function = [valuationSettingsLVB, recoveryRate, validatedPremiumLegName, validatedProtectionLegName, includeAccruedInterest, this] ( const double inputHazardRate ) -> double
		{
			return parSpreadFromHazardRate_impl( valuationSettingsLVB, inputHazardRate, recoveryRate, validatedPremiumLegName, validatedProtectionLegName, includeAccruedInterest );
		};

        // Solver Results Contain: Solution, nInterations and Jacobian
		double hazardRate = solvers::newtonRaphson( function, targetCdsSpread, initialGuessForHazardRate, tolerance, maxIterations, shiftSize ).solution;

		return hazardRate;
	}

	/* @brief Solves for the hazard rate, given the specified CDS par spread.
	*         NOTE: This modified the hazard rate in the credit model. Used in calibration
	*
	* @param[in]	parSpread				The CDS par spread ( as a decimal )
	* @param[inout]	creditModel				The calibrated credit model
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @returns	The CDS hazard rate
	*/
	double CreditDefaultSwap::hazardRateFromParSpread( const double targetCdsSpread, CreditModel& creditModel, const LAString& premiumLegName, const LAString& protectionLegName ) const
	{
		validateCreditModel( creditModel );

		LAString validatedPremiumLegName    = validateLegName( premiumLegName, PREMIUM_SCHEDULE_TYPE, legs_ );
		LAString validatedProtectionLegName = validateLegName( protectionLegName, PROTECTION_SCHEDULE_TYPE, legs_ );

		const double recoveryRate = creditModel.getRecoveryRate();
		// const bool includeAccruedInterest = creditModel.getIncludeAccruedInterest(); <--- Unused Variable
		const std::string curveCollection = creditModel.getCDSCurveCollection();

		// Newton-Raphson Solver Settings
        const double initialGuessForHazardRate = targetCdsSpread / ( 1 - recoveryRate );
        const double tolerance            = 1.0e-10;
        const unsigned int maxIterations  = 1000;
        const double shiftSize            = 0.0000001;

		// Get the final payment date from Leg1
		const DateVector& paymentDates = this->getLeg(0)->getSchedule()->getPaymentDates();
		const LADate finalPaymentDate = paymentDates.back();

		// One-dimensional objective function used by the solver:
		// This lambda function captures the valuationSettingsLVB and recoveryRate as fixed parameters.
		// The inputHazardRate is the variable which the solver will adjust in order to obtain the targeCdsSpread.
		auto function = [&, this] ( const double inputHazardRate ) -> double
		{
			creditModel.setCalibrationPoint( finalPaymentDate, inputHazardRate );	
			return parSpread_impl( curveCollection.c_str(), creditModel, validatedPremiumLegName, validatedProtectionLegName );
		};

        // Solver Results Contain: Solution, nInterations and Jacobian
		double hazardRate = solvers::newtonRaphson( function, targetCdsSpread, initialGuessForHazardRate, tolerance, maxIterations, shiftSize ).solution;

		return hazardRate;
	}

}

