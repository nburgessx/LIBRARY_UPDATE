// DataUtilities.h


/*
 * @brief			Helper methods used within validation_api
 * @Created:		22 September 2017
 * @Author:			Nicholas Burgess
 * @Department:		ISD Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "Variant.h"
#include "LWOUtilities.h"
#include <sstream>

namespace etrading
{
	/* @brief			Function to test if a Variant is a potential Error starting with the '[' or '#' error prefix symbols
    *  @param [in]		inputVariant                The input variant to test
    *  @param [in]      checkForError               Flag to check for errors, when TRUE will check for strings starting with error prefixes '[' and / or '#' symbols
    *  @return			Returns a boolean to indicate if the input variant is an error string
    */
    bool isError( const Variant & inputVariant, const bool & checkForError = true );

    /* @brief			Function to remove a row from a matrix if it contains blanks, if a column number is specified then only that column is checked for blanks
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		onlyCheckColumnNumber		The column number to check for blanks, if not specified then all matrix columns will be checked
    *  @param [in]      removeErrors                Flag to remove errors, when TRUE will remove strings starting with error prefixes '[' and / or '#' symbols
    *  @return			Returns a Matrix with blank rows removed
    */
    VariantMatrix removeBlankRows( const VariantMatrix & inputMatrix, const int & onlyCheckColumnNumber = 0, const bool & removeErrors = true );
    
    /* @brief			Function to remove a row from a matrix if the entire row is empty
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @return			Returns a Matrix with empty rows removed
    */
    VariantMatrix removeEmptyRows( const VariantMatrix & inputMatrix );

    /* @brief			Function to remove a column from a matrix if it contains blanks, if a row number is specified then only that row is checked for blanks
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		onlyCheckRowNumber		    The row number to check for blanks, if not specified then all matrix rows will be checked
    *  @param [in]      removeErrors                Flag to remove AlgoQuantLib errors, when TRUE will remove strings starting with AlgoQuantLib error prefixes '[' and / or '#' symbols
    *  @return			Returns a Matrix with blank columns removed
    */
    VariantMatrix removeBlankColumns( const VariantMatrix & inputMatrix, const int & onlyCheckRowNumber = 0, const bool & removeErrors = true );

    /* @brief			Function to remove a column from a matrix if the entire column is empty
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @return			Returns a Matrix with empty columns removed
    */
    VariantMatrix removeEmptyColumns( const VariantMatrix & inputMatrix );

    /* @brief			Function to remove a column from a matrix if the entire column is empty
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		blockName		            The name of the inputMatrix or dataBlock
    *  @return			Returns nothing, but will throw if an error input is encountered
    */
    void checkDataMatrixForErrors( const VariantMatrix & inputMatrix, const std::string blockName );

    /* @brief			Function to append matrices below each other by row, matrix 2 will be placed below matrix 1.
    *  @param [inout]	matrix1		                Input matrix 1. On output, contains the augmented matrix
    *  @param [in]		matrix2		                Input matrix 2
    *  @return			Returns an augmented matrix with all matrices appended below each other by row, matrix 2 will be placed below matrix 1.
    */
    VariantMatrix appendRows( VariantMatrix & matrix1,
                              const VariantMatrix & matrix2 );

    /* @brief			Function to append matrices below each other by row
    *  @param [in]		matrix1		                Input matrix 1
    *  @param [in]		matrix2		                Input matrix 2
    *  @param [in]		matrix3		                Input matrix 3
    *  @param [in]		matrix4		                Input matrix 4
    *  @param [in]		matrix5		                Input matrix 5
    *  @param [in]		matrix6		                Input matrix 6
    *  @param [in]		matrix7		                Input matrix 7
    *  @param [in]		matrix8		                Input matrix 8
    *  @param [in]		matrix9		                Input matrix 9
    *  @param [in]		matrix10	                Input matrix 10
    *  @return			Returns an augmented matrix with all matrices appended below each other by row
    */
    VariantMatrix appendRows( const VariantMatrix & matrix1,
                              const VariantMatrix & matrix2,
                              const VariantMatrix & matrix3,
                              const VariantMatrix & matrix4,
                              const VariantMatrix & matrix5,
                              const VariantMatrix & matrix6,
                              const VariantMatrix & matrix7,
                              const VariantMatrix & matrix8,
                              const VariantMatrix & matrix9,
                              const VariantMatrix & matrix10 );


    /* @brief			Function to append matrices to the right of each other by column, matrix 2 will be placed to the right of matrix 1.
    *  @param [inout]	matrix1		                Input matrix 1. On output, contains the augmented matrix
    *  @param [in]		matrix2		                Input matrix 2
    *  @return			Returns an augmented matrix with all matrices to the right of each other by column, matrix 2 will be placed to the right of matrix 1.
    */
    VariantMatrix appendColumns( VariantMatrix & matrix1,
                                 const VariantMatrix & matrix2 );


    /* @brief			Function to append matrices to the right of each other by column 
    *  @param [in]		matrix1		                Input matrix 1
    *  @param [in]		matrix2		                Input matrix 2
    *  @param [in]		matrix3		                Input matrix 3
    *  @param [in]		matrix4		                Input matrix 4
    *  @param [in]		matrix5		                Input matrix 5
    *  @param [in]		matrix6		                Input matrix 6
    *  @param [in]		matrix7		                Input matrix 7
    *  @param [in]		matrix8		                Input matrix 8
    *  @param [in]		matrix9		                Input matrix 9
    *  @param [in]		matrix10	                Input matrix 10
    *  @return			Returns an augmented matrix with all matrices appended to the right of each other by column
    */
    VariantMatrix appendColumns( const VariantMatrix & matrix1,
                                 const VariantMatrix & matrix2,
                                 const VariantMatrix & matrix3,
                                 const VariantMatrix & matrix4,
                                 const VariantMatrix & matrix5,
                                 const VariantMatrix & matrix6,
                                 const VariantMatrix & matrix7,
                                 const VariantMatrix & matrix8,
                                 const VariantMatrix & matrix9,
                                 const VariantMatrix & matrix10 );


    /* @brief			Function to filter data for unique input 
    *  @param [in]		dataVector		            A vector of data
    *  @param [in]		displayByRow		        True = Display by Row, False = Display by Column, Default is False
    *  @return			Returns a vector of filtered data removing all duplicates
    */
    VariantMatrix dataFilter( const VariantVector & dataVector, const bool displayByRow = false );

	/* @brief Returns a VariantMatrix containing the configuration information for the specified propertyKey
	*  @param [in]	freeObject		The object containing blocks of data indexed by propertyKey
	*  @param [in]  propertyKey		The property data block to be retrieved from the freeObject
	*  @param [in]  throwIfMissing	Whether to throw an exception if the specified propertyKey is missing. Default behaviour is to throw.
	*  @returns		A VariantMatrix containing the data specified by propertyKey
	*/
	VariantMatrix getVariantMatrixFromFreeObject( const FreeObject& freeObject, const std::string& propertyKey, const bool throwIfMissing = true );

	/* @brief Returns a LAStringMatrix containing the configuration information for the specified propertyKey
	*  @param [in]	freeObject		The object containing blocks of data indexed by propertyKey
	*  @param [in]  propertyKey		The property data block to be retrieved from the freeObject
	*  @param [in]  trimBlankRows	Whether to trim blank rows at the end of the block. Default behaviour is to trim
	*  @param [in]  throwIfMissing	Whether to throw an exception if the specified propertyKey is missing. Default behaviour is to throw.
	*  @returns		A VariantMatrix containing the data specified by propertyKey
	*/
	LAStringMatrix getLAStringMatrixFromFreeObject( const FreeObject& freeObject, const std::string& propertyKey, const bool trimBlankRows = true, const bool throwIfMissing = true);
    
    /* @brief Returns a StandardStringMatrix containing the configuration information for the specified propertyKey
	*  @param [in]	freeObject		The object containing blocks of data indexed by propertyKey
	*  @param [in]  propertyKey		The property data block to be retrieved from the freeObject
	*  @param [in]  trimBlankRows	Whether to trim blank rows at the end of the block. Default behaviour is to trim
	*  @returns		A VariantMatrix containing the data specified by propertyKey
	*/
    StandardStringMatrix getStandardStringMatrixFromFreeObject( const FreeObject& freeObject, const std::string& propertyKey, const bool trimBlankRows = true );

	// Function to trim a LA String Vector and remove blanks
	LAStringVector trimLAStringVector(const LAStringVector & inputVector);
	
	// Function to trim a Standard String Vector and remove blanks
	StandardStringVector trimStandardStringVector(const StandardStringVector & inputVector);

    // Function to trim a LA String Matrix and remove blanks
	LAStringMatrix trimLAStringMatrix(const LAStringMatrix & inputMatrix);
	
	// Function to trim a Standard String Matrix and remove blanks
	StandardStringMatrix trimStandardStringMatrix(const StandardStringMatrix & inputMatrix);

	// Function to convert a LAStringMatrix to a VariantMatrix
	etrading::VariantMatrix convertLAStringMatrixToVariantMatrix( LAStringMatrix stringMatrix );

	// Function to convert a Standard StringMatrix to a VariantMatrix
	etrading::VariantMatrix convertStandardStringMatrixToVariantMatrix( StandardStringMatrix stringMatrix );

	/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
	*  @param [in]	freeObject		The object containing blocks of data indexed by propertyKey
	*  @param [in]  propertyKey		The property data block to be retrieved from the freeObject
	*  @param [out]  A VariantMatrix containing a LabelValue block of properties
	*/
	VariantMatrix viewInputParametersFromFreeObject(const FreeObject& freeObject, const std::string& propertyKey);

}