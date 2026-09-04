// TestEuropeanSwaptions.cpp

/*
 * @brief			Tests to test the European Swaption Product
 * @Created:		22nd August 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "Dependency.h"
#include "GetGoogleTestFolder.h"
#include "CoreEnumerations.h"

#include "AQLCoreTemplateType.h"                 // AQLStringVector and AQLStringMatrix TypeDefs
#include "tryAqObjects.h"                       // Curve, Trade and Volatility Object Loading
#include "EuropeanIRSwaption.h"             // Swaption Mathematics
#include "tryAqObjectsSwaption.h"               // Swaption Pricing
#include "tryAqObjectsSwapPricing.h"            // Swap Pricing


// Define Test Folder Path(s)
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***

const std::string TEST_FOLDER = "ETrading/IROptions/TestEuropeanSwaptions/";

// Test Files
// ==============================================================================================

// Curves
const std::string fileNameCurveUSDOIS                   = TEST_FOLDER + "USD_OIS_CURVE.JSON";
const std::string fileNameCurveUSD3ML                   = TEST_FOLDER + "USD_SWAP_3M_CURVE.JSON";

// Swaption Trades
const std::string fileNameTradeSwaption                 = TEST_FOLDER + "SWAPTION.JSON";
const std::string fileNameTradePayerSwaption            = TEST_FOLDER + "LONGPAYER.JSON";
const std::string fileNameTradeReceiverSwaption         = TEST_FOLDER + "SHORTRECEIVER.JSON";
const std::string fileNameTradeReceiverSwap             = TEST_FOLDER + "SWAP.JSON";
const std::string fileNameTradeSwaptionNegativeVol      = TEST_FOLDER + "NEGATIVE_STRIKE.JSON";
const std::string fileNameTradeSwaptionWithFee          = TEST_FOLDER + "SWAPTIONWITHFEE.JSON";
const std::string fileNameTradeSwaptionNormalVol        = TEST_FOLDER + "SWAPTIONNORMAL.JSON";
const std::string fileNameTradeSwaptionCashParYield     = TEST_FOLDER + "SWAPTIONCASHPARYIELD.JSON";

// Swaption Volatilities
const std::string fileNameVolatilityNormal              = TEST_FOLDER + "NORMALVOL.JSON";
const std::string fileNameVolatilityLogNormal           = TEST_FOLDER + "LOGNORMALVOL.JSON";
const std::string fileNameVolatilityShiftedLogNormal    = TEST_FOLDER + "SHIFTEDLOGNORMALVOL.JSON";

// ==============================================================================================


namespace google_test
{

    // Helper Method to Create the Valuation Settings String Matrix LVB
    AQLStringMatrix createValationSettingsLVB( const std::string & curveCollection, const std::string & volatilityModel )
    {
        AQLStringMatrix valuationSettings(2);
        valuationSettings[0] = { AQLString("CurveCollection"), curveCollection.c_str() };
        valuationSettings[1] = { AQLString("VolatilityModel"), volatilityModel.c_str() };
        return valuationSettings;
    }


    // Declare Test Fixture Class
    class TestEuropeanSwaption : public virtual testing::Test, public virtual google_test::InitializeGoogleTest
    {
    public:

        // Load Curves
        const std::string curveCollection_                  = "USDYC";
        const std::string loadCurveUSDOIS_                  = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameCurveUSDOIS, etrading::JSON );
        const std::string loadCurveUSD3ML_                  = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameCurveUSD3ML, etrading::JSON );

        // Load Trades
        const std::string loadTradeSwaptionNormalVol_       = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameTradeSwaptionNormalVol, etrading::JSON );
        const std::string loadTradeSwaption_                = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameTradeSwaption, etrading::JSON );
        const std::string loadTradePayerSwaption_           = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameTradePayerSwaption, etrading::JSON );
        const std::string loadTradeReceiverSwaption_        = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameTradeReceiverSwaption, etrading::JSON );
        const std::string loadTradeReceiverSwap_            = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameTradeReceiverSwap, etrading::JSON );
        const std::string loadTradeNegativeRates_           = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameTradeSwaptionNegativeVol, etrading::JSON );
        const std::string loadTradeSwaptionWithFee_         = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameTradeSwaptionWithFee, etrading::JSON );
        const std::string loadTradeSwaptionCashParYield_    = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameTradeSwaptionCashParYield, etrading::JSON );
        
        // Load Volatilities
        const std::string loadVolNormal_                    = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameVolatilityNormal, etrading::JSON );
        const std::string loadVolLogNormal_                 = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameVolatilityLogNormal, etrading::JSON );
        const std::string loadVolShiftedLogNormal_          = validation::tryAqObjectsLoad( etrading::getGoogleTestFolder() + fileNameVolatilityShiftedLogNormal, etrading::JSON );
    };


    // Snapshot Test - Swaption PV using LogNormal Vol
    TEST_F( TestEuropeanSwaption, SNAPSHOT_USDSwaptionPV_Normal )
    {
        // Create Valuation Settings LVB
        AQLStringMatrix valuationSettings = createValationSettingsLVB( curveCollection_, loadVolNormal_ );
        
        // Price the European Swaption
        const double swaptionPV = validation::tryAqObjectsSwaptionPV( loadTradeSwaption_, valuationSettings );
        const double expectedPV = 10991.51;

        const double tolerancePV = 1.0;
        EXPECT_NEAR( swaptionPV, expectedPV, tolerancePV );
    }

    // Snapshot Test - Swaption PV using LogNormal Vol
    TEST_F( TestEuropeanSwaption, SNAPSHOT_USDSwaptionPV_LogNormal )
    {
        // Create Valuation Settings LVB
        AQLStringMatrix valuationSettings = createValationSettingsLVB( curveCollection_, loadVolLogNormal_ );
        
        // Price the European Swaption
        const double swaptionPV = validation::tryAqObjectsSwaptionPV( loadTradeSwaption_, valuationSettings );
        const double expectedPV = 12165.57;

        const double tolerancePV = 1.0;
        EXPECT_NEAR( swaptionPV, expectedPV, tolerancePV );
    }

    // Snapshot Test - Swaption PV using LogNormal Vol with a Fee
    TEST_F( TestEuropeanSwaption, SNAPSHOT_USDSwaptionPV_LogNormal_WithFee )
    {
        // Create Valuation Settings LVB
        AQLStringMatrix valuationSettings = createValationSettingsLVB( curveCollection_, loadVolLogNormal_ );
        
        // Price the European Swaption
        const double swaptionPV = validation::tryAqObjectsSwaptionPV( loadTradeSwaptionWithFee_, valuationSettings );
        const double expectedPV = -987834.42;

        const double tolerancePV = 1.0;
        EXPECT_NEAR( swaptionPV, expectedPV, tolerancePV );
    }

    // CONSISTENCY Test - Swaption PV using LogNormal Vol with Cash Par-Yield Settlement
    TEST_F( TestEuropeanSwaption, SNAPSHOT_USDSwaptionPV_LogNormal_CashParYieldSettlement )
    {
         // Create Valuation Settings LVB
        AQLStringMatrix valuationSettings = createValationSettingsLVB( curveCollection_, loadVolLogNormal_ );
        
        // Price the European Swaption
        const double swaptionPV = validation::tryAqObjectsSwaptionPV( loadTradeSwaptionCashParYield_, valuationSettings );
        const double expectedPV = 12065.96;

        const double tolerancePV = 1.0;
        EXPECT_NEAR( swaptionPV, expectedPV, tolerancePV );
    }

    // Snapshot Test - Swaption PV using Shifted-LogNormal Vol and a Negative Strike
    TEST_F( TestEuropeanSwaption, SNAPSHOT_USDSwaptionPV_ShiftedLogNormal )
    {
        // Create Valuation Settings LVB
        AQLStringMatrix valuationSettings = createValationSettingsLVB( curveCollection_, loadVolShiftedLogNormal_ );
        
        // Price the European Swaption
        const double swaptionPV = validation::tryAqObjectsSwaptionPV( loadTradeNegativeRates_, valuationSettings );
        const double expectedPV = 156327.77;

        const double tolerancePV = 1.0;
        EXPECT_NEAR( swaptionPV, expectedPV, tolerancePV );
    }

    // CONSISTENCY Test - Swaption Parity Test: LONG_PAYER_SWAPTION + SHORT_RECEIVER_SWAPTION + RECEIVER_SWAP = 0
    TEST_F( TestEuropeanSwaption, CONSISTENCY_TestSwaptionParityForNonArbitrage )
    {
        // Create Valuation Settings LVB
        AQLStringMatrix valuationSettings = createValationSettingsLVB( curveCollection_, loadVolShiftedLogNormal_ );
        
        // Price the European Swaptions and the Underlying Swap
        const double longPayerSwaption          = validation::tryAqObjectsSwaptionPV( loadTradePayerSwaption_, valuationSettings );
        const double shortReceiverSwaption      = validation::tryAqObjectsSwaptionPV( loadTradeReceiverSwaption_, valuationSettings );
        const double receiverSwap               = validation::tryAqObjectsSwapPV( loadTradeReceiverSwap_, valuationSettings );
        
        const double tolerancePV = 0.1;
        EXPECT_NEAR( longPayerSwaption + shortReceiverSwaption + receiverSwap, 0.0, tolerancePV );
    }
    
}