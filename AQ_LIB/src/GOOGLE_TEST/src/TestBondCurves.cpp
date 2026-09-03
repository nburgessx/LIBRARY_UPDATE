// Include: Google Test Library
#include "Dependency.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include "TestHelperUtilities.h"
#include "InitializeAQETrading.h"
#include "tryMeBondCurves.h"
#include "tryMeLWOBond.h"

#include "FolderConfig.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "ETrading/Bonds/BondCurves/"

namespace
{
    // Test Bond Input File(s)
    const std::string nelsonSiegelCalibrate				= TEST_DIR "tryMeBondCurveNelsonSiegelCalibrate_inputs.csv";
	const std::string svenssonCalibrate					= TEST_DIR "tryMeBondCurveSvenssonCalibrate_inputs.csv";
	const std::string polynomialCalibrate				= TEST_DIR "tryMeBondCurvePolynomialCalibrate_inputs.csv";
	
	const std::string nelsonSiegelBondYield				= TEST_DIR "tryMeBondCurveNelsonSiegelYield_inputs.csv";
    const std::string svenssonBondYield					= TEST_DIR "tryMeBondCurveSvenssonYield_inputs.csv";
    const std::string polynomialBondYield				= TEST_DIR "tryMeBondCurvePolynomialYield_inputs.csv";

	const std::string nelsonSiegelCalibrate_outputs_32	= TEST_DIR "tryMeBondCurveNelsonSiegelCalibrate_outputs_32bit.csv";
	const std::string svenssonCalibrate_outputs_32		= TEST_DIR "tryMeBondCurveSvenssonCalibrate_outputs_32bit.csv";
	const std::string polynomialCalibrate_outputs_32	= TEST_DIR "tryMeBondCurvePolynomialCalibrate_outputs_32bit.csv";

	const std::string nelsonSiegelCalibrate_outputs_64	= TEST_DIR "tryMeBondCurveNelsonSiegelCalibrate_outputs_64bit.csv";
	const std::string svenssonCalibrate_outputs_64		= TEST_DIR "tryMeBondCurveSvenssonCalibrate_outputs_64bit.csv";
	const std::string polynomialCalibrate_outputs_64	= TEST_DIR "tryMeBondCurvePolynomialCalibrate_outputs_64bit.csv";

	const std::string nelsonSiegelBondYield_outputs		= TEST_DIR "tryMeBondCurveNelsonSiegelYield_outputs.csv";
    const std::string svenssonBondYield_outputs			= TEST_DIR "tryMeBondCurveSvenssonYield_outputs.csv";
	const std::string polynomialBondYield_outputs		= TEST_DIR "tryMeBondCurvePolynomialYield_outputs.csv";

	// Bootstrapped bond curve
	const std::string createBondCurve					= TEST_DIR "USTREASURIES1@127_tryMeBondCurveCreate_inputs.csv";
	const std::string bondCurveDisplay_inputs			= TEST_DIR "tryMeBondCurveDisplay_inputs.csv";
	const std::string bondCurveDisplay_outputs			= TEST_DIR "tryMeBondCurveDisplay_outputs.csv";

	// Corporate bond curve as a spread to government treasury benchmark bond curve
	const std::string corporateSpreadBondCurve			= TEST_DIR "USD_CORP_CURVE@128_tryMeBondCurveCreate_inputs.csv";

	 /*
	 * This test builds 15 US Treasuries from input file, and uses these bonds to calibrate a bond curve.
	 * Rather than list out all bond filenames individually, we load then using the following regular expression pattern:
	 * The file pattern is:
	 * pathname / ( bondObjectName ) _tryMeLWOBondCreateFromGenerator_inputs.csv
	*/
    const std::string treasuryCreateInputFilePattern    = ".+_tryMeLWOBondCreateFromGenerator_inputs.csv";

	const std::string treasuryPriceFromYieldPattern     = ".+_tryMeLWOBondPrice_inputs.csv";

	const std::string treasuryPriceFromBondCurveSuffix  =  "_tryMeLWOBondPriceFromBondCurve_inputs.csv";


	/*
	 * Given a filePath and a pattern string, locate all the files in filePath which match the pattern
	 *
	 * The function returns a vector of matching filenames
	 */
	// https://stackoverflow.com/questions/1257721/can-i-use-a-mask-to-iterate-files-in-a-directory-with-boost
	std::vector<std::string> getFilesMatchingPattern( const std::string& filePath, const std::string& patternString )
	{
		
		boost::filesystem::path inputPath( filePath );
		if( ! inputPath.is_absolute() )
        {
            // Get the Google Test Unit Test Input Folder Path using the AlgoQuantLib Environment Variable
            // Format = AlgoQuantLib Goolge Test DataInstance Path + '/' + path to the 'p' Variable
            inputPath = etrading::FolderConfig::getGoogleTestInputPath() / inputPath;
        }

		std::vector<std::string> allMatchingFiles;

		boost::regex my_regex( patternString );

		boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
		for( boost::filesystem::directory_iterator i( inputPath ); i != end_itr; ++i )
		{
			// Skip if not a file
			if ( ! boost::filesystem::is_regular_file( i->status() ) )
			{
				continue;
			}

			const std::string& filename = i->path().string();

			boost::smatch string_match;
			if ( ! boost::regex_match( filename, string_match, my_regex ) )
			{
				// Skip if this file does not match the pattern
				continue;
			}

			allMatchingFiles.push_back( filename );
		}

		return allMatchingFiles;
	};
	
	
	typedef std::tuple<std::vector<std::string>, std::vector<etrading::ContainedTypeEnum>, etrading::VariantMatrix>  TableInfo;

	/* @brief			A helper function which converts a LAStringMatrix into a VariantMatrix
	*                   If the input LAStringMatrix is empty, creates a dummy VariantMatrix containing two blank columns 
	*  @param [in]		stringMatrix		The input LAStringMatrix
	*  @returns			The corresponding VariantMatrix
	*/
	etrading::VariantMatrix convertStringMatrixToVariantMatrix( LAStringMatrix stringMatrix )
	{
		etrading::VariantMatrix variantMatrix;

		const size_t numRows = stringMatrix.size();
		if ( numRows > 0 )
		{
			const size_t numCols = stringMatrix[0].size();

			// Transpose the matrix at the same time as converting to Variant
			for (size_t j=0; j<numCols; j++)
			{
				etrading::VariantVector variantVector;
				for (size_t i=0; i<numRows; i++)
				{
					variantVector.push_back( stringMatrix[i][j] );
				}
				variantMatrix.push_back( variantVector );
			}
		}
		else
		{
			// The input LAStringMatrix is empty.
			// Create a default variantMatrix with 2 columns of dummy data.
			// This simulates an empty block in Excel.
			etrading::VariantVector dummyVector ( 1, "" );
			variantMatrix.push_back( dummyVector );
			variantMatrix.push_back( dummyVector );
		}

		return variantMatrix;
	}
	
	/* @brief			Builds a "TableInfo" tuple from a LAStringMatrix of marketdata
	*                   This tuple consists of columnNames, columnTypes and the actual data values.
	*  @param [in]		marketDataBlock		A LAStringMatrix containing key/value market data values
	*/
	TableInfo getTableInfoFromStringMatrix( const LAStringMatrix& marketDataBlock )
	{
		etrading::VariantMatrix dataValues =  convertStringMatrixToVariantMatrix( marketDataBlock );
		size_t numColumns = dataValues.size();

		// EnumTypes for each column
		std::vector<etrading::ContainedTypeEnum> columnEnumTypes = etrading::Variant::getContainedTypeInfo( dataValues );

		// Construct dummy column headings
		std::vector<int> nColCounters;
        boost::push_back( nColCounters, boost::irange( 1, static_cast<int>( numColumns ) + 1 ) );
		std::vector<std::string> columnNames( numColumns, std::string( "COL_" ) );
        columnNames = etrading::zip_paste<std::vector<std::string>, std::vector<int>, std::string>( columnNames, nColCounters );

		// Construct the TableInfo
        return std::make_tuple( columnNames, columnEnumTypes, dataValues );
	}

	/*
	 * Given an inputFile, read the file and construct the bond from the specified BondGenerator
	 * Returns a string corresponding to the bond object name in the LWO cache.
	 */
	std::string createBondFromFileName( const std::string& inputFile )
    {
		// Build the Bond Object
		const ReadDataFile::Load createBondInputFile( inputFile.c_str() );
        std::string bondObjectName     = createBondInputFile["bondObjectName"];
        std::string bondGeneratorName  = createBondInputFile["bondGeneratorName"];
        LAStringMatrix bondExpressionLVB = createBondInputFile["expressionLVB"];
        bool validateKeys              = createBondInputFile["validateKeys"];
        
		std::string objectName = validation::tryMeLWOBondCreateFromGenerator( bondObjectName, bondGeneratorName, bondExpressionLVB, validateKeys );
		return objectName;
    };

	/*
	 * Given an inputFile, read the file and construct a BondCurve object
	 * Returns a string corresponding to the bondcurve object name in the LWO cache.
	 */
	std::string createBondCurveFromFileName( const std::string& bondCurveFileName )
	{
		etrading::ReadDataFile::Load bondCurveFileObj = etrading::ReadDataFile::Load( bondCurveFileName.c_str() );
		const std::string bondCurveName		= bondCurveFileObj[ "objectName" ];
		const LAStringMatrix modelProperties	= bondCurveFileObj[ "BONDCURVE_PROPERTIES" ];
		const LAStringMatrix bondMarketData	= bondCurveFileObj[ "BONDCURVE_MARKETDATA" ];

		std::vector<std::string> propertyNames;
		propertyNames.push_back( "BONDCURVE_PROPERTIES" );
		propertyNames.push_back( "BONDCURVE_MARKETDATA" );

		std::vector<validation::TableInfo> infoBlocks;
		infoBlocks.push_back( getTableInfoFromStringMatrix( modelProperties ));
		infoBlocks.push_back( getTableInfoFromStringMatrix( bondMarketData ));

		std::string objectName = validation::tryMeBondCurveCreate( bondCurveName, propertyNames, infoBlocks );
		return objectName;
	}

	/*
	 * Given an inputFile, read the file and construct a spread BondCurve object
	 * Returns a string corresponding to the bondcurve object name in the LWO cache.
	 */
	std::string createBondSpreadCurveFromFileName( const std::string& bondCurveFileName )
	{
		etrading::ReadDataFile::Load bondCurveFileObj = etrading::ReadDataFile::Load( bondCurveFileName.c_str() );
		const std::string bondspreadCurveName	= bondCurveFileObj[ "objectName" ];
		const LAStringMatrix modelProperties		= bondCurveFileObj[ "BONDSPREADCURVE_PROPERTIES" ]; // Contains spread and benchmarkBondCurve 

		std::vector<std::string> propertyNames;
		propertyNames.push_back( "BONDSPREADCURVE_PROPERTIES" );

		std::vector<validation::TableInfo> infoBlocks;
		infoBlocks.push_back( getTableInfoFromStringMatrix( modelProperties ));

		std::string objectName = validation::tryMeBondCurveCreate( bondspreadCurveName, propertyNames, infoBlocks );
		return objectName;
	}


	void checkBondCurveCalibration( const AnyTypeMatrix& actualResultsMatrix, const std::string& expectedCalibrationResultsFileName, const double& tolerance )
	{
		const size_t nRows = actualResultsMatrix.size();
		ASSERT_EQ( nRows, 15 ) << "#Error: Expected 15 rows of calibration results";  // One row per calibration point
		
		const size_t nCols = actualResultsMatrix[0].size();
		ASSERT_EQ( nCols, 3) << "#Error: Expected 3 columns of calibration results";  // MaturityDate, HazardRate, SurvivialProbability, DefaultProbability

		if ( etrading::CreateDataFile::rebaseResultsEnabled() )
		{
			etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
			LAStringVector v = LAString( expectedCalibrationResultsFileName.c_str() ).toToken( '/' );
            LAString resultFileName = v.back();
			etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName ) );
            file.write( "output", actualResultsMatrix );
		}
		else
		{
			etrading::ReadDataFile::Load expectedBondCurveCalibrationFileObj = etrading::ReadDataFile::Load( expectedCalibrationResultsFileName.c_str() );		
			LAStringMatrix expectedResults = expectedBondCurveCalibrationFileObj[ "output" ];
			ASSERT_EQ( nRows, expectedResults.size() ) << "#Error: Expected number of rows differ";
			ASSERT_EQ( nCols, expectedResults[0].size() ) << "#Error: Expected number of columns differ";

			for ( size_t row=0; row<nRows; row++ )
			{
				for( size_t col=0; col<nCols; col++ )
				{
					AnyType anyValue = actualResultsMatrix[row][col];

					double calculatedValueAsDouble = 0.;
					if ( col == 0)
					{
						// First column contains a date
						calculatedValueAsDouble = boost::get<int>( anyValue );
					}
					else
					{
						// All other columns are doubles
						calculatedValueAsDouble = boost::get<double>( anyValue );
					}
				
					LAString expectedValue = expectedResults[row][col];
					double expectedValueAsDouble = expectedValue.getDoubleValue();

					EXPECT_NEAR( calculatedValueAsDouble, expectedValueAsDouble, tolerance ) << "Difference in calibration parameters at " << row << ", " << col ;
				}
			}

		}
	}

	void buildBondsForCalibrationAndBondCurve()
	{
		// Build the portfolio of treasury bonds used to calibrate the curve

		// Get all bond CreateFromGenerator input files in TEST_DIR folder
		std::vector<std::string> allMatchingFiles = getFilesMatchingPattern( TEST_DIR, treasuryCreateInputFilePattern );

		// For each matching file in TEST_DIR create the corresponding bond object
		for (size_t i=0; i< allMatchingFiles.size(); i++ )
		{
			// Create the bond
			std::string inputFile = allMatchingFiles[ i ];
			std::string bondObjectName = createBondFromFileName( inputFile );
		}

		// Create the Bond Curve
		createBondCurveFromFileName( createBondCurve );
	}

}

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestBondCurves);

	//----------------------------------------------------------------------------------------

    TEST_F( TestBondCurves, SNAPSHOT_tryMeBondCurveNelsonSiegelCalibrate )
    {

        // Load the Nelson-Siegel Calibration inputs
        const ReadDataFile::Load nsCalibrationInputFile( nelsonSiegelCalibrate.c_str() );

		// Initial starting point for calibration parameters
		// The calibration replaces these values with the solution
		etrading::NelsonSiegelSvenssonParameters initialGuess;
        initialGuess.beta0_   = nsCalibrationInputFile["beta0"];
		initialGuess.beta1_   = nsCalibrationInputFile["beta1"];
		initialGuess.beta2_   = nsCalibrationInputFile["beta2"];
		initialGuess.lambda1_ = nsCalibrationInputFile["lambda"];

		// Parameter bounds
		const std::vector<double> lowerBounds = nsCalibrationInputFile["lowerBounds"];
		const std::vector<double> upperBounds = nsCalibrationInputFile["upperBounds"];

		// Solver end conditions
		const unsigned int maxIterations = nsCalibrationInputFile["maxIterations"];
		const int maxStationaryStateIterations = nsCalibrationInputFile["maxStationaryStateIterations"];

		// Bond market data to fit
		const std::vector<double> bondMaturities = nsCalibrationInputFile["bondMaturities"];
		const std::vector<double> bondYields     = nsCalibrationInputFile["bondYields"];

		etrading::NelsonSiegelSvenssonCalibrationResults calibrationResult = validation::tryMeBondCurveNelsonSiegelCalibrate( bondMaturities, bondYields,
																														 initialGuess,
																														 maxIterations, maxStationaryStateIterations,
																														 lowerBounds, upperBounds );

		// Construct vector with Nelson-Siegel calibration parameters
		std::vector<double> actualCalibration( 4 );
		actualCalibration[0] = calibrationResult.parameters_.beta0_;
		actualCalibration[1] = calibrationResult.parameters_.beta1_;
		actualCalibration[2] = calibrationResult.parameters_.beta2_;
		actualCalibration[3] = calibrationResult.parameters_.lambda1_;

#ifdef GTEST32
		const double tolerance = 1.e-9;
        LAString outputFileName = nelsonSiegelCalibrate_outputs_32.c_str();
#else
        LAString outputFileName = nelsonSiegelCalibrate_outputs_64.c_str();
		const double tolerance = 1.e-5; // Reduce tolerance for 64bit. Build server gives slightly different result compared to local PC.
#endif
        CheckTestResultsAndRebaseOnRequest( actualCalibration, TEST_DIR, outputFileName, tolerance );
    }



    TEST_F( TestBondCurves, SNAPSHOT_tryMeBondCurveSvenssonCalibrate )
    {
        // Load the Nelson-Siegel Calibration inputs
        const ReadDataFile::Load svenssonCalibrationInputFile( svenssonCalibrate.c_str() );

		// Initial starting point for calibration parameters
		// The calibration replaces these values with the solution
		etrading::NelsonSiegelSvenssonParameters initialGuess;
        initialGuess.beta0_  = svenssonCalibrationInputFile["beta0"];
		initialGuess.beta1_  = svenssonCalibrationInputFile["beta1"];
		initialGuess.beta2_  = svenssonCalibrationInputFile["beta2"];
		initialGuess.beta3_  = svenssonCalibrationInputFile["beta3"];
		initialGuess.lambda1_ = svenssonCalibrationInputFile["lambda1"];
		initialGuess.lambda2_ = svenssonCalibrationInputFile["lambda2"];

		// Parameter bounds
		const std::vector<double> lowerBounds = svenssonCalibrationInputFile["lowerBounds"];
		const std::vector<double> upperBounds = svenssonCalibrationInputFile["upperBounds"];

		// Solver end conditions
		const unsigned int maxIterations = svenssonCalibrationInputFile["maxIterations"];
		const int maxStationaryStateIterations = svenssonCalibrationInputFile["maxStationaryStateIterations"];

		// Bond market data to fit
		const std::vector<double> bondMaturities = svenssonCalibrationInputFile["bondMaturities"];
		const std::vector<double> bondYields     = svenssonCalibrationInputFile["bondYields"];

		etrading::NelsonSiegelSvenssonCalibrationResults calibrationResult = validation::tryMeBondCurveSvenssonCalibrate( bondMaturities, bondYields,
																													 initialGuess,
																													 maxIterations, maxStationaryStateIterations,
																													 lowerBounds, upperBounds );


        // Compare Results

		// Construct vector with Nelson-Siegel calibration parameters
		std::vector<double> actualCalibration( 6 );
		actualCalibration[0] = calibrationResult.parameters_.beta0_;
		actualCalibration[1] = calibrationResult.parameters_.beta1_;
		actualCalibration[2] = calibrationResult.parameters_.beta2_;
		actualCalibration[3] = calibrationResult.parameters_.beta3_;
		actualCalibration[4] = calibrationResult.parameters_.lambda1_;
		actualCalibration[5] = calibrationResult.parameters_.lambda2_;

		#ifdef GTEST32
		const double tolerance = 1.e-9;
        LAString outputFileName = svenssonCalibrate_outputs_32.c_str();
#else
		const double tolerance = 1.e-4; // Reduce tolerance for 64bit. Build server gives slightly different result compared to local PC.
        LAString outputFileName = svenssonCalibrate_outputs_64.c_str();
#endif

        CheckTestResultsAndRebaseOnRequest( actualCalibration, TEST_DIR, outputFileName, tolerance );
    }

	TEST_F( TestBondCurves, SNAPSHOT_tryMeBondCurvePolynomialCalibrate )
    {

        // Load the Polynomial Calibration inputs
        const ReadDataFile::Load polynomialCalibrationInputFile( polynomialCalibrate.c_str() );

		// Parameter bounds
		const unsigned int polynomialOrder = polynomialCalibrationInputFile["polynomialOrder"];
		const double lowerBound = polynomialCalibrationInputFile["lowerBound"];
		const double upperBound = polynomialCalibrationInputFile["upperBound"];

		// Solver end conditions
		const unsigned int maxIterations = polynomialCalibrationInputFile["maxIterations"];
		const unsigned int maxStationaryStateIterations = polynomialCalibrationInputFile["maxStationaryStateIterations"];

		// Bond market data to fit
		const std::vector<double> bondMaturities = polynomialCalibrationInputFile["bondMaturities"];
		const std::vector<double> bondYields     = polynomialCalibrationInputFile["bondYields"];

		etrading::PolynomialCalibrationResults calibrationResult = validation::tryMeBondCurvePolynomialCalibrate( polynomialOrder,
																														bondMaturities,
																														bondYields,
																														maxIterations,
																														maxStationaryStateIterations,
																														lowerBound,
																														upperBound );
        // Compare Results
		DoubleVector actualCoefficients = calibrationResult.coefficients_;

#ifdef GTEST32
		const double tolerance = 1.e-9;
        LAString outputFileName = polynomialCalibrate_outputs_32.c_str();
#else
        LAString outputFileName = polynomialCalibrate_outputs_64.c_str();
		const double tolerance = 1.e-4; // Reduce tolerance for 64bit. Build server gives slightly different result compared to local PC.
#endif
        CheckTestResultsAndRebaseOnRequest( actualCoefficients, TEST_DIR, outputFileName, tolerance );
    }


    TEST_F( TestBondCurves, SNAPSHOT_tryMeBondCurveNelsonSiegelYield )
    {
        // Load the Nelson-Siegel Bond Yield inputs
        const ReadDataFile::Load nsBondYieldInputFile( nelsonSiegelBondYield.c_str() );

		// Calibrated model values
        double beta0  = nsBondYieldInputFile["beta0"];
		double beta1  = nsBondYieldInputFile["beta1"];
		double beta2  = nsBondYieldInputFile["beta2"];
		double lambda = nsBondYieldInputFile["lambda"];

		// Bond maturities to calculate
		const std::vector<double> bondMaturities = nsBondYieldInputFile["maturities"];

		const std::vector<double> calculatedYields = validation::tryMeBondCurveNelsonSiegelYield( beta0, beta1, beta2, lambda, bondMaturities );

        // Compare Results
        const double tolerance = 0.000000001;

        CheckTestResultsAndRebaseOnRequest( calculatedYields, TEST_DIR, LAString( nelsonSiegelBondYield_outputs.c_str() ), tolerance );

    }

    TEST_F( TestBondCurves, SNAPSHOT_tryMeBondCurveSvenssonYield )
    {
        // Load the Nelson-Siegel Bond Yield inputs
        const ReadDataFile::Load svenssonBondYieldInputFile( svenssonBondYield.c_str() );

		// Calibrated model values
        double beta0   = svenssonBondYieldInputFile["beta0"];
		double beta1   = svenssonBondYieldInputFile["beta1"];
		double beta2   = svenssonBondYieldInputFile["beta2"];
		double beta3   = svenssonBondYieldInputFile["beta3"];
		double lambda1 = svenssonBondYieldInputFile["lambda1"];
		double lambda2 = svenssonBondYieldInputFile["lambda2"];

		// Bond maturities to calculate
		const std::vector<double> bondMaturities = svenssonBondYieldInputFile["maturities"];

		const std::vector<double> calculatedYields = validation::tryMeBondCurveSvenssonYield( beta0, beta1, beta2, beta3, lambda1, lambda2, bondMaturities );

        // Compare Results
        const double tolerance = 0.000000001;

        CheckTestResultsAndRebaseOnRequest( calculatedYields, TEST_DIR, LAString( svenssonBondYield_outputs.c_str() ), tolerance );

    }

	TEST_F( TestBondCurves, SNAPSHOT_tryMeBondCurvePolynomialYield )
    {
        // Load the Nelson-Siegel Bond Yield inputs
        const ReadDataFile::Load polynomialBondYieldInputFile( polynomialBondYield.c_str() );

		// Calibrated model values
        const DoubleVector coefficients  = polynomialBondYieldInputFile["coefficients"];

		// Bond maturities to calculate
		const std::vector<double> bondMaturities = polynomialBondYieldInputFile["maturities"];

		const std::vector<double> calculatedYields = validation::tryMeBondCurvePolynomialYield( coefficients, bondMaturities );

        // Compare Results
        const double tolerance = 0.000000001;

        CheckTestResultsAndRebaseOnRequest( calculatedYields, TEST_DIR, LAString( polynomialBondYield_outputs.c_str() ), tolerance );
    }

	// Build a bond-curve from individual bond quotes and check the calibrated yields agree with a recorded snapshot.
    TEST_F( TestBondCurves, SNAPSHOT_tryMeBondCurveBuildAndDisplay )
    {
		// Build the calibration bonds and the bond curve
		buildBondsForCalibrationAndBondCurve();

		// Display the Bond Curve Calibration
        const ReadDataFile::Load bondCurveDisplayInputFile( bondCurveDisplay_inputs.c_str() );
		const std::string bondCurveObjectName  = bondCurveDisplayInputFile["bondCurveName"];

		const AnyTypeMatrix calibratedYields = validation::tryMeBondCurveDisplay( bondCurveObjectName );

		const double tolerance = 1.0e-9;
		checkBondCurveCalibration( calibratedYields, bondCurveDisplay_outputs, tolerance );
	}

	// Price a portfolio of bonds from yield-to-maturity and from the bond-curve
	// Expect the bond prices to match closely.
	TEST_F( TestBondCurves, CONSISTENCY_BondCurve_CheckBondPrices )
    {
		// Build the calibration bonds and the bond curve
		buildBondsForCalibrationAndBondCurve();

		// For each bond, price from yield
		std::vector<std::string> allMatchingPriceFromYieldFiles = getFilesMatchingPattern( TEST_DIR, treasuryPriceFromYieldPattern );

		// For each matching file in TEST_DIR invoke the corresponding price from yield and price from bond curve
		const double tolerance = 1.0e-9;
		for (size_t i=0; i< allMatchingPriceFromYieldFiles.size(); i++ )
		{
			// Calculate bond price from yield
			const std::string inputFileName = allMatchingPriceFromYieldFiles[ i ];
			const ReadDataFile::Load priceFromYieldFile( inputFileName.c_str() );

			const std::string bondObjectName          = priceFromYieldFile[ "bondObjectName" ];
			const std::vector<LADate> settlementDates = priceFromYieldFile[ "settlementDates" ];
			const std::vector<double> yields          = priceFromYieldFile[ "yields" ];
			const DoubleVector priceFromYields = validation::tryMeLWOBondPrice( bondObjectName, settlementDates, yields );
			const double priceFromYield = priceFromYields[0];

			// Load the corresponding file to price the bond from bond curve
			const std::string priceFromBondCurveFileName = bondObjectName + treasuryPriceFromBondCurveSuffix;
			const boost::filesystem::path fullFilePath = etrading::FolderConfig::getGoogleTestInputPath() / TEST_DIR / priceFromBondCurveFileName;
			const std::string fullFileNameWithPath = fullFilePath.string();
			const ReadDataFile::Load priceFromBondCurveFile( fullFileNameWithPath.c_str() );

			// Calculate bond price from bond curve
			const LADate settlementDate     = priceFromBondCurveFile[ "settlementDate" ];
			const std::string bondCurveName = priceFromBondCurveFile[ "bondCurveName" ];
			const double priceFromBondCurve = validation::tryMeLWOBondPriceFromBondCurve( bondObjectName, settlementDate, bondCurveName );

			// Compare the two prices for consistency
			EXPECT_NEAR( priceFromYield, priceFromBondCurve, tolerance ) << "Difference in priceFromYield vs priceFromBondCurve: " << bondObjectName;
		}

	}

	// Build a Bond Curve and use it to calculate the yield of a portfolio of bonds.
	// Check that the calculated yield matches the bond yield-to-maturity.
	TEST_F( TestBondCurves, CONSISTENCY_BondCurve_CheckBondYields )
    {
		// Build the calibration bonds and the bond curve
		buildBondsForCalibrationAndBondCurve();

		// For each bond, grab the price from yield files, but instead of calculating the price,
		// compare the yield stored in the file with the yield calculated using the bond curve
		std::vector<std::string> allMatchingPriceFromYieldFiles = getFilesMatchingPattern( TEST_DIR, treasuryPriceFromYieldPattern );

		// For each matching file in TEST_DIR invoke the corresponding price from yield and price from bond curve
		const double tolerance = 1.0e-9;
		for (size_t i=0; i< allMatchingPriceFromYieldFiles.size(); i++ )
		{
			// Calculate bond price from yield
			const std::string inputFileName = allMatchingPriceFromYieldFiles[ i ];
			const ReadDataFile::Load priceFromYieldFile( inputFileName.c_str() );

			const std::string bondObjectName          = priceFromYieldFile[ "bondObjectName" ];
			const std::vector<LADate> settlementDates = priceFromYieldFile[ "settlementDates" ];
			const std::vector<double> yields          = priceFromYieldFile[ "yields" ];
			const double bondYTM = yields[0];

			// Load the corresponding file to price the bond from bond curve, and use it to calculate the YIELD from bond curve
			const std::string priceFromBondCurveFileName = bondObjectName + treasuryPriceFromBondCurveSuffix;
			const boost::filesystem::path fullFilePath = etrading::FolderConfig::getGoogleTestInputPath() / TEST_DIR / priceFromBondCurveFileName;
			const std::string fullFileNameWithPath = fullFilePath.string();
			const ReadDataFile::Load priceFromBondCurveFile( fullFileNameWithPath.c_str() );

			// Calculate bond price from bond curve
			const LADate settlementDate     = priceFromBondCurveFile[ "settlementDate" ];
			const std::string bondCurveName = priceFromBondCurveFile[ "bondCurveName" ];

			// Calculate the YIELD from bond curve
			const double yieldFromBondCurve = validation::tryMeLWOBondYieldFromBondCurve( bondObjectName, settlementDate, bondCurveName );

			// Compare the two prices for consistency
			EXPECT_NEAR( bondYTM, yieldFromBondCurve, tolerance ) << "Difference in bond yield-to-maturity vs yieldFromBondCurve: " << bondObjectName;
		}
	}

	// Price a portfolio of bonds from yield-to-maturity and from a bond-curve built as a spread to a benchmark bond curve.
	// Because of the yield spread, Expect the bond price calculated from the bondSpreadCurve to be strictly LESS
	// than the bond price calculated from yield-to-maturity.
	TEST_F( TestBondCurves, CONSISTENCY_BondCurve_SpreadToBenchmarkCurve )
	{
		const double smaller = 2.718;
		const double larger = 3.14;
		// Build the calibration bonds and the bond curve
		buildBondsForCalibrationAndBondCurve();	

		// build spread bond curve from government benchmark curve
		std::string bondSpreadCurveName = createBondSpreadCurveFromFileName( corporateSpreadBondCurve );
		
		// For each bond, calculate price from yield
		std::vector<std::string> allMatchingPriceFromYieldFiles = getFilesMatchingPattern( TEST_DIR, treasuryPriceFromYieldPattern );

		// For each matching file in TEST_DIR invoke the corresponding price from yield and price from bond curve
		for (size_t i=0; i< allMatchingPriceFromYieldFiles.size(); i++ )
		{
			// Calculate bond price from yield
			const std::string inputFileName = allMatchingPriceFromYieldFiles[ i ];
			const ReadDataFile::Load priceFromYieldFile( inputFileName.c_str() );

			const std::string bondObjectName          = priceFromYieldFile[ "bondObjectName" ];
			const std::vector<LADate> settlementDates = priceFromYieldFile[ "settlementDates" ];
			const std::vector<double> yields          = priceFromYieldFile[ "yields" ];
			const DoubleVector priceFromYields = validation::tryMeLWOBondPrice( bondObjectName, settlementDates, yields );
			const double priceFromYield = priceFromYields[0];

			// Load the corresponding file to price the bond from bond curve
			const std::string priceFromBondCurveFileName = bondObjectName + treasuryPriceFromBondCurveSuffix;
			const boost::filesystem::path fullFilePath = etrading::FolderConfig::getGoogleTestInputPath() / TEST_DIR / priceFromBondCurveFileName;
			const std::string fullFileNameWithPath = fullFilePath.string();
			const ReadDataFile::Load priceFromBondCurveFile( fullFileNameWithPath.c_str() );

			// Calculate bond price from bond SPREAD curve
			const LADate settlementDate     = priceFromBondCurveFile[ "settlementDate" ];
			const double priceFromBondCurve = validation::tryMeLWOBondPriceFromBondCurve( bondObjectName, settlementDate, bondSpreadCurveName );

			// Compare the two prices for consistency:
			// The price calculated from the spread bond curve should be strictly less than the price from yield-to-maturity

			// Expect priceFromBondCurve < priceFromYield
			EXPECT_LT( priceFromBondCurve, priceFromYield ) << "Expecting priceFromYield to be larger than priceFromBondSpreadCurve: " << bondObjectName;
		}
	}

};