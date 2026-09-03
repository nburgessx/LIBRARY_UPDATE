// EuropeanIRSwaption.cpp

/*
* @brief			European Swaptions
* @Created:		    5th December 2017
* @Author:			Nicholas Burgess
* @Department:	    Quant Research & Analytics
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/

#include "EuropeanIRSwaption.h"
#include "NormalDistribution.h"
#include "BlackScholes.h"
#include "ExceptionMacros.h"
#include "Solvers.h"
#include <cmath>
#include <algorithm>
#include <functional>

namespace etrading
{

    // Alternative Constructor
    EuropeanIRSwaption::EuropeanIRSwaption( const PayerReceiverSwaptionEnum& payerReceiver,
                                            const double & annuity,
                                            const double & swapRate,
                                            const double & strike,
                                            const double & vol,
                                            const double & timeToExpiry,
                                            const double & shift,
                                            const VolatilityTypeEnum & volatilityType )
        : payerReceiver_( payerReceiver ), annuity_( annuity ), swapRate_( swapRate ), strike_( strike ), vol_( vol ), timeToExpiry_( timeToExpiry ), shift_( shift ), volatilityType_( volatilityType )
    {
        MLIB_REQUIRE( ( payerReceiver_ == PAYER_SWAPTION || payerReceiver_ == RECEIVER_SWAPTION ),
            "European Swaption PayerReceiverSwaption parameter must be 'Payer' or 'Receiver'" );
    }

    // Copy Constructor
    EuropeanIRSwaption::EuropeanIRSwaption( const EuropeanIRSwaption & rhs )
        : payerReceiver_( rhs.payerReceiver_ ), annuity_( rhs.annuity_ ), swapRate_( rhs.swapRate_ ), strike_( rhs.strike_ ), vol_( rhs.vol_ ), timeToExpiry_( rhs.timeToExpiry_ ), shift_( rhs.shift_ ), volatilityType_( rhs.volatilityType_ )
    {
    }

    // Assignment Operator
    EuropeanIRSwaption & EuropeanIRSwaption::operator=( const EuropeanIRSwaption & rhs )
    {
        // For Exception Safety

        // 1. Make a temp copy
        EuropeanIRSwaption temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( payerReceiver_,  temp.payerReceiver_ );
        std::swap( annuity_,        temp.annuity_ );
        std::swap( swapRate_,       temp.swapRate_ );
        std::swap( strike_,         temp.strike_ );
        std::swap( vol_,            temp.vol_ );
        std::swap( timeToExpiry_,   temp.timeToExpiry_ );
        std::swap( shift_,          temp.shift_ );
        std::swap( volatilityType_, temp.volatilityType_);

        return *this;
    }


    // Calculate Lognormal Price
    double EuropeanIRSwaption::lognormalPrice( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift )
    {
        // Payer (Call), Receiver (Put)
        // Use Black-76 Model, Carry = 0.0;
        BlackScholes black76( ( payerReceiver == PAYER_SWAPTION ) ? CALL_OPTION : PUT_OPTION,
                                swapRate,   // Spot = SwapRate
                                strike,
                                vol,
                                timeToExpiry,
                                0.0,        // Zero Rate = 0.0 - We are discounting using the Annuity measure not the risk neutral savings account
                                0.0,        // Black-76 Carry = 0.0
                                shift );    // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes
        
        // Price
        const double price = annuity * black76.price();
        return price;
    }
    
    // Calculate the Normal Price
    double EuropeanIRSwaption::normalPrice( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry )
    {
        // Validation
         MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( timeToExpiry ), "Swaption time to expiry must be greater than zero" );
         
        // Normal Pricing Variables
        const double phi = ( payerReceiver == PAYER_SWAPTION ) ? 1.0 : -1.0;
        
        double volSqrtTime = vol * std::sqrt( timeToExpiry );
        if ( MLIB_IS_EQUAL_ZERO( volSqrtTime ) ) volSqrtTime = MLIB_EPSILON;    // Don't allow divide by zero
        
        const double d1 = ( swapRate - strike ) / ( volSqrtTime );
        const double Nd1 = standardNormalDistribution( phi * d1 );              // Standard Normal CDF i.e. N(phi.d1)
        const double nd1 = standardNormalDistributionPDF( d1 );                 // Standard Normal PDF i.e. n(d1) with no phi term
        
        // Normal Pricing Function
        const double price = annuity * ( ( phi * ( swapRate - strike ) * Nd1 ) + ( volSqrtTime * nd1 ) );
        
        return price;
    }

    // Price Accessor using the Constructor Input Parameters
    double EuropeanIRSwaption::price() const
    {
        const double price = this->price( payerReceiver_, annuity_, swapRate_, strike_, vol_, timeToExpiry_, shift_, volatilityType_ );
        return price;
    }


    // Calculate the Black-Scholes price providing all the inputs
    double EuropeanIRSwaption::price( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift, const VolatilityTypeEnum & volatilityType )
    {
        double price = 0.0;

        switch ( volatilityType )
        {
            case LOGNORMAL_VOLATILITY:

                // Volatility in Percent
                price = lognormalPrice( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry, 0.0 ); // Shift = 0.0
                break;

            case SHIFTED_LOGNORMAL_VOLATILITY:

                // Volatility in Percent
                price = lognormalPrice( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry, shift );
                break;

            case NORMAL_VOLATILITY:

                // Volatility in Basis Points
                price = normalPrice( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry ); // Note: No Shift term
                break;

            default:

                // Should never reach here
                MLIB_THROW("Invalid volatility type. Must be 'LOGNORMAL', 'SHIFTED_LOGNORMAL' or 'NORMAL'");
                break;
        }
        
        return price;
    }


    // Target Function for the Implied Volatilty Solver
    double EuropeanIRSwaption::updateVolAndReprice( const double & inputVol ) const
    {
        const double price = this->price( payerOrReceiver(), annuity(), swapRate(), strike(), inputVol, timeToExpiry(), shift(), volatilityType() );
        return price;
    }


    // Solve for the Implied Volatility given the Black-Scholes Price
    // Important Note: We must pass the initial guess for the volatility to the Black-Scholes constructor
    double EuropeanIRSwaption::calculateImpliedVol( const double & price ) const
    {
        try
        {
            // We allow negative prices to test for Put-Call Super-Symmetry
            //MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( price ), "Black-Scholes price parameter cannot be negative" );

            // Solver Settings
            const double targetPrice = price;

            // Manaster and Koehler Seed Value - This is a good initial guess for vol
            //const double initialGuessForVol = pow( std::fabs( std::log( spot / strike ) + rate * time ) * 2.0 / time, 0.5 ); 

            // Use the volatility passed to the Black-Scholes constructor as the initial guess
            // NOTE THIS INITIAL VOL GUESS IS VALID FOR LOG-NORMAL VOL. USE FOR NORMAL VOL SHOULD BE TREATED WITH CAUTION

		    // Set a lower bound for the volatility estimate of 10% for LOGNORMAL, 1% for NORMAL.
		    const double lowerBoundOnVolatilityEstimate     = volatilityType_ == LOGNORMAL_VOLATILITY ? 0.1 : 0.01;
            const double initialGuessForVolatility          = BlackScholes::initialGuessForImpliedVol( swapRate_, strike_, timeToExpiry_, 0.0, shift_, lowerBoundOnVolatilityEstimate ); // rate = 0.0
            const double tolerance                          = 1e-12 * annuity(); // *** Important *** Normalize the tolerance by notional when not working with unit prices;
            const double shiftSize                          = 1e-10;
            const unsigned int maxIterations                = 1000;

            // One-dimensional objective function used by the solver:
            // This lambda function captures the Black-Scholes class fixed parameters
            // The inputVolatility is the variable which the solver will adjust in order to obtain the targetPrice.

            // 
            // *** IMPORTANT *** 
            // Must include a dummy parameter before or after the 'this' parameter in the capture list to resolve a msvcc compiler bug relating to lambda functions
            //
            auto targetFunction = [targetPrice, this]( const double & inputVol )
            {
			    // When using the toms748 solver we must subtract the targetPrice from the result so that
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
        catch( LACoreError &)
        {
            throw;
        }
        catch(...)
        {
            MLIB_THROW( "Unable to solve for the Implied Volatility" )
        }
    }

    
    // Static Helper Method to Calculate the Cash Annuity - we assume constant notional here
    double EuropeanIRSwaption::cashAnnuity( const double & notional, const double & swapRate, const unsigned int & nCouponsPerYear, const double & tenorInYears, const StubTypeEnum & stubType )
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_ZERO( tenorInYears ), "Cash Annuity tenor must be greater than zero" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_ZERO( nCouponsPerYear ), "Cash Annuity number of coupon payments per year must be greater than zero" );

        double          cashAnnuity                     = 0.0;
        double          shortStartStubDiscountFactor    = 1.0;
        double          longStartStubDiscountFactor     = 1.0;
        double          discountFactorFromSwapRate      = 1.0;
        const double    couponYearFraction              = 1.0 / (double)( nCouponsPerYear );                        // Divide for zero check above
        unsigned int    nWholeCoupons                   = ( unsigned int )( tenorInYears * nCouponsPerYear );       // number of whole coupons
        double          shortStubYearFraction           = tenorInYears - ( nWholeCoupons * couponYearFraction );
        bool            hasStartStub                    = false;
        bool            hasEndStub                      = false;
        bool            isSingleCashflowStub            = MLIB_IS_EQUAL( shortStubYearFraction, tenorInYears );

        // Check for Stub Coupons
        // -------------------
        if ( !MLIB_IS_EQUAL_ZERO( std::fmod( tenorInYears, couponYearFraction ) ) )  // fmod is floating modulus
        {
            MLIB_REQUIRE( stubType!=NONE_STUBTYPE, "Cash Annuity has a stub in which case the stub type must be specified and cannot be 'NONE'." )
            
            if ( stubType == SHORT_START_STUBTYPE || stubType == LONG_START_STUBTYPE )
            {
                hasStartStub    = true;
                hasEndStub      = false;
            }
            else if ( stubType == SHORT_END_STUBTYPE || stubType == LONG_END_STUBTYPE )
            {
                hasStartStub    = false;
                hasEndStub      = true;
            }
        }


        // Short Start Stub - or Single Cashflow Stub
        // ----------------------------------------------------
        if ( hasStartStub && ( stubType == SHORT_START_STUBTYPE || isSingleCashflowStub ) )
        {
            // This discount factor - Single Compound Period: 1 / ( 1 + r.t )
            shortStartStubDiscountFactor = 1.0 / ( 1.0 + swapRate * shortStubYearFraction );

            // Update the cash annuity result
            cashAnnuity += notional * shortStubYearFraction * shortStartStubDiscountFactor;
        }


        // Short End Stub - or Single Cashflow Stub
        // ----------------------------------------------------
        if ( hasEndStub && ( stubType == SHORT_END_STUBTYPE || isSingleCashflowStub ) )
        {
            // This discount factor: 1 / ( 1 + r/m * n ) ^ m.n, where m = no Coupons per year and n = no years
            discountFactorFromSwapRate = 1.0 / ( std::pow( 1 + swapRate * couponYearFraction, (int)(nWholeCoupons) ) ); // df compounded to end of regular coupons
            discountFactorFromSwapRate *= 1.0 / ( 1.0 + swapRate * shortStubYearFraction );  // extra df compoundng to end of short end stub

            // Update the cash annuity result
            cashAnnuity += notional * shortStubYearFraction * discountFactorFromSwapRate;
        }


        // Regular Coupons and Long Stubs
        // ----------------------------------------------------
        for ( unsigned int i = 1; i <= nWholeCoupons; ++i )
        {
            // Long Start Stubs
            // -----------------
            if ( hasStartStub && stubType == LONG_START_STUBTYPE && i == 1 )
            {
                double longStubYearfraction = shortStubYearFraction + couponYearFraction;
                
                // This discount factor - Single Compound Period: 1 / ( 1 + r.t )
                longStartStubDiscountFactor = 1.0 / ( 1.0 + swapRate * longStubYearfraction );

                // Update the cash annuity result
                cashAnnuity += notional * longStubYearfraction * longStartStubDiscountFactor;

            }
            // Long End Stubs
            // -----------------
            else if ( hasEndStub && stubType == LONG_END_STUBTYPE && i == nWholeCoupons )
            {
                double longStubYearfraction = shortStubYearFraction + couponYearFraction;

                // This discount factor
                discountFactorFromSwapRate = 1.0 / std::pow( 1.0 + swapRate * couponYearFraction, (int)(i-1) ); // df to penultimate coupon end date
                discountFactorFromSwapRate *= 1.0 / ( 1.0 + swapRate * longStubYearfraction ); // df from last coupon start to  long end stub date

                // Update the cash annuity result
                cashAnnuity += notional * longStubYearfraction * discountFactorFromSwapRate;

            }
            // Regular Coupons
            // -------------------
            else
            {
                // Adjust regular discount factor by start stub discount factor if required
                if ( hasStartStub && stubType == SHORT_START_STUBTYPE )
                {
                        // This discount factor: 1 / ( 1 + r/m * n ) ^ m.n, where m = no Coupons per year and n = no years
                        // Must adjust by short start stub discount factor
                        discountFactorFromSwapRate = 1.0 / std::pow( 1.0 + swapRate * couponYearFraction, (int)(i) );
                        discountFactorFromSwapRate *= shortStartStubDiscountFactor;
                }
                else if ( hasStartStub && stubType == LONG_START_STUBTYPE )
                {
                    // This discount factor: 1 / ( 1 + r/m * n ) ^ m.n, where m = no Coupons per year and n = no years
                    // Must adjust by long start stub discount factor
                    discountFactorFromSwapRate = 1.0 / std::pow( 1.0 + swapRate * couponYearFraction, (int)(i-1) ); // Note: power exponent = i-1
                    discountFactorFromSwapRate *= longStartStubDiscountFactor;
                }
                else
                {
                    // This discount factor: 1 / ( 1 + r/m * n ) ^ m.n, where m = no Coupons per year and n = no years
                    discountFactorFromSwapRate = 1.0 / std::pow( 1.0 + swapRate * couponYearFraction, (int)(i) );
                }


                // Update the cash annuity result
                cashAnnuity += notional * couponYearFraction * discountFactorFromSwapRate;
            }
        }
                
        return cashAnnuity;
    }



    // -------------------------------------------------------------------------------------------------------
    //
    // Risk Calculations
    //
    // -------------------------------------------------------------------------------------------------------



    // Calculate Lognormal Delta
    double EuropeanIRSwaption::lognormalDelta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift )
    {
        // Payer (Call), Receiver (Put)
        // Use Black-76 Model, Carry = 0.0;
        const double black76Delta = BlackScholes::deltaSpot( ( payerReceiver == PAYER_SWAPTION ) ? CALL_OPTION : PUT_OPTION,
                                                               swapRate,   // Spot = SwapRate
                                                               strike,
                                                               vol,
                                                               timeToExpiry,
                                                               0.0,        // Zero Rate = 0.0 - We are discounting using the Annuity measure not the risk neutral savings account
                                                               0.0,        // Black-76 Carry = 0.0
                                                               shift );    // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes
        
        // Delta for a NEGATIVE 1 basis point change
        const double delta = annuity * black76Delta * -0.0001;
        return delta;
    }
    
    // Calculate the Normal Delta
    double EuropeanIRSwaption::normalDelta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry )
    {
        MLIB_THROW("EuropeanIRSwaption Delta using 'Normal' volatility not yet supported.")
        return 0.0;
    }

    // Virtual Class Delta calls the Static Delta Method
    double EuropeanIRSwaption::delta() const
    {
        const double delta = this->delta( payerReceiver_, annuity_, swapRate_, strike_, vol_, timeToExpiry_, shift_, volatilityType_ );
        return delta;
    }


    // Static Delta Method
    double EuropeanIRSwaption::delta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift, const VolatilityTypeEnum & volatilityType )
    {
        double delta = 0.0;

        switch ( volatilityType )
        {

            case LOGNORMAL_VOLATILITY:
            case SHIFTED_LOGNORMAL_VOLATILITY:

                // Volatility in Percent
                delta = lognormalDelta( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry, shift );
                break;

            case NORMAL_VOLATILITY:

                // Volatility in Basis Points
                delta = normalDelta( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry ); // Note: No Shift term
                break;

            default:

                // Should never reach here
                MLIB_THROW("Invalid volatility type. Must be 'LOGNORMAL', 'SHIFTED_LOGNORMAL' or 'NORMAL'");
                break;
        }
        
        return delta;
    }


    // Calculate Lognormal Gamma
    double EuropeanIRSwaption::lognormalGamma( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift )
    {
        // Payer (Call), Receiver (Put)
        // Use Black-76 Model, Carry = 0.0;
        const double black76Gamma = BlackScholes::gamma( ( payerReceiver == PAYER_SWAPTION ) ? CALL_OPTION : PUT_OPTION,
                                                          swapRate,   // Spot = SwapRate
                                                          strike,
                                                          vol,
                                                          timeToExpiry,
                                                          0.0,        // Zero Rate = 0.0 - We are discounting using the Annuity measure not the risk neutral savings account
                                                          0.0,        // Black-76 Carry = 0.0
                                                          shift );    // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes
        
        // Gamma for a 1 basis point change
        const double gamma = annuity * black76Gamma * 0.0001 * 0.0001;
        return gamma;
    }
    
    // Calculate the Normal Gamma
    double EuropeanIRSwaption::normalGamma( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry )
    {
        MLIB_THROW("EuropeanIRSwaption Gamma using 'Normal' volatility not yet supported.")
        return 0.0;
    }

    // Virtual Class Gamma calls the Static Gamma Method
    double EuropeanIRSwaption::gamma() const
    {
        const double gamma = this->gamma( payerReceiver_, annuity_, swapRate_, strike_, vol_, timeToExpiry_, shift_, volatilityType_ );
        return gamma;
    }


    // Static Gamma Method
    double EuropeanIRSwaption::gamma( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift, const VolatilityTypeEnum & volatilityType )
    {
        double gamma = 0.0;

        switch ( volatilityType )
        {
            case LOGNORMAL_VOLATILITY:
            case SHIFTED_LOGNORMAL_VOLATILITY:

                // Volatility in Percent
                gamma = lognormalGamma( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry, shift );
                break;

            case NORMAL_VOLATILITY:

                // Volatility in Basis Points
                gamma = normalGamma( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry ); // Note: No Shift term
                break;

            default:

                // Should never reach here
                MLIB_THROW("Invalid volatility type. Must be 'LOGNORMAL', 'SHIFTED_LOGNORMAL' or 'NORMAL'");
                break;
        }
        
        return gamma;
    }


    // Calculate Lognormal Vega
    double EuropeanIRSwaption::lognormalVega( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift )
    {
        // Payer (Call), Receiver (Put)
        // Use Black-76 Model, Carry = 0.0;
        const double black76Vega = BlackScholes::vega( ( payerReceiver == PAYER_SWAPTION ) ? CALL_OPTION : PUT_OPTION,
                                                        swapRate,   // Spot = SwapRate
                                                        strike,
                                                        vol,
                                                        timeToExpiry,
                                                        0.0,        // Zero Rate = 0.0 - We are discounting using the Annuity measure not the risk neutral savings account
                                                        0.0,        // Black-76 Carry = 0.0
                                                        shift );    // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes
        
        // Vega for a 1% Shift (same as black-76)
        const double vega = annuity * black76Vega;
        return vega;
    }
    
    // Calculate the Normal Vega
    double EuropeanIRSwaption::normalVega( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry )
    {
        MLIB_THROW("EuropeanIRSwaption Vega using 'Normal' volatility not yet supported.")
        return 0.0;
    }

    // Virtual Class Vega calls the Static Vega Method
    double EuropeanIRSwaption::vega() const
    {
        const double vega = this->vega( payerReceiver_, annuity_, swapRate_, strike_, vol_, timeToExpiry_, shift_, volatilityType_ );
        return vega;
    }


    // Static Vega Method
    double EuropeanIRSwaption::vega( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift, const VolatilityTypeEnum & volatilityType )
    {
        double vega = 0.0;

        switch ( volatilityType )
        {
            case LOGNORMAL_VOLATILITY:
            case SHIFTED_LOGNORMAL_VOLATILITY:

                // Volatility in Percent
                vega = lognormalVega( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry, shift );
                break;

            case NORMAL_VOLATILITY:

                // Volatility in Basis Points
                vega = normalVega( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry ); // Note: No Shift term
                break;

            default:

                // Should never reach here
                MLIB_THROW("Invalid volatility type. Must be 'LOGNORMAL', 'SHIFTED_LOGNORMAL' or 'NORMAL'");
                break;
        }
        
        return vega;
    }


    // Calculate Lognormal Theta
    double EuropeanIRSwaption::lognormalTheta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift )
    {
        // Payer (Call), Receiver (Put)
        // Use Black-76 Model, Carry = 0.0;
        const double black76Theta = BlackScholes::theta( ( payerReceiver == PAYER_SWAPTION ) ? CALL_OPTION : PUT_OPTION,
                                                          swapRate,   // Spot = SwapRate
                                                          strike,
                                                          vol,
                                                          timeToExpiry,
                                                          0.0,        // Zero Rate = 0.0 - We are discounting using the Annuity measure not the risk neutral savings account
                                                          0.0,        // Black-76 Carry = 0.0
                                                          shift );    // Shift Parameter: This is to support Shifted-Lognormal / Displaced Diffusion processes
        
        // Theta
        const double theta = annuity * black76Theta;
        return theta;
    }
    
    // Calculate the Normal Theta
    double EuropeanIRSwaption::normalTheta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry )
    {
        MLIB_THROW("EuropeanIRSwaption Theta using 'Normal' volatility not yet supported.")
        return 0.0;
    }

    // Virtual Class Theta calls the Static Theta Method
    double EuropeanIRSwaption::theta() const
    {
        const double theta = this->theta( payerReceiver_, annuity_, swapRate_, strike_, vol_, timeToExpiry_, shift_, volatilityType_ );
        return theta;
    }


    // Static Theta Method
    double EuropeanIRSwaption::theta( const PayerReceiverSwaptionEnum & payerReceiver, const double & annuity, const double & swapRate, const double & strike, const double & vol, const double & timeToExpiry, const double & shift, const VolatilityTypeEnum & volatilityType )
    {
        double theta = 0.0;

        switch ( volatilityType )
        {
            case LOGNORMAL_VOLATILITY:
            case SHIFTED_LOGNORMAL_VOLATILITY:

                // Volatility in Percent
                theta = lognormalTheta( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry, shift );
                break;

            case NORMAL_VOLATILITY:

                // Volatility in Basis Points
                theta = normalTheta( payerReceiver, annuity, swapRate, strike, vol, timeToExpiry ); // Note: No Shift term
                break;

            default:

                // Should never reach here
                MLIB_THROW("Invalid volatility type. Must be 'LOGNORMAL', 'SHIFTED_LOGNORMAL' or 'NORMAL'");
                break;
        }
        
        return theta;
    }

}