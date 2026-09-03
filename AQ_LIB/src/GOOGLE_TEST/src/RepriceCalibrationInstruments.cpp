// repriceCalibrationInstruments.cpp

/*
 * @brief			Reprice Curve Calibration Instruments
 * @Created:	    24th August 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "RepriceCalibrationInstruments.h"
#include "tryMeLWO.h"
#include "tryMeLWOSwapCreation.h"
#include "tryMeLWOSwapPricing.h"
#include "BuildSwapTradeFromGenerator.h"
#include "ExtractCurveCalibrationData.h"
#include "DataUtilities.h"	// For MLIB_TO_STRING macros

using etrading::CurveTypeEnum;

namespace google_test
{
    
    /* @brief			Function to Reprice Curve Callibration Instruments using Swap Generators. Curves must be loaded independently of this function.
    *                   The function will throw if an instrument does not reprice and also if no trades are repriced i.e. when no tenors in the tenorsToReprice are calibration points
    *
    *                   For an example of how to use this function, see project: GOOGLE_TEST -> EUROisCurveConsistency.cpp
    *
    *  @param [in]		calibrationInstrumentType:      The Calibration Market Data Type:       etrading::OIS_MARKETDATA
    *                                                                                           etrading::LIBOR_OIS_BASISSPREAD_MARKETDATA
    *                                                                                           etrading::SWAP_MARKETDATA
    *                                                                                           etrading::BASIS_SWAP_MARKETDATA
    *
    *  @param [in]		tenorsToReprice                 A list of calibration instrument tenors to reprice
    *                                                   Please specify here which instrument tenors in the curve were used for calibration and should be consistency testing
    *                                                   Enter as a std:set for example as { "1W", "1M", "2M", "3M", "4M", "5M", "6M", "9M", "1Y", "2Y" } 
    *
    *  @param [in]		lwoCurveObjectName	            The LWO Curve Object Name - Needed to access the LWO Curve Market Data Object
    *  @param [in]		curveCollection                 Needed for Swap Pricing
    *  @param [in]		swapEffectiveDateYYYYMMDD		The Swap Effective Date as a String with Format YYYYMMDD
    *  @param [in]		swapGenerator                   The LWO Swap Generator Namee - The Swap Generator is Preloaded on MLIBQ StartUp
    *  @param [in]		tolerance                       Test Tolerance
    *
    * @ return          The function will return nothing, but throws if the test fails and also if no tests were run i.e. when no tenors in the tenorsToReprice list are calibration points
    */
    void repriceCalibrationInstruments( const etrading::CurveMarketDataEnum & calibrationInstrumentType,
                                        const std::set<std::string> tenorsToReprice,
                                        const std::string & lwoCurveObjectName,
                                        const std::string & curveCollection,
                                        const std::string & swapEffectiveDateYYYYMMDD,
                                        const std::string & swapGenerator,
                                        const double & tolerance )
    {
        std::vector<std::string>    swapTenorsFromCurve;
        std::vector<double>         swapRatesFromCurve;

        switch ( calibrationInstrumentType )
        {
            case etrading::OIS_MARKETDATA:
            {
                swapTenorsFromCurve         = curveCalibrationOisSwapTenors( lwoCurveObjectName );
                swapRatesFromCurve          = curveCalibrationOisSwapParRates( lwoCurveObjectName );
                break;
            }
            case etrading::LIBOR_OIS_BASISSPREAD_MARKETDATA:
            {
                swapTenorsFromCurve         = curveCalibrationLiborOisTenors( lwoCurveObjectName );
                swapRatesFromCurve          = curveCalibrationLiborOisSpreads( lwoCurveObjectName );
                break;
            }
            case etrading::SWAP_MARKETDATA:
            {
                swapTenorsFromCurve         = curveCalibrationSwapTenors( lwoCurveObjectName );
                swapRatesFromCurve          = curveCalibrationSwapParRates( lwoCurveObjectName );
                break;
            }
            case etrading::BASIS_SWAP_MARKETDATA:
            {
                swapTenorsFromCurve         = curveCalibrationBasisSwapTenors( lwoCurveObjectName );
                swapRatesFromCurve          = curveCalibrationBasisSwapSpreads( lwoCurveObjectName );
                break;
            }
            // Unsupported calibrationInstrumentType Types
            default:
            {
                MLIB_THROW("Unsupported Calibration Instrument")
            }
        }

        // Check Tenors and ParRates have the same Dimensions
        MLIB_REQUIRE( swapTenorsFromCurve.size() == swapRatesFromCurve.size(), "Inconsistent Calibration Data: Inconsistent Number of Swap Tenors and Rates from Curve" )


        // Reprice Calibration Instruments
        size_t testCount = 0;
        for ( size_t i = 0; i < swapTenorsFromCurve.size(); ++i )
        {
            // =============================================================================================================
            // *** IMPORTANT *** We only wish to reprice and test calibration instruments with the following Maturity Tenors
            // =============================================================================================================
            
            // Don't try to reprice non-calibration instruments
            auto it = tenorsToReprice.find( swapTenorsFromCurve[i] );
            
            if ( it == tenorsToReprice.end() )
            {
                // Skip to the next maturity if not a calibration instrument
                continue;
            }
            
            // Use the BuildSwaptradeFromGenerator Helpers to Create the Swap from the Generator
            const std::string swapName                  = "SWAP" + MLIB_TO_STRING_FROM_SIZE_T( i );
            const etrading::LabelValueBlock curveLVB    = etrading::fromStringToLVB(curveCollection);

            // Create the Swap & Calculate the Instrument Par Rate or Par Spread
            const std::string swapObject                = google_test::createSwapCalibrationInstrument( swapName, swapGenerator, swapEffectiveDateYYYYMMDD, swapTenorsFromCurve[i] );
            
            double actualResult = 0.0;
            switch ( calibrationInstrumentType )
            {
                case etrading::OIS_MARKETDATA:
                {
                    // OIS PAR RATE - Percent
                    actualResult                        = validation_api::tryMeLWOSwapParRate( swapObject, curveLVB );
                    break;
                }
                case etrading::LIBOR_OIS_BASISSPREAD_MARKETDATA:
                {
                    // LIBOR-OIS BASIS - Basis Points
                    actualResult                        = validation_api::tryMeLWOSwapSpread( swapObject, curveLVB, etrading::LabelValueBlock(), "Leg1:Float" ) / 10000; // Basis Points
                    break;
                }
                case etrading::SWAP_MARKETDATA:
                {
                    // SWAP PAR RATE - Percent
                    actualResult                        = validation_api::tryMeLWOSwapParRate( swapObject, curveLVB );
                    break;
                }
                case etrading::BASIS_SWAP_MARKETDATA:
                {
                    // TENOR BASIS PAR SPREAD - Basis Points
                    actualResult                        = validation_api::tryMeLWOSwapSpread( swapObject, curveLVB ) / 10000; // Basis Points
                    break;
                }
                // Unsupported calibrationInstrumentType Types
                default:
                {
                    MLIB_THROW("Unsupported Calibration Instrument")
                }
            }
                        
            // Test For Calibration Repricing Failure - Compare the Actual and Expected Results
            const double expectedResult = swapRatesFromCurve[i];
            
            if ( fabs(actualResult - expectedResult) > tolerance )
            {
                    MLIB_THROW( "Swap Instrument " + swapTenorsFromCurve[i] + " does not reprice. Actual Result: " + std::to_string( static_cast<long double> (actualResult) ) + " Expected Result: " + std::to_string( static_cast<long double>(expectedResult) ) )
            }

            // Update the Test Count
            testCount++;
        }

        // Ensure at least 1 instrument was price tested
        MLIB_REQUIRE( testCount > 0, "Repricing Test Failure: No Calibration Instruments were Tested" )
        return;
    }

}