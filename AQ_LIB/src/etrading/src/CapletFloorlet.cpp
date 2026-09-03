#include "CapletFloorlet.h"
#include "NormalDistribution.h"
#include "ExceptionMacros.h"
#include "Solvers.h"
#include <cmath>
#include <algorithm>
#include <functional>

namespace etrading
{

    // Alternative Constructor
    CapletFloorlet::CapletFloorlet( const CapletFloorletEnum& capletOrFloorlet,
                                    const double & annuityFactor,
								    const double & liborRate,
								    const double & strike,
								    const double & vol,
								    const double & timeToExpiry, // yearFraction from valuation date to expiry date
								    const double & shift,
                                    const VolatilityTypeEnum & volatilityType )
        : capletOrFloorlet_(capletOrFloorlet), annuityFactor_(annuityFactor), liborRate_(liborRate), strike_(strike), vol_(vol), timeToExpiry_(timeToExpiry), shift_(shift), volatilityType_(volatilityType)
	{
		AQ_REQUIRE( ( capletOrFloorlet_ == CAPLET_OPTION || capletOrFloorlet_ == FLOORLET_OPTION ),
            "capletOrFloorlet parameter must be 'CAPLET' or 'FLOORLET'" );
	}

    // Copy Constructor
    CapletFloorlet::CapletFloorlet( const CapletFloorlet & rhs )
        : capletOrFloorlet_(rhs.capletOrFloorlet_), annuityFactor_(rhs.annuityFactor_), liborRate_(rhs.liborRate_), strike_(rhs.strike_), vol_(rhs.vol_), timeToExpiry_(rhs.timeToExpiry_), shift_(rhs.shift_), volatilityType_(rhs.volatilityType_)
    {
	}

    // Assignment Operator
    CapletFloorlet & CapletFloorlet::operator=( const CapletFloorlet & rhs )
    {
		// For Exception Safety

		// 1. Make a temp copy
		CapletFloorlet temp(rhs);

		// 2. Swap Data Members with the temp copy
		std::swap( capletOrFloorlet_,   temp.capletOrFloorlet_);
		std::swap( annuityFactor_,      temp.annuityFactor_ );
        std::swap( liborRate_,          temp.liborRate_ );
        std::swap( strike_,             temp.strike_ );
        std::swap( vol_,                temp.vol_ );
        std::swap( timeToExpiry_,       temp.timeToExpiry_ );
        std::swap( shift_,              temp.shift_ );
        std::swap( volatilityType_,     temp.volatilityType_);

        return *this;
    }


    // Price Accessor using the Constructor Input Parameters
    double CapletFloorlet::price() const
    {
        const double price = this->price( capletOrFloorlet_, annuityFactor_, liborRate_, strike_, vol_, timeToExpiry_, shift_, volatilityType_ );
        return price;
    }

    // Calculate the Black-Scholes price providing all the inputs
    double CapletFloorlet::price( const CapletFloorletEnum & capletOrFloorlet, const double & annuityFactor, const double & liborRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift, const VolatilityTypeEnum & volatilityType )
    {
		const auto callPut = (capletOrFloorlet == CAPLET_OPTION) ? CALL_OPTION : PUT_OPTION;
		
		double price = BlackModelUtils::blackPrice(volatilityType, callPut, liborRate, strike, vol, timeToExpiry, shift, annuityFactor);
        
        return price;
    }

    // Target Function for the Implied Volatilty Solver
    double CapletFloorlet::updateVolAndReprice( const double & inputVol ) const
    {
        const double price = this->price( capletOrFloorlet(), annuityFactor(), liborRate(), strike(), inputVol, timeToExpiry(), shift() );
        return price;
    }


    // Solve for the Implied Volatility given the Black-Scholes Price
    // Important Note: We must pass the initial guess for the volatility to the Black-Scholes constructor
    double CapletFloorlet::calculateImpliedVol( const double & price ) const
    {
        // We allow negative prices to test for Put-Call Super-Symmetry
        //AQ_REQUIRE( AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( price ), "Black-Scholes price parameter cannot be negative" );

        // Solver Settings
        const double targetPrice = price;

        // Manaster and Koehler Seed Value - This is a good initial guess for vol
        //const double initialGuessForVol = pow( std::fabs( std::log( spot / strike ) + rate * time ) * 2.0 / time, 0.5 ); 

        // Use the volatility passed to the Black-Scholes constructor as the initial guess
        // NOTE THIS INITIAL VOL GUESS IS VALID FOR LOG-NORMAL VOL. USE FOR NORMAL VOL SHOULD BE TREATED WITH CAUTION

		// Set a lower bound for the volatility estimate of 10% for LOGNORMAL, 1% for NORMAL.
		const double lowerBoundOnVolatilityEstimate = volatilityType_ == LOGNORMAL_VOLATILITY ? 0.1 : 0.01;
        const double initialGuessForVolatility = BlackScholes::initialGuessForImpliedVol( liborRate_, strike_, timeToExpiry_, 0.0, shift_, lowerBoundOnVolatilityEstimate ); // rate = 0.0
        const double tolerance = 1e-10;
        const double shiftSize = 1e-4;
        const unsigned int maxIterations = 1000;


        // One-dimensional objective function used by the solver:
        // This lambda function captures the Black-Scholes class fixed parameters
        // The inputVolatility is the variable which the solver will adjust in order to obtain the targetPrice.

        // 
        // *** IMPORTANT *** 
        // Must include a dummy parameter before or after the 'this' parameter in the capture list to resolve a GCC compiler bug relating to lambda functions
        //
        auto targetFunction = [targetPrice, this]( const double & inputVol )
        {
			// When using the toms748 solver we must subtract the targetPrice from the result so that
			// the targetFunction evaluates to zero at the dataInstance.
            return this->updateVolAndReprice( inputVol ) - targetPrice;
        };

        // Solver Results Contain: Solution, nInterations and epsilon				
		// Use the toms748 dataInstance finder because it is more stable than NewtonRaphson for deep in/out of the money options (where the vega goes to zero)
        solvers::SolverResults results  = solvers::toms748( targetFunction, initialGuessForVolatility, tolerance, maxIterations );

        // The dPrice/dVol Jacobian gives the Black-Scholes Vega. Need multiply by 0.01, since Vega is for 1% change in vol.
        double jacobian = results.jacobian;
        double nInterations = results.numberOfIterations;
        double impliedVolatility = results.solution;
        double solverError = results.epsilon;

        return impliedVolatility;
    }

	
	BlackScholesGreeks CapletFloorlet::greeksAnalytical(const CapletFloorletEnum & capletOrFloorlet, const double& fwd, const double& strike, const double& vol, const double& timeToExpiry, const double & shift, const AnnuityTerm& annuityTerm, const VolatilityTypeEnum & volType)
	{
		const auto callPut = (capletOrFloorlet == CAPLET_OPTION) ? CALL_OPTION : PUT_OPTION;

		BlackScholesGreeks greeks = BlackModelUtils::blackGreeksAnalytical(volType, callPut, fwd, strike, vol, timeToExpiry, shift, annuityTerm);

		return greeks;
	}

	BlackScholesGreeks CapletFloorlet::greeksNumerical(const CapletFloorletEnum & capletOrFloorlet, const double& fwd, const double& strike, const double& vol, const double& timeToExpiry, const double & shift, const AnnuityTerm& annuityTerm, const VolatilityTypeEnum & volType,
													  const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump)
	{

		const auto callPut = (capletOrFloorlet == CAPLET_OPTION) ? CALL_OPTION : PUT_OPTION;

		etrading::NumericalGreekBump greekBump;
		greekBump.deltaBump = deltaBump;
		greekBump.gammaBump = gammaBump;
		greekBump.vegaBump = vegaBump;
		greekBump.thetaBump = thetaBump;
		greekBump.rhoBump = 0.0;

		BlackScholesGreeks greeks = BlackModelUtils::blackGreeksNumerical(volType, callPut, fwd, strike, vol, timeToExpiry, shift, annuityTerm, greekBump);

		return greeks;
	}


	

	

}