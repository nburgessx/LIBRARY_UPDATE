#include "Dependency.h"
#include "ReadDataFile.h"
#include "TestHelperUtilities.h"
#include "CurveOis.h"
#include "tryMirGetDF.h"
#include "tryMirGetDate.h"
#include "tryMirOISParRate.h"
#include "tryMirOISSwapPV.h"
#include "YieldCurveUtil.h"
#include "AQLDateScheduleHelpers.h"
#include "InitializeETrading.h"
#include "tryAqToolsSetup.h"
#include <sstream>

using etrading::ReadDataFile;

#define TEST_DIR			  "Vanilla/CurveConsistencies/OIS/"

namespace
{    
	// test tolerance
	const double PARRATE_TOLERANCE  = 1e-6;
	const double PV_TOLERANCE		= 1e-5;

	// Test count
	const size_t USD_TEST_COUNT = 2;
	const size_t EUR_TEST_COUNT = 7;
	const size_t GBP_TEST_COUNT = 2;
	const size_t JPY_TEST_COUNT = 1;
	const size_t AUD_TEST_COUNT = 1;
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(CurveConsistencies_OIS);

	/* @brief	Run consistency test on all available swap curves in all ccys
    */
	void OISCurveConsistencyCheck(const AQLString& ccy, const AQLString& testDir)
	{
		AQLString prefix("");
		prefix = ccy + AQLString("_");
		
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
	
		for (size_t i = 0; i < TEST_COUNT; ++i)
		{
			//----------------------------------------------------------------------------------------
			// Build yield curves of the current test case

			AQLString curveFile = prefix + AQLString("OIS_") + AQLString(static_cast<int>(i + 1));
						
			AQLString curveFileDir = testDir + curveFile + AQLString(".csv");
			SET_UP_OIS_CURVE(curveFileDir);

			//----------------------------------------------------------------------------------------
			// Retrieve blocks of data from OIS curve file

			const ReadDataFile::Load inputFile_OIS( curveFileDir );

			// generateProp block
			etrading::ReadDataFile generateProp;
			try
			{
				generateProp = inputFile_OIS["generateProp"];			
			}
			catch (const std::exception&)
			{
				generateProp = inputFile_OIS["generalProps"];			
			}
			const std::set<AQLString>& generatePropkeys = generateProp.getKeys();

			// OIS par rates block
			etrading::ReadDataFile oisRates = inputFile_OIS["oisRates"];			
			const std::set<AQLString>& oisRateskeys = oisRates.getKeys();

			// OIS convention block
			etrading::ReadDataFile oisConv = inputFile_OIS["oisConv"];			
			const std::set<AQLString>& oisConvkeys = oisConv.getKeys();

			// LO basis rates block
			etrading::ReadDataFile loBasisRates = inputFile_OIS["loBasisRates"];			
			const std::set<AQLString>& loBasisRateskeys = loBasisRates.getKeys();

			// LO basis convention block
			etrading::ReadDataFile loBasisConv = inputFile_OIS["loBasisConv"];			
			const std::set<AQLString>& loBasisConvkeys = loBasisConv.getKeys();

			// Swap par rates block
			etrading::ReadDataFile swapRates = inputFile_OIS["swapRates"];			
			const std::set<AQLString>& swapRatekeys = swapRates.getKeys();

			// Swap convention block
			etrading::ReadDataFile swapConv = inputFile_OIS["swapConv"];			
			const std::set<AQLString>& swapConvkeys = swapConv.getKeys();
			
			//----------------------------------------------------------------------------------------
			// Retrieve all parameters required from various data blocks. 
			// Should any of these parameters be not available, skip this test as it's not a valid test
			
			// Interpolation
			AQLString interpolation;
			findValByKey(interpolation, generateProp, generatePropkeys, "yieldgen.interpolation");
			interpolation = interpolationShortName(interpolation);

			// Curve collection and forecast curve
			AQLString curveCollection = etrading::getCurveID( inputFile_OIS );
			AQLString foreCurve	     = etrading::getMarketName( inputFile_OIS );
			AQLString discCurve = foreCurve;

			//--------------------------
			// OIS swap related

			// Spot lag
			AQLString spotLag;
			findValByKey(spotLag, oisConv, oisConvkeys, "ResetLag");
			if (spotLag.findString("D") == -1)
			{
				spotLag += "D";
			}

			// OIS calendar
			AQLString oisCalendar;
			findValByKey(oisCalendar, oisConv, oisConvkeys, "Calendar");
						
			// OIS day count convention 
			AQLString oisDayCount;
			findValByKey(oisDayCount, oisConv, oisConvkeys, "DayCount");

			// OIS Frequency 
			AQLString oisFrequency;
			findValByKey(oisFrequency, oisConv, oisConvkeys, "Frequency");

			// OIS roll convention
			AQLString oisRollConvention;
			findValByKey(oisRollConvention, oisConv, oisConvkeys, "SlidingRule");

			// OIS compound method
			AQLString compoundMethod("");
			findValByKey(compoundMethod, oisConv, oisConvkeys, "CompoundMethod", true);
			if (compoundMethod.size() == 0)
			{
				findValByKey(compoundMethod, oisConv, oisConvkeys, "CompoundingMethod", true);
			}

			// Is rolling at end of month?
			AQLString isEomRollStr("");
			findValByKey(isEomRollStr, oisConv, oisConvkeys, "IsEomRoll", true);
			isEomRollStr.toUpper();
			
			bool isEomRoll(false);
			if (isEomRollStr == "TRUE")
			{
				isEomRoll = true;
			}

			
			//--------------------------
			// Libor-OIS basis related

			// LOBasis calendar
			AQLString loBasisCalendar("");
			findValByKey(loBasisCalendar, loBasisConv, loBasisConvkeys, "Calendar", true);
						
			// LOBasis day count convention 
			AQLString loBasisDayCount("");
			findValByKey(loBasisDayCount, loBasisConv, loBasisConvkeys, "DayCount", true);

			// LOBasis Frequency 
			AQLString loBasisFrequency("");
			findValByKey(loBasisFrequency, loBasisConv, loBasisConvkeys, "Frequency", true);

			// LOBasis roll convention
			AQLString loBasisRollConvention("");
			findValByKey(loBasisRollConvention, loBasisConv, loBasisConvkeys, "SlidingRule", true);

			//--------------------------
			// Related to fixed leg of Libor swaps

			// Swap calendar
			AQLString swapCalendar("");
			findValByKey(swapCalendar, swapConv, swapConvkeys, "CalendarFix", true);
			if (swapCalendar.size() == 0)
			{
				findValByKey(swapCalendar, swapConv, swapConvkeys, "Calendar", true);
			}

			// Swap day count convention 
			AQLString swapDayCount("");
			findValByKey(swapDayCount, swapConv, swapConvkeys, "DayCountFix", true);
			if (swapDayCount.size() == 0)
			{
				findValByKey(swapDayCount, swapConv, swapConvkeys, "DayCount", true);
			}

			// Swap Frequency 
			AQLString swapFrequency("");
			findValByKey(swapFrequency, swapConv, swapConvkeys, "FrequencyFix", true);
			if (swapFrequency.size() == 0)
			{
				findValByKey(swapFrequency, swapConv, swapConvkeys, "Frequency", true);
			}

			// Swap roll convention
			AQLString swapRollConvention("");
			findValByKey(swapRollConvention, swapConv, swapConvkeys, "SlidingRuleFix", true);
			if (swapRollConvention.size() == 0)
			{
				findValByKey(swapRollConvention, swapConv, swapConvkeys, "SlidingRule", true);
			}
			
			// Effective date
			AQLString asofDateStr;
			findValByKey(asofDateStr, generateProp, generatePropkeys, "AsOfDate");
			AQLDate asofDate = etrading::AQLDateScheduleHelpers::getAQLDate(asofDateStr);
			AQLDate effectiveDate = etrading::AQLDateScheduleHelpers::getDate( asofDate, spotLag, oisRollConvention, oisCalendar );
					
			// OIS long term convention
			AQLString oisLongTermConvention("");
			findValByKey(oisLongTermConvention, oisConv, oisConvkeys, "LongTermConvention", true);

			// OIS long term generation method
			AQLString oisLongTermGenMethod("");
			findValByKey(oisLongTermGenMethod, oisConv, oisConvkeys, "longterm.generatemethod", true);
			oisLongTermGenMethod.toUpper();

			// OIS long term cut off tenor
			AQLString oisLongTermTenor("");
			findValByKey(oisLongTermTenor, oisConv, oisConvkeys, "LongTerm", true);
			AQLDate longTermCutoffDate;
			if (oisLongTermTenor.size() != 0 && oisLongTermTenor.size() != 0)
			{
				longTermCutoffDate = etrading::AQLDateScheduleHelpers::getDate(effectiveDate, oisLongTermTenor, "", "");
			}

			//----------------------------------------------------------------------------------------
			// Check pricing consistency on all outright OIS swaps
			if (compoundMethod.toUpper() != "ARITHMETIC")
			{				
				// At the time of writing this test program, tryMirOISParRate has problem working with
				// ARITHMETIC and hence this is excluded from the test cases

				for(auto iterator = oisRateskeys.begin(); iterator != oisRateskeys.end(); ++iterator)
				{
					AQLString maturityTenor = *iterator;
					if (maturityTenor == "ON" || maturityTenor == "TN" )
					{
						continue;	// Do not support ON or TN instruments yet
					}

					bool unit_W = (maturityTenor.findString("W") == maturityTenor.size() - 1);
					bool unit_Y = (maturityTenor.findString("Y") == maturityTenor.size() - 1);
					bool unit_M = (maturityTenor.findString("M") == maturityTenor.size() - 1);
					bool unit_D = (maturityTenor.findString("D") == maturityTenor.size() - 1);
					if (! (unit_W || unit_Y || unit_M || unit_D))
					{
						continue;	// If this is a central bank swap, don't evaluate it.
					}

					// Do NOT test this swap as an OIS outright swap when the maturity tenor is longer than the longTermCutOff tenor
					if (oisLongTermConvention.size() != 0 && oisLongTermTenor.size() != 0)
					{					
						AQLDate maturity = etrading::AQLDateScheduleHelpers::getDate(effectiveDate, maturityTenor, "", "");	// Maturity date must not be adjusted first
						if (maturity >= longTermCutoffDate)
						{
							continue;
						}
					}
								
					const double calcParRate = validation::tryMirOISParRate( etrading::InitializeETrading::instance().dataInstance(),
								                                                 effectiveDate.stringWithFormat( "YYYYMMDD" ),
								                                                 maturityTenor,
								                                                 curveCollection,
								                                                 oisFrequency,
								                                                 oisDayCount,
								                                                 oisRollConvention,
								                                                 oisCalendar,
								                                                 "",	// fixedFirstStub
								                                                 "",	// fixedLastStub
								                                                 "",	// fixedRollDay
								                                                 "",	// fixedPayLag
								                                                 "",	// fixedStubType
								                                                 oisFrequency,
								                                                 oisDayCount,
								                                                 oisRollConvention,
								                                                 oisCalendar,
								                                                 "",	// floatFirstStub
								                                                 "",	// floatLastStub
								                                                 "",	// floatRollDay
								                                                 "",	// floatFixingLag
								                                                 0.0,	// floatFirstFixing
								                                                 0.0,	// floatLastFixing,
								                                                 "",	// floatPayLag
								                                                 "",	// floatStubType
								                                                 interpolation,
								                                                 foreCurve,
								                                                 discCurve,
								                                                 isEomRoll,
								                                                 0.0,	// floatSpread
								                                                 compoundMethod );

					AQLString inputParRateStr; 
					findValByKey(inputParRateStr, oisRates, oisRateskeys, maturityTenor);
				
					double inputParRate(0.0);
					std::stringstream(inputParRateStr.getCString()) >> inputParRate;

					double diff = std::fabs(inputParRate - calcParRate);
				
					if (diff > PARRATE_TOLERANCE)
					{
						AQLString err = "#Err: Test curve '" + curveFile + "' has error at " + maturityTenor;
						throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__); 
					}
				}
			}

			//----------------------------------------------------------------------------------------
			// Check pricing consistency on all Libor-OIS swaps
			if (oisLongTermGenMethod != "DAILYAVERAGING")
			{
				// tryMirOISSwapPV does not support long term daily averaging and hence the consistency tests
				// do not work well when LongTermGenerationMethod is DAILYAVERAGING

				for(auto iterator = loBasisRateskeys.begin(); iterator != loBasisRateskeys.end(); ++iterator)
				{
					AQLString maturityTenor = *iterator;
				
					if (oisLongTermConvention.size() != 0 && oisLongTermTenor.size() != 0)
					{					
						AQLDate maturity = etrading::AQLDateScheduleHelpers::getDate(effectiveDate, maturityTenor, "", "");	// Maturity date must not be adjusted first
						if (maturity >= longTermCutoffDate)
						{
							// Find libor swap par rate
							AQLString swapParRateStr; 
							findValByKey(swapParRateStr, swapRates, swapRatekeys, maturityTenor);

							double liborSwapParRate(0.0);
							std::stringstream(swapParRateStr.getCString()) >> liborSwapParRate;

							// Find Libor-OIS basis spread
							AQLString loBasisStr; 
							findValByKey(loBasisStr, loBasisRates, loBasisRateskeys, maturityTenor);

							double loBasis(0.0);
							std::stringstream(loBasisStr.getCString()) >> loBasis;
							loBasis *= 10000;	// Basis are passed to AlgoQuantLib in basis points unit

							// Compute Libor-OIS basis swap PV
							const double PV = validation::tryMirOISSwapPV( 
								etrading::InitializeETrading::instance().dataInstance(),
								1.0,	// notional
								"PAY",	// payRec
								effectiveDate.stringWithFormat( "YYYYMMDD" ),
								maturityTenor,
								curveCollection,
								liborSwapParRate,
								swapFrequency,
								swapDayCount,
								swapRollConvention,
								swapCalendar,
								"",	// fixedFirstStub
								"",	// fixedLastStub
								"",	// fixedRollDay
								"",	// fixedPayLag
								"",	// fixedStubType
								loBasisFrequency,
								loBasisDayCount,
								loBasisRollConvention,
								loBasisCalendar,
								"",	// floatFirstStub
								"",	// floatLastStub
								"",	// floatRollDay
								"",	// floatFixingLag
								0.0,	// floatFirstFixing
								0.0,	// floatLastFixing,
								"",	// floatPayLag
								"",	// floatStubType
								interpolation,
								foreCurve,
								discCurve,
								isEomRoll,
								loBasis,
								""	// compoundMethod 
								);

							double diff = std::fabs(PV - 0.0);
				
							if (diff > PV_TOLERANCE)
							{
								AQLString err = "#Err: Test curve '" + curveFile + "' has error at " + maturityTenor;
								throw AQLCoreInvalidData(err.getCString(), __FILE__, __LINE__); 
							}
						}
					}								
				}
			}

			// Flush the cache in preparation for a new set of curves
			validation::tryAqToolsClearEntityPool();
		}
    }


	TEST_F( CurveConsistencies_OIS, CONSISTENCY_USD_Calibration_Instrument_Repricing)
    {
        OISCurveConsistencyCheck("USD", TEST_DIR);
	}

	TEST_F( CurveConsistencies_OIS, CONSISTENCY_EUR_Calibration_Instrument_Repricing)
    {
        OISCurveConsistencyCheck("EUR", TEST_DIR);
	}

	TEST_F( CurveConsistencies_OIS, CONSISTENCY_AUD_Calibration_Instrument_Repricing)
    {
        OISCurveConsistencyCheck("AUD", TEST_DIR);
	}

	TEST_F( CurveConsistencies_OIS, CONSISTENCY_JPY_Calibration_Instrument_Repricing)
    {
        OISCurveConsistencyCheck("JPY", TEST_DIR);
	}

	TEST_F( CurveConsistencies_OIS, CONSISTENCY_GBP_Calibration_Instrument_Repricing)
    {
        OISCurveConsistencyCheck("GBP", TEST_DIR);
	}

}
