 // Curves
#include "tryAqObjects.h"

// API functions
#include "tryAqObjectsInflationPricing.h"
#include "tryAqObjectsSwapCreation.h"

// Helper to extract par rates from a curve
#include "ExtractCurveCalibrationData.h"

// Test Infrastructure
#include "Dependency.h"   // Curve Macros are Here !!!
#include "GetGoogleTestFolder.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"

// etrading
#include "AQLMathDateUtilities.h"
#include "AQLDateScheduleHelpers.h"
#include "AQOUtilities.h"
#include "InflationCurve.h"
#include "ZeroCouponInflationSwap.h"



using etrading::ReadDataFile;

namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1e-8;


    //
    // curve input files
    //
	const std::string TEST_DIR = "ETrading/Inflation/";

	// -------------------------------------------------------------

	// Curve market data files for use with CurveGenerators
	const std::string obj_GBP_OIS_CURVE					= TEST_DIR + "GBP_OIS_CURVE@17.JSON";


	// -------------------------------------------------------------

	// Build Inflation Curve
	const std::string obj_INFLATION_FIXINGS				= TEST_DIR + "UKRPI@1.JSON";
	const std::string obj_INFLATION_CURVE_SINGLE_SWAP	= TEST_DIR + "INFL_GBP_RPI_SINGLE_SWAP@3.JSON";
	const std::string obj_INFLATION_CURVE_MANY_SWAPS	= TEST_DIR + "INFL_GBP_RPI_MANY_SWAPS@113.JSON";
	const std::string obj_INFLATION_CURVE_FIRST_YEAR	= TEST_DIR + "INFL_GBP_RPI_SINGLE_SWAP_FIRST_YEAR@112.JSON";


	// API methods
	const std::string api_ZC_INFLATIONSWAP_CREATE_TEMPLATE	= TEST_DIR + "GBP_SWAP1@79_tryAqObjectsSwapCreateFromGenerator_inputs.csv"; // This template has no maturity date
	const std::string api_ZC_INFLATIONSWAP_PAR_RATE			= TEST_DIR + "tryAqObjectsInflationZCSwapParRate_inputs.csv";

	// Utility functions
	std::vector<AQLDate> calculateLaggedDates( const std::vector<AQLDate>& dates, const std::string& tenor )
	{
		std::string busDayAdjust = "MOD_FOLLOWING";
		std::string calendar = "LNB";
		std::string rollConvention;
		std::vector<AQLDate> laggedDates = etrading::getDateFromTenor( dates, tenor, busDayAdjust, calendar, rollConvention );

		for ( auto& date : laggedDates )
		{
			date.setDay( 1 );
		}
		return laggedDates;
	}

	void checkCalibrationDates( std::vector<AQLDate> expectedDates, const AnyTypeMatrix& calibrationResults )
	{
		for (size_t i = 0; i< expectedDates.size(); i++ )
		{
			const AQLDate expectedDate = expectedDates[ i ];
			const int expectedExcelDate = etrading::AQLDateScheduleHelpers::getExcelDate( expectedDate );

			const int calibrationDate = boost::get<int>( calibrationResults[ i ][0] );

			EXPECT_EQ( expectedExcelDate, calibrationDate ) << "Mismatch in expected calibration date at node " << i ;
		}
	}

	void checkCalibrationValues( const AQLDate& asOfDateWithLag,
								const std::vector<double>& swapMaturitiesInYears,
								const std::shared_ptr<etrading::FixingTable> & fixingTable,
								const std::shared_ptr<etrading::InflationCurve>& inflationCurve,
								const AnyTypeMatrix& calibrationResults )
	{
		const size_t nCalibrationPoints = calibrationResults.size();
		ASSERT_TRUE( nCalibrationPoints > 1 );

		// 1. Check first calibration point in the curve

		const double fixingAtAsOfDateWithLag = fixingTable->getFixingValue(etrading::toGregorianDateFromAQLDate(asOfDateWithLag));

		const double calibratedValueAtFrontOfCurve = boost::get<double>( calibrationResults[0][1] );

		EXPECT_NEAR( fixingAtAsOfDateWithLag, calibratedValueAtFrontOfCurve, tolerance );


		/* An exact check on calibrated CPI values:
		*  From Kerkhof, For a simple ZC InflationSwap at par, this relationship holds: I_t = I_0 ( 1 + b )^t
		*  where b is the break-even par rate
		*/

		std::vector<etrading::ZCInflationSwapMarketData> zcInflationSwapQuotes = inflationCurve->getZCInflationSwapInputMarketData();
		const size_t nMarketDataQuotes = zcInflationSwapQuotes.size();

		ASSERT_TRUE( zcInflationSwapQuotes.size() > 0 );

		EXPECT_EQ( nCalibrationPoints, nMarketDataQuotes+1 );

		//std::cout << std::setprecision(8);
		
		for ( size_t i=0; i < nMarketDataQuotes; i++ )
		{
			etrading::ZCInflationSwapMarketData zcInflationSwapMarketData = zcInflationSwapQuotes[ i ];
			const double zcInflationSwapBreakEven = zcInflationSwapMarketData.instrumentQuote;

			const double maturityYears = swapMaturitiesInYears[ i ];
			const double expectedInflationLevel	= fixingAtAsOfDateWithLag * std::pow( ( 1 + zcInflationSwapBreakEven ), maturityYears );

			const double calibratedValue = boost::get<double>( calibrationResults[ i+1 ][ 1 ]);

			/*
			std::cout	<< "t: " << maturityYears 
						<< " breakEven: " << zcInflationSwapBreakEven
						<< " expectedCPI: " << expectedInflationLevel 
						<< " calib: " << calibratedValue
						<< " diff: " << ( expectedInflationLevel - calibratedValue)
						<< std::endl;
						*/

			EXPECT_NEAR( expectedInflationLevel, calibratedValue, tolerance );
		}
	}

	void checkCurveCalibrationParameters( const size_t nExpectedRows, const std::string& inflationFixingsHandle, const std::string& inflationCurveHandle )
	{

		AnyTypeMatrix results = validation::tryAqObjectsInflationCurveCalibrationParameters( inflationCurveHandle );

		const size_t nRows = results.size();
		EXPECT_EQ(nExpectedRows, nRows) << "#Error: Expected " << nExpectedRows << " rows of calibration results";  // One row per calibration point

		const size_t nCols = results[0].size();
		EXPECT_EQ(nCols, 2) << "#Error: Expected 2 columns of calibration results";  // CPI date, CPI level

		// Get the curve object so we can directly invoke methods that are not exposed via validation api
		auto inflationCurve = etrading::getInflationCurve( inflationCurveHandle );
		AQLDate asOfDate = inflationCurve->getAsOfDate();

		std::vector<AQLDate> dates = { asOfDate };

		// Get market data quotes
		std::vector<etrading::ZCInflationSwapMarketData> zcInflationSwapQuotes = inflationCurve->getZCInflationSwapInputMarketData();
		const size_t nMarketDataQuotes = zcInflationSwapQuotes.size();

		std::vector<double> swapMaturitiesInYears;
		for (auto swapQuote : zcInflationSwapQuotes)
		{
			std::string quoteTenor = swapQuote.tenorOrDate;
			std::string maturityStr = quoteTenor.substr(0, quoteTenor.size() - 1);
			char * pFirstNonNumber;
            const double maturityYears = std::strtod( maturityStr.c_str(), &pFirstNonNumber );
			swapMaturitiesInYears.push_back( maturityYears );

			//std::cout << "Tenor: " << quoteTenor << " " << tenorMaturityYear << std::endl;

			AQLDate swapMaturity = asOfDate;
			swapMaturity.addYears( maturityYears );
			dates.push_back( swapMaturity );
		}

		const std::string lagTenor = "-2M";
		const std::vector<AQLDate> laggedDates = calculateLaggedDates(dates, lagTenor);

		checkCalibrationDates( laggedDates, results );

		// Check calibration values are consistent with the input market data and fixings
		const AQLDate asOfDateWithLag = laggedDates[0];
		auto fixingTable = etrading::getFixingTable( inflationFixingsHandle );

		checkCalibrationValues( asOfDateWithLag, swapMaturitiesInYears, fixingTable, inflationCurve, results );
	}

}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestInflationCurve);

	//----------------------------------------------------------------------------------------

	TEST_F(TestInflationCurve, TestCalibrationSingleInstrument )
    {
		auto loadGBPOIS				= validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_GBP_OIS_CURVE, etrading::JSON);
		auto inflationFixingsHandle = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_INFLATION_FIXINGS, etrading::JSON);
		auto inflationCurveHandle	= validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_INFLATION_CURVE_SINGLE_SWAP, etrading::JSON);

		/* Check the calibration points. We expect two points in total:
			1. A point corresponding to the asOf date -2M lag, obtained from the fixing table
			2. A point in 1 year -2M, corresponding to the single ZeroCouponInflationSwap calibration instrument
		*/
		const size_t nExpectedRows = 2;

		checkCurveCalibrationParameters( nExpectedRows, inflationFixingsHandle, inflationCurveHandle );
	}

	TEST_F(TestInflationCurve, TestCalibrationManyInstruments)
	{
		auto loadGBPOIS = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_GBP_OIS_CURVE, etrading::JSON);
		auto inflationFixingsHandle = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_INFLATION_FIXINGS, etrading::JSON);
		auto inflationCurveHandle	= validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_INFLATION_CURVE_MANY_SWAPS, etrading::JSON);

		/* Check the calibration points. We expect 18 points in total:
			1. A point corresponding to the asOf date -2M lag, obtained from the fixing table
			2. A point for each of 17 ZeroCouponInflationSwap calibration instruments
		*/
		const size_t nExpectedRows = 18;
		checkCurveCalibrationParameters( nExpectedRows, inflationFixingsHandle, inflationCurveHandle );
	}

	TEST_F(TestInflationCurve, TestCalibrationFirstYear)
	{
		auto loadGBPOIS				= validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_GBP_OIS_CURVE, etrading::JSON);
		auto inflationFixingsHandle = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_INFLATION_FIXINGS, etrading::JSON);
		auto inflationCurveHandle	= validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_INFLATION_CURVE_FIRST_YEAR, etrading::JSON);

		/* Check the calibration points. We expect 13 points in total:
			1. A point corresponding to the asOf date -2M lag, obtained from the fixing table
			2. 11 "First year" CPI points: 1M, 2M, ... 11M
			3. A point in 1 year -2M, corresponding to the single ZeroCouponInflationSwap calibration instrument
		*/
		AnyTypeMatrix results = validation::tryAqObjectsInflationCurveCalibrationParameters( inflationCurveHandle );

		const size_t nExpectedRows = 13;
		const size_t nRows = results.size();
		EXPECT_EQ(nExpectedRows, nRows) << "#Error: Expected " << nExpectedRows << " rows of calibration results";  // One row per calibration point

		// Check first year calibration pillar dates
		auto inflationCurve = etrading::getInflationCurve( inflationCurveHandle );
		AQLDate asOfDate = inflationCurve->getAsOfDate();

		std::vector<AQLDate> dates = { asOfDate };
		AQLDate currentDate = asOfDate;
		for (size_t i=1; i<nExpectedRows; i++ )
		{
			currentDate.addMonths( 1 );
			dates.push_back( currentDate );
		}
		const std::string lagTenor = "-2M";
		std::vector<AQLDate> laggedDates = calculateLaggedDates(dates, lagTenor );

		checkCalibrationDates( laggedDates, results );

		// TODO: Check the first-year monthly *values*

	}

	TEST_F(TestInflationCurve, TestZCInflationSwapRepricing)
	{
		auto loadGBPOIS = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_GBP_OIS_CURVE, etrading::JSON);
		auto inflationFixingsHandle = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_INFLATION_FIXINGS, etrading::JSON);
		auto inflationCurveHandle = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_INFLATION_CURVE_MANY_SWAPS, etrading::JSON);

		// Parameters for creating a ZC Inflation Swap
		etrading::ReadDataFile::Load createSwap = etrading::ReadDataFile::Load( api_ZC_INFLATIONSWAP_CREATE_TEMPLATE );
		const std::string swapGeneratorName		= createSwap["swapGeneratorName"];
		const AQLStringMatrix expressionMat		= createSwap["expressionLVB"];
		const AQLStringMatrix swapPropertiesMat	= createSwap["swapPropertiesLVB"];
		const bool isXccy						= false;
		const bool validateKeys					= true;

		LabelValueBlock expressionTemplateLVB( expressionMat );
		LabelValueBlock swapPropertiesLVB( swapPropertiesMat );

		// Parameters for invoking parRate
		etrading::ReadDataFile::Load parRateParams = etrading::ReadDataFile::Load(api_ZC_INFLATIONSWAP_PAR_RATE);
		const AQLStringMatrix valuationMat = parRateParams[ "valuationSettingsLVB" ];
		LabelValueBlock valuationSettingsLVB( valuationMat );

		// Get market data quotes
		auto inflationCurve = etrading::getInflationCurve( inflationCurveHandle);
		AQLDate asOfDate = inflationCurve->getAsOfDate();
		std::vector<etrading::ZCInflationSwapMarketData> zcInflationSwapQuotes = inflationCurve->getZCInflationSwapInputMarketData();
		const size_t nMarketDataQuotes = zcInflationSwapQuotes.size();

		std::vector<double> swapMaturitiesInYears;
		for (auto swapQuote : zcInflationSwapQuotes)
		{
			std::string quoteTenor = swapQuote.tenorOrDate;
			std::string maturityStr = quoteTenor.substr(0, quoteTenor.size() - 1);
			char * pFirstNonNumber;
            const double maturityYears = std::strtod(maturityStr.c_str(), &pFirstNonNumber);
			swapMaturitiesInYears.push_back(maturityYears);

			AQLDate swapMaturity = asOfDate;
			swapMaturity.addYears(maturityYears);
			const int maturityExcelDate = etrading::AQLDateScheduleHelpers::getExcelDate( swapMaturity );
			std::string maturityString = std::to_string( maturityExcelDate );

			LabelValueBlock expressionLVB( expressionTemplateLVB, "MATURITYDATE", maturityString );
			auto swapHandle = validation::tryAqObjectsSwapCreateFromGenerator( "dummySwapName", swapGeneratorName, expressionLVB, swapPropertiesLVB, isXccy, validateKeys );

			const double calculatedParRate = validation::tryAqObjectsInflationZCSwapParRate( swapHandle, inflationCurveHandle, valuationSettingsLVB );
			EXPECT_NEAR( swapQuote.instrumentQuote, calculatedParRate, tolerance );

		}
	}

	TEST_F( TestInflationCurve, TestAqObjectsInflationCPI_OnPillarDates )
	{
		auto loadGBPOIS = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_GBP_OIS_CURVE, etrading::JSON);
		auto inflationFixingsHandle = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_INFLATION_FIXINGS, etrading::JSON);
		auto inflationCurveHandle = validation::tryAqObjectsLoad(etrading::getGoogleTestFolder() + obj_INFLATION_CURVE_MANY_SWAPS, etrading::JSON);

		AnyTypeMatrix calibrationResults = validation::tryAqObjectsInflationCurveCalibrationParameters(inflationCurveHandle);
		
		/* Check the calibration points. We expect 18 points in total:
		1. A point corresponding to the asOf date -2M lag, obtained from the fixing table
		2. A point for each of 17 ZeroCouponInflationSwap calibration instruments
	*/
		const size_t nExpectedRows = 18;
		const size_t nRows = calibrationResults.size();
		EXPECT_EQ(nExpectedRows, nRows) << "#Error: Expected " << nExpectedRows << " rows of calibration results";  // One row per calibration point

		const size_t nCols = calibrationResults[0].size();
		EXPECT_EQ(nCols, 2) << "#Error: Expected 2 columns of calibration results";  // CPI date, CPI level

		// Get the curve object so we can directly invoke methods that are not exposed via validation api
		auto inflationCurve = etrading::getInflationCurve( inflationCurveHandle );
		AQLDate asOfDate = inflationCurve->getAsOfDate();

		// Test going directly to inflation curve with lagged date
		for ( size_t i=0; i<nExpectedRows; i++)
		{
			const double calibrationValue	= boost::get<double>( calibrationResults[i][1] );
			const int calibrationExcelDate	= boost::get<int>( calibrationResults[i][0] );
			const AQLDate laggedDate = AQLMathDateUtilities::getAQLDate( calibrationExcelDate );

			const double interpolatedValue = inflationCurve->getMonthlyInflationIndexForLaggedDate( laggedDate );

			EXPECT_NEAR( calibrationValue, interpolatedValue, tolerance );
		}

		// Test going to inflation curve, but with non-lagged date

		// First produce list of market data dates i.e. pillarDates
		std::vector<etrading::ZCInflationSwapMarketData> zcInflationSwapQuotes = inflationCurve->getZCInflationSwapInputMarketData();
		const size_t nMarketDataQuotes = zcInflationSwapQuotes.size();

		std::vector<AQLDate> pillarDates = { asOfDate };
		for ( auto swapQuote : zcInflationSwapQuotes)
		{
			std::string quoteTenor = swapQuote.tenorOrDate;
			std::string maturityStr = quoteTenor.substr(0, quoteTenor.size() - 1);
			char * pFirstNonNumber;
            const double maturityYears = std::strtod(maturityStr.c_str(), &pFirstNonNumber);

			AQLDate swapMaturity = asOfDate;
			swapMaturity.addYears( maturityYears );
			pillarDates.push_back( swapMaturity );
		}

		// Now check the interpolated value from curve on each pillar date
		const std::string fixLag = "2M";
		const etrading::InflationResetTypeEnum inflationResetTypeEnum = etrading::INFLATION_RESET_TYPE_MONTHLY_INTERPOLATION;
		for (size_t i = 0; i < nExpectedRows; i++)
		{
			const double calibrationValue = boost::get<double>( calibrationResults[i][1] );

			const AQLDate pillarDate = pillarDates[i];
			const double interpolatedValue = inflationCurve->getInflationIndexForDate( pillarDate, inflationResetTypeEnum, fixLag );

			EXPECT_NEAR(calibrationValue, interpolatedValue, tolerance);
		}

		// Now check via validation
		std::string inflationResetType = etrading::toString( inflationResetTypeEnum );
		for (size_t i = 0; i < nExpectedRows; i++)
		{
			const double calibrationValue = boost::get<double>(calibrationResults[i][1]);

			const AQLDate pillarDate = pillarDates[i];
			const double interpolatedValue = validation::tryAqObjectsInflationCPI( inflationCurveHandle, pillarDate, inflationResetType, fixLag );
				
			EXPECT_NEAR( calibrationValue, interpolatedValue, tolerance );
		}

		// Check with different day of month - using MONTHLY_INTERPOLATION so should have no effect.
		for (size_t i = 0; i < nExpectedRows; i++)
		{
			const double calibrationValue = boost::get<double>(calibrationResults[i][1]);

			AQLDate pillarDate = pillarDates[i];
			pillarDate.setDay(1);
			const double interpolatedValue = inflationCurve->getInflationIndexForDate(pillarDate, inflationResetTypeEnum, fixLag);

			EXPECT_NEAR(calibrationValue, interpolatedValue, tolerance);
		}

	}

	/*
	TEST_F( TestInflationCurve, TestSeasonality )
	{
	}

	TEST_F( TestInflationCurve, TestGetCPI )
	{

		// Check aqObjectsInflationCPI
		// - on anchor
		// - on 1yr point and compare against calibration points
		// - Check with different days of month
		// - Check in between points - linear interp in logCPI
		// - check extrapolation flat.
	}
		
		*/

	

}