// TestCurveStreaming.cpp

/*
 * @brief			Tests to test the curve streaming methods
 * @Created:		15th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

#include "Dependency.h"
#include "ReadDataFile.h"
#include "GetGoogleTestFolder.h"
#include "ResultsProcessor.h"
#include "tryMeLWO.h"
#include "tryAqCurvesDiscountFactor.h"
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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Actual Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        std::vector<double> actualDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Compare Results with Stored Snapshot Results - Allow Rebasing
        const double tolerance = 1e-6;
        CheckTestResultsAndRebaseOnRequest( actualDiscountFactors, TEST_FOLDER.c_str(), outputsRawDiscountFactors.c_str(), tolerance );
    }

    TEST_F( TestCurveStreaming, UNIT_USDYC_USDOIS_OIS_Overwrite_DiscountFactors )
    {
		// Load Curves
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Overwrite Discount Factors - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> newPaymentDates = {
            AQLDate("20190101"),
            AQLDate("20200101"),
            AQLDate("20210101"),
            AQLDate("20220101"),
            AQLDate("20230101") };

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Overwrite Discount Factors - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> newPaymentDates = {
            AQLDate("20190101"),
            AQLDate("20190102"),
            AQLDate("20190103"),
            AQLDate("20190104"),
            AQLDate("20190105"),
            AQLDate("20190106"),
            AQLDate("20190107"),
            AQLDate("20190108"),
            AQLDate("20190109"),
            AQLDate("20190110"),
            AQLDate("20190111"),
            AQLDate("20190112"),
            AQLDate("20190113"),
            AQLDate("20190114"),
            AQLDate("20190115"),
            AQLDate("20190116"),
            AQLDate("20190117"),
            AQLDate("20190118"),
            AQLDate("20190119"),
            AQLDate("20190120"),
            AQLDate("20190121"),
            AQLDate("20190122"),
            AQLDate("20190123"),
            AQLDate("20190124"),
            AQLDate("20190125"),
            AQLDate("20190126"),
            AQLDate("20190127"),
            AQLDate("20190128"),
            AQLDate("20190129"),
            AQLDate("20190130"),
            AQLDate("20190131") };

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Discount Factors - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> newPaymentDates = {
            AQLDate("20190101"),
            AQLDate("20200101"),
            AQLDate("20210101"),
            AQLDate("20220101"),
            AQLDate("20230101") };

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Discount Factors - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> newPaymentDates = {
            AQLDate("20190101"),
            AQLDate("20190102"),
            AQLDate("20190103"),
            AQLDate("20190104"),
            AQLDate("20190105"),
            AQLDate("20190106"),
            AQLDate("20190107"),
            AQLDate("20190108"),
            AQLDate("20190109"),
            AQLDate("20190110"),
            AQLDate("20190111"),
            AQLDate("20190112"),
            AQLDate("20190113"),
            AQLDate("20190114"),
            AQLDate("20190115"),
            AQLDate("20190116"),
            AQLDate("20190117"),
            AQLDate("20190118"),
            AQLDate("20190119"),
            AQLDate("20190120"),
            AQLDate("20190121"),
            AQLDate("20190122"),
            AQLDate("20190123"),
            AQLDate("20190124"),
            AQLDate("20190125"),
            AQLDate("20190126"),
            AQLDate("20190127"),
            AQLDate("20190128"),
            AQLDate("20190129"),
            AQLDate("20190130"),
            AQLDate("20190131") };

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> fixingDates = {
            AQLDate("20190101"),
            AQLDate("20200101"),
            AQLDate("20210101"),
            AQLDate("20220101"),
            AQLDate("20230101") };

        // Constant Forward Rates of 2.0%
        const std::vector<double> expectedForwardRates = {
            0.02,
            0.02,
            0.02,
            0.02,
            0.02 };
        
        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        const bool setEquivalentDiscountFactors = true;
        AQLString setForwardsResult = etrading::setCurveForwardRates( "USDYC", "USD3ML", fixingDates, expectedForwardRates, setEquivalentDiscountFactors );

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> fixingDates = {
            AQLDate("20190101"),
            AQLDate("20200101"),
            AQLDate("20210101"),
            AQLDate("20220101"),
            AQLDate("20230101") };

        // Constant Forward Rates of 10%
        const std::vector<double> expectedForwardRates = {
            0.10,
            0.10,
            0.10,
            0.10,
            0.10 };
        
        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        const bool setEquivalentDiscountFactors = true;
        AQLString setForwardsResult = etrading::setCurveForwardRates( "USDYC", "USD3ML", fixingDates, expectedForwardRates, setEquivalentDiscountFactors );

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> fixingDates = {
            AQLDate("20190101"),
            AQLDate("20200101"),
            AQLDate("20210101"),
            AQLDate("20220101"),
            AQLDate("20230101") };

        // Constant Forward Rates of 25%
        const std::vector<double> expectedForwardRates = {
            0.25,
            0.25,
            0.25,
            0.25,
            0.25 };
        
        // Solve for the Forward Rate Equivalent Discount Factors and then Set these Discount Factors
        const bool setEquivalentDiscountFactors = true;
        AQLString setForwardsResult = etrading::setCurveForwardRates( "USDYC", "USD3ML", fixingDates, expectedForwardRates, setEquivalentDiscountFactors );

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

         // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> fixingDates = {
            AQLDate("20190101"),
            AQLDate("20190201"),
            AQLDate("20190301"),
            AQLDate("20190401"),
            AQLDate("20190501"),
            AQLDate("20190601"),
            AQLDate("20190701"),
            AQLDate("20190801"),
            AQLDate("20190901"),
            AQLDate("20191001"),
            AQLDate("20191101"),
            AQLDate("20191201"),
            AQLDate("20200101"),
            AQLDate("20200201"),
            AQLDate("20200301"),
            AQLDate("20200401"),
            AQLDate("20200501"),
            AQLDate("20200601"),
            AQLDate("20200701"),
            AQLDate("20200801"),
            AQLDate("20200901"),
            AQLDate("20201001"),
            AQLDate("20201101"),
            AQLDate("20201201"),
            AQLDate("20210101"),
            AQLDate("20210201"),
            AQLDate("20210301"),
            AQLDate("20210401"),
            AQLDate("20210501"),
            AQLDate("20210601"),
            AQLDate("20210701"),
            AQLDate("20210801"),
            AQLDate("20211001"), 
            AQLDate("20211101"),
            AQLDate("20211201") };

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
        AQLString setForwardsResult = etrading::setCurveForwardRates( "USDYC", "USD3ML", fixingDates, expectedForwardRates, setEquivalentDiscountFactors );

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> fixingDates = {
            AQLDate("20190101"),
            AQLDate("20200101"),
            AQLDate("20210101"),
            AQLDate("20220101"),
            AQLDate("20230101") };

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const ReadDataFile::Load inputFile( inputsRawDiscountFactors.c_str() );
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( inputFile["curveCollection"], inputFile["curveIndex"] ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> fixingDates = {
            AQLDate("20190101"),
            AQLDate("20200101"),
            AQLDate("20210101"),
            AQLDate("20220101"),
            AQLDate("20230101") };

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( "USDYC", "USD3ML" ).discountFactors_;

        // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> fixingDates = {
            AQLDate("20190101"),
            AQLDate("20200101"),
            AQLDate("20210101"),
            AQLDate("20220101"),
            AQLDate("20230101") };

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Original Discount Factors
        const std::vector<double> originalDiscountFactors = validation::tryAqCurvesDiscountFactorsDisplay( "USDYC", "USD6ML" ).discountFactors_;

         // Overwrite Forward Rates - *** NOTE ASCENDING ORDER REQUIRED ***
        const std::vector<AQLDate> fixingDates = {
            AQLDate("20190101"),
            AQLDate("20190201"),
            AQLDate("20190301"),
            AQLDate("20190401"),
            AQLDate("20190501"),
            AQLDate("20190601"),
            AQLDate("20190701"),
            AQLDate("20190801"),
            AQLDate("20190901"),
            AQLDate("20191001"),
            AQLDate("20191101"),
            AQLDate("20191201"),
            AQLDate("20200101"),
            AQLDate("20200201"),
            AQLDate("20200301"),
            AQLDate("20200401"),
            AQLDate("20200501"),
            AQLDate("20200601"),
            AQLDate("20200701"),
            AQLDate("20200801"),
            AQLDate("20200901"),
            AQLDate("20201001"),
            AQLDate("20201101"),
            AQLDate("20201201"),
            AQLDate("20210101"),
            AQLDate("20210201"),
            AQLDate("20210301"),
            AQLDate("20210401"),
            AQLDate("20210501"),
            AQLDate("20210601"),
            AQLDate("20210701"),
            AQLDate("20210801"),
            AQLDate("20211001"), 
            AQLDate("20211101"),
            AQLDate("20211201") };

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

        // Test the Curve Validation Utility function CurveIndexList, which should return a vector of aliases for a given curve index
        AQLStringVector USDOISCurveAliasList   = etrading::curveIndexAliasList( "USDYC", "USDOIS" );
        AQLStringVector USD3MLCurveAliasList   = etrading::curveIndexAliasList( "USDYC", "USD3ML" );
        AQLStringVector USD6MLCurveAliasList   = etrading::curveIndexAliasList( "USDYC", "USD6ML" );

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

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
        const std::string loadUSDOIS        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
        const std::string loadUSD3ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
        const std::string loadUSD6ML        = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );

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