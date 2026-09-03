/*
 * @brief			Generalized Black-Scholes
 * @Created:		11th August 2017
 * @Author:			Nicholas Burgess
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once
#include "CoreEnumerations.h"

// AAD library
#include <adept.h>

namespace etrading
{
	struct BlackScholesGreeks
	{
		double price;
		double deltaSpot;
		double deltaForward;
		double gamma;
		double vega;
		double rho;
		double theta;
	};

	template<typename xdouble>
	struct ProbabilityFactors
	{
		xdouble d1;
		xdouble d2;
		xdouble nd1; // PDF
		xdouble Nd1; // CDF
		xdouble Nd2; // CDF
		xdouble Nminusd1;
		xdouble Nminusd2;
	};


    class BlackScholes
    {
    public:

        // Constructor / Destructor
        BlackScholes() {};
        virtual ~BlackScholes() {};
		
        // Alternative Constructor
        // Note: We support and apply a shift parameter to accomodate a shifted-lognormal / displaced diffusion processes
        // The shift is subtracted from both the spot and strike parameters
        BlackScholes( const CallOrPutEnum & callOrPut,
                      const double & spot,
                      const double & strike,
                      const double & vol,
                      const double & time,
                      const double & rate,
                      const double & carry,
                      const double & shift = 0.0 ); // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes. Defaults to zero.

        // Copy Constructors
        BlackScholes( const BlackScholes & rhs );
        
        // Assignment Operator
        BlackScholes & operator=( const BlackScholes & rhs );

        // Get Accessors
        virtual CallOrPutEnum callOrPut() const                     { return callOrPut_; }
		virtual double spot() const                                 { return spot_; }
        virtual double strike() const                               { return strike_; }
        virtual double vol() const                                  { return vol_; }
        virtual double time() const                                 { return time_; }
        virtual double rate() const                                 { return rate_; }
        virtual double carry() const                                { return carry_; }
        virtual double shift() const                                { return shift_; } // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes

         // Solve for the implied volatility given the price
        virtual double updateVolAndReprice( const double & inputVol ) const;  // Note we make a copy of the Black-Scholes class here before updating the vol, so that we have a thread-safe immutable Black-Scholes class
        virtual double calculateImpliedVol( const double & price ) const;     // Important Note: We must pass the initial guess for the volatility to the Black-Scholes constructor

        // Static Helper Method to Allow Calling Code to imply a good initial guess for volatility - Using Manaster and Koehler Seed Value
        static double initialGuessForImpliedVol( const double & spot, const double & strike, const double & time, const double & rate, const double & shift = 0.0, const double & lowerBound = 0.0 );

        // Black-Scholes Results
        virtual double price()  const;
		virtual BlackScholesGreeks priceAndGreeksAnalytical() const;
		virtual BlackScholesGreeks priceAndGreeksAAD() const;
		virtual BlackScholesGreeks priceAndGreeksNumerical( const double & deltaBump = 1.0e-5,
															const double & gammaBump = 1.0e-4,
															const double & vegaBump  = 1.0e-5,
															const double & rhoBump   = 1.0e-5,
															const double & thetaBump = 1.0 / 365.25 ) const;
		
		// Static: Calculate the Black-Scholes Price
		template<typename xdouble>
		static xdouble price( CallOrPutEnum callOrPut, 
							  xdouble spot,
							  xdouble strike,
							  xdouble vol, 
							  xdouble time,
							  xdouble rate,
							  xdouble carry,
							  xdouble shift );

		// Static: Calculate the Black-Scholes Price, by passing in the discount time, which can be different from the time to expiry
		template<typename xdouble>
		static xdouble price(CallOrPutEnum callOrPut,
							xdouble spot,
							xdouble strike,
							xdouble vol,
							xdouble time, //time to expiry, asOf to expiry
							xdouble timeInDiscounting, // time from valuation date to delivery date
							xdouble rate,
							xdouble carry,
							xdouble shift);

        // Static: Risk Functions
        static BlackScholesGreeks calculatePriceAndGreeks( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift );

		// timeToDelivery (used in discounting) can be different from timeToExpiry
		static BlackScholesGreeks calculatePriceAndGreeks(CallOrPutEnum callOrPut, double spot, double strike, double vol, double timeToExpiry, double timeToDelivery, double rate, double carry, double shift);

        static double deltaSpot( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift );
        static double deltaForward( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift );
        static double gamma( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift );
        static double vega( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift );
        static double theta( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift );
        static double rho( CallOrPutEnum callOrPut, double spot, double strike, double vol, double time, double rate, double carry, double shift );

		static BlackScholesGreeks priceAndGreeksNumerical(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & rhoBump, const double & thetaBump);
		static double numericalDeltaSpot(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump);
		static double numericalDeltaForward(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump);
		static double numericalVega(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump);
		static double numericalGamma(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump);
		static double numericalTheta(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump);
		static double numericalRho(const CallOrPutEnum& callOrPut, const double& spot, const double& strike, const double& vol, const double& timeToExpiry, const double& timeToDelivery, const double& rate, const double& carry, const double& shift, const double& bump);

		// Non-Static Risk Functions
        virtual double deltaSpot()  const;
        virtual double deltaForward() const;
        virtual double gamma()  const;
        virtual double vega()   const;
        virtual double theta()  const;
        virtual double rho()    const;

        // Black-Scholes Numerical Results
        virtual double numericalDeltaSpot( const double & bump ) const;
        virtual double numericalDeltaForward( const double & bump )  const;
        virtual double numericalVega( const double & bump )   const;
        virtual double numericalGamma( const double & bump )  const;
        virtual double numericalTheta( const double & bump )  const;
        virtual double numericalRho( const double & bump )    const;

		template<typename xdouble>
		static ProbabilityFactors<xdouble> calculateProbabilityFactors(const xdouble & spot,
																	const xdouble & strike,
																	const xdouble & vol,
																	const xdouble & time,
																	const xdouble & timeInDiscounting,  //which can be different from the time in expiry
																	const xdouble & carry);

    protected:
        
        // Input Parameters
        CallOrPutEnum   callOrPut_;
		double  spot_;
		double  strike_;
		double  vol_;
		double  time_;
		double  rate_;
		double  carry_;
		double  shift_;

    private:

		template<typename xdouble>
		static xdouble calculateSpotDiscountTerm( xdouble rate, xdouble carry, xdouble time );

		template<typename xdouble>
		static xdouble calculateStrikeDiscountTerm( xdouble rate, xdouble time);
		
		template<typename xdouble>
		static xdouble analyticDelta( CallOrPutEnum callOrPut, xdouble spotDiscountTerm, xdouble Nd1 );

    };

    // ============================ BLACK MODEL ( EQUIVALENT TO BLACK-SCHOLES WITH CARRY TERM = 0 ) ============================ 

    class BlackModel : public BlackScholes
    {
        // Constructor / Destructor
        BlackModel() {};
        virtual ~BlackModel() {};
		
        // Alternative Constructor
        // Note: We support and apply a shift parameter to accomodate a shifted-lognormal / displaced diffusion processes
        // The shift is subtracted from both the spot and strike parameters
        BlackModel( const CallOrPutEnum & callOrPut,
                    const double & spot,
                    const double & strike,
                    const double & vol,
                    const double & time,
                    const double & rate,
                    const double & shift = 0.0 ); // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes. Defaults to zero.

        // Copy Constructors
        BlackModel( const BlackModel & rhs );
        
        // Assignment Operator
        BlackModel & operator=( const BlackModel & rhs );
    };

}
