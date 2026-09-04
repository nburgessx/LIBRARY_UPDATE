#include "Dependency.h"
#include "ReadDataFile.h"
#include "TestHelperUtilities.h"
#include "CurveOis.h"
#include "CurveStd.h"
#include "CurveTenorBasis.h"
#include "tryMirGetDF.h"
#include "tryMirGetDate.h"
#include "tryMirGetParRate.h"
#include "YieldCurveUtil.h"
#include "AQLDateScheduleHelpers.h"
#include "InitializeETrading.h"
#include "tryAqToolsSetup.h"
#include "ExceptionMacros.h"
#include <sstream>

using etrading::ReadDataFile;

#define TEST_DIR			  "Vanilla/CurveConsistencies/TenorBasis/"
#define TEST_DIR_LINEARSPLINE "Vanilla/CurveConsistencies/TenorBasis_LinearSpline/"

namespace
{    
	// test tolerance
    const double tolerance = 1e-8;
	const double linearSplineTolerance = 1e-7;

	// Test count
	const size_t USD_TEST_COUNT = 21;
	const size_t EUR_TEST_COUNT = 2;
	const size_t AUD_TEST_COUNT = 0;
	const size_t JPY_TEST_COUNT = 5;
	const size_t GBP_TEST_COUNT = 6;
	const size_t LINEARSPLINE_TEST_COUNT = 13;
}

namespace google_test
{
	/* @brief	Run consistency test on all available tenor basis curves in all ccys
    */
	void basisCurveConsistencyCheck(const AQLString& ccy, const AQLString& testDir)
	{
		AQLString prefix("");
		if (ccy != "LINEARSPLINE")
		{
			prefix = ccy + AQLString("_");
		}

		size_t TEST_COUNT;
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
			std::cout << "TestCount: " << TEST_COUNT << std::endl;

			//----------------------------------------------------------------------------------------
			// Build yield curves of the current test case

			AQLString swapCurveFile		 = prefix + AQLString("STD_") + AQLString(static_cast<int>(i + 1));
			AQLString oisCurveFile		 = prefix + AQLString("OIS_") + AQLString(static_cast<int>(i + 1));
			AQLString tenorBasisCurveFile = prefix + AQLString("TenorBasis_") + AQLString(static_cast<int>(i + 1));
			
			AQLString swapFileDir	   = testDir + swapCurveFile + AQLString(".csv");
			AQLString oisFileDir		   = testDir + oisCurveFile + AQLString(".csv");
			AQLString tenorBasisFileDir = testDir + tenorBasisCurveFile + AQLString(".csv");
			SET_UP_TENOR_BASIS_CURVE_1(oisFileDir, swapFileDir, tenorBasisFileDir);

			//----------------------------------------------------------------------------------------
			// Retrieve blocks of data from the STD curve file

			const ReadDataFile::Load inputFile_STD( swapFileDir );

			// generateProp block
			etrading::ReadDataFile generateProp_STD;
			try
			{
				generateProp_STD = inputFile_STD["generateProp"];			
			}
			catch (const std::exception&)
			{
				generateProp_STD = inputFile_STD["generalProps"];			
			}
			const std::set<AQLString>& generatePropkeys_STD = generateProp_STD.getKeys();

			//----------------------------------------------------------------------------------------
			// Retrieve blocks of data from the tenor basis curve file

			const ReadDataFile::Load inputFile_TenorBasis( tenorBasisFileDir );

			// generateProp block
			etrading::ReadDataFile generateProp;
			try
			{
				generateProp = inputFile_TenorBasis["generateProp"];			
			}
			catch (const std::exception&)
			{
				generateProp = inputFile_TenorBasis["generalProps"];			
			}
			const std::set<AQLString>& generatePropkeys = generateProp.getKeys();
			
			// basis convention block
			etrading::ReadDataFile basisConv = inputFile_TenorBasis["basisConv"];			
			const std::set<AQLString>& basisConvkeys = basisConv.getKeys();
			
			//----------------------------------------------------------------------------------------
			// Retrieve all parameters required from various data blocks. 
			// Should any of these parameters be not available, skip this test as it's not a valid test
			
			// IsFwdBasis
			AQLString isFwdBasisStr;
			findValByKey(isFwdBasisStr, generateProp, generatePropkeys, "isFwdBasis", true);
			isFwdBasisStr.toUpper();
			bool isFwdBasis = false;
			if (isFwdBasisStr == "TRUE")
			{
				isFwdBasis = true;
			}

			// IsYieldSpreadCalc
			AQLString IsYieldSpreadCalcStr("");
			bool IsYieldSpreadCalc = false;
			findValByKey(IsYieldSpreadCalcStr, basisConv, basisConvkeys, "isyieldspreadcalc", true);
			if (IsYieldSpreadCalcStr.toUpper() == "TRUE")
			{
				IsYieldSpreadCalc = true;
			}

			// IsSameGridIndex
			AQLString IsSameGridIndexStr("");
			bool IsSameGridIndex = true;
			findValByKey(IsSameGridIndexStr, basisConv, basisConvkeys, "IsSameGridIndex", true);
			if (IsSameGridIndexStr.toUpper() == "FALSE")
			{
				IsSameGridIndex = false;
			}

			// Skip all tests where IsSameGridIndex is FALSE until development has completed in supporting fixing lag in curve building
			if (IsSameGridIndex == false)
			{
				continue;
			}

			// IsFwdInterp
			AQLString IsFwdInterStr("");
			bool IsFwdInter = false;
			findValByKey(IsFwdInterStr, basisConv, basisConvkeys, "IsFwdInter", true);
			if (IsFwdInterStr.toUpper() == "TRUE")
			{
				IsFwdInter = true;
			}

			// Target leg
			AQLString targetLeg;
			findValByKey(targetLeg, basisConv, basisConvkeys, "Target");
			bool isLeg1Target = targetLeg.subString(0, 3).toUpper() == "LEG1" ? true : false;

			// Spread leg
			AQLString basisLeg;
			findValByKey(basisLeg, basisConv, basisConvkeys, "IsLeg1Spread");
			bool isLeg1Spread = basisLeg.toUpper() == "TRUE" ? true : false;

			// Against leg calendar
			AQLString a_calendar;
			AQLString a_calendarKey = isLeg1Target ? "Leg2Cashlet.calendar" : "Leg1Cashlet.calendar";
			findValByKey(a_calendar, basisConv, basisConvkeys, a_calendarKey);

			// Against leg date count convention
			AQLString a_dayCount;
			AQLString a_dayCountKey = isLeg1Target ? "Leg2Cashlet.daycount" : "Leg1Cashlet.daycount";
			findValByKey(a_dayCount, basisConv, basisConvkeys, a_dayCountKey);

			// Against leg frequency
			AQLString a_frequency;
			AQLString a_frequencyKey = isLeg1Target ? "Leg2Cashlet.frequency" : "Leg1Cashlet.frequency";
			findValByKey(a_frequency, basisConv, basisConvkeys, a_frequencyKey);

			// Target leg compounding frequency
			AQLString a_compounding_frequency("");
			AQLString a_compoundingFrequencyKey = isLeg1Target ? "Leg2Cashlet.frequencycompound" : "Leg1Cashlet.frequencycompound";
			findValByKey(a_compounding_frequency, basisConv, basisConvkeys, a_compoundingFrequencyKey, true);
			if (a_compounding_frequency.size() != 0 && a_compounding_frequency.toUpper() != a_frequency.toUpper())
			{
				std::cout << "Test file " << tenorBasisCurveFile << " is using compounding frequency. This is currently not supported for testing." << std::endl;
				continue;
			}

			// Against leg roll convention
			AQLString a_rollConvention;
			AQLString a_rollConventionKey = isLeg1Target ? "Leg2Cashlet.slidingrule" : "Leg1Cashlet.slidingrule";
			findValByKey(a_rollConvention, basisConv, basisConvkeys, a_rollConventionKey);

			// Against leg forecast curve
			AQLString a_forecast;
			AQLString a_forecastKey = isLeg1Target ? "Leg2forecast" : "Leg1forecast";
			findValByKey(a_forecast, basisConv, basisConvkeys, a_forecastKey);

			// Against leg discount curve
			AQLString a_discount;
			AQLString a_discountKey = isLeg1Target ? "Leg2discount" : "Leg1discount";
			findValByKey(a_discount, basisConv, basisConvkeys, a_discountKey);

			// Against leg spot lag
			AQLString a_spotLag;
			AQLString a_spotLagKey = isLeg1Target ? "Leg2Cashlet.spotlag" : "Leg1Cashlet.spotlag";
			findValByKey(a_spotLag, basisConv, basisConvkeys, a_spotLagKey);
			if (a_spotLag.findString("D") == -1)
			{
				a_spotLag += "D";
			}

			// Against leg fixing lag
			AQLString a_fixingLag("0D");
			if (!IsSameGridIndex)
			{
				AQLString a_fixingLagKey = isLeg1Target ? "Leg2index.resetlag" : "Leg1index.resetlag";
				findValByKey(a_fixingLag, basisConv, basisConvkeys, a_fixingLagKey);
				if (a_fixingLag.findString("D") == -1)
				{
					a_fixingLag += "D";
				}
			}

			// Target leg calendar
			AQLString t_calendar;
			AQLString t_calendarKey = isLeg1Target ? "Leg1Cashlet.calendar" : "Leg2Cashlet.calendar";
			findValByKey(t_calendar, basisConv, basisConvkeys, t_calendarKey);

			// Target leg date count convention
			AQLString t_dayCount;
			AQLString t_dayCountKey = isLeg1Target ? "Leg1Cashlet.daycount" : "Leg2Cashlet.daycount";
			findValByKey(t_dayCount, basisConv, basisConvkeys, t_dayCountKey);

			// Target leg frequency
			AQLString t_frequency;
			AQLString t_frequencyKey = isLeg1Target ? "Leg1Cashlet.frequency" : "Leg2Cashlet.frequency";
			findValByKey(t_frequency, basisConv, basisConvkeys, t_frequencyKey);

			// Target leg compounding frequency
			AQLString t_compounding_frequency("");
			AQLString t_compoundingFrequencyKey = isLeg1Target ? "Leg1Cashlet.frequencycompound" : "Leg2Cashlet.frequencycompound";
			findValByKey(t_compounding_frequency, basisConv, basisConvkeys, t_compoundingFrequencyKey, true);
			if (t_compounding_frequency.size() != 0 && t_compounding_frequency.toUpper() != t_frequency.toUpper())
			{
				std::cout << "Test file " << tenorBasisCurveFile << " is using compounding frequency. This is currently not supported for testing." << std::endl;
				continue;
			}

			// Target leg roll convention
			AQLString t_rollConvention;
			AQLString t_rollConventionKey = isLeg1Target ? "Leg1Cashlet.slidingrule" : "Leg2Cashlet.slidingrule";
			findValByKey(t_rollConvention, basisConv, basisConvkeys, t_rollConventionKey);

			// Target leg discount curve
			AQLString t_discount;
			AQLString t_discountKey = isLeg1Target ? "Leg1discount" : "Leg2discount";
			findValByKey(t_discount, basisConv, basisConvkeys, t_discountKey);

			// Target leg spot lag
			AQLString t_spotLag;
			AQLString t_spotLagKey = isLeg1Target ? "Leg1Cashlet.spotlag" : "Leg2Cashlet.spotlag";
			findValByKey(t_spotLag, basisConv, basisConvkeys, t_spotLagKey);
			if (t_spotLag.findString("D") == -1)
			{
				t_spotLag += "D";
			}

			// Target leg fixing lag
			AQLString t_fixingLag("0D");
			if (!IsSameGridIndex)
			{
				AQLString t_fixingLagKey = isLeg1Target ? "Leg1index.resetlag" : "Leg2index.resetlag";
				findValByKey(t_fixingLag, basisConv, basisConvkeys, t_fixingLagKey);
				if (t_fixingLag.findString("D") == -1)
				{
					t_fixingLag += "D";
				}
			}

			// Is rolling at end of month?
			AQLString isEomRollStr;
			findValByKey(isEomRollStr, basisConv, basisConvkeys, "IsEomRoll", true);
			isEomRollStr.toUpper();
			bool isEomRoll(false);
			if (isEomRollStr.toUpper() == "TRUE")
			{
				isEomRoll = true;
			}

			// Against Leg Interpolation
			AQLString a_interpolation;
			if (IsFwdInter)
			{
				findValByKey(a_interpolation, basisConv, basisConvkeys, "fwdinterpolation");
			}
			else
			{
				findValByKey(a_interpolation, generateProp_STD, generatePropkeys_STD, "yieldgen.interpolation");				
			}
			a_interpolation = interpolationShortName(a_interpolation);

			// Target Leg Interpolation
			AQLString t_interpolation;
			if (IsYieldSpreadCalc)
			{
				// When calibrating spreads, target curve interpolation is the same as that of the reference curve which
				// is assumed to be the STD curve in these tests
				findValByKey(t_interpolation, generateProp_STD, generatePropkeys_STD, "yieldgen.interpolation");				
			}
			else
			{
				findValByKey(t_interpolation, generateProp, generatePropkeys, "basis.interpolation");
			}
			t_interpolation = interpolationShortName(t_interpolation);
			
			// Effective dates
			AQLString asofDateStr;
			findValByKey(asofDateStr, generateProp, generatePropkeys, "AsOfDate");
			AQLDate asofDate = etrading::AQLDateScheduleHelpers::getAQLDate(asofDateStr);
			AQLDate a_startDate = etrading::AQLDateScheduleHelpers::getDate( asofDate, a_spotLag, a_rollConvention, a_calendar );
			AQLDate t_startDate = etrading::AQLDateScheduleHelpers::getDate( asofDate, t_spotLag, t_rollConvention, t_calendar );
			if (a_startDate != t_startDate)
			{
				AQLString err = "#Err: Start date is not the same on both legs. This scenario is currently not supported by the test. Please amend your test file or enhance code.";
				throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__); 
			}

			// Curve collection and forecast curve
			AQLString curveCollection = etrading::getCurveID( inputFile_TenorBasis );
			AQLString t_forecast		 = etrading::getMarketName( inputFile_TenorBasis );

			// basis rates block 
			etrading::ReadDataFile basisRates = inputFile_TenorBasis["basisRates"];			
			size_t rowCount = basisRates.rows();
			size_t colCount = basisRates.cols();


			//----------------------------------------------------------------------------------------
			// Check pricing consistency on all basis swap tenors
			
			//for(auto iterator = spotStartingbasisRatekeys.begin(); iterator != spotStartingbasisRatekeys.end(); ++iterator)
			for (size_t k = 0; k < rowCount; ++k)
			{
				AQLString maturityTenor;
				AQLString startTenor;

				AQLDate a_effectiveStart = a_startDate;
				AQLDate t_effectiveStart = t_startDate;
				if (isFwdBasis)
				{
					maturityTenor = basisRates(k, colCount - 1);
					startTenor = basisRates(k, colCount - 2);
					a_effectiveStart = etrading::AQLDateScheduleHelpers::getDate( a_startDate, startTenor, a_rollConvention, a_calendar );
					t_effectiveStart = etrading::AQLDateScheduleHelpers::getDate( t_startDate, startTenor, t_rollConvention, t_calendar );
				}
				else
				{
					maturityTenor = basisRates(k, 0);
				}
				AQLDate maturity = etrading::AQLDateScheduleHelpers::getDate(a_effectiveStart, maturityTenor, "", "");	// Maturity date must not be adjusted first
				
				AQLString frequencyFixed = (isLeg1Spread == isLeg1Target) ? t_frequency : a_frequency;
				AQLString dayCountFixed  = (isLeg1Spread == isLeg1Target) ? t_dayCount : a_dayCount;
				AQLString rollConvFixed  = (isLeg1Spread == isLeg1Target) ? t_rollConvention : a_rollConvention;
				AQLString calendarFixed  = (isLeg1Spread == isLeg1Target) ? t_calendar : a_calendar;								

				// Calculate 'against leg' par rate
				const double a_ParRate = validation::tryMirGetParRate4(
																   etrading::InitializeETrading::instance().dataInstance(),
																   a_effectiveStart.stringWithFormat( "YYYYMMDD" ),
																   maturity.stringWithFormat("YYYYMMDD"),
																   curveCollection,
																   frequencyFixed,
																   dayCountFixed,
																   rollConvFixed,
																   calendarFixed,
																   "",					// xFirstStub_,
																   "",					// xLastStub_,
																   "",					// xRollDay_,
																   "",					// xPayLag_,
																   "NONE",				// xStub_,
																   a_frequency,			// floating leg frequency
																   a_dayCount,			// floating leg day count
																   a_rollConvention,	// floating leg roll convention
																   a_calendar,			// floating leg calendar
																   "",					// tFirstStub_,
																   "",					// tLastStub_,
																   "",					// tRollDay_,
																   a_fixingLag,
																   0.0,					// tFirstFix_,
																   0.0,					// tLastFix_,
																   "",					// tPayLag_,
																   "NONE",				// tStub_,
																   a_interpolation,
																   a_forecast,
																   a_discount,
																   IsFwdInter,			// interpFwds_,
																   isEomRoll,
																   0.0,					// tSpd_,
																   IsFwdInter,			// useFwdData_ 
																   a_calendar			// fixing calendar
																   );

				// Calculate 'target leg' par rate
				const double t_ParRate = validation::tryMirGetParRate4(
																   etrading::InitializeETrading::instance().dataInstance(),
																   t_effectiveStart.stringWithFormat( "YYYYMMDD" ),
																   maturity.stringWithFormat("YYYYMMDD"),
																   curveCollection,
																   frequencyFixed,		// Fixed leg frequency, calendar, day count, and roll conv must be the same in both par rates
																   dayCountFixed,		
																   rollConvFixed,
																   calendarFixed,
																   "",					// xFirstStub_,
																   "",					// xLastStub_,
																   "",					// xRollDay_,
																   "",					// xPayLag_,
																   "NONE",				// xStub_,
																   t_frequency,			// floating leg frequency
																   t_dayCount,			// floating leg day count
																   t_rollConvention,	// floating leg roll convention
																   t_calendar,			// floating leg calendar
																   "",					// tFirstStub_,
																   "",					// tLastStub_,
																   "",					// tRollDay_,
																   t_fixingLag,
																   0.0,					// tFirstFix_,
																   0.0,					// tLastFix_,
																   "",					// tPayLag_,
																   "NONE",				// tStub_,
																   t_interpolation,
																   t_forecast,
																   t_discount,
																   false,				// interpFwds_ must be FALSE on the target leg par rate
																   isEomRoll,
																   0.0,					// tSpd_,
																   false,				// useFwdData_ must be FALSE on the target leg par rate
																   t_calendar			// fixing calendar
																   );

				AQLString inputBasisStr = basisRates(k, 1);
				
				double inputBasisRate(0.0);
				std::stringstream(inputBasisStr.getCString()) >> inputBasisRate;

				double impliedBasisRate = (isLeg1Spread == isLeg1Target) ? a_ParRate - t_ParRate : t_ParRate - a_ParRate;
				double diff = std::fabs(impliedBasisRate -  inputBasisRate);
				
				// Tolerance Setttings - Linear Spline Test is unstable / near tolerance so we use special tolerance here
				double testTolerance = tolerance;
				if ( ccy == "LINEARSPLINE" )
				{
					testTolerance = linearSplineTolerance;
				}

				if (diff > testTolerance)
				{
					if ((maturityTenor == "3M" || maturityTenor == "6M") && (startTenor.size() == 0 || startTenor == "0D"))
					{
						AQ_PRINT("This test currently does not support checking the rate consistency of the Libor instrument")
						continue;
					}
					else
					{
						AQ_PRINT("Test curve '" + tenorBasisCurveFile + "' has error at " + maturityTenor)
						AQ_PRINT("Calibration Rate: " + AQ_TO_STRING_FROM_DOUBLE(inputBasisRate))
						AQ_PRINT("Implied Rate	: " + AQ_TO_STRING_FROM_DOUBLE(impliedBasisRate))
						AQ_PRINT("Difference		: " + AQ_TO_STRING_FROM_DOUBLE(diff))
						AQ_PRINT("Tolerance		: " + AQ_TO_STRING_FROM_DOUBLE(tolerance))
					}
				}
			}

			// Flush the cache in preparation for a new set of curves
			validation::tryAqToolsClearEntityPool();	
		}		
    }

	
	//=====================================================================================================================================//
	//=====================================================================================================================================//


	DECLARE_TEST_FIXTURE(CurveConsistencies_TenorBasis);

    TEST_F( CurveConsistencies_TenorBasis, CONSISTENCY_USD_Calibration_Instrument_Repricing)
    {
        basisCurveConsistencyCheck("USD", TEST_DIR);
	}

	TEST_F( CurveConsistencies_TenorBasis, CONSISTENCY_EUR_Calibration_Instrument_Repricing)
    {
        basisCurveConsistencyCheck("EUR", TEST_DIR);
	}

	TEST_F( CurveConsistencies_TenorBasis, CONSISTENCY_JPY_Calibration_Instrument_Repricing)
    {
        basisCurveConsistencyCheck("JPY", TEST_DIR);
	}

	TEST_F( CurveConsistencies_TenorBasis, CONSISTENCY_GBP_Calibration_Instrument_Repricing)
    {
        basisCurveConsistencyCheck("GBP", TEST_DIR);
	}

	TEST_F( CurveConsistencies_TenorBasis, CONSISTENCY_LINEAR_SPLINE_Calibration_Instrument_Repricing)
	{
		basisCurveConsistencyCheck("LINEARSPLINE", TEST_DIR_LINEARSPLINE);
	}

		
}
