// Curves
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"

// Swap Creation and Pricing
#include "tryAqSwapObjectCreation.h"
#include "tryAqSwapObjectPricing.h"
#include "tryAqRateFixingTable.h"
#include "tryAqSwapObjectLeg.h"

// Test Infrastructure
#include "Dependency.h"   // IMPORTANT: Curve Macros are Here !!!
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include <gTest/gTest.h>

#include "AQObjUtilities.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

// Define the Test Input Folder Here
#define TEST_DIR "ETrading/AQObjects/TestAQObjSwapFloatRate/"

namespace
{

    // test tolerances and number of tests
    // -----------------------------------
    const double tolerance = 1e-8;

    //
    // curve input files
    //
    extern const char EURYC_OIS[]			    = TEST_DIR "EURYC_OIS_tryAqCurveCalibrateOIS_inputs";
    extern const char EURYC_1M[]			    = "";
    extern const char EURYC_3M[]			    = "";
    extern const char EURYC_6M[]			    = TEST_DIR "EURYC_STD_tryAqCurveCalibrateSwap_inputs";
    extern const char EURYC_12M[]			    = "";



    //
    // test call input and reference files
    //
    extern const char fixingTableInputs[]	= TEST_DIR "EUR6M_FIXINGS@191_tryAqRateFixingTableCreate_inputs";
    extern const char swapInputs1[]	= TEST_DIR "SWAP_1@189_tryAqSwapObjectCreate_inputs";
    extern const char swapInputs2[]	= TEST_DIR "SWAP_2@7_tryAqSwapObjectCreate_inputs";
    extern const char swapInputs3[]	= TEST_DIR "SWAP_3@7_tryAqSwapObjectCreate_inputs";
    extern const char swapInputs4[]	= TEST_DIR "SWAP_4@7_tryAqSwapObjectCreate_inputs";
    extern const char swapInputs5[]	= TEST_DIR "SWAP_5@7_tryAqSwapObjectCreate_inputs";
    extern const char swapInputs6[]	= TEST_DIR "SWAP_6@7_tryAqSwapObjectCreate_inputs";
    extern const char swapInputs7[]	= TEST_DIR "SWAP_7@7_tryAqSwapObjectCreate_inputs";

	extern const char TestOutputs_6_32bit[] = TEST_DIR "TestOutput6_32bit";
	extern const char TestOutputs_6_64bit[] = TEST_DIR "TestOutput6_64bit";
}

namespace google_test
{

    //
    // Build the Yield Curve by calling the Curve Constructor Classes in the correct order, respecting curve dependencies.
    //
    //
    AQL_BUILD_EUR_CURVE( TestAQObjSwapFloatRatesFixingsAndStubs, EURYC_OIS, EURYC_1M, EURYC_3M, EURYC_6M, EURYC_12M);
    
    //
    // Call Test Fixture
    //

    std::shared_ptr<etrading::Swap> getSwapFromCacheWithCurveDataPopulated(const std::string& swapName, etrading::DataProvider& dataProvider)
    {
        auto cachedSwap = etrading::getSwap(swapName);

        //Need to call a pricing function so that the discountFactors & floatRates are populated from the curve

		validation::tryAqSwapObjectPV(swapName, etrading::fromStringToLVB("EURYC"), "", etrading::fromStringToLVB(dataProvider.getValuationSettings().getFixingTableName("", etrading::FLOAT_SCHEDULE_TYPE)));

		cachedSwap->getLeg(1)->initializeDataProvider( dataProvider);

        return cachedSwap;
    }

	etrading::DataProvider getDataProvider(const std::string& fixingTableName)
	{

		etrading::DataProvider dataProvider(etrading::ValuationSettings(etrading::fromStringToLVB("EURYC"), etrading::fromStringToLVB(fixingTableName), ""));

		return dataProvider;
	}


    //Testing: FloatRate is the same as the rate in fixing table, given a fixingDate
    TEST_F( TestAQObjSwapFloatRatesFixingsAndStubs, CONSISTENCY_SourceFloatRates_FromFixingTable )
    {
        try
        {

                // Load the Input Files
                const ReadDataFile::Load fixingTableInputFile( fixingTableInputs );

                const ReadDataFile::Load tradeInputFile( swapInputs1 );
        
                // Get the Fixing Table Inputs & Create the FixingTable
                auto tableName                  = fixingTableInputFile["tableName"];
                auto currency                   = fixingTableInputFile["currency"];
                auto curveTenor                 = fixingTableInputFile["curveTenor"];
                auto fixingDates                = fixingTableInputFile["fixingDates"];
                auto fixingValues               = fixingTableInputFile["fixingValues"];

                std::string fixingTableName          = validation::tryAqRateFixingTableCreate( tableName, currency, curveTenor, fixingDates, fixingValues );

                // Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );

				etrading::DataProvider dataProvider = getDataProvider(fixingTableName);
		
				auto swap = getSwapFromCacheWithCurveDataPopulated(swapTradeName, dataProvider);
				double actualFloatRate = dataProvider.getCashflowDataExcludingUpfront( 0 ).floatRateData.resetRate;

                auto floatLegFirstCashflow = swap->getLeg(1)->getSchedule()->getCashflow(0);
                AQLDate floatLegFirstCashflowFixingDate = floatLegFirstCashflow->getFixingDate();
                auto fixingTable = etrading::getFixingTable(tableName, false /* do not throw when missing*/);
                double expectedFixingRate = fixingTable->getFixingValue( etrading::toGregorianDateFromAQLDate(floatLegFirstCashflowFixingDate) );

                // Check the Test Results
                EXPECT_NEAR( expectedFixingRate, actualFloatRate, tolerance );
        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }
    

    //Testing: FirstFixingAndLastFixingFields have higher priority than the fixing table
    TEST_F( TestAQObjSwapFloatRatesFixingsAndStubs, CONSISTENCY_SourceFloatRates_FromFirstFixingAndLastFixing_Parameters )
    {
        try
        {
                // Load the Input Files
                const ReadDataFile::Load fixingTableInputFile( fixingTableInputs );

                const ReadDataFile::Load tradeInputFile( swapInputs2 );
        
                // Get the Fixing Table Inputs & Create the FixingTable
                auto tableName                  = fixingTableInputFile["tableName"];
                auto currency                   = fixingTableInputFile["currency"];
                auto curveTenor                 = fixingTableInputFile["curveTenor"];
                auto fixingDates                = fixingTableInputFile["fixingDates"];
                auto fixingValues               = fixingTableInputFile["fixingValues"];

                std::string fixingTableName          = validation::tryAqRateFixingTableCreate( tableName, currency, curveTenor, fixingDates, fixingValues );

                // Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );

				etrading::DataProvider dataProvider = getDataProvider(fixingTableName);
				auto swap = getSwapFromCacheWithCurveDataPopulated(swapTradeName, dataProvider );
				double actualFirstCashflowFloatRate = dataProvider.getCashflowDataExcludingUpfront( 0 ).floatRateData.resetRate;

                // 1) Check the first cashflow's floatRate matching firstFixing
                double expectedFirstFixing = swap->getLeg(1)->getStaticData()->getFirstFixing();

                // Check the Test Results
                EXPECT_NEAR( expectedFirstFixing, actualFirstCashflowFloatRate, tolerance );

                // 2) Check the last cashflow's floatRate matching lastFixing
                double expectedLastFixing = swap->getLeg(1)->getStaticData()->getLastFixing();

                auto cashflowSize = swap->getLeg(1)->getSchedule()->getCashflowSize();        
				double actualLastCashflowFloatRate = dataProvider.getCashflowDataExcludingUpfront( cashflowSize - 1 ).floatRateData.resetRate;

                // Check the Test Results
                EXPECT_NEAR( expectedLastFixing, actualLastCashflowFloatRate, tolerance );

        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }


    // Testing: When paymentDate is in the past (earlier than asOfDate) and no fixing table is provided, the floatRate should be zero
    TEST_F( TestAQObjSwapFloatRatesFixingsAndStubs, CONSISTENCY_CheckZeroFloatRates_WhenPaymentDateInThePast_NoFixingTable )
    {
        try
        {
               // Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs3 );
        
                // Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
				
				etrading::DataProvider dataProvider = getDataProvider("");

				auto swap = getSwapFromCacheWithCurveDataPopulated(swapTradeName, dataProvider );
				double actualFirstCashflowFloatRate = dataProvider.getCashflowDataExcludingUpfront( 0 ).floatRateData.resetRate;

                // Check the Test Results
                EXPECT_NEAR( 0, actualFirstCashflowFloatRate, tolerance );

        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }

    // Testing: When paymentDate is the same as asOfDate, floatRate is from Fixing Table
    TEST_F( TestAQObjSwapFloatRatesFixingsAndStubs, CONSISTENCY_SourceFloatRate_FromFixingTable_WhenPaymentDateIsAsOfDate )
    {
        try
        {
                // Load the Input Files
                const ReadDataFile::Load fixingTableInputFile( fixingTableInputs );
                const ReadDataFile::Load tradeInputFile( swapInputs4 );
        
                // Get the Fixing Table Inputs & Create the FixingTable
                auto tableName                  = fixingTableInputFile["tableName"];
                auto currency                   = fixingTableInputFile["currency"];
                auto curveTenor                 = fixingTableInputFile["curveTenor"];
                auto fixingDates                = fixingTableInputFile["fixingDates"];
                auto fixingValues               = fixingTableInputFile["fixingValues"];

                std::string fixingTableName          = validation::tryAqRateFixingTableCreate( tableName, currency, curveTenor, fixingDates, fixingValues );
        
                // Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
				
				etrading::DataProvider dataProvider = getDataProvider(fixingTableName);
				
				auto swap = getSwapFromCacheWithCurveDataPopulated(swapTradeName, dataProvider );
				double actualFirstCashflowFloatRate = dataProvider.getCashflowDataExcludingUpfront( 0 ).floatRateData.resetRate;

                auto floatLegFirstCashflow = swap->getLeg(1)->getSchedule()->getCashflow(0);

                AQLDate floatLegFirstCashflowFixingDate = floatLegFirstCashflow->getFixingDate();
                auto fixingTable = etrading::getFixingTable(tableName, false /* do not throw when missing*/);
                double expectedFixingRate = fixingTable->getFixingValue( etrading::toGregorianDateFromAQLDate(floatLegFirstCashflowFixingDate) );

                // Check the Test Results
                EXPECT_NEAR( expectedFixingRate, actualFirstCashflowFloatRate, tolerance );

        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }


    // Testing: When the current cashflow is not the first cashflow, FirstFixing is applied to the current cashflow.
    TEST_F( TestAQObjSwapFloatRatesFixingsAndStubs, CONSISTENCY_CheckFirstFixingParameter_AppliedToCurrentCashflow )
    {
        try
        {
                // Load the Input Files
                const ReadDataFile::Load fixingTableInputFile( fixingTableInputs );
                const ReadDataFile::Load tradeInputFile( swapInputs4 );
        
                // Get the Fixing Table Inputs & Create the FixingTable
                auto tableName                  = fixingTableInputFile["tableName"];
                auto currency                   = fixingTableInputFile["currency"];
                auto curveTenor                 = fixingTableInputFile["curveTenor"];
                auto fixingDates                = fixingTableInputFile["fixingDates"];
                auto fixingValues               = fixingTableInputFile["fixingValues"];

                std::string fixingTableName          = validation::tryAqRateFixingTableCreate( tableName, currency, curveTenor, fixingDates, fixingValues );
        
                // Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );

				etrading::DataProvider dataProvider = getDataProvider(fixingTableName);
				
				auto swap = getSwapFromCacheWithCurveDataPopulated(swapTradeName, dataProvider );
				double actualFirstCashflowFloatRate = dataProvider.getCashflowDataExcludingUpfront( 0 ).floatRateData.resetRate;

                //1) Check the first cashflow's floatRate is from Fixing Table
                auto floatLegFirstCashflow = swap->getLeg(1)->getSchedule()->getCashflow(0);
                AQLDate floatLegFirstCashflowFixingDate = floatLegFirstCashflow->getFixingDate();
                auto fixingTable = etrading::getFixingTable(tableName, false /* do not throw when missing*/);
                double expectedFixingRate = fixingTable->getFixingValue( etrading::toGregorianDateFromAQLDate(floatLegFirstCashflowFixingDate) );

                // Check the Test Results
                EXPECT_NEAR( expectedFixingRate, actualFirstCashflowFloatRate, tolerance );


                //2) The second cashflow is the current cashflow (i.e. PaymentDate is later than AsOfDate), check the current cashflow's floatRate is from First Fixing

                double expectedFirstFixing = swap->getLeg(1)->getStaticData()->getFirstFixing();

				double floatLegCurrentCashflowFloatRate = dataProvider.getCashflowDataExcludingUpfront(1).floatRateData.resetRate;

                // Check the Test Results
                EXPECT_NEAR( expectedFirstFixing, floatLegCurrentCashflowFloatRate, tolerance );
        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }

    // Testing: FirstFixing is applied when the firstFixingDate is later than AsOfDate
    TEST_F( TestAQObjSwapFloatRatesFixingsAndStubs, CONSISTENCY_CheckFirstFixingParameterApplied_WhenFirstFixingDateInFuture )
    {
        try
        {
                // Load the Input Files
                const ReadDataFile::Load fixingTableInputFile( fixingTableInputs );
                const ReadDataFile::Load tradeInputFile( swapInputs5 );
        
                // Get the Fixing Table Inputs & Create the FixingTable
                auto tableName                  = fixingTableInputFile["tableName"];
                auto currency                   = fixingTableInputFile["currency"];
                auto curveTenor                 = fixingTableInputFile["curveTenor"];
                auto fixingDates                = fixingTableInputFile["fixingDates"];
                auto fixingValues               = fixingTableInputFile["fixingValues"];

                std::string fixingTableName          = validation::tryAqRateFixingTableCreate( tableName, currency, curveTenor, fixingDates, fixingValues );
        
                // Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
				
				etrading::DataProvider dataProvider = getDataProvider(fixingTableName);
				
				auto swap = getSwapFromCacheWithCurveDataPopulated(swapTradeName, dataProvider );
				double actualFirstCashflowFloatRate = dataProvider.getCashflowDataExcludingUpfront( 0 ).floatRateData.resetRate;

                // Check the first cashflow's floatRate is from First Fixing
                double expectedFirstFixing = swap->getLeg(1)->getStaticData()->getFirstFixing();

                // Check the Test Results
                EXPECT_NEAR( expectedFirstFixing, actualFirstCashflowFloatRate, tolerance );

        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }

    // Testing: FirstFixing has higher priority than the first stub rate
    TEST_F( TestAQObjSwapFloatRatesFixingsAndStubs, CONSISTENCY_CheckStubPriority_FirstFixingParameter_HasHigherPriorityThan_InternalStubRateMethod )
    {
        try
        {
                // Load the Input Files
                const ReadDataFile::Load fixingTableInputFile( fixingTableInputs );
                const ReadDataFile::Load tradeInputFile( swapInputs5 );
        
                // Get the Fixing Table Inputs & Create the FixingTable
                auto tableName                  = fixingTableInputFile["tableName"];
                auto currency                   = fixingTableInputFile["currency"];
                auto curveTenor                 = fixingTableInputFile["curveTenor"];
                auto fixingDates                = fixingTableInputFile["fixingDates"];
                auto fixingValues               = fixingTableInputFile["fixingValues"];

                std::string fixingTableName          = validation::tryAqRateFixingTableCreate( tableName, currency, curveTenor, fixingDates, fixingValues );
        
                // Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
				
				etrading::DataProvider dataProvider = getDataProvider(fixingTableName);
				
				auto swap = getSwapFromCacheWithCurveDataPopulated(swapTradeName, dataProvider );
				double actualFirstCashflowFloatRate = dataProvider.getCashflowDataExcludingUpfront( 0 ).floatRateData.resetRate;

                // Check the first cashflow's floatRate is from First Fixing
                double expectedFirstFixing = swap->getLeg(1)->getStaticData()->getFirstFixing();

                // Check the Test Results
                EXPECT_NEAR( expectedFirstFixing, actualFirstCashflowFloatRate, tolerance );

        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }

    // Testing: StubRate is used when FirstFixing is not provided
    TEST_F( TestAQObjSwapFloatRatesFixingsAndStubs, SNAPSHOT_CheckSwapWithStub_StubRateUsedWhenNoFirstFixing )
    {
        try
        {
                // Load the Input Files
                const ReadDataFile::Load tradeInputFile( swapInputs6 );
        
                // Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
				auto dataProvider = getDataProvider("");
				auto swap = getSwapFromCacheWithCurveDataPopulated(swapTradeName, dataProvider);
				double actualFirstCashflowFloatRate = dataProvider.getCashflowDataExcludingUpfront( 0 ).floatRateData.resetRate;

				#if defined(GTEST32)
				AQLString outputFileName = TestOutputs_6_32bit;
				CheckTestResultsAndRebaseOnRequest( actualFirstCashflowFloatRate, TEST_DIR, outputFileName, tolerance );
				#else
				AQLString outputFileName = TestOutputs_6_64bit;
				CheckTestResultsAndRebaseOnRequest( actualFirstCashflowFloatRate, TEST_DIR, outputFileName, tolerance );
				#endif

        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }

    // Testing: When float leg has one cashflow, and both FirstFixing hand LastFixing are specified, FirstFixing is applied
    TEST_F( TestAQObjSwapFloatRatesFixingsAndStubs, CONSISTENCY_CheckSwapWithSingleCashflow_CheckFirstFixingParameter_HasHigherPriorityThan_LastFixingParameter )
    {
        try
        {
                // Load the Input Files
                const ReadDataFile::Load fixingTableInputFile( fixingTableInputs );
                const ReadDataFile::Load tradeInputFile( swapInputs7 );
        
                // Get the Fixing Table Inputs & Create the FixingTable
                auto tableName                  = fixingTableInputFile["tableName"];
                auto currency                   = fixingTableInputFile["currency"];
                auto curveTenor                 = fixingTableInputFile["curveTenor"];
                auto fixingDates                = fixingTableInputFile["fixingDates"];
                auto fixingValues               = fixingTableInputFile["fixingValues"];

                std::string fixingTableName          = validation::tryAqRateFixingTableCreate( tableName, currency, curveTenor, fixingDates, fixingValues );
        
                // Get the Trade Inputs & Create the Swap
                std::string swapTradeName       = tradeInputFile["swapName"];
                AQLStringMatrix swapLVB            = tradeInputFile["swapLVB"];
                AQLStringMatrix swapPropertiesLVB  = tradeInputFile["swapPropertiesLVB"];
                bool isXccySwap                 = tradeInputFile["isXccySwap"];
                bool validateKeys               = tradeInputFile["validateKeys"];

                std::string createSwap          = validation::tryAqSwapObjectCreate( swapTradeName, swapLVB, swapPropertiesLVB, isXccySwap, validateKeys );
				etrading::DataProvider dataProvider = getDataProvider(fixingTableName);
				auto swap = getSwapFromCacheWithCurveDataPopulated(swapTradeName, dataProvider );
				double actualFirstCashflowFloatRate = dataProvider.getCashflowDataExcludingUpfront( 0 ).floatRateData.resetRate;

                // Check the first cashflow's floatRate is from First Fixing
                double expectedFirstFixing = swap->getLeg(1)->getStaticData()->getFirstFixing();

                // Check the Test Results
                EXPECT_NEAR( expectedFirstFixing, actualFirstCashflowFloatRate, tolerance );

        }
        catch( const AQLCoreError& m )
        {
            std::cout <<  m.getMsg();
            ASSERT_FALSE( true );
        }
        catch( const std::exception& e )
        {
            std::cout << e.what();
            ASSERT_FALSE( true );
        }
    }







 

}
