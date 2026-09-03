#include "BlackScholes.h"
#include "NormalDistribution.h"
#include "ExceptionMacros.h"
#include "Solvers.h"
#include <cmath>
#include <algorithm>
#include <functional>
// AAD Library
#include <adept.h>

namespace etrading
{
    // Alternative Constructor
    // Note: We support and apply a shift parameter to accomodate Shifted-Lognormal / Displaced Diffusion processes
    // The shift is subtracted from both the spot and strike parameters
    BlackScholes::BlackScholes( const CallOrPutEnum & callOrPut,
                                const double & spot,
                                const double & strike,
                                const double & vol,
                                const double & time,
                                const double & rate,
                                const double & carry,
                                const double & shift ) // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes
        : callOrPut_( callOrPut ), spot_( spot + shift ), strike_( strike + shift ), vol_( vol ), time_( time ), rate_( rate ), carry_( carry ), shift_( shift )
    {
        AQ_REQUIRE( ( callOrPut == CALL_OPTION || callOrPut == PUT_OPTION ), 
                    "Black-Scholes CallOrPut parameter must be 'Call' or 'Put'" );

        AQ_REQUIRE( AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( spot_  ),    "Black-Scholes spot parameter cannot be negative" );
        AQ_REQUIRE( AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( strike_  ),  "Black-Scholes strike parameter cannot be negative" );
        AQ_REQUIRE( AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( time_ ),    "Black-Scholes time to expiry parameter cannot be negative" );
        
        // We allow negative volatility to test for Put-Call Super-Symmetry
        //AQ_REQUIRE( AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( vol_ ),     "Black-Scholes volatility parameter cannot be negative" );
	}

    // Copy Constructor
    BlackScholes::BlackScholes( const BlackScholes & rhs )
    : callOrPut_( rhs.callOrPut_ ), spot_( rhs.spot_ ), strike_( rhs.strike_ ), vol_( rhs.vol_ ), time_( rhs.time_ ), rate_( rhs.rate_ ), carry_( rhs.carry_ ), shift_( rhs.shift_ )
    {
    }
        

    // Assignment Operator
    BlackScholes & BlackScholes::operator=( const BlackScholes & rhs )
    {
        // For Exception Safety
            
        // 1. Make a temp copy
        BlackScholes temp( rhs );

        // 2. Swap Data Members with the temp copy

        // 2A. Input Parameters
        std::swap( callOrPut_,      temp.callOrPut_                 );
        std::swap( spot_,           temp.spot_                      );
        std::swap( strike_,         temp.strike_                    );
        std::swap( vol_,            temp.vol_                       );
        std::swap( time_,           temp.time_                      );
        std::swap( rate_,           temp.rate_                      );
        std::swap( carry_,          temp.carry_                     );
        std::swap( shift_,          temp.shift_                     );

        return *this;
    }


    // Black-Scholes Pricing & Risk
    // ----------------------------

	template<typename xdouble>
	ProbabilityFactors<xdouble> BlackScholes::calculateProbabilityFactors( const xdouble & spot,
																						const xdouble & strike,
																						const xdouble & vol, 
																						const xdouble & time,
																						const xdouble & timeToDelivery,
																						const xdouble & carry )
	{
		ProbabilityFactors<xdouble> probabilityFactors;

		// note: ln(F/K) = ln[[*exp((r-d)*t)/K] = ln(P/K) + (r - d) * t, where t is the "timeToDelivery"  which can be different from the "time" in option formula (timeToExpiry) 
		probabilityFactors.d1 = ( log( spot / strike ) +  carry * timeToDelivery + 0.5 * vol * vol * time ) / ( vol * sqrt( time ) );
        probabilityFactors.d2 = probabilityFactors.d1 - vol * sqrt( time );

        probabilityFactors.nd1 = standardNormalDistributionPDF( probabilityFactors.d1 );
        probabilityFactors.Nd1 = standardNormalDistribution( probabilityFactors.d1 );
        probabilityFactors.Nd2 = standardNormalDistribution( probabilityFactors.d2 );

        probabilityFactors.Nminusd1 = standardNormalDistribution( xdouble( -probabilityFactors.d1 ) );
        probabilityFactors.Nminusd2 = standardNormalDistribution( xdouble( -probabilityFactors.d2 ) );

		return probabilityFactors;
	}

	template<typename xdouble>
	xdouble BlackScholes::calculateSpotDiscountTerm( xdouble rate, xdouble carry, xdouble time )
	{
		return exp( ( carry - rate ) * time );
	}

	template<typename xdouble>
	xdouble BlackScholes::calculateStrikeDiscountTerm( xdouble rate, xdouble time)
	{
		return exp( - rate * time );
	}

	// Calculate the Black-Scholes price providing all the inputs
	template<typename xdouble>
    xdouble BlackScholes::price( CallOrPutEnum callOrPut, 
								 xdouble spot,
								 xdouble strike,
								 xdouble vol, 
								 xdouble time,
								 xdouble rate,
								 xdouble carry,
								 xdouble shift )
    {

		xdouble bsPrice =  price(callOrPut, spot, strike, vol, time, time, rate, carry, shift);

		return bsPrice;
    }

	// Static: Calculate the Black-Scholes Price, by passing in the discount time, which can be different from the time in expiry
	template<typename xdouble>
	xdouble BlackScholes::price(CallOrPutEnum callOrPut,
							xdouble spot,
							xdouble strike,
							xdouble vol,
							xdouble time,
							xdouble timeToDelivery,
							xdouble rate,
							xdouble carry,
							xdouble shift)
	{

		// Boundary Conditions
		// -----------------------------------

		// Manage the Black-Scholes zero value Boundaries by adding a small precision epsilon value to the underlying parameter
        if ( AQ_IS_EQUAL_ZERO( spot ) )       spot    += AQ_EPSILON;
        if ( AQ_IS_EQUAL_ZERO( strike ) )     strike  += AQ_EPSILON;
        if ( AQ_IS_EQUAL_ZERO( vol ) )        vol     += AQ_EPSILON;
        if ( AQ_IS_EQUAL_ZERO( time ) )       time    += AQ_EPSILON;

		// Black-Scholes Calculation Parameters
        // ------------------------------------
		ProbabilityFactors<xdouble> probabilityFactors = calculateProbabilityFactors( spot, strike, vol, time, timeToDelivery, carry );

        // Black-Scholes Formulae
        // ------------------------------------
		xdouble price = 0.0;

		xdouble spotDiscountTerm   = calculateSpotDiscountTerm( rate, carry, timeToDelivery);
        xdouble strikeDiscountTerm = calculateStrikeDiscountTerm( rate, timeToDelivery);

		xdouble callPutIndicator = ( callOrPut == CALL_OPTION ) ? 1.0 : -1.0;

        if ( callOrPut == CALL_OPTION )
        {
            // For Call Options
            price          = ( callPutIndicator * spot * probabilityFactors.Nd1 * spotDiscountTerm )
                            - ( callPutIndicator * strike * probabilityFactors.Nd2 * strikeDiscountTerm );
        }
        else
        {
            // For Put Options
            price          =  ( callPutIndicator * spot * probabilityFactors.Nminusd1 * spotDiscountTerm )
                            - ( callPutIndicator * strike * probabilityFactors.Nminusd2 * strikeDiscountTerm );
        }

		return price;
    }

	// Explicit instantiations
	template double BlackScholes::price<double>( CallOrPutEnum callOrPut, 
												 double spot,
												 double strike,
												 double vol, 
												 double time,
												 double rate,
												 double carry,
												 double shift );

	template adept::adouble BlackScholes::price<adept::adouble>( CallOrPutEnum callOrPut, 
																 adept::adouble spot,
																 adept::adouble strike,
																 adept::adouble vol, 
																 adept::adouble time,
																 adept::adouble rate,
																 adept::adouble carry,
																 adept::adouble shift );


	template<typename xdouble>
	xdouble BlackScholes::analyticDelta( CallOrPutEnum callOrPut, xdouble spotDiscountTerm, xdouble Nd1 )
	{
		xdouble deltaSpot = 0.0;
		if ( callOrPut == CALL_OPTION )
        {
			deltaSpot      = spotDiscountTerm * Nd1;
		}
		else
		{
			deltaSpot      = spotDiscountTerm * ( Nd1 - 1.0 );
		}
		return deltaSpot;
	}

	BlackScholesGreeks BlackScholes::priceAndGreeksAnalytical() const
	{								
		BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut_, spot_, strike_, vol_, time_, rate_, carry_, shift_ );
        return greeks;
	}


	double BlackScholes::price() const
	{
		return price( callOrPut_, spot_, strike_, vol_, time_, rate_, carry_, shift_ );
	}

	double BlackScholes::deltaSpot() const
	{
		BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut_, spot_, strike_, vol_, time_, rate_, carry_, shift_ );
		return greeks.deltaSpot;
	}

	double BlackScholes::deltaForward() const
	{
		BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut_, spot_, strike_, vol_, time_, rate_, carry_, shift_ ); 
		return greeks.deltaForward;
	}

	double BlackScholes::gamma() const
	{
		BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut_, spot_, strike_, vol_, time_, rate_, carry_, shift_ ); 
		return greeks.gamma;
	}

	double BlackScholes::vega() const
	{
		BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut_, spot_, strike_, vol_, time_, rate_, carry_, shift_ ); 
		return greeks.vega;
	}

	double BlackScholes::theta() const
	{
		BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut_, spot_, strike_, vol_, time_, rate_, carry_, shift_ );
		return greeks.theta;
	}
	
	double BlackScholes::rho() const
	{
		BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut_, spot_, strike_, vol_, time_, rate_, carry_, shift_ );
		return greeks.rho;
	}
	
    // Target Function for the Implied Volatilty Solver
    double BlackScholes::updateVolAndReprice( const double & inputVol ) const
    {
        // Return the price using the input vol
        // Note: All shift parameters has already been applied via the constructor to the spot and strike directly, so we set the shift to zero here
        const double unitPrice = this->price( callOrPut(), spot_, strike_, inputVol, time_, rate_, carry_, 0.0 ); // Shift = 0.0
    	return unitPrice;
    }


    // Solve for the Implied Volatility given the Black-Scholes Price
    // Important Note: We must pass the initial guess for the volatility to the Black-Scholes constructor
    double BlackScholes::calculateImpliedVol( const double & price ) const
    {
        try
        {
            // We allow negative prices to test for Put-Call Super-Symmetry
            //AQ_REQUIRE( AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( price ), "Black-Scholes price parameter cannot be negative" );

            // Solver Settings
            const double targetPrice                = price;

            // Manaster and Koehler Seed Value - This is a good initial guess for vol
            //const double initialGuessForVol = pow( std::fabs( std::log( spot / strike ) + rate * time ) * 2.0 / time, 0.5 ); 

            // Use the volatility passed to the Black-Scholes constructor as the initial guess
            const double initialGuessForVolatility  = this->vol();
            const double tolerance                  = 1e-10;
            const double shiftSize                  = 1e-4;
            const unsigned int maxIterations        = 1000;

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
            double jacobian                 = results.jacobian; 
            double nInterations             = results.numberOfIterations;
            double impliedVolatility        = results.solution;
            double solverError              = results.epsilon;

            return impliedVolatility;
        }
        catch( LACoreError &)
        {
            throw;
        }
        catch(...)
        {
            AQ_THROW( "Unable to solve for the Implied Volatility" )
        }
    }

    // Static Helper Method to Allow Calling Code to imply a good initial guess for volatility - Using Manaster and Koehler Seed Value
    double BlackScholes::initialGuessForImpliedVol( const double & spot, const double & strike, const double & time, const double & rate, const double & shift, const double & lowerBound )
    {
        double result = 0.1;

        try
        {
            if ( !AQ_IS_EQUAL_ZERO(strike+shift) )
            {
                // Manaster and Koehler Seed Value
                result = std::sqrt( std::fabs( std::log( (spot + shift) / (strike + shift) ) + rate * time ) * 2.0 / time );
            }
            else
            {
                // Divide by Zero Case
                result = std::sqrt( std::fabs( std::log( (spot + shift) / AQ_EPSILON ) + rate * time ) * 2.0 / time );
            }
        }
        catch ( ... )
        {
            // Return the lowerBound if the above seed fails for any reason
            return lowerBound;
        }

		return std::max( lowerBound, result );
    }

    // Numerical Risk
    // ---------------------------------------------------------------------
    

	double BlackScholes::numericalDeltaSpot(const double & bump) const
	{
		const double priceNoBump = this->price();
		const double priceUpBump = this->price( callOrPut_, spot_ + bump, strike_, vol_, time_, rate_, carry_, shift_ );

		const double deltaSpotNumerical = (priceUpBump - priceNoBump) / bump;
		return deltaSpotNumerical;
	}

	/*
	* Compute Black Scholes greeks numerically via AAD
	*/
	BlackScholesGreeks BlackScholes::priceAndGreeksAAD() const
	{
		adept::Stack stack;

		adept::adouble aspot( spot_ );
		adept::adouble astrike( strike_ );
		adept::adouble avol( vol_ );
		adept::adouble atime( time_ );
		adept::adouble arate( rate_ );
		adept::adouble acarry( carry_ );
		adept::adouble ashift( shift_ );

		stack.new_recording();

		const adept::adouble bsPrice = price( callOrPut_, aspot, astrike, avol, atime, arate, acarry, ashift ); 

		bsPrice.set_gradient( 1.0 );  // Calculate price derivatives
		stack.compute_adjoint();

		// Fetch price derivative with respect to each underlying variable of interest
		BlackScholesGreeks greeks;
		greeks.price = adept::value( bsPrice );
		greeks.deltaSpot = aspot.get_gradient();

		const double inverseSpotDiscountFactor = std::exp( -( carry() - rate() ) * time() );
		greeks.deltaForward = greeks.deltaSpot * inverseSpotDiscountFactor;

		greeks.vega = avol.get_gradient() / 100.;

		// Note the minus sign in Theta: Theta is defined as: What is the change in option value overnight, when there is one day LESS to expiry.
		greeks.theta = - atime.get_gradient() / 365.25;

		// Rho defined as a parallel bump in discount rate and carry
		greeks.rho   = ( arate.get_gradient()  + acarry.get_gradient() ) / 100.;

		//
		// Calculate Gamma by performing AAD on the AnalyticDelta
		//
		stack.clear_gradients();
		stack.new_recording();

		adept::adouble spotDiscountTerm = calculateSpotDiscountTerm( arate, acarry, atime );
		ProbabilityFactors<adept::adouble> probabilityFactors = calculateProbabilityFactors( aspot, astrike, avol, atime, atime, acarry );

		adept::adouble aDelta = analyticDelta( callOrPut_, spotDiscountTerm, probabilityFactors.Nd1 );

		aDelta.set_gradient( 1.0 );  // Calculate analytic delta derivatives
		stack.compute_adjoint();

		greeks.gamma = aspot.get_gradient();

		return greeks;
	}

	BlackScholesGreeks BlackScholes::priceAndGreeksNumerical( const double & deltaBump,
															  const double & gammaBump,
															  const double & vegaBump,
															  const double & rhoBump,
															  const double & thetaBump ) const
	{
		BlackScholesGreeks greeks;
		greeks.price        = price( callOrPut_, spot_, strike_, vol_, time_, rate_, carry_, shift_ );
		greeks.deltaSpot    = numericalDeltaSpot( deltaBump );
		greeks.deltaForward = numericalDeltaForward( deltaBump ); 
        greeks.rho          = numericalRho( rhoBump );
        greeks.theta        = numericalTheta( thetaBump );
        greeks.gamma        = numericalGamma( gammaBump );
        greeks.vega         = numericalVega( vegaBump );
        
		return greeks;
	}

    double BlackScholes::numericalDeltaForward( const double & bump ) const
    {
        const double inverseSpotDiscountFactor = std::exp( -( this->carry() - this->rate() ) * this->time() );
        const double deltaSpotNumerical = numericalDeltaSpot( bump );

        const double deltaForwardNumerical = deltaSpotNumerical * inverseSpotDiscountFactor ;
        return deltaForwardNumerical;
    }


    double BlackScholes::numericalVega( const double & bump ) const
    {
        const double priceNoBump    = this->price();
        const double priceUpBump    = this->price( callOrPut_, spot_, strike_, vol_ + bump, time_, rate_, carry_, shift_ );
    
        // Vega - Scale to 1% Shift
        const double vegaNumerical  = ( priceUpBump - priceNoBump ) / bump * 0.01;
        return vegaNumerical;
    }


    double BlackScholes::numericalGamma( const double & bump ) const
    {
        const double priceNoBump    = this->price();
        const double priceUpBump    = this->price( callOrPut_, spot_ + bump, strike_, vol_, time_, rate_, carry_, shift_ );
        const double priceDownBump  = this->price( callOrPut_, spot_ - bump, strike_, vol_, time_, rate_, carry_, shift_ );

        const double gammaNumerical = ( priceUpBump - 2 * priceNoBump + priceDownBump ) / ( bump * bump );
        return gammaNumerical;
    }


    double BlackScholes::numericalTheta( const double & bump ) const
    {
        // Bump Size should be 1 day i.e. 1 / 365.25
        // Note we bump up instead of down to manage the boundary condition when time to expiry is close to zero
        const double priceNoBump    = this->price();
        const double priceUpBump    = this->price( callOrPut_, spot_, strike_, vol_, time_ + bump, rate_, carry_, shift_ );

        const double thetaNumerical = priceNoBump - priceUpBump;
        return thetaNumerical;
    }


    double BlackScholes::numericalRho( const double & bump ) const
    {
        const double priceNoBump    = this->price();
        const double priceUpBump    = this->price( callOrPut_, spot_, strike_, vol_, time_, rate_ + bump, carry_ + bump, shift_ );

        // Rho - Scale to 1% Shift
        const double rhoNumerical = ( priceUpBump - priceNoBump ) / bump * 0.01;
        return rhoNumerical;
    }

	BlackScholesGreeks BlackScholes::priceAndGreeksNumerical(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift,
	    const double & deltaBump, const double & gammaBump,	const double & vegaBump, const double & rhoBump,	const double & thetaBump) 
	{
		BlackScholesGreeks greeks;
		greeks.deltaSpot = numericalDeltaSpot(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift, deltaBump);
		greeks.deltaForward = numericalDeltaForward(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift, deltaBump);
		greeks.rho = numericalRho(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift, rhoBump);
		greeks.theta = numericalTheta(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift, thetaBump);
		greeks.gamma = numericalGamma(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift, gammaBump);
		greeks.vega = numericalVega(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift, vegaBump);

		return greeks;
	}

	double BlackScholes::numericalDeltaSpot(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump)
	{
		const double priceNoBump = price(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift);
		const double priceUpBump = price(callOrPut, spot + bump, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift);

		const double deltaSpotNumerical = (priceUpBump - priceNoBump) / bump;
		return deltaSpotNumerical;
	}


	double BlackScholes::numericalDeltaForward(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump)
	{
		const double inverseSpotDiscountFactor = std::exp(-(carry - rate) * timeToDelivery);
		const double deltaSpotNumerical = numericalDeltaSpot(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift, bump);

		const double deltaForwardNumerical = deltaSpotNumerical * inverseSpotDiscountFactor;
		return deltaForwardNumerical;
	}


	double BlackScholes::numericalVega(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump)
	{
		const double priceNoBump = price(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift);
		const double priceUpBump = price(callOrPut, spot, strike, vol + bump, timeToExpiry, timeToDelivery, rate, carry, shift);

		// Vega - Scale to 1% Shift
		const double vegaNumerical = (priceUpBump - priceNoBump) / bump * 0.01;
		return vegaNumerical;
	}


	double BlackScholes::numericalGamma(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump)
	{
		const double priceNoBump = price(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift);

		const double priceUpBump = price(callOrPut, spot + bump, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift);
		const double priceDownBump = price(callOrPut, spot - bump, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift);

		const double gammaNumerical = (priceUpBump - 2 * priceNoBump + priceDownBump) / (bump * bump);
		return gammaNumerical;
	}


	double BlackScholes::numericalTheta(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump)
	{
		// Bump Size should be 1 day i.e. 1 / 365.25
		// Note we bump up instead of down to manage the boundary condition when time to expiry is close to zero
		const double priceNoBump = price(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift);
		const double priceUpBump = price(callOrPut, spot, strike, vol, timeToExpiry + bump, timeToDelivery + bump, rate, carry, shift);

		const double thetaNumerical = priceNoBump - priceUpBump;
		return thetaNumerical;
	}


	double BlackScholes::numericalRho(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump)
	{
		const double priceNoBump = price(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift);
		const double priceUpBump = price(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate + bump, carry + bump, shift);

		// Rho - Scale to 1% Shift
		const double rhoNumerical = (priceUpBump - priceNoBump) / bump * 0.01;
		return rhoNumerical;
	}
    
    // ---------------------------------------------------------------------------------------
    //
    // Static: Risk Functions
    //
    // ---------------------------------------------------------------------------------------

	BlackScholesGreeks BlackScholes::calculatePriceAndGreeks(CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift)
	{
		BlackScholesGreeks greeks = calculatePriceAndGreeks(callOrPut, spot, strike, vol, time, time, rate, carry, shift);
		
		return greeks;
	}

	// timeToDelivery (used in discounting) can be different from timeToExpiry
	BlackScholesGreeks BlackScholes::calculatePriceAndGreeks(CallOrPutEnum callOrPut, double spot, double strike, double vol, double timeToExpiry, double timeToDelivery, double rate, double carry, double shift)
	{
		BlackScholesGreeks greeks;
		greeks.price = price(callOrPut, spot, strike, vol, timeToExpiry, timeToDelivery, rate, carry, shift);

		const double spotDiscountTerm = calculateSpotDiscountTerm(rate, carry, timeToDelivery);
		const double strikeDiscountTerm = calculateStrikeDiscountTerm(rate, timeToDelivery);

		ProbabilityFactors<double> probabilityFactors = calculateProbabilityFactors(spot, strike, vol, timeToExpiry, timeToDelivery, carry);

		if ( callOrPut == CALL_OPTION )
        {

            // For Call Options            
            greeks.deltaSpot      = analyticDelta( callOrPut, spotDiscountTerm, probabilityFactors.Nd1 );
            greeks.deltaForward   = probabilityFactors.Nd1;
            
            greeks.rho            = timeToDelivery * strike * strikeDiscountTerm * probabilityFactors.Nd2;
            greeks.rho            = greeks.rho / 100.0;

            // Note nd1 is the PDF and ND1 the CDF
            greeks.theta          = -1.0 * ( spot * spotDiscountTerm * probabilityFactors.nd1 * vol ) / ( 2.0 * sqrt(timeToExpiry) )
									- ( carry - rate ) * spot * spotDiscountTerm * probabilityFactors.Nd1
									- ( rate * strike * strikeDiscountTerm * probabilityFactors.Nd2 );
            
            greeks.theta          = greeks.theta / 365.25;
        }
        else
        {
            // For Put Options
            greeks.deltaSpot      = analyticDelta( callOrPut, spotDiscountTerm, probabilityFactors.Nd1 );
            greeks.deltaForward   = ( probabilityFactors.Nd1 - 1.0 );
            
            greeks.rho            = -timeToDelivery * strike * strikeDiscountTerm * probabilityFactors.Nminusd2;
            greeks.rho            = greeks.rho / 100.0;

            // Note nd1 is the PDF and ND1 the CDF
            greeks.theta          = -1.0 * ( spot * spotDiscountTerm * probabilityFactors.nd1 * vol ) / ( 2.0 * sqrt(timeToExpiry) )
									+ ( carry - rate ) * spot * spotDiscountTerm * probabilityFactors.Nminusd1
									+ ( rate * strike * strikeDiscountTerm * probabilityFactors.Nminusd2 );
            
            greeks.theta          = greeks.theta / 365.25;
        }
                
        // Gamma - note nd1 here is the PDF not the CDF
        greeks.gamma = ( probabilityFactors.nd1 * spotDiscountTerm ) / ( spot * vol * sqrt(timeToExpiry) );

        // Vega
        greeks.vega = spot * spotDiscountTerm * probabilityFactors.nd1 * sqrt(timeToExpiry);
        
        greeks.vega = greeks.vega / 100.0;

		return greeks;
	}

    double BlackScholes::deltaSpot( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift )
    {
        BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut, spot, strike, vol, time, rate, carry, shift );
		return greeks.deltaSpot;
    }
    
    double BlackScholes::deltaForward( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift )
    {
        BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut, spot, strike, vol, time, rate, carry, shift );
        return greeks.deltaForward;
    }
    
    double BlackScholes::gamma( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift )
    {
        BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut, spot, strike, vol, time, rate, carry, shift );
        return greeks.gamma;
    }
    
    double BlackScholes::vega( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift )
    {
        BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut, spot, strike, vol, time, rate, carry, shift );
        return greeks.vega;
    }

    double BlackScholes::theta( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift )
    {
        BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut, spot, strike, vol, time, rate, carry, shift );
        return greeks.theta;
    }
    
    double BlackScholes::rho( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift )
    {
        BlackScholesGreeks greeks = calculatePriceAndGreeks( callOrPut, spot, strike, vol, time, rate, carry, shift );
        return greeks.rho;
    }


    // ============================ BLACK MODEL ( EQUIVALENT TO BLACK-SCHOLES WITH CARRY TERM = 0 ) ============================ 

    // Alternative Constructor
    // Note: We support and apply a shift parameter to accomodate Shifted-Lognormal / Displaced Diffusion processes
    // The shift is subtracted from both the spot and strike parameters
    BlackModel::BlackModel( const CallOrPutEnum & callOrPut,
                            const double & spot,
                            const double & strike,
                            const double & vol,
                            const double & time,
                            const double & rate,
                            const double & shift ) // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes
        : BlackScholes( callOrPut, spot, strike, vol, time, rate, 0, shift ) // Black Model: Carry Parameter set to Zero
    {
	}

    // Copy Constructor
    BlackModel::BlackModel( const BlackModel & rhs )
        : BlackScholes( rhs.callOrPut_, rhs.spot_, rhs.strike_, rhs.vol_, rhs.time_, rhs.rate_, rhs.carry_ , rhs.shift_ )
    {
    }
     
    // Assignment Operator
    BlackModel & BlackModel::operator=( const BlackModel & rhs )
    {
        // For Exception Safety
            
        // 1. Make a temp copy
        BlackModel temp( rhs );

        // 2. Swap Data Members with the temp copy

        // 2A. Input Parameters
        std::swap( callOrPut_,      temp.callOrPut_ );
        std::swap( spot_,           temp.spot_      );
        std::swap( strike_,         temp.strike_    );
        std::swap( vol_,            temp.vol_       );
        std::swap( time_,           temp.time_      );
        std::swap( rate_,           temp.rate_      );
        std::swap( carry_,          temp.carry_     );
        std::swap( shift_,          temp.shift_     );

        return *this;
    }
}