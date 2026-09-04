#include "Variant.h"
#include "LabelValueBlock.h"
#include "Dependency.h"
#include "ReadDataFile.h"
#include "TestHelperUtilities.h"
#include "InitializeETrading.h"
#include "tryAqToolsSetup.h"
#include "ContainerUtilities.h"
#include "CoreEnumerations.h"
#include "CommonConstants.h"
#include "BuildMarketDataObjectFromFile.h"

#include <sstream>
#include <boost/range/irange.hpp>

#include "CurveOis.h"

// "me" API
#include "TryAqCurvesOis.h"
#include "TryAqCurvesStd.h"
#include "TryAqCurvesTenorBasis.h"
#include "TryAqCurvesXccyBasis.h"

// "Generator" API
#include "tryAqObjectsCurveMarketData.h"
#include "tryAqObjectsCurveCalibrate.h"

// Forward Rates Table
#include "tryAqCurvesForwardRate.h"

using etrading::ReadDataFile;

// Define the Test Input Folder Here
#define TEST_DIR			  "ETrading/Curves/TestCurveGeneratorForwardRates/"


namespace
{
    // test tolerances
    // ---------------
    const double tolerance = 1e-7;


    //
    // curve input files
    //
	
    // NEW EUR Tests
    // -------------
    const char ME_USD_OIS_CURVE2[]				= TEST_DIR "USDME_OIS_tryAqCurvesCalibrateOIS_inputs2.csv";
	const char ME_USD_STD_CURVE2[]				= TEST_DIR "USDME_STD_tryAqCurvesCalibrateSwap_inputs2.csv";
    
    const char ME_EUR_OIS_CURVE2[]				= TEST_DIR "EURME_OIS_tryAqCurvesCalibrateOIS_inputs2.csv";
	const char ME_EUR_STD_CURVE2[]				= TEST_DIR "EURME_STD_tryAqCurvesCalibrateSwap_inputs2.csv";

    const char GEN_USD_OIS_MARKETDATA2[]		= TEST_DIR "USDGEN_OIS_tryAqObjectsCurveMarketDataCreate_inputs2.csv";
	const char GEN_USD_STD_MARKETDATA2[]		= TEST_DIR "USDGEN_STD_tryAqObjectsCurveMarketDataCreate_inputs2.csv";

    const char GEN_EUR_OIS_MARKETDATA2[]		= TEST_DIR "EURGEN_OIS_tryAqObjectsCurveMarketDataCreate_inputs2.csv";
	const char GEN_EUR_STD_MARKETDATA2[]		= TEST_DIR "EURGEN_STD_tryAqObjectsCurveMarketDataCreate_inputs2.csv";

    const char GEN_USD_OIS_CURVE2[]				= TEST_DIR "USDGEN_OIS_tryAqObjectsCurveCalibrate_inputs2.csv";
	const char GEN_USD_STD_CURVE2[]				= TEST_DIR "USDGEN_STD_tryAqObjectsCurveCalibrate_inputs2.csv";

    const char GEN_EUR_OIS_CURVE2[]				= TEST_DIR "EURGEN_OIS_tryAqObjectsCurveCalibrate_inputs2.csv";
	const char GEN_EUR_STD_CURVE2[]				= TEST_DIR "EURGEN_STD_tryAqObjectsCurveCalibrate_inputs2.csv";


	// Curve recording files build using the ME interface
	const char ME_USD_OIS_CURVE[]				= TEST_DIR "USDME_OIS_tryAqCurvesCalibrateOIS_inputs.csv";
	const char ME_USD_STD_CURVE[]				= TEST_DIR "USDME_STD_tryAqCurvesCalibrateSwap_inputs.csv";
	const char ME_USD_3X6BASIS_CURVE[]			= TEST_DIR "USDME_3X6BASIS_tryAqCurvesCalibrateBasis_inputs.csv";
	const char ME_USD_1X3BASIS_CURVE[]			= TEST_DIR "USDME_1X3BASIS_tryAqCurvesCalibrateBasis_inputs.csv";
	const char ME_USD_3X12BASIS_CURVE[]			= TEST_DIR "USDME_3X12BASIS_tryAqCurvesCalibrateBasis_inputs.csv";

    const char ME_EUR_OIS_CURVE[]				= TEST_DIR "EURME_OIS_tryAqCurvesCalibrateOIS_inputs.csv";
	const char ME_EUR_STD_CURVE[]				= TEST_DIR "EURME_STD_tryAqCurvesCalibrateSwap_inputs.csv";
	const char ME_EUR_3X6BASIS_CURVE[]			= TEST_DIR "EURME_3X6BASIS_tryAqCurvesCalibrateBasis_inputs.csv";
	const char ME_EUR_XCCY_CURVE[]				= TEST_DIR "EURME_XCCY_tryAqCurvesCalibrateBasis_inputs.csv";

	const char ME_JPY_OIS_CURVE[]				= TEST_DIR "JPYME_OIS_tryAqCurvesCalibrateOIS_inputs.csv";
	const char ME_JPY_JSCC_SWAP_CURVE[]			= TEST_DIR "JPYME_JSCC_SWAP_tryAqCurvesCalibrateSwap_inputs.csv";
	const char ME_JPY_JSCC_3X6BASIS_CURVE[]		= TEST_DIR "JPYME_JSCC_3X6BASIS_tryAqCurvesCalibrateBasis_inputs.csv";
	const char ME_JPY_JSCC_1X6BASIS_CURVE[]		= TEST_DIR "JPYME_JSCC_1X6BASIS_tryAqCurvesCalibrateBasis_inputs.csv";

	const char ME_JPY_LCH_SWAP_CURVE[]			= TEST_DIR "JPYME_LCH_SWAP_tryAqCurvesCalibrateSwap_inputs.csv";
	const char ME_JPY_LCH_3X6BASIS_CURVE[]		= TEST_DIR "JPYME_LCH_3X6BASIS_tryAqCurvesCalibrateBasis_inputs.csv";
	const char ME_JPY_LCH_1X6BASIS_CURVE[]		= TEST_DIR "JPYME_LCH_1X6BASIS_tryAqCurvesCalibrateBasis_inputs.csv";

	const char ME_JPY_XCCY_CURVE[]				= TEST_DIR "JPYME_XCCY_tryAqCurvesCalibrateBasis_inputs.csv";

    const char ME_JPY_TIBOR_SWAP_CURVE[]		= TEST_DIR "JPYME_TIBOR_SWAP_tryAqCurvesCalibrateSwap_inputs.csv";
	const char ME_JPY_TIBOR_6M_BASIS_CURVE[]	= TEST_DIR "JPYME_TIBOR_6M_BASIS_tryAqCurvesCalibrateBasis_inputsNEW.csv";
	const char ME_JPY_TIBOR_3X6BASIS_CURVE[]	= TEST_DIR "JPYME_TIBOR_3X6BASIS_tryAqCurvesCalibrateBasis_inputsNEW.csv";
	const char ME_JPY_TIBOR_1X6BASIS_CURVE[]	= TEST_DIR "JPYME_TIBOR_1X6BASIS_tryAqCurvesCalibrateBasis_inputsNEW.csv";

	// -------------------------------------------------------------

	// Curve market data files for use with CurveGenerators
	const char GEN_USD_OIS_MARKETDATA[]			= TEST_DIR "USDGEN_OIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_USD_STD_MARKETDATA[]			= TEST_DIR "USDGEN_STD_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_USD_3X6BASIS_MARKETDATA[]	= TEST_DIR "USDGEN_3X6BASIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_USD_1X3BASIS_MARKETDATA[]	= TEST_DIR "USDGEN_1X3BASIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_USD_3X12BASIS_MARKETDATA[]	= TEST_DIR "USDGEN_3X12BASIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";

	const char GEN_EUR_OIS_MARKETDATA[]			= TEST_DIR "EURGEN_OIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_EUR_STD_MARKETDATA[]			= TEST_DIR "EURGEN_STD_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_EUR_3X6BASIS_MARKETDATA[]	= TEST_DIR "EURGEN_3X6BASIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_EUR_XCCY_MARKETDATA[]		= TEST_DIR "EURGEN_XCCY_tryAqObjectsCurveMarketDataCreate_inputs.csv";

	const char GEN_JPY_OIS_MARKETDATA[]			= TEST_DIR "JPYGEN_OIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_JPY_JSCC_SWAP_MARKETDATA[]	= TEST_DIR "JPYGEN_JSCC_SWAP_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_JPY_JSCC_3X6BASIS_MARKETDATA[]	= TEST_DIR "JPYGEN_JSCC_3X6BASIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_JPY_JSCC_1X6BASIS_MARKETDATA[]	= TEST_DIR "JPYGEN_JSCC_1X6BASIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";

	const char GEN_JPY_LCH_SWAP_MARKETDATA[]	= TEST_DIR "JPYGEN_LCH_SWAP_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_JPY_LCH_3X6BASIS_MARKETDATA[]	= TEST_DIR "JPYGEN_LCH_3X6BASIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_JPY_LCH_1X6BASIS_MARKETDATA[]	= TEST_DIR "JPYGEN_LCH_1X6BASIS_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	
	const char GEN_JPY_XCCY_MARKETDATA[]		= TEST_DIR "JPYGEN_XCCY_tryAqObjectsCurveMarketDataCreate_inputs.csv";

    const char GEN_JPY_TIBOR_SWAP_MARKETDATA[]	    = TEST_DIR "JPYGEN_TIBOR_SWAP_tryAqObjectsCurveMarketDataCreate_inputs.csv";
	const char GEN_JPY_TIBOR_6M_BASIS_MARKETDATA[]	= TEST_DIR "JPYGEN_TIBOR_6M_BASIS_tryAqObjectsCurveMarketDataCreate_inputsNEW.csv";
	const char GEN_JPY_TIBOR_3X6BASIS_MARKETDATA[]	= TEST_DIR "JPYGEN_TIBOR_3X6BASIS_tryAqObjectsCurveMarketDataCreate_inputsNEW.csv";
	const char GEN_JPY_TIBOR_1X6BASIS_MARKETDATA[]	= TEST_DIR "JPYGEN_TIBOR_1X6BASIS_tryAqObjectsCurveMarketDataCreate_inputsNEW.csv";

	// -------------------------------------------------------------

	// Curves built from generator and market data
	const char GEN_USD_OIS_CURVE[]				= TEST_DIR "USDGEN_OIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_USD_STD_CURVE[]				= TEST_DIR "USDGEN_STD_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_USD_3X6BASIS_CURVE[]			= TEST_DIR "USDGEN_3X6BASIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_USD_1X3BASIS_CURVE[]			= TEST_DIR "USDGEN_1X3BASIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_USD_3X12BASIS_CURVE[]			= TEST_DIR "USDGEN_3X12BASIS_tryAqObjectsCurveCalibrate_inputs.csv";

	const char GEN_EUR_OIS_CURVE[]				= TEST_DIR "EURGEN_OIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_EUR_STD_CURVE[]				= TEST_DIR "EURGEN_STD_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_EUR_3X6BASIS_CURVE[]			= TEST_DIR "EURGEN_3X6BASIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_EUR_XCCY_CURVE[]				= TEST_DIR "EURGEN_XCCY_tryAqObjectsCurveCalibrate_inputs.csv";

	const char GEN_JPY_OIS_CURVE[]				= TEST_DIR "JPYGEN_OIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_JPY_JSCC_SWAP_CURVE[]		= TEST_DIR "JPYGEN_JSCC_SWAP_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_JPY_JSCC_3X6BASIS_CURVE[]	= TEST_DIR "JPYGEN_JSCC_3X6BASIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_JPY_JSCC_1X6BASIS_CURVE[]	= TEST_DIR "JPYGEN_JSCC_1X6BASIS_tryAqObjectsCurveCalibrate_inputs.csv";
	
	const char GEN_JPY_LCH_SWAP_CURVE[]		= TEST_DIR "JPYGEN_LCH_SWAP_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_JPY_LCH_3X6BASIS_CURVE[]	= TEST_DIR "JPYGEN_LCH_3X6BASIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_JPY_LCH_1X6BASIS_CURVE[]	= TEST_DIR "JPYGEN_LCH_1X6BASIS_tryAqObjectsCurveCalibrate_inputs.csv";
	
	const char GEN_JPY_XCCY_CURVE[]				= TEST_DIR "JPYGEN_XCCY_tryAqObjectsCurveCalibrate_inputs.csv";

    const char GEN_JPY_TIBOR_SWAP_CURVE[]		= TEST_DIR "JPYGEN_TIBOR_SWAP_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_JPY_TIBOR_6M_BASIS_CURVE[]	= TEST_DIR "JPYGEN_TIBOR_6M_BASIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_JPY_TIBOR_3X6BASIS_CURVE[]	= TEST_DIR "JPYGEN_TIBOR_3X6BASIS_tryAqObjectsCurveCalibrate_inputs.csv";
	const char GEN_JPY_TIBOR_1X6BASIS_CURVE[]	= TEST_DIR "JPYGEN_TIBOR_1X6BASIS_tryAqObjectsCurveCalibrate_inputs.csv";
}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestCurveGeneratorForwardRates);

	/* @brief			Builds Generator curve by invoking the tryAqObjectsCurveCalibration() API.
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/	
	void createLWOCurveFromFileName( const AQLString& curveCalibrationFileName )
	{
		etrading::ReadDataFile::Load curveCalibrationFileObj = etrading::ReadDataFile::Load( curveCalibrationFileName );
		
		std::string lwoCurveGeneratorName	= curveCalibrationFileObj[ "lwoCurveGeneratorName" ];
		std::string lwoCurveMarketDataName	= curveCalibrationFileObj[ "lwoCurveMarketDataName" ];
		std::string domesticCurveCollection	= curveCalibrationFileObj[ "domesticCurveCollection" ];
		std::string foreignCurveCollection	= curveCalibrationFileObj[ "foreignCurveCollection" ];
		
		std::string objectName = lwoCurveGeneratorName;

		validation::tryAqObjectsCurveCalibrate(	objectName, lwoCurveGeneratorName, lwoCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
	}

	/* @brief			Builds and Generator curve using the specified marketData and calibration filename
	*  @param [in]		marketDataFileName			The filename specifying generator curve data
	*  @param [in]		curveCalibrationFileName	The filename specifying generator curve build instructions
	*/
	void setUpGeneratorCurve( const AQLString& marketDataFileName, const AQLString& curveCalibrationFileName )
	{
		google_test::createLWOMarketDataObjectFromFileName( marketDataFileName );
		createLWOCurveFromFileName( curveCalibrationFileName );
	}


	//----------------------------------------------------------------------------------------

	/* @brief			Builds OIS curve using ME and Generator functions
    *  @param [in]		aqCurvesCalibrate	The filename specifying ME curve data
	*  @param [in]		genCurveMarketData	The filename specifying generator curve data
	*  @param [in]		aqCurvesCalibrate	The filename specifying generator curve build instructions
	*/
    void buildAqAndGeneratorCurves_OIS( const AQLString& aqCurvesCalibrate, const AQLString& genCurveMarketData, const AQLString& genCurveCalibrate )
	{
		// Build ME curve	
		setUpAqOISCurve( aqCurvesCalibrate );

		// Build Generator curve
		setUpGeneratorCurve( genCurveMarketData, genCurveCalibrate );
	}

	/* @brief			Builds swap curve using ME and Generator functions
    *  @param [in]		aqCurvesCalibrate	The filename specifying ME curve data
	*  @param [in]		genCurveMarketData	The filename specifying generator curve data
	*  @param [in]		aqCurvesCalibrate	The filename specifying generator curve build instructions
	*/
	void buildAqAndGeneratorCurves_STD( const AQLString& aqCurvesCalibrate, const AQLString& genCurveMarketData, const AQLString& genCurveCalibrate )
	{
		setUpAqSTDCurve( aqCurvesCalibrate );							// Build "ME" curve
		setUpGeneratorCurve( genCurveMarketData, genCurveCalibrate );	// Build "Generator" curve
	}

	/* @brief			Builds tenor basis curve using ME and Generator functions
    *  @param [in]		aqCurvesCalibrate	The filename specifying ME curve data
	*  @param [in]		genCurveMarketData	The filename specifying generator curve data
	*  @param [in]		aqCurvesCalibrate	The filename specifying generator curve build instructions
	*/
	void buildAqAndGeneratorCurves_BASIS( const AQLString& aqCurvesCalibrate, const AQLString& genCurveMarketData, const AQLString& genCurveCalibrate )
	{
		setUpAqTenorBasisCurve( aqCurvesCalibrate );						// Build "ME" curve
		setUpGeneratorCurve( genCurveMarketData, genCurveCalibrate );	// Build Generator curve
	}

	/* @brief			Builds XccyBasis curve using ME and Generator functions
    *  @param [in]		aqCurvesCalibrate	The filename specifying ME curve data
	*  @param [in]		genCurveMarketData	The filename specifying generator curve data
	*  @param [in]		aqCurvesCalibrate	The filename specifying generator curve build instructions
	*/
	void buildAqAndGeneratorCurves_XCCY( const AQLString& aqCurvesCalibrate, const AQLString& genCurveMarketData, const AQLString& genCurveCalibrate )
	{
		setUpAqXccyBasisCurve( aqCurvesCalibrate );						// Build "ME" curve
		setUpGeneratorCurve( genCurveMarketData, genCurveCalibrate );	// Build Generator curve
	}

	/* @brief			Calculates a matrix of forward rates and fixing dates for the curve indices in the specified curve collection.
    *  @param [in]		curveCollection		The curve collection containing the curveIndices to test
    *  @param [in]		curveIndices		A vector of curveIndices to check
	*  @param [out]		fixingDates			A vector containing monthly dates, up to 25Y in the future
	*  @param [out]		forwardRates		A matrix containing forward rates corresponding to each fixing date, and curve index.
	*/
	void getForwardRatesForCollection( const AQLString& curveCollection, const AQLStringVector& curveIndices, DateVector& fixingDates, DoubleMatrix& forwardRates )
	{
		fixingDates.clear();
		forwardRates.clear();

		const AQLString startDate;  // Leave blank; default to curve asOfDate.
		const AQLString maturity          = "25Y";
		const AQLString businessDayAdjust = "NO_CHANGE";
		const AQLString calendar;
		const AQLString rollConvention    = "NORMAL";
		const AQLString frequency         = "Monthly";

		validation::tryAqObjectsCurveForwardRatesTable( fixingDates, forwardRates, curveCollection, curveIndices, startDate, maturity, businessDayAdjust, calendar, rollConvention, frequency );
	}

	/* @brief			Check forward rate consistency between ME and Generator curves
    *  @param [in]		aqCurvesCollection		The curve collection containing ME curves
    *  @param [in]		genCurveCollection		The curve collection containing Generator curves
    *  @param [in]		curveIndices			A vector of curveIndices to check
	*/
	void checkForwardRatesConsistency( const AQLString& aqCurvesCollection, const AQLString& genCurveCollection, const AQLStringVector& curveIndices )
	{
		DateVector aqRatesFixingDates;
		DoubleMatrix aqRatesForwardRates;
		getForwardRatesForCollection( aqCurvesCollection, curveIndices, aqRatesFixingDates, aqRatesForwardRates );

		DateVector genFixingDates;
		DoubleMatrix genForwardRates;
		getForwardRatesForCollection( genCurveCollection, curveIndices, genFixingDates, genForwardRates );

		// First some basic sanity checks on the result of calling tryAqObjectsCurveForwardRatesTable()
		if ( aqRatesFixingDates.size() != genFixingDates.size() )
		{
			FAIL() << "aqRatesFixingDates has different number of dates to genFixingDates: " << aqRatesFixingDates.size() << " vs " << genFixingDates.size() << std::endl;
		}

		const size_t numAqForwardRateCols  = aqRatesForwardRates.size();
		const size_t numGenForwardRateCols = genForwardRates.size();
		const size_t numCurveIndices       = curveIndices.size();
		if ( numAqForwardRateCols != numCurveIndices )
		{
			FAIL() << "Expecting aqRatesForwardRates to have " << numCurveIndices << " columns, but it contained this many:" << numAqForwardRateCols << std::endl;
		}
		if ( numGenForwardRateCols != numCurveIndices )
		{
			FAIL() << "Expecting genForwardRates to have " << numCurveIndices << " columns, but it contained this many:" << numGenForwardRateCols << std::endl;
		}

		size_t numAqForwardRateRows  = aqRatesForwardRates[0].size();
		size_t numGenForwardRateRows = genForwardRates[0].size();
		if ( aqRatesFixingDates.size() != numAqForwardRateRows )
		{
			FAIL() << "aqRatesFixingDates size is different to the number of rows in aqRatesForwardRates matrix: " << aqRatesFixingDates.size() << " vs " << numAqForwardRateRows << std::endl;
		}
		if ( genFixingDates.size() != numGenForwardRateRows )
		{
			FAIL() << "genFixingDates size is different to the number of rows in genForwardRates matrix: " << genFixingDates.size() << " vs " << numGenForwardRateRows << std::endl;
		}

		// Now check all the forwards
		for (size_t i=0; i<numAqForwardRateCols; i++)
		{
			for (size_t j=0; j<numAqForwardRateRows; j++)
			{
				AQLDate fixingDate                = aqRatesFixingDates[j];
				double aqCurvesForwardRate        = aqRatesForwardRates[i][j];
				double generatorCurveForwardRate = genForwardRates[i][j];

				ASSERT_NEAR( aqCurvesForwardRate, generatorCurveForwardRate, tolerance ) << "#Error: forward rates do not match for fixing date: " << fixingDate 
																						 << " and curve: " << curveIndices[ i ] << std::endl;
			}
		}

		validation::tryAqToolsClearEntityPool();
	}

	TEST_F( TestCurveGeneratorForwardRates, CONSISTENCY_MeVsLWOCurveForwardRatesFor25Y_USD )
    {
		// Build ME and Generator curves for OIS, STD, 3X6Basis, 1X3Basis and 3X12Basis.
		// The ME curves are stored in the "USDME" collection,
		// while Generator curves are stored in the "USDGEN" collection
        buildAqAndGeneratorCurves_OIS( ME_USD_OIS_CURVE, GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		buildAqAndGeneratorCurves_STD( ME_USD_STD_CURVE, GEN_USD_STD_MARKETDATA, GEN_USD_STD_CURVE );
		buildAqAndGeneratorCurves_BASIS( ME_USD_3X6BASIS_CURVE, GEN_USD_3X6BASIS_MARKETDATA, GEN_USD_3X6BASIS_CURVE );
		buildAqAndGeneratorCurves_BASIS( ME_USD_1X3BASIS_CURVE, GEN_USD_1X3BASIS_MARKETDATA, GEN_USD_1X3BASIS_CURVE );
		buildAqAndGeneratorCurves_BASIS( ME_USD_3X12BASIS_CURVE, GEN_USD_3X12BASIS_MARKETDATA, GEN_USD_3X12BASIS_CURVE );

		// Compare forward rates between USDME and USDGEN collections
		AQLStringVector curveIndices;
		curveIndices.push_back( AQLString( "OIS" ) );
		curveIndices.push_back( AQLString( "STD" ) );
		curveIndices.push_back( AQLString( "3M6MBasis" ) );
		curveIndices.push_back( AQLString( "1M3MBasis" ) );
		curveIndices.push_back( AQLString( "3M12MBasis" ) );
		checkForwardRatesConsistency( "USDME", "USDGEN", curveIndices );
	}	
	
	TEST_F( TestCurveGeneratorForwardRates, CONSISTENCY_MeVsLWOCurveForwardRatesFor25Y_EUR )
    {
		buildAqAndGeneratorCurves_OIS( ME_USD_OIS_CURVE2, GEN_USD_OIS_MARKETDATA2, GEN_USD_OIS_CURVE2 );
		buildAqAndGeneratorCurves_STD( ME_USD_STD_CURVE2, GEN_USD_STD_MARKETDATA2, GEN_USD_STD_CURVE2 );

        buildAqAndGeneratorCurves_OIS( ME_EUR_OIS_CURVE2, GEN_EUR_OIS_MARKETDATA2, GEN_EUR_OIS_CURVE2 );
		buildAqAndGeneratorCurves_STD( ME_EUR_STD_CURVE2, GEN_EUR_STD_MARKETDATA2, GEN_EUR_STD_CURVE2 );

		AQLStringVector curveIndices;
		curveIndices.push_back( AQLString( "OIS" ) );
		curveIndices.push_back( AQLString( "EUR3ML" ) );

        checkForwardRatesConsistency( "EURME", "EURGEN", curveIndices );
	}

	TEST_F( TestCurveGeneratorForwardRates, CONSISTENCY_MeVsLWOCurveForwardRatesFor25Y_JPY )
    {
		buildAqAndGeneratorCurves_OIS( ME_USD_OIS_CURVE, GEN_USD_OIS_MARKETDATA, GEN_USD_OIS_CURVE );
		buildAqAndGeneratorCurves_STD( ME_USD_STD_CURVE, GEN_USD_STD_MARKETDATA, GEN_USD_STD_CURVE );

		buildAqAndGeneratorCurves_OIS( ME_JPY_OIS_CURVE, GEN_JPY_OIS_MARKETDATA, GEN_JPY_OIS_CURVE );
		buildAqAndGeneratorCurves_STD( ME_JPY_JSCC_SWAP_CURVE, GEN_JPY_JSCC_SWAP_MARKETDATA, GEN_JPY_JSCC_SWAP_CURVE );
		buildAqAndGeneratorCurves_BASIS( ME_JPY_JSCC_3X6BASIS_CURVE, GEN_JPY_JSCC_3X6BASIS_MARKETDATA, GEN_JPY_JSCC_3X6BASIS_CURVE );
		buildAqAndGeneratorCurves_BASIS( ME_JPY_JSCC_1X6BASIS_CURVE, GEN_JPY_JSCC_1X6BASIS_MARKETDATA, GEN_JPY_JSCC_1X6BASIS_CURVE );

		buildAqAndGeneratorCurves_STD( ME_JPY_LCH_SWAP_CURVE, GEN_JPY_LCH_SWAP_MARKETDATA, GEN_JPY_LCH_SWAP_CURVE );
		buildAqAndGeneratorCurves_BASIS( ME_JPY_LCH_3X6BASIS_CURVE, GEN_JPY_LCH_3X6BASIS_MARKETDATA, GEN_JPY_LCH_3X6BASIS_CURVE );
		buildAqAndGeneratorCurves_BASIS( ME_JPY_LCH_1X6BASIS_CURVE, GEN_JPY_LCH_1X6BASIS_MARKETDATA, GEN_JPY_LCH_1X6BASIS_CURVE );

		buildAqAndGeneratorCurves_XCCY( ME_JPY_XCCY_CURVE, GEN_JPY_XCCY_MARKETDATA, GEN_JPY_XCCY_CURVE );

        buildAqAndGeneratorCurves_STD( ME_JPY_TIBOR_SWAP_CURVE, GEN_JPY_TIBOR_SWAP_MARKETDATA, GEN_JPY_TIBOR_SWAP_CURVE );
		buildAqAndGeneratorCurves_BASIS( ME_JPY_TIBOR_6M_BASIS_CURVE, GEN_JPY_TIBOR_6M_BASIS_MARKETDATA, GEN_JPY_TIBOR_6M_BASIS_CURVE );
		buildAqAndGeneratorCurves_BASIS( ME_JPY_TIBOR_3X6BASIS_CURVE, GEN_JPY_TIBOR_3X6BASIS_MARKETDATA, GEN_JPY_TIBOR_3X6BASIS_CURVE );
		buildAqAndGeneratorCurves_BASIS( ME_JPY_TIBOR_1X6BASIS_CURVE, GEN_JPY_TIBOR_1X6BASIS_MARKETDATA, GEN_JPY_TIBOR_1X6BASIS_CURVE );

		AQLStringVector curveIndices;
		curveIndices.push_back( AQLString( "OIS" ) );
		curveIndices.push_back( AQLString( "STD" ) );
		curveIndices.push_back( AQLString( "3M6MBasis" ) );  // JSCC 3M
		curveIndices.push_back( AQLString( "1M6MBasis" ) );  // JSCC 1M
		curveIndices.push_back( AQLString( "JPY6ML" ) );
		curveIndices.push_back( AQLString( "JPY3ML" ) );
		curveIndices.push_back( AQLString( "JPY1ML" ) );
		curveIndices.push_back( AQLString( "XccyBasis" ) );
        curveIndices.push_back( AQLString( "JPY6ML" ) );     // JSCC 6M
        curveIndices.push_back( AQLString( "Tibor6M" ) );    
		curveIndices.push_back( AQLString( "Tibor3M" ) );
		curveIndices.push_back( AQLString( "Tibor1M" ) );
		checkForwardRatesConsistency( "JPYME", "JPYGEN", curveIndices );
	}

}
