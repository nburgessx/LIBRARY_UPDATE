/*
 * @brief			Structured Credit DataFrame class
 *					Represents an "R" DataFrame, which consists of a matrix with column names
 *
 *					This code was ported from MGEN AlgoQuantHub R analytics library
 *					Author Andrew Friend
 *
 * @Created:		28th Nov 2019
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 */

#include "DataFrame.h"
#include "ExceptionMacros.h"
#include "DataUtilities.h"			// For MLIB_TO_STRING macros
#include "LADateScheduleHelpers.h"	// isValidDate()

#include "Variant.h"
#include "ExceptionMacros.h"

#include <algorithm>
#include <cmath>


namespace etrading
{

	namespace
	{
		// Helper functions to assist with loading loan CSV files

		/* @brief Converts a CSV delimited line of text into tokens
		*  @param[in]	line	A comma delimited line of text
		*  @returns		The CSV line converted broken into string tokens
		*/
		std::vector<std::string> parseCSVLine( const std::string& line )
		{
			std::stringstream ss(line);
			std::vector<std::string> tokens;

			std::string token;
			while ( getline(ss, token, ',') )
			{
				tokens.push_back( token );
			}
			return tokens;
		}

		// Creates a vector of Variant from an input string of tokens. Used for parsing CSV files.
		VariantVector extractRowValues( const std::vector<std::string>& tokens )
		{
			VariantVector values;
			values.reserve( tokens.size() );

			for ( auto item : tokens )
			{
				Variant value(item.c_str());
				values.push_back( value );
			}

			return values;
		}
	}
	
	// A helper method which coerces the values in a Variant vector into the specified type, returning a new vector
	template<typename T>
	std::vector<T> convertVariantVectorToVectorOfType( const VariantVector& variantVector )
	{
		std::vector<T> valueVector;
		valueVector.reserve( variantVector.size() );
		for ( auto variantValue : variantVector )
		{
			// Note:: Just truncate / round-down
			double value = variantValue.getValue<T>();
			valueVector.push_back( value );
		}
		return valueVector;
	}


	// Default constructor
	DataFrame::DataFrame( )
	{}

	// Constructor which reads in values from a csvFile.
	DataFrame::DataFrame( const std::string& csvFilename )
	{
		std::ifstream infile( csvFilename.c_str() );
		if ( !infile )
		{
			MLIB_THROW( "Unable to open file: " + csvFilename );
		}

		std::string line;
		std::getline( infile, line );

		// Read in column headings and uppercase
		columnNames_ = parseCSVLine( line );
		for( std::string& columnName : columnNames_ )
		{
			std::transform( columnName.begin(), columnName.end(), columnName.begin(),
				[]( char c ) { return std::toupper( c ); });
		}
		const size_t numColumns = columnNames_.size();

		int row = 2;
		while ( std::getline( infile, line ) )
		{
			std::vector<std::string> tokens = parseCSVLine( line );
			VariantVector values = extractRowValues( tokens );

			MLIB_REQUIRE(values.size() == numColumns, "Inconsistent number of columns in file " << csvFilename << " at row " << row);
			variantMatrix_.push_back( values );
			row++;
		}
	}

	/* @brief	Constructor from a vector of column headings and a VariantMatrix of values
	*  @param[in]	columnHeadings		The name of each column in the dataBlock variant matrix
	*  @oaram[in]	dataBlock			The data arranged as a matrix. The number of columns in the matrix must match the number of columnHeadings
	*/
	DataFrame::DataFrame( const std::vector<std::string>& columnHeadings, const VariantMatrix& dataBlock )
		: columnNames_( columnHeadings ), variantMatrix_( dataBlock )
	{
		// Perform sanity checking of the input values
		const size_t dataRows = dataBlock.size();
		MLIB_REQUIRE( dataRows > 0, "Require at least 1 row of data to create a DataFrame" );

		const size_t nCols = columnHeadings.size();
		const size_t dataCols = dataBlock[0].size();
		MLIB_REQUIRE( nCols == dataCols, "Number of column headings and data columns should match" );
	}


	// Extends an existing dataframe by adding an additional column of values.
	// If the DataFrame already contains a column with this name, the original column of values is replaced.
	void DataFrame::addColumn( const std::string& columnName, const VariantVector& values )
	{
		const size_t nRowsInNewColumn = values.size();

		auto iter = std::find(columnNames_.begin(), columnNames_.end(), columnName);
		if ( iter == columnNames_.end() )
		{
			// This is a new column. Create a new column and append to the right of the matrix.
			columnNames_.push_back( columnName );

			if ( variantMatrix_.size() == 0 )
			{
				// We start with an empty variantMatrix / dataframe. Allocate the required number of rows				
				variantMatrix_.resize( nRowsInNewColumn );
			}

			MLIB_REQUIRE( nRowsInNewColumn == variantMatrix_.size(), "Incorrect number of rows in the input column" );

			// Note that row below is intentionally a reference in order to update the variantMatrix.
			size_t rowIdx = 0;
			for ( auto& row : variantMatrix_ )
			{
				Variant value = values[ rowIdx ];
				row.push_back( value );
				rowIdx++;
			}
		}
		else
		{
			// Column already exists. Update the existing column

			MLIB_REQUIRE( nRowsInNewColumn == variantMatrix_.size(), "Incorrect number of rows in the input column" );

			size_t rowIdx = 0;
			size_t columnIdx = std::distance( columnNames_.begin(), iter );
			for ( auto& row : variantMatrix_ )
			{
				Variant value = values[ rowIdx ];
				row[ columnIdx ] = value;
				rowIdx++;
			}
		}
	}

	// Extend the dataframe by adding an additional column of type  T
	template<typename T>
	void DataFrame::addColumn( const std::string& columnName, const std::vector<T>& values )
	{
		const size_t nValues = values.size();

		VariantVector variantVector(nValues);
		for (size_t i = 0; i < nValues; i++)
		{
			variantVector[i] = values[i];
		}
		addColumn( columnName, variantVector );
	}

	// Extend the dataframe by adding an additional column of type  T
	// This version accepts a prefix which is attached to the column name.
	template<typename T>
	void DataFrame::addColumnWithPrefix( const std::string& prefix, const std::string& columnName, const std::vector<T>& values )
	{
		const size_t nValues = values.size();

		VariantVector variantVector( nValues );
		for (size_t i = 0; i < nValues; i++)
		{
			variantVector[i] = values[i];
		}

		std::string columnNameWithPrefix = prefix + columnName;
		addColumn( columnNameWithPrefix, variantVector );
	}

	// Extend the dataframe by adding an additional column of type double
	// This is a specialization which checks for double isnan()
	// This version accepts a prefix which is attached to the column name.
	void DataFrame::addColumnWithPrefix( const std::string& prefix, const std::string& columnName, const std::vector<double>& values )
	{
		const size_t nValues = values.size();

		VariantVector variantVector(nValues);
		for (size_t i = 0; i < nValues; i++)
		{
			double doubleValue = values[i];
			if (! std::isnan( doubleValue ) )
			{
				variantVector[i] = values[i];
			}
			else
			{
				variantVector[i] = "#N/A";
			}
		}

		std::string columnNameWithPrefix = prefix + columnName;
		addColumn( columnNameWithPrefix, variantVector );
	}

	// Extend the dataframe by adding an additional column of type  LADate
	// This is a specialization which checks if the LADate is valid.
	// This version accepts a prefix which is attached to the column name.
	void DataFrame::addColumnWithPrefix( const std::string& prefix, const std::string& columnName, const std::vector<LADate>& values )
	{
		const size_t nValues = values.size();

		VariantVector variantVector( nValues );
		for (size_t i = 0; i < nValues; i++)
		{
			LADate dateValue = values[i];
			if ( LADateScheduleHelpers::isValidDate( dateValue ) )
			{
				variantVector[i] = values[i];
			}
			else
			{
				variantVector[i] = "#N/A";
			}
		}

		std::string columnNameWithPrefix = prefix + columnName;
		addColumn( columnNameWithPrefix, variantVector );
	}


	// Extends an existing dataframe by adding an additional column of values, where the values on each row are the same.
	// If the DataFrame already contains a column with this name, the original column of values is replaced.
	void DataFrame::addColumn( const std::string& columnName, const double& value )
	{
		auto iter = std::find(columnNames_.begin(), columnNames_.end(), columnName );
		if ( iter == columnNames_.end() )
		{
			// This is a new column. Create a new column and append to the right of the matrix.
			columnNames_.push_back( columnName );
			// Note that row below is intentionally a reference in order to update the variantMatrix.
			for ( auto& row : variantMatrix_ )
			{
				row.push_back( value );
			}
		}
		else
		{
			// Column already exists. Update the existing column
			size_t columnIdx = std::distance( columnNames_.begin(), iter );
			for ( auto& row : variantMatrix_ )
			{
				row[columnIdx] = value;
			}
		}
	}

	// Attach the rhs dataframe onto the side of this dataframe.
	void DataFrame::combineColumns( const DataFrame& rhs )
	{
		const size_t nColsToAdd = rhs.numCols();

		for ( size_t i=0; i<nColsToAdd; i++ )
		{
			const std::string columnName = rhs.columnNames_[ i ];

			const VariantVector column = rhs.getColumn( columnName );
			addColumn( columnName, column );
		}
	}

	// Retrieves a column of values from the DataFrame by columnName.
	VariantVector DataFrame::getColumn( const std::string& columnName ) const
	{
		auto iter = std::find( columnNames_.begin(), columnNames_.end(), columnName);
		MLIB_REQUIRE(iter != columnNames_.end(), "Column name '" << columnName << "' not found within data block." );

		size_t columnIdx = std::distance(columnNames_.begin(), iter);

		VariantVector column;
		for (auto row : variantMatrix_)
		{
			column.push_back( row[ columnIdx ]);
		}
		return column;
	}

	std::vector<double> DataFrame::getColumnAsDoubleVector( const std::string& columnName) const
	{
		const VariantVector column = getColumn(columnName);
		const std::vector<double> doubleVector = convertVariantVectorToVectorOfType<double>( column );
		return doubleVector;
	}
	// Retrieves a column of values from the DataFrame by columnName. The values in the result vector are coerced to int
	std::vector<int> DataFrame::getColumnAsIntVector( const std::string& columnName ) const
	{
		VariantVector column = getColumn( columnName );
		std::vector<int> intVector = convertVariantVectorToVectorOfType<int>( column );
		return intVector;
	}

	// Returns the number of rows of data in the DataFrame
	size_t DataFrame::numRows() const
	{
		const size_t nRows = variantMatrix_.size();
		return nRows;
	}

	// Returns the number of columns of data in the DataFrame
	size_t DataFrame::numCols() const
	{
		size_t nCols = 0;

		if (numRows() > 0)
		{
			nCols = variantMatrix_[0].size();
		}
		return nCols;
	}

	// Returns true if the DataFrame contains the specified column
	bool DataFrame::hasColumn( const std::string& columnNameToFind ) const
	{
		auto iter = std::find(columnNames_.begin(), columnNames_.end(), columnNameToFind );
		if (iter == columnNames_.end())
		{
			return false;
		}
		return true;
	}

	bool DataFrame::empty() const
	{
		const bool isEmpty = ( numRows() == 0 );
		return isEmpty;
	}

	// Returns the column names of this data frame
	std::vector<std::string> DataFrame::getColumnNames() const
	{
		return columnNames_;
	}

	// Returns the data block of this data frame
	VariantMatrix DataFrame::getDataAsVariantMatrix() const
	{
		return variantMatrix_;
	}

	// template instantiations
	template
	void DataFrame::addColumn<int>( const std::string& columnName, const std::vector<int>& values );

	template
	void DataFrame::addColumn<double>( const std::string& columnName, const std::vector<double>& values );

	template
	void DataFrame::addColumn<std::string>( const std::string& columnName, const std::vector<std::string>& values );

	template
	void DataFrame::addColumn<LADate>( const std::string& columnName, const std::vector<LADate>& values );

	template
	void DataFrame::addColumnWithPrefix<int>( const std::string& prefix, const std::string& columnName, const std::vector<int>& values );

	template
	void DataFrame::addColumnWithPrefix<std::string>( const std::string& prefix, const std::string& columnName, const std::vector<std::string>& values );

}

