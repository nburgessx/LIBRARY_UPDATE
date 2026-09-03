#include "ZeroCouponInflationSwap.h"
#include "CoreEnumerations.h"
#include "SwapUtilities.h"
#include "SwapCalculation.h"
#include "Solvers.h"


namespace etrading
{

	namespace
	{
		/* @brief	Internal helper function: Given two swap legs (leg1 and leg2), identify which is the Fixed Leg and which is the Inflation Leg
		*			Throws an exception if there is not exactly 1 fixed leg and 1 float leg.
		*
		*  @param[in]	leg1			Leg 1 of the swap
		*  @param[in]	leg2			Leg 2 of the swap
		*  @param[out]	fixedLeg		The identified fixed leg of the swap
		*  @param[out]	inflationLeg	The identified inflation leg of the swap
		*/
		void identifyLegs_impl( const LegPtr& leg1, const LegPtr& leg2, std::shared_ptr<FixedLeg>& fixedLeg, std::shared_ptr<InflationLeg>& inflationLeg )
		{
			// The inflation leg could be either Leg1 or Leg.

			fixedLeg = (leg1->getType() == FIXED_SCHEDULE_TYPE)
				? std::dynamic_pointer_cast<FixedLeg>(leg1)
				: std::dynamic_pointer_cast<FixedLeg>(leg2);

			inflationLeg = (leg1->getType() == INFLATION_SCHEDULE_TYPE)
				? std::dynamic_pointer_cast<InflationLeg>(leg1)
				: std::dynamic_pointer_cast<InflationLeg>(leg2);

			AQ_REQUIRE(fixedLeg != nullptr, "A Fixed leg has not been provided.");
			AQ_REQUIRE(inflationLeg != nullptr, "An Inflation leg has not been provided.");
		}

	}

	/* @brief	Helper function: identify which of the swap's legs is the Fixed Leg and which is the Inflation Leg
	*			Throws an exception if there are not exactly 2 legs in the swap
	*
	*  @param[out]	fixedLeg		The identified fixed leg of the swap
	*  @param[out]	inflationLeg	The identified inflation leg of the swap
	*/
	void ZeroCouponInflationSwap::identifyLegs( std::shared_ptr<FixedLeg>& fixedLeg, std::shared_ptr<InflationLeg>& inflationLeg ) const
	{
		AQ_REQUIRE(legs_.size() == 2, "Zero Coupon Swap must have two legs.");

		auto leg1 = legs_.get(0);
		auto leg2 = legs_.get(1);
		identifyLegs_impl( leg1, leg2, fixedLeg, inflationLeg );

	}

	// Constructor
	ZeroCouponInflationSwap::ZeroCouponInflationSwap( const std::string& instanceName, const LegPtr& leg1, const LegPtr& leg2, const LabelValueBlock& swapPropertiesLVB) : Swap(instanceName, ZERO_COUPON_INFLATION_SWAP, swapPropertiesLVB) 
	{
		validateLegs( leg1, leg2 );

		std::shared_ptr<FixedLeg> fixedLeg;
		std::shared_ptr<InflationLeg> inflationLeg;
		identifyLegs_impl( leg1, leg2, fixedLeg, inflationLeg );

        // Require Single Currency and notional to be specified
		AQ_REQUIRE( leg1->getStaticData()->getCurrency() == leg2->getStaticData()->getCurrency(),  "ZeroCouponInflationSwap must be configured as a single currency Swap" );
		AQ_REQUIRE( ( ! boost::math::isnan(leg1->getSchedule()->getNotional() ) ) && ( ! boost::math::isnan(leg2->getSchedule()->getNotional() )  ), "Please provide Notional" );

        inputParameters_ = swapPropertiesLVB;
        
        addToLegCollection(leg1);
		addToLegCollection(leg2);
    }

	// Copy Constructor
    ZeroCouponInflationSwap::ZeroCouponInflationSwap( const ZeroCouponInflationSwap& rhs) : Swap(rhs)
    {}

    std::shared_ptr<Swap> ZeroCouponInflationSwap::clone()
    {
        SwapPtr swap = SwapPtr( new ZeroCouponInflationSwap(*this) );
        return swap;
    }



	/* @brief	Calculates the total PV of all of the Zero Coupon Inflation Swap legs.
	*			This simple PV calculation requires the user to specify the inflation fixings at start and end of the trade
	*
	* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
	* @param[in]	baseIndex				The inflation level at the effective date of the swap
	* @param[in]	resetIndex				The inflation level at the maturity of the swap
	* @param[in]	legName					Optionally calculate the PV of the specified leg only.
	* @returns	The calculated PV value
	*/
	double ZeroCouponInflationSwap::pvFromInflationIndex( const LabelValueBlock& valuationSettingsLVB, const double baseIndex, const double resetIndex, const std::string& legName ) const
	{
		double pv = 0.0;

        if (legName.size() == 0)
        {
			// Calc PV of all legs
			for ( size_t i = 0; i < legs_.size(); ++i )
			{
				auto leg = legs_.get( i );

				DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, {}, leg->getLegName()));

				if ( leg->getType() == INFLATION_SCHEDULE_TYPE )
				{
					// OK to use static_pointer_cast here because the leg type was validated in the constructor
					std::shared_ptr<InflationLeg> inflationLeg = std::static_pointer_cast<InflationLeg> (leg);
					pv += inflationLeg->pvFromInflationIndex( dataProvider, baseIndex, resetIndex );
				}
				else
				{
					pv += leg->pv( dataProvider );
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
				if ( same( leg->getLegName(), legName ))
				{
					legFound = true;

					DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, {}, leg->getLegName()));

					if (leg->getType() == INFLATION_SCHEDULE_TYPE)
					{
						// OK to use static_pointer_cast here because the leg type was validated in the constructor
						std::shared_ptr<InflationLeg> inflationLeg = std::static_pointer_cast<InflationLeg> (leg);
						pv = inflationLeg->pvFromInflationIndex( dataProvider, baseIndex, resetIndex );
					}
					else
					{
						pv = leg->pv( dataProvider );
					}

					break;
				}
			}
			if ( ! legFound )
			{
				AQ_THROW("Specified legName '" + legName + "' does not exist in the ZeroCouponInflationSwap." );
			}
		}

		return pv;
	}

	/* @brief	Calculates the total PV of all of the Zero Coupon Inflation Swap legs.
	*			This version obtains the inflation resets from a supplied InflationCurve.
	*
	* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
	* @param[in]	inflationCurve			InflationCurve used to obtain inflation index reset values
	* @param[in]	legName					Optionally calculate the PV of the specified leg only.
	* @returns	The calculated PV value
	*/
	double ZeroCouponInflationSwap::pvFromInflationCurve( const LabelValueBlock& valuationSettingsLVB, const InflationCurve& inflationCurve, const std::string& legName ) const
	{
		double pv = 0.0;

		if ( legName.size() == 0 )
		{
			// Calc PV of all legs
			for (size_t i = 0; i < legs_.size(); ++i)
			{
				auto leg = legs_.get(i);

				DataProvider dataProvider(ValuationSettings(valuationSettingsLVB, {}, leg->getLegName()));
			
				if (leg->getType() == INFLATION_SCHEDULE_TYPE)
				{
					std::shared_ptr<InflationLeg> inflationLeg = std::static_pointer_cast<InflationLeg> (leg);
					pv += inflationLeg->pv( dataProvider, inflationCurve );

				}
				else
				{
					pv += leg->pv( dataProvider );
				}
			}
		}
		else
		{
			// Search for the specified leg and calculate the PV of that leg only.
			bool legFound = false;
			for ( size_t i = 0; i < legs_.size(); ++i )
			{
				auto leg = legs_.get(i);
				if ( same( leg->getLegName(), legName ) )
				{
					legFound = true;
					DataProvider dataProvider( ValuationSettings(valuationSettingsLVB, {}, leg->getLegName() ) );

					if ( leg->getType() == INFLATION_SCHEDULE_TYPE )
					{
						std::shared_ptr<InflationLeg> inflationLeg = std::static_pointer_cast<InflationLeg> ( leg );
						pv = inflationLeg->pv( dataProvider, inflationCurve );
					}
					else
					{			
						pv = leg->pv( dataProvider );
					}

					break;
				}
			}
			if ( ! legFound)
			{
				AQ_THROW("Specified legName '" + legName + "' does not exist in the ZeroCouponInflationSwap.");
			}
		}

		return pv;
	}


	/* @brief	Calculates the par rate of the Zero Coupon Inflation Swap
	*			i.e. the break-even coupon rate of the fixed leg which causes the swap to PV to zero.
	*			This simple calculation requires the user to specify the inflation fixings at start and end of the trade
	*
	* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
	* @param[in]	baseIndex				The inflation level at the effective date of the swap
	* @param[in]	resetIndex				The inflation level at the maturity of the swap
	* @returns	The calculated par rate value
	*/
	double ZeroCouponInflationSwap::parRateFromInflationIndex( const LabelValueBlock& valuationSettingsLVB, const double baseIndex, const double resetIndex ) const
	{
		std::shared_ptr<FixedLeg> fixedLeg;
		std::shared_ptr<InflationLeg> inflationLeg;
		identifyLegs( fixedLeg, inflationLeg );

		DataProvider fixedLegDataProvider( ValuationSettings(valuationSettingsLVB, {}, fixedLeg->getLegName()) );
		DataProvider inflationDataProvider( ValuationSettings(valuationSettingsLVB, {}, inflationLeg->getLegName()) );
		const double inflationLegPV = inflationLeg->pvFromInflationIndex( inflationDataProvider, baseIndex, resetIndex );

		const double parRate = parRate_impl( inflationLegPV, fixedLeg, fixedLegDataProvider );

		return parRate;
	}

	/* @brief	Internal implementation method for par-rate calculation
	*			i.e. the break-even coupon rate of the fixed leg which causes the swap to PV to zero.
	*
	* @param[in]	inflationLegPV			PV of the inflation leg
	* @param[in]	fixedLeg				The fixed leg of the swap.
	* @param[in]	fixedLegDataProvider	Holds discount factors used to PV the coupons of the fixed leg
	* @returns	The calculated par rate value
	*/
	double ZeroCouponInflationSwap::parRate_impl( const double inflationLegPV, const std::shared_ptr<FixedLeg>& fixedLeg, DataProvider& fixedLegDataProvider ) const
	{
		// Normalize the solver tolerance by notional. 
		const double normalizationFactor = std::fabs( fixedLeg->getSchedule()->getNotional() );
		const double epsilonForPv = 1e-8 * normalizationFactor;

		// 1) Form an initial estimate of the parRate
		const double fixedAnnuity    = fixedLeg->annuityWithNotional( fixedLegDataProvider );
		const double parRateEstimate = inflationLegPV / fixedAnnuity;

		setSwapLegSpreadOrFixedRate( fixedLegDataProvider, fixedLeg->getType(), parRateEstimate );
		const double fixedLegPV = fixedLeg->pv(fixedLegDataProvider, false/* nativeCurrency */, false/* updateCurveData */);
		const double pv = fixedLegPV + inflationLegPV;

		if ( fabs( pv ) <= epsilonForPv )
		{
			return parRateEstimate;
		}

		//2) Use Secant method to find the sensitivity of the fixedLeg to parRate
		double ds = 0.0005;

		setSwapLegSpreadOrFixedRate( fixedLegDataProvider, fixedLeg->getType(), parRateEstimate - ds );
		auto pv0 = fixedLeg->pv( fixedLegDataProvider, false/* nativeCurrency */, false/* updateCurveData */) + inflationLegPV;

		setSwapLegSpreadOrFixedRate( fixedLegDataProvider, fixedLeg->getType(), parRateEstimate + ds );
		auto pv1 = fixedLeg->pv(fixedLegDataProvider, false/* nativeCurrency */, false/* updateCurveData */) + inflationLegPV;

		auto deltaPV = (pv1 - pv0) / (2.0*ds);

		// Solve For the Spread; Using a tolerance that has been normalized by the trade notional
		// ------------------------------------------------------
		auto result = solveSpread( fixedLegDataProvider, pv, inflationLegPV, deltaPV, parRateEstimate, epsilonForPv, fixedLeg );
		bool solutionFound = result.first;
		AQ_REQUIRE( solutionFound, "Unable to calculate the swap spread. Solver failed to converge to a solution.")
		const double spread = result.second;
		// ------------------------------------------------------

		//Spread is in basis point, so need to scale back
		const double parRate = spread * 0.0001;
		return parRate;
	}

	/* @brief	Calculates the par rate of the Zero Coupon Inflation Swap
	*			i.e. the break-even coupon rate of the fixed leg which causes the swap to PV to zero.
	*			This simple calculation requires the user to specify the inflation fixings at start and end of the trade
	*
	* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
	* @param[in]	inflationCurve			InflationCurve used to obtain inflation index reset values
	* @returns	The calculated par rate value
	*/
	double ZeroCouponInflationSwap::parRateFromInflationCurve( const LabelValueBlock& valuationSettingsLVB, const InflationCurve& inflationCurve ) const
	{
		std::shared_ptr<FixedLeg> fixedLeg;
		std::shared_ptr<InflationLeg> inflationLeg;
		identifyLegs( fixedLeg, inflationLeg );

		DataProvider fixedLegDataProvider( ValuationSettings( valuationSettingsLVB, {}, fixedLeg->getLegName() ));
		DataProvider inflationDataProvider( ValuationSettings( valuationSettingsLVB, {}, inflationLeg->getLegName() ));
		
		const double inflationLegPV  = inflationLeg->pv( inflationDataProvider, inflationCurve );

		const double parRate = parRate_impl( inflationLegPV, fixedLeg, fixedLegDataProvider );

		return parRate;
	}


	/* @brief Solves for the inflation index which prices the swap at par
	*         NOTE: This modifies the inflation index within the Inflation Curve. Used in calibration.
	*
	* @param[in]	valuationSettingsLVB	The valuation settings containing curveCollections, and valuation date
	* @param[inout]	inflationCurve			The calibrated inflation curve
	* @returns	The implied inflation index to make swap price at par
	*/
	double ZeroCouponInflationSwap::impliedInflationIndexAtPar( const LabelValueBlock& valuationSettingsLVB, InflationCurve& inflationCurve ) const
	{
		std::shared_ptr<FixedLeg> fixedLeg;
		std::shared_ptr<InflationLeg> inflationLeg;
		identifyLegs( fixedLeg, inflationLeg );

		// Newton-Raphson Solver Settings
		const double targetPV = 0.0;
		const double initialGuessForInflationIndex = 100.;
		const double tolerance = 1.0e-10;
		const unsigned int maxIterations = 1000;
		const double shiftSize = 0.0000001;

		AQLDate finalFixingDate = inflationLeg->getSchedule()->getFixingDates().back();
		const std::string curveCollection = inflationCurve.getCurveCollection();

		// One-dimensional objective function used by the solver:
		// This lambda function captures the valuationSettingsLVB and inflationCurve as fixed parameters.
		// The inputInflationIndex is the variable which the solver will adjust in order to obtain the targetPV.
		auto function = [&, this](const double inputInflationIndex ) -> double
		{
			inflationCurve.setCalibrationPoint( finalFixingDate, inputInflationIndex );
			return pvFromInflationCurve( valuationSettingsLVB, inflationCurve );
		};

		// Solver Results Contain: Solution, nInterations and Jacobian
		const double inflationIndex = solvers::newtonRaphson( function, targetPV, initialGuessForInflationIndex, tolerance, maxIterations, shiftSize ).solution;

		return inflationIndex;
	}


}

