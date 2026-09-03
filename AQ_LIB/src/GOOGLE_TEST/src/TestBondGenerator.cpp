// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeAQGoogleTest.h"

#include "ObjectUtilities.h"
#include "tryMeLWOBond.h"

#include "Dependency.h"
#include "ReadDataFile.h"
#include "ResultsProcessor.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "ETrading/LWObjects/LWOBonds/BondGenerator/"

namespace
{
    // Test Bond Input File(s)
    extern const char bondInputsFile[]          = TEST_DIR "tryMeLWOBondCreateFromGenerator_inputs.csv";
    extern const char cleanPriceInputs[]        = TEST_DIR "tryMeLWOBondCleanPrice_inputs.csv";
    extern const char cleanPriceOutputs[]       = TEST_DIR "tryMeLWOBondCleanPrice_outputs.csv";
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestBondGenerator);

    TEST_F( TestBondGenerator, SNAPSHOT_tryMeLWOBondCreateFromGenerator )
    {
        try
        {
            // Create the Bond from Bond Generator
			// This assumes the USTREASURY_TYPE1 bond generator has already been loaded during AlgoQuantLib initialisation
			etrading::ReadDataFile::Load bondCreateFromGeneratorFileObj = etrading::ReadDataFile::Load( bondInputsFile );
		
			const std::string bondObjectName	  = bondCreateFromGeneratorFileObj[ "bondObjectName" ];
			const std::string bondGeneratorName   = bondCreateFromGeneratorFileObj[ "bondGeneratorName" ];
			const LAStringMatrix expressionLVB	  = bondCreateFromGeneratorFileObj[ "expressionLVB" ];
			const bool validateKeys               = bondCreateFromGeneratorFileObj[ "validateKeys" ];
		
			// Check that we can create the bond instance from the specified BondGenerator, and the
			// custom properties in the expressionLVB.
			const std::string bondObjHandle = validation_api::tryMeLWOBondCreateFromGenerator( bondObjectName, bondGeneratorName, expressionLVB, validateKeys );

			// Perform a sanity check: Calculate the clean price of the bond
			etrading::ReadDataFile::Load cleanPriceInputFileObj = etrading::ReadDataFile::Load( cleanPriceInputs );

			const std::string expectedBondObjHandle   = cleanPriceInputFileObj[ "bondObjectName" ];
			const std::vector<LADate> settlementDates = cleanPriceInputFileObj[ "settlementDates" ];
			const std::vector<double> yields          = cleanPriceInputFileObj[ "yields" ];
			const std::string yieldCalculationType    = cleanPriceInputFileObj[ "yieldCalculationType" ];

			std::vector<double> actualResults = validation_api::tryMeLWOBondCleanPrice( bondObjHandle, settlementDates, yields, yieldCalculationType );

            // Compare Results
            const double tolerance = 1.e-9;
            CheckTestResultsAndRebaseOnRequest( actualResults, TEST_DIR, LAString( cleanPriceOutputs ), tolerance );
			
        }
        catch( const ReadDataFile::LoadError& )
        {
            std::cout <<  "#Error: Unable to open the Google Test File";
            ASSERT_FALSE( true );
        }
        catch( const LACoreError& m )
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

    TEST_F( TestBondGenerator, UNIT_tryMeLWOBondCreateFromGenerator_US_Treasury_PriceAndRiskResults )
    {
        const std::string bondObjectName	  = "US_TREASURY_2.25_31-DEC-2023";
		const std::string bondGeneratorName   = "US_TREASURY_TYPE1";
			            
        // Parameter Place Holders of size 2  i.e. key and value
        LAStringVector p1( 2 );
        LAStringVector p2( 2 );
        LAStringVector p3( 2 );
        LAStringVector p4( 2 );
        LAStringVector p5( 2 );
        LAStringVector p6( 2 );
        LAStringVector p7( 2 );
        LAStringVector p8( 2 );
        LAStringVector p9( 2 );

        p1[0] = "BondDescription";              p1[1] = "US_TREASURY_2.25_31DEC2023";
        p2[0] = "ISIN";                         p2[1] = "US912828V236";                     
        p3[0] = "IssueDate";                    p3[1] = "42738";      // 3-Jan-2017
        p4[0] = "FirstCouponDate";              p4[1] = "42916";      // 30-Jun-2017
        p5[0] = "LastCouponDate";               p5[1] = "45107";      // 30-Jun-2023
        p6[0] = "MaturityDate";                 p6[1] = "45291";      // 31-Dec-2023
        p7[0] = "Coupon";                       p7[1] = "0.02250";    // 2.25%
        p8[0] = "RollDay";                      p8[1] = "";
        p9[0] = "AccrualStartDate";             p9[1] = "42735";      // 31-Dec-2016
            
        // Send Parameters to Bond Expression LVB - size 9 to hold 9 parameter key-values
        LAStringMatrix expressionLVB( 9 );
        expressionLVB[0] = p1;
        expressionLVB[1] = p2;
        expressionLVB[2] = p3;
        expressionLVB[3] = p4;
        expressionLVB[4] = p5;
        expressionLVB[5] = p6;
        expressionLVB[6] = p7;
        expressionLVB[7] = p8;
        expressionLVB[8] = p9;
		
		// Create Bond from Generator
		const std::string bondObjHandle = validation_api::tryMeLWOBondCreateFromGenerator( bondObjectName, bondGeneratorName, expressionLVB, true ); // Validate keys = true

        // Settlement Date
        const LADate settlementDate = LADate("20180131"); // 31-Jan-2018
        const std::vector< LADate > settlementDates( 1, settlementDate );
            
        // Yield
        const double yield = 0.025838257000; // 2.58%
        const std::vector< double > yields( 1, yield );
            
        // Price and Risk Results
        std::vector<double> cleanPrices         = validation_api::tryMeLWOBondCleanPrice( bondObjHandle, settlementDates, yields );
        std::vector<double> dirtyPrices         = validation_api::tryMeLWOBondDirtyPrice( bondObjHandle, settlementDates, yields );
        std::vector<double> accruedInterests    = validation_api::tryMeLWOBondAccruedInterest( bondObjHandle, settlementDates );
        std::vector<int> accruedInterestDays    = validation_api::tryMeLWOBondAccruedInterestDays( bondObjHandle, settlementDates );
        std::vector<double> dv01s               = validation_api::tryMeLWOBondDV01( bondObjHandle, settlementDates, yields );

        // Compare Results
        const double resultsTolerance = 1.0e-6;
        
        ASSERT_EQ( cleanPrices.size(), 1 );
        EXPECT_NEAR( cleanPrices[0], 98.179553722503911, resultsTolerance );

        ASSERT_EQ( dirtyPrices.size(), 1 );
        EXPECT_NEAR( dirtyPrices[0], 98.377355920306, resultsTolerance );

        ASSERT_EQ( accruedInterests.size(), 1 );
        EXPECT_NEAR( accruedInterests[0], 1978.02197802198, resultsTolerance );

        ASSERT_EQ( accruedInterestDays.size(), 1 );
        EXPECT_EQ( accruedInterestDays[0], 32 ); // 32 Days of Accrued Interest

        ASSERT_EQ( dv01s.size(), 1 );
        EXPECT_NEAR( dv01s[0], 539.4301256626400, resultsTolerance );

    }


    TEST_F( TestBondGenerator, UNIT_tryMeLWOBondCreateFromGenerator_GermanBund_PriceAndRiskResults )
    {
        const std::string bondObjectName	  = "BUNDESREPUBLIK_2.00_15AUG2023";
		const std::string bondGeneratorName   = "GERMAN_BUND_TYPE60";
			            
        // Parameter Place Holders of size 2  i.e. key and value
        LAStringVector p1( 2 );
        LAStringVector p2( 2 );
        LAStringVector p3( 2 );
        LAStringVector p4( 2 );
        LAStringVector p5( 2 );
        LAStringVector p6( 2 );
        LAStringVector p7( 2 );
        LAStringVector p8( 2 );
        LAStringVector p9( 2 );

        p1[0] = "BondDescription";              p1[1] = "BUNDESREPUBLIK_2.00_15AUG2023";
        p2[0] = "ISIN";                         p2[1] = "DE0001102325";                     
        p3[0] = "IssueDate";                    p3[1] = "41530";      // 13-Sep-2013
        p4[0] = "FirstCouponDate";              p4[1] = "41866";      // 15-Aug-2014
        p5[0] = "LastCouponDate";               p5[1] = "44788";      // 15-Aug-2023
        p6[0] = "MaturityDate";                 p6[1] = "45153";      // 31-Dec-2023
        p7[0] = "Coupon";                       p7[1] = "0.0200";     // 2.00%
        p8[0] = "RollDay";                      p8[1] = "";
        p9[0] = "AccrualStartDate";             p9[1] = "41501";      // 15-Aug-2013
            
        // Send Parameters to Bond Expression LVB - size 9 to hold 9 parameter key-values
        LAStringMatrix expressionLVB( 9 );
        expressionLVB[0] = p1;
        expressionLVB[1] = p2;
        expressionLVB[2] = p3;
        expressionLVB[3] = p4;
        expressionLVB[4] = p5;
        expressionLVB[5] = p6;
        expressionLVB[6] = p7;
        expressionLVB[7] = p8;
        expressionLVB[8] = p9;
		
		// Create Bond from Generator
		const std::string bondObjHandle = validation_api::tryMeLWOBondCreateFromGenerator( bondObjectName, bondGeneratorName, expressionLVB, true ); // Validate keys = true

        // Settlement Date
        const LADate settlementDate = LADate("20180201"); // 1-Feb-2018
        const std::vector< LADate > settlementDates( 1, settlementDate );
            
        // Yield
        const double yield = -0.00242816751526698; // -0.2428%
        const std::vector< double > yields( 1, yield );
            
        // Price and Risk Results
        std::vector<double> cleanPrices         = validation_api::tryMeLWOBondCleanPrice( bondObjHandle, settlementDates, yields );
        std::vector<double> dirtyPrices         = validation_api::tryMeLWOBondDirtyPrice( bondObjHandle, settlementDates, yields );
        std::vector<double> accruedInterests    = validation_api::tryMeLWOBondAccruedInterest( bondObjHandle, settlementDates );
        std::vector<int> accruedInterestDays    = validation_api::tryMeLWOBondAccruedInterestDays( bondObjHandle, settlementDates );
        std::vector<double> dv01s               = validation_api::tryMeLWOBondDV01( bondObjHandle, settlementDates, yields );

        // Compare Results
        const double resultsTolerance = 1.0e-6;
        
        ASSERT_EQ( cleanPrices.size(), 1 );
        EXPECT_NEAR( cleanPrices[0], 112.511977775776, resultsTolerance );

        ASSERT_EQ( dirtyPrices.size(), 1 );
        EXPECT_NEAR( dirtyPrices[0], 113.443484625091, resultsTolerance );

        ASSERT_EQ( accruedInterests.size(), 1 );
        EXPECT_NEAR( accruedInterests[0], 9315.06849315068, resultsTolerance );

        ASSERT_EQ( accruedInterestDays.size(), 1 );
        EXPECT_EQ( accruedInterestDays[0], 170 ); // 170 Days of Accrued Interest

        ASSERT_EQ( dv01s.size(), 1 );
        EXPECT_NEAR( dv01s[0], 599.142376691742, resultsTolerance );

    }

    TEST_F( TestBondGenerator, UNIT_tryMeLWOBondCreateFromGenerator_JapaneseJGB_PriceAndRiskResults )
    {
        const std::string bondObjectName	  = "JAPAN_JGB_2.20_20JUN2024";
		const std::string bondGeneratorName   = "JAPAN_JGB_TYPE51";
			            
        // Parameter Place Holders of size 2  i.e. key and value
        LAStringVector p1( 2 );
        LAStringVector p2( 2 );
        LAStringVector p3( 2 );
        LAStringVector p4( 2 );
        LAStringVector p5( 2 );
        LAStringVector p6( 2 );
        LAStringVector p7( 2 );
        LAStringVector p8( 2 );
        LAStringVector p9( 2 );

        p1[0] = "BondDescription";              p1[1] = "JAPAN_JGB_2.20_20JUN2024";
        p2[0] = "ISIN";                         p2[1] = "JP1200711487";                     
        p3[0] = "IssueDate";                    p3[1] = "41881";      // 30-Aug-2014
        p4[0] = "FirstCouponDate";              p4[1] = "41993";      // 20-Dec-2014
        p5[0] = "LastCouponDate";               p5[1] = "45280";      // 20-Dec-2023
        p6[0] = "MaturityDate";                 p6[1] = "45463";      // 20-Jun-2024
        p7[0] = "Coupon";                       p7[1] = "0.0220";     // 2.20%
        p8[0] = "RollDay";                      p8[1] = "";
        p9[0] = "AccrualStartDate";             p9[1] = "41810";      // 20-Jun-2014
            
        // Send Parameters to Bond Expression LVB - size 9 to hold 9 parameter key-values
        LAStringMatrix expressionLVB( 9 );
        expressionLVB[0] = p1;
        expressionLVB[1] = p2;
        expressionLVB[2] = p3;
        expressionLVB[3] = p4;
        expressionLVB[4] = p5;
        expressionLVB[5] = p6;
        expressionLVB[6] = p7;
        expressionLVB[7] = p8;
        expressionLVB[8] = p9;
		
		// Create Bond from Generator
		const std::string bondObjHandle = validation_api::tryMeLWOBondCreateFromGenerator( bondObjectName, bondGeneratorName, expressionLVB, true ); // Validate keys = true

        // Settlement Date
        const LADate settlementDate = LADate("20180201"); // 1-Feb-2018
        const std::vector< LADate > settlementDates( 1, settlementDate );
            
        // Yield
        const double yield = -0.000370; // -0.00370%
        const std::vector< double > yields( 1, yield );
            
        // Price and Risk Results
        std::vector<double> cleanPrices         = validation_api::tryMeLWOBondCleanPrice( bondObjHandle, settlementDates, yields );
        std::vector<double> dirtyPrices         = validation_api::tryMeLWOBondDirtyPrice( bondObjHandle, settlementDates, yields );
        std::vector<double> accruedInterests    = validation_api::tryMeLWOBondAccruedInterest( bondObjHandle, settlementDates );
        std::vector<int> accruedInterestDays    = validation_api::tryMeLWOBondAccruedInterestDays( bondObjHandle, settlementDates );
        std::vector<double> dv01s               = validation_api::tryMeLWOBondDV01( bondObjHandle, settlementDates, yields );

        // Compare Results
        const double resultsTolerance = 1.0e-6;
        
        ASSERT_EQ( cleanPrices.size(), 1 );
        EXPECT_NEAR( cleanPrices[0], 114.307677685183, resultsTolerance );

        ASSERT_EQ( dirtyPrices.size(), 1 );
        EXPECT_NEAR( dirtyPrices[0], 114.300000000000, resultsTolerance );

        ASSERT_EQ( accruedInterests.size(), 1 );
        EXPECT_NEAR( accruedInterests[0], 2591.78000000, resultsTolerance );

        ASSERT_EQ( accruedInterestDays.size(), 1 );
        EXPECT_EQ( accruedInterestDays[0], 43 ); // 43 Days of Accrued Interest

        ASSERT_EQ( dv01s.size(), 1 );
        EXPECT_NEAR( dv01s[0], 686.427945205479, resultsTolerance );

    }

    TEST_F( TestBondGenerator, UNIT_tryMeLWOBondCreateFromGenerator_FrenchOAT_PriceAndRiskResults )
    {
        const std::string bondObjectName	  = "FRENCH_OAT_1.750_25MAY2023";
		const std::string bondGeneratorName   = "FRENCH_OAT_TYPE89";
			            
        // Parameter Place Holders of size 2  i.e. key and value
        LAStringVector p1( 2 );
        LAStringVector p2( 2 );
        LAStringVector p3( 2 );
        LAStringVector p4( 2 );
        LAStringVector p5( 2 );
        LAStringVector p6( 2 );
        LAStringVector p7( 2 );
        LAStringVector p8( 2 );
        LAStringVector p9( 2 );

        p1[0] = "BondDescription";              p1[1] = "FRENCH_OAT_1.750_25MAY2023";
        p2[0] = "ISIN";                         p2[1] = "FR0011486067";                     
        p3[0] = "IssueDate";                    p3[1] = "41401";      // 7-May-2013
        p4[0] = "FirstCouponDate";              p4[1] = "41419";      // 25-May-2013
        p5[0] = "LastCouponDate";               p5[1] = "44706";      // 25-May-2022
        p6[0] = "MaturityDate";                 p6[1] = "45071";      // 25-May-2023
        p7[0] = "Coupon";                       p7[1] = "0.0175";     // 1.750%
        p8[0] = "RollDay";                      p8[1] = "";
        p9[0] = "AccrualStartDate";             p9[1] = "41054";      // 25-May-2012
            
        // Send Parameters to Bond Expression LVB - size 9 to hold 9 parameter key-values
        LAStringMatrix expressionLVB( 9 );
        expressionLVB[0] = p1;
        expressionLVB[1] = p2;
        expressionLVB[2] = p3;
        expressionLVB[3] = p4;
        expressionLVB[4] = p5;
        expressionLVB[5] = p6;
        expressionLVB[6] = p7;
        expressionLVB[7] = p8;
        expressionLVB[8] = p9;
		
		// Create Bond from Generator
		const std::string bondObjHandle = validation_api::tryMeLWOBondCreateFromGenerator( bondObjectName, bondGeneratorName, expressionLVB, true ); // Validate keys = true

        // Settlement Date
        const LADate settlementDate = LADate("20180201"); // 1-Feb-2018
        const std::vector< LADate > settlementDates( 1, settlementDate );
            
        // Yield
        const double yield = -0.00055709175025347; // -0.0557%
        const std::vector< double > yields( 1, yield );
            
        // Price and Risk Results
        std::vector<double> cleanPrices         = validation_api::tryMeLWOBondCleanPrice( bondObjHandle, settlementDates, yields );
        std::vector<double> dirtyPrices         = validation_api::tryMeLWOBondDirtyPrice( bondObjHandle, settlementDates, yields );
        std::vector<double> accruedInterests    = validation_api::tryMeLWOBondAccruedInterest( bondObjHandle, settlementDates );
        std::vector<int> accruedInterestDays    = validation_api::tryMeLWOBondAccruedInterestDays( bondObjHandle, settlementDates );
        std::vector<double> dv01s               = validation_api::tryMeLWOBondDV01( bondObjHandle, settlementDates, yields );

        // Compare Results
        const double resultsTolerance = 1.0e-6;
        
        ASSERT_EQ( cleanPrices.size(), 1 );
        EXPECT_NEAR( cleanPrices[0], 109.60455099614, resultsTolerance );

        ASSERT_EQ( dirtyPrices.size(), 1 );
        EXPECT_NEAR( dirtyPrices[0], 110.81277019614, resultsTolerance );

        ASSERT_EQ( accruedInterests.size(), 1 );
        EXPECT_NEAR( accruedInterests[0], 12082.19200000, resultsTolerance );

        ASSERT_EQ( accruedInterestDays.size(), 1 );
        EXPECT_EQ( accruedInterestDays[0], 252 ); // 252 Days of Accrued Interest

        ASSERT_EQ( dv01s.size(), 1 );
        EXPECT_NEAR( dv01s[0], 562.409534377597, resultsTolerance );

    }

    TEST_F( TestBondGenerator, UNIT_tryMeLWOBondCreateFromGenerator_ItalianBTPS_PriceAndRiskResults )
    {
        const std::string bondObjectName	  = "ITALIAN_BTPS_8.5_22JUN2023";
		const std::string bondGeneratorName   = "ITALIAN_BTPS_TYPE523";
			            
        // Parameter Place Holders of size 2  i.e. key and value
        LAStringVector p1( 2 );
        LAStringVector p2( 2 );
        LAStringVector p3( 2 );
        LAStringVector p4( 2 );
        LAStringVector p5( 2 );
        LAStringVector p6( 2 );
        LAStringVector p7( 2 );
        LAStringVector p8( 2 );
        LAStringVector p9( 2 );
        LAStringVector p10( 2 );

        p1[0] = "BondDescription";              p1[1] = "ITALIAN_BTPS_8.5_22JUN2023";
        p2[0] = "ISIN";                         p2[1] = "IT0000366721";                     
        p3[0] = "IssueDate";                    p3[1] = "42360";      // 22-Dec-2015
        p4[0] = "FirstCouponDate";              p4[1] = "42543";      // 22-Jun-2016
        p5[0] = "LastCouponDate";               p5[1] = "45099";      // 22-Jun-2023
        p6[0] = "MaturityDate";                 p6[1] = "45282";      // 22-Dec-2023
        p7[0] = "Coupon";                       p7[1] = "0.085000";   // 8.5%
        p8[0] = "RollDay";                      p8[1] = "";
        p9[0] = "AccrualStartDate";             p9[1] = "42360";      // 22-Dec-2015
        p10[0] = "IssuePrice";                  p10[1] = "92.50";     // 92.50
            
        // Send Parameters to Bond Expression LVB - size 10 to hold 10 parameter key-values
        LAStringMatrix expressionLVB( 10 );
        expressionLVB[0] = p1;
        expressionLVB[1] = p2;
        expressionLVB[2] = p3;
        expressionLVB[3] = p4;
        expressionLVB[4] = p5;
        expressionLVB[5] = p6;
        expressionLVB[6] = p7;
        expressionLVB[7] = p8;
        expressionLVB[8] = p9;
        expressionLVB[9] = p10;
		
		// Create Bond from Generator
		const std::string bondObjHandle = validation_api::tryMeLWOBondCreateFromGenerator( bondObjectName, bondGeneratorName, expressionLVB, true ); // Validate keys = true

        // Settlement Date
        const LADate settlementDate = LADate("20180201"); // 1-Feb-2018
        const std::vector< LADate > settlementDates( 1, settlementDate );
            
        // Yield
        const double yield = 0.0147; // 1.47%
        const std::vector< double > yields( 1, yield );
            
        // Price and Risk Results
        std::vector<double> cleanPrices         = validation_api::tryMeLWOBondCleanPrice( bondObjHandle, settlementDates, yields );
        std::vector<double> dirtyPrices         = validation_api::tryMeLWOBondDirtyPrice( bondObjHandle, settlementDates, yields );
        std::vector<double> accruedInterests    = validation_api::tryMeLWOBondAccruedInterest( bondObjHandle, settlementDates );
        std::vector<int> accruedInterestDays    = validation_api::tryMeLWOBondAccruedInterestDays( bondObjHandle, settlementDates );
        std::vector<double> dv01s               = validation_api::tryMeLWOBondDV01( bondObjHandle, settlementDates, yields );

        // Compare Results
        const double resultsTolerance = 1.0e-6;
        
        ASSERT_EQ( cleanPrices.size(), 1 );
        EXPECT_NEAR( cleanPrices[0], 139.542175877533, resultsTolerance );

        ASSERT_EQ( dirtyPrices.size(), 1 );
        EXPECT_NEAR( dirtyPrices[0], 140.499595877533, resultsTolerance );

        ASSERT_EQ( accruedInterests.size(), 1 );
        EXPECT_NEAR( accruedInterests[0], 9574.2, resultsTolerance );

        ASSERT_EQ( accruedInterestDays.size(), 1 );
        EXPECT_EQ( accruedInterestDays[0], 41 ); // 41 Days of Accrued Interest

        ASSERT_EQ( dv01s.size(), 1 );
        EXPECT_NEAR( dv01s[0], 686.017529761819, resultsTolerance );

    }
}

