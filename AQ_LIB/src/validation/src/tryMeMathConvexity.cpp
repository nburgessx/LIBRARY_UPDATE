/*
 * @brief			Validation Interface for Convexity Adjustment Class
 * @Created:		12th January
 * @Author:			Nicholas Burgess
 * @Department:		MHI Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "tryMeMathConvexity.h"
#include "ConvexityModel.h"

// Includes for Structured Exception Handling and Input/Output Logging
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"

// Namespaces for Input/Output Logging
using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation_api
{

    //
    // Function to convert from lognormal to normal volatility using the deterministic drift freezing approach
    //
    double tryMeMathVolatilityToNormalFromLognormal( const double & lognormalVol, const double underlyingRate )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( lognormalVol, underlyingRate );

        // Calculation
        const double result = etrading::Convexity::toNormalVolFromLognormalVol( lognormalVol, underlyingRate );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }
    
    //
    // Function to convert from normal to lognormal volatility using the deterministic drift freezing approach
    //
    double tryMeMathVolatilityToLognormalFromNormal( const double & normalVol, const double underlyingRate )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( normalVol, underlyingRate );

        // Calculation
        const double result = etrading::Convexity::toLognormalVolFromNormalVol( normalVol, underlyingRate );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

    //
    // Function to convert from Lognormal volatility to shifted lognormal volatility using the deterministic drift freezing approach
    //
    double tryMeMathVolatilityToShiftedLognormalFromLognormal( const double & lognormalVol, const double underlyingRate, const double shiftSize )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( lognormalVol, underlyingRate, shiftSize );

        // Calculation
        const double result = etrading::Convexity::toShiftedLognormalVolFromLognormalVol( lognormalVol, underlyingRate, shiftSize );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

    //
    // Function to convert from shifted lognormal volatility to Lognormal volatility using the deterministic drift freezing approach
    //
    double tryMeMathVolatilityToLognormalFromShiftedLognormal( const double & shiftedLognormalVol, const double underlyingRate, const double shiftSize )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( shiftedLognormalVol, underlyingRate, shiftSize );

        // Calculation
        const double result = etrading::Convexity::toLognormalVolFromShiftedLognormalVol( shiftedLognormalVol, underlyingRate, shiftSize );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

    //
    // Function to convert from normal volatility to shifted lognormal volatility using the deterministic drift freezing approach
    //
    double tryMeMathVolatilityToShiftedLognormalFromNormal( const double & normalVol, const double underlyingRate, const double shiftSize )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( normalVol, underlyingRate, shiftSize );

        // Calculation
        const double result = etrading::Convexity::toShiftedLognormalVolFromNormalVol( normalVol, underlyingRate, shiftSize );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

    //
    // Function to convert from shifted lognormal volatility to normal volatility using the deterministic drift freezing approach
    //
    double tryMeMathVolatilityToNormalFromShiftedLognormal( const double & shiftedLognormalVol, const double underlyingRate, const double shiftSize )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( shiftedLognormalVol, underlyingRate, shiftSize );

        // Calculation
        const double result = etrading::Convexity::toNormalVolFromShiftedLognormalVol( shiftedLognormalVol, underlyingRate, shiftSize );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }


    /* @brief			Function to calculate the libor in arrears convexity adjustment
                        *** This is a special case Libor Adjustment, where our Libor rate fixes in-arrears on the accrual end date ***
    *  @param [in]		liborRate			    The natural Libor rate in %
    *  @param [in]		couponYearFraction		Coupon Year Fraction
    *  @param [in]		timeToMaturity	        The time to maturity for Libor Coupon
    *  @param [in]		volatility              The volatility in %
    *  @param [in]		volatilityType          The volotility type: NORMAL, LOGNORMAL, SHIFTED_LOGNORMAL 
    *  @param [in]		volatilityShift         The volatility shift for SHIFTED_LOGNORMAL volatility only, defaults to zero
    *  @param [in]		useHullApproximation    Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjustment to the natural Libor rate in %
    */
    double tryMeMathLiborConvexityAdjustmentInArrears( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & volatility, const etrading::VolatilityTypeEnum & volatilityType, const double & volatilityShift, const bool useHullApproximation )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( liborRate, couponYearFraction, timeToMaturity, volatility, volatilityType, volatilityShift );

        etrading::Convexity::VolatilityParameters volInputs( volatility, volatilityType, volatilityShift );
        etrading::ConvexityModel convexityModel( volInputs );

        const double result = convexityModel.liborConvexityAdjustmentInArrears( liborRate, couponYearFraction, timeToMaturity, useHullApproximation );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

    /* @brief			Function to calculate the libor in arrears convexity adjusted Libor Rate
                        *** This is a special case Libor Adjustment, where our Libor rate fixes in-arrears on the accrual end date ***
    *  @param [in]		liborRate			    The natural Libor rate in %
    *  @param [in]		couponYearFraction		Coupon Year Fraction
    *  @param [in]		timeToMaturity	        The time to maturity for Libor Coupon
    *  @param [in]		volatility              The volatility in %
    *  @param [in]		volatilityType          The volotility type: NORMAL, LOGNORMAL, SHIFTED_LOGNORMAL 
    *  @param [in]		volatilityShift         The volatility shift for SHIFTED_LOGNORMAL volatility only, defaults to zero
    *  @param [in]		useHullApproximation    Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjusted Libor rate in %
    */
    double tryMeMathLiborRateInArrears( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & volatility, const etrading::VolatilityTypeEnum & volatilityType, const double & volatilityShift, const bool useHullApproximation )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( liborRate, couponYearFraction, timeToMaturity, volatility, volatilityType, volatilityShift );

        etrading::Convexity::VolatilityParameters volInputs( volatility, volatilityType, volatilityShift );
        etrading::ConvexityModel convexityModel( volInputs );

        const double result = convexityModel.liborRateInArrears( liborRate, couponYearFraction, timeToMaturity, useHullApproximation );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

    /* @brief			Function to calculate the libor in arbitrary time convexity adjustment
    *  @param [in]		liborRate			            The natural Libor rate in %
    *  @param [in]		couponYearFraction		        Coupon Year Fraction
    *  @param [in]		timeToMaturity	                The time to maturity for Libor Coupon
    *  @param [in]		fixingDateYearFraction	        Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In MLIBQ curves and swaps we have natural fixing date = accrual start.
    *  @param [in]		startDiscountFactor	            OIS Discount Factor on the Natural Libor Fixing Start Date. This is the Accrual Start Date when there is no fixing lag
    *  @param [in]		endDiscountFactor	            OIS Discount Factor on the Natural Libor Fixing End Date. This is the Accrual End Date when there is no fixing lag
    *  @param [in]		volatility                      The volatility in %
    *  @param [in]		volatilityType                  The volotility type: NORMAL, LOGNORMAL, SHIFTED_LOGNORMAL 
    *  @param [in]		volatilityShift                 The volatility shift for SHIFTED_LOGNORMAL volatility only, defaults to zero
    *  @param [in]		useHullApproximation            Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjustment to the natural Libor rate in %
    */
    double tryMeMathLiborConvexityAdjustmentForArbitraryFixingDate( const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & startDiscountFactor, const double & endDiscountFactor, const double & volatility, const etrading::VolatilityTypeEnum & volatilityType, const double & volatilityShift, const bool useHullApproximation )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( liborRate, couponYearFraction, timeToMaturity, volatility, volatilityType, volatilityShift );

        etrading::Convexity::VolatilityParameters volInputs( volatility, volatilityType, volatilityShift );
        etrading::ConvexityModel convexityModel( volInputs );

        const double result = convexityModel.liborConvexityAdjustmentForArbitraryFixingDate( liborRate, couponYearFraction, timeToMaturity, fixingDateYearFraction, startDiscountFactor, endDiscountFactor, useHullApproximation  );

        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }
    
    /* @brief			Function to calculate the libor in arbitrary time convexity adjusted Libor Rate
    *  @param [in]		liborRate			            The natural Libor rate in %
    *  @param [in]		couponYearFraction		        Coupon Year Fraction
    *  @param [in]		timeToMaturity	                The time to maturity for Libor Coupon
    *  @param [in]		fixingDateYearFraction	        Fixing Year Fraction from Accrual Start to Fixing Date. More precisely from the natural fixing to the unnatural fixing. In MLIBQ curves and swaps we have natural fixing date = accrual start.
    *  @param [in]		startDiscountFactor	            OIS Discount Factor on the Natural Libor Fixing Start Date. This is the Accrual Start Date when there is no fixing lag
    *  @param [in]		endDiscountFactor	            OIS Discount Factor on the Natural Libor Fixing End Date. This is the Accrual End Date when there is no fixing lag
    *  @param [in]		volatility                      The volatility in %
    *  @param [in]		volatilityType                  The volotility type: NORMAL, LOGNORMAL, SHIFTED_LOGNORMAL 
    *  @param [in]		volatilityShift                 The volatility shift for SHIFTED_LOGNORMAL volatility only, defaults to zero
    *  @param [in]		useHullApproximation            Use the Hull Approximation, Defaults to false: In the log-normal case the expontential term exp(x) is approximated as ( 1 + x )
    *  @return			The convexity adjusted Libor rate in %
    */
    double tryMeMathLiborRateForArbitraryFixingDate(const double & liborRate, const double & couponYearFraction, const double & timeToMaturity, const double & fixingDateYearFraction, const double & startDiscountFactor, const double & endDiscountFactor, const double & volatility, const etrading::VolatilityTypeEnum & volatilityType, const double & volatilityShift, const bool useHullApproximation )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( liborRate, couponYearFraction, timeToMaturity, volatility, volatilityType, volatilityShift );

        etrading::Convexity::VolatilityParameters volInputs( volatility, volatilityType, volatilityShift );
        etrading::ConvexityModel convexityModel( volInputs );

        const double result = convexityModel.liborRateForArbitraryFixingDate( liborRate, couponYearFraction, timeToMaturity, fixingDateYearFraction, startDiscountFactor, endDiscountFactor, useHullApproximation );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
    }

}