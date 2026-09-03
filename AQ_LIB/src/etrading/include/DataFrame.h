#pragma once

#include "CoreEnumerations.h"
#include "Variant.h"

#include <string>
#include <vector>


namespace etrading
{
	class DataFrame
	{
	public:

		// Default constructor
		DataFrame();

		// Constructor which reads in values from a csvFile.
		DataFrame( const std::string& csvFilename );

		/* @brief	Constructor from a vector of column headings and a VariantMatrix of values
		*  @param[in]	columnHeadings		The name of each column in the dataBlock variant matrix
		*  @oaram[in]	dataBlock			The data arranged as a matrix. The number of columns in the matrix must match the number of columnHeadings
		*/
		DataFrame( const std::vector<std::string>& columnHeadings, const VariantMatrix& dataBlock );

		// Extends the dataframe by adding an additional column of values.
		// If the DataFrame already contains a column with this name, the original column of values is replaced.
		void addColumn( const std::string& columnName, const VariantVector& values );

		// Extend the dataframe by adding an additional column of type  T
		template<typename T>
		void addColumn( const std::string& columnName, const std::vector<T>& values );

		// Extend the dataframe by adding an additional column of type  T
		// This version accepts a prefix which is attached to the column name.
		template<typename T>
		void addColumnWithPrefix( const std::string& prefix, const std::string& columnName, const std::vector<T>& values );

		// Extend the dataframe by adding an additional column of type double
		// This is a specialization which checks for double isnan()
		// This version accepts a prefix which is attached to the column name.
		void addColumnWithPrefix( const std::string& prefix, const std::string& columnName, const std::vector<double>& values );

		// Extend the dataframe by adding an additional column of type  LADate
		// This is a specialization which checks if the LADate is valid.
		// This version accepts a prefix which is attached to the column name.
		void addColumnWithPrefix( const std::string& prefix, const std::string& columnName, const std::vector<LADate>& values );

		// Extends an existing dataframe by adding an additional column of values, where the values on each row are the same.
		// If the DataFrame already contains a column with this name, the original column of values is replaced.
		void addColumn( const std::string& columnName, const double& value );

		// Attach the rhs dataframe onto the side of this dataframe.
		void combineColumns( const DataFrame& rhs );

		// Retrieves a column of values from the DataFrame by columnName.
		std::vector<Variant> getColumn( const std::string& columnName ) const;

		// Retrieves a column of values from the DataFrame by columnName. The values in the result vector are coerced to double
		std::vector<double> getColumnAsDoubleVector( const std::string& columnName ) const;

		// Retrieves a column of values from the DataFrame by columnName. The values in the result vector are coerced to double
		std::vector<int> getColumnAsIntVector( const std::string& columnName ) const;

		// Returns the number of rows of data in the DataFrame
		size_t numRows() const;

		// Returns the number of columns of data in the DataFrame
		size_t numCols() const;

		bool empty() const;

		// Returns true if the DataFrame contains the specified column
		bool hasColumn( const std::string& columnName ) const;

		// Returns the column names of this data frame
		std::vector<std::string> getColumnNames() const;

		// Returns the data block of this data frame
		VariantMatrix getDataAsVariantMatrix() const;

	private:
		std::vector<std::string> columnNames_;

		VariantMatrix variantMatrix_;

	};

}

