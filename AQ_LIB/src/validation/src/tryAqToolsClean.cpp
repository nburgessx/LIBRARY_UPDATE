// tryAqToolsClean.cpp

/*
 * @brief			Functions to clean data to remove rows or columns with blank or error values
 */

#include "tryAqToolsClean.h"
#include "ExceptionMacros.h"
#include "DataUtilities.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "RecordMacros.h"


namespace validation
{
    using etrading::CreateDataFile;
    using etrading::decorateFilename;

    /* @brief			Function to clean a matrix of blank or error values
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		cleanByRow                  Boolean: TRUE = Clean by Row (Default), FALSE = Clean by Column
    *  @param [in]		checkRowColNumber           Int: Row / Column number to check, leave blank to check entire inputMatrix
    *  @param [in]		removeBlanks                Boolean: TRUE = Remove Blank data (Default), FALSE = Do not remove blank data
    *  @param [in]      removeErrors                Flag to remove AlgoQuantLib errors, when TRUE will remove strings starting with AlgoQuantLib error prefixes '[' and / or '#' symbols
    *  @return			Returns a Matrix with errors and blank rows removed
    */
    VariantMatrix tryAqToolsClean( const VariantMatrix &      inputMatrix,
                                     const bool &               cleanByRow,
                                     const int &                checkRowColNumber,
                                     const bool &               removeBlanks,
                                     const bool &               removeErrors )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( inputMatrix, cleanByRow, checkRowColNumber, removeBlanks, removeErrors );

        VariantMatrix result = cleanVariantMatrix( inputMatrix, cleanByRow, checkRowColNumber, removeBlanks, removeErrors );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Function to clean a variant Matrix of blank or error values
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		cleanByRow                  Boolean: TRUE = Clean by Row (Default), FALSE = Clean by Column
    *  @param [in]		checkRowColNumber           Int: Row / Column number to check, leave blank or set to zero to check entire inputMatrix
    *  @param [in]		removeBlanks                Boolean: TRUE = Remove blank data (Default), FALSE = Do not remove blank data
    *  @param [in]      removeErrors                Flag to remove AlgoQuantLib errors, when TRUE will remove strings starting with AlgoQuantLib error prefixes '[' and / or '#' symbols
    *  @return			Returns a Matrix with errors and blank rows removed
    */
    VariantMatrix cleanVariantMatrix( const VariantMatrix &      inputMatrix,
                                      const bool &               cleanByRow,
                                      const int &                checkRowColNumber,
                                      const bool &               removeBlanks,
                                      const bool &               removeErrors )
    {
        VariantMatrix result = inputMatrix;

        if ( removeBlanks )
        {
            // Remove Blank Data
            if ( cleanByRow )
            {
                // Clean by row
                result = etrading::removeBlankRows( result, checkRowColNumber, removeErrors );
            }
            else
            {
                // Clean by column
                result = etrading::removeBlankColumns( result, checkRowColNumber, removeErrors );
            }
        }

        return result;
    }

    /* @brief			Function to trime a variant Matrix and remove a row or column if empty
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		trimByRow                  Boolean: TRUE = trim by Row (Default), FALSE = trim by Column
    *  @return			Returns a Matrix with errors and blank rows removed
    */
    VariantMatrix trimVariantMatrix( const VariantMatrix &      inputMatrix,
                                     const bool &               trimByRow )
    {
        VariantMatrix result = inputMatrix;

        // Remove Blank Data
        if ( trimByRow )
        {
            // Trim by row
            result = etrading::removeEmptyRows( result );
        }
        else
        {
            // Trim by column
            result = etrading::removeEmptyColumns( result );
        }

        return result;
    }

	// Function to trim a LA String Vector and remove blanks
	AQLStringVector trimAQLStringVector( const AQLStringVector & inputVector )
	{
		AQLStringVector results = etrading::trimAQLStringVector( inputVector );
		return results;
	}

	// Function to trim a Standard String Vector and remove blanks
	StandardStringVector trimStandardStringVector( const StandardStringVector & inputVector )
	{
		StandardStringVector results = etrading::trimStandardStringVector( inputVector );
		return results;
	}

    // Function to trim a LA String Matrix and remove blanks
	AQLStringMatrix trimAQLStringMatrix( const AQLStringMatrix & inputMatrix )
	{
		AQLStringMatrix results = etrading::trimAQLStringMatrix( inputMatrix );
		return results;
	}

	// Function to trim a Standard String Matrix and remove blanks
	StandardStringMatrix trimStandardStringMatrix( const StandardStringMatrix & inputMatrix )
	{
		StandardStringMatrix results = etrading::trimStandardStringMatrix( inputMatrix );
		return results;
	}
}