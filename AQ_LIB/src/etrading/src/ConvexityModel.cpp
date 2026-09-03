// ConvexityModel.cpp

#include "ConvexityModel.h"
#include "ExceptionMacros.h"

#include <cmath>
#include <algorithm>
#include <functional>

namespace etrading
{
    // Convexity Helper Methods
    // ---------------------------------

    double Convexity::toNormalVolFromLognormalVol( const double & lognormalVol, const double underlyingRate )
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( lognormalVol ), "Volatility Cannot be Negative" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( underlyingRate ), "Underlying Rate Cannot be Negative" );
        const double normalVol = lognormalVol * underlyingRate;
        return normalVol;
    }

    double Convexity::toLognormalVolFromNormalVol( const double & normalVol, const double underlyingRate )
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( normalVol ), "Volatility Cannot be Negative" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( underlyingRate ), "Underlying Rate Cannot be Negative" );

        const double lognormalVol = normalVol / underlyingRate;        
        return lognormalVol;
    }

    double Convexity::toShiftedLognormalVolFromLognormalVol( const double & lognormalVol, const double underlyingRate, const double shiftSize )
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( lognormalVol ), "Volatility Cannot be Negative" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( underlyingRate ), "Underlying Rate Cannot be Negative" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( shiftSize ), "ShiftSize Cannot be Negative" );

        const double shiftedLognormalVol = ( lognormalVol * underlyingRate ) / ( underlyingRate + shiftSize );
        return shiftedLognormalVol;
    }

    double Convexity::toLognormalVolFromShiftedLognormalVol( const double & shiftedLognormalVol, const double underlyingRate, const double shiftSize )
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( shiftedLognormalVol ), "Volatility Cannot be Negative" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( underlyingRate ), "Underlying Rate Cannot be Negative" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( shiftSize ), "ShiftSize Cannot be Negative" );

        const double lognormalVol = shiftedLognormalVol * ( underlyingRate + shiftSize ) / underlyingRate;
        return lognormalVol;
    }
    
    double Convexity::toShiftedLognormalVolFromNormalVol( const double & normalVol, const double underlyingRate, const double shiftSize )
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( normalVol ), "Volatility Cannot be Negative" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( underlyingRate + shiftSize ), "The Sum Total of the UnderlyingRate + ShiftSize Cannot be Negative" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( shiftSize ), "ShiftSize Cannot be Negative" );

        const double shiftedLognormalVol = normalVol / ( underlyingRate + shiftSize );
        return shiftedLognormalVol;
    }
    
    double Convexity::toNormalVolFromShiftedLognormalVol( const double & shiftedLognormalVol, const double underlyingRate, const double shiftSize )
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( shiftedLognormalVol ), "Volatility Cannot be Negative" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( underlyingRate + shiftSize ), "The Sum Total of the UnderlyingRate + ShiftSize Cannot be Negative" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( shiftSize ), "ShiftSize Cannot be Negative" );

        const double normalVol = shiftedLognormalVol * ( underlyingRate + shiftSize );
        return normalVol;
    }


    // Volatility Input Parameter Class
    // --------------------------------
    typedef Convexity::VolatilityParameters VolatilityParameters;

    
    // Default Constructor
    VolatilityParameters::VolatilityParameters()
        : volatility_(0.0), volatilityType_( etrading::NONE_VOLATILITY), volatilityShiftSize_(0.0)
    {
    }

    // Constructor
    VolatilityParameters::VolatilityParameters( const double & volatility, const VolatilityTypeEnum & volatilityType, const double & volatilityShiftSize )
        : volatility_(volatility), volatilityType_(volatilityType), volatilityShiftSize_(volatilityShiftSize)
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( volatility_ ), "Volatility Cannot be Negative" );
        MLIB_REQUIRE( volatilityType_ != etrading::NONE_VOLATILITY, "Volatility Type not Provided" );
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( volatilityShiftSize ), "Volatility Shift Size Cannot be Negative" );
    }
    
    // Copy Constructor
    VolatilityParameters::VolatilityParameters( const VolatilityParameters & rhs )
        : volatility_(rhs.volatility_), volatilityType_(rhs.volatilityType_), volatilityShiftSize_(rhs.volatilityShiftSize_)
    {
    }
        
    // Assignment Operator
    VolatilityParameters & VolatilityParameters::operator=( const VolatilityParameters & rhs )
    {
        // For Exception Safety
            
        // 1. Make a temp copy
        Convexity::VolatilityParameters temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( volatility_,             temp.volatility_        );
        std::swap( volatilityType_,         temp.volatilityType_    );
        std::swap( volatilityShiftSize_,    temp.volatilityShiftSize_   );

        return *this;
    }
    
    

    // End of Convexity Helper Methods
    // ---------------------------------



    // Default Constructor
    ConvexityModel::ConvexityModel()
    {
        Convexity::VolatilityParameters defaultVolParameters;
        volatilityParameters_ = defaultVolParameters;
    }

    // Constructor
    ConvexityModel::ConvexityModel( const Convexity::VolatilityParameters & volatilityParameters )
        : volatilityParameters_( volatilityParameters )
    {
    }

    // Copy Constructor
    ConvexityModel::ConvexityModel( const ConvexityModel & rhs )
        : volatilityParameters_( rhs.volatilityParameters_ )
    {
    }
        
    // Assignment Operator
    ConvexityModel & ConvexityModel::operator=( const ConvexityModel & rhs )
    {
        // For Exception Safety
            
        // 1. Make a temp copy
        ConvexityModel temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( volatilityParameters_, temp.volatilityParameters_ );

        return *this;
    }


    /* @brief			Function to calculate the libor in arrears convexity adjustment
                        *** This is a special case Libor Adjustment, where our Libor rate fixes in-arrears on the accrual end date ***
    *  @param [in]		liborRate			    The natural Libor rate in %
    *  @param [in]		couponYearFraction		Coupon Year Fraction
    *  @param [in]		timeToMaturity	        The time to maturity for Libor Coupon
    *  @param [in]		useHullApproximation    Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjustment to the natural Libor rate in %
    */
    double ConvexityModel::liborConvexityAdjustmentInArrears( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const bool useHullApproximation ) const
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( couponYearFraction ), "Convexity Adjustment Error - CouponYearFraction cannot be Negative" );
        
        // Boundary Case: timeToMaturity < 0
        if ( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( timeToMaturity ) )
        {
            return 0.0;
        }

        double convexityAdjustment  = liborRateInArrears( liborRate, couponYearFraction, timeToMaturity, useHullApproximation ) - liborRate;
        return convexityAdjustment;
    }

    /* @brief			Function to calculate the libor in arrears convexity adjusted Libor Rate
                        *** This is a special case Libor Adjustment, where our Libor rate fixes in-arrears on the accrual end date ***
    *  @param [in]		liborRate			    The natural Libor rate in %
    *  @param [in]		couponYearFraction		Coupon Year Fraction
    *  @param [in]		timeToMaturity	        The time to maturity for Libor Coupon
    *  @param [in]		useHullApproximation    Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjusted Libor Rate in %
    */
    double ConvexityModel::liborRateInArrears(const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const bool useHullApproximation ) const
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( couponYearFraction ), "Convexity Adjustment Error - CouponYearFraction cannot be Negative" );

        // Boundary Case: timeToMaturity < 0
        if ( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( timeToMaturity ) )
        {
            return liborRate;
        }

        // Boundary Case: liborRate = 0
        if ( MLIB_IS_EQUAL_ZERO( liborRate ) )
        {
            return 0.0;
        }

        double convexityAdjustedRate    = 0.0;
        const double tau                = couponYearFraction;
        const double vol                = volatilityParameters_.volatility_;
        const double shiftSize          = volatilityParameters_.volatilityShiftSize_;

        switch ( volatilityParameters_.volatilityType_ )
        {
            case NONE_VOLATILITY:
            {
                MLIB_THROW("Convexity Adjustment Error - No Volatility Type Provided")
                break;
            }   
            case NORMAL_VOLATILITY:
            {
                // Divide by Zero Guard
                MLIB_REQUIRE( ! MLIB_IS_EQUAL_ZERO( liborRate * ( 1 + tau * liborRate ) ), "Convexity Adjustment Error - Didvide by Zero Error" );
                convexityAdjustedRate = liborRate * ( 1 + ( tau * vol * vol * timeToMaturity ) / ( liborRate * ( 1 + tau * liborRate ) ) );
                break;
            }
            case LOGNORMAL_VOLATILITY:
            {
                // Enforce Log Normal Requirement that State Variable must be Strictly Positive
                MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO ( liborRate ), "Convexity Adjustment Error - Invalid VolatilityType: LOGNORMAL volatility invalid for negative Libor rates, Use NORMAL or SHIFTED_LOGNORMAL instead." );

                // Divide by Zero Guard
                MLIB_REQUIRE( ! MLIB_IS_EQUAL_ZERO( 1 + tau * liborRate ), "Convexity Adjustment Error - Didvide by Zero Error" );
                
                if ( useHullApproximation )
                {
                    convexityAdjustedRate = liborRate * ( 1 + tau * liborRate * ( 1 + vol * vol * timeToMaturity ) ) / ( 1 + tau * liborRate );
                }
                else
                {
                    convexityAdjustedRate = liborRate * ( 1 + tau * liborRate * std::exp( vol * vol * timeToMaturity ) ) / ( 1 + tau * liborRate );
                }
                break;
            }
            case SHIFTED_LOGNORMAL_VOLATILITY:
            {
                // Enforce Shifted-Lognormal Requirement that State Variable + Shift must be Strictly Positive
                const double shiftedLiborRate = liborRate + shiftSize;
                MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO ( shiftedLiborRate ), "Convexity Adjustment Error - Invalid Volatility ShiftSize. The Shifted-Lognormal ShiftSize is too small; LiborRate + ShiftSize cannot be negative." );
                
                // Variance Term Adjustment
                const double shiftAdjustment = ( 2 * liborRate * shiftSize ) + ( shiftSize * shiftSize );
                const double varianceFactor = std::exp( vol * vol * timeToMaturity ) + shiftAdjustment;

                // Divide by Zero Guard
                MLIB_REQUIRE( ! MLIB_IS_EQUAL_ZERO( 1 + tau * shiftedLiborRate ), "Convexity Adjustment Error - Didvide by Zero Error" );
                convexityAdjustedRate = shiftedLiborRate * ( 1 + tau * shiftedLiborRate * varianceFactor ) / ( 1 + tau * shiftedLiborRate );

                // Adjust Convexity to be Relative to LiborRate instead of the Shifted-Libor Rate
                convexityAdjustedRate -= shiftSize;

                break;
            }
            default:
            {
                MLIB_THROW("Convexity Volatility Types - Only NORMAL, LOGNORMAL and SHIFTED_LOGNORMAL volatility supported")
            }
        }
        
        return convexityAdjustedRate;
    }

    /* @brief			Function to calculate the libor in arbitrary time convexity adjustment
    *  @param [in]		liborRate			                The natural Libor rate in %
    *  @param [in]		couponYearFraction		            Coupon Year Fraction
    *  @param [in]		timeToMaturity	                    The time to maturity for Libor Coupon
    *  @param [in]		fixingDateYearFraction	            Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In MLIBQ curves and swaps we have natural fixing date = accrual start.
    *  @param [in]		startDiscountFactor	                OIS Discount Factor on the Natural Libor Fixing Start Date. This is the Accrual Start Date when there is no fixing lag
    *  @param [in]		endDiscountFactor	                OIS Discount Factor on the Natural Libor Fixing End Date. This is the Accrual End Date when there is no fixing lag
    *  @param [in]		useHullApproximation                Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjustment to the natural Libor rate in %
    */
    double ConvexityModel::liborConvexityAdjustmentForArbitraryFixingDate( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & startDiscountFactor, const double & endDiscountFactor, const bool useHullApproximation ) const
    {
       MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( couponYearFraction ), "Convexity Adjustment Error - CouponYearFraction cannot be Negative" );
        
        // Boundary Case: timeToMaturity < 0
        if ( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( timeToMaturity ) )
        {
            return 0.0;
        }

        double convexityAdjustment  = liborRateForArbitraryFixingDate( liborRate, couponYearFraction, timeToMaturity, fixingDateYearFraction, startDiscountFactor, endDiscountFactor, useHullApproximation ) - liborRate;
        return convexityAdjustment;
    }

    /* @brief			Function to calculate the libor in arbitrary time convexity adjusted rate
    *  @param [in]		liborRate			                The natural Libor rate in %
    *  @param [in]		couponYearFraction		            Coupon Year Fraction
    *  @param [in]		timeToMaturity	                    The time to maturity for Libor Coupon
    *  @param [in]		fixingDateYearFraction	            Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In MLIBQ curves and swaps we have natural fixing date = accrual start.
    *  @param [in]		startDiscountFactor	                OIS Discount Factor on the Natural Libor Fixing Start Date. This is the Accrual Start Date when there is no fixing lag
    *  @param [in]		endDiscountFactor	                OIS Discount Factor on the Natural Libor Fixing End Date. This is the Accrual End Date when there is no fixing lag
    *  @param [in]		useHullApproximation                Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjustment to the natural Libor rate in %
    */
    double ConvexityModel::liborRateForArbitraryFixingDate( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & startDiscountFactor, const double & endDiscountFactor, const bool useHullApproximation ) const
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( couponYearFraction ), "Convexity Adjustment Error - CouponYearFraction cannot be Negative" );

        // Boundary Case: timeToMaturity < 0
        if ( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( timeToMaturity ) )
        {
            return liborRate;
        }

        // Boundary Case: liborRate = 0
        if ( MLIB_IS_EQUAL_ZERO( liborRate ) )
        {
            return 0.0;
        }

        double convexityAdjustedRate    = 0.0;
        const double accrualTau         = couponYearFraction;
        const double fixingTau          = fixingDateYearFraction;

        // Calculate the Libor-OIS Tenor Basis or Credit Spread, where Credit Spread = Libor(3M) - Libor(OIS)
        // i.e. Credit Spread = Natural Libor Rate - OIS Libor Rate Implied from Discount Factors
        const double riskFreeRate   = ( ( startDiscountFactor / endDiscountFactor ) - 1 ) / accrualTau;
        const double creditSpread   = liborRate - riskFreeRate;
        
        // Calculate the Libor Stub Rate for our arbitrary fixing date
        // We determine the stub rate by linearly interpolating the credit spread, which is the same as linear interpolation between the OIS and Libor Rate.
        const double liborStubRate  = riskFreeRate + ( fixingTau / accrualTau ) * creditSpread;

        // Use the main convexity adjustment method once we have the Libor stub rate
        convexityAdjustedRate = liborRateForArbitraryFixingDate( liborRate, couponYearFraction, timeToMaturity, fixingDateYearFraction, liborStubRate, useHullApproximation );
        return convexityAdjustedRate;
    }

    // Libor Convexity Adjustment for Arbitrary Time Adjustment
    /* @brief			Function to calculate the libor in arbitrary time convexity adjustment
    *  @param [in]		liborRate			                The natural Libor rate in %
    *  @param [in]		couponYearFraction		            Coupon Year Fraction
    *  @param [in]		timeToMaturity	                    The time to maturity for Libor Coupon
    *  @param [in]		fixingDateYearFraction	            Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In MLIBQ curves and swaps we have natural fixing date = accrual start.
    *  @param [in]		impliedLiborOrStubRate              The Libor stub rate for the arbitrary coupon period %
    *  @param [in]		useHullApproximation                Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjustment to the natural Libor rate in %
    */
    double ConvexityModel::liborConvexityAdjustmentForArbitraryFixingDate( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & impliedLiborOrStubRate, const bool useHullApproximation ) const
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( couponYearFraction ), "Convexity Adjustment Error - CouponYearFraction cannot be Negative" );
        
        // Boundary Case: timeToMaturity < 0
        if ( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( timeToMaturity ) )
        {
            return 0.0;
        }

        double convexityAdjustment  = liborRateForArbitraryFixingDate( liborRate, couponYearFraction, timeToMaturity, fixingDateYearFraction, impliedLiborOrStubRate, useHullApproximation ) - liborRate;
        return convexityAdjustment;
    }
        
    /* @brief			Function to calculate the libor in arbitrary time convexity adjusted rate
    *  @param [in]		liborRate			                The natural Libor rate in %
    *  @param [in]		couponYearFraction		            Coupon Year Fraction
    *  @param [in]		timeToMaturity	                    The time to maturity for Libor Coupon
    *  @param [in]		fixingDateYearFraction	            Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In MLIBQ curves and swaps we have natural fixing date = accrual start.
    *  @param [in]		impliedLiborOrStubRate	            The Libor stub rate for the arbitrary coupon period %
    *  @param [in]		useHullApproximation                Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjusted Libor Rate in %
    */
    double ConvexityModel::liborRateForArbitraryFixingDate( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & impliedLiborOrStubRate, const bool useHullApproximation ) const
    {
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( couponYearFraction ), "Convexity Adjustment Error - CouponYearFraction cannot be Negative" );

        // Boundary Case: timeToMaturity < 0
        if ( MLIB_IS_LESS_THAN_OR_EQUAL_TO_ZERO( timeToMaturity ) )
        {
            return liborRate;
        }

        // Boundary Case: liborRate = 0
        if ( MLIB_IS_EQUAL_ZERO( liborRate ) )
        {
            return 0.0;
        }

        double convexityAdjustedRate    = 0.0;
        // const double accrualTau         = couponYearFraction; <--- Unused Variable
        const double fixingTau          = fixingDateYearFraction;
        const double vol                = volatilityParameters_.volatility_;
        
        // Only apply shift when using shifted-lognormal volatility
        double shiftSize = 0.0;
        if ( volatilityParameters_.volatilityType_ == etrading::SHIFTED_LOGNORMAL_VOLATILITY )
        {
            shiftSize = volatilityParameters_.volatilityShiftSize_;
        }

        // Linear Rate Model: Slope Parameter, B
        // Use the Libor Stub Rate for our arbitrary fixing date to calculate this
        const double LRMslopeParameter  = fixingTau * impliedLiborOrStubRate / liborRate;

        switch ( volatilityParameters_.volatilityType_ )
        {
            case NONE_VOLATILITY:
            {
                MLIB_THROW("Convexity Adjustment Error - No Volatility Type Provided")
                break;
            }   
            case NORMAL_VOLATILITY:
            {
                // Divide by Zero Guard
                MLIB_REQUIRE( ! MLIB_IS_EQUAL_ZERO( liborRate * ( 1 + LRMslopeParameter * liborRate ) ), "Convexity Adjustment Error - Didvide by Zero Error" );
                convexityAdjustedRate = liborRate * ( 1 + ( LRMslopeParameter * vol * vol * timeToMaturity ) / ( liborRate * ( 1 + LRMslopeParameter * liborRate ) ) );
                break;
            }
            case LOGNORMAL_VOLATILITY:
            {
                // Enforce Log Normal Requirement that State Variable must be Strictly Positive
                MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO ( liborRate ), "Convexity Adjustment Error - Invalid VolatilityType: LOGNORMAL volatility invalid for negative Libor rates, Use NORMAL or SHIFTED_LOGNORMAL instead." );

                // Divide by Zero Guard
                MLIB_REQUIRE( ! MLIB_IS_EQUAL_ZERO( 1 + LRMslopeParameter * liborRate ), "Convexity Adjustment Error - Didvide by Zero Error" );
                if ( useHullApproximation )
                {
                    convexityAdjustedRate = liborRate * ( 1 + LRMslopeParameter * liborRate * ( 1 + vol * vol * timeToMaturity ) ) / ( 1 + LRMslopeParameter * liborRate );
                }
                else
                {
                    convexityAdjustedRate = liborRate * ( 1 + LRMslopeParameter * liborRate * std::exp( vol * vol * timeToMaturity ) ) / ( 1 + LRMslopeParameter * liborRate );
                }
                break;
            }
            case SHIFTED_LOGNORMAL_VOLATILITY:
            {
                // Enforce Shifted-Lognormal Requirement that State Variable + Shift must be Strictly Positive
                const double shiftedLiborRate = liborRate + shiftSize;
                MLIB_REQUIRE( MLIB_IS_GREATER_THAN_OR_EQUAL_TO_ZERO ( shiftedLiborRate ), "Convexity Adjustment Error - Invalid Volatility ShiftSize. The Shifted-Lognormal ShiftSize is too small; LiborRate + ShiftSize cannot be negative." );
                
                // Variance Term Adjustment
                const double shiftAdjustment = ( 2 * liborRate * shiftSize ) + ( shiftSize * shiftSize );
                const double varianceFactor = std::exp( vol * vol * timeToMaturity ) + shiftAdjustment;

                // Divide by Zero Guard
                MLIB_REQUIRE( ! MLIB_IS_EQUAL_ZERO( 1 + LRMslopeParameter * shiftedLiborRate ), "Convexity Adjustment Error - Didvide by Zero Error" );
                convexityAdjustedRate = shiftedLiborRate * ( 1 + LRMslopeParameter * shiftedLiborRate * varianceFactor ) / ( 1 + LRMslopeParameter * shiftedLiborRate );
                
                // Adjust Convexity to be Relative to LiborRate instead of the Shifted-Libor Rate
                convexityAdjustedRate -= shiftSize;

                break;
            }
            default:
            {
                MLIB_THROW("Convexity Volatility Types - Only NORMAL, LOGNORMAL and SHIFTED_LOGNORMAL volatility supported")
            }
        }
        
        return convexityAdjustedRate;
    }    

    // *** TODO: Implement Swap Convexity Adjustmnets for Arbitrary Time ***
    // ----------------

    // Swap Convexity Adjustment for Arbitrary Time Adjustment
    double ConvexityModel::swapConvexityAdjustmentForArbitraryFixingDate( const double & swapRate, const double & couponYearFraction, const double & timeToMaturity ) const
    {
        MLIB_THROW("Not supported - To be implemented");
        return 0.0;
    }
    
    // Swap Convexity Adjustment for Arbitrary Time Adjustment
    double ConvexityModel::swapRateForArbitraryFixingDate( const double & swapRate, const double & couponYearFraction, const double & timeToMaturity ) const
    {
        MLIB_THROW("Not supported - To be implemented");
        return 0.0;
    }

}