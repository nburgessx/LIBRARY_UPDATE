// TestRiskUtilities.cpp

// Include: Google Test Library
#include <gTest/gTest.h>

#include "RiskUtilities.h"
#include "Dependency.h"
#include "InitializeETrading.h"
#include "GetGoogleTestFolder.h"
#include "tryMeLWO.h"

// Test Folder
const std::string TEST_FOLDER = "ETrading/Curves/TestCurveResults/";

// Test Files: USD CURVES
const std::string fileNameUSDOIS  = TEST_FOLDER + "USD_OIS_CURVE.JSON";
const std::string fileNameUSD3ML  = TEST_FOLDER + "USD_SWAP_3M_CURVE.JSON";
const std::string fileNameUSD6ML  = TEST_FOLDER + "USD_BASIS_6M_CURVE.JSON";
const std::string fileNameUSD1ML  = TEST_FOLDER + "USD_BASIS_1M_CURVE.JSON";
const std::string fileNameUSD12ML = TEST_FOLDER + "USD_BASIS_12M_CURVE.JSON";

namespace google_test
{

	DECLARE_TEST_FIXTURE(TestRiskUtilities);

	void loadUSDCurves()
	{
		// Load USD Curves
		const std::string curveObjectUSDOIS_  = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSDOIS, etrading::JSON );
		const std::string curveObjectUSD3ML_  = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD3ML, etrading::JSON );
		const std::string curveObjectUSD6ML_  = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD6ML, etrading::JSON );
		const std::string curveObjectUSD1ML_  = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD1ML, etrading::JSON );
		const std::string curveObjectUSD12ML_ = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + fileNameUSD12ML, etrading::JSON );
	}


	TEST_F( TestRiskUtilities, UNIT_isCurveList )
	{
		// Test Not a Curve List
		const AQLString curveName = "USD3ML";
		EXPECT_EQ( false, etrading::isCurveList( curveName ) );

		// Test for a Curve List
		const AQLString curveList = "USD3ML:USD6ML";
		EXPECT_EQ( true, etrading::isCurveList( curveList ) );

		// Another test for a Curve List
		const AQLString curveList1 = "USD3ML:USD6ML:USD1ML:USD12ML";
		EXPECT_EQ( true, etrading::isCurveList( curveList1 ) );
	}

	TEST_F( TestRiskUtilities, UNIT_generateCurveList )
	{
		// Test Single Curve Name
		const AQLString curveName = "USD3ML";
		const std::vector<AQLString> actualCurveList = etrading::generateCurveList( curveName );
		const std::vector<AQLString> expectedCurveList = { "USD3ML" };

		ASSERT_EQ( actualCurveList.size(), expectedCurveList.size() );
		for( size_t i = 0; i< actualCurveList.size(); ++i)
		{
			EXPECT_EQ( expectedCurveList[i], actualCurveList[i] );
		}

		// Test Multiple Curve Names
		const AQLString curveList = "USD1ML:USD3ML:USD6ML:USD12ML";
		const std::vector<AQLString> actualCurveList1 = etrading::generateCurveList( curveList );
		const std::vector<AQLString> expectedCurveList1 = { "USD1ML", "USD3ML", "USD6ML", "USD12ML" };

		ASSERT_EQ( actualCurveList1.size(), expectedCurveList1.size() );
		for( size_t i = 0; i < actualCurveList1.size(); ++i )
		{
			EXPECT_EQ( expectedCurveList1[i], actualCurveList1[i] );
		}
	}


	TEST_F( TestRiskUtilities, UNIT_generateStaticDataList )
	{
		// Load Curves
		loadUSDCurves();
		const AQLString curveCollection = "USDYC";
	

		// Test Single Curve Name
		const AQLString curveName = "USDOIS";
		const std::vector<AQLString> actualStaticDataList = etrading::generateStaticDataList( curveCollection, curveName );
		const std::vector<AQLString> expectedStaticDataList = { "OIS" };

		ASSERT_EQ( actualStaticDataList.size(), expectedStaticDataList.size() );
		for( size_t i = 0; i < actualStaticDataList.size(); ++i )
		{
			EXPECT_EQ( expectedStaticDataList[i], actualStaticDataList[i] );
		}

		// Test Multiple Curve Names
		const AQLString curveList = "USDOIS:USD1ML:USD3ML:USD6ML:USD12ML";
		const std::vector<AQLString> actualStaticDataList1 = etrading::generateStaticDataList( curveCollection, curveList );
		const std::vector<AQLString> expectedStaticDataList1 = { "OIS", "1M3MBASIS", "STD", "3M6MBASIS", "3M12MBASIS" };

		ASSERT_EQ( actualStaticDataList1.size(), expectedStaticDataList1.size() );
		for( size_t i = 0; i < actualStaticDataList1.size(); ++i )
		{
			EXPECT_EQ( expectedStaticDataList1[i], actualStaticDataList1[i] );
		}
	}

	TEST_F( TestRiskUtilities, UNIT_generateStaticDataListAsString )
	{
		// Load Curves
		loadUSDCurves();
		const AQLString curveCollection = "USDYC";


		// Test Single Curve Name
		const AQLString curveName = "USDOIS";
		const AQLString actualStaticDataList = etrading::generateStaticDataListAsString( curveCollection, curveName );
		const AQLString expectedStaticDataList = "OIS";
		EXPECT_EQ( expectedStaticDataList, actualStaticDataList );

		// Test Multiple Curve Names
		const AQLString curveList = "USDOIS:USD1ML:USD3ML:USD6ML:USD12ML";
		const AQLString actualStaticDataList1 = etrading::generateStaticDataListAsString( curveCollection, curveList );
		const AQLString expectedStaticDataList1 = "OIS:1M3MBASIS:STD:3M6MBASIS:3M12MBASIS";
		EXPECT_EQ( expectedStaticDataList1, actualStaticDataList1 );
	}

}