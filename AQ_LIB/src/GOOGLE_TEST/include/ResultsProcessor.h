#pragma once

#include "CreateDataFile.h"

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
                                             int index = -1 );

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
                                             int index = -1 );

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
                                             int index = -1 );

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
                                             int index = -1 );

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
                                             int index = -1 );

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
                                             int index = -1 );

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
                                             int index = -1 );

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
                                             int index = -1 );

	/* @brief			Check or rebase delta risk bucket results
	*  @param [in]		pillarNames			The final list of pillar names
	*  @param [in]		headers				Headers of the delta report
	*  @param [in]		deltas				All the bucket deltas
	*  @param [in]		tolerance			Result discrepancy tolerance
	*  @param [in]		baseDir				The base input directory
	*  @param [in]		output_32	        Full reesult file name for 32bit result
	*  @param [in]		output_64	        Full reesult file name for 32bit result
	*/
	void verifyDeltaBucketAmounts(const AQLStringVector& pillarNames, const AQLStringVector& headers, const DoubleMatrix& deltas, double tolerance, const AQLString& baseDir, const AQLString& output_32, const AQLString& output_64);
}