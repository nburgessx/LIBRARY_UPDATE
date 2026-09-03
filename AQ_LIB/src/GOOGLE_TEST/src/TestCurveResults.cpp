/*
 * @brief			Tests to validate the Curve Results and Container Object
 * @Created:		17th May 2019
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

// Include: Google Test Library
#include <gTest/gTest.h>

// Test Includes
#include "InitializeMLibGoogleTest.h"
#include "CurveResultsContainer.h"
#include "tryMeCurveResults.h"
#include "CurveUtilities.h"						// Contains Legcacy Curve Discount Factor and Forward Rate Methods
#include "LACurveForwardRateHelpers.h"			// Also contains Legcacy Curve Discount Factor and Forward Rate Methods
#include "ParameterValidation.h"				// Contains etrading::getDataInstance() method
#include "GetGoogleTestFolder.h"
#include "tryMeLWO.h"
#include "RecordMacros.h"						// Logfile Macros

// Test Folder
const std::string TEST_FOLDER = "ETrading/Curves/TestCurveResults/";

// Test Files: USD CURVES
const std::string fileNameUSDOIS    = TEST_FOLDER + "USD_OIS_CURVE.JSON";
const std::string fileNameUSD3ML    = TEST_FOLDER + "USD_SWAP_3M_CURVE.JSON";
const std::string fileNameUSD6ML    = TEST_FOLDER + "USD_BASIS_6M_CURVE.JSON";
const std::string fileNameUSD1ML    = TEST_FOLDER + "USD_BASIS_1M_CURVE.JSON";
const std::string fileNameUSD12ML   = TEST_FOLDER + "USD_BASIS_12M_CURVE.JSON";

// Test Files: EUR CURVES
const std::string fileNameEUROIS    = TEST_FOLDER + "EUR_OIS_CURVE.JSON";
const std::string fileNameEUR3ML    = TEST_FOLDER + "EUR_SWAP_3M_CURVE.JSON";
const std::string fileNameEUR6ML    = TEST_FOLDER + "EUR_SWAP_6M_CURVE.JSON";
const std::string fileNameEUR1ML    = TEST_FOLDER + "EUR_BASIS_1M_CURVE.JSON";
const std::string fileNameEUR12ML   = TEST_FOLDER + "EUR_BASIS_12M_CURVE.JSON";


namespace google_test
{
    // Tolerance Settings
    const double discountFactorTolerance = 1e-6;
    const double forwardRateTolerance = 1e-6;


    // TEST FIXTURE CONSTRUCTOR
    // ------------------------------------------------------------------------------------------------------------------
    class TestCurveResults : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
        public:

            // Test data has been taken from the example workbook "meCurveResults.xlsx"
            // This file can be found in folder ... %MLIBQ%/resources/examples/MLIBQ_EXAMPLES/Curve/meCurveResults.xlsx"
            virtual void SetUp()
            {
                curveType_          = "OIS";            // OIS_CURVETYPE
                curveTenor_         = "1D";             // CURVE_TENOR_1D
                curveCollection_    = "USDYC";
                curveIndex_         = "OIS";

                curveLVB_ = 
                    { { LAString("CURVETYPE"),                      LAString("OIS")             },
                      { LAString("CURVETENOR"),                     LAString("1D")              },
                      { LAString("CURVECOLLECTION"),                LAString("USDYC")           }, 
                      { LAString("CURVEINDEX"),                     LAString("OIS")             } };

                parameterLVB_ = 
                    { { LAString("ASOFDATE"),                       LAString(43571)             },      // 16-Apr-2019
                      { LAString("JOINDATE"),                       LAString(43571)             },      // 16-Apr-2019
                      { LAString("SPOTDATE"),                       LAString(43573)		        },		// 18-Apr-2019
					  { LAString("DAYCOUNT"),                       LAString("ACT/360")         },
					  { LAString("INTERPOLATION"),                  LAString("SPLINE")          },
                      { LAString("FIXINGBUSINESSDAYADJUSTMENT"),    LAString("MOD_FOLLOWING")   },
                      { LAString("FIXINGCALENDAR"),                 LAString("NYB")  }          };

                paymentDatesIn_ = 
                    { LAString(43571),
                      LAString(43661), 
                      LAString(43751), 
                      LAString(43841), 
                      LAString(43931), 
                      LAString(44021),
                      LAString(44111), 
                      LAString(44201), 
                      LAString(44291), 
                      LAString(44381), 
                      LAString(44471), 
                      LAString(44561),
                      LAString(44651), 
                      LAString(44741), 
                      LAString(44831), 
                      LAString(44921), 
                      LAString(45011), 
                      LAString(45101),
                      LAString(45191) };

                discountFactorsIn_ = 
                    { LAString(1.0),		// Boundary Condition - Discount Factors on curve the 'AsOfDate' must be one
                      LAString(0.993791), 
                      LAString(0.987688), 
                      LAString(0.981622), 
                      LAString(0.975594), 
                      LAString(0.969602),
                      LAString(0.963648), 
                      LAString(0.957730), 
                      LAString(0.951848), 
                      LAString(0.946003), 
                      LAString(0.940193), 
                      LAString(0.934419),
                      LAString(0.928681), 
                      LAString(0.922977), 
                      LAString(0.917309), 
                      LAString(0.911676), 
                      LAString(0.906077), 
                      LAString(0.900512),
                      LAString(0.894982) };

                paymentDatesExpected_= 
                    { LADate("16-04-2019", "DD-MM-YYYY"),
                      LADate("15-07-2019", "DD-MM-YYYY"),
                      LADate("13-10-2019", "DD-MM-YYYY"),
                      LADate("11-01-2020", "DD-MM-YYYY"), 
                      LADate("10-04-2020", "DD-MM-YYYY"), 
                      LADate("09-07-2020", "DD-MM-YYYY"),
                      LADate("07-10-2020", "DD-MM-YYYY"), 
                      LADate("05-01-2021", "DD-MM-YYYY"), 
                      LADate("05-04-2021", "DD-MM-YYYY"), 
                      LADate("04-07-2021", "DD-MM-YYYY"), 
                      LADate("02-10-2021", "DD-MM-YYYY"), 
                      LADate("31-12-2021", "DD-MM-YYYY"),
                      LADate("31-03-2022", "DD-MM-YYYY"),
                      LADate("29-06-2022", "DD-MM-YYYY"), 
                      LADate("27-09-2022", "DD-MM-YYYY"), 
                      LADate("26-12-2022", "DD-MM-YYYY"), 
                      LADate("26-03-2023", "DD-MM-YYYY"), 
                      LADate("24-06-2023", "DD-MM-YYYY"),
                      LADate("22-09-2023", "DD-MM-YYYY") };

                discountFactorsExpected_= 
                    { 1.0,			// Boundary Condition - Discount Factors on the curve 'AsOfDate' must be one
                      0.993791, 
                      0.987688, 
                      0.981622, 
                      0.975594, 
                      0.969602,
                      0.963648,
                      0.957730, 
                      0.951848, 
                      0.946003,
                      0.940193, 
                      0.934419,
                      0.928681,
                      0.922977, 
                      0.917309, 
                      0.911676,
                      0.906077, 
                      0.900512,
                      0.894982 };

            }

            virtual void TearDown() {}

            // Helper Function to Create a DiscountFactorTable of type LAStringMatrix from a vector of paymentDates and discountFactors of type LAString
            LAStringMatrix createDiscountFactorTable( const std::vector<LAString> & paymentDates, const std::vector<LAString> & discountFactors )
            {
                MLIB_REQUIRE( paymentDates.size() == discountFactors.size(), "The number of paymentDates and discountFactors does not match" );
                LAStringMatrix discountFactorTable( discountFactorsIn_.size() );
                for ( size_t i = 0; i < discountFactorsIn_.size(); ++i )
                {
                    std::vector<LAString> currentDataRow(2); // 2 Columns
                    currentDataRow[0]       = paymentDatesIn_[i];
                    currentDataRow[1]       = discountFactorsIn_[i];
                    discountFactorTable[i]  = currentDataRow;
                }
                return discountFactorTable;
            }


            // Member Data
            std::string curveType_;
            std::string curveTenor_;
            std::string curveCollection_;
            std::string curveIndex_;

            LAStringMatrix curveLVB_;
            LAStringMatrix parameterLVB_;
            LAStringMatrix discountFactorLVB_;

            std::vector<LAString> paymentDatesIn_;
            std::vector<LAString> discountFactorsIn_;
            std::vector<LADate> paymentDatesExpected_;
            std::vector<double> discountFactorsExpected_;
    };

    // ------------------------------------------------------------------------------------------------------------------


    // This Test was manufactured with curve asOfDate 16-Apr-2019 with discount factors based on a zero rate of 2.5% using DF=exp(-r.t) with Act/365.25 as the daycount
    TEST_F( TestCurveResults, UNIT_ConvertFromDiscountFactorTableToDateValueVectors )
    {
        // Populate Input LAStringMatrix
        ASSERT_EQ( paymentDatesIn_.size(), discountFactorsIn_.size() );
        LAStringMatrix DiscountFactorTableIn( discountFactorsIn_.size() );
        
        for( size_t i = 0; i < discountFactorsIn_.size(); ++i )
        {
            DiscountFactorTableIn[i].push_back( paymentDatesIn_[i] );        // Column 1 must contain Dates as LAString
            DiscountFactorTableIn[i].push_back( discountFactorsIn_[i] );     // Column 2 must contain Values as LAString
        }

        // Call Method which casts LAStringMatrix and populates DateVector and ValueVector results
        std::vector<LADate> paymentDatesOut;
        std::vector<double> discountFactorsOut;
        
        etrading::populateDateValueVectorsFromStringMatrix( paymentDatesOut, discountFactorsOut, DiscountFactorTableIn );

        // Check Test Result Dimensions to Prevent Access Violations
        ASSERT_EQ( paymentDatesOut.size(), discountFactorsOut.size() );
        ASSERT_EQ( paymentDatesExpected_.size(), discountFactorsExpected_.size() );
        ASSERT_EQ( discountFactorsOut.size(), discountFactorsExpected_.size() );

        // Test Expected Results Match Actual Results
        for ( size_t i = 0; i < discountFactorsOut.size(); ++i )
        {
            EXPECT_EQ( paymentDatesExpected_[i], paymentDatesOut[i] );
            EXPECT_NEAR( discountFactorsExpected_[i], discountFactorsOut[i], discountFactorTolerance );
        }
    }


    // Test that we can update discount factors within curve results with no throw
    TEST_F( TestCurveResults, UNIT_DiscountFactors_Update )
    {
        // Check Inputs Sizes Consistent
        ASSERT_EQ( paymentDatesIn_.size(), discountFactorsIn_.size() );
        
        // Create Discount Factor Table
        LAStringMatrix discountFactorTable = createDiscountFactorTable( paymentDatesIn_, discountFactorsIn_ );
        
        // Update Discount Factors within Curve Results
        EXPECT_NO_THROW( validation_api::tryMeCurveResultsDiscountFactorsUpdate( curveLVB_, parameterLVB_, discountFactorTable ) );
    }


    // Test that we can display discount factors with the correct results
    TEST_F( TestCurveResults, UNIT_DiscountFactors_Display )
    {
        // Check Inputs Sizes Consistent
        ASSERT_EQ( paymentDatesIn_.size(), discountFactorsIn_.size() );
        
        // Create Discount Factor Table
        LAStringMatrix discountFactorTable = createDiscountFactorTable( paymentDatesIn_, discountFactorsIn_ );
        
        // Update Discount Factors within Curve Results
        validation_api::tryMeCurveResultsDiscountFactorsUpdate( curveLVB_, parameterLVB_, discountFactorTable );

        // Display the results
        etrading::VariantMatrix resultsMatrix = validation_api::tryMeCurveResultsDiscountFactorsDisplayAll( curveCollection_, curveIndex_ );

        // Validate Result Dimensions
        ASSERT_EQ( resultsMatrix.size(), paymentDatesExpected_.size() );
        ASSERT_EQ( resultsMatrix.size(), discountFactorsExpected_.size() );
        
        const size_t SIZE_ZERO = 0;
        const size_t SIZE_TWO = 2;

        ASSERT_GT( resultsMatrix.size(), SIZE_ZERO );       // we expect more than 1 row of data
        ASSERT_EQ( resultsMatrix[0].size(), SIZE_TWO );    // we expect exactly 2 columns of data

        // Check Results
        for ( size_t i = 0; i < resultsMatrix.size(); ++i )
        {
            EXPECT_EQ( paymentDatesExpected_[i], resultsMatrix[i][0] ); // Results Column 1
            EXPECT_NEAR( discountFactorsExpected_[i], resultsMatrix[i][1], discountFactorTolerance ); // Results Column 2
        }
    }


    // Test that we can display discount factors with the correct results
    TEST_F( TestCurveResults, UNIT_DiscountFactors_GetDFsForPaymentDatesSpecifiedOnly )
    {
        // Check Inputs Sizes Consistent
        ASSERT_EQ( paymentDatesIn_.size(), discountFactorsIn_.size() );
        
        // Create Discount Factor Table
        LAStringMatrix discountFactorTable = createDiscountFactorTable( paymentDatesIn_, discountFactorsIn_ );
        
        // Update Discount Factors within Curve Results
        validation_api::tryMeCurveResultsDiscountFactorsUpdate( curveLVB_, parameterLVB_, discountFactorTable );

        // Display the results
        etrading::VectorDouble discountFactorResults = validation_api::tryMeCurveResultsDiscountFactorsDisplay( curveCollection_, curveIndex_, paymentDatesExpected_ );

        // Validate Result Dimensions
        ASSERT_EQ( discountFactorResults.size(), paymentDatesExpected_.size() );
        ASSERT_EQ( discountFactorResults.size(), discountFactorsExpected_.size() );

        // Check Results
        for ( size_t i = 0; i < discountFactorResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsExpected_[i], discountFactorResults[i], discountFactorTolerance );
        }
    }


    // Test that we can delete discount factors within curve results with no throw
    TEST_F( TestCurveResults, UNIT_DiscountFactors_Delete )
    {
        // Check Inputs Sizes Consistent
        ASSERT_EQ( paymentDatesIn_.size(), discountFactorsIn_.size() );
        
        // Create Discount Factor Table
        LAStringMatrix discountFactorTable = createDiscountFactorTable( paymentDatesIn_, discountFactorsIn_ );
        
        // Update Discount Factors within Curve Results
        EXPECT_NO_THROW( validation_api::tryMeCurveResultsDiscountFactorsUpdate( curveLVB_, parameterLVB_, discountFactorTable ) );

        // Delete Discount Factors within Curve Results
        EXPECT_NO_THROW( validation_api::tryMeCurveResultsDelete( curveCollection_, curveIndex_ ) );
    }


    // Test that we can delete all discount factors within curve results with no throw
    TEST_F( TestCurveResults, UNIT_DiscountFactors_DeleteAll )
    {
        // Check Inputs Sizes Consistent
        ASSERT_EQ( paymentDatesIn_.size(), discountFactorsIn_.size() );
        
        // Create Discount Factor Table
        LAStringMatrix discountFactorTable = createDiscountFactorTable( paymentDatesIn_, discountFactorsIn_ );
        
        // Update Discount Factors within Curve Results
        EXPECT_NO_THROW( validation_api::tryMeCurveResultsDiscountFactorsUpdate( curveLVB_, parameterLVB_, discountFactorTable ) );

        // Delete All Discount Factors within Curve Results
        EXPECT_NO_THROW( validation_api::tryMeCurveResultsDeleteAll() );
    }


    // Test that we can delete all discount factors within curve results with no throw
    TEST_F( TestCurveResults, UNIT_DiscountFactors_ThrowIfObjectDoesNotExist )
    {
        // Expect display function to throw if no results to display
        EXPECT_ANY_THROW( validation_api::tryMeCurveResultsDiscountFactorsDisplayAll( curveCollection_, curveIndex_ ) );
        
        // Expect get discount factors function to throw if no results to display
        EXPECT_ANY_THROW( validation_api::tryMeCurveResultsDiscountFactorsDisplay( curveCollection_, curveIndex_,  paymentDatesExpected_ ) );
    }


    // Test that we can update discount factors, display and tidy-up afterwards
    TEST_F( TestCurveResults, UNIT_DiscountFactors_UpdateThenDisplayThenDelete )
    {
        // Check Inputs Sizes Consistent
        ASSERT_EQ( paymentDatesIn_.size(), discountFactorsIn_.size() );
        
        // Create Discount Factor Table
        LAStringMatrix discountFactorTable = createDiscountFactorTable( paymentDatesIn_, discountFactorsIn_ );
        
        // 1. Create DFs and Test Results
        // ================================================
        // Update Discount Factors within Curve Results
        EXPECT_NO_THROW( validation_api::tryMeCurveResultsDiscountFactorsUpdate( curveLVB_, parameterLVB_, discountFactorTable ) );
        
        // Display the results
        etrading::VariantMatrix resultsMatrix = validation_api::tryMeCurveResultsDiscountFactorsDisplayAll( curveCollection_, curveIndex_ );

        // Validate Result Dimensions
        ASSERT_EQ( resultsMatrix.size(), paymentDatesExpected_.size() );
        ASSERT_EQ( resultsMatrix.size(), discountFactorsExpected_.size() );
        
        const size_t SIZE_ZERO = 0;
        const size_t SIZE_TWO = 2;

        ASSERT_GT( resultsMatrix.size(), SIZE_ZERO );       // we expect more than 1 row of data
        ASSERT_EQ( resultsMatrix[0].size(), SIZE_TWO );    // we expect exactly 2 columns of data

        // Check Results
        for ( size_t i = 0; i < resultsMatrix.size(); ++i )
        {
            EXPECT_EQ( paymentDatesExpected_[i], resultsMatrix[i][0] ); // Results Column 1
            EXPECT_NEAR( discountFactorsExpected_[i], resultsMatrix[i][1], discountFactorTolerance ); // Results Column 2
        }

        // 2. Delete DFs
        // ================================================
        // Delete Discount Factors within Curve Results
        EXPECT_NO_THROW( validation_api::tryMeCurveResultsDelete( curveCollection_, curveIndex_ ) );

        // 3. Check DFs are Deleted
        // ================================================
        // Expect display function to throw if no results to display
        EXPECT_ANY_THROW( validation_api::tryMeCurveResultsDiscountFactorsDisplayAll( curveCollection_, curveIndex_ ) );
        
        // Expect get discount factors function to throw if no results to display
        EXPECT_ANY_THROW( validation_api::tryMeCurveResultsDiscountFactorsDisplay( curveCollection_, curveIndex_,  paymentDatesExpected_ ) );
    }



    // TEST FIXTURE CONSTRUCTOR
    // ------------------------------------------------------------------------------------------------------------------

    // Declare Test Fixture Class
    class TestCurveResultsVsObjectPool : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
    public:

        virtual void SetUp()
        {
            loadUSDCurves();
            loadEURCurves();
        }

        virtual void TearDown() {}

        virtual void loadUSDCurves()
        {
            // Load USD Curves
            const std::string curveObjectUSDOIS_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
            const std::string curveObjectUSD3ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
            const std::string curveObjectUSD6ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );
            const std::string curveObjectUSD1ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD1ML, etrading::JSON );
            const std::string curveObjectUSD12ML_   = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD12ML, etrading::JSON );
        }
        
        virtual void loadEURCurves()
        {
            // Load EUR Curves
            const std::string curveObjectEUROIS_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUROIS, etrading::JSON );
            const std::string curveObjectEUR3ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR3ML, etrading::JSON );
            const std::string curveObjectEUR6ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR6ML, etrading::JSON );
            const std::string curveObjectEUR1ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR1ML, etrading::JSON );
            const std::string curveObjectEUR12ML_   = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR12ML, etrading::JSON );
        }
    };

    // ------------------------------------------------------------------------------------------------------------------
    
    // Test that Discount Factors Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareDiscountFactors_EURYC_OIS )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "OIS";

        DateVector paymentDates { LADate("20190503"),       // Curve AsOfDate
                                  LADate("20200503"),       // 1Y
                                  LADate("20210503"),       // 2Y
                                  LADate("20220503"),       // 3Y
                                  LADate("20230503"),       // 4Y
                                  LADate("20240503"),       // 5Y
                                  LADate("20250503"),       // 6Y
                                  LADate("20260503"),       // 7Y
                                  LADate("20270503"),       // 8Y
                                  LADate("20280503"),       // 9Y
                                  LADate("20290503") };     // 10Y


        // 3.   Get Discount Factors from Curve Results
        // ================================================
        const DoubleVector discountFactorsFromCurveResults = etrading::getDiscountFactorsFromCurveResultsObject( curveCollection, curveIndex, paymentDates );
        
        // 4.   Get Discount Factors from Object Pool
        // ================================================
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        etrading::populateDiscountFactorConventions( curveCollection.c_str(), curveIndex.c_str(), interp, bdAdj, cal );
        
        const DoubleVector discountFactorsFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates,
                                                                                                                                 etrading::getDataInstance(),
                                                                                                                                 curveCollection.c_str(),
                                                                                                                                 etrading::getDiscountFactorDayCount(),
                                                                                                                                 bdAdj,
                                                                                                                                 cal,
                                                                                                                                 interp,
                                                                                                                                 etrading::isBasisFlagForDiscountFactor(),
                                                                                                                                 curveIndex.c_str() );
        // 5.   Compare Discount Factors from 3. and 4. above
        // ================================================
        ASSERT_EQ( discountFactorsFromCurveResults.size(), discountFactorsFromEntityPool.size() );
        for( size_t i = 0; i < discountFactorsFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsFromCurveResults[i], discountFactorsFromEntityPool[i], discountFactorTolerance  );
        }
    }


    // Test that Discount Factors Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareDiscountFactors_EURYC_EUR1ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR1ML";

        DateVector paymentDates { LADate("20190503"),       // Curve AsOfDate
                                  LADate("20200503"),       // 1Y
                                  LADate("20210503"),       // 2Y
                                  LADate("20220503"),       // 3Y
                                  LADate("20230503"),       // 4Y
                                  LADate("20240503"),       // 5Y
                                  LADate("20250503"),       // 6Y
                                  LADate("20260503"),       // 7Y
                                  LADate("20270503"),       // 8Y
                                  LADate("20280503"),       // 9Y
                                  LADate("20290503") };     // 10Y


        // 3.   Get Discount Factors from Curve Results
        // ================================================
        const DoubleVector discountFactorsFromCurveResults = etrading::getDiscountFactorsFromCurveResultsObject( curveCollection, curveIndex, paymentDates );
        
        // 4.   Get Discount Factors from Object Pool
        // ================================================
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        etrading::populateDiscountFactorConventions( curveCollection.c_str(), curveIndex.c_str(), interp, bdAdj, cal );
        
        const DoubleVector discountFactorsFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates,
                                                                                                                                 etrading::getDataInstance(),
                                                                                                                                 curveCollection.c_str(),
                                                                                                                                 etrading::getDiscountFactorDayCount(),
                                                                                                                                 bdAdj,
                                                                                                                                 cal,
                                                                                                                                 interp,
                                                                                                                                 etrading::isBasisFlagForDiscountFactor(),
                                                                                                                                 curveIndex.c_str() );
        // 5.   Compare Discount Factors from 3. and 4. above
        // ================================================
        ASSERT_EQ( discountFactorsFromCurveResults.size(), discountFactorsFromEntityPool.size() );
        for( size_t i = 0; i < discountFactorsFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsFromCurveResults[i], discountFactorsFromEntityPool[i], discountFactorTolerance  );
        }
    }

    
    // Test that Discount Factors Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareDiscountFactors_EURYC_EUR3ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR3ML";
    
        DateVector paymentDates { LADate("20190503"),       // Curve AsOfDate
                                  LADate("20200503"),       // 1Y
                                  LADate("20210503"),       // 2Y
                                  LADate("20220503"),       // 3Y
                                  LADate("20230503"),       // 4Y
                                  LADate("20240503"),       // 5Y
                                  LADate("20250503"),       // 6Y
                                  LADate("20260503"),       // 7Y
                                  LADate("20270503"),       // 8Y
                                  LADate("20280503"),       // 9Y
                                  LADate("20290503") };     // 10Y
    
    
        // 3.   Get Discount Factors from Curve Results
        // ================================================
        const DoubleVector discountFactorsFromCurveResults = etrading::getDiscountFactorsFromCurveResultsObject( curveCollection, curveIndex, paymentDates );
        
        // 4.   Get Discount Factors from Object Pool
        // ================================================
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        etrading::populateDiscountFactorConventions( curveCollection.c_str(), curveIndex.c_str(), interp, bdAdj, cal );
        
        const DoubleVector discountFactorsFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates,
                                                                                                                                 etrading::getDataInstance(),
                                                                                                                                 curveCollection.c_str(),
                                                                                                                                 etrading::getDiscountFactorDayCount(),
                                                                                                                                 bdAdj,
                                                                                                                                 cal,
                                                                                                                                 interp,
                                                                                                                                 etrading::isBasisFlagForDiscountFactor(),
                                                                                                                                 curveIndex.c_str() );
        // 5.   Compare Discount Factors from 3. and 4. above
        // ================================================
        ASSERT_EQ( discountFactorsFromCurveResults.size(), discountFactorsFromEntityPool.size() );
        for( size_t i = 0; i < discountFactorsFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsFromCurveResults[i], discountFactorsFromEntityPool[i], discountFactorTolerance  );
        }
    }

    // Test that Discount Factors Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareDiscountFactors_EURYC_EUR6ML_STD_SWAPCURVE )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR6ML";
    
        DateVector paymentDates { LADate("20190503"),       // Curve AsOfDate
                                  LADate("20200503"),       // 1Y
                                  LADate("20210503"),       // 2Y
                                  LADate("20220503"),       // 3Y
                                  LADate("20230503"),       // 4Y
                                  LADate("20240503"),       // 5Y
                                  LADate("20250503"),       // 6Y
                                  LADate("20260503"),       // 7Y
                                  LADate("20270503"),       // 8Y
                                  LADate("20280503"),       // 9Y
                                  LADate("20290503") };     // 10Y
    
    
        // 3.   Get Discount Factors from Curve Results
        // ================================================
        const DoubleVector discountFactorsFromCurveResults = etrading::getDiscountFactorsFromCurveResultsObject( curveCollection, curveIndex, paymentDates );
        
        // 4.   Get Discount Factors from Object Pool
        // ================================================
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        etrading::populateDiscountFactorConventions( curveCollection.c_str(), curveIndex.c_str(), interp, bdAdj, cal );
        
        const DoubleVector discountFactorsFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates,
                                                                                                                                 etrading::getDataInstance(),
                                                                                                                                 curveCollection.c_str(),
                                                                                                                                 etrading::getDiscountFactorDayCount(),
                                                                                                                                 bdAdj,
                                                                                                                                 cal,
                                                                                                                                 interp,
                                                                                                                                 etrading::isBasisFlagForDiscountFactor(),
                                                                                                                                 curveIndex.c_str() );
        // 5.   Compare Discount Factors from 3. and 4. above
        // ================================================
        ASSERT_EQ( discountFactorsFromCurveResults.size(), discountFactorsFromEntityPool.size() );
        for( size_t i = 0; i < discountFactorsFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsFromCurveResults[i], discountFactorsFromEntityPool[i], discountFactorTolerance  );
        }
    }

    // Test that Discount Factors Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareDiscountFactors_EURYC_EUR12ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR12ML";

        DateVector paymentDates { LADate("20190503"),       // Curve AsOfDate
                                  LADate("20200503"),       // 1Y
                                  LADate("20210503"),       // 2Y
                                  LADate("20220503"),       // 3Y
                                  LADate("20230503"),       // 4Y
                                  LADate("20240503"),       // 5Y
                                  LADate("20250503"),       // 6Y
                                  LADate("20260503"),       // 7Y
                                  LADate("20270503"),       // 8Y
                                  LADate("20280503"),       // 9Y
                                  LADate("20290503") };     // 10Y


        // 3.   Get Discount Factors from Curve Results
        // ================================================
        const DoubleVector discountFactorsFromCurveResults = etrading::getDiscountFactorsFromCurveResultsObject( curveCollection, curveIndex, paymentDates );
        
        // 4.   Get Discount Factors from Object Pool
        // ================================================
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        etrading::populateDiscountFactorConventions( curveCollection.c_str(), curveIndex.c_str(), interp, bdAdj, cal );
        
        const DoubleVector discountFactorsFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates,
                                                                                                                                 etrading::getDataInstance(),
                                                                                                                                 curveCollection.c_str(),
                                                                                                                                 etrading::getDiscountFactorDayCount(),
                                                                                                                                 bdAdj,
                                                                                                                                 cal,
                                                                                                                                 interp,
                                                                                                                                 etrading::isBasisFlagForDiscountFactor(),
                                                                                                                                 curveIndex.c_str() );
        // 5.   Compare Discount Factors from 3. and 4. above
        // ================================================
        ASSERT_EQ( discountFactorsFromCurveResults.size(), discountFactorsFromEntityPool.size() );
        for( size_t i = 0; i < discountFactorsFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsFromCurveResults[i], discountFactorsFromEntityPool[i], discountFactorTolerance  );
        }
    }

    // Test that Discount Factors Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareDiscountFactors_USDYC_OIS )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "OIS";
         
        DateVector paymentDates { LADate("20190503"),       // Curve AsOfDate
                                  LADate("20200503"),       // 1Y
                                  LADate("20210503"),       // 2Y
                                  LADate("20220503"),       // 3Y
                                  LADate("20230503"),       // 4Y
                                  LADate("20240503"),       // 5Y
                                  LADate("20250503"),       // 6Y
                                  LADate("20260503"),       // 7Y
                                  LADate("20270503"),       // 8Y
                                  LADate("20280503"),       // 9Y
                                  LADate("20290503") };     // 10Y


        // 3.   Get Discount Factors from Curve Results
        // ================================================
        const DoubleVector discountFactorsFromCurveResults = etrading::getDiscountFactorsFromCurveResultsObject( curveCollection, curveIndex, paymentDates );
        
        // 4.   Get Discount Factors from Object Pool
        // ================================================
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        etrading::populateDiscountFactorConventions( curveCollection.c_str(), curveIndex.c_str(), interp, bdAdj, cal );
        
        const DoubleVector discountFactorsFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates,
                                                                                                                                 etrading::getDataInstance(),
                                                                                                                                 curveCollection.c_str(),
                                                                                                                                 etrading::getDiscountFactorDayCount(),
                                                                                                                                 bdAdj,
                                                                                                                                 cal,
                                                                                                                                 interp,
                                                                                                                                 etrading::isBasisFlagForDiscountFactor(),
                                                                                                                                 curveIndex.c_str() );
        // 5.   Compare Discount Factors from 3. and 4. above
        // ================================================
        ASSERT_EQ( discountFactorsFromCurveResults.size(), discountFactorsFromEntityPool.size() );
        for( size_t i = 0; i < discountFactorsFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsFromCurveResults[i], discountFactorsFromEntityPool[i], discountFactorTolerance  );
        }
    }


    // Test that Discount Factors Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareDiscountFactors_USDYC_USD1ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD1ML";
         
        DateVector paymentDates { LADate("20190503"),       // Curve AsOfDate
                                  LADate("20200503"),       // 1Y
                                  LADate("20210503"),       // 2Y
                                  LADate("20220503"),       // 3Y
                                  LADate("20230503"),       // 4Y
                                  LADate("20240503"),       // 5Y
                                  LADate("20250503"),       // 6Y
                                  LADate("20260503"),       // 7Y
                                  LADate("20270503"),       // 8Y
                                  LADate("20280503"),       // 9Y
                                  LADate("20290503") };     // 10Y


        // 3.   Get Discount Factors from Curve Results
        // ================================================
        const DoubleVector discountFactorsFromCurveResults = etrading::getDiscountFactorsFromCurveResultsObject( curveCollection, curveIndex, paymentDates );
        
        // 4.   Get Discount Factors from Object Pool
        // ================================================
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        etrading::populateDiscountFactorConventions( curveCollection.c_str(), curveIndex.c_str(), interp, bdAdj, cal );
        
        const DoubleVector discountFactorsFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates,
                                                                                                                                 etrading::getDataInstance(),
                                                                                                                                 curveCollection.c_str(),
                                                                                                                                 etrading::getDiscountFactorDayCount(),
                                                                                                                                 bdAdj,
                                                                                                                                 cal,
                                                                                                                                 interp,
                                                                                                                                 etrading::isBasisFlagForDiscountFactor(),
                                                                                                                                 curveIndex.c_str() );
        // 5.   Compare Discount Factors from 3. and 4. above
        // ================================================
        ASSERT_EQ( discountFactorsFromCurveResults.size(), discountFactorsFromEntityPool.size() );
        for( size_t i = 0; i < discountFactorsFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsFromCurveResults[i], discountFactorsFromEntityPool[i], discountFactorTolerance  );
        }
    }

    // Test that Discount Factors Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareDiscountFactors_USDYC_USD3ML_STD_SWAPCURVE )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD3ML";
         
        DateVector paymentDates { LADate("20190503"),       // Curve AsOfDate
                                  LADate("20200503"),       // 1Y
                                  LADate("20210503"),       // 2Y
                                  LADate("20220503"),       // 3Y
                                  LADate("20230503"),       // 4Y
                                  LADate("20240503"),       // 5Y
                                  LADate("20250503"),       // 6Y
                                  LADate("20260503"),       // 7Y
                                  LADate("20270503"),       // 8Y
                                  LADate("20280503"),       // 9Y
                                  LADate("20290503") };     // 10Y
    
    
        // 3.   Get Discount Factors from Curve Results
        // ================================================
        const DoubleVector discountFactorsFromCurveResults = etrading::getDiscountFactorsFromCurveResultsObject( curveCollection, curveIndex, paymentDates );
        
        // 4.   Get Discount Factors from Object Pool
        // ================================================
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        etrading::populateDiscountFactorConventions( curveCollection.c_str(), curveIndex.c_str(), interp, bdAdj, cal );
        
        const DoubleVector discountFactorsFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates,
                                                                                                                                 etrading::getDataInstance(),
                                                                                                                                 curveCollection.c_str(),
                                                                                                                                 etrading::getDiscountFactorDayCount(),
                                                                                                                                 bdAdj,
                                                                                                                                 cal,
                                                                                                                                 interp,
                                                                                                                                 etrading::isBasisFlagForDiscountFactor(),
                                                                                                                                 curveIndex.c_str() );
        // 5.   Compare Discount Factors from 3. and 4. above
        // ================================================
        ASSERT_EQ( discountFactorsFromCurveResults.size(), discountFactorsFromEntityPool.size() );
        for( size_t i = 0; i < discountFactorsFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsFromCurveResults[i], discountFactorsFromEntityPool[i], discountFactorTolerance  );
        }
        
    }


    // Test that Discount Factors Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareDiscountFactors_USDYC_USD6ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD6ML";
         
        DateVector paymentDates { LADate("20190503"),       // Curve AsOfDate
                                  LADate("20200503"),       // 1Y
                                  LADate("20210503"),       // 2Y
                                  LADate("20220503"),       // 3Y
                                  LADate("20230503"),       // 4Y
                                  LADate("20240503"),       // 5Y
                                  LADate("20250503"),       // 6Y
                                  LADate("20260503"),       // 7Y
                                  LADate("20270503"),       // 8Y
                                  LADate("20280503"),       // 9Y
                                  LADate("20290503") };     // 10Y


        // 3.   Get Discount Factors from Curve Results
        // ================================================
        const DoubleVector discountFactorsFromCurveResults = etrading::getDiscountFactorsFromCurveResultsObject( curveCollection, curveIndex, paymentDates );
        
        // 4.   Get Discount Factors from Object Pool
        // ================================================
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        etrading::populateDiscountFactorConventions( curveCollection.c_str(), curveIndex.c_str(), interp, bdAdj, cal );
        
        const DoubleVector discountFactorsFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates,
                                                                                                                                 etrading::getDataInstance(),
                                                                                                                                 curveCollection.c_str(),
                                                                                                                                 etrading::getDiscountFactorDayCount(),
                                                                                                                                 bdAdj,
                                                                                                                                 cal,
                                                                                                                                 interp,
                                                                                                                                 etrading::isBasisFlagForDiscountFactor(),
                                                                                                                                 curveIndex.c_str() );
        // 5.   Compare Discount Factors from 3. and 4. above
        // ================================================
        ASSERT_EQ( discountFactorsFromCurveResults.size(), discountFactorsFromEntityPool.size() );
        for( size_t i = 0; i < discountFactorsFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsFromCurveResults[i], discountFactorsFromEntityPool[i], discountFactorTolerance  );
        }
    }


    // Test that Discount Factors Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareDiscountFactors_USDYC_USD12ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD12ML";
         
        DateVector paymentDates { LADate("20190503"),       // Curve AsOfDate
                                  LADate("20200503"),       // 1Y
                                  LADate("20210503"),       // 2Y
                                  LADate("20220503"),       // 3Y
                                  LADate("20230503"),       // 4Y
                                  LADate("20240503"),       // 5Y
                                  LADate("20250503"),       // 6Y
                                  LADate("20260503"),       // 7Y
                                  LADate("20270503"),       // 8Y
                                  LADate("20280503"),       // 9Y
                                  LADate("20290503") };     // 10Y


        // 3.   Get Discount Factors from Curve Results
        // ================================================
        const DoubleVector discountFactorsFromCurveResults = etrading::getDiscountFactorsFromCurveResultsObject( curveCollection, curveIndex, paymentDates );
        
        // 4.   Get Discount Factors from Object Pool
        // ================================================
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        etrading::populateDiscountFactorConventions( curveCollection.c_str(), curveIndex.c_str(), interp, bdAdj, cal );
        
        const DoubleVector discountFactorsFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates,
                                                                                                                             etrading::getDataInstance(),
                                                                                                                             curveCollection.c_str(),
                                                                                                                             etrading::getDiscountFactorDayCount(),
                                                                                                                             bdAdj,
                                                                                                                             cal,
                                                                                                                             interp,
                                                                                                                             etrading::isBasisFlagForDiscountFactor(),
                                                                                                                             curveIndex.c_str() );
        // 5.   Compare Discount Factors from 3. and 4. above
        // ================================================
        ASSERT_EQ( discountFactorsFromCurveResults.size(), discountFactorsFromEntityPool.size() );
        for( size_t i = 0; i < discountFactorsFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( discountFactorsFromCurveResults[i], discountFactorsFromEntityPool[i], discountFactorTolerance  );
        }
    }

    // Test that Forward Rates Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareForwardRates_EURYC_OIS )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        const std::string curveCollection                       = "EURYC";
        const std::string curveIndex                            = "OIS";
        const bool interpolateOnForwards                        = false;
        etrading::CompoundingFrequencyEnum simpleCompounding    = etrading::SIMPLE_COMPOUNDING;
    
        DateVector fixingStartDates { LADate("20190503"),       // Curve AsOfDate
                                      LADate("20200503"),       // 1Y
                                      LADate("20210503"),       // 2Y
                                      LADate("20220503"),       // 3Y
                                      LADate("20230503"),       // 4Y
                                      LADate("20240503"),       // 5Y
                                      LADate("20250503"),       // 6Y
                                      LADate("20260503"),       // 7Y
                                      LADate("20270503"),       // 8Y
                                      LADate("20280503"),       // 9Y
                                      LADate("20290503") };     // 10Y
    
        DateVector fixingEndDates { LADate("20190506"),       // Curve AsOfDate + 1D (weekend)
                                    LADate("20200506"),       // 1Y + 1D (weekend)
                                    LADate("20210506"),       // 2Y + 1D (weekend)
                                    LADate("20220506"),       // 3Y + 1D (weekend)
                                    LADate("20230506"),       // 4Y + 1D (weekend)
                                    LADate("20240506"),       // 5Y + 1D (weekend)
                                    LADate("20250506"),       // 6Y + 1D (weekend)
                                    LADate("20260506"),       // 7Y + 1D (weekend)
                                    LADate("20270506"),       // 8Y + 1D (weekend)
                                    LADate("20280506"),       // 9Y + 1D (weekend)
                                    LADate("20290506") };     // 10Y + 1D (weekend)
    
    
        // 3.   Get Forward Rates from Curve Results
        // ================================================
        const DoubleVector forwardRatesFromCurveResults = etrading::implyForwardRatesFromCurveResultsObject( curveCollection, curveIndex, fixingStartDates, fixingEndDates, false, simpleCompounding );
        
        // 4.   Get ForwardRates from Object Pool
        // ================================================
        LAString dayC   = "";
		LAString bdAdj = "";
		LAString cal = "";
        LAString interp;
        bool isFwdInter;
        bool useFwdData;
    
        etrading::populateForwardRateConventions( curveCollection.c_str(), curveIndex.c_str(), etrading::toBooleanEnumFromBool( interpolateOnForwards ), interp, bdAdj, cal, dayC, isFwdInter, useFwdData );
        
        const DoubleVector forwardRatesFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiForwardRate( fixingStartDates, fixingEndDates, etrading::getDataInstance(), curveCollection.c_str(), toString(simpleCompounding).c_str(), dayC, bdAdj, cal, interp, curveIndex.c_str(), isFwdInter, useFwdData );
        
        // 5.   Compare Forward Rates from 3. and 4. above
        // ================================================
        ASSERT_EQ( forwardRatesFromCurveResults.size(), forwardRatesFromEntityPool.size() );
        for( size_t i = 0; i < forwardRatesFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( forwardRatesFromCurveResults[i], forwardRatesFromEntityPool[i], forwardRateTolerance  );
        }
    }
    
    
    // Test that Forward Rates Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareForwardRates_EURYC_EUR1ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        const std::string curveCollection                       = "EURYC";
        const std::string curveIndex                            = "EUR1ML";
        const bool interpolateOnForwards                        = false;
        etrading::CompoundingFrequencyEnum simpleCompounding    = etrading::SIMPLE_COMPOUNDING;
    
        DateVector fixingStartDates { LADate("20190503"),       // Curve AsOfDate
                                      LADate("20200503"),       // 1Y
                                      LADate("20210503"),       // 2Y
                                      LADate("20220503"),       // 3Y
                                      LADate("20230503"),       // 4Y
                                      LADate("20240503"),       // 5Y
                                      LADate("20250503"),       // 6Y
                                      LADate("20260503"),       // 7Y
                                      LADate("20270503"),       // 8Y
                                      LADate("20280503"),       // 9Y
                                      LADate("20290503") };     // 10Y
    
        DateVector fixingEndDates { LADate("20190603"),       // Curve AsOfDate + 1M
                                    LADate("20200603"),       // 1Y + 1M
                                    LADate("20210603"),       // 2Y + 1M
                                    LADate("20220603"),       // 3Y + 1M
                                    LADate("20230603"),       // 4Y + 1M
                                    LADate("20240603"),       // 5Y + 1M
                                    LADate("20250603"),       // 6Y + 1M
                                    LADate("20260603"),       // 7Y + 1M
                                    LADate("20270603"),       // 8Y + 1M
                                    LADate("20280603"),       // 9Y + 1M
                                    LADate("20290603") };     // 10Y + 1M
    
    
        // 3.   Get Forward Rates from Curve Results
        // ================================================
        const DoubleVector forwardRatesFromCurveResults = etrading::implyForwardRatesFromCurveResultsObject( curveCollection, curveIndex, fixingStartDates, fixingEndDates, false, simpleCompounding );
        
        // 4.   Get ForwardRates from Object Pool
        // ================================================
        LAString dayC   = "";
		LAString bdAdj = "";
		LAString cal = "";
        LAString interp;
        bool isFwdInter;
        bool useFwdData;
    
        etrading::populateForwardRateConventions( curveCollection.c_str(), curveIndex.c_str(), etrading::toBooleanEnumFromBool( interpolateOnForwards ), interp, bdAdj, cal, dayC, isFwdInter, useFwdData );
        
        const DoubleVector forwardRatesFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiForwardRate( fixingStartDates, fixingEndDates, etrading::getDataInstance(), curveCollection.c_str(), toString(simpleCompounding).c_str(), dayC, bdAdj, cal, interp, curveIndex.c_str(), isFwdInter, useFwdData );
        
        // 5.   Compare Forward Rates from 3. and 4. above
        // ================================================
        ASSERT_EQ( forwardRatesFromCurveResults.size(), forwardRatesFromEntityPool.size() );
        for( size_t i = 0; i < forwardRatesFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( forwardRatesFromCurveResults[i], forwardRatesFromEntityPool[i], forwardRateTolerance  );
        }
    }
    
    
    // Test that Forward Rates Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareForwardRates_EURYC_EUR3ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        const std::string curveCollection                       = "EURYC";
        const std::string curveIndex                            = "EUR3ML";
        const bool interpolateOnForwards                        = false;
        etrading::CompoundingFrequencyEnum simpleCompounding    = etrading::SIMPLE_COMPOUNDING;
    
        DateVector fixingStartDates { LADate("20190503"),       // Curve AsOfDate
                                      LADate("20200503"),       // 1Y
                                      LADate("20210503"),       // 2Y
                                      LADate("20220503"),       // 3Y
                                      LADate("20230503"),       // 4Y
                                      LADate("20240503"),       // 5Y
                                      LADate("20250503"),       // 6Y
                                      LADate("20260503"),       // 7Y
                                      LADate("20270503"),       // 8Y
                                      LADate("20280503"),       // 9Y
                                      LADate("20290503") };     // 10Y
    
        DateVector fixingEndDates { LADate("20190803"),       // Curve AsOfDate + 3M
                                    LADate("20200803"),       // 1Y + 3M
                                    LADate("20210803"),       // 2Y + 3M
                                    LADate("20220803"),       // 3Y + 3M
                                    LADate("20230803"),       // 4Y + 3M
                                    LADate("20240803"),       // 5Y + 3M
                                    LADate("20250803"),       // 6Y + 3M
                                    LADate("20260803"),       // 7Y + 3M
                                    LADate("20270803"),       // 8Y + 3M
                                    LADate("20280803"),       // 9Y + 3M
                                    LADate("20290803") };     // 10Y + 3M
    
    
        // 3.   Get Forward Rates from Curve Results
        // ================================================
        const DoubleVector forwardRatesFromCurveResults = etrading::implyForwardRatesFromCurveResultsObject( curveCollection, curveIndex, fixingStartDates, fixingEndDates, false, simpleCompounding );
        
        // 4.   Get ForwardRates from Object Pool
        // ================================================
        LAString dayC   = "";
		LAString bdAdj = "";
		LAString cal = "";
        LAString interp;
        bool isFwdInter;
        bool useFwdData;
    
        etrading::populateForwardRateConventions( curveCollection.c_str(), curveIndex.c_str(), etrading::toBooleanEnumFromBool( interpolateOnForwards ), interp, bdAdj, cal, dayC, isFwdInter, useFwdData );
        
        const DoubleVector forwardRatesFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiForwardRate( fixingStartDates, fixingEndDates, etrading::getDataInstance(), curveCollection.c_str(), toString(simpleCompounding).c_str(), dayC, bdAdj, cal, interp, curveIndex.c_str(), isFwdInter, useFwdData );
        
        // 5.   Compare Forward Rates from 3. and 4. above
        // ================================================
        ASSERT_EQ( forwardRatesFromCurveResults.size(), forwardRatesFromEntityPool.size() );
        for( size_t i = 0; i < forwardRatesFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( forwardRatesFromCurveResults[i], forwardRatesFromEntityPool[i], forwardRateTolerance  );
        }
    }
    
    
    // Test that Forward Rates Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareForwardRates_EURYC_EUR6ML_STD_SWAPCURVE )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        const std::string curveCollection                       = "EURYC";
        const std::string curveIndex                            = "EUR6ML";
        const bool interpolateOnForwards                        = false;
        etrading::CompoundingFrequencyEnum simpleCompounding    = etrading::SIMPLE_COMPOUNDING;
    
        DateVector fixingStartDates { LADate("20190503"),       // Curve AsOfDate
                                      LADate("20200503"),       // 1Y
                                      LADate("20210503"),       // 2Y
                                      LADate("20220503"),       // 3Y
                                      LADate("20230503"),       // 4Y
                                      LADate("20240503"),       // 5Y
                                      LADate("20250503"),       // 6Y
                                      LADate("20260503"),       // 7Y
                                      LADate("20270503"),       // 8Y
                                      LADate("20280503"),       // 9Y
                                      LADate("20290503") };     // 10Y
    
        DateVector fixingEndDates { LADate("20191103"),         // Curve AsOfDate + 6M
                                    LADate("20201103"),         // 1Y + 6M
                                    LADate("20211103"),         // 2Y + 6M
                                    LADate("20221103"),         // 3Y + 6M
                                    LADate("20231103"),         // 4Y + 6M
                                    LADate("20241103"),         // 5Y + 6M
                                    LADate("20251103"),         // 6Y + 6M
                                    LADate("20261103"),         // 7Y + 6M
                                    LADate("20271103"),         // 8Y + 6M
                                    LADate("20281103"),         // 9Y + 6M
                                    LADate("20291103") };       // 10Y + 6M
    
    
        // 3.   Get Forward Rates from Curve Results
        // ================================================
        const DoubleVector forwardRatesFromCurveResults = etrading::implyForwardRatesFromCurveResultsObject( curveCollection, curveIndex, fixingStartDates, fixingEndDates, false, simpleCompounding );
        
        // 4.   Get ForwardRates from Object Pool
        // ================================================
        LAString dayC   = "";
		LAString bdAdj = "";
		LAString cal = "";
        LAString interp;
        bool isFwdInter;
        bool useFwdData;
    
        etrading::populateForwardRateConventions( curveCollection.c_str(), curveIndex.c_str(), etrading::toBooleanEnumFromBool( interpolateOnForwards ), interp, bdAdj, cal, dayC, isFwdInter, useFwdData );
        
        const DoubleVector forwardRatesFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiForwardRate( fixingStartDates, fixingEndDates, etrading::getDataInstance(), curveCollection.c_str(), toString(simpleCompounding).c_str(), dayC, bdAdj, cal, interp, curveIndex.c_str(), isFwdInter, useFwdData );
        
        // 5.   Compare Forward Rates from 3. and 4. above
        // ================================================
        ASSERT_EQ( forwardRatesFromCurveResults.size(), forwardRatesFromEntityPool.size() );
        for( size_t i = 0; i < forwardRatesFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( forwardRatesFromCurveResults[i], forwardRatesFromEntityPool[i], forwardRateTolerance  );
        }
    }
    
    
    // Test that Forward Rates Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareForwardRates_EURYC_EUR12ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        const std::string curveCollection                       = "EURYC";
        const std::string curveIndex                            = "EUR12ML";
        const bool interpolateOnForwards                        = false;
        etrading::CompoundingFrequencyEnum simpleCompounding    = etrading::SIMPLE_COMPOUNDING;
    
        DateVector fixingStartDates { LADate("20190503"),       // Curve AsOfDate
                                      LADate("20200503"),       // 1Y
                                      LADate("20210503"),       // 2Y
                                      LADate("20220503"),       // 3Y
                                      LADate("20230503"),       // 4Y
                                      LADate("20240503"),       // 5Y
                                      LADate("20250503"),       // 6Y
                                      LADate("20260503"),       // 7Y
                                      LADate("20270503"),       // 8Y
                                      LADate("20280503"),       // 9Y
                                      LADate("20290503") };     // 10Y
    
        DateVector fixingEndDates { LADate("20200503"),         // Curve AsOfDate + 12M
                                    LADate("20210503"),         // 1Y + 12M
                                    LADate("20220503"),         // 2Y + 12M
                                    LADate("20230503"),         // 3Y + 12M
                                    LADate("20240503"),         // 4Y + 12M
                                    LADate("20250503"),         // 5Y + 12M
                                    LADate("20260503"),         // 6Y + 12M
                                    LADate("20270503"),         // 7Y + 12M
                                    LADate("20280503"),         // 8Y + 12M
                                    LADate("20290503"),         // 9Y + 12M
                                    LADate("20300503") };       // 10Y + 12M
    
    
        // 3.   Get Forward Rates from Curve Results
        // ================================================
        const DoubleVector forwardRatesFromCurveResults = etrading::implyForwardRatesFromCurveResultsObject( curveCollection, curveIndex, fixingStartDates, fixingEndDates, false, simpleCompounding );
        
        // 4.   Get ForwardRates from Object Pool
        // ================================================
        LAString dayC   = "";
		LAString bdAdj = "";
		LAString cal = "";
        LAString interp;
        bool isFwdInter;
        bool useFwdData;
    
        etrading::populateForwardRateConventions( curveCollection.c_str(), curveIndex.c_str(), etrading::toBooleanEnumFromBool( interpolateOnForwards ), interp, bdAdj, cal, dayC, isFwdInter, useFwdData );
        
        const DoubleVector forwardRatesFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiForwardRate( fixingStartDates, fixingEndDates, etrading::getDataInstance(), curveCollection.c_str(), toString(simpleCompounding).c_str(), dayC, bdAdj, cal, interp, curveIndex.c_str(), isFwdInter, useFwdData );
        
        // 5.   Compare Forward Rates from 3. and 4. above
        // ================================================
        ASSERT_EQ( forwardRatesFromCurveResults.size(), forwardRatesFromEntityPool.size() );
        for( size_t i = 0; i < forwardRatesFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( forwardRatesFromCurveResults[i], forwardRatesFromEntityPool[i], forwardRateTolerance  );
        }
    }

    // Test that Forward Rates Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareForwardRates_USDYC_OIS )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        const std::string curveCollection                       = "USDYC";
        const std::string curveIndex                            = "OIS";
        const bool interpolateOnForwards                        = false;
        etrading::CompoundingFrequencyEnum simpleCompounding    = etrading::SIMPLE_COMPOUNDING;
    
        DateVector fixingStartDates { LADate("20190503"),       // Curve AsOfDate
                                      LADate("20200503"),       // 1Y
                                      LADate("20210503"),       // 2Y
                                      LADate("20220503"),       // 3Y
                                      LADate("20230503"),       // 4Y
                                      LADate("20240503"),       // 5Y
                                      LADate("20250503"),       // 6Y
                                      LADate("20260503"),       // 7Y
                                      LADate("20270503"),       // 8Y
                                      LADate("20280503"),       // 9Y
                                      LADate("20290503") };     // 10Y
    
        DateVector fixingEndDates { LADate("20190506"),       // Curve AsOfDate + 1D (weekend)
                                    LADate("20200506"),       // 1Y + 1D (weekend)
                                    LADate("20210506"),       // 2Y + 1D (weekend)
                                    LADate("20220506"),       // 3Y + 1D (weekend)
                                    LADate("20230506"),       // 4Y + 1D (weekend)
                                    LADate("20240506"),       // 5Y + 1D (weekend)
                                    LADate("20250506"),       // 6Y + 1D (weekend)
                                    LADate("20260506"),       // 7Y + 1D (weekend)
                                    LADate("20270506"),       // 8Y + 1D (weekend)
                                    LADate("20280506"),       // 9Y + 1D (weekend)
                                    LADate("20290506") };     // 10Y + 1D (weekend)
    
    
        // 3.   Get Forward Rates from Curve Results
        // ================================================
        const DoubleVector forwardRatesFromCurveResults = etrading::implyForwardRatesFromCurveResultsObject( curveCollection, curveIndex, fixingStartDates, fixingEndDates, false, simpleCompounding );
        
        // 4.   Get ForwardRates from Object Pool
        // ================================================
        LAString dayC   = "";
		LAString bdAdj = "";
		LAString cal = "";
        LAString interp;
        bool isFwdInter;
        bool useFwdData;
    
        etrading::populateForwardRateConventions( curveCollection.c_str(), curveIndex.c_str(), etrading::toBooleanEnumFromBool( interpolateOnForwards ), interp, bdAdj, cal, dayC, isFwdInter, useFwdData );
        
        const DoubleVector forwardRatesFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiForwardRate( fixingStartDates, fixingEndDates, etrading::getDataInstance(), curveCollection.c_str(), toString(simpleCompounding).c_str(), dayC, bdAdj, cal, interp, curveIndex.c_str(), isFwdInter, useFwdData );
        
        // 5.   Compare Forward Rates from 3. and 4. above
        // ================================================
        ASSERT_EQ( forwardRatesFromCurveResults.size(), forwardRatesFromEntityPool.size() );
        for( size_t i = 0; i < forwardRatesFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( forwardRatesFromCurveResults[i], forwardRatesFromEntityPool[i], forwardRateTolerance  );
        }
    }
    
    
    // Test that Forward Rates Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareForwardRates_USDYC_USD1ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        const std::string curveCollection                       = "USDYC";
        const std::string curveIndex                            = "USD1ML";
        const bool interpolateOnForwards                        = false;
        etrading::CompoundingFrequencyEnum simpleCompounding    = etrading::SIMPLE_COMPOUNDING;
    
        DateVector fixingStartDates { LADate("20190503"),       // Curve AsOfDate
                                      LADate("20200503"),       // 1Y
                                      LADate("20210503"),       // 2Y
                                      LADate("20220503"),       // 3Y
                                      LADate("20230503"),       // 4Y
                                      LADate("20240503"),       // 5Y
                                      LADate("20250503"),       // 6Y
                                      LADate("20260503"),       // 7Y
                                      LADate("20270503"),       // 8Y
                                      LADate("20280503"),       // 9Y
                                      LADate("20290503") };     // 10Y
    
        DateVector fixingEndDates { LADate("20190603"),       // Curve AsOfDate + 1M
                                    LADate("20200603"),       // 1Y + 1M
                                    LADate("20210603"),       // 2Y + 1M
                                    LADate("20220603"),       // 3Y + 1M
                                    LADate("20230603"),       // 4Y + 1M
                                    LADate("20240603"),       // 5Y + 1M
                                    LADate("20250603"),       // 6Y + 1M
                                    LADate("20260603"),       // 7Y + 1M
                                    LADate("20270603"),       // 8Y + 1M
                                    LADate("20280603"),       // 9Y + 1M
                                    LADate("20290603") };     // 10Y + 1M
    
    
        // 3.   Get Forward Rates from Curve Results
        // ================================================
        const DoubleVector forwardRatesFromCurveResults = etrading::implyForwardRatesFromCurveResultsObject( curveCollection, curveIndex, fixingStartDates, fixingEndDates, false, simpleCompounding );
        
        // 4.   Get ForwardRates from Object Pool
        // ================================================
        LAString dayC   = "";
		LAString bdAdj = "";
		LAString cal = "";
        LAString interp;
        bool isFwdInter;
        bool useFwdData;
    
        etrading::populateForwardRateConventions( curveCollection.c_str(), curveIndex.c_str(), etrading::toBooleanEnumFromBool( interpolateOnForwards ), interp, bdAdj, cal, dayC, isFwdInter, useFwdData );
        
        const DoubleVector forwardRatesFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiForwardRate( fixingStartDates, fixingEndDates, etrading::getDataInstance(), curveCollection.c_str(), toString(simpleCompounding).c_str(), dayC, bdAdj, cal, interp, curveIndex.c_str(), isFwdInter, useFwdData );
        
        // 5.   Compare Forward Rates from 3. and 4. above
        // ================================================
        ASSERT_EQ( forwardRatesFromCurveResults.size(), forwardRatesFromEntityPool.size() );
        for( size_t i = 0; i < forwardRatesFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( forwardRatesFromCurveResults[i], forwardRatesFromEntityPool[i], forwardRateTolerance  );
        }
    }
    
    
    // Test that Forward Rates Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareForwardRates_USDYC_USD3ML_STD_SWAPCURVE )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        const std::string curveCollection                       = "USDYC";
        const std::string curveIndex                            = "USD3ML";
        const bool interpolateOnForwards                        = false;
        etrading::CompoundingFrequencyEnum simpleCompounding    = etrading::SIMPLE_COMPOUNDING;
    
        DateVector fixingStartDates { LADate("20190503"),       // Curve AsOfDate
                                      LADate("20200503"),       // 1Y
                                      LADate("20210503"),       // 2Y
                                      LADate("20220503"),       // 3Y
                                      LADate("20230503"),       // 4Y
                                      LADate("20240503"),       // 5Y
                                      LADate("20250503"),       // 6Y
                                      LADate("20260503"),       // 7Y
                                      LADate("20270503"),       // 8Y
                                      LADate("20280503"),       // 9Y
                                      LADate("20290503") };     // 10Y
    
        DateVector fixingEndDates { LADate("20190803"),       // Curve AsOfDate + 3M
                                    LADate("20200803"),       // 1Y + 3M
                                    LADate("20210803"),       // 2Y + 3M
                                    LADate("20220803"),       // 3Y + 3M
                                    LADate("20230803"),       // 4Y + 3M
                                    LADate("20240803"),       // 5Y + 3M
                                    LADate("20250803"),       // 6Y + 3M
                                    LADate("20260803"),       // 7Y + 3M
                                    LADate("20270803"),       // 8Y + 3M
                                    LADate("20280803"),       // 9Y + 3M
                                    LADate("20290803") };     // 10Y + 3M
    
    
        // 3.   Get Forward Rates from Curve Results
        // ================================================
        const DoubleVector forwardRatesFromCurveResults = etrading::implyForwardRatesFromCurveResultsObject( curveCollection, curveIndex, fixingStartDates, fixingEndDates, false, simpleCompounding );
        
        // 4.   Get ForwardRates from Object Pool
        // ================================================
        LAString dayC   = "";
		LAString bdAdj = "";
		LAString cal = "";
        LAString interp;
        bool isFwdInter;
        bool useFwdData;
    
        etrading::populateForwardRateConventions( curveCollection.c_str(), curveIndex.c_str(), etrading::toBooleanEnumFromBool( interpolateOnForwards ), interp, bdAdj, cal, dayC, isFwdInter, useFwdData );
        
        const DoubleVector forwardRatesFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiForwardRate( fixingStartDates, fixingEndDates, etrading::getDataInstance(), curveCollection.c_str(), toString(simpleCompounding).c_str(), dayC, bdAdj, cal, interp, curveIndex.c_str(), isFwdInter, useFwdData );
        
        // 5.   Compare Forward Rates from 3. and 4. above
        // ================================================
        ASSERT_EQ( forwardRatesFromCurveResults.size(), forwardRatesFromEntityPool.size() );
        for( size_t i = 0; i < forwardRatesFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( forwardRatesFromCurveResults[i], forwardRatesFromEntityPool[i], forwardRateTolerance  );
        }
    }
    
    
    // Test that Forward Rates Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareForwardRates_USDYC_USD6ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        const std::string curveCollection                       = "USDYC";
        const std::string curveIndex                            = "USD6ML";
        const bool interpolateOnForwards                        = false;
        etrading::CompoundingFrequencyEnum simpleCompounding    = etrading::SIMPLE_COMPOUNDING;
    
        DateVector fixingStartDates { LADate("20190503"),       // Curve AsOfDate
                                      LADate("20200503"),       // 1Y
                                      LADate("20210503"),       // 2Y
                                      LADate("20220503"),       // 3Y
                                      LADate("20230503"),       // 4Y
                                      LADate("20240503"),       // 5Y
                                      LADate("20250503"),       // 6Y
                                      LADate("20260503"),       // 7Y
                                      LADate("20270503"),       // 8Y
                                      LADate("20280503"),       // 9Y
                                      LADate("20290503") };     // 10Y
    
        DateVector fixingEndDates { LADate("20191103"),         // Curve AsOfDate + 6M
                                    LADate("20201103"),         // 1Y + 6M
                                    LADate("20211103"),         // 2Y + 6M
                                    LADate("20221103"),         // 3Y + 6M
                                    LADate("20231103"),         // 4Y + 6M
                                    LADate("20241103"),         // 5Y + 6M
                                    LADate("20251103"),         // 6Y + 6M
                                    LADate("20261103"),         // 7Y + 6M
                                    LADate("20271103"),         // 8Y + 6M
                                    LADate("20281103"),         // 9Y + 6M
                                    LADate("20291103") };       // 10Y + 6M
    
    
        // 3.   Get Forward Rates from Curve Results
        // ================================================
        const DoubleVector forwardRatesFromCurveResults = etrading::implyForwardRatesFromCurveResultsObject( curveCollection, curveIndex, fixingStartDates, fixingEndDates, false, simpleCompounding );
        
        // 4.   Get ForwardRates from Object Pool
        // ================================================
        LAString dayC   = "";
		LAString bdAdj = "";
		LAString cal = "";
        LAString interp;
        bool isFwdInter;
        bool useFwdData;
    
        etrading::populateForwardRateConventions( curveCollection.c_str(), curveIndex.c_str(), etrading::toBooleanEnumFromBool( interpolateOnForwards ), interp, bdAdj, cal, dayC, isFwdInter, useFwdData );
        
        const DoubleVector forwardRatesFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiForwardRate( fixingStartDates, fixingEndDates, etrading::getDataInstance(), curveCollection.c_str(), toString(simpleCompounding).c_str(), dayC, bdAdj, cal, interp, curveIndex.c_str(), isFwdInter, useFwdData );
        
        // 5.   Compare Forward Rates from 3. and 4. above
        // ================================================
        ASSERT_EQ( forwardRatesFromCurveResults.size(), forwardRatesFromEntityPool.size() );
        for( size_t i = 0; i < forwardRatesFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( forwardRatesFromCurveResults[i], forwardRatesFromEntityPool[i], forwardRateTolerance  );
        }
    }
    
    
    // Test that Forward Rates Generated by the Object Pool are Consistent with those Captured in the Curve Results Object
    TEST_F( TestCurveResultsVsObjectPool, CONSISTENCY_CompareForwardRates_USDYC_USD12ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above
    
        // 2. Curve Parameters
        // ================================================
        const std::string curveCollection                       = "USDYC";
        const std::string curveIndex                            = "USD12ML";
        const bool interpolateOnForwards                        = false;
        etrading::CompoundingFrequencyEnum simpleCompounding    = etrading::SIMPLE_COMPOUNDING;
    
        DateVector fixingStartDates { LADate("20190503"),       // Curve AsOfDate
                                      LADate("20200503"),       // 1Y
                                      LADate("20210503"),       // 2Y
                                      LADate("20220503"),       // 3Y
                                      LADate("20230503"),       // 4Y
                                      LADate("20240503"),       // 5Y
                                      LADate("20250503"),       // 6Y
                                      LADate("20260503"),       // 7Y
                                      LADate("20270503"),       // 8Y
                                      LADate("20280503"),       // 9Y
                                      LADate("20290503") };     // 10Y
    
        DateVector fixingEndDates { LADate("20200503"),         // Curve AsOfDate + 12M
                                    LADate("20210503"),         // 1Y + 12M
                                    LADate("20220503"),         // 2Y + 12M
                                    LADate("20230503"),         // 3Y + 12M
                                    LADate("20240503"),         // 4Y + 12M
                                    LADate("20250503"),         // 5Y + 12M
                                    LADate("20260503"),         // 6Y + 12M
                                    LADate("20270503"),         // 7Y + 12M
                                    LADate("20280503"),         // 8Y + 12M
                                    LADate("20290503"),         // 9Y + 12M
                                    LADate("20300503") };       // 10Y + 12M
    
    
        // 3.   Get Forward Rates from Curve Results
        // ================================================
        const DoubleVector forwardRatesFromCurveResults = etrading::implyForwardRatesFromCurveResultsObject( curveCollection, curveIndex, fixingStartDates, fixingEndDates, false, simpleCompounding );
        
        // 4.   Get ForwardRates from Object Pool
        // ================================================
        LAString dayC   = "";
		LAString bdAdj = "";
		LAString cal = "";
        LAString interp;
        bool isFwdInter;
        bool useFwdData;
    
        etrading::populateForwardRateConventions( curveCollection.c_str(), curveIndex.c_str(), etrading::toBooleanEnumFromBool( interpolateOnForwards ), interp, bdAdj, cal, dayC, isFwdInter, useFwdData );
        
        const DoubleVector forwardRatesFromEntityPool = etrading::LACurveForwardRateHelpers::getMultiForwardRate( fixingStartDates, fixingEndDates, etrading::getDataInstance(), curveCollection.c_str(), toString(simpleCompounding).c_str(), dayC, bdAdj, cal, interp, curveIndex.c_str(), isFwdInter, useFwdData );
        
        // 5.   Compare Forward Rates from 3. and 4. above
        // ================================================
        ASSERT_EQ( forwardRatesFromCurveResults.size(), forwardRatesFromEntityPool.size() );
        for( size_t i = 0; i < forwardRatesFromCurveResults.size(); ++i )
        {
            EXPECT_NEAR( forwardRatesFromCurveResults[i], forwardRatesFromEntityPool[i], forwardRateTolerance  );
        }
    }

    
    // TEST FIXTURE CONSTRUCTOR
    // ------------------------------------------------------------------------------------------------------------------

    // Declare Test Fixture Class
    class TestCurveResultsDaycount : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
    public:

        virtual void SetUp()
        {
            loadUSDCurves();
            loadEURCurves();
        }

        virtual void TearDown() {}

        virtual void loadUSDCurves()
        {
            // Load USD Curves
            const std::string curveObjectUSDOIS_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
            const std::string curveObjectUSD3ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
            const std::string curveObjectUSD6ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );
            const std::string curveObjectUSD1ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD1ML, etrading::JSON );
            const std::string curveObjectUSD12ML_   = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD12ML, etrading::JSON );
        }
        
        virtual void loadEURCurves()
        {
            // Load EUR Curves
            const std::string curveObjectEUROIS_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUROIS, etrading::JSON );
            const std::string curveObjectEUR3ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR3ML, etrading::JSON );
            const std::string curveObjectEUR6ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR6ML, etrading::JSON );
            const std::string curveObjectEUR1ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR1ML, etrading::JSON );
            const std::string curveObjectEUR12ML_   = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR12ML, etrading::JSON );
        }
    };


    // ------------------------------------------------------------------------------------------------------------------
    

    // Test that the daycount in the curve results object is correct
    TEST_F( TestCurveResultsDaycount, UNIT_Daycount_EURYC_OIS )
    {
        // This test indirectly tests the private getDayCountFromCurveData method within the curve engine. The daycount from this method
        // is stored in the curve results -> discount factors object so we can test the daycount value from here.

        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "OIS";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::DayCountEnum daycountFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->daycount();
        
        // 4. Curve Daycount
        etrading::DayCountEnum expectedOISSwapFloatDaycount = etrading::ACT_360_DAYCOUNT;
        EXPECT_EQ( expectedOISSwapFloatDaycount, daycountFromCurveResultsObject );
    }


    // Test that the daycount in the curve results object is correct
    TEST_F( TestCurveResultsDaycount, UNIT_Daycount_EURYC_EUR1ML )
    {
        // This test indirectly tests the private getDayCountFromCurveData method within the curve engine. The daycount from this method
        // is stored in the curve results -> discount factors object so we can test the daycount value from here.

        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR1ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::DayCountEnum daycountFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->daycount();
        
        // 4. Curve Daycount
        etrading::DayCountEnum expectedTenorBasisDaycount = etrading::ACT_360_DAYCOUNT;
        EXPECT_EQ( expectedTenorBasisDaycount, daycountFromCurveResultsObject );
    }


    // Test that the daycount in the curve results object is correct
    TEST_F( TestCurveResultsDaycount, UNIT_Daycount_EURYC_EUR3ML )
    {
        // This test indirectly tests the private getDayCountFromCurveData method within the curve engine. The daycount from this method
        // is stored in the curve results -> discount factors object so we can test the daycount value from here.

        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR3ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::DayCountEnum daycountFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->daycount();
        
        // 4. Curve Daycount
        etrading::DayCountEnum expectedTenorBasisDaycount = etrading::ACT_360_DAYCOUNT;
        EXPECT_EQ( expectedTenorBasisDaycount, daycountFromCurveResultsObject );
    }


    // Test that the daycount in the curve results object is correct
    TEST_F( TestCurveResultsDaycount, UNIT_Daycount_EURYC_EUR6ML_STD_SWAPCURVE )
    {
        // This test indirectly tests the private getDayCountFromCurveData method within the curve engine. The daycount from this method
        // is stored in the curve results -> discount factors object so we can test the daycount value from here.

        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR6ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::DayCountEnum daycountFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->daycount();
        
        // 4. Curve Daycount
        etrading::DayCountEnum expectedSwapFloatDaycount = etrading::ACT_360_DAYCOUNT;
        EXPECT_EQ( expectedSwapFloatDaycount, daycountFromCurveResultsObject );
    }


    // Test that the daycount in the curve results object is correct
    TEST_F( TestCurveResultsDaycount, UNIT_Daycount_EURYC_EUR12ML )
    {
        // This test indirectly tests the private getDayCountFromCurveData method within the curve engine. The daycount from this method
        // is stored in the curve results -> discount factors object so we can test the daycount value from here.

        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR12ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::DayCountEnum daycountFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->daycount();
        
        // 4. Curve Daycount
        etrading::DayCountEnum expectedTenorBasisDaycount = etrading::ACT_360_DAYCOUNT;
        EXPECT_EQ( expectedTenorBasisDaycount, daycountFromCurveResultsObject );
    }


    // Test that the daycount in the curve results object is correct
    TEST_F( TestCurveResultsDaycount, UNIT_Daycount_USDYC_OIS )
    {
        // This test indirectly tests the private getDayCountFromCurveData method within the curve engine. The daycount from this method
        // is stored in the curve results -> discount factors object so we can test the daycount value from here.

        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "OIS";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::DayCountEnum daycountFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->daycount();
        
        // 4. Curve Daycount
        etrading::DayCountEnum expectedOISSwapFloatDaycount = etrading::ACT_360_DAYCOUNT;
        EXPECT_EQ( expectedOISSwapFloatDaycount, daycountFromCurveResultsObject );
    }


    // Test that the daycount in the curve results object is correct
    TEST_F( TestCurveResultsDaycount, UNIT_Daycount_USDYC_USD1ML )
    {
        // This test indirectly tests the private getDayCountFromCurveData method within the curve engine. The daycount from this method
        // is stored in the curve results -> discount factors object so we can test the daycount value from here.

        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD1ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::DayCountEnum daycountFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->daycount();
        
        // 4. Curve Daycount
        etrading::DayCountEnum expectedTenorBasisDaycount = etrading::ACT_360_DAYCOUNT;
        EXPECT_EQ( expectedTenorBasisDaycount, daycountFromCurveResultsObject );
    }


    // Test that the daycount in the curve results object is correct
    TEST_F( TestCurveResultsDaycount, UNIT_Daycount_USDYC_USD3ML_STD_SWAPCURVE )
    {
        // This test indirectly tests the private getDayCountFromCurveData method within the curve engine. The daycount from this method
        // is stored in the curve results -> discount factors object so we can test the daycount value from here.

        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD3ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::DayCountEnum daycountFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->daycount();
        
        // 4. Curve Daycount
        etrading::DayCountEnum expectedSwapFloatDaycount = etrading::ACT_360_DAYCOUNT;
        EXPECT_EQ( expectedSwapFloatDaycount, daycountFromCurveResultsObject );
    }


    // Test that the daycount in the curve results object is correct
    TEST_F( TestCurveResultsDaycount, UNIT_Daycount_USDYC_USD6ML )
    {
        // This test indirectly tests the private getDayCountFromCurveData method within the curve engine. The daycount from this method
        // is stored in the curve results -> discount factors object so we can test the daycount value from here.

        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD6ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::DayCountEnum daycountFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->daycount();
        
        // 4. Curve Daycount
        etrading::DayCountEnum expectedTenorBasisDaycount = etrading::ACT_360_DAYCOUNT;
        EXPECT_EQ( expectedTenorBasisDaycount, daycountFromCurveResultsObject );
    }


    // Test that the daycount in the curve results object is correct
    TEST_F( TestCurveResultsDaycount, UNIT_Daycount_USDYC_USD12ML )
    {
        // This test indirectly tests the private getDayCountFromCurveData method within the curve engine. The daycount from this method
        // is stored in the curve results -> discount factors object so we can test the daycount value from here.

        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD12ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::DayCountEnum daycountFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->daycount();
        
        // 4. Curve Daycount
        etrading::DayCountEnum expectedTenorBasisDaycount = etrading::ACT_360_DAYCOUNT;
        EXPECT_EQ( expectedTenorBasisDaycount, daycountFromCurveResultsObject );
    }


    // TEST FIXTURE CONSTRUCTOR
    // ------------------------------------------------------------------------------------------------------------------

    // Declare Test Fixture Class
    class TestCurveResultsCurveTenor : public virtual testing::Test, public virtual google_test::InitializeMLibGoogleTest
    {
    public:

        virtual void SetUp()
        {
            loadUSDCurves();
            loadEURCurves();
        }

        virtual void TearDown() {}

        virtual void loadUSDCurves()
        {
            // Load USD Curves
            const std::string curveObjectUSDOIS_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
            const std::string curveObjectUSD3ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
            const std::string curveObjectUSD6ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );
            const std::string curveObjectUSD1ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD1ML, etrading::JSON );
            const std::string curveObjectUSD12ML_   = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD12ML, etrading::JSON );
        }
        
        virtual void loadEURCurves()
        {
            // Load EUR Curves
            const std::string curveObjectEUROIS_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUROIS, etrading::JSON );
            const std::string curveObjectEUR3ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR3ML, etrading::JSON );
            const std::string curveObjectEUR6ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR6ML, etrading::JSON );
            const std::string curveObjectEUR1ML_    = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR1ML, etrading::JSON );
            const std::string curveObjectEUR12ML_   = validation_api::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameEUR12ML, etrading::JSON );
        }
    };


    // ------------------------------------------------------------------------------------------------------------------
    

    // Test that Curve Tenor stored in the curve results object is correct
    TEST_F( TestCurveResultsCurveTenor, UNIT_CurveTenor_EURYC_OIS )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "OIS";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::CurveTenorEnum curveTenorFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->curveTenorEnum();
        
        // 4. Curve Daycount
        etrading::CurveTenorEnum expectedCurveTenor = etrading::CURVE_TENOR_1D;
        EXPECT_EQ( expectedCurveTenor, curveTenorFromCurveResultsObject );
    }


    // Test that Curve Tenor stored in the curve results object is correct
    TEST_F( TestCurveResultsCurveTenor, UNIT_CurveTenor_EURYC_EUR1ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR1ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::CurveTenorEnum curveTenorFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->curveTenorEnum();
        
        // 4. Curve Daycount
        etrading::CurveTenorEnum expectedCurveTenor = etrading::CURVE_TENOR_1M;
        EXPECT_EQ( expectedCurveTenor, curveTenorFromCurveResultsObject );
    }


    // Test that Curve Tenor stored in the curve results object is correct
    TEST_F( TestCurveResultsCurveTenor, UNIT_CurveTenor_EURYC_EUR3ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR3ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::CurveTenorEnum curveTenorFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->curveTenorEnum();
        
        // 4. Curve Daycount
        etrading::CurveTenorEnum expectedCurveTenor = etrading::CURVE_TENOR_3M;
        EXPECT_EQ( expectedCurveTenor, curveTenorFromCurveResultsObject );
    }


    // Test that Curve Tenor stored in the curve results object is correct
    TEST_F( TestCurveResultsCurveTenor, UNIT_CurveTenor_EURYC_EUR6ML_STD_SWAPCURVE )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR6ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::CurveTenorEnum curveTenorFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->curveTenorEnum();
        
        // 4. Curve Daycount
        etrading::CurveTenorEnum expectedCurveTenor = etrading::CURVE_TENOR_6M;
        EXPECT_EQ( expectedCurveTenor, curveTenorFromCurveResultsObject );
    }


    // Test that Curve Tenor stored in the curve results object is correct
    TEST_F( TestCurveResultsCurveTenor, UNIT_CurveTenor_EURYC_EUR12ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "EURYC";
        std::string curveIndex          = "EUR12ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::CurveTenorEnum curveTenorFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->curveTenorEnum();
        
        // 4. Curve Daycount
        etrading::CurveTenorEnum expectedCurveTenor = etrading::CURVE_TENOR_12M;
        EXPECT_EQ( expectedCurveTenor, curveTenorFromCurveResultsObject );
    }


    // Test that Curve Tenor stored in the curve results object is correct
    TEST_F( TestCurveResultsCurveTenor, UNIT_CurveTenor_USDYC_OIS )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "OIS";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::CurveTenorEnum curveTenorFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->curveTenorEnum();
        
        // 4. Curve Daycount
        etrading::CurveTenorEnum expectedCurveTenor = etrading::CURVE_TENOR_1D;
        EXPECT_EQ( expectedCurveTenor, curveTenorFromCurveResultsObject );
    }


    // Test that Curve Tenor stored in the curve results object is correct
    TEST_F( TestCurveResultsCurveTenor, UNIT_CurveTenor_USDYC_USD1ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD1ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::CurveTenorEnum curveTenorFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->curveTenorEnum();
        
        // 4. Curve Daycount
        etrading::CurveTenorEnum expectedCurveTenor = etrading::CURVE_TENOR_1M;
        EXPECT_EQ( expectedCurveTenor, curveTenorFromCurveResultsObject );
    }


    // Test that Curve Tenor stored in the curve results object is correct
    TEST_F( TestCurveResultsCurveTenor, UNIT_CurveTenor_USDYC_USD3ML_STD_SWAPCURVE )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD3ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::CurveTenorEnum curveTenorFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->curveTenorEnum();
        
        // 4. Curve Daycount
        etrading::CurveTenorEnum expectedCurveTenor = etrading::CURVE_TENOR_3M;
        EXPECT_EQ( expectedCurveTenor, curveTenorFromCurveResultsObject );
    }


    // Test that Curve Tenor stored in the curve results object is correct
    TEST_F( TestCurveResultsCurveTenor, UNIT_CurveTenor_USDYC_USD6ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD6ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::CurveTenorEnum curveTenorFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->curveTenorEnum();
        
        // 4. Curve Daycount
        etrading::CurveTenorEnum expectedCurveTenor = etrading::CURVE_TENOR_6M;
        EXPECT_EQ( expectedCurveTenor, curveTenorFromCurveResultsObject );
    }


    // Test that Curve Tenor stored in the curve results object is correct
    TEST_F( TestCurveResultsCurveTenor, UNIT_CurveTenor_USDYC_USD12ML )
    {
        // 1. Load Curves and Create Curve Results Objects
        // ================================================
        // This is done within the test fixture constructor above

        // 2. Curve Parameters
        // ================================================
        std::string curveCollection     = "USDYC";
        std::string curveIndex          = "USD12ML";

        // 3. Get Discount Factors from Curve Results via the curve results container
        // ================================================
        etrading::CurveTenorEnum curveTenorFromCurveResultsObject
            = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->curveTenorEnum();
        
        // 4. Curve Daycount
        etrading::CurveTenorEnum expectedCurveTenor = etrading::CURVE_TENOR_12M;
        EXPECT_EQ( expectedCurveTenor, curveTenorFromCurveResultsObject );
    }
}