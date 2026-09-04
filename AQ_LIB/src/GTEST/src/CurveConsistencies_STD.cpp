#include "Dependency.h"
#include "ReadDataFile.h"
#include "TestHelperUtilities.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "tryMirGetDF.h"
#include "tryMirGetDate.h"
#include "tryMirGetParRate.h"
#include "YieldCurveUtil.h"
#include "AQLDateScheduleHelpers.h"
#include "InitializeETrading.h"
#include "tryAqToolsSetup.h"
#include <sstream>

using etrading::ReadDataFile;

#define TEST_DIR			  "Vanilla/CurveConsistencies/STD/"
#define TEST_DIR_LINEARSPLINE "Vanilla/CurveConsistencies/STD_LinearSpline/"

namespace
{    
	// test tolerance
    const double tolerance  = 1e-7;
	const double tolerance2 = 1e-6;

	// Test count
	const size_t EUR_TEST_COUNT = 5;
	const size_t USD_TEST_COUNT = 5;
	const size_t GBP_TEST_COUNT = 5;
	const size_t JPY_TEST_COUNT = 4;
	const size_t AUD_TEST_COUNT = 0;
	const size_t LINEARSPLINE_TEST_COUNT = 9;
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(CurveConsistencies_STD);

	/* @brief	Run consistency test on all available swap curves in all ccys
    */
    void swapCurveConsistencyCheck(const AQLString& ccy, const AQLString& testDir)
	{
		AQLString prefix("");
		if (ccy != "LINEARSPLINE")
		{
			prefix = ccy + AQLString("_");
		}

		size_t TEST_COUNT = 0;
		if (ccy == "USD")
		{
			TEST_COUNT = USD_TEST_COUNT;
		}
		else if (ccy == "EUR")
		{
			TEST_COUNT = EUR_TEST_COUNT;
		}
		else if (ccy == "JPY")
		{
			TEST_COUNT = JPY_TEST_COUNT;
		}
		else if (ccy == "GBP")
		{
			TEST_COUNT = GBP_TEST_COUNT;
		}
		else if (ccy == "AUD")
		{
			TEST_COUNT = AUD_TEST_COUNT;
		}
		else if (ccy == "LINEARSPLINE")
		{
			TEST_COUNT = LINEARSPLINE_TEST_COUNT;
		}
        
		for (size_t i = 0; i < TEST_COUNT; ++i)
		{
			//----------------------------------------------------------------------------------------
			// Build yield curves of the current test case

			AQLString forecastCurveFile = prefix + AQLString("STD_") + AQLString(static_cast<int>(i + 1));
			AQLString discountCurveFile = prefix + AQLString("OIS_") + AQLString(static_cast<int>(i + 1));
						
			AQLString forecastFileDir = testDir + forecastCurveFile + AQLString(".csv");
			AQLString discountFileDir = testDir + discountCurveFile + AQLString(".csv");
			SET_UP_STD_CURVE(discountFileDir, forecastFileDir);

			//----------------------------------------------------------------------------------------
			// Retrieve blocks of data from STD curve file

			const ReadDataFile::Load inputFile_STD( forecastFileDir );

			// generateProp block
			etrading::ReadDataFile generateProp;
			try
			{
				generateProp = inputFile_STD["generateProp"];			
			}
			catch (const std::exception&)
			{
				generateProp = inputFile_STD["generalProps"];			
			}
			const std::set<AQLString>& generatePropkeys = generateProp.getKeys();

			// swap par rates block
			etrading::ReadDataFile swapRates = inputFile_STD["swapRates"];			
			const std::set<AQLString>& swapRatekeys = swapRates.getKeys();

			// swap convention block
			etrading::ReadDataFile swapConv = inputFile_STD["swapConv"];			
			const std::set<AQLString>& swapConvkeys = swapConv.getKeys();
			
			//----------------------------------------------------------------------------------------
			// Retrieve all parameters required from various data blocks. 
			// Should any of these parameters be not available, skip this test as it's not a valid test
			
			// Discount curve
			AQLString dfCurve("OIS");
			findValByKey(dfCurve, generateProp, generatePropkeys, "dfcurvename");

			// Calendar
			AQLString calendar;
			findValByKey(calendar, swapConv, swapConvkeys, "Calendar");

			// Spot lag
			AQLString spotLag;
			findValByKey(spotLag, swapConv, swapConvkeys, "ResetLag");
			if (spotLag.findString("D") == -1)
			{
				spotLag += "D";
			}

			// Day count convention fixed leg
			AQLString dayCountFixed;
			findValByKey(dayCountFixed, swapConv, swapConvkeys, "DayCount");

			// Day count convention float leg
			AQLString dayCountFloat;
			findValByKey(dayCountFloat, swapConv, swapConvkeys, "DayCountFloat");

			// Frequency fixed leg
			AQLString frequencyFixed;
			findValByKey(frequencyFixed, swapConv, swapConvkeys, "Frequency");

			// Frequency float leg
			AQLString frequencyFloat;
			findValByKey(frequencyFloat, swapConv, swapConvkeys, "FrequencyFloat");

			// Roll convention
			AQLString rollConvention;
			findValByKey(rollConvention, swapConv, swapConvkeys, "SlidingRule");

			// Fixing lag
			AQLString fixingLag("0D");
			findValByKey(fixingLag, swapConv, swapConvkeys, "FixingLag", true);

			// Interpolation
			AQLString interpolation;
			findValByKey(interpolation, generateProp, generatePropkeys, "yieldgen.interpolation");
			interpolation = interpolationShortName(interpolation);

			// Is rolling at end of month?
			AQLString isEomRollStr;
			findValByKey(isEomRollStr, swapConv, swapConvkeys, "IsEomRoll");
			isEomRollStr.toUpper();
			
			bool isEomRoll(false);
			if (isEomRollStr == "TRUE")
			{
				isEomRoll = true;
			}

			// Effective date
			AQLString asofDateStr;
			findValByKey(asofDateStr, generateProp, generatePropkeys, "AsOfDate");
			AQLDate asofDate = etrading::AQLDateScheduleHelpers::getAQLDate(asofDateStr);
			AQLDate effectiveDate = etrading::AQLDateScheduleHelpers::getDate( asofDate, spotLag, rollConvention, calendar );

			// Curve collection and forecast curve
			AQLString curveCollection = etrading::getCurveID( inputFile_STD );
			AQLString foreCurve	 = etrading::getMarketName( inputFile_STD );

			// Forcibly setting fixing lag to 0D until development has completed in supporting fixing lag in curve building
			fixingLag = "0D";
			
			// Find the largest tenor
			std::vector<int> tempMaturityVec;
			for(auto iterator = swapRatekeys.begin(); iterator != swapRatekeys.end(); ++iterator)
			{
				AQLString maturityTenor = *iterator;
                char * pFirstNonNumber;
				int amount = strtol(maturityTenor.subString(0, maturityTenor.size() - 1).getCString(), &pFirstNonNumber, 10); // base 10 numbers
				tempMaturityVec.push_back(amount);
			}
			std::sort(tempMaturityVec.begin(), tempMaturityVec.end());
			int maxTenor = tempMaturityVec.back();
			
			//----------------------------------------------------------------------------------------
			// Check pricing consistency on all swap tenors
			for(auto iterator = swapRatekeys.begin(); iterator != swapRatekeys.end(); ++iterator)
			{
				AQLString maturityTenor = *iterator;
                char * pFirstNonNumber;
                int amount = strtol(maturityTenor.subString(0, maturityTenor.size() - 1).getCString(), &pFirstNonNumber, 10); // base 10 numbers
				AQLDate maturity = etrading::AQLDateScheduleHelpers::getDate(effectiveDate, maturityTenor, "", "");	// Maturity date must not be adjusted first
				
                const double calcParRate = validation::tryMirGetParRate4( etrading::InitializeETrading::instance().dataInstance(),
																               effectiveDate.stringWithFormat( "YYYYMMDD" ),
																               maturity.stringWithFormat("YYYYMMDD"),
																               curveCollection,
																               frequencyFixed,
																               dayCountFixed,
																               rollConvention,
																               calendar,
																               "",		// xFirstStub_,
																               "",		// xLastStub_,
																               "",		// xRollDay_,
																               "",		// xPayLag_,
																               "NONE",	// xStub_,
																               frequencyFloat,
																               dayCountFloat,
																               rollConvention,
																               calendar,
																               "",		// tFirstStub_,
																               "",		// tLastStub_,
																               "",		// tRollDay_,
																               fixingLag,
																               0.0,		// tFirstFix_,
																               0.0,		// tLastFix_,
																               "",		// tPayLag_,
																               "NONE",	// tStub_,
																               interpolation,
																               foreCurve,
																               dfCurve,
																               true,	// interpFwds_,
																               isEomRoll,
																               0.0,		// tSpd_,
																               true,	// useFwdData_ 
																               calendar	// fixing calendar
																               );

				AQLString inputParRateStr; 
				findValByKey(inputParRateStr, swapRates, swapRatekeys, maturityTenor);
				
				double inputParRate(0.0);
				std::stringstream(inputParRateStr.getCString()) >> inputParRate;

				double diff = std::fabs(inputParRate - calcParRate);
				if (amount == maxTenor)
				{
					if (diff > tolerance2)
					{
						AQLString err = "#Err: Test curve '" + forecastCurveFile + "' has error at " + maturityTenor;
						throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__); 
					}	
				}
				else
				{
					if (diff > tolerance)
					{
						AQLString err = "#Err: Test curve '" + forecastCurveFile + "' has error at " + maturityTenor;
						throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__); 
					}
				}
			}

			// Flush the cache in preparation for a new set of curves
			validation::tryAqToolsClearEntityPool();
		}
    }

	TEST_F( CurveConsistencies_STD, CONSISTENCY_USD_Calibration_Instrument_Repricing)
    {
        swapCurveConsistencyCheck("USD", TEST_DIR);
	}

	TEST_F( CurveConsistencies_STD, CONSISTENCY_EUR_Calibration_Instrument_Repricing)
    {
        swapCurveConsistencyCheck("EUR", TEST_DIR);
	}

	TEST_F( CurveConsistencies_STD, CONSISTENCY_AUD_Calibration_Instrument_Repricing)
    {
        swapCurveConsistencyCheck("AUD", TEST_DIR);
	}

	TEST_F( CurveConsistencies_STD, CONSISTENCY_JPY_Calibration_Instrument_Repricing)
    {
        swapCurveConsistencyCheck("JPY", TEST_DIR);
	}

	TEST_F( CurveConsistencies_STD, CONSISTENCY_GBP_Calibration_Instrument_Repricing)
    {
        swapCurveConsistencyCheck("GBP", TEST_DIR);
	}

	TEST_F( CurveConsistencies_STD, CONSISTENCY_LINEAR_SPLINE_Calibration_Instrument_Repricing)
    {
        swapCurveConsistencyCheck("LINEARSPLINE", TEST_DIR_LINEARSPLINE);
	}
}
