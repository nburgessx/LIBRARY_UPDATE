// Include: Google Test Library
#include <gTest/gTest.h>
#include "InitializeMLibGoogleTest.h"
#include "FolderConfig.h"

#include "LWOUtilities.h"
#include "FloatingBond.h"
#include "tryMeLWOBond.h"

#include "Dependency.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"
#include "ResultsProcessor.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

using etrading::ReadDataFile;
using etrading::CreateDataFile;

#define TEST_DIR "ETrading/Bonds/FloatingRateNotesType21/"


namespace
{
    /*
	 * This test builds 138 Floating Rate Notes from input file, and computes the price from discount margin for each bond.
	 * Rather than list out all bond filenames individually, we load then using the following regular expression pattern:
	 * The file pattern is:
	 * pathname / ( bondObjectName ) _tryMeLWOBondCreateFromGenerator_inputs.csv
	*/
    extern const std::string frnCreateInputFilePattern  = ".+_tryMeLWOBondCreateFromGenerator_inputs.csv";
	
	/*
	 * The bond price input and output filenames are contructed from  ( bondObjectName ) and the following suffixes:
	 */
    extern const std::string frnPriceFromDiscountMarginInputFileSuffix    = "_tryMeLWOBondPriceFromDiscountMargin_inputs.csv";
    extern const std::string frnPriceFromDiscountMarginOutputFileSuffix   = "_tryMeLWOBondPriceFromDiscountMargin_outputs.csv";
   
    extern const std::string frnPriceFromYieldInputFileSuffix             = "_tryMeLWOBondFRNPriceFromYield_inputs.csv";


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
            // Get the Google Test Unit Test Input Folder Path using the MLIBQ Environment Variable
            // Format = MLIBQ Goolge Test DataInstance Path + '/' + path to the 'p' Variable
            inputPath = etrading::FolderConfig::getMLIBQEnvironmentVariableGoogleTestPath() / inputPath;
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


	/*
	 * Given an inputFile, read the file and construct the bond from the specified BondGenerator
	 * Returns a pointer to the newly created bond
	 */
	std::string createBond( const std::string& inputFile )
    {
		// Build the Bond Object
		const ReadDataFile::Load createBondInputFile( inputFile.c_str() );
        std::string bondObjectName     = createBondInputFile["bondObjectName"];
        std::string bondGeneratorName  = createBondInputFile["bondGeneratorName"];
        LAStringMatrix bondExpressionLVB = createBondInputFile["expressionLVB"];
        bool validateKeys              = createBondInputFile["validateKeys"];
        
		std::string objectName = validation_api::tryMeLWOBondCreateFromGenerator( bondObjectName, bondGeneratorName, bondExpressionLVB, validateKeys );
		return objectName;
    };

}

namespace google_test
{

	// --------------------------------------------------
    
	DECLARE_TEST_FIXTURE(TestBondFRN_Type21);

    TEST_F( TestBondFRN_Type21, SNAPSHOT_tryMeLWOBondFRNPriceFromDiscountMargin )
    {
		const double tolerance = 1.0e-8;

		// Get all bond CreateFromGenerator input files in TEST_DIR folder
		std::vector<std::string> allMatchingFiles = getFilesMatchingPattern( TEST_DIR, frnCreateInputFilePattern );

		// For each matching file in TEST_DIR...
		for (size_t i=0; i< allMatchingFiles.size(); i++ )
		{
			// Create the bond
			std::string inputFile = allMatchingFiles[ i ];
			std::string bondObjectName = createBond( inputFile );

			// Read bond price inputs
			std::string priceFromDiscountMarginInputFileName = TEST_DIR + bondObjectName + frnPriceFromDiscountMarginInputFileSuffix;
	        const ReadDataFile::Load priceInputFile( priceFromDiscountMarginInputFileName.c_str() );

			const LADate settlementDate = priceInputFile[ "settlementDate" ];
			const double discountMargin = priceInputFile[ "discountMargin" ];
			const double assumedRate    = priceInputFile[ "assumedRate" ];
			const double indexToNextCoupon = priceInputFile[ "indexToNextCoupon" ];
			const double annualizedNextCouponRate = priceInputFile[ "annualizedNextCouponRate" ];

			// Calculate the bond price
			const double calculatedPrice = validation_api::tryMeLWOBondFRNPriceFromDiscountMargin( bondObjectName, settlementDate, discountMargin, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
			
			// Compare the calculatedPrice against snaptshot results
			std::string priceOutputFilename = TEST_DIR + bondObjectName + frnPriceFromDiscountMarginOutputFileSuffix;
			CheckTestResultsAndRebaseOnRequest( calculatedPrice, TEST_DIR, LAString( priceOutputFilename.c_str() ), tolerance );
		}
	}


	TEST_F( TestBondFRN_Type21, SNAPSHOT_tryMeLWOBondFRNPriceFromYield )
	{
		const double tolerance = 1.0e-8;

		// Get all bond CreateFromGenerator input files in TEST_DIR folder
		std::vector<std::string> allMatchingFiles = getFilesMatchingPattern( TEST_DIR, frnCreateInputFilePattern );

		// For each matching file in TEST_DIR...
		for (size_t i=0; i< allMatchingFiles.size(); i++ )
		{
			// Create the bond
			std::string inputFile = allMatchingFiles[ i ];
			std::string bondObjectName = createBond( inputFile );

			// Read bond price inputs
			std::string priceFromYieldInputFileName = TEST_DIR + bondObjectName + frnPriceFromYieldInputFileSuffix;
	        const ReadDataFile::Load priceInputFile( priceFromYieldInputFileName.c_str() );

			const LADate settlementDate = priceInputFile[ "settlementDate" ];
			const double yield          = priceInputFile[ "yield" ];
			const double assumedRate    = priceInputFile[ "assumedRate" ];
			const double indexToNextCoupon = priceInputFile[ "indexToNextCoupon" ];
			const double annualizedNextCouponRate = priceInputFile[ "annualizedNextCouponRate" ];

			// Calculate the bond price
			const double calculatedPrice = validation_api::tryMeLWOBondFRNPriceFromYield( bondObjectName, settlementDate, yield, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
			
			// Compare the calculatedPrice against snaptshot results
			// Re-use the PriceFromDiscountMargin output file, since the priceFromYield should match exactly.
			std::string priceOutputFilename = TEST_DIR + bondObjectName + frnPriceFromDiscountMarginOutputFileSuffix;
			CheckTestResultsAndRebaseOnRequest( calculatedPrice, TEST_DIR, LAString( priceOutputFilename.c_str() ), tolerance );
		}

	}

	TEST_F( TestBondFRN_Type21, CONSISTENCY_tryMeLWOBondFRNYieldRoundTrip )
	{
		const double tolerance = 1.0e-8;
				
		// Get all bond CreateFromGenerator input files in TEST_DIR folder
		std::vector<std::string> allMatchingFiles = getFilesMatchingPattern( TEST_DIR, frnCreateInputFilePattern );

		// For each matching file in TEST_DIR...
		for (size_t i=0; i< allMatchingFiles.size(); i++ )
		{
			// Create the bond
			std::string inputFile = allMatchingFiles[ i ];
			std::string bondObjectName = createBond( inputFile );

			// Read bond price inputs
			std::string priceFromYieldInputFileName = TEST_DIR + bondObjectName + frnPriceFromYieldInputFileSuffix;
	        const ReadDataFile::Load priceInputFile( priceFromYieldInputFileName.c_str() );

			const LADate settlementDate = priceInputFile[ "settlementDate" ];
			const double yield          = priceInputFile[ "yield" ];
			const double assumedRate    = priceInputFile[ "assumedRate" ];
			const double indexToNextCoupon = priceInputFile[ "indexToNextCoupon" ];
			const double annualizedNextCouponRate = priceInputFile[ "annualizedNextCouponRate" ];

			// 1. Calculate Price from yield
			const double calculatedPrice = validation_api::tryMeLWOBondFRNPriceFromYield( bondObjectName, settlementDate, yield, assumedRate, indexToNextCoupon, annualizedNextCouponRate );

			// 2. Calculate Yield from the Price in step 1
			const double calculatedYield = validation_api::tryMeLWOBondFRNYieldFromPrice( bondObjectName, settlementDate, calculatedPrice, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
			
			// 3. Verify that the calculated Yield matches the input yield.
			//    This must always match, and we do not allow rebase.
			ASSERT_NEAR( calculatedYield, yield, tolerance );
		}
	}

	TEST_F( TestBondFRN_Type21, CONSISTENCY_tryMeLWOBondFRNDiscountMarginRoundTrip )
	{
		const double tolerance = 1.0e-8;
				
		// Get all bond CreateFromGenerator input files in TEST_DIR folder
		std::vector<std::string> allMatchingFiles = getFilesMatchingPattern( TEST_DIR, frnCreateInputFilePattern );

		// For each matching file in TEST_DIR...
		for (size_t i=0; i< allMatchingFiles.size(); i++ )
		{
			// Create the bond
			std::string inputFile = allMatchingFiles[ i ];
			std::string bondObjectName = createBond( inputFile );

			// Read bond price inputs
			std::string priceFromDiscountMarginInputFileName = TEST_DIR + bondObjectName + frnPriceFromDiscountMarginInputFileSuffix;
	        const ReadDataFile::Load priceInputFile( priceFromDiscountMarginInputFileName.c_str() );

			const LADate settlementDate = priceInputFile[ "settlementDate" ];
			const double discountMargin = priceInputFile[ "discountMargin" ];
			const double assumedRate    = priceInputFile[ "assumedRate" ];
			const double indexToNextCoupon = priceInputFile[ "indexToNextCoupon" ];
			const double annualizedNextCouponRate = priceInputFile[ "annualizedNextCouponRate" ];

			// Calculate the bond price
			const double calculatedPrice = validation_api::tryMeLWOBondFRNPriceFromDiscountMargin( bondObjectName, settlementDate, discountMargin, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
			
			// 2. Calculate Discount Margin from the Price in step 1
			const double calculatedDiscountMargin = validation_api::tryMeLWOBondFRNDiscountMarginFromPrice( bondObjectName, settlementDate, calculatedPrice, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
			
			// 3. Verify that the calculated Yield matches the input yield.
			//    This must always match, and we do not allow rebase.
			ASSERT_NEAR( calculatedDiscountMargin, discountMargin, tolerance );
		}
	}

};

