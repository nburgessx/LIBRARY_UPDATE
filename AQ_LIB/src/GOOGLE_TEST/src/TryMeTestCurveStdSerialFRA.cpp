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

#define TEST_DIR "ETrading/Curves/TestCurveStdSerialFRA/"

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
	//
    // test call input and reference files
    //
    extern const char Contiguous_ForwardRatesInputs[]				= TEST_DIR "Contiguous_EURYC_EUR6ML_tryMeCurveForwardRatesFromForwardDates_inputs.csv";
    extern const char Contiguous_ForwardRatesOutputs[]				= TEST_DIR "Contiguous_EURYC_EUR6ML_tryMeCurveForwardRatesFromForwardDates_outputs.csv";

	extern const char SerialRate_ForwardRatesInputs[]				= TEST_DIR "SerialRate_EURYC_EUR6ML_tryMeCurveForwardRatesFromForwardDates_inputs.csv";
	extern const char SerialRate_ForwardRatesOutputs[]				= TEST_DIR "SerialRate_EURYC_EUR6ML_tryMeCurveForwardRatesFromForwardDates_outputs.csv";

	extern const char SerialDF_ForwardRatesInputs[]					= TEST_DIR "SerialDF_EURYC_EUR6ML_tryMeCurveForwardRatesFromForwardDates_inputs.csv";
	extern const char SerialDF_ForwardRatesOutputs[]				= TEST_DIR "SerialDF_EURYC_EUR6ML_tryMeCurveForwardRatesFromForwardDates_outputs.csv";

	// curve input files
	//
	const std::string EURYC_OIS = etrading::getGoogleTestFolder() + TEST_DIR "EUR_OIS_CURVE.JSON";
	const std::string Contiguous_EURYC_STD = etrading::getGoogleTestFolder() + TEST_DIR "Contiguous_EUR_SWAP_6M_CURVE.JSON";
	const std::string SerialRate_EURYC_STD = etrading::getGoogleTestFolder() + TEST_DIR "SerialRate_EUR_SWAP_6M_CURVE.JSON";
	const std::string SerialDF_EURYC_STD = etrading::getGoogleTestFolder() + TEST_DIR "SerialDF_EUR_SWAP_6M_CURVE.JSON";

}

namespace google_test
{

	DECLARE_TEST_FIXTURE(TryMeTestCurveStdSerialFRA);

    TEST_F(TryMeTestCurveStdSerialFRA, SNAPSHOT_CheckForwardRates_Contiguous)
    {
		validation::tryMeLWOLoad(EURYC_OIS);
		validation::tryMeLWOLoad(Contiguous_EURYC_STD);

		const ReadDataFile::Load inputFile(Contiguous_ForwardRatesInputs);

		DateVector fromDateVector = inputFile["fromDates"];
		DateVector toDateVector = inputFile["toDates"];

		const DoubleArray results = validation::tryMeCurveForwardRatesFromForwardDates(fromDateVector, toDateVector, inputFile["curveCollection"], inputFile["curveIndex"]);

		CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, Contiguous_ForwardRatesOutputs, tolerance);
    }

	TEST_F(TryMeTestCurveStdSerialFRA, SNAPSHOT_CheckForwardRates_SerialRate)
	{

		validation::tryMeLWOLoad(EURYC_OIS);
		validation::tryMeLWOLoad(SerialRate_EURYC_STD);

		const ReadDataFile::Load inputFile(SerialRate_ForwardRatesInputs);

		DateVector fromDateVector = inputFile["fromDates"];
		DateVector toDateVector = inputFile["toDates"];

		const DoubleArray results = validation::tryMeCurveForwardRatesFromForwardDates(fromDateVector, toDateVector, inputFile["curveCollection"], inputFile["curveIndex"]);

		CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, SerialRate_ForwardRatesOutputs, tolerance);
	}

	TEST_F(TryMeTestCurveStdSerialFRA, SNAPSHOT_CheckForwardRates_SerialDF)
	{
		validation::tryMeLWOLoad(EURYC_OIS);
		validation::tryMeLWOLoad(SerialDF_EURYC_STD);

		const ReadDataFile::Load inputFile(SerialDF_ForwardRatesInputs);

		DateVector fromDateVector = inputFile["fromDates"];
		DateVector toDateVector = inputFile["toDates"];

		const DoubleArray results = validation::tryMeCurveForwardRatesFromForwardDates(fromDateVector, toDateVector, inputFile["curveCollection"], inputFile["curveIndex"]);

		CheckTestResultsAndRebaseOnRequest(results, TEST_DIR, SerialDF_ForwardRatesOutputs, tolerance);
	}



}
