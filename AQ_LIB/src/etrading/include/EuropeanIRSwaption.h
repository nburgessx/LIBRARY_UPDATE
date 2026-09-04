// EuropeanIRSwaption.h

/*
* @brief			European Swaptions
*/

#pragma once
#include "CoreEnumerations.h"


namespace etrading
{
    class EuropeanIRSwaption
    {
    
    public:

        // Constructor / Destructor
        EuropeanIRSwaption() {};
        virtual ~EuropeanIRSwaption() {};

        // Alternative Constructor
        EuropeanIRSwaption( const PayerReceiverSwaptionEnum & payerReceiver,
                            const double & annuity,
                            const double & swapRate,
                            const double & strike,
                            const double & vol,
                            const double & timeToExpiry,
                            const double & shift = 0.0,                                             // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes. Defaults to zero.                
                            const VolatilityTypeEnum & volatilityType = LOGNORMAL_VOLATILITY );     // Volatility Type. Defaults to LogNormal Vol i.e. Black-76 Vol
                            

        // Copy Constructor
        EuropeanIRSwaption( const EuropeanIRSwaption & rhs );

        // Assignment Operator
        EuropeanIRSwaption & operator=( const EuropeanIRSwaption & rhs );

        // Get Accessors
        virtual PayerReceiverSwaptionEnum payerOrReceiver() const   { return payerReceiver_; };
        virtual double annuity() const                              { return annuity_; };
        virtual double swapRate() const                             { return swapRate_; }
        virtual double strike() const                               { return strike_; }
        virtual double vol() const                                  { return vol_; }
        virtual double timeToExpiry() const                         { return timeToExpiry_; }
        virtual double shift() const                                { return shift_; }
        virtual VolatilityTypeEnum volatilityType() const           { return volatilityType_; };

        // Price Accessor using the Constructor Input Parameters
        virtual double price() const;
        virtual double delta() const;
        virtual double gamma() const;
        virtual double vega() const;
        virtual double theta() const;

        // Solve for the implied volatility given the price
        virtual double updateVolAndReprice( const double & inputVol ) const;
        virtual double calculateImpliedVol( const double & price ) const;

        // *** Static ***
        // Pricing and Risk methods with respect to the Volaility Model Type e.g. Lognormal- or Normal volatility

        // *** Static *** Price
        static double price( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0, const VolatilityTypeEnum & volatilityType = LOGNORMAL_VOLATILITY );
        static double lognormalPrice( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0 );
        static double normalPrice( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry );

        // *** Static *** Delta
        static double delta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0, const VolatilityTypeEnum & volatilityType = LOGNORMAL_VOLATILITY );
        static double lognormalDelta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0 );
        static double normalDelta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry );

        // *** Static *** Gamma
        static double gamma( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0, const VolatilityTypeEnum & volatilityType = LOGNORMAL_VOLATILITY );
        static double lognormalGamma( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0 );
        static double normalGamma( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry );

        // *** Static *** Vega
        static double vega( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0, const VolatilityTypeEnum & volatilityType = LOGNORMAL_VOLATILITY );
        static double lognormalVega( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0 );
        static double normalVega( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry );

        // *** Static *** Theta
        static double theta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0, const VolatilityTypeEnum & volatilityType = LOGNORMAL_VOLATILITY );
        static double lognormalTheta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift = 0.0 );
        static double normalTheta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry );

        // Static Helper Method to Calculate the Cash Annuity - we assume constant notional here
        static double cashAnnuity( const double & notional, const double & swapRate, const unsigned int & nCouponsPerYear, const double & tenorInYears, const StubTypeEnum & stubType = SHORT_START_STUBTYPE );

    private:

        PayerReceiverSwaptionEnum payerReceiver_;
        double annuity_;
        double swapRate_;
        double strike_;
        double vol_;
        double timeToExpiry_;
        double shift_;
        VolatilityTypeEnum volatilityType_;
    };
}