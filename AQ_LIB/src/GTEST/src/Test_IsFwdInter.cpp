#include "Dependency.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "TestHelperUtilities.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "tryMirGetDate.h"
#include "tryMirGetParRate.h"
#include "YieldCurveUtil.h"
#include "LADateScheduleHelpers.h"
#include "InitializeETrading.h"
#include "tryMeUtilitySetup.h"
#include <sstream>

using etrading::ReadDataFile;

#define TEST_DIR "Vanilla/Curves/Test_IsFwdInter/"

namespace
{    
	// test tolerance
    const double tolerance = 1e-8;

	const char* TENORS [] = {"3M", "4M", "5M", "6M", "7M", "8M", "9M", "10M", "11M", "1Y", "1Y2M", "1Y4M", "1Y6M", "1Y9M", "2Y", "2Y3M", "2Y6M", "2Y8M", "2Y9M", 
	"3Y", "3Y1M", "3Y5M", "3Y7M", "4Y", "4Y3M", "4Y6M", "4Y8M", "4Y11M", "5Y", "5Y3M", "5Y6M", "5Y9M", "6Y", "6Y3M", "6Y5M", "6Y10M", "7Y", "7Y3M", "7Y6M", "7Y9M", "8Y6M", "9Y", "9Y6M",
	"10Y", "10Y6M", "11Y", "11Y6M", "12Y", "12Y6M", "13Y", "13Y6M", "14Y", "15Y", "16Y", "17Y", "18Y", "19Y", "20Y"};

	const char  OIS_CURVE_FILE[] = "Setup_OIS_Inputs.csv";
	
	const char* STD_FILES[]		 = {"Setup_STD_Inputs_Spline.csv", 
									"Setup_STD_Inputs_Linear.csv", 
									"Setup_STD_Inputs_ConstrainedSpline.csv", 
									"Setup_STD_Inputs_MonotoneConvex.csv"};

	const char outputsFile_32[]		= "outputs_32bit.csv";
    const char outputsFile_64[]		= "outputs_64bit.csv";
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(Test_IsFwdInter);

    TEST_F( Test_IsFwdInter, SNAPSHOT_CheckParRates)
	{		        
		std::vector<std::pair<AQLString, double> > outputs;

		for (size_t i = 0; i < sizeof(STD_FILES) / sizeof(char*); ++i)
		{
			//----------------------------------------------------------------------------------------
			// Build yield curves of the current test case
			
			AQLString forecastFileDir = TEST_DIR + AQLString(STD_FILES[i]);
			AQLString discountFileDir = TEST_DIR + AQLString(OIS_CURVE_FILE);
			SET_UP_STD_CURVE(discountFileDir, forecastFileDir);
						
			//----------------------------------------------------------------------------------------
			// Retrieve data from STD curve file

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

			// swap convention block
			etrading::ReadDataFile swapConv = inputFile_STD["swapConv"];			
			const std::set<AQLString>& swapConvkeys = swapConv.getKeys();

			// Spot lag
			AQLString spotLag;
			findValByKey(spotLag, swapConv, swapConvkeys, "ResetLag");
			if (spotLag.findString("D") == -1)
			{
				spotLag += "D";
			}

			// Frequency fixed leg
			AQLString frequencyFixed;
			findValByKey(frequencyFixed, swapConv, swapConvkeys, "Frequency");

			// Frequency float leg
			AQLString frequencyFloat;
			findValByKey(frequencyFloat, swapConv, swapConvkeys, "FrequencyFloat");

			// Day count convention fixed leg
			AQLString dayCountFixed;
			findValByKey(dayCountFixed, swapConv, swapConvkeys, "DayCount");

			// Day count convention float leg
			AQLString dayCountFloat;
			findValByKey(dayCountFloat, swapConv, swapConvkeys, "DayCountFloat");

			// Roll convention
			AQLString rollConvention;
			findValByKey(rollConvention, swapConv, swapConvkeys, "SlidingRule");

			// Calendar
			AQLString calendar;
			findValByKey(calendar, swapConv, swapConvkeys, "Calendar");

			// Effective date
			AQLString asofDateStr;
			findValByKey(asofDateStr, generateProp, generatePropkeys, "AsOfDate");
			AQLDate asofDate = etrading::LADateScheduleHelpers::getLADate(asofDateStr);
			AQLDate effectiveDate = etrading::LADateScheduleHelpers::getDate( asofDate, spotLag, rollConvention, calendar );

			// Curve collection and forecast curve
			AQLString curveCollection = etrading::getCurveID( inputFile_STD );
			AQLString foreCurve	 = etrading::getMarketName( inputFile_STD );

			// Discount curve
			AQLString dfCurve("OIS");
			findValByKey(dfCurve, generateProp, generatePropkeys, "dfcurvename");

			// Fixing lag
			AQLString fixingLag("0D");
			findValByKey(fixingLag, swapConv, swapConvkeys, "FixingLag", true);

			// Is rolling at end of month?
			AQLString isEomRollStr;
			findValByKey(isEomRollStr, swapConv, swapConvkeys, "IsEomRoll");
			isEomRollStr.toUpper();
			
			bool isEomRoll(false);
			if (isEomRollStr == "TRUE")
			{
				isEomRoll = true;
			}

			// Interpolation
			AQLString interpolation;
			findValByKey(interpolation, swapConv, swapConvkeys, "Interpolation");
			interpolation = interpolationShortName(interpolation);

			//----------------------------------------------------------------------------------------
			// Check pricing consistency on all swap tenors
			
			for (size_t j = 0; j < sizeof(TENORS) / sizeof(char*); ++j)
			{
				AQLString maturityTenor = AQLString(TENORS[j]);
				AQLDate maturity = etrading::LADateScheduleHelpers::getDate(effectiveDate, maturityTenor, "", "");	// Maturity date must not be adjusted first

				// Case 1: IsFwdInter = FALSE, UseFwdData = FALSE
				double parRate = validation::tryMirGetParRate4(etrading::InitializeETrading::instance().dataInstance(),
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
																   false,	// interpFwds_,
																   isEomRoll,
																   0.0,		// tSpd_,
																   false,	// useFwdData_ 
																   calendar	// fixing calendar
																   );

				AQLString key = interpolation + "_" + maturityTenor + "_" + "IsFwdInter_FALSE_UseFwdData_FALSE";
				outputs.push_back(std::make_pair(key, parRate));

				// Case 2: IsFwdInter = TRUE, UseFwdData = FALSE
				parRate = validation::tryMirGetParRate4(
																   etrading::InitializeETrading::instance().dataInstance(),
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
																   false,	// useFwdData_ 
																   calendar	// fixing calendar
																   );

				key = interpolation + "_" + maturityTenor + "_" + "IsFwdInter_TRUE_UseFwdData_FALSE";
				outputs.push_back(std::make_pair(key, parRate));

				// Case 3: IsFwdInter = TRUE, UseFwdData = TRUE

				const bool IS_FWD_INTER = true;
				parRate = validation::tryMirGetParRate4(
																   etrading::InitializeETrading::instance().dataInstance(),
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
																   IS_FWD_INTER,	// interpFwds_,
																   isEomRoll,
																   0.0,		// tSpd_,
																   IS_FWD_INTER,	// useFwdData_ 
																   calendar	// fixing calendar
																   );

				key = interpolation + "_" + maturityTenor + "_" + "IsFwdInter_TRUE_UseFwdData_TRUE";
				outputs.push_back(std::make_pair(key, parRate));
			}

			// Flush the cache in preparation for a new set of curves
			validation::tryMeUtilityClearEntityPool();
		}

		if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
#ifdef GTEST32
            AQLString outputFileName = outputsFile_32;
#else
            AQLString outputFileName = outputsFile_64;
#endif

            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
            etrading::CreateDataFile file( etrading::decorateFilename( outputFileName ) );
            for ( size_t i = 0; i < outputs.size(); ++i )
            {
                // 12 is the number of decimal points required
                file.write( outputs[i].first, outputs[i].second, 12 );
            }
        }
        else
        {
            // Carry out actual test and peform result comparison
#ifdef GTEST32
            const ReadDataFile::Load resultFile( TEST_DIR + AQLString(outputsFile_32) );
#else
            const ReadDataFile::Load resultFile( TEST_DIR + AQLString(outputsFile_64) );
#endif

            for ( size_t i = 0; i < outputs.size(); ++i )
            {
                AQLString key	= outputs[i].first;
				double parRate	= outputs[i].second;

                double ref = resultFile[key];

                EXPECT_NEAR( parRate, ref, tolerance )
                            << " Par rate for key " << key.getCString() << " is incorrect ";
            }
        }
    }
	
}
