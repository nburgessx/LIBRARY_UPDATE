#pragma once
#include "CoreEnumerations.h"
#include "BlackScholes.h"
#include "BlackUtils.h"

namespace etrading
{
    class CapletFloorlet
    {
    
    public:

        // Constructor / Destructor
        CapletFloorlet() {};
        virtual ~CapletFloorlet() {};

        // Alternative Constructor
		CapletFloorlet( const CapletFloorletEnum& capletOrFloorlet,
                        const double & annuityFactor,
                        const double & liborRate,
					    const double & strike,
					    const double & vol,
					    const double & timeToExpiry,                                            // yearFraction from valuation date to expiry date
					    const double & shift = 0.0,                                             // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes. Defaults to zero.                
                        const VolatilityTypeEnum & volatilityType = LOGNORMAL_VOLATILITY );     // Volatility Type. Defaults to LogNormal Vol i.e. Black-76 Vol

        // Copy Constructor
        CapletFloorlet( const CapletFloorlet & rhs );

        // Assignment Operator
        CapletFloorlet & operator=( const CapletFloorlet & rhs );

        // Get Accessors
        virtual CapletFloorletEnum capletOrFloorlet() const         { return capletOrFloorlet_; };
        virtual double annuityFactor() const                        { return annuityFactor_; };
        virtual double liborRate() const                            { return liborRate_; };
        virtual double strike() const                               { return strike_; }
        virtual double vol() const                                  { return vol_; }
        virtual double timeToExpiry() const                         { return timeToExpiry_; }
        virtual double shift() const                                { return shift_; }
        virtual VolatilityTypeEnum volatilityType() const           { return volatilityType_; };

		// Price Accessor using the Constructor Input Parameters
        virtual double price() const;

        // Solve for the implied volatility given the price
        virtual double updateVolAndReprice( const double & inputVol ) const;
        virtual double calculateImpliedVol( const double & price ) const;

        // *** Static *** Pricing methods with respect to the Volaility Model Type e.g. Lognormal- or Normal volatility
        static double price( const CapletFloorletEnum & capOrFloorLet, const double & annuityFactor, const double & liborRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0, const VolatilityTypeEnum & volatilityType = LOGNORMAL_VOLATILITY );

		static BlackScholesGreeks greeksAnalytical(const CapletFloorletEnum & capletOrFloorlet, const double& fwd, const double& strike, const double& vol, const double& timeToExpiry, const double & shift, const AnnuityTerm& annuityTerm, const VolatilityTypeEnum & volType);
		static BlackScholesGreeks greeksNumerical(const CapletFloorletEnum & capletOrFloorlet, const double& fwd, const double& strike, const double& vol, const double& timeToExpiry, const double & shift, const AnnuityTerm& annuityTerm, const VolatilityTypeEnum & volType,
												const double & deltaBump, const double & gammaBump, const double & vegaBump, const double & thetaBump);

	private:

        CapletFloorletEnum capletOrFloorlet_;
        double annuityFactor_;
        double liborRate_;
        double strike_;
        double vol_;
        double timeToExpiry_;
        double shift_;
        VolatilityTypeEnum volatilityType_;

    };
}