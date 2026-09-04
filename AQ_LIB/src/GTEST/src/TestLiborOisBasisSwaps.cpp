// Include: Google Test Library
#include <gTest/gTest.h>

#include "Dependency.h"
#include "ReadDataFile.h"
#include "GetGoogleTestFolder.h"
#include "ResultsProcessor.h"
#include "tryAqObjects.h"
#include "tryAqObjSwapsPricing.h"

namespace
{
    const std::string googleTestFolder      = etrading::getGoogleTestFolder();

    const std::string OIScurveUSD           = googleTestFolder + "ETrading/Trades/TestLiborOISBasisSwaps/USD/USD_OIS_CURVE.JSON";
    const std::string STDcurveUSD           = googleTestFolder + "ETrading/Trades/TestLiborOISBasisSwaps/USD/USD_SWAP_3M_CURVE.JSON";
    const std::string liborOISTradeUSD      = googleTestFolder + "ETrading/Trades/TestLiborOISBasisSwaps/USD/USD_SWAP";
    
    const std::string OIScurveEUR           = googleTestFolder + "ETrading/Trades/TestLiborOISBasisSwaps/EUR/EUR_OIS_CURVE.JSON";
    const std::string STDcurveEUR           = googleTestFolder + "ETrading/Trades/TestLiborOISBasisSwaps/EUR/EUR_SWAP_3M_CURVE.JSON";
    const std::string liborOISTradeEUR      = googleTestFolder + "ETrading/Trades/TestLiborOISBasisSwaps/EUR/EUR_SWAP";

    const std::string OIScurveGBP           = googleTestFolder + "ETrading/Trades/TestLiborOISBasisSwaps/GBP/GBP_OIS_CURVE.JSON";
    const std::string STDcurveGBP           = googleTestFolder + "ETrading/Trades/TestLiborOISBasisSwaps/GBP/GBP_SWAP_3M_CURVE.JSON";
    const std::string liborOISTradeGBP      = googleTestFolder + "ETrading/Trades/TestLiborOISBasisSwaps/GBP/GBP_SWAP";
}

namespace google_test
{
    // Declare Test Fixture Class
    DECLARE_TEST_FIXTURE( TestLiborOisSwaps )

    // Call Test Fixture Class
    TEST_F( TestLiborOisSwaps, CONSISTENCY_CheckLiborOisSwapsReprice_USD )
    {
        // Load Curves
        const std::string curveUSDOIS   = validation::tryAqObjLoad( OIScurveUSD, etrading::JSON );
        const std::string curveUSD3ML   = validation::tryAqObjLoad( STDcurveUSD, etrading::JSON );

        // Load Libor-OIS Swap Trades
        const std::string liborOis1     = validation::tryAqObjLoad( liborOISTradeUSD + "1.JSON" , etrading::JSON );
        const std::string liborOis2     = validation::tryAqObjLoad( liborOISTradeUSD + "2.JSON" , etrading::JSON );
        const std::string liborOis3     = validation::tryAqObjLoad( liborOISTradeUSD + "3.JSON" , etrading::JSON );
        const std::string liborOis4     = validation::tryAqObjLoad( liborOISTradeUSD + "4.JSON" , etrading::JSON );
        const std::string liborOis5     = validation::tryAqObjLoad( liborOISTradeUSD + "5.JSON" , etrading::JSON );
        const std::string liborOis6     = validation::tryAqObjLoad( liborOISTradeUSD + "6.JSON" , etrading::JSON );
        const std::string liborOis7     = validation::tryAqObjLoad( liborOISTradeUSD + "7.JSON" , etrading::JSON );
        const std::string liborOis8     = validation::tryAqObjLoad( liborOISTradeUSD + "8.JSON" , etrading::JSON );
        const std::string liborOis9     = validation::tryAqObjLoad( liborOISTradeUSD + "9.JSON" , etrading::JSON );
        const std::string liborOis10    = validation::tryAqObjLoad( liborOISTradeUSD + "10.JSON", etrading::JSON );
        
        // Valuation Settings LVB
        LabelValueBlock valuationSettings = LabelValueBlock::createLabelValueBlock( "CURVECOLLECTION", "USDYC" );

        // Get Par Spreads
        const double result1            = validation::tryAqObjSwapsParSpread( liborOis1,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result2            = validation::tryAqObjSwapsParSpread( liborOis2,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result3            = validation::tryAqObjSwapsParSpread( liborOis3,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result4            = validation::tryAqObjSwapsParSpread( liborOis4,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result5            = validation::tryAqObjSwapsParSpread( liborOis5,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result6            = validation::tryAqObjSwapsParSpread( liborOis6,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result7            = validation::tryAqObjSwapsParSpread( liborOis7,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result8            = validation::tryAqObjSwapsParSpread( liborOis8,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result9            = validation::tryAqObjSwapsParSpread( liborOis9,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result10           = validation::tryAqObjSwapsParSpread( liborOis10, valuationSettings, LabelValueBlock(), "Leg1:Float" );
        
        // Compare against Hard Coded Par Rates used in curve calibration instruments
        // These results should never be rebased, since this is a curve calibration arbitrage and consistency test
        const double tolerance = 1e-2;
        EXPECT_NEAR( result1,  37.2500, tolerance );
        EXPECT_NEAR( result2,  37.2500, tolerance );
        EXPECT_NEAR( result3,  37.3750, tolerance );
        EXPECT_NEAR( result4,  37.6875, tolerance );
        EXPECT_NEAR( result5,  39.5000, tolerance );
        EXPECT_NEAR( result6,  40.5000, tolerance );
        EXPECT_NEAR( result7,  41.5000, tolerance );
        EXPECT_NEAR( result8,  43.1250, tolerance );
        EXPECT_NEAR( result9,  44.4770, tolerance );
        EXPECT_NEAR( result10, 45.2500, tolerance );
    }

    // Call Test Fixture Class
    TEST_F( TestLiborOisSwaps, CONSISTENCY_CheckLiborOisSwapsReprice_EUR )
    {
        // Load Curves
        const std::string curveEUROIS   = validation::tryAqObjLoad( OIScurveEUR, etrading::JSON );
        const std::string curveEUR3ML   = validation::tryAqObjLoad( STDcurveEUR, etrading::JSON );

        // Load Libor-OIS Swap Trades
        const std::string liborOis1     = validation::tryAqObjLoad( liborOISTradeEUR + "1.JSON" , etrading::JSON );
        const std::string liborOis2     = validation::tryAqObjLoad( liborOISTradeEUR + "2.JSON" , etrading::JSON );
        const std::string liborOis3     = validation::tryAqObjLoad( liborOISTradeEUR + "3.JSON" , etrading::JSON );
        const std::string liborOis4     = validation::tryAqObjLoad( liborOISTradeEUR + "4.JSON" , etrading::JSON );
        const std::string liborOis5     = validation::tryAqObjLoad( liborOISTradeEUR + "5.JSON" , etrading::JSON );
        const std::string liborOis6     = validation::tryAqObjLoad( liborOISTradeEUR + "6.JSON" , etrading::JSON );
        const std::string liborOis7     = validation::tryAqObjLoad( liborOISTradeEUR + "7.JSON" , etrading::JSON );
        const std::string liborOis8     = validation::tryAqObjLoad( liborOISTradeEUR + "8.JSON" , etrading::JSON );
        const std::string liborOis9     = validation::tryAqObjLoad( liborOISTradeEUR + "9.JSON" , etrading::JSON );
        const std::string liborOis10    = validation::tryAqObjLoad( liborOISTradeEUR + "10.JSON", etrading::JSON );
        const std::string liborOis11    = validation::tryAqObjLoad( liborOISTradeEUR + "11.JSON", etrading::JSON );
        const std::string liborOis12    = validation::tryAqObjLoad( liborOISTradeEUR + "12.JSON", etrading::JSON );
        const std::string liborOis13    = validation::tryAqObjLoad( liborOISTradeEUR + "13.JSON", etrading::JSON );
        const std::string liborOis14    = validation::tryAqObjLoad( liborOISTradeEUR + "14.JSON", etrading::JSON );
        const std::string liborOis15    = validation::tryAqObjLoad( liborOISTradeEUR + "15.JSON", etrading::JSON );
        const std::string liborOis16    = validation::tryAqObjLoad( liborOISTradeEUR + "16.JSON", etrading::JSON );
        
        // Valuation Settings LVB
        LabelValueBlock valuationSettings = LabelValueBlock::createLabelValueBlock( "CURVECOLLECTION", "EURYC" );

        // Get Par Spreads
        const double result1            = validation::tryAqObjSwapsParSpread( liborOis1,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result2            = validation::tryAqObjSwapsParSpread( liborOis2,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result3            = validation::tryAqObjSwapsParSpread( liborOis3,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result4            = validation::tryAqObjSwapsParSpread( liborOis4,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result5            = validation::tryAqObjSwapsParSpread( liborOis5,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result6            = validation::tryAqObjSwapsParSpread( liborOis6,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result7            = validation::tryAqObjSwapsParSpread( liborOis7,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result8            = validation::tryAqObjSwapsParSpread( liborOis8,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result9            = validation::tryAqObjSwapsParSpread( liborOis9,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result10           = validation::tryAqObjSwapsParSpread( liborOis10, valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result11           = validation::tryAqObjSwapsParSpread( liborOis11, valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result12           = validation::tryAqObjSwapsParSpread( liborOis12, valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result13           = validation::tryAqObjSwapsParSpread( liborOis13, valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result14           = validation::tryAqObjSwapsParSpread( liborOis14, valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result15           = validation::tryAqObjSwapsParSpread( liborOis15, valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result16           = validation::tryAqObjSwapsParSpread( liborOis16, valuationSettings, LabelValueBlock(), "Leg1:Float" );
        
        // Compare against Hard Coded Par Rates used in curve calibration instruments
        // These results should never be rebased, since this is a curve calibration arbitrage and consistency test
        const double tolerance = 1e-2;
        EXPECT_NEAR( result1,  8.5000, tolerance );
        EXPECT_NEAR( result2,  9.5000, tolerance );
        EXPECT_NEAR( result3,  10.3000, tolerance );
        EXPECT_NEAR( result4,  10.6000, tolerance );
        EXPECT_NEAR( result5,  10.9000, tolerance );
        EXPECT_NEAR( result6,  11.1000, tolerance );
        EXPECT_NEAR( result7,  11.2000, tolerance );
        EXPECT_NEAR( result8,  11.4000, tolerance );
        EXPECT_NEAR( result9,  11.4000, tolerance );
        EXPECT_NEAR( result10, 11.4000, tolerance );
        EXPECT_NEAR( result11, 11.3000, tolerance );
        EXPECT_NEAR( result12, 10.9000, tolerance );
        EXPECT_NEAR( result13, 10.5000, tolerance );
        EXPECT_NEAR( result14, 10.1000, tolerance );
        EXPECT_NEAR( result15, 9.4000, tolerance );
        EXPECT_NEAR( result16, 8.7000, tolerance );
    }

    // Call Test Fixture Class
    TEST_F( TestLiborOisSwaps, CONSISTENCY_CheckLiborOisSwapsReprice_GBP )
    {
        // Load Curves
        const std::string curveGBPOIS   = validation::tryAqObjLoad( OIScurveGBP, etrading::JSON );
        const std::string curveGBP3ML   = validation::tryAqObjLoad( STDcurveGBP, etrading::JSON );

        // Load Libor-OIS Swap Trades
        const std::string liborOis1     = validation::tryAqObjLoad( liborOISTradeGBP + "1.JSON" , etrading::JSON );
        const std::string liborOis2     = validation::tryAqObjLoad( liborOISTradeGBP + "2.JSON" , etrading::JSON );
        const std::string liborOis3     = validation::tryAqObjLoad( liborOISTradeGBP + "3.JSON" , etrading::JSON );
        const std::string liborOis4     = validation::tryAqObjLoad( liborOISTradeGBP + "4.JSON" , etrading::JSON );
        const std::string liborOis5     = validation::tryAqObjLoad( liborOISTradeGBP + "5.JSON" , etrading::JSON );
        const std::string liborOis6     = validation::tryAqObjLoad( liborOISTradeGBP + "6.JSON" , etrading::JSON );
        const std::string liborOis7     = validation::tryAqObjLoad( liborOISTradeGBP + "7.JSON" , etrading::JSON );
        const std::string liborOis8     = validation::tryAqObjLoad( liborOISTradeGBP + "8.JSON" , etrading::JSON );
        
        // Valuation Settings LVB
        LabelValueBlock valuationSettings = LabelValueBlock::createLabelValueBlock( "CURVECOLLECTION", "GBPYC" );

        // Get Par Spreads
        const double result1            = validation::tryAqObjSwapsParSpread( liborOis1,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result2            = validation::tryAqObjSwapsParSpread( liborOis2,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result3            = validation::tryAqObjSwapsParSpread( liborOis3,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result4            = validation::tryAqObjSwapsParSpread( liborOis4,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result5            = validation::tryAqObjSwapsParSpread( liborOis5,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result6            = validation::tryAqObjSwapsParSpread( liborOis6,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result7            = validation::tryAqObjSwapsParSpread( liborOis7,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        const double result8            = validation::tryAqObjSwapsParSpread( liborOis8,  valuationSettings, LabelValueBlock(), "Leg1:Float" );
        
        // Compare against Hard Coded Par Rates used in curve calibration instruments
        // These results should never be rebased, since this is a curve calibration arbitrage and consistency test
        const double tolerance = 1e-2;
        EXPECT_NEAR( result1,  17.5000, tolerance );
        EXPECT_NEAR( result2,  18.9000, tolerance );
        EXPECT_NEAR( result3,  20.5000, tolerance );
        EXPECT_NEAR( result4,  23.3000, tolerance );
        EXPECT_NEAR( result5,  25.9000, tolerance );
        EXPECT_NEAR( result6,  28.0000, tolerance );
        EXPECT_NEAR( result7,  29.2000, tolerance );
        EXPECT_NEAR( result8,  30.6000, tolerance );
    }
   
}