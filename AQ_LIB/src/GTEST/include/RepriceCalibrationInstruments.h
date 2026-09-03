// repriceCalibrationInstruments.h

/*
 * @brief			Reprice Curve Calibration Instruments
 * @Created:	    24th August 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "AQLCoreTemplateType.h"
#include "CoreEnumerations.h"

namespace google_test
{
    /* @brief			Function to Reprice Curve Callibration Instruments using Swap Generators. Curves must be loaded independently of this function.
    *                   The function will throw if an instrument does not reprice and also if no trades are repriced i.e. when no tenors in the tenorsToReprice are calibration points
    *
    *                   For an example of how to use this function, see project: GTEST -> EUROisCurveConsistency.cpp
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
    *  @param [in]		swapGenerator                   The LWO Swap Generator Namee - The Swap Generator is Preloaded on AlgoQuantLib StartUp
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
                                        const double & tolerance );
}