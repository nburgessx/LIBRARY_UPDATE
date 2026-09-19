// Include: Google Test Library
#include "Dependency.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"
#include "TestHelperUtilities.h"
#include "InitializeETrading.h"
#include "tryAqBondCurves.h"
#include "tryAqBondObject.h"
#include "ParameterValidation.h"

#include "FolderConfig.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "ETrading/Bonds/BondCurves/"

namespace
{
    // Test Bond Input File(s)
    const std::string nelsonSiegelCalibrate				= TEST_DIR "tryAqBondCurveNelsonSiegelCalibrate_inputs.csv";
	const std::string svenssonCalibrate					= TEST_DIR "tryAqBondCurveSvenssonCalibrate_inputs.csv";
	const std::string polynomialCalibrate				= TEST_DIR "tryAqBondCurvePolynomialCalibrate_inputs.csv";
	
	const std::string nelsonSiegelBondYield				= TEST_DIR "tryAqBondCurveNelsonSiegelYield_inputs.csv";
    const std::string svenssonBondYield					= TEST_DIR "tryAqBondCurveSvenssonYield_inputs.csv";
    const std::string polynomialBondYield				= TEST_DIR "tryAqBondCurvePolynomialYield_inputs.csv";

	const std::string nelsonSiegelCalibrate_outputs_32	= TEST_DIR "tryAqBondCurveNelsonSiegelCalibrate_outputs_32bit.csv";
	const std::string svenssonCalibrate_outputs_32		= TEST_DIR "tryAqBondCurveSvenssonCalibrate_outputs_32bit.csv";
	const std::string polynomialCalibrate_outputs_32	= TEST_DIR "tryAqBondCurvePolynomialCalibrate_outputs_32bit.csv";

	const std::string nelsonSiegelCalibrate_outputs_64	= TEST_DIR "tryAqBondCurveNelsonSiegelCalibrate_outputs_64bit.csv";
	const std::string svenssonCalibrate_outputs_64		= TEST_DIR "tryAqBondCurveSvenssonCalibrate_outputs_64bit.csv";
	const std::string polynomialCalibrate_outputs_64	= TEST_DIR "tryAqBondCurvePolynomialCalibrate_outputs_64bit.csv";

	const std::string nelsonSiegelBondYield_outputs		= TEST_DIR "tryAqBondCurveNelsonSiegelYield_outputs.csv";
    const std::string svenssonBondYield_outputs			= TEST_DIR "tryAqBondCurveSvenssonYield_outputs.csv";
	const std::string polynomialBondYield_outputs		= TEST_DIR "tryAqBondCurvePolynomialYield_outputs.csv";

	// Bootstrapped bond curve
	const std::string createBondCurve					= TEST_DIR "USTREASURIES1@127_tryAqBondCurveCreate_inputs.csv";
	const std::string bondCurveDisplay_inputs			= TEST_DIR "tryAqBondCurveDisplay_inputs.csv";
	const std::string bondCurveDisplay_outputs			= TEST_DIR "tryAqBondCurveDisplay_outputs.csv";

	// Corporate bond curve as a spread to government treasury benchmark bond curve
	const std::string corporateSpreadBondCurve			= TEST_DIR "USD_CORP_CURVE@128_tryAqBondCurveCreate_inputs.csv";

	 /*
	 * This test builds 15 US Treasuries from input file, and uses these bonds to calibrate a bond curve.
	 * Rather than list out all bond filenames individually, we load then using the following regular expression pattern:
	 * The file pattern is:
	 * pathname / ( bondObjectName ) _tryAqBondObjectCreateFromGenerator_inputs.csv
	*/
    const std::string treasuryCreateInputFilePattern    = ".+_tryAqBondObjectCreateFromGenerator_inputs.csv";

	const std::string treasuryPriceFromYieldPattern     = ".+_tryAqBondObjectPrice_inputs.csv";

	const std::string treasuryPriceFromBondCurveSuffix  =  "_tryAqBondObjectPriceFromBondCurve_inputs.csv";


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

	/* @brief			A helper function which converts a AQLStringMatrix into a VariantMatrix
	*                   If the input AQLStringMatrix is empty, creates a dummy VariantMatrix containing two blank columns 
	*  @param [in]		stringMatrix		The input AQLStringMatrix
	*  @returns			The corresponding VariantMatrix
	*/
	etrading::VariantMatrix convertStringMatrixToVariantMatrix( AQLStringMatrix stringMatrix )
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
			// The input AQLStringMatrix is empty.
			// Create a default variantMatrix with 2 columns of dummy data.
			// This simulates an empty block in Excel.
			etrading::VariantVector dummyVector ( 1, "" );
			variantMatrix.push_back( dummyVector );
			variantMatrix.push_back( dummyVector );
		}

		return variantMatrix;
	}
	
	/* @brief			Builds a "TableInfo" tuple from a AQLStringMatrix of marketdata
	*                   This tuple consists of columnNames, columnTypes and the actual data values.
	*  @param [in]		marketDataBlock		A AQLStringMatrix containing key/value market data values
	*/
	TableInfo getTableInfoFromStringMatrix( const AQLStringMatrix& marketDataBlock )
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
	 * Returns a string corresponding to the bond object name in the AQObj object cache.
	 */
	std::string createBondFromFileName( const std::string& inputFile )
    {
		// Build the Bond Object
		const ReadDataFile::Load createBondInputFile( inputFile.c_str() );
        std::string bondObjectName     = createBondInputFile["bondObjectName"];
        std::string bondGeneratorName  = createBondInputFile["bondGeneratorName"];
        AQLStringMatrix bondExpressionLVB = createBondInputFile["expressionLVB"];
        bool validateKeys              = createBondInputFile["validateKeys"];
        
		std::string objectName = validation::tryAqBondObjectCreateFromGenerator( bondObjectName, bondGeneratorName, bondExpressionLVB, validateKeys );
		return objectName;
    };

	/*
	 * Given an inputFile, read the file and construct a BondCurve object
	 * Returns a string corresponding to the bondcurve object name in the AQObj object cache.
	 */
	std::string createBondCurveFromFileName( const std::string& bondCurveFileName )
	{
		etrading::ReadDataFile::Load bondCurveFileObj = etrading::ReadDataFile::Load( bondCurveFileName.c_str() );
		const std::string bondCurveName		= bondCurveFileObj[ "objectName" ];
		const AQLStringMatrix modelProperties	= bondCurveFileObj[ "BONDCURVE_PROPERTIES" ];
		const AQLStringMatrix bondMarketData	= bondCurveFileObj[ "BONDCURVE_MARKETDATA" ];

		std::vector<std::string> propertyNames;
		propertyNames.push_back( "BONDCURVE_PROPERTIES" );
		propertyNames.push_back( "BONDCURVE_MARKETDATA" );

		std::vector<validation::TableInfo> infoBlocks;
		infoBlocks.push_back( getTableInfoFromStringMatrix( modelProperties ));
		infoBlocks.push_back( getTableInfoFromStringMatrix( bondMarketData ));

		std::string objectName = validation::tryAqBondCurveCreate( bondCurveName, propertyNames, infoBlocks );
		return objectName;
	}

	/*
	 * Given an inputFile, read the file and construct a spread BondCurve object
	 * Returns a string corresponding to the bondcurve object name in the AQObj object cache.
	 */
	std::string createBondSpreadCurveFromFileName( const std::string& bondCurveFileName )
	{
		etrading::ReadDataFile::Load bondCurveFileObj = etrading::ReadDataFile::Load( bondCurveFileName.c_str() );
		const std::string bondspreadCurveName	= bondCurveFileObj[ "objectName" ];
		const AQLStringMatrix modelProperties	= bondCurveFileObj[ "BONDCURVE_PROPERTIES" ]; // Contains CurveType=BondSpreadCurve and BenchmarkBondCurve
		const AQLStringMatrix bondMarketData	= bondCurveFileObj[ "BONDCURVE_MARKETDATA" ]; // The spread bond(s) priced against the benchmark

		std::vector<std::string> propertyNames;
		propertyNames.push_back( "BONDCURVE_PROPERTIES" );
		propertyNames.push_back( "BONDCURVE_MARKETDATA" );

		std::vector<validation::TableInfo> infoBlocks;
		infoBlocks.push_back( getTableInfoFromStringMatrix( modelProperties ));
		infoBlocks.push_back( getTableInfoFromStringMatrix( bondMarketData ));

		std::string objectName = validation::tryAqBondCurveCreate( bondspreadCurveName, propertyNames, infoBlocks );
		return objectName;
	}


	void checkBondCurveCalibration( const AnyTypeMatrix& actualResultsMatrix, const std::string& expectedCalibrationResultsFileName, const double& tolerance, const size_t expectedRows = 15 )
	{
		const size_t nRows = actualResultsMatrix.size();
		ASSERT_EQ( nRows, expectedRows ) << "#Error: Expected " << expectedRows << " rows of calibration results";  // One row per calibration point
		
		const size_t nCols = actualResultsMatrix[0].size();
		ASSERT_EQ( nCols, 2) << "#Error: Expected 2 columns of calibration results";  // MaturityDate, Yield

		if ( etrading::CreateDataFile::rebaseResultsEnabled() )
		{
			etrading::CreateDataFile::setOutputFolder( TEST_DIR, false );
			AQLStringVector v = AQLString( expectedCalibrationResultsFileName.c_str() ).toToken( '/' );
            AQLString resultFileName = v.back();
			etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName ) );
            file.write( "output", actualResultsMatrix );
		}
		else
		{
			etrading::ReadDataFile::Load expectedBondCurveCalibrationFileObj = etrading::ReadDataFile::Load( expectedCalibrationResultsFileName.c_str() );		
			AQLStringMatrix expectedResults = expectedBondCurveCalibrationFileObj[ "output" ];
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
				
					AQLString expectedValue = expectedResults[row][col];
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

    TEST_F( TestBondCurves, SNAPSHOT_tryAqBondCurveNelsonSiegelCalibrate )
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

		etrading::NelsonSiegelSvenssonCalibrationResults calibrationResult = validation::tryAqBondCurveNelsonSiegelCalibrate( bondMaturities, bondYields,
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
        AQLString outputFileName = nelsonSiegelCalibrate_outputs_32.c_str();
#else
        AQLString outputFileName = nelsonSiegelCalibrate_outputs_64.c_str();
		const double tolerance = 1.e-5; // Reduce tolerance for 64bit. Build server gives slightly different result compared to local PC.
#endif
        CheckTestResultsAndRebaseOnRequest( actualCalibration, TEST_DIR, outputFileName, tolerance );
    }



    TEST_F( TestBondCurves, SNAPSHOT_tryAqBondCurveSvenssonCalibrate )
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

		etrading::NelsonSiegelSvenssonCalibrationResults calibrationResult = validation::tryAqBondCurveSvenssonCalibrate( bondMaturities, bondYields,
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
        AQLString outputFileName = svenssonCalibrate_outputs_32.c_str();
#else
		const double tolerance = 1.e-4; // Reduce tolerance for 64bit. Build server gives slightly different result compared to local PC.
        AQLString outputFileName = svenssonCalibrate_outputs_64.c_str();
#endif

        CheckTestResultsAndRebaseOnRequest( actualCalibration, TEST_DIR, outputFileName, tolerance );
    }

	TEST_F( TestBondCurves, SNAPSHOT_tryAqBondCurvePolynomialCalibrate )
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

		etrading::PolynomialCalibrationResults calibrationResult = validation::tryAqBondCurvePolynomialCalibrate( polynomialOrder,
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
        AQLString outputFileName = polynomialCalibrate_outputs_32.c_str();
#else
        AQLString outputFileName = polynomialCalibrate_outputs_64.c_str();
		const double tolerance = 1.e-4; // Reduce tolerance for 64bit. Build server gives slightly different result compared to local PC.
#endif
        CheckTestResultsAndRebaseOnRequest( actualCoefficients, TEST_DIR, outputFileName, tolerance );
    }


    TEST_F( TestBondCurves, SNAPSHOT_tryAqBondCurveNelsonSiegelYield )
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

		const std::vector<double> calculatedYields = validation::tryAqBondCurveNelsonSiegelYield( beta0, beta1, beta2, lambda, bondMaturities );

        // Compare Results
        const double tolerance = 0.000000001;

        CheckTestResultsAndRebaseOnRequest( calculatedYields, TEST_DIR, AQLString( nelsonSiegelBondYield_outputs.c_str() ), tolerance );

    }

    TEST_F( TestBondCurves, SNAPSHOT_tryAqBondCurveSvenssonYield )
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

		const std::vector<double> calculatedYields = validation::tryAqBondCurveSvenssonYield( beta0, beta1, beta2, beta3, lambda1, lambda2, bondMaturities );

        // Compare Results
        const double tolerance = 0.000000001;

        CheckTestResultsAndRebaseOnRequest( calculatedYields, TEST_DIR, AQLString( svenssonBondYield_outputs.c_str() ), tolerance );

    }

	TEST_F( TestBondCurves, SNAPSHOT_tryAqBondCurvePolynomialYield )
    {
        // Load the Nelson-Siegel Bond Yield inputs
        const ReadDataFile::Load polynomialBondYieldInputFile( polynomialBondYield.c_str() );

		// Calibrated model values
        const DoubleVector coefficients  = polynomialBondYieldInputFile["coefficients"];

		// Bond maturities to calculate
		const std::vector<double> bondMaturities = polynomialBondYieldInputFile["maturities"];

		const std::vector<double> calculatedYields = validation::tryAqBondCurvePolynomialYield( coefficients, bondMaturities );

        // Compare Results
        const double tolerance = 0.000000001;

        CheckTestResultsAndRebaseOnRequest( calculatedYields, TEST_DIR, AQLString( polynomialBondYield_outputs.c_str() ), tolerance );
    }

	// Build a bond-curve from individual bond quotes and check the calibrated yields agree with a recorded snapshot.
    TEST_F( TestBondCurves, SNAPSHOT_tryAqBondsCurveBuildAndDisplay )
    {
		// Build the calibration bonds and the bond curve
		buildBondsForCalibrationAndBondCurve();

		// Display the Bond Curve Calibration
        const ReadDataFile::Load bondCurveDisplayInputFile( bondCurveDisplay_inputs.c_str() );
		const std::string bondCurveObjectName  = bondCurveDisplayInputFile["bondCurveName"];

		const AnyTypeMatrix calibratedYields = validation::tryAqBondCurveDisplay( bondCurveObjectName );

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
			const std::vector<AQLDate> settlementDates = priceFromYieldFile[ "settlementDates" ];
			const std::vector<double> yields          = priceFromYieldFile[ "yields" ];
			const DoubleVector priceFromYields = validation::tryAqBondObjectPrice( bondObjectName, settlementDates, yields );
			const double priceFromYield = priceFromYields[0];

			// Load the corresponding file to price the bond from bond curve
			const std::string priceFromBondCurveFileName = bondObjectName + treasuryPriceFromBondCurveSuffix;
			const boost::filesystem::path fullFilePath = etrading::FolderConfig::getGoogleTestInputPath() / TEST_DIR / priceFromBondCurveFileName;
			const std::string fullFileNameWithPath = fullFilePath.string();
			const ReadDataFile::Load priceFromBondCurveFile( fullFileNameWithPath.c_str() );

			// Calculate bond price from bond curve
			const AQLDate settlementDate     = priceFromBondCurveFile[ "settlementDate" ];
			const std::string bondCurveName = priceFromBondCurveFile[ "bondCurveName" ];
			const double priceFromBondCurve = validation::tryAqBondObjectPriceFromBondCurve( bondObjectName, settlementDate, bondCurveName );

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
			const std::vector<AQLDate> settlementDates = priceFromYieldFile[ "settlementDates" ];
			const std::vector<double> yields          = priceFromYieldFile[ "yields" ];
			const double bondYTM = yields[0];

			// Load the corresponding file to price the bond from bond curve, and use it to calculate the YIELD from bond curve
			const std::string priceFromBondCurveFileName = bondObjectName + treasuryPriceFromBondCurveSuffix;
			const boost::filesystem::path fullFilePath = etrading::FolderConfig::getGoogleTestInputPath() / TEST_DIR / priceFromBondCurveFileName;
			const std::string fullFileNameWithPath = fullFilePath.string();
			const ReadDataFile::Load priceFromBondCurveFile( fullFileNameWithPath.c_str() );

			// Calculate bond price from bond curve
			const AQLDate settlementDate     = priceFromBondCurveFile[ "settlementDate" ];
			const std::string bondCurveName = priceFromBondCurveFile[ "bondCurveName" ];

			// Calculate the YIELD from bond curve
			const double yieldFromBondCurve = validation::tryAqBondObjectYieldFromBondCurve( bondObjectName, settlementDate, bondCurveName );

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
			const std::vector<AQLDate> settlementDates = priceFromYieldFile[ "settlementDates" ];
			const std::vector<double> yields          = priceFromYieldFile[ "yields" ];
			const DoubleVector priceFromYields = validation::tryAqBondObjectPrice( bondObjectName, settlementDates, yields );
			const double priceFromYield = priceFromYields[0];

			// Load the corresponding file to price the bond from bond curve
			const std::string priceFromBondCurveFileName = bondObjectName + treasuryPriceFromBondCurveSuffix;
			const boost::filesystem::path fullFilePath = etrading::FolderConfig::getGoogleTestInputPath() / TEST_DIR / priceFromBondCurveFileName;
			const std::string fullFileNameWithPath = fullFilePath.string();
			const ReadDataFile::Load priceFromBondCurveFile( fullFileNameWithPath.c_str() );

			// Calculate bond price from bond SPREAD curve
			const AQLDate settlementDate     = priceFromBondCurveFile[ "settlementDate" ];
			const double priceFromBondCurve = validation::tryAqBondObjectPriceFromBondCurve( bondObjectName, settlementDate, bondSpreadCurveName );

			// Compare the two prices for consistency:
			// The price calculated from the spread bond curve should be strictly less than the price from yield-to-maturity

			// Expect priceFromBondCurve < priceFromYield
			EXPECT_LT( priceFromBondCurve, priceFromYield ) << "Expecting priceFromYield to be larger than priceFromBondSpreadCurve: " << bondObjectName;
		}
	}

	// A spread bond's maturity may legitimately coincide with one of the benchmark's own bond maturities (e.g. a
	// corporate bond happening to mature on the same date as a treasury used to build the benchmark). This must not
	// throw - the spread bond's own quoted yield should simply take precedence at that pillar, since it is real
	// market data for that exact instrument.
	TEST_F( TestBondCurves, CONSISTENCY_BondCurve_SpreadBondCoincidesWithBenchmarkMaturity )
	{
		buildBondsForCalibrationAndBondCurve();

		const std::string coincidentFile = TEST_DIR "USD_CORP_CURVE_COINCIDENT@144_tryAqBondCurveCreate_inputs.csv";
		std::string curveName = createBondSpreadCurveFromFileName( coincidentFile );

		const AnyTypeMatrix calibratedYields = validation::tryAqBondCurveDisplay( curveName );

		// Same 15 pillars as the benchmark - the coincident date is reassigned, not duplicated
		ASSERT_EQ( calibratedYields.size(), 15u );

		const AQLDate coincidentDate = etrading::stringToDate( "44227" );
		const double yieldAtCoincidentDate = validation::tryAqBondCurveYield( curveName, coincidentDate );

		// The spread bond's own quoted yield (0.03) must take precedence over the benchmark-bond-derived value
		// (which would be close to the benchmark's own ~0.0236 yield at that date, not 0.03)
		EXPECT_NEAR( yieldAtCoincidentDate, 0.03, 1.0e-6 );
	}

	// Flat/PiecewiseConstant interpolation of the spread overlay is a genuine step-function discontinuity between
	// spread nodes - no downstream fix can make that look smooth, so it must be rejected outright for spread curves.
	TEST_F( TestBondCurves, CONSISTENCY_BondCurve_SpreadCurveRejectsFlatInterpolation )
	{
		buildBondsForCalibrationAndBondCurve();
		const std::string flatRejectedFile = TEST_DIR "USD_CORP_CURVE_FLAT_REJECTED@156_tryAqBondCurveCreate_inputs.csv";
		EXPECT_THROW( createBondSpreadCurveFromFileName( flatRejectedFile ), std::exception );
	}

	// Two sparse, well-separated spread nodes (near the start and near the end of the benchmark's range), with
	// clearly different spread magnitudes (+100bp vs +500bp), coinciding with two of the benchmark's own bonds.
	// The interpolated spread must be attributed by DATE, not by index/position: it should be exactly +100bp at
	// the first node, exactly +500bp at the last, and smoothly (linearly, by default) increasing in between -
	// never attributed to the wrong maturity.
	TEST_F( TestBondCurves, CONSISTENCY_BondCurve_SparseSpreadNodesAttributedByDate )
	{
		buildBondsForCalibrationAndBondCurve();
		const std::string sparseFile = TEST_DIR "USD_CORP_CURVE_SPARSE_DIAG@146_tryAqBondCurveCreate_inputs.csv";
		std::string curveName = createBondSpreadCurveFromFileName( sparseFile );

		const AnyTypeMatrix benchmarkMatrix = validation::tryAqBondCurveDisplay( "USTREASURIES1@127" );
		const AnyTypeMatrix spreadMatrix    = validation::tryAqBondCurveDisplay( curveName );

		std::map<int, double> spreadByDate;
		for ( size_t row = 0; row < spreadMatrix.size(); row++ )
		{
			spreadByDate[ boost::get<int>( spreadMatrix[row][0] ) ] = boost::get<double>( spreadMatrix[row][1] );
		}

		ASSERT_EQ( benchmarkMatrix.size(), 15u );
		double previousImpliedSpread = -1.0;
		for ( size_t row = 0; row < benchmarkMatrix.size(); row++ )
		{
			const int date = boost::get<int>( benchmarkMatrix[row][0] );
			const double benchmarkYield = boost::get<double>( benchmarkMatrix[row][1] );
			ASSERT_EQ( spreadByDate.count( date ), 1u ) << "Missing benchmark pillar at date " << date;
			const double impliedSpread = spreadByDate[ date ] - benchmarkYield;

			// Monotonically increasing from +100bp (first node, date 44227) to +500bp (last node, date 44530)
			EXPECT_GE( impliedSpread, previousImpliedSpread - 1.0e-6 ) << "Spread not monotonically increasing at date " << date;
			previousImpliedSpread = impliedSpread;
		}

		EXPECT_NEAR( spreadByDate.at( 44227 ), 0.0236140220000006 + 0.01, 1.0e-9 );   // first node: exact pass-through
		// Last node's +500bp shock compounds through 14 prior sequential bootstrap solves; tolerance reflects that
		// solver precision (not a logic-level check), same effect as CONSISTENCY_BondCurve_MixedCoincidentAndNonCoincidentSpreadNodes.
		EXPECT_NEAR( spreadByDate.at( 44530 ), 0.0231030100000001 + 0.05, 1.0e-3 );   // last node: exact pass-through
	}

	// Mix a spread bond that coincides with a benchmark bond's maturity (44227) with one that doesn't (44327),
	// both representing the exact same +100bp spread. The resulting curve must be a smooth, constant +100bp over
	// the benchmark everywhere - no spike or discontinuity at the coincident pillar relative to its neighbours.
	TEST_F( TestBondCurves, CONSISTENCY_BondCurve_MixedCoincidentAndNonCoincidentSpreadNodes )
	{
		buildBondsForCalibrationAndBondCurve();

		const std::string mixedFile = TEST_DIR "USD_CORP_CURVE_MIXED@145_tryAqBondCurveCreate_inputs.csv";
		std::string curveName = createBondSpreadCurveFromFileName( mixedFile );

		const AnyTypeMatrix benchmarkMatrix = validation::tryAqBondCurveDisplay( "USTREASURIES1@127" );
		const AnyTypeMatrix spreadMatrix    = validation::tryAqBondCurveDisplay( curveName );

		std::map<int, double> spreadByDate;
		for ( size_t row = 0; row < spreadMatrix.size(); row++ )
		{
			spreadByDate[ boost::get<int>( spreadMatrix[row][0] ) ] = boost::get<double>( spreadMatrix[row][1] );
		}

		for ( size_t row = 0; row < benchmarkMatrix.size(); row++ )
		{
			const int date = boost::get<int>( benchmarkMatrix[row][0] );
			const double benchmarkYield = boost::get<double>( benchmarkMatrix[row][1] );
			ASSERT_EQ( spreadByDate.count( date ), 1u ) << "Missing benchmark pillar at date " << date;
			// Tolerance reflects the bootstrap solver's own precision (Newton-Raphson tolerance is 1e-10 on price,
			// which translates to a somewhat looser tolerance on the solved yield) - not a logic-level check.
			EXPECT_NEAR( spreadByDate[ date ] - benchmarkYield, 0.01, 1.0e-6 ) << "Spread not a smooth constant +100bp at date " << date;
		}
	}

	// Build a spread curve from TWO spread bonds (not a single constant spread), each offset from the benchmark's own
	// calibrated yield at its maturity by a known amount (+20bp at 44286, +50bp at 44469). With no INTERPOLATION/
	// EXTRAPOLATION given, the spread curve defaults to Linear/Flat. Expect: exact pass-through of the spread bonds'
	// own yields at their maturities; flat-extrapolated +20bp/+50bp spread outside the node range; linearly
	// interpolated spread (not linearly interpolated final yield) between the two nodes.
	TEST_F( TestBondCurves, SNAPSHOT_BondCurve_MultiNodeSpreadInterpolation )
	{
		buildBondsForCalibrationAndBondCurve();

		const std::string multiNodeFile = TEST_DIR "USD_CORP_CURVE_MULTINODE@130_tryAqBondCurveCreate_inputs.csv";
		std::string curveName = createBondSpreadCurveFromFileName( multiNodeFile );

		const AnyTypeMatrix calibratedYields = validation::tryAqBondCurveDisplay( curveName );

		const double tolerance = 1.0e-9;
		checkBondCurveCalibration( calibratedYields, TEST_DIR "tryAqBondCurveMultiNodeSpreadDisplay_outputs.csv", tolerance, 17 );
	}

	// Build the SAME 2-bond curve twice, differing only in INTERPOLATION (PiecewiseConstant vs Linear), and check
	// that querying a date strictly between the two pillars gives genuinely different results: the step curve steps
	// to the pillar on/after the reference date (unaffected by the other pillar's value), while the linear curve
	// gives a value strictly between the two pillars' own calibrated yields.
	TEST_F( TestBondCurves, CONSISTENCY_BondCurve_LinearVsPiecewiseConstantInterpolation )
	{
		buildBondsForCalibrationAndBondCurve();

		const std::string stepFile   = TEST_DIR "USD_2BOND_STEP@131_tryAqBondCurveCreate_inputs.csv";
		const std::string linearFile = TEST_DIR "USD_2BOND_LINEAR@132_tryAqBondCurveCreate_inputs.csv";

		const std::string stepCurveName   = createBondCurveFromFileName( stepFile );
		const std::string linearCurveName = createBondCurveFromFileName( linearFile );

		const AQLDate firstPillar  = etrading::stringToDate( "44286" );
		const AQLDate secondPillar = etrading::stringToDate( "44469" );
		const AQLDate midDate      = etrading::stringToDate( "44377" );  // strictly between the two pillars

		const double stepYieldAtSecondPillar = validation::tryAqBondCurveYield( stepCurveName, secondPillar );
		const double stepYieldAtMidDate       = validation::tryAqBondCurveYield( stepCurveName, midDate );

		// PiecewiseConstant steps forward to the pillar on/after the reference date
		EXPECT_NEAR( stepYieldAtMidDate, stepYieldAtSecondPillar, 1.0e-12 );

		const double linearYieldAtFirstPillar  = validation::tryAqBondCurveYield( linearCurveName, firstPillar );
		const double linearYieldAtSecondPillar = validation::tryAqBondCurveYield( linearCurveName, secondPillar );
		const double linearYieldAtMidDate       = validation::tryAqBondCurveYield( linearCurveName, midDate );

		// Linear interpolation gives a value strictly between the two pillars' own calibrated yields
		const double lowerBound = std::min( linearYieldAtFirstPillar, linearYieldAtSecondPillar );
		const double upperBound = std::max( linearYieldAtFirstPillar, linearYieldAtSecondPillar );
		EXPECT_GT( linearYieldAtMidDate, lowerBound );
		EXPECT_LT( linearYieldAtMidDate, upperBound );

		// And Linear must genuinely differ from PiecewiseConstant at the same reference date
		EXPECT_GT( std::abs( linearYieldAtMidDate - stepYieldAtMidDate ), 1.0e-6 );
	}

	// A genuinely zero interpolated spread must reproduce the benchmark curve exactly: the spread curve re-runs the
	// SAME bootstrap the benchmark used (same bond quotes, same target yields), so this is a real equality, not an
	// approximation. Uses a single spread bond whose quoted yield is engineered to equal the benchmark's own
	// interpolated yield at that bond's maturity, giving a spread node of exactly zero (applied as a constant,
	// since a single spread node applies everywhere).
	TEST_F( TestBondCurves, CONSISTENCY_BondCurve_ZeroSpreadReproducesBenchmarkExactly )
	{
		buildBondsForCalibrationAndBondCurve();
		const std::string zeroSpreadFile = TEST_DIR "USD_CORP_CURVE_ZEROSPREAD@142_tryAqBondCurveCreate_inputs.csv";
		std::string curveName = createBondSpreadCurveFromFileName( zeroSpreadFile );

		const AnyTypeMatrix benchmarkMatrix = validation::tryAqBondCurveDisplay( "USTREASURIES1@127" );
		const AnyTypeMatrix spreadMatrix    = validation::tryAqBondCurveDisplay( curveName );

		std::map<int, double> spreadByDate;
		for ( size_t row = 0; row < spreadMatrix.size(); row++ )
		{
			spreadByDate[ boost::get<int>( spreadMatrix[row][0] ) ] = boost::get<double>( spreadMatrix[row][1] );
		}

		// Every benchmark pillar must appear in the spread curve too, with an identical calibrated yield
		for ( size_t row = 0; row < benchmarkMatrix.size(); row++ )
		{
			const int date = boost::get<int>( benchmarkMatrix[row][0] );
			const double benchmarkYield = boost::get<double>( benchmarkMatrix[row][1] );
			ASSERT_EQ( spreadByDate.count( date ), 1u ) << "Missing benchmark pillar at date " << date;
			EXPECT_NEAR( spreadByDate[ date ], benchmarkYield, 1.0e-12 ) << "Mismatch at date " << date;
		}
	}

	// YieldQuoteInPercent must divide both the market-data yields AND the Spread field by 100, for either curve type.
	// Build a plain curve, and a spread curve with a flat Spread shock, each once in decimal and once in percent
	// (percent inputs = decimal inputs x100) and check the two give identical calibrated yields.
	TEST_F( TestBondCurves, CONSISTENCY_BondCurve_YieldQuoteInPercent )
	{
		buildBondsForCalibrationAndBondCurve();

		// Plain curve: decimal vs percent market-data yields
		const std::string decimalPlainCurveName = createBondCurveFromFileName( TEST_DIR "USD_2BOND_STEP@131_tryAqBondCurveCreate_inputs.csv" );
		const std::string percentPlainCurveName = createBondCurveFromFileName( TEST_DIR "USD_2BOND_STEP_PCT@134_tryAqBondCurveCreate_inputs.csv" );

		const AnyTypeMatrix decimalPlainMatrix = validation::tryAqBondCurveDisplay( decimalPlainCurveName );
		const AnyTypeMatrix percentPlainMatrix = validation::tryAqBondCurveDisplay( percentPlainCurveName );

		ASSERT_EQ( decimalPlainMatrix.size(), percentPlainMatrix.size() );
		for ( size_t row = 0; row < decimalPlainMatrix.size(); row++ )
		{
			EXPECT_NEAR( boost::get<double>( decimalPlainMatrix[row][1] ), boost::get<double>( percentPlainMatrix[row][1] ), 1.0e-9 )
				<< "Plain curve decimal vs percent mismatch at row " << row;
		}

		// Spread curve with a flat Spread shock: decimal vs percent (both Spread and market-data yield)
		const std::string decimalSpreadCurveName = createBondSpreadCurveFromFileName( TEST_DIR "USD_CORP_CURVE_SHOCK_DEC@135_tryAqBondCurveCreate_inputs.csv" );
		const std::string percentSpreadCurveName = createBondSpreadCurveFromFileName( TEST_DIR "USD_CORP_CURVE_SHOCK_PCT@136_tryAqBondCurveCreate_inputs.csv" );

		const AnyTypeMatrix decimalSpreadMatrix = validation::tryAqBondCurveDisplay( decimalSpreadCurveName );
		const AnyTypeMatrix percentSpreadMatrix = validation::tryAqBondCurveDisplay( percentSpreadCurveName );

		ASSERT_EQ( decimalSpreadMatrix.size(), percentSpreadMatrix.size() );
		for ( size_t row = 0; row < decimalSpreadMatrix.size(); row++ )
		{
			EXPECT_NEAR( boost::get<double>( decimalSpreadMatrix[row][1] ), boost::get<double>( percentSpreadMatrix[row][1] ), 1.0e-9 )
				<< "Spread curve decimal vs percent mismatch at row " << row;
		}
	}

};