#include "ResultsProcessor.h"
#include "ReadDataFile.h"
#include <gTest/gTest.h>

namespace google_test
{
	/* @brief			Either rebase test files or compare new test results against old ones
    *  @param [in]		results				Results collection
    *  @param [in]		testPath			Path of the result file
    *  @param [in]		resultFileWithPath	Result file name with path
    *  @param [in]		tolerance			Comparison tolerance
    *  @param [in]		index				Index number used to form part of the result file name
    */
    void CheckTestResultsAndRebaseOnRequest( const etrading::VariantMatrix& results,
                                             const AQLString& testPath,
                                             const AQLString& resultFileWithPath,
                                             double tolerance,
                                             int index )
    {
        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( testPath, false );
            AQLStringVector v = AQLString( resultFileWithPath ).toToken( '/' );
            AQLString resultFileName = v.back();

            etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName, index ) );
            file.write( "output", results );
        }
        else
        {
            const etrading::ReadDataFile::Load resultFile( etrading::CreateDataFile::makeFilename( resultFileWithPath, index ) );
            const DoubleMatrix expectedResult = resultFile["output"];

			AQ_REQUIRE( results.size() == expectedResult.size(), "Expected result and Actual result row sizes must be identical" )
            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
				for ( size_t j = 0; j != expectedResult[i].size(); ++j )
				{
					AQ_REQUIRE( results[i].size() == expectedResult[i].size(), "Expected result and Actual result column sizes must be identical" )
					EXPECT_NEAR( expectedResult[i][j], results[i][j], tolerance ) << "Test number: " << index << ", Matrix result: row " << i << ", column " << j << std::endl;
				}
            }
        }
    }

	/* @brief			Either rebase test files or compare new test results against old ones
    *  @param [in]		results				Results collection
    *  @param [in]		testPath			Path of the result file
    *  @param [in]		resultFileWithPath	Result file name with path
    *  @param [in]		tolerance			Comparison tolerance
    *  @param [in]		index				Index number used to form part of the result file name
    */
    void CheckTestResultsAndRebaseOnRequest( const DoubleMatrix& results,
                                             const AQLString& testPath,
                                             const AQLString& resultFileWithPath,
                                             double tolerance,
                                             int index )
    {
        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( testPath, false );
            AQLStringVector v = AQLString( resultFileWithPath ).toToken( '/' );
            AQLString resultFileName = v.back();

            etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName, index ) );
            file.write( "output", results );
        }
        else
        {
            const etrading::ReadDataFile::Load resultFile( etrading::CreateDataFile::makeFilename( resultFileWithPath, index ) );
            const DoubleMatrix expectedResult = resultFile["output"];

			AQ_REQUIRE( results.size() == expectedResult.size(), "Expected result and Actual result row sizes must be identical" )
            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
				for ( size_t j = 0; j != expectedResult[i].size(); ++j )
				{
					AQ_REQUIRE( results[i].size() == expectedResult[i].size(), "Expected result and Actual result column sizes must be identical" )
					EXPECT_NEAR( expectedResult[i][j], results[i][j], tolerance ) << "Test number: " << index << ", Matrix result: row " << i << ", column " << j << std::endl;
				}
            }
        }
    }

    /* @brief			Either rebase test files or compare new test results against old ones
    *  @param [in]		results				Results collection
    *  @param [in]		testPath			Path of the result file
    *  @param [in]		resultFileWithPath	Result file name with path
    *  @param [in]		tolerance			Comparison tolerance
    *  @param [in]		index				Index number used to form part of the result file name
    */
    void CheckTestResultsAndRebaseOnRequest( const DoubleArray& results,
                                             const AQLString& testPath,
                                             const AQLString& resultFileWithPath,
                                             double tolerance,
                                             int index )
    {
        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( testPath, false );
            AQLStringVector v = AQLString( resultFileWithPath ).toToken( '/' );
            AQLString resultFileName = v.back();

            etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName, index ) );
            file.write( "output", results );
        }
        else
        {
            const etrading::ReadDataFile::Load resultFile( etrading::CreateDataFile::makeFilename( resultFileWithPath, index ) );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance ) << "Test number: " << index << ", result number: " << i << std::endl;
            }
        }
    }


    /* @brief			Either rebase test files or compare new test results against old ones
    *  @param [in]		results				Results collection
    *  @param [in]		testPath			Path of the result file
    *  @param [in]		resultFileWithPath	Result file name with path
    *  @param [in]		fileSuffix	        Result file suffix
    *  @param [in]		tolerance			Comparison tolerance
    *  @param [in]		index				Index number used to form part of the result file name
    */
    void CheckTestResultsAndRebaseOnRequest( const DoubleArray& results,
                                             const AQLString& testPath,
                                             const AQLString& resultFileWithPath,
                                             const AQLString& fileSuffix,
                                             double tolerance,
                                             int index )
    {
        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( testPath, false );
            AQLStringVector v = AQLString( resultFileWithPath ).toToken( '/' );
            AQLString resultFileName = v.back();

            etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName, fileSuffix, index ) );
            file.write( "output", results );
        }
        else
        {
            const etrading::ReadDataFile::Load resultFile( etrading::CreateDataFile::makeFilename( resultFileWithPath, fileSuffix, index ) );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance ) << "Test number: " << index << ", result number: " << i << std::endl;
            }
        }
    }


    /* @brief			Either rebase test files or compare new test results against old ones
    *  @param [in]		results				Results collection
    *  @param [in]		testPath			Path of the result file
    *  @param [in]		resultFileWithPath	Result file name with path
    *  @param [in]		filePrefix	        Result file prefix
    *  @param [in]		fileSuffix	        Result file suffix
    *  @param [in]		tolerance			Comparison tolerance
    *  @param [in]		index				Index number used to form part of the result file name
    */
    void CheckTestResultsAndRebaseOnRequest( const DoubleArray& results,
                                             const AQLString& testPath,
                                             const AQLString& resultFileWithPath,
                                             const AQLString& filePrefix,
                                             const AQLString& fileSuffix,
                                             double tolerance,
                                             int index )
    {
        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( testPath, false );
            AQLStringVector v = AQLString( resultFileWithPath ).toToken( '/' );
            AQLString resultFileName = v.back();

            etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName, filePrefix, fileSuffix, index ) );
            file.write( "output", results );
        }
        else
        {
            const etrading::ReadDataFile::Load resultFile( etrading::CreateDataFile::makeFilename( resultFileWithPath, filePrefix, fileSuffix, index ) );
            const DoubleArray expectedResult = resultFile["output"];

            for ( size_t i = 0; i != expectedResult.size(); ++i )
            {
                EXPECT_NEAR( expectedResult.at( i ), results.at( i ), tolerance ) << "Test number: " << index << ", result number: " << i << std::endl;
            }
        }
    }


    /* @brief			Either rebase test files or compare new test results against old ones
    *  @param [in]		result				A single value for comparision
    *  @param [in]		testPath			Path of the result file
    *  @param [in]		resultFileWithPath	Result file name with path
    *  @param [in]		tolerance			Comparison tolerance
    *  @param [in]		index				Index number used to form part of the result file name
    */
    void CheckTestResultsAndRebaseOnRequest( double result,
                                             const AQLString& testPath,
                                             const AQLString& resultFileWithPath,
                                             double tolerance,
                                             int index )
    {
        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( testPath, false );
            AQLStringVector v = AQLString( resultFileWithPath ).toToken( '/' );
            AQLString resultFileName = v.back();

            etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName, index ) );
            file.write( "output", result );
        }
        else
        {
            const etrading::ReadDataFile::Load resultFile( etrading::CreateDataFile::makeFilename( resultFileWithPath, index ) );
            const double expectedResult = resultFile["output"];

            EXPECT_NEAR( expectedResult, result, tolerance ) << "Test number: " << index << std::endl;
        }
    }

     /* @brief			Either rebase test files or compare new test results against old ones
    *  @param [in]		result				A single value for comparision
    *  @param [in]		testPath			Path of the result file
    *  @param [in]		resultFileWithPath	Result file name with path
    *  @param [in]		fileSuffix	        Result file suffix
    *  @param [in]		tolerance			Comparison tolerance
    *  @param [in]		index				Index number used to form part of the result file name
    */
    void CheckTestResultsAndRebaseOnRequest( double result,
                                             const AQLString& testPath,
                                             const AQLString& resultFileWithPath,
                                             const AQLString& fileSuffix,
                                             double tolerance,
                                             int index )
    {
        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( testPath, false );
            AQLStringVector v = AQLString( resultFileWithPath ).toToken( '/' );
            AQLString resultFileName = v.back();

            etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName, fileSuffix, index ) );
            file.write( "output", result );
        }
        else
        {
            const etrading::ReadDataFile::Load resultFile( etrading::CreateDataFile::makeFilename( resultFileWithPath, fileSuffix, index ) );
            const double expectedResult = resultFile["output"];

            EXPECT_NEAR( expectedResult, result, tolerance ) << "Test number: " << index << std::endl;
        }
    }

     /* @brief			Either rebase test files or compare new test results against old ones
    *  @param [in]		result				A single value for comparision
    *  @param [in]		testPath			Path of the result file
    *  @param [in]		resultFileWithPath	Result file name with path
    *  @param [in]		filePrefix	        Result file prefix
    *  @param [in]		fileSuffix	        Result file suffix
    *  @param [in]		tolerance			Comparison tolerance
    *  @param [in]		index				Index number used to form part of the result file name
    */
    void CheckTestResultsAndRebaseOnRequest( double result,
                                             const AQLString& testPath,
                                             const AQLString& resultFileWithPath,
                                             const AQLString& filePrefix,
                                             const AQLString& fileSuffix,
                                             double tolerance,
                                             int index )
    {
        if ( etrading::CreateDataFile::rebaseResultsEnabled() )
        {
            // Record outputs and rebase test outputs
            etrading::CreateDataFile::setOutputFolder( testPath, false );
            AQLStringVector v = AQLString( resultFileWithPath ).toToken( '/' );
            AQLString resultFileName = v.back();

            etrading::CreateDataFile file( etrading::CreateDataFile::makeFilename( resultFileName, filePrefix, fileSuffix, index ) );
            file.write( "output", result );
        }
        else
        {
            const etrading::ReadDataFile::Load resultFile( etrading::CreateDataFile::makeFilename( resultFileWithPath, filePrefix, fileSuffix, index ) );
            const double expectedResult = resultFile["output"];

            EXPECT_NEAR( expectedResult, result, tolerance ) << "Test number: " << index << std::endl;
        }
    }

	/* @brief			Check or rebase delta risk bucket results
	*  @param [in]		pillarNames			The final list of pillar names
	*  @param [in]		headers				Headers of the delta report
	*  @param [in]		deltas				All the bucket deltas
	*  @param [in]		tolerance			Result discrepancy tolerance
	*  @param [in]		baseDir				The base input directory
	*  @param [in]		output_32	        Full reesult file name for 32bit result
	*  @param [in]		output_64	        Full reesult file name for 32bit result
	*/
	void verifyDeltaBucketAmounts(const AQLStringVector& pillarNames, const AQLStringVector& headers, const DoubleMatrix& deltas, double tolerance, const AQLString& baseDir, const AQLString& output_32, const AQLString& output_64)
	{
		// Verify delta bucket amounts
		if (etrading::CreateDataFile::rebaseResultsEnabled())
		{
#ifdef GTEST32
			AQLString outputFileName = output_32;
#else
			AQLString outputFileName = output_64;
#endif

			// Record outputs and rebase test outputs
			etrading::CreateDataFile::setOutputFolder(baseDir, false);
			etrading::CreateDataFile file(etrading::decorateFilename(outputFileName));

			file.write("headers", headers);

			for (size_t i = 0; i < pillarNames.size(); ++i)
			{
				// 12 is the number of decimal points required
				file.write(pillarNames[i], deltas[i], 12);
			}
		}
		else
		{
			// Carry out actual test and peform result comparison
#ifdef GTEST32
			AQLString resultDir = baseDir;
			resultDir += output_32;
			const etrading::ReadDataFile::Load resultFile(resultDir);
#else
			AQLString resultDir = baseDir;
			resultDir += output_64;
			const etrading::ReadDataFile::Load resultFile(resultDir);
#endif

			if (!resultFile.hasItem("headers"))
			{
				FAIL() << "Reference baseline does not contain delta column headers" << std::endl;
			}

			AQLStringVector refHeaders = resultFile["headers"];
			if (headers.size() != refHeaders.size())
			{
				FAIL() << "Calculated delta has different number of column headers compared to reference baseline : " << headers.size() << " vs " << refHeaders.size() << std::endl;
			}
			for (size_t i = 0; i < headers.size(); ++i)
			{
				ASSERT_EQ(headers[i], refHeaders[i]);
			}

			for (size_t i = 0; i < pillarNames.size(); ++i)
			{
				AQLString key = pillarNames[i];

				if (!resultFile.hasItem(key))
				{
					FAIL() << "Calculated results contains a pillarName: " << key << " which is missing in reference baseline" << std::endl;
				}

				DoubleVector delta = deltas[i];
				DoubleVector ref = resultFile[key];

				if (delta.size() != ref.size())
				{
					FAIL() << "Calculated delta has different number of columns compared to reference baseline: " << delta.size() << " vs " << ref.size() << std::endl;
				}

				for (size_t j = 0; j < delta.size(); ++j)
				{
					EXPECT_NEAR(delta[j], ref[j], tolerance)
						<< " Delta for pillar point " << key.getCString() << " is incorrect ";
				}
			}
		}
	}
}