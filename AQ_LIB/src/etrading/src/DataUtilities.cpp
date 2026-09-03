// DataUtilities.cpp


/*
 * @brief			Helper methods used within validation_api
 * @Created:		22 September 2017
 * @Author:			Nicholas Burgess
 * @Department:		ISD Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "DataUtilities.h"
#include "ExceptionMacros.h"
#include "DebugMacros.h"

#include <boost/algorithm/string/predicate.hpp>
#include <set>

namespace etrading
{
    /* @brief			Function to test if a Variant is a potential Error starting with the '[' or '#' error prefix symbols
    *  @param [in]		inputVariant                The input variant to test
    *  @param [in]      checkForError               Flag to check for errors, when TRUE will check for strings starting with error prefixes '[' and / or '#' symbols
    *  @return			Returns a boolean to indicate if the input variant is an error string
    */
    bool isError( const Variant & inputVariant, const bool & checkForError )
    {
        bool result = false;
        
        // Test if Variant type is STRING and that we are checking for Errors
        if ( inputVariant.getType() != STRING_VALUE || checkForError == false )
        {
            return result;
        }

        // Test if the inputVariant of type STRING starts with the AlgoQuantLib error symbol prefix '[' or '#'
        std::string inputValue = inputVariant.getValue<std::string>();
        if ( boost::starts_with( inputValue, "#" ) || boost::starts_with( inputValue, "[" ) )
        {
            result = true;
        }

        return result;
    }

    /* @brief			Function to remove a row from a matrix if it contains blanks, if a column number is specified then only that column is checked for blanks
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		onlyCheckColumnNumber		The column number to check for blanks, if not specified then all matrix columns will be checked
    *  @param [in]      removeErrors                Flag to remove errors, when TRUE will remove strings starting with error prefixes '[' and / or '#' symbols
    *  @return			Returns a Matrix with blank rows removed
    */
    VariantMatrix removeBlankRows( const VariantMatrix & inputMatrix, const int & onlyCheckColumnNumber, const bool & removeErrors )
    {
        // Access Violation Guard
        if( inputMatrix.size() == 0 )
        {
            return inputMatrix;
        }

        // Validation
        if ( onlyCheckColumnNumber != 0 )
        {
            // Note: Column Index as input is Base 1 and internal Column Index is Base 0
            AQ_REQUIRE( size_t( onlyCheckColumnNumber ) <= inputMatrix[0].size(), "Invalid column number. Column number is larger than total number of columns" )
            AQ_REQUIRE( onlyCheckColumnNumber > 0, "Invalid Column Number. Column number cannot be negative" )
        }

        VariantMatrix result;
        bool checkAllColumnsForBlanks       = ( onlyCheckColumnNumber == 0 ) ? true : false;
        bool isBlankOrErrorFound            = false;

        for( unsigned int row = 0; row < inputMatrix.size(); ++row )
        {
            isBlankOrErrorFound = false;

            if ( checkAllColumnsForBlanks )
            {
                // Check every column for blanks
                for( unsigned int col = 0; col < inputMatrix[0].size(); ++col )
                {
                    if ( inputMatrix[row][col].isEmpty()
                      || isError( inputMatrix[row][col], removeErrors ) )
                    {
                        isBlankOrErrorFound = true;
                        break;
                    }
                }
            }
            else
            {
                // Check just the column specified by onlyCheckColumnNumber variable
                // Note that we convert column indexing with base zero i.e. column Index - 1
                if ( inputMatrix[row][onlyCheckColumnNumber-1].isEmpty()
                    || isError( inputMatrix[row][onlyCheckColumnNumber-1], removeErrors ) )
                {
                    isBlankOrErrorFound = true;
                }
            }

            // Populate row no blanks or errors found
            if ( !isBlankOrErrorFound )
            {
                result.push_back( inputMatrix[row] );
            }
        }

        return result;
    }
    
    /* @brief			Function to remove a row from a matrix if the entire row is empty
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @return			Returns a Matrix with empty rows removed
    */
    VariantMatrix removeEmptyRows( const VariantMatrix & inputMatrix )
    {
        // Access Violation Guard
        if( inputMatrix.size() == 0 )
        {
            return inputMatrix;
        }

        VariantMatrix result;
        
        for( unsigned int row = 0; row < inputMatrix.size(); ++row )
        {
            // Check every column for blanks
            bool isEntireRowEmpty = true;
            
            for( unsigned int col = 0; col < inputMatrix[0].size(); ++col )
            {
                if ( !inputMatrix[row][col].isEmpty() )
                {
                    isEntireRowEmpty = false;
                    break;
                }
            }

            // Populate row no blanks or errors found
            if ( !isEntireRowEmpty )
            {
                result.push_back( inputMatrix[row] );
            }
        }

        return result;
    }

    /* @brief			Function to remove a column from a matrix if it contains blanks, if a row number is specified then only that column is checked for blanks
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		onlyCheckRowNumber		    The row number to check for blanks, if not specified then all matrix rows will be checked
    *  @param [in]      removeErrors                Flag to remove errors, when TRUE will remove strings starting with error prefixes '[' and / or '#' symbols
    *  @return			Returns a Matrix with blank columns removed
    */
    VariantMatrix removeBlankColumns( const VariantMatrix & inputMatrix, const int & onlyCheckRowNumber, const bool & removeErrors )
    {
        // Access Violation Guard
        if( inputMatrix.size() == 0 )
        {
            return inputMatrix;
        }

        // Validation
        if ( onlyCheckRowNumber != 0 )
        {
            // Note: Column Index as input is Base 1 and internal Column Index is Base 0
            AQ_REQUIRE( size_t( onlyCheckRowNumber ) <= inputMatrix.size(), "Invalid row number. Row number is larger than total number of rows" )
            AQ_REQUIRE( onlyCheckRowNumber > 0, "Invalid Row Number. Row number cannot be negative" )
        }

        VariantMatrix result;
        bool checkAllRowsForBlanks       = ( onlyCheckRowNumber == 0 ) ? true : false;
        bool isBlankOrErrorFound            = false;

        for( unsigned int col = 0; col < inputMatrix[0].size(); ++col )
        {
            isBlankOrErrorFound = false;

            if ( checkAllRowsForBlanks )
            {
                // Check every row for blanks
                for( unsigned int row = 0; row < inputMatrix.size(); ++row )
                {
                    if ( inputMatrix[row][col].isEmpty()
                      || isError( inputMatrix[row][col], removeErrors ) )
                    {
                        isBlankOrErrorFound = true;
                        break;
                    }
                }
            }
            else
            {
                // Check just the row specified by onlyCheckRowNumber variable
                // Note that we convert row indexing with base zero i.e. row Index - 1
                if ( inputMatrix[onlyCheckRowNumber-1][col].isEmpty()
                    || isError( inputMatrix[onlyCheckRowNumber-1][col], removeErrors ) )
                {
                    isBlankOrErrorFound = true;
                }
            }

            // Populate row no blanks or errors found
            if ( !isBlankOrErrorFound )
            {
                // Create Rows need to store the column result(s)
                if( result.size() == 0 )
                {
                    result.resize( inputMatrix.size() );
                }

                // Populate column results
                for( size_t rowIndex = 0; rowIndex < inputMatrix.size(); ++rowIndex )
                {
                    result[rowIndex].push_back( inputMatrix[rowIndex][col] );
                }
            }
        }

        return result;
    }
   
    /* @brief			Function to remove a column from a matrix if the entire column is empty
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @return			Returns a Matrix with empty columns removed
    */
    VariantMatrix removeEmptyColumns( const VariantMatrix & inputMatrix )
    {
        // Access Violation Guard
        if( inputMatrix.size() == 0 )
        {
            return inputMatrix;
        }

        VariantMatrix result;
        for( unsigned int col = 0; col < inputMatrix[0].size(); ++col )
        {
            // Check if each column is empty
            bool isEntireColumnEmpty  = true;

            for( unsigned int row = 0; row < inputMatrix.size(); ++row )
            {
                if ( !inputMatrix[row][col].isEmpty() )
                {
                    isEntireColumnEmpty = false;
                    break;
                }
            }

            // Populate row no blanks or errors found
            if ( !isEntireColumnEmpty )
            {
                // Create Rows need to store the column result(s)
                if( result.size() == 0 )
                {
                    result.resize( inputMatrix.size() );
                }

                // Populate column results
                for( size_t rowIndex = 0; rowIndex < inputMatrix.size(); ++rowIndex )
                {
                    result[rowIndex].push_back( inputMatrix[rowIndex][col] );
                }
            }
        }

        return result;
    }

    /* @brief			Function to remove a column from a matrix if the entire column is empty
    *  @param [in]		inputMatrix		            The raw inputMatrix
    *  @param [in]		blockName		            The name of the inputMatrix or dataBlock
    *  @return			Returns nothing, but will throw if an error input is encountered
    */
    void checkDataMatrixForErrors( const VariantMatrix & inputMatrix, const std::string blockName )
    {
        for( unsigned int i = 0; i<inputMatrix.size(); ++i )
        {
            for ( unsigned int j = 0; j<inputMatrix[i].size(); ++j )
            {
                AQ_REQUIRE( isError( inputMatrix[i][j], true /* CheckForErrors*/ ) == false /* Require No Errors*/, "Invalid Data: " + blockName + " Data Contains Errors" )
            }
        }
        return;
    }
    
    /* @brief			Function to append matrices below each other by row, matrix 2 will be placed below matrix 1.
    *  @param [inout]	matrix1		                Input matrix 1. On output, contains the augmented matrix
    *  @param [in]		matrix2		                Input matrix 2
    *  @return			Returns an augmented matrix with all matrices appended below each other by row, matrix 2 will be placed below matrix 1.
    */
    VariantMatrix appendRows( VariantMatrix & matrix1,
                              const VariantMatrix & matrix2 )
    {
        // No need to append matrix 2 if it is empty
        if ( matrix2.empty() || matrix2.size() == 0 ) return matrix1;
        
        AQ_REQUIRE( matrix1.size()>0,  "Matrices must contain the same number of Columns. Only regular rectangular matrices supported" )
        AQ_REQUIRE( matrix1[0].size() == matrix2[0].size(), "Matrices must contain the same number of Columns. Only regular rectangular matrices supported" )

        for ( size_t i = 0; i < matrix2.size(); ++i )
        {
            // Push back onto matrix 1 rows one row of matrix 2 at a time
            matrix1.push_back( matrix2[i] );
        }

        return matrix1;
    }


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
                              const VariantMatrix & matrix10 )
    {
        VariantMatrix result = matrix1;

        appendRows( result, matrix2 );
        appendRows( result, matrix3 );
        appendRows( result, matrix4 );
        appendRows( result, matrix5 );
        appendRows( result, matrix6 );
        appendRows( result, matrix7 );
        appendRows( result, matrix8 );
        appendRows( result, matrix9 );
        appendRows( result, matrix10 );

        return result;
    }


    /* @brief			Function to append matrices to the right of each other by column, matrix 2 will be placed to the right of matrix 1.
    *  @param [inout]	matrix1		                Input matrix 1. On output, contains the augmented matrix
    *  @param [in]		matrix2		                Input matrix 2
    *  @return			Returns an augmented matrix with all matrices to the right of each other by column, matrix 2 will be placed to the right of matrix 1.
    */
    VariantMatrix appendColumns( VariantMatrix & matrix1,
                                 const VariantMatrix & matrix2 )
    {
        // No need to append matrix 2 if it is empty
        if ( matrix2.empty() || matrix2.size() == 0 ) return matrix1;

        AQ_REQUIRE( matrix1.size()>0, "Matrices must contain the same number of rows. Only regular rectangular matrices supported" )
        AQ_REQUIRE( matrix1.size() == matrix2.size(), "Matrices must contain the same number of rows. Only regular rectangular matrices supported" )

        for ( size_t i = 0; i < matrix2.size(); ++i )
        {
            for ( size_t j = 0; j < matrix2[0].size(); ++j )
            {
                // Push back onto matrix1 rows one element of matrix2 at a time
                matrix1[i].push_back( matrix2[i][j] );
            }
        }

        return matrix1;
    }

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
    VariantMatrix appendColumns( const VariantMatrix & matrix1,
                                 const VariantMatrix & matrix2,
                                 const VariantMatrix & matrix3,
                                 const VariantMatrix & matrix4,
                                 const VariantMatrix & matrix5,
                                 const VariantMatrix & matrix6,
                                 const VariantMatrix & matrix7,
                                 const VariantMatrix & matrix8,
                                 const VariantMatrix & matrix9,
                                 const VariantMatrix & matrix10 )
    {
        VariantMatrix result = matrix1;

        appendColumns( result, matrix2 );
        appendColumns( result, matrix3 );
        appendColumns( result, matrix4 );
        appendColumns( result, matrix5 );
        appendColumns( result, matrix6 );
        appendColumns( result, matrix7 );
        appendColumns( result, matrix8 );
        appendColumns( result, matrix9 );
        appendColumns( result, matrix10 );

        return result;
    }

    /* @brief			Function to filter data for unique input 
    *  @param [in]		dataVector		            A vector of data
    *  @param [in]		displayByRow		        True = Display by Row, False = Display by Column, Default is False
    *  @return			Returns a vector of filtered data removing all duplicates
    */
    VariantMatrix dataFilter( const VariantVector & dataVector, const bool displayByRow )
    {
        VariantMatrix uniqueResults;
        VariantVector resultRow;

        // Insert all Elements into a std::set which will exclude duplicates
        std::set< etrading::Variant> setData;
        
        for( size_t i = 0; i < dataVector.size(); ++i )
        {
            // Set Result Type
            // ------------------------------------------------------------------------------------
            // std::set returns std::pair<iterator, bool>, where the bool is false when the insertion failed 
            // by adding duplicate values for example
            auto setResult = setData.insert( dataVector[i] );

            // Insert result into the results container if not a duplicate
            if ( setResult.second )
            {
                if( !displayByRow )
                {
                    // Display by column means store all results in a single row vector
                    resultRow.push_back( dataVector[i] );                    
                }
                else
                {
                    // Display by row means each row contains a single element of the results vector        
                    VariantVector thisRow( 1, dataVector[i] );
                    uniqueResults.push_back( thisRow );
                }
            }
        }

        // Display by column means results matrix contains a single results row
        if ( !displayByRow )
        {
            uniqueResults.push_back( resultRow );
        }

        return uniqueResults;
    }

	/* @brief Returns a VariantMatrix containing the configuration information for the specified propertyKey
	*  @param [in]	freeObject		The object containing blocks of data indexed by propertyKey
	*  @param [in]  propertyKey		The property data block to be retrieved from the freeObject
	*  @param [in]  throwIfMissing	Whether to throw an exception if the specified propertyKey is missing. Default behaviour is to throw.
	*  @returns		A VariantMatrix containing the data specified by propertyKey
	*/
	VariantMatrix getVariantMatrixFromFreeObject( const FreeObject& freeObject, const std::string& propertyKey, const bool throwIfMissing )
	{
	
		VariantMatrix variantMatrix;

		if ( freeObject.doesKeyExist( propertyKey ) )
		{
			// extract just the data for the specified property
			const etrading::VariantMatrix& allDataView = freeObject.viewAllData();
			const auto columnIndices = freeObject.columnsOfSchema( propertyKey );
        
			std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView]( const int idx )
			{
				variantMatrix.push_back( allDataView[ idx ] );
			} );
		}
		else
		{
			// The freeObject does not contain propertyKey
			if ( throwIfMissing )
			{
				AQ_THROW( "PropertyKey '" + propertyKey + "' not found in input data." );
			}
		}

		return variantMatrix;
	}

	/* @brief Returns a LAStringMatrix containing the configuration information for the specified propertyKey
	*  @param [in]	freeObject		The object containing blocks of data indexed by propertyKey
	*  @param [in]  propertyKey		The property data block to be retrieved from the freeObject
	*  @param [in]  trimBlankRows	Whether to trim blank rows at the end of the block. Default behaviour is to trim
	*  @param [in]  throwIfMissing	Whether to throw an exception if the specified propertyKey is missing. Default behaviour is to throw.
	*  @returns		A VariantMatrix containing the data specified by propertyKey
	*/
	LAStringMatrix getLAStringMatrixFromFreeObject( const FreeObject& freeObject, const std::string& propertyKey, const bool trimBlankRows, const bool throwIfMissing)
	{
		VariantMatrix variantMatrix = getVariantMatrixFromFreeObject( freeObject, propertyKey, throwIfMissing);

		LAStringMatrix tmpMatrix;
		if (variantMatrix.size() == 0)
		{
			return tmpMatrix;
		}


		for (size_t i=0; i<variantMatrix.size(); i++)
		{
			LAStringVector stringVector;
			auto columnData = variantMatrix[i];
			for (size_t j=0; j<columnData.size(); j++)
			{
				stringVector.push_back( variantMatrix[i][j] );
			}
			tmpMatrix.push_back( stringVector );
		}

		// AlgoQuantLib LAStringMatrix is represented as : rows x cols
		// whereas VariantMatrix is:  cols x rows
		LAStringMatrix resultMatrix = transpose( tmpMatrix );

		// There may be one or more blank rows at the end of the block
		if ( trimBlankRows )
		{
			size_t nRows = resultMatrix.size();
			while ( (nRows > 0)  && (resultMatrix[nRows-1][0] == "") )
			{
				nRows--;
			}

			// Copy the remaining rows to a new matrix
			LAStringMatrix trimmedMatrix;
			for (size_t i=0; i<nRows; i++)
			{
				trimmedMatrix.push_back( resultMatrix[i] );
			}
		
			return trimmedMatrix;
		}

		return resultMatrix;
	}

    /* @brief Returns a LAStringMatrix containing the configuration information for the specified propertyKey
	*  @param [in]	freeObject		The object containing blocks of data indexed by propertyKey
	*  @param [in]  propertyKey		The property data block to be retrieved from the freeObject
	*  @param [in]  trimBlankRows	Whether to trim blank rows at the end of the block. Default behaviour is to trim
	*  @returns		A VariantMatrix containing the data specified by propertyKey
	*/
	StandardStringMatrix getStandardStringMatrixFromFreeObject( const FreeObject& freeObject, const std::string& propertyKey, const bool trimBlankRows )
	{
		VariantMatrix variantMatrix = getVariantMatrixFromFreeObject( freeObject, propertyKey );

		StandardStringMatrix tmpMatrix;
		for (size_t i=0; i<variantMatrix.size(); i++)
		{
			StandardStringVector stringVector;
			auto columnData = variantMatrix[i];
			for (size_t j=0; j<columnData.size(); j++)
			{
				stringVector.push_back( variantMatrix[i][j] );
			}
			tmpMatrix.push_back( stringVector );
		}

		// AlgoQuantLib LAStringMatrix is represented as : rows x cols
		// whereas VariantMatrix is:  cols x rows
		StandardStringMatrix resultMatrix = transpose( tmpMatrix );

		// There may be one or more blank rows at the end of the block
		if ( trimBlankRows )
		{
			size_t nRows = resultMatrix.size();
			while ( (nRows > 0)  && (resultMatrix[nRows-1][0] == "") )
			{
				nRows--;
			}

			// Copy the remaining rows to a new matrix
			StandardStringMatrix trimmedMatrix;
			for (size_t i=0; i<nRows; i++)
			{
				trimmedMatrix.push_back( resultMatrix[i] );
			}
		
			return trimmedMatrix;
		}

		return resultMatrix;
	}

	// Function to trim a LA String Vector and remove blanks
	LAStringVector trimLAStringVector(const LAStringVector & inputVector)
	{
		LAStringVector results;
		for (size_t i = 0; i < inputVector.size(); ++i)
		{
			if ( inputVector[i].size() != 0 ) results.push_back( inputVector[i] );
		}
		return results;
	}

	// Function to trim a Standard String Vector and remove blanks
	StandardStringVector trimStandardStringVector(const StandardStringVector & inputVector)
	{
		StandardStringVector results;
		for (size_t i = 0; i < inputVector.size(); ++i)
		{
			if ( inputVector[i].size() != 0 ) results.push_back( inputVector[i] );
		}
		return results;
	}

    // Function to trim a LA String Matrix and remove blanks
	LAStringMatrix trimLAStringMatrix(const LAStringMatrix & inputMatrix)
    {
        // Access Violation Guard
        if( inputMatrix.size() == 0 )
        {
            return inputMatrix;
        }

        LAStringMatrix result;
        
        for( unsigned int row = 0; row < inputMatrix.size(); ++row )
        {
            // Check every column for blanks
            bool isEntireRowEmpty = true;
            
            for( unsigned int col = 0; col < inputMatrix[0].size(); ++col )
            {
                if ( inputMatrix[row][col].size() != 0 )
                {
                    isEntireRowEmpty = false;
                    break;
                }
            }

            // Populate row no blanks or errors found
            if ( !isEntireRowEmpty )
            {
                result.push_back( inputMatrix[row] );
            }
        }

        return result;
    }

	// Function to trim a Standard String Matrix and remove blanks
	StandardStringMatrix trimStandardStringMatrix(const StandardStringMatrix & inputMatrix)
    {
        // Access Violation Guard
        if( inputMatrix.size() == 0 )
        {
            return inputMatrix;
        }

        StandardStringMatrix result;
        
        for( unsigned int row = 0; row < inputMatrix.size(); ++row )
        {
            // Check every column for blanks
            bool isEntireRowEmpty = true;
            
            for( unsigned int col = 0; col < inputMatrix[0].size(); ++col )
            {
                if ( inputMatrix[row][col].size() != 0 )
                {
                    isEntireRowEmpty = false;
                    break;
                }
            }

            // Populate row no blanks or errors found
            if ( !isEntireRowEmpty )
            {
                result.push_back( inputMatrix[row] );
            }
        }

        return result;
    }

	// Function to convert a LAStringMatrix to a VariantMatrix
	etrading::VariantMatrix convertLAStringMatrixToVariantMatrix( LAStringMatrix stringMatrix )
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

	// Function to convert a Standard StringMatrix to a VariantMatrix
	etrading::VariantMatrix convertStandardStringMatrixToVariantMatrix( StandardStringMatrix stringMatrix )
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

	/* @brief Returns the configuration information for the specified propertyKey. If propertyKey is blank, all properties are returned.
	*  @param [in]	freeObject		The object containing blocks of data indexed by propertyKey
	*  @param [in]  propertyKey		The property data block to be retrieved from the freeObject
	*  @param [out]  A VariantMatrix containing a LabelValue block of properties
	*/
	VariantMatrix viewInputParametersFromFreeObject(const FreeObject& freeObject, const std::string& propertyKey)
	{
		VariantMatrix inputParameters;

		// If no propertyKey is specified, Return a matrix containing all properties
		if (propertyKey == "" || propertyKey == "NONE")
		{
			bool addSeparatorRow = false;
			const std::vector<std::string> keyNames = freeObject.keyNames();
			for (auto it = keyNames.begin(); it != keyNames.end(); ++it)
			{
				if (addSeparatorRow)
				{
					std::vector<Variant> row(2, "");
					inputParameters.push_back(row);
				}
				addSeparatorRow = true; // Add a separator before all but the first row

				std::string propertyKey = *it;
				std::vector<Variant> row(2, "");
				row[0] = propertyKey;
				inputParameters.push_back(row);

				VariantMatrix variantMatrix = transpose(getVariantMatrixFromFreeObject(freeObject, propertyKey));
				for (unsigned int i = 0; i < variantMatrix.size(); i++)
				{
					inputParameters.push_back(variantMatrix[i]);
				}
			}
		}

		// extract just the data for the specified property
		inputParameters = getVariantMatrixFromFreeObject(freeObject, propertyKey);

		return transpose(inputParameters);
	}


}