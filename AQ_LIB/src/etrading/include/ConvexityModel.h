// ConvexityModel.h

#pragma once
#include <string>
#include "AQLCoreTemplateType.h"
#include "CoreEnumerations.h"

namespace etrading
{
    // Convexity Helper Namespace
    // ---------------------------------
    namespace Convexity
    {

         // Volatility Helper Methods
        // --------------------------------
        double toNormalVolFromLognormalVol( const double & lognormalVol, const double underlyingRate );
        double toLognormalVolFromNormalVol( const double & normalVol, const double underlyingRate );
        
        double toShiftedLognormalVolFromLognormalVol( const double & lognormalVol, const double underlyingRate, const double shiftSize );
        double toLognormalVolFromShiftedLognormalVol( const double & shiftedLognormalVol, const double underlyingRate, const double shiftSize );

        double toShiftedLognormalVolFromNormalVol( const double & lognormalVol, const double underlyingRate, const double shiftSize );
        double toNormalVolFromShiftedLognormalVol( const double & shiftedLognormalVol, const double underlyingRate, const double shiftSize );

        // Volatility Input Parameter Class
        // --------------------------------
        class VolatilityParameters
        {
        public:
            VolatilityParameters();
            ~VolatilityParameters() {};
            
            // Constructor
            VolatilityParameters( const double & volatility, const VolatilityTypeEnum & volatilityType, const double & volatilityShiftSize = 0.0 );

            // Copy Constructor
            VolatilityParameters( const VolatilityParameters & rhs );
        
            // Assignment Operator
            VolatilityParameters & operator=( const VolatilityParameters & rhs );


            double              volatility_;
            VolatilityTypeEnum  volatilityType_;
            double              volatilityShiftSize_;
        };
       
    }


    // End of Convexity Helper Namespace
    // ---------------------------------

    class ConvexityModel
    {
    public:
        ConvexityModel();
        virtual ~ConvexityModel() {};

        // Constructor
        ConvexityModel( const Convexity::VolatilityParameters & volatilityParameters );

        // Copy Constructor
        ConvexityModel( const ConvexityModel & rhs );
        
        // Assignment Operator
        ConvexityModel & operator=( const ConvexityModel & rhs );


        /* @brief			Function to calculate the libor in arrears convexity adjustment
                            *** This is a special case Libor Adjustment, where our Libor rate fixes in-arrears on the accrual end date ***
        *  @param [in]		liborRate			    The natural Libor rate in %
        *  @param [in]		couponYearFraction		Coupon Year Fraction
        *  @param [in]		timeToMaturity	        The time to maturity for Libor Coupon
        *  @param [in]		useHullApproximation    Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
        *  @return			The convexity adjustment to the natural Libor rate in %
        */
        virtual double liborConvexityAdjustmentInArrears( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const bool useHullApproximation = false ) const;


        /* @brief			Function to calculate the libor in arrears convexity adjusted Libor Rate
                            *** This is a special case Libor Adjustment, where our Libor rate fixes in-arrears on the accrual end date ***
        *  @param [in]		liborRate			    The natural Libor rate in %
        *  @param [in]		couponYearFraction		Coupon Year Fraction
        *  @param [in]		timeToMaturity	        The time to maturity for Libor Coupon
        *  @param [in]		useHullApproximation    Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
        *  @return			The convexity adjusted Libor Rate in %
        */
        virtual double liborRateInArrears( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const bool useHullApproximation = false ) const;


        // Libor Convexity Adjustment for Arbitrary Time Adjustment
        /* @brief			Function to calculate the libor in arbitrary time convexity adjustment
        *  @param [in]		liborRate			                The natural Libor rate in %
        *  @param [in]		couponYearFraction		            Coupon Year Fraction
        *  @param [in]		timeToMaturity	                    The time to maturity for Libor Coupon
        *  @param [in]		fixingDateYearFraction	            Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In AlgoQuantLib curves and swaps we have natural fixing date = accrual start.
        *  @param [in]		startDiscountFactor	                OIS Discount Factor on the Natural Libor Fixing Start Date. This is the Accrual Start Date when there is no fixing lag
        *  @param [in]		endDiscountFactor	                OIS Discount Factor on the Natural Libor Fixing End Date. This is the Accrual End Date when there is no fixing lag
        *  @param [in]		useHullApproximation                Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
        *  @return			The convexity adjustment to the natural Libor rate in %
        */
        virtual double liborConvexityAdjustmentForArbitraryFixingDate( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & startDiscountFactor, const double & endDiscountFactor, const bool useHullApproximation = false ) const;
        
        /* @brief			Function to calculate the libor in arbitrary time convexity adjusted rate
        *  @param [in]		liborRate			                The natural Libor rate in %
        *  @param [in]		couponYearFraction		            Coupon Year Fraction
        *  @param [in]		timeToMaturity	                    The time to maturity for Libor Coupon
        *  @param [in]		fixingDateYearFraction	            Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In AlgoQuantLib curves and swaps we have natural fixing date = accrual start.
        *  @param [in]		startDiscountFactor	                OIS Discount Factor on the Natural Libor Fixing Start Date. This is the Accrual Start Date when there is no fixing lag
        *  @param [in]		endDiscountFactor	                OIS Discount Factor on the Natural Libor Fixing End Date. This is the Accrual End Date when there is no fixing lag
        *  @param [in]		useHullApproximation                Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
        *  @return			The convexity adjusted Libor Rate in %
        */
        virtual double liborRateForArbitraryFixingDate( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & startDiscountFactor, const double & endDiscountFactor, const bool useHullApproximation = false ) const;

        // Libor Convexity Adjustment for Arbitrary Time Adjustment
        /* @brief			Function to calculate the libor in arbitrary time convexity adjustment
        *  @param [in]		liborRate			                The natural Libor rate in %
        *  @param [in]		couponYearFraction		            Coupon Year Fraction
        *  @param [in]		timeToMaturity	                    The time to maturity for Libor Coupon
        *  @param [in]		fixingDateYearFraction	            Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In AlgoQuantLib curves and swaps we have natural fixing date = accrual start.
        *  @param [in]		impliedLiborOrStubRate              The Libor stub rate for the arbitrary coupon period %
        *  @param [in]		useHullApproximation                Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
        *  @return			The convexity adjustment to the natural Libor rate in %
        */
        virtual double liborConvexityAdjustmentForArbitraryFixingDate( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & impliedLiborOrStubRate, const bool useHullApproximation = false ) const;
        
        /* @brief			Function to calculate the libor in arbitrary time convexity adjusted rate
        *  @param [in]		liborRate			                The natural Libor rate in %
        *  @param [in]		couponYearFraction		            Coupon Year Fraction
        *  @param [in]		timeToMaturity	                    The time to maturity for Libor Coupon
        *  @param [in]		fixingDateYearFraction	            Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In AlgoQuantLib curves and swaps we have natural fixing date = accrual start.
        *  @param [in]		impliedLiborOrStubRate	            The Libor stub rate for the arbitrary coupon period %
        *  @param [in]		useHullApproximation                Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
        *  @return			The convexity adjusted Libor Rate in %
        */
        virtual double liborRateForArbitraryFixingDate( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & impliedLiborOrStubRate, const bool useHullApproximation = false ) const;

        // *** TODO: Implement Swap Convexity Adjustmnets for Arbitrary Time ***
        // ----------------

        // Swap Convexity Adjustment for Arbitrary Time Adjustment
        virtual double swapConvexityAdjustmentForArbitraryFixingDate( const double & swapRate, const double & couponYearFraction, const double & timeToMaturity ) const;
        virtual double swapRateForArbitraryFixingDate( const double & swapRate, const double & couponYearFraction, const double & timeToMaturity ) const;

    private:
        Convexity::VolatilityParameters volatilityParameters_;
    };
}
