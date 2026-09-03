#include "ReadDataFile.h"
#include "ResultsProcessor.h"
#include "GetGoogleTestFolder.h"

#include "TryMeCurveOis.h"
#include "TryMeCurveStd.h"
#include "tryMeLWO.h"
#include "tryMeCurveForwardRate.h"
#include "tryMeLWOSwapPricing.h"

#include <gTest/gTest.h>

using etrading::ReadDataFile;

#define TEST_DIR "ETrading/Curves/TestCurveStdSerialFuture/"

namespace
{

    // test tolerance
    // --------------

#ifdef GTEST32
    // This Test was recorded in WIN32 so higher precision here i.e. better match against test results
    const double tolerance = 1e-8;
#else
    // The test should still pass in WIN64, but since the test was recorded using WIN32, results won't
    // match the expected hard-coded results too well
    const double tolerance = 1e-5;
#endif


    //
    // curve input files
    //
    extern const char Contiguous_USDYC_OIS[]					= TEST_DIR "Contiguous_OIS_tryMeCurveCalibrateOIS_inputs.csv";
    extern const char Contiguous_USDYC_STD[]					= TEST_DIR "Contiguous_STD_tryMeCurveCalibrateSwap_inputs.csv";

	extern const char SerialRate_USDYC_OIS[]					= TEST_DIR "SerialRate_OIS_tryMeCurveCalibrateOIS_inputs.csv";
	extern const char SerialRate_USDYC_STD[]					= TEST_DIR "SerialRate_STD_tryMeCurveCalibrateSwap_inputs.csv";
	
	extern const char SerialDF_USDYC_OIS[]						= TEST_DIR "SerialDF_OIS_tryMeCurveCalibrateOIS_inputs.csv";
	extern const char SerialDF_USDYC_STD[]						= TEST_DIR "SerialDF_STD_tryMeCurveCalibrateSwap_inputs.csv";
	
	//
    // test call input and reference files
    //
    extern const char Contiguous_ForwardRatesInputs[]				= TEST_DIR "Contiguous_USD3M_tryMeCurveForwardRatesFromForwardDates_inputs.csv";
    extern const char Contiguous_ForwardRatesOutputs[]				= TEST_DIR "Contiguous_USD3M_tryMeCurveForwardRatesFromForwardDates_outputs.csv";

	extern const char SerialRate_ForwardRatesInputs[]				= TEST_DIR "SerialRate_USD3M_tryMeCurveForwardRatesFromForwardDates_inputs.csv";
	extern const char SerialRate_ForwardRatesOutputs[]				= TEST_DIR "SerialRate_USD3M_tryMeCurveForwardRatesFromForwardDates_outputs.csv";

	extern const char SerialDF_ForwardRatesInputs[]					= TEST_DIR "SerialDF_USD3M_tryMeCurveForwardRatesFromForwardDates_inputs.csv";
	extern const char SerialDF_ForwardRatesOutputs[]				= TEST_DIR "SerialDF_USD3M_tryMeCurveForwardRatesFromForwardDates_outputs.csv";

	extern const char Contiguous_SwapParRateInputs[]				= TEST_DIR "Contiguous_tryMeLWOSwapParRate_inputs.csv";
	extern const char Contiguous_SwapParRateOutputs[]				= TEST_DIR "Contiguous_tryMeLWOSwapParRate_outputs.csv";

	extern const char SerialRate_SwapParRateInputs[]				= TEST_DIR "SerialRate_tryMeLWOSwapParRate_inputs.csv";
	extern const char SerialRate_SwapParRateOutputs[]				= TEST_DIR "SerialRate_tryMeLWOSwapParRate_outputs.csv";

	extern const char SerialDF_SwapParRateInputs[]					= TEST_DIR "SerialDF_tryMeLWOSwapParRate_inputs.csv";
	extern const char SerialDF_SwapParRateOutputs[]					= TEST_DIR "SerialDF_tryMeLWOSwapParRate_outputs.csv";

	// Test Files
	//const std::string SwapInput = etrading::getGoogleTestFolder() + "ETrading/Curves/TestCurveStdSerialFuture/EDJ0_COMDTY_SWAP.JSON";
	const std::string SwapInput = etrading::getGoogleTestFolder() + TEST_DIR "EDJ0_COMDTY_SWAP.JSON";

}

namespace google_test
{

	DECLARE_TEST_FIXTURE(TryMeTestCurveStdSerialFuture);

    TEST_F(TryMeTestCurveStdSerialFuture, SNAPSHOT_CheckForwardRates_Contiguous)
    {

		setUpMeOISCurve(Contiguous_USDYC_OIS);
		setUpMeSTDCurve(Contiguous_USDYC_STD);

		const ReadDataFile::Load inputFile(Contiguous_ForwardRatesInputs);

		DateVector fromDateVector = inputFile["fromDates"];
		DateVector toDateVector = inputFile["toDates"];

		const DoubleArray results = validation::tryMeCurveForwardRatesFromForwardDates(fromDateVector, toDateVector, inputFile["curveCollection"], inputFile["curveIndex"]);

		CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, Contiguous_ForwardRatesOutputs, tolerance);
    }

	TEST_F(TryMeTestCurveStdSerialFuture, SNAPSHOT_CheckForwardRates_SerialRate)
	{

		setUpMeOISCurve(SerialRate_USDYC_OIS);
		setUpMeSTDCurve(SerialRate_USDYC_STD);

		const ReadDataFile::Load inputFile(SerialRate_ForwardRatesInputs);

		DateVector fromDateVector = inputFile["fromDates"];
		DateVector toDateVector = inputFile["toDates"];

		const DoubleArray results = validation::tryMeCurveForwardRatesFromForwardDates(fromDateVector, toDateVector, inputFile["curveCollection"], inputFile["curveIndex"]);

		CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, SerialRate_ForwardRatesOutputs, tolerance);
	}

	TEST_F(TryMeTestCurveStdSerialFuture, SNAPSHOT_CheckForwardRates_SerialDF)
	{
		setUpMeOISCurve(SerialDF_USDYC_OIS);
		setUpMeSTDCurve(SerialDF_USDYC_STD);

		const ReadDataFile::Load inputFile(SerialDF_ForwardRatesInputs);

		DateVector fromDateVector = inputFile["fromDates"];
		DateVector toDateVector = inputFile["toDates"];

		const DoubleArray results = validation::tryMeCurveForwardRatesFromForwardDates(fromDateVector, toDateVector, inputFile["curveCollection"], inputFile["curveIndex"]);

		CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, SerialDF_ForwardRatesOutputs, tolerance);
	}

	TEST_F(TryMeTestCurveStdSerialFuture, SNAPSHOT_CheckSerialFutureSwapParRate_Contiguous)
	{

		setUpMeOISCurve(Contiguous_USDYC_OIS);
		setUpMeSTDCurve(Contiguous_USDYC_STD);

		const std::string swapObjectName = validation::tryMeLWOLoad(SwapInput);

		const ReadDataFile::Load inputFile(Contiguous_SwapParRateInputs);
		const AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];

		const double result = validation::tryMeLWOSwapParRate(swapObjectName, valuationSettingsLVB);

		CheckTestResultsAndRebaseOnRequest(result, TEST_DIR, Contiguous_SwapParRateOutputs, tolerance);
	}

	TEST_F(TryMeTestCurveStdSerialFuture, SNAPSHOT_CheckSerialFutureSwapParRate_SerialRate)
	{

		setUpMeOISCurve(SerialRate_USDYC_OIS);
		setUpMeSTDCurve(SerialRate_USDYC_STD);

		const std::string swapObjectName = validation::tryMeLWOLoad(SwapInput);

		const ReadDataFile::Load inputFile(SerialRate_SwapParRateInputs);
		const AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];

		const double result = validation::tryMeLWOSwapParRate(swapObjectName, valuationSettingsLVB);

		CheckTestResultsAndRebaseOnRequest(result, TEST_DIR, SerialRate_SwapParRateOutputs, tolerance);
	}

	TEST_F(TryMeTestCurveStdSerialFuture, SNAPSHOT_CheckSerialFutureSwapParRate_SerialDF)
	{

		setUpMeOISCurve(SerialDF_USDYC_OIS);
		setUpMeSTDCurve(SerialDF_USDYC_STD);

		const std::string swapObjectName = validation::tryMeLWOLoad(SwapInput);

		const ReadDataFile::Load inputFile(SerialDF_SwapParRateInputs);
		const AQLStringMatrix valuationSettingsLVB = inputFile["valuationSettingsLVB"];

		const double result = validation::tryMeLWOSwapParRate(swapObjectName, valuationSettingsLVB);

		CheckTestResultsAndRebaseOnRequest(result, TEST_DIR, SerialDF_SwapParRateOutputs, tolerance);
	}


}
