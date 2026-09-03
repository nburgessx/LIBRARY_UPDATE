// TestCurveStreaming.cpp

/*
 * @brief			Tests to test the curve streaming methods
 * @Created:		15th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		MHI London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "Dependency.h"
#include "ReadDataFile.h"
#include "GetGoogleTestFolder.h"
#include "ResultsProcessor.h"
#include "tryMeLWO.h"
#include "tryMeCurveDiscountFactor.h"
#include "CurveStreaming.h"
#include "CurveResultsContainer.h"
#include "CoreEnumerations.h"

// Define Test Folder Path(s)
// *** The Google Test DataInstance folder is defined within etrading project, GoogleTest Folder, GetGoogleTestFolder.h ***

const std::string TEST_FOLDER = "ETrading/Curves/TestCurveStreaming/";

// Test Files
const std::string fileNameUSDOIS = TEST_FOLDER + "USD_OIS_CURVE.JSON";
const std::string fileNameUSD3ML = TEST_FOLDER + "USD_SWAP_3M_CURVE.JSON";
const std::string fileNameUSD6ML = TEST_FOLDER + "USD_BASIS_6M_CURVE.JSON";

const std::string inputsRawDiscountFactors = TEST_FOLDER + "tryMeCurveDiscountFactorsDisplay_inputs.csv";
const std::string outputsRawDiscountFactors = TEST_FOLDER + "tryMeCurveDiscountFactorsDisplay_outputs.csv";

using etrading::ReadDataFile;
    

namespace google_test
{
    
    // Declare Test Fixture Class
    DECLARE_TEST_FIXTURE( TestCurveStreaming )

    // Call Test Fixture Class
    TEST_F( TestCurveStreaming, SNAPSHOT_USDYC_USDOIS_OIS_Display_DiscountFactors )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Actual Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        std::vector<double> actualDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Compare Results with Stored Snapshot Results - Allow Rebasing
        const double tolerance = 1e-6;
        CheckTestResultsAndRebaseOnRequest( actualDiscountFactors, TEST_FOLDER.c_str(), outputsRawDiscountFactors.c_str(), tolerance );
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USDOIS_OIS_Overwrite_DiscountFactors )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Overwrite Discount Factors - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> newPaymentDates = {
            LADate("20190101"),
            LADate("20200101"),
            LADate("20210101"),
            LADate("20220101"),
            LADate("20230101") };

        const std::vector<double> newDiscountFactors = {
            0.999999,
            0.888888,
            0.777777,
            0.666666,
            0.555555 };
        
        etrading::setCurveDiscountFactors( "USDYC", "USDOIS", newPaymentDates, newDiscountFactors );

        // Get Discount Factors
        const std::vector<double> resultDiscountFactors = etrading::getCurveDiscountFactors( "USDYC", "USDOIS" ).discountFactors_;

        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( resultDiscountFactors.size(), newDiscountFactors.size() );

        // Check Reults Dimensions As Expected
        EXPECT_EQ( 5, resultDiscountFactors.size() );

        // Check Results as Expected
        const double tolerance = 1e-6;
        for( size_t i = 0; i < resultDiscountFactors.size(); ++i )
        {
            EXPECT_NEAR( resultDiscountFactors[i], newDiscountFactors[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USDOIS_OIS_Overwrite_DiscountFactors_LargeDataSet )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Overwrite Discount Factors - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> newPaymentDates = {
            LADate("20190101"),
            LADate("20190102"),
            LADate("20190103"),
            LADate("20190104"),
            LADate("20190105"),
            LADate("20190106"),
            LADate("20190107"),
            LADate("20190108"),
            LADate("20190109"),
            LADate("20190110"),
            LADate("20190111"),
            LADate("20190112"),
            LADate("20190113"),
            LADate("20190114"),
            LADate("20190115"),
            LADate("20190116"),
            LADate("20190117"),
            LADate("20190118"),
            LADate("20190119"),
            LADate("20190120"),
            LADate("20190121"),
            LADate("20190122"),
            LADate("20190123"),
            LADate("20190124"),
            LADate("20190125"),
            LADate("20190126"),
            LADate("20190127"),
            LADate("20190128"),
            LADate("20190129"),
            LADate("20190130"),
            LADate("20190131") };

        const std::vector<double> newDiscountFactors = {
            0.999901,
            0.999902,
            0.999903,
            0.999904,
            0.999905,
            0.999906,
            0.999907,
            0.999908,
            0.999909,
            0.999910,
            0.999911,
            0.999912,
            0.999913,
            0.999914,
            0.999915,
            0.999916,
            0.999917,
            0.999918,
            0.999919,
            0.999920,
            0.999921,
            0.999922,
            0.999923,
            0.999924,
            0.999925,
            0.999926,
            0.999927,
            0.999928,
            0.999929,
            0.999930,
            0.999931 };
        
        etrading::setCurveDiscountFactors( "USDYC", "USDOIS", newPaymentDates, newDiscountFactors );

        // Get Discount Factors
        const std::vector<double> resultDiscountFactors = etrading::getCurveDiscountFactors( "USDYC", "USDOIS" ).discountFactors_;

        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( resultDiscountFactors.size(), newDiscountFactors.size() );

        const double tolerance = 1e-6;
        for( size_t i = 0; i < resultDiscountFactors.size(); ++i )
        {
            EXPECT_NEAR( resultDiscountFactors[i], newDiscountFactors[i], tolerance );
        }
    }
    
    TEST_F( TestCurveStreaming, UNIT_USDYC_USDOIS_OIS_Overwrite_DiscountFactors_SetToOne )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Set Discount Factors to One
        etrading::setCurveDiscountFactorsToOne( "USDYC", "USDOIS" );

        // Get Discount Factors
        const std::vector<double> resultDiscountFactors = etrading::getCurveDiscountFactors( "USDYC", "USDOIS" ).discountFactors_;

        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( resultDiscountFactors.size(), originalDiscountFactors.size() );

        const double tolerance = 1e-6;
        for( size_t i = 0; i < resultDiscountFactors.size(); ++i )
        {
            EXPECT_NEAR( resultDiscountFactors[i], 1.0, tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD3ML_SWAP_Overwrite_DiscountFactors )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Discount Factors - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> newPaymentDates = {
            LADate("20190101"),
            LADate("20200101"),
            LADate("20210101"),
            LADate("20220101"),
            LADate("20230101") };

        const std::vector<double> newDiscountFactors = {
            0.999999,
            0.888888,
            0.777777,
            0.666666,
            0.555555 };
        
        etrading::setCurveDiscountFactors( "USDYC", "USD3ML", newPaymentDates, newDiscountFactors );

        // Get Discount Factors
        const std::vector<double> resultDiscountFactors = etrading::getCurveDiscountFactors( "USDYC", "USD3ML" ).discountFactors_;

        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( resultDiscountFactors.size(), newDiscountFactors.size() );

        // Check Reults Dimensions As Expected
        EXPECT_EQ( 5, resultDiscountFactors.size() );

        // Check Results as Expected
        const double tolerance = 1e-6;
        for( size_t i = 0; i < resultDiscountFactors.size(); ++i )
        {
            EXPECT_NEAR( resultDiscountFactors[i], newDiscountFactors[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD3ML_SWAP_Overwrite_DiscountFactors_LargeDataSet )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Discount Factors - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> newPaymentDates = {
            LADate("20190101"),
            LADate("20190102"),
            LADate("20190103"),
            LADate("20190104"),
            LADate("20190105"),
            LADate("20190106"),
            LADate("20190107"),
            LADate("20190108"),
            LADate("20190109"),
            LADate("20190110"),
            LADate("20190111"),
            LADate("20190112"),
            LADate("20190113"),
            LADate("20190114"),
            LADate("20190115"),
            LADate("20190116"),
            LADate("20190117"),
            LADate("20190118"),
            LADate("20190119"),
            LADate("20190120"),
            LADate("20190121"),
            LADate("20190122"),
            LADate("20190123"),
            LADate("20190124"),
            LADate("20190125"),
            LADate("20190126"),
            LADate("20190127"),
            LADate("20190128"),
            LADate("20190129"),
            LADate("20190130"),
            LADate("20190131") };

        const std::vector<double> newDiscountFactors = {
            0.999901,
            0.999902,
            0.999903,
            0.999904,
            0.999905,
            0.999906,
            0.999907,
            0.999908,
            0.999909,
            0.999910,
            0.999911,
            0.999912,
            0.999913,
            0.999914,
            0.999915,
            0.999916,
            0.999917,
            0.999918,
            0.999919,
            0.999920,
            0.999921,
            0.999922,
            0.999923,
            0.999924,
            0.999925,
            0.999926,
            0.999927,
            0.999928,
            0.999929,
            0.999930,
            0.999931 };
        
        etrading::setCurveDiscountFactors( "USDYC", "USD3ML", newPaymentDates, newDiscountFactors );

        // Get Discount Factors
        const std::vector<double> resultDiscountFactors = etrading::getCurveDiscountFactors( "USDYC", "USD3ML" ).discountFactors_;

        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( resultDiscountFactors.size(), newDiscountFactors.size() );

        const double tolerance = 1e-6;
        for( size_t i = 0; i < resultDiscountFactors.size(); ++i )
        {
            EXPECT_NEAR( resultDiscountFactors[i], newDiscountFactors[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD3ML_SWAP_Overwrite_DiscountFactors_SetToOne )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Set Discount Factors to One
        etrading::setCurveDiscountFactorsToOne( "USDYC", "USD3ML" );

        // Get Discount Factors
        const std::vector<double> resultDiscountFactors = etrading::getCurveDiscountFactors( "USDYC", "USD3ML" ).discountFactors_;

        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( resultDiscountFactors.size(), originalDiscountFactors.size() );

        const double tolerance = 1e-6;
        for( size_t i = 0; i < resultDiscountFactors.size(); ++i )
        {
            EXPECT_NEAR( resultDiscountFactors[i], 1.0, tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD3ML_SWAP_SetForwardRates_2PCT )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> fixingDates = {
            LADate("20190101"),
            LADate("20200101"),
            LADate("20210101"),
            LADate("20220101"),
            LADate("20230101") };

        // Constant Forward Rates of 2.0%
        const std::vector<double> expectedForwardRates = {
            0.02,
            0.02,
            0.02,
            0.02,
            0.02 };
        
        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        const bool setEquivalentDiscountFactors = true;
        LAString setForwardsResult = etrading::setCurveForwardRates( "USDYC", "USD3ML", fixingDates, expectedForwardRates, setEquivalentDiscountFactors );

        // Get ForwardRates
        const std::vector<double> actualForwardRates
            = etrading::getCurveForwardRates( fixingDates, "USDYC", "USD3ML", etrading::NO_CHANGE, "", etrading::TRUE_BOOL ); //isFwdInter = true
        
        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size() );

        const double tolerance = 1e-5;
        for( size_t i = 0; i < actualForwardRates.size(); ++i )
        {
            EXPECT_NEAR( actualForwardRates[i], expectedForwardRates[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD3ML_SWAP_SetForwardRates_10PCT )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> fixingDates = {
            LADate("20190101"),
            LADate("20200101"),
            LADate("20210101"),
            LADate("20220101"),
            LADate("20230101") };

        // Constant Forward Rates of 10%
        const std::vector<double> expectedForwardRates = {
            0.10,
            0.10,
            0.10,
            0.10,
            0.10 };
        
        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        const bool setEquivalentDiscountFactors = true;
        LAString setForwardsResult = etrading::setCurveForwardRates( "USDYC", "USD3ML", fixingDates, expectedForwardRates, setEquivalentDiscountFactors );

        // Get ForwardRates
        const std::vector<double> actualForwardRates
            = etrading::getCurveForwardRates( fixingDates, "USDYC", "USD3ML", etrading::NO_CHANGE, "", etrading::TRUE_BOOL ); // isFwdInter = true
        
        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size() );

        const double tolerance = 1e-5;
        for( size_t i = 0; i < actualForwardRates.size(); ++i )
        {
            EXPECT_NEAR( actualForwardRates[i], expectedForwardRates[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD3ML_SWAP_SetForwardRates_25PCT )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> fixingDates = {
            LADate("20190101"),
            LADate("20200101"),
            LADate("20210101"),
            LADate("20220101"),
            LADate("20230101") };

        // Constant Forward Rates of 25%
        const std::vector<double> expectedForwardRates = {
            0.25,
            0.25,
            0.25,
            0.25,
            0.25 };
        
        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        const bool setEquivalentDiscountFactors = true;
        LAString setForwardsResult = etrading::setCurveForwardRates( "USDYC", "USD3ML", fixingDates, expectedForwardRates, setEquivalentDiscountFactors );

        // Get ForwardRates
        const std::vector<double> actualForwardRates
            = etrading::getCurveForwardRates( fixingDates, "USDYC", "USD3ML", etrading::NO_CHANGE, "", etrading::TRUE_BOOL ); // isFwdInter = true
        
        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size() );

        const double tolerance = 1e-5;
        for( size_t i = 0; i < actualForwardRates.size(); ++i )
        {
            EXPECT_NEAR( actualForwardRates[i], expectedForwardRates[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD3ML_SWAP_SetForwardRates_2PCT_LargeDataSet )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

         // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> fixingDates = {
            LADate("20190101"),
            LADate("20190201"),
            LADate("20190301"),
            LADate("20190401"),
            LADate("20190501"),
            LADate("20190601"),
            LADate("20190701"),
            LADate("20190801"),
            LADate("20190901"),
            LADate("20191001"),
            LADate("20191101"),
            LADate("20191201"),
            LADate("20200101"),
            LADate("20200201"),
            LADate("20200301"),
            LADate("20200401"),
            LADate("20200501"),
            LADate("20200601"),
            LADate("20200701"),
            LADate("20200801"),
            LADate("20200901"),
            LADate("20201001"),
            LADate("20201101"),
            LADate("20201201"),
            LADate("20210101"),
            LADate("20210201"),
            LADate("20210301"),
            LADate("20210401"),
            LADate("20210501"),
            LADate("20210601"),
            LADate("20210701"),
            LADate("20210801"),
            LADate("20211001"), 
            LADate("20211101"),
            LADate("20211201") };

        const std::vector<double> expectedForwardRates = {
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02 };
            

        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        const bool setEquivalentDiscountFactors = true;
        LAString setForwardsResult = etrading::setCurveForwardRates( "USDYC", "USD3ML", fixingDates, expectedForwardRates, setEquivalentDiscountFactors );

        // Get ForwardRates
        const std::vector<double> actualForwardRates
            = etrading::getCurveForwardRates( fixingDates, "USDYC", "USD3ML", etrading::NO_CHANGE, "", etrading::TRUE_BOOL ); // isFwdInter = true
        
        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size() );

        const double tolerance = 1e-5;
        for( size_t i = 0; i < actualForwardRates.size(); ++i )
        {
            EXPECT_NEAR( actualForwardRates[i], expectedForwardRates[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD6ML_BASIS_SetForwardRates_2PCT )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> fixingDates = {
            LADate("20190101"),
            LADate("20200101"),
            LADate("20210101"),
            LADate("20220101"),
            LADate("20230101") };

        // Constant Forward Rates of 2.0%
        const std::vector<double> expectedForwardRates = {
            0.02,
            0.02,
            0.02,
            0.02,
            0.02 };
        
        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        etrading::setForwardRateEquivalentDiscountFactors( fixingDates, expectedForwardRates, "USDYC", "USD6ML" );

        // Get ForwardRates
        const std::vector<double> actualForwardRates
            = etrading::getCurveForwardRates( fixingDates, "USDYC", "USD6ML", etrading::NO_CHANGE, "", etrading::NONE_BOOL ); // isFwdInter = false
        
        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size() );

        const double tolerance = 1e-8;
        for( size_t i = 0; i < actualForwardRates.size(); ++i )
        {
            EXPECT_NEAR( actualForwardRates[i], expectedForwardRates[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD6ML_BASIS_SetForwardRates_10PCT )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> fixingDates = {
            LADate("20190101"),
            LADate("20200101"),
            LADate("20210101"),
            LADate("20220101"),
            LADate("20230101") };

        // Constant Forward Rates of 10%
        const std::vector<double> expectedForwardRates = {
            0.10,
            0.10,
            0.10,
            0.10,
            0.10 };
        
        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        etrading::setForwardRateEquivalentDiscountFactors( fixingDates, expectedForwardRates, "USDYC", "USD6ML" );

        // Get ForwardRates
        const std::vector<double> actualForwardRates
            = etrading::getCurveForwardRates( fixingDates, "USDYC", "USD6ML", etrading::NO_CHANGE, "", etrading::NONE_BOOL ); // isFwdInter = false
        
        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size() );

        const double tolerance = 1e-8;
        for( size_t i = 0; i < actualForwardRates.size(); ++i )
        {
            EXPECT_NEAR( actualForwardRates[i], expectedForwardRates[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD6ML_BASIS_SetForwardRates_25PCT )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> fixingDates = {
            LADate("20190101"),
            LADate("20200101"),
            LADate("20210101"),
            LADate("20220101"),
            LADate("20230101") };

        // Constant Forward Rates of 25%
        const std::vector<double> expectedForwardRates = {
            0.25,
            0.25,
            0.25,
            0.25,
            0.25 };
        
        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        etrading::setForwardRateEquivalentDiscountFactors( fixingDates, expectedForwardRates, "USDYC", "USD6ML" );

        // Get ForwardRates
        const std::vector<double> actualForwardRates
            = etrading::getCurveForwardRates( fixingDates, "USDYC", "USD6ML", etrading::NO_CHANGE, "", etrading::NONE_BOOL ); // isFwdInter = false
        
        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size() );

        const double tolerance = 1e-8;
        for( size_t i = 0; i < actualForwardRates.size(); ++i )
        {
            EXPECT_NEAR( actualForwardRates[i], expectedForwardRates[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USD6ML_BASIS_SetForwardRates_2PCT_LargeDataSet )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation_api::tryMeCurveDiscountFactorsDisplay( "USDYC", "USD6ML" ).discountFactors_;

         // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<LADate> fixingDates = {
            LADate("20190101"),
            LADate("20190201"),
            LADate("20190301"),
            LADate("20190401"),
            LADate("20190501"),
            LADate("20190601"),
            LADate("20190701"),
            LADate("20190801"),
            LADate("20190901"),
            LADate("20191001"),
            LADate("20191101"),
            LADate("20191201"),
            LADate("20200101"),
            LADate("20200201"),
            LADate("20200301"),
            LADate("20200401"),
            LADate("20200501"),
            LADate("20200601"),
            LADate("20200701"),
            LADate("20200801"),
            LADate("20200901"),
            LADate("20201001"),
            LADate("20201101"),
            LADate("20201201"),
            LADate("20210101"),
            LADate("20210201"),
            LADate("20210301"),
            LADate("20210401"),
            LADate("20210501"),
            LADate("20210601"),
            LADate("20210701"),
            LADate("20210801"),
            LADate("20211001"), 
            LADate("20211101"),
            LADate("20211201") };

        const std::vector<double> expectedForwardRates = {
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02,
            0.02 };
            

        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        etrading::setForwardRateEquivalentDiscountFactors( fixingDates, expectedForwardRates, "USDYC", "USD6ML" );

        // Get ForwardRates
        const std::vector<double> actualForwardRates
            = etrading::getCurveForwardRates( fixingDates, "USDYC", "USD6ML", etrading::NO_CHANGE, "", etrading::NONE_BOOL ); // isFwdInter = false
        
        // Check Overwritten Discount Factor Size and Results
        ASSERT_EQ( actualForwardRates.size(), expectedForwardRates.size() );

        const double tolerance = 1e-8;
        for( size_t i = 0; i < actualForwardRates.size(); ++i )
        {
            EXPECT_NEAR( actualForwardRates[i], expectedForwardRates[i], tolerance );
        }
    }

    TEST_F( TestCurveStreaming, UNIT_TestValidationUtility_CurveIndexAliasList )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Test the Curve Validation Utility function CurveIndexList, which should return a vector of aliases for a given curve index
        LAStringVector USDOISCurveAliasList   = etrading::curveIndexAliasList( "USDYC", "USDOIS" );
        LAStringVector USD3MLCurveAliasList   = etrading::curveIndexAliasList( "USDYC", "USD3ML" );
        LAStringVector USD6MLCurveAliasList   = etrading::curveIndexAliasList( "USDYC", "USD6ML" );

        // 1. Test USDOIS Curve Index Alias List
        ASSERT_EQ( USDOISCurveAliasList.size(), 5 );
        EXPECT_EQ( "OIS", USDOISCurveAliasList[0] );
        EXPECT_EQ( "USDDF", USDOISCurveAliasList[1] );
        EXPECT_EQ( "USDDF_NOCSA", USDOISCurveAliasList[2] );
        EXPECT_EQ( "USDDF_USDCSA", USDOISCurveAliasList[3] );
        EXPECT_EQ( "USDOIS", USDOISCurveAliasList[4] );

        // 2. Test STD USD3ML Curve Index Alias List
        ASSERT_EQ( USD3MLCurveAliasList.size(), 2 );
        EXPECT_EQ( "STD", USD3MLCurveAliasList[0] );
        EXPECT_EQ( "USD3ML", USD3MLCurveAliasList[1] );
        
        // 3. Test BASIS USD6ML Curve Index Alias List
        ASSERT_EQ( USD6MLCurveAliasList.size(), 2 );
        EXPECT_EQ( "3M6MBasis", USD6MLCurveAliasList[0] );
        EXPECT_EQ( "USD6ML", USD6MLCurveAliasList[1] );
    }

    TEST_F( TestCurveStreaming, UNIT_TestValidationUtility_CurveFrequencyAsYearFraction )
    {
		// TODO: Relax this constraint; Curve Results will be disabled whilst the DisableCurveResults class remains in scope. It's destructor will re-enable the CurveResultsObject
		etrading::DisableCurveResults d;

        // Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Test the Curve Validation Utility function CurveIndexList, which should return a vector of aliases for a given curve index
        const double frequencyUSDOIS        = etrading::getCurveFrequencyAsYearFraction( "USDYC", "USDOIS" );
        const double frequencyUSD3ML        = etrading::getCurveFrequencyAsYearFraction( "USDYC", "USD3ML" );
        const double frequencyUSD6ML        = etrading::getCurveFrequencyAsYearFraction( "USDYC", "USD6ML" );

        // Check Curve Frequency Year Fraction Results
        const double tolerance = 1e-9;
        const double oneDay = 1.0 / 365.0; // ACT/365 Fixed

        EXPECT_NEAR( frequencyUSDOIS, oneDay, tolerance );
        EXPECT_NEAR( frequencyUSD3ML, 0.25, tolerance );
        EXPECT_NEAR( frequencyUSD6ML, 0.5, tolerance );
    }

    TEST_F( TestCurveStreaming, UNIT_TestValidationUtility_CurveType )
    {
		// TODO: Relax this constraint; Curve Results will be disabled whilst the DisableCurveResults class remains in scope. It's destructor will re-enable the CurveResultsObject
		etrading::DisableCurveResults d;

        // Load Curves
        const std::string loadUSDOIS        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Test that we can identify STD curves, which are treated differently for setting discount factors and forward rates
        // STD curves have discount factor and forward rate data tables. All other curves have discount factor data tables only.
        const bool isSTDCurveUSDOIS         = etrading::isSTDCurve( "USDYC", "USDOIS" );
        const bool isSTDCurveUSD3ML         = etrading::isSTDCurve( "USDYC", "USD3ML" );
        const bool isSTDCurveUSD6ML         = etrading::isSTDCurve( "USDYC", "USD6ML" );

        EXPECT_EQ( false, isSTDCurveUSDOIS );  // OIS Curve   = false
        EXPECT_EQ( true,  isSTDCurveUSD3ML );  // Swap Curve  = true
        EXPECT_EQ( false, isSTDCurveUSD6ML );  // Basis Curve = false

     }

}