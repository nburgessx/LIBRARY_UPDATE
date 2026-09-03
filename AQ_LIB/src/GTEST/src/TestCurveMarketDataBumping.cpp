// Include: Google Test Library
#include <gTest/gTest.h>

// Test Includes
#include "InitializeAQGoogleTest.h"
#include "GetGoogleTestFolder.h"
#include "RecordMacros.h"						// Logfile Macros
#include "ResultsProcessor.h"					// Test Rebase Methods
#include "tryMeLWO.h"							// LWO Curve Loading et al.
#include "tryMeLWOCurveMarketData.h"			// Curve Market Data Bumping Method(s)

// Test Folders
const std::string TEST_FOLDER						= "ETrading/Curves/TestCurveMarketData/";

// Test Files: Function Inputs / Outputs
const std::string curveMarketData_USDOIS		= TEST_FOLDER + "USD_OIS_CURVE_MARKETDATA.JSON";
const std::string curveMarketData_USD3ML		= TEST_FOLDER + "USD_SWAP_3M_CURVE_MARKETDATA.JSON";
const std::string curveMarketData_USD6ML		= TEST_FOLDER + "USD_BASIS_3X6_CURVE_MARKETDATA.JSON";

const std::string curveMarketData_USDOIS_WithFutures_Original	= TEST_FOLDER + "USD_OIS_CURVE_MARKETDATA_WITH_FUTURES_ORIGINAL.JSON";
const std::string curveMarketData_USDOIS_WithFutures_Bumped		= TEST_FOLDER + "USD_OIS_CURVE_MARKETDATA_WITH_FUTURES_BUMPED.JSON";


namespace google_test
{
    // Tolerance Settings
	const double tolerance	= 1e-8;
	
    // TEST FIXTURE CONSTRUCTOR
    DECLARE_TEST_FIXTURE( TestCurveMarketDataBumping )

	TEST_F( TestCurveMarketDataBumping, CONSISTENCY_BumpOutrightInstrumentsOnly_USDOIS )
	{
		// 1. Load Original Curve Market Data
		const std::string loadUSDOISMarketData					= validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + curveMarketData_USDOIS, etrading::JSON );

		// 2. Capture the Original Market Data Inputs
		const etrading::VariantMatrix originalOISQuotes			= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "OIS" );
		const etrading::VariantMatrix originalLiborOISQuotes	= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "LIBOROISBASISSPREADS" );
		const etrading::VariantMatrix originalSwapQuotes		= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "SWAPS" );

		// 3. Bump the Outright Instruments
		// *** Only bump outright instruments = true ***
		const double bumpSize = 1.0e-4;
		validation::tryMeLWOCurveMarketDataBumpAll( loadUSDOISMarketData, bumpSize, true ); 

		// 4. Capture the Bumped Market Data Inputs
		const etrading::VariantMatrix bumpedOISQuotes			= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "OIS" );
		const etrading::VariantMatrix bumpedLiborOISQuotes		= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "LIBOROISBASISSPREADS" );
		const etrading::VariantMatrix bumpedSwapQuotes			= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "SWAPS" );

		// 5. Check the Results
		ASSERT_EQ( originalOISQuotes.size(),		bumpedOISQuotes.size() );
		ASSERT_EQ( originalLiborOISQuotes.size(),	bumpedLiborOISQuotes.size() );
		ASSERT_EQ( originalSwapQuotes.size(),		bumpedSwapQuotes.size() );

		for( size_t i = 0; i < bumpedOISQuotes.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix
			EXPECT_NEAR( bumpedOISQuotes[i][1].getValue<double>(), originalOISQuotes[i][1].getValue<double>() + bumpSize, tolerance );
		}

		for( size_t i = 0; i < bumpedLiborOISQuotes.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix - Note we bumped the outrights only - so don't add bump size here
			EXPECT_NEAR( bumpedLiborOISQuotes[i][1].getValue<double>(), originalLiborOISQuotes[i][1].getValue<double>(), tolerance );
		}

		for( size_t i = 0; i < bumpedSwapQuotes.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix
			EXPECT_NEAR( bumpedSwapQuotes[i][1].getValue<double>(), originalSwapQuotes[i][1].getValue<double>() + bumpSize, tolerance );
		}
	}

	TEST_F( TestCurveMarketDataBumping, CONSISTENCY_BumpAllInstruments_USDOIS )
	{
		// 1. Load Original Curve Market Data
		const std::string loadUSDOISMarketData					= validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + curveMarketData_USDOIS, etrading::JSON );

		// 2. Capture the Original Market Data Inputs
		const etrading::VariantMatrix originalOISQuotes			= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "OIS" );
		const etrading::VariantMatrix originalLiborOISQuotes	= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "LIBOROISBASISSPREADS" );
		const etrading::VariantMatrix originalSwapQuotes		= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "SWAPS" );

		// 3. Bump the Outright Instruments
		// *** Only bump outright instruments = false ***
		const double bumpSize = 1.0e-4;
		validation::tryMeLWOCurveMarketDataBumpAll( loadUSDOISMarketData, bumpSize, false ); 

		// 4. Capture the Bumped Market Data Inputs
		const etrading::VariantMatrix bumpedOISQuotes			= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "OIS" );
		const etrading::VariantMatrix bumpedLiborOISQuotes		= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "LIBOROISBASISSPREADS" );
		const etrading::VariantMatrix bumpedSwapQuotes			= validation::tryMeLWOCurveMarketDataDisplay( loadUSDOISMarketData, "SWAPS" );

		// 5. Check the Results
		ASSERT_EQ( originalOISQuotes.size(),		bumpedOISQuotes.size() );
		ASSERT_EQ( originalLiborOISQuotes.size(),	bumpedLiborOISQuotes.size() );
		ASSERT_EQ( originalSwapQuotes.size(),		bumpedSwapQuotes.size() );

		for( size_t i = 0; i < bumpedOISQuotes.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix
			EXPECT_NEAR( bumpedOISQuotes[i][1].getValue<double>(), originalOISQuotes[i][1].getValue<double>() + bumpSize, tolerance );
		}

		for( size_t i = 0; i < bumpedLiborOISQuotes.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix - Apply BumpSize to Spread Instruments in this case
			EXPECT_NEAR( bumpedLiborOISQuotes[i][1].getValue<double>(), originalLiborOISQuotes[i][1].getValue<double>() + bumpSize, tolerance );
		}

		for( size_t i = 0; i < bumpedSwapQuotes.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix
			EXPECT_NEAR( bumpedSwapQuotes[i][1].getValue<double>(), originalSwapQuotes[i][1].getValue<double>() + bumpSize, tolerance );
		}
	}

	TEST_F( TestCurveMarketDataBumping, CONSISTENCY_BumpOutrightInstrumentsOnly_USD3ML )
	{
		// 1. Load Original Curve Market Data
		const std::string loadUSD3MLMarketData					= validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + curveMarketData_USD3ML, etrading::JSON );

		// 2. Capture the Original Market Data Inputs
		const etrading::VariantMatrix originalFixings			= validation::tryMeLWOCurveMarketDataDisplay( loadUSD3MLMarketData, "LIBORFIXINGS" );
		const etrading::VariantMatrix originalFutures			= validation::tryMeLWOCurveMarketDataDisplay( loadUSD3MLMarketData, "FUTURES" );
		const etrading::VariantMatrix originalSwaps				= validation::tryMeLWOCurveMarketDataDisplay( loadUSD3MLMarketData, "SWAPS" );

		// 3. Bump the Outright Instruments
		// *** Only bump outright instruments = true ***
		const double bumpSize = 1.0e-4;
		validation::tryMeLWOCurveMarketDataBumpAll( loadUSD3MLMarketData, bumpSize, true ); 

		// 4. Capture the Bumped Market Data Inputs
		const etrading::VariantMatrix bumpedFixings			= validation::tryMeLWOCurveMarketDataDisplay( loadUSD3MLMarketData, "LIBORFIXINGS" );
		const etrading::VariantMatrix bumpedFutures			= validation::tryMeLWOCurveMarketDataDisplay( loadUSD3MLMarketData, "FUTURES" );
		const etrading::VariantMatrix bumpedSwaps			= validation::tryMeLWOCurveMarketDataDisplay( loadUSD3MLMarketData, "SWAPS" );

		// 5. Check the Results
		ASSERT_EQ( originalFixings.size(),		bumpedFixings.size() );
		ASSERT_EQ( originalFutures.size(),		bumpedFutures.size() );
		ASSERT_EQ( originalSwaps.size(),		bumpedSwaps.size() );

		for( size_t i = 0; i < bumpedFixings.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix
			EXPECT_NEAR( bumpedFixings[i][1].getValue<double>(), originalFixings[i][1].getValue<double>() + bumpSize, tolerance );
		}

		for( size_t i = 0; i < bumpedFutures.size(); ++i )
		{
			// Calculate the expected bumped future value, we expect the underlying future's rate to be bumped not the future's price
			// Futures quotes are in Column 4 of the Quotes Matrix i.e. index 3
			const double originalFuture			= originalFutures[i][3].getValue<double>();
			const double futureRateInPercent	= ( 100.0 - originalFuture ) / 100.0;
			const double adjustedFutureRate		= ( futureRateInPercent + bumpSize ) * 100.0; // scale by 100 to get back from percent
			const double expectedBumpedFuture	= 100.0 - adjustedFutureRate;

			// Futures quotes are in Column 4 of the Quotes Matrix i.e. index 3
			EXPECT_NEAR( bumpedFutures[i][3].getValue<double>(), expectedBumpedFuture, tolerance );
		}

		for( size_t i = 0; i < bumpedSwaps.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix
			EXPECT_NEAR( bumpedSwaps[i][1].getValue<double>(), originalSwaps[i][1].getValue<double>() + bumpSize, tolerance );
		}
	}

	TEST_F( TestCurveMarketDataBumping, CONSISTENCY_BumpOutrightInstrumentsOnly_USD6ML )
	{
		// 1. Load Original Curve Market Data
		const std::string loadUSD6MLMarketData					= validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + curveMarketData_USD6ML, etrading::JSON );

		// 2. Capture the Original Market Data Inputs
		const etrading::VariantMatrix originalBasisSwaps		= validation::tryMeLWOCurveMarketDataDisplay( loadUSD6MLMarketData, "BASISSWAPS" );
		
		// 3. Bump the Outright Instruments
		// *** Only bump outright instruments = true ***
		const double bumpSize = 1.0e-4;
		validation::tryMeLWOCurveMarketDataBumpAll( loadUSD6MLMarketData, bumpSize, true ); 

		// 4. Capture the Bumped Market Data Inputs
		const etrading::VariantMatrix bumpedBasisSwaps			= validation::tryMeLWOCurveMarketDataDisplay( loadUSD6MLMarketData, "BASISSWAPS" );
		
		// 5. Check the Results
		ASSERT_EQ( originalBasisSwaps.size(), bumpedBasisSwaps.size() );

		for( size_t i = 0; i < bumpedBasisSwaps.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix - No need to apply bump here since we are bumping outrights only ... in this case the market data should be identical
			EXPECT_NEAR( bumpedBasisSwaps[i][1].getValue<double>(), originalBasisSwaps[i][1].getValue<double>(), tolerance );
		}
	}

	TEST_F( TestCurveMarketDataBumping, CONSISTENCY_BumpAllInstruments_USD6ML )
	{
		// 1. Load Original Curve Market Data
		const std::string loadUSD6MLMarketData					= validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + curveMarketData_USD6ML, etrading::JSON );

		// 2. Capture the Original Market Data Inputs
		const etrading::VariantMatrix originalBasisSwaps		= validation::tryMeLWOCurveMarketDataDisplay( loadUSD6MLMarketData, "BASISSWAPS" );
		
		// 3. Bump the Outright Instruments
		// *** Only bump outright instruments = false ***
		const double bumpSize = 1.0e-4;
		validation::tryMeLWOCurveMarketDataBumpAll( loadUSD6MLMarketData, bumpSize, false ); 

		// 4. Capture the Bumped Market Data Inputs
		const etrading::VariantMatrix bumpedBasisSwaps			= validation::tryMeLWOCurveMarketDataDisplay( loadUSD6MLMarketData, "BASISSWAPS" );
		
		// 5. Check the Results
		ASSERT_EQ( originalBasisSwaps.size(), bumpedBasisSwaps.size() );

		for( size_t i = 0; i < bumpedBasisSwaps.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix - In this case we specified bump all instruments so we apply the bumpSize
			EXPECT_NEAR( bumpedBasisSwaps[i][1].getValue<double>(), originalBasisSwaps[i][1].getValue<double>() + bumpSize, tolerance );
		}
	}

	TEST_F( TestCurveMarketDataBumping, CONSISTENCY_BumpAllInstruments_USDOIS_WithFutures )
	{
		// 1. Load Original Curve Market Data
		const std::string originalUSDOISMarketData				= validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + curveMarketData_USDOIS_WithFutures_Original, etrading::JSON );

		// 2. Capture the Original Market Data Inputs
		const etrading::VariantMatrix originalOISQuotes			= validation::tryMeLWOCurveMarketDataDisplay( originalUSDOISMarketData, "OIS" );

		// 3. Bump the Outright Instruments
		// *** Only bump outright instruments = true ***
		const double bumpSize = 1.0e-4;
		validation::tryMeLWOCurveMarketDataBumpAll( originalUSDOISMarketData, bumpSize, true ); 

		// 4. Capture the Bumped Market Data Inputs
		const etrading::VariantMatrix bumpedOISQuotes			= validation::tryMeLWOCurveMarketDataDisplay( originalUSDOISMarketData, "OIS" );

		// 5. Load Expected Results
		const std::string expectedUSDOISMarketData				= validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + curveMarketData_USDOIS_WithFutures_Bumped, etrading::JSON );
		const etrading::VariantMatrix expectedBumpedOISQuotes	= validation::tryMeLWOCurveMarketDataDisplay( expectedUSDOISMarketData, "OIS" );

		// 5. Check the Results
		ASSERT_EQ( originalOISQuotes.size(),		bumpedOISQuotes.size() );
		ASSERT_EQ( expectedBumpedOISQuotes.size(),	bumpedOISQuotes.size() );

		for( size_t i = 0; i < bumpedOISQuotes.size(); ++i )
		{
			// Quotes are in Column 2 of the Quotes Matrix
			EXPECT_NEAR( bumpedOISQuotes[i][1].getValue<double>(), expectedBumpedOISQuotes[i][1].getValue<double>(), tolerance );
		}
	}

}