// TestCurveMarketDataUseParameter.cpp

// Test Includes
#include "InitializeGoogleTest.h"			// Load static and gtest.h
#include "GetGoogleTestFolder.h"
#include "RecordMacros.h"						// Logfile Macros
#include "ResultsProcessor.h"					// Test Rebase Methods
#include "tryMeLWO.h"							// LWO Curve Loading et al.
#include "tryMeLWOCurveMarketData.h"			// Curve Market Data Bumping Method(s)
#include "CurveMarketData.h"					// Method to clean market data - checkDimensionsAndRemoveUnusedData

#include "ReadDataFile.h"
using etrading::ReadDataFile;

// Test Folders
const std::string TEST_FOLDER					= "ETrading/Curves/TestCurveMarketData2/";

// File Names
const std::string emptyData_OIS					= "EUR_OIS_NODATA";
const std::string emptyData_Swap				= "EUR_SWAP_NODATA";
const std::string emptyData_TenorBasis			= "EUR_TENOR_BASIS_NODATA";
const std::string emptyData_Xccy				= "EUR_XCCY_NODATA";

const std::string singleDataPoint_OIS			= "EUR_OIS_SINGLE_POINT";
const std::string singleDataPoint_Swap			= "EUR_SWAP_SINGLE_POINT";
const std::string singleDataPoint_TenorBasis	= "EUR_TENOR_BASIS_SINGLE_POINT";
const std::string singleDataPoint_Xccy			= "EUR_XCCY_SINGLE_POINT";

// Curve Market Data LWO Files
const std::string oisMarket_NoData				= TEST_FOLDER + emptyData_OIS;
const std::string swapMarket_NoData				= TEST_FOLDER + emptyData_Swap;
const std::string tenorBasisMarket_NoData		= TEST_FOLDER + emptyData_TenorBasis;
const std::string xccyMarket_NoData				= TEST_FOLDER + emptyData_Xccy;

const std::string oisMarket_SinglePoint			= TEST_FOLDER + singleDataPoint_OIS;
const std::string swapMarket_SinglePoint		= TEST_FOLDER + singleDataPoint_Swap;
const std::string tenorBasisMarket_SinglePoint	= TEST_FOLDER + singleDataPoint_TenorBasis;
const std::string xccyMarket_SinglePoint		= TEST_FOLDER + singleDataPoint_Xccy;

namespace google_test
{
	// TEST FIXTURE CONSTRUCTOR
    DECLARE_TEST_FIXTURE( TestCurveMarketDataUseParameter )

	TEST_F( TestCurveMarketDataUseParameter, UNIT_LoadOisCurve_DisableAllData )
	{
		// Load Curve Market Data
		const std::string loadMarketData = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + oisMarket_NoData, etrading::JSON );

		// Get Market Data
		etrading::VariantMatrix results1 = validation::tryMeLWOCurveMarketDataDisplay( emptyData_OIS, "OIS" );
		etrading::VariantMatrix results2 = validation::tryMeLWOCurveMarketDataDisplay( emptyData_OIS, "LIBOROISBASISSPREADS" );
		etrading::VariantMatrix results3 = validation::tryMeLWOCurveMarketDataDisplay( emptyData_OIS, "SWAPS" );
	
		// Expecting Empty Data Results			
		EXPECT_EQ( 0, results1.size() );
		EXPECT_EQ( 0, results2.size() );
		EXPECT_EQ( 0, results3.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_LoadOisCurve_EnableSingleDataPoint )
	{
		// Load Curve Market Data
		const std::string loadMarketData = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + oisMarket_SinglePoint, etrading::JSON );

		// Get Market Data
		etrading::VariantMatrix results1 = validation::tryMeLWOCurveMarketDataDisplay( singleDataPoint_OIS, "OIS" );
		etrading::VariantMatrix results2 = validation::tryMeLWOCurveMarketDataDisplay( singleDataPoint_OIS, "LIBOROISBASISSPREADS" );
		etrading::VariantMatrix results3 = validation::tryMeLWOCurveMarketDataDisplay( singleDataPoint_OIS, "SWAPS" );

		// Expecting Single Data Point Results			
		EXPECT_EQ( 1, results1.size() );
		EXPECT_EQ( 1, results2.size() );
		EXPECT_EQ( 1, results3.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_LoadSwapCurve_DisableAllData )
	{
		// Load Curve Market Data
		const std::string loadMarketData	= validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + swapMarket_NoData, etrading::JSON );

		// Get Market Data
		etrading::VariantMatrix results1 = validation::tryMeLWOCurveMarketDataDisplay( emptyData_Swap, "FUTURES" );
		etrading::VariantMatrix results2 = validation::tryMeLWOCurveMarketDataDisplay( emptyData_Swap, "SWAPS" );
	
		// Expecting Empty Data Results			
		EXPECT_EQ( 0, results1.size() );
		EXPECT_EQ( 0, results2.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_LoadSwapCurve_EnableSingleDataPoint )
	{
		// Load Curve Market Data
		const std::string loadMarketData = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + swapMarket_SinglePoint, etrading::JSON );

		// Get Market Data
		etrading::VariantMatrix results1 = validation::tryMeLWOCurveMarketDataDisplay( singleDataPoint_Swap, "FUTURES" );
		etrading::VariantMatrix results2 = validation::tryMeLWOCurveMarketDataDisplay( singleDataPoint_Swap, "SWAPS" );

		// Expecting Single Data Point Results			
		EXPECT_EQ( 1, results1.size() );
		EXPECT_EQ( 1, results2.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_LoadTenorBasisCurve_DisableAllData )
	{
		// Load Curve Market Data
		const std::string loadMarketData	= validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + tenorBasisMarket_NoData, etrading::JSON );

		// Get Market Data
		etrading::VariantMatrix results1 = validation::tryMeLWOCurveMarketDataDisplay( emptyData_TenorBasis, "BASISSWAPS" );
	
		// Expecting Empty Data Results			
		EXPECT_EQ( 0, results1.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_LoadTenorBasisCurve_EnableSingleDataPoint )
	{
		// Load Curve Market Data
		const std::string loadMarketData = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + tenorBasisMarket_SinglePoint, etrading::JSON );

		// Get Market Data
		etrading::VariantMatrix results1 = validation::tryMeLWOCurveMarketDataDisplay( singleDataPoint_TenorBasis, "BASISSWAPS" );

		// Expecting Single Data Point Results			
		EXPECT_EQ( 1, results1.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_LoadXccyCurve_DisableAllData )
	{
		// Load Curve Market Data
		const std::string loadMarketData	= validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + xccyMarket_NoData, etrading::JSON );

		// Get Market Data
		etrading::VariantMatrix results1 = validation::tryMeLWOCurveMarketDataDisplay( emptyData_Xccy, "FXFWDS" );
		etrading::VariantMatrix results2 = validation::tryMeLWOCurveMarketDataDisplay( emptyData_Xccy, "XCCYSWAPS" );

		// Expecting Empty Data Results			
		EXPECT_EQ( 0, results1.size() );
		EXPECT_EQ( 0, results2.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_LoadXccyCurve_EnableSingleDataPoint )
	{
		// Load Curve Market Data
		const std::string loadMarketData = validation::tryMeLWOLoad( etrading::getGoogleTestFolder() + xccyMarket_SinglePoint, etrading::JSON );

		// Get Market Data
		etrading::VariantMatrix results1 = validation::tryMeLWOCurveMarketDataDisplay( singleDataPoint_Xccy, "FXFWDS" );
		etrading::VariantMatrix results2 = validation::tryMeLWOCurveMarketDataDisplay( singleDataPoint_Xccy, "XCCYSWAPS" );

		// Expecting Single Data Point Results			
		EXPECT_EQ( 1, results1.size() );
		EXPECT_EQ( 1, results2.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_OISCurve_DisableAllData )
	{
		// OIS Swaps & Futures Columns: Tenor, Rate, Start, End, Convexity, Use
		const etrading::VariantMatrix oisData		= {	{"ON", -0.00467,   "",	  "",	 0,	"FALSE"},
														{"ECB1", -0.00476, 44090, 44138, 0, "FALSE"},
														{"ECB2", -0.00483, 44139, 44180, 0, "FALSE"} };

		// LIBOR-OIS Basis Columns: Tenor, Rate, BasisType, Use
		const etrading::VariantMatrix lobasisData	= {	{"1Y", 0.00013, "LIBOR", "FALSE"},
														{"2Y", 0.00026, "LIBOR", "FALSE"},
														{"3Y", 0.00039, "LIBOR", "FALSE"} };

		// SWAPS Basis Columns: Tenor, Rate, Use
		const etrading::VariantMatrix swapsData		= {	{"1Y", -0.00505, "FALSE"},
														{"2Y", -0.00518, "FALSE"},
														{"3Y", -0.00514, "FALSE"} };

		// Check Market Data Dimensions and Remove Unused Data
		// Note JSON Variant Matrix Inputs need to be Transposed
		etrading::VariantMatrix results1 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "OIS",					transpose(oisData) );
		etrading::VariantMatrix results2 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "LIBOROISBASISSPREADS", transpose(lobasisData) );
		etrading::VariantMatrix results3 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "SWAPS",				transpose(swapsData) );
	
		// JSON Variant Matrix Results are Transposed
		results1 = transpose( results1 );
		results2 = transpose( results2 );
		results3 = transpose( results3 );

		// Expecting Empty Data Results			
		EXPECT_EQ( 0, results1.size() );
		EXPECT_EQ( 0, results2.size() );
		EXPECT_EQ( 0, results3.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_OISCurve_SingleDataPoint )
	{
		// OIS Swaps & Futures Columns: Tenor, Rate, Start, End, Convexity, Use
		const etrading::VariantMatrix oisData		= {	{"ON", -0.00467,   "",	  "",	 0,	"TRUE"},
														{"ECB1", -0.00476, 44090, 44138, 0, "FALSE"},
														{"ECB2", -0.00483, 44139, 44180, 0, "FALSE"} };

		// LIBOR-OIS Basis Columns: Tenor, Rate, BasisType, Use
		const etrading::VariantMatrix lobasisData	= {	{"1Y", 0.00013, "LIBOR", "TRUE"},
														{"2Y", 0.00026, "LIBOR", "FALSE"},
														{"3Y", 0.00039, "LIBOR", "FALSE"} };

		// SWAPS Basis Columns: Tenor, Rate, Use
		const etrading::VariantMatrix swapsData		= {	{"1Y", -0.00505, "TRUE"},
														{"2Y", -0.00518, "FALSE"},
														{"3Y", -0.00514, "FALSE"} };

		// Check Market Data Dimensions and Remove Unused Data
		// Note JSON Variant Matrix Inputs need to be Transposed
		etrading::VariantMatrix results1 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "OIS",					transpose(oisData) );
		etrading::VariantMatrix results2 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "LIBOROISBASISSPREADS", transpose(lobasisData) );
		etrading::VariantMatrix results3 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "SWAPS",				transpose(swapsData) );
		
		// JSON Variant Matrix Results are Transposed
		results1 = transpose( results1 );
		results2 = transpose( results2 );
		results3 = transpose( results3 );

		// Expecting Empty Data Results			
		EXPECT_EQ( 1, results1.size() );
		EXPECT_EQ( 1, results2.size() );
		EXPECT_EQ( 1, results3.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_SwapCurve_DisableAllData )
	{
		// FUTURES Columns: Tenor, Start, End, Rate, Convexity, Use
		const etrading::VariantMatrix futuresData	= {	{"ER1", 44090, 44181, 100.485, 0.005, "FALSE"},
														{"ER2", 44181, 44272, 100.490, 0.005, "FALSE"},
														{"ER3", 44272, 44363, 100.505, 0.005, "FALSE"} };

		// SWAPS Basis Columns: Tenor, Rate, Use
		const etrading::VariantMatrix swapsData		= {	{"3Y", -0.00514, "FALSE"},
														{"4Y", -0.00495, "FALSE"},
														{"5Y", -0.00467, "FALSE"} };

		// Check Market Data Dimensions and Remove Unused Data
		// Note JSON Variant Matrix Inputs need to be Transposed
		etrading::VariantMatrix results1 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "FUTURES",	transpose(futuresData) );
		etrading::VariantMatrix results2 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "SWAPS",		transpose(swapsData) );
	
		// JSON Variant Matrix Results are Transposed
		results1 = transpose( results1 );
		results2 = transpose( results2 );

		// Expecting Empty Data Results			
		EXPECT_EQ( 0, results1.size() );
		EXPECT_EQ( 0, results2.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_SwapCurve_SingleDataPoint )
	{
		// FUTURES Columns: Tenor, Start, End, Rate, Convexity, Use
		const etrading::VariantMatrix futuresData	= {	{"ER1", 44090, 44181, 100.485, 0.005, "TRUE"},
														{"ER2", 44181, 44272, 100.490, 0.005, "FALSE"},
														{"ER3", 44272, 44363, 100.505, 0.005, "FALSE"} };

		// SWAPS Basis Columns: Tenor, Rate, Use
		const etrading::VariantMatrix swapsData		= {	{"3Y", -0.00514, "TRUE"},
														{"4Y", -0.00495, "FALSE"},
														{"5Y", -0.00467, "FALSE"} };

		// Check Market Data Dimensions and Remove Unused Data
		// Note JSON Variant Matrix Inputs need to be Transposed
		etrading::VariantMatrix results1 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "FUTURES",	transpose(futuresData) );
		etrading::VariantMatrix results2 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "SWAPS",		transpose(swapsData) );
	
		// JSON Variant Matrix Results are Transposed
		results1 = transpose( results1 );
		results2 = transpose( results2 );

		// Expecting Empty Data Results			
		EXPECT_EQ( 1, results1.size() );
		EXPECT_EQ( 1, results2.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_BasisCurve_DisableAllData )
	{
		// BASISSWAPS Basis Columns: Tenor, Rate, Use
		const etrading::VariantMatrix basisData		= {	{"1Y", 0.00045, "FALSE"},
														{"2Y", 0.00047, "FALSE"},
														{"3Y", 0.00048, "FALSE"} };

		// Check Market Data Dimensions and Remove Unused Data
		// Note JSON Variant Matrix Inputs need to be Transposed
		etrading::VariantMatrix results1 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "BASISSWAPS",	transpose(basisData) );
	
		// JSON Variant Matrix Results are Transposed
		results1 = transpose( results1 );

		// Expecting Empty Data Results			
		EXPECT_EQ( 0, results1.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_BasisCurve_SingleDataPoint )
	{
		// BASISSWAPS Basis Columns: Tenor, Rate, Use
		const etrading::VariantMatrix basisData		= {	{"1Y", 0.00045, "TRUE"},
														{"2Y", 0.00047, "FALSE"},
														{"3Y", 0.00048, "FALSE"} };

		// Check Market Data Dimensions and Remove Unused Data
		// Note JSON Variant Matrix Inputs need to be Transposed
		etrading::VariantMatrix results1 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "BASISSWAPS",	transpose(basisData) );
	
		// JSON Variant Matrix Results are Transposed
		results1 = transpose( results1 );

		// Expecting Empty Data Results			
		EXPECT_EQ( 1, results1.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_XccyCurve_DisableAllData )
	{
		// FXFWDS Basis Columns: Tenor, Rate, Use
		const etrading::VariantMatrix fxFwdData		= {	{"ON", 0.2400, "FALSE"},
														{"TN", 0.2500, "FALSE"},
														{"1W", 1.7300, "FALSE"} };

		// BASISSWAPS Basis Columns: Tenor, Rate, Use
		const etrading::VariantMatrix xccyData		= {	{"1Y", -0.0010, "FALSE"},
														{"2Y", -0.0011, "FALSE"},
														{"3Y", -0.0012, "FALSE"} };

		// Check Market Data Dimensions and Remove Unused Data
		// Note JSON Variant Matrix Inputs need to be Transposed
		etrading::VariantMatrix results1 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "FXFWDS",	transpose(fxFwdData) );
		etrading::VariantMatrix results2 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "XCCYSWAPS",	transpose(xccyData) );
	
		// JSON Variant Matrix Results are Transposed
		results1 = transpose( results1 );
		results2 = transpose( results2 );

		// Expecting Empty Data Results			
		EXPECT_EQ( 0, results1.size() );
		EXPECT_EQ( 0, results2.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_XccyCurve_SingleDataPoint )
	{
		// FXFWDS Basis Columns: Tenor, Rate, Use
		const etrading::VariantMatrix fxFwdData		= {	{"ON", 0.2400, "TRUE"},
														{"TN", 0.2500, "FALSE"},
														{"1W", 1.7300, "FALSE"} };

		// BASISSWAPS Basis Columns: Tenor, Rate, Use
		const etrading::VariantMatrix xccyData		= {	{"1Y", -0.0010, "TRUE"},
														{"2Y", -0.0011, "FALSE"},
														{"3Y", -0.0012, "FALSE"} };

		// Check Market Data Dimensions and Remove Unused Data
		// Note JSON Variant Matrix Inputs need to be Transposed
		etrading::VariantMatrix results1 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "FXFWDS",	transpose(fxFwdData) );
		etrading::VariantMatrix results2 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "XCCYSWAPS",	transpose(xccyData) );
	
		// JSON Variant Matrix Results are Transposed
		results1 = transpose( results1 );
		results2 = transpose( results2 );

		// Expecting Empty Data Results			
		EXPECT_EQ( 1, results1.size() );
		EXPECT_EQ( 1, results2.size() );
	}

    TEST_F( TestCurveMarketDataUseParameter, UNIT_FXCurve_DisableAllData )
	{
		// FXFWDS_BIDASK Columns: Tenor, Bid, Ask, Use
		const etrading::VariantMatrix fxFwdData		= {	{"ON", 0.2400, 0.2400, "FALSE"},
														{"TN", 0.2500, 0.2500, "FALSE"},
														{"1W", 1.7300, 1.7300, "FALSE"} };

		// Check Market Data Dimensions and Remove Unused Data
		// Note JSON Variant Matrix Inputs need to be Transposed
		etrading::VariantMatrix results1 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "FXFWDS_BIDASK",	transpose(fxFwdData) );
	
		// JSON Variant Matrix Results are Transposed
		results1 = transpose( results1 );

		// Expecting Empty Data Results			
		EXPECT_EQ( 0, results1.size() );
	}

	TEST_F( TestCurveMarketDataUseParameter, UNIT_FXCurve_SingleDataPoint )
	{
		// FXFWDS_BIDASK Columns: Tenor, Bid, Ask, Use
		const etrading::VariantMatrix fxFwdData		= {	{"ON", 0.2400, 0.2400, "TRUE"},
														{"TN", 0.2500, 0.2500, "FALSE"},
														{"1W", 1.7300, 1.7300, "FALSE"} };

		// Check Market Data Dimensions and Remove Unused Data
		// Note JSON Variant Matrix Inputs need to be Transposed
		etrading::VariantMatrix results1 = etrading::CurveMarketData::checkDimensionsAndRemoveUnusedData( "FXFWDS_BIDASK",	transpose(fxFwdData) );
	
		// JSON Variant Matrix Results are Transposed
		results1 = transpose( results1 );

		// Expecting Empty Data Results			
		EXPECT_EQ( 1, results1.size() );
	}
}