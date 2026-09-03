#include "TypeUtilities.h"
#include "AnyTypeUtilities.h"       // Boost AnyType Matrix Helpers
#include "ScheduleValidation.h"     // transpose()
#include "ParameterValidation.h"
#include "DateUtilities.h"          // date to string converters

namespace swig
{
	/* @brief		build LAStringMatrix from a vector of string vectors
	*  @param [out]		sMatrix					the LAStringMatrix object being built
	*  @param [in]		inputData				A vector of string vectors
	*  @param [in]		curveTypeEnum			The curveType which thw data block "rhs" comes from.
	*  @param [in]		curveMarketDataEnum		The name of the curve data block contained in inputData.
	*/
	void buildStringMatrix( LAStringMatrix& sMatrix, const std::vector<std::vector<std::string> >& inputData,	
							const etrading::CurveTypeEnum& curveTypeEnum,
							const etrading::CurveMarketDataEnum& curveMarketDataEnum )
	{
		sMatrix.clear();
		
		for(size_t i=0; i<inputData.size(); ++i)
		{
			std::vector<LAString> mbStr;
			std::vector<std::string> vec = inputData[i];
			for(size_t j=0; j<vec.size(); ++j)
			{
				mbStr.push_back(LAString(vec[j].c_str()));
			}

			sMatrix.push_back(mbStr);
		}
	}

#if defined(SWIG_R) || defined(SWIGR)
	// A structure which specifies the matrix dimensions for an R matrix.
	// i.e. the number of rows and columns in the matrix.
	struct MatrixDimension
	{
		size_t nRows;
		size_t nCols;
	};

	/* Utility method which determines if the curve inputData block contains an additional "USE" data column,
	 * which controls whether that row of market data is used in curve building.
	 * Only required for R API, where we figure out the dimensions of the input matrix.
	*/
	bool marketDataContainsBoolColumn( const SWIG_STRINGMATRIX&  inputData )
	{
		bool boolUseDataColumnExists = false;
		const size_t nItems = inputData.size();
		for (size_t i=0; i<nItems; i++ )
		{
			if ( inputData[i] == "TRUE" || inputData[i] == "FALSE")
			{
				boolUseDataColumnExists = true;
				break;
			}
		}
		return boolUseDataColumnExists;
	}

	/* @brief		Determines the dimensions of the R matrix represented by curveDataBlock.
	*				R matrices are passed around as vectors of string, so
	*				this function determines the actual dimensions of the matrix.
	*
	*  @param [in]	curveTypeEnum			The curveType of the curve that is to be built.
	*  @param [in]	curveMarketDataEnum		The name of the curve data block contained in inputData.
	* @param[in]	inputData				The block of curve data under consideration
	*/
	MatrixDimension getMatrixDimension( const etrading::CurveTypeEnum& curveTypeEnum,
										const etrading::CurveMarketDataEnum& curveMarketDataEnum,
										const SWIG_STRINGMATRIX inputData )
	{
		// Set a default number of columns
		size_t nCols = 2;

		// Rules for determining the size of data blocks
		switch( curveTypeEnum )
		{
		case etrading::OIS_CURVETYPE:
		{
			if ( curveMarketDataEnum == etrading::OIS_MARKETDATA )
			{
				nCols = 4; // Term  Rate  StartDate  EndDate 
				if ( marketDataContainsBoolColumn( inputData ) )
				{
					nCols = 5;  // Term  Rate  StartDate  EndDate UseData
				}
			}
			break;
		}
		case etrading::SWAP_CURVETYPE:
		{
			if ( curveMarketDataEnum == etrading::FUTURES_MARKETDATA )
			{
				nCols = 5;  // Term  StartDate  EndDate  Rate  Volatility
			}
			if ( curveMarketDataEnum == etrading::SWAP_MARKETDATA )
			{
				nCols = 2;   // Term  Rate
				if ( marketDataContainsBoolColumn( inputData ))
				{
					nCols = 3;  // Term  Rate  UseData
				}
			}
			break;
		}
		default:
			break;
		}

		const size_t nItems = inputData.size();
		MatrixDimension matrixDimension;
		matrixDimension.nCols = nCols;
		matrixDimension.nRows = nItems / nCols;
		
		return matrixDimension;
	}

    /* @brief			Build LAStringMatrix from input type SWIG_STRINGMATRIX.
	*                   In "R", the SWIG_STRINGMATRIX is defined as vector< string >.
	*					In all other languages, SWIG_STRINGMATRIX is a vector of string vectors.
	*					The curveTypeEnum and curveMarketDataEnum is used to figure out how many columns there should be in the final matrix.
	*
	*  @param [out]		sMatrix					The string matrix object being built
	*  @param [in]		inputData				An R block of data, represented as a vector of string
	*  @param [in]		curveTypeEnum			The curveType of the curve that is to be built.
	*  @param [in]		curveMarketDataEnum		The name of the curve data block contained in inputData.
	*/
	void buildStringMatrix( LAStringMatrix& sMatrix, const SWIG_STRINGMATRIX & inputData,
							const etrading::CurveTypeEnum& curveTypeEnum,
							const etrading::CurveMarketDataEnum& curveMarketDataEnum )
	{
		sMatrix.clear();

		MatrixDimension matrixDimension = getMatrixDimension( curveTypeEnum, curveMarketDataEnum, inputData );

		for(size_t i=0; i<matrixDimension.nRows; ++i)
		{
			std::vector<LAString> rowData;
			for (size_t j=0; j<matrixDimension.nCols; ++j )
			{
				rowData.push_back( LAString( inputData[matrixDimension.nRows * j + i ].c_str() ) );
			}
			sMatrix.push_back( rowData );
		}
	}
#endif


	/* @brief			build DateVector from a vector of strings 
	*  @param [out]		toDateVector		Result Vector of Dates
	*  @param [in]		fromStringVector	A vector of strings
	*/
	void buildDateVector( DateVector& toDateVector, const std::vector<std::string>& fromStringVector )
	{
		toDateVector.clear();
        toDateVector.reserve( fromStringVector.size() );

		for( size_t i = 0; i < fromStringVector.size(); ++i )
		{
			LADate laDate( etrading::stringToDate( fromStringVector[i] ) );
			toDateVector.push_back(laDate);
		}
	}

    /* @brief			build boost::gregorian::date Vector from a vector of strings 
	*  @param [out]		toDateVector			Result Vector of Dates
	*  @param [in]		fromStringVector		A vector of strings
	*/
	void buildGregorianDateVector( std::vector<boost::gregorian::date> & toDateVector, const std::vector<std::string>& fromStringVector )
    {
        toDateVector.clear();
        toDateVector.reserve( fromStringVector.size() );

		for( size_t i = 0; i < fromStringVector.size(); ++i )
		{
			boost::gregorian::date gregorianDate( etrading::validateAndConvertStringToGregorianDate( fromStringVector[i] ) );
            toDateVector.push_back( gregorianDate );
		}
    }

    /* @brief			build DateVector from a vector of strings 
	*  @param [out]		dVector			A vector of dates
	*  @param [in]		rhs				a vector of strings
	*/
	void buildStringVectorFromDateVector( StandardStringVector& toStringVector, const DateVector& fromDateVector )
	{
		toStringVector.clear();
        toStringVector.reserve( fromDateVector.size() );

		for( size_t i = 0; i < fromDateVector.size(); ++i )
		{
			LAString dateString( etrading::toYYYYMMDDFromDate( fromDateVector[i] ) );
			toStringVector.push_back( dateString.c_str() );
		}
	}

	/* @brief			build LAStringVector from a vector of strings 
	*  @param [out]		sVector			A LAStringVector object
	*  @param [in]		rhs				a vector of strings
	*/
	void buildStringVector(LAStringVector& sVector, const std::vector<std::string>& rhs)
	{
		sVector.clear();

		for(size_t i=0; i<rhs.size(); ++i)
		{
			LAString temp(rhs[i].c_str());
			sVector.push_back(temp);
		}
	}

    /* @brief			Build a LADate from a std::string type
	*  @param [in]		dateString		String Date
    *  @param [in]		dateFormat		Date Format, typically "YYYYMMDD"
	*  @output			Date in LADate format
	*/
    LADate fromStringToLADate(const std::string& dateString, const std::string& dateFormat )
    {
        const LADate result( dateString.c_str(), dateFormat.c_str() );
        return result;
    }

	/* @brief			build a vector of strings from a DoubleVector object
	*  @param [in]		inVal	input value as a string
	*  @output			output value as a LAString
	*/
	LAString fromStringToLAString(const std::string& inVal) 
	{
		LAString outVal(inVal.c_str()); 
		return outVal; 
	}

	/* @brief			Check each row of the input matrix and keep a record of the maximum column size. This is designed to work with ragged matrices.
	*  @param [in]		inputMatrix	The matrix of type T to check.
	*  @param [in]		checkForEmptyMatrix	Specify whether to throw a message if the number of rows or number of columns is zero.
	*  @output			The maximum column size
	*/
	template<typename T>
	size_t getMatrixMaxColumnDimension( const std::vector< std::vector< T > >& inputMatrix, const bool checkForEmptyMatrix )
	{
		size_t maxColumnSize = 0;
		const size_t rowSize = inputMatrix.size();

		if ( checkForEmptyMatrix )
		{
			AQ_REQUIRE( rowSize > 0, "There are no rows to display." );
		}

		//Column size, check the column size of each row and get the max column size
		for (size_t j = 0; j < rowSize; ++j)
		{
			size_t curColumnSize = inputMatrix[j].size();
			if (maxColumnSize < curColumnSize)
			{
				maxColumnSize = curColumnSize;
			}
		}

		if ( checkForEmptyMatrix )
		{
			AQ_REQUIRE( maxColumnSize > 0, "There are no columns to display." );
		}
		
		return maxColumnSize;
	}

	/* @brief			build a matrix of strings from a LAStringMatrix object
	*  @param [in]		LAStringMatrix	a LAStringMatrix object
	*  @output			output a matrix of strings
	*/
	SWIG_STRINGMATRIX fromStringMatrixToMatrixOfString( const LAStringMatrix& laStringMatrix )
	{
		const bool checkForEmptyMatrix = true;
		const size_t maxColumnSize = getMatrixMaxColumnDimension( laStringMatrix, checkForEmptyMatrix );
	
		// String Matrices Require Special Treatment for R API
		// ---------------------------------------------------
		#if defined(SWIG_R) || defined(SWIGR)

			// R_API treats Matrices as Vectors
			// ----------------------------------			
			SWIG_STRINGMATRIX result;

			// Store the matrix values into the R vector, column by column

			// 2) Populate data, padding spaces with blank strings
			for (size_t j = 0; j < maxColumnSize; ++j)
			{
				for (size_t i = 0; i < laStringMatrix.size(); ++i)
				{
					if (j < laStringMatrix[i].size())
					{
						result.push_back( laStringMatrix[i][j].getCString() );
					}
					else
					{
						result.push_back( "" );
					}
				}
			}
			return result;
		
		#else

			SWIG_STRINGMATRIX result;

			// 2) Populate data, padding spaces with blank strings
			for (size_t i = 0; i < laStringMatrix.size(); ++i)
			{
				std::vector<std::string> thisRow( maxColumnSize );
				for (size_t j = 0; j < maxColumnSize; ++j)
				{
					if (j < laStringMatrix[i].size())
					{
						thisRow[j] = laStringMatrix[i][j].getCString();
					}
				}
				result.push_back( thisRow );

			}
			return result;

		#endif
	}

    /* @brief			build a matrix of strings from a AnyTypeMatrix object
	*  @param [in]		anyMatrix	a AnyTypeMatrix object
	*  @output			output a matrix of strings
	*/
	SWIG_STRINGMATRIX fromAnyTypeMatrixToMatrixOfString(const AnyTypeMatrix& anyMatrix )
	{
		// Access Violation Guards
		AQ_REQUIRE(anyMatrix.size() > 0, "Results Matrix is empty")
		AQ_REQUIRE(anyMatrix[0].size() > 0, "Results Matrix is empty")

		// 1) Get Results Dimensions and Max Sizes, since the vectors of variantMatrix may not have the same
		// dimensions. In Step 2 below we pad any spaces with blank strings.
		const bool checkForEmptyMatrix = true;
		const size_t maxColumnSize = getMatrixMaxColumnDimension( anyMatrix, checkForEmptyMatrix );

		// String Matrices Require Special Treatment for R API
		// ---------------------------------------------------
		#if defined(SWIG_R) || defined(SWIGR)

			// R_API treats Matrices as Vectors
			// ----------------------------------			
			SWIG_STRINGMATRIX result;

			// Store the matrix values into the R vector, column by column

			// 2) Populate data, padding spaces with blank strings
			for ( size_t j = 0; j < maxColumnSize; ++j )
			{
				for ( size_t i = 0; i < anyMatrix.size(); ++i )
				{
					if ( j < anyMatrix[i].size() )
					{
						result.push_back( etrading::fromAnyTypeToString( anyMatrix[i][j] ) );
					}
					else
					{
						result.push_back( "" );
					}
				}
			}
			return result;

		#else

			SWIG_STRINGMATRIX result;

			// 2) Populate data, padding spaces with blank strings
			for (size_t i = 0; i < anyMatrix.size(); ++i)
			{
				std::vector<std::string> thisRow( maxColumnSize );
				for (size_t j = 0; j < maxColumnSize; ++j)
				{
					if (j < anyMatrix[i].size())
					{
						thisRow[j] = etrading::fromAnyTypeToString(anyMatrix[i][j] );
					}
				}
				result.push_back( thisRow );
				
			}
			return result;

		#endif

	}


	/* @brief			build a matrix of strings from a vector of AnyTypeMatrix object
	*  @param [in]		vectorOfAnyMatrix	a vector of AnyTypeMatrix object
	*  @output			output a matrix of strings
	*/
	SWIG_STRINGMATRIX fromVectorOfAnyTypeMatrixToMatrixOfString( const std::vector<AnyTypeMatrix>& vectorOfAnyMatrix )
	{
		// 1) Get Results Dimensions and Max Sizes, since the vectors of AnyMatrix may not have the same
		// dimensions. In Step 2 below we pad any spaces with blank strings.
		size_t totalRowSize = 0;
		size_t maxColumnSize = 0;
		const bool checkForEmptyMatrix = false;
		for( size_t i = 0; i < vectorOfAnyMatrix.size(); ++i )
		{
			const AnyTypeMatrix& thisAnyMatrix = vectorOfAnyMatrix[i];
			size_t columnSize = getMatrixMaxColumnDimension( thisAnyMatrix, false );
			if (maxColumnSize < columnSize)
			{
				maxColumnSize = columnSize;
			}
		}

		if( totalRowSize == 0 || maxColumnSize == 0 )
		{
			throw LACoreInvalidData( "#Error: There are no rows or columns to display.", __FILE__, __LINE__ );
		}

		// String Matrices Require Special Treatment for R API
		// ---------------------------------------------------
		#if defined(SWIG_R) || defined(SWIGR)

			// R_API treats Matrices as Vectors. NOTE that R expects the matrix to be enumerated column by column.
			// So for each column, we iterate over each result matrix, and for each matrix we iterate over each row.
			// ----------------------------------			
			SWIG_STRINGMATRIX result;
			
			// 2) Populate data, padding spaces with blank strings
			for (size_t j = 0; j < maxColumnSize; ++j)
			{
				for (size_t k = 0; k < vectorOfAnyMatrix.size(); ++k)
				{
					const AnyTypeMatrix& anyTypeMatrix = vectorOfAnyMatrix[k];

					for (size_t i = 0; i < anyTypeMatrix.size(); ++i)
					{
						if (j < anyTypeMatrix[i].size())
						{
							result.push_back( etrading::fromAnyTypeToString(anyTypeMatrix[i][j] ) );
						}
						else
						{
							result.push_back( "" );
						}
					}
				}
			}
			return result;

		#else
			
			SWIG_STRINGMATRIX result;

			// 2) Populate data, padding spaces with blank strings
			for( size_t k = 0; k < vectorOfAnyMatrix.size(); ++k )
			{
				const AnyTypeMatrix& anyTypeMatrix = vectorOfAnyMatrix[k];
				for( size_t i = 0; i < anyTypeMatrix.size(); ++i )
				{
					std::vector<std::string> thisRow( maxColumnSize );
					for( size_t j = 0; j < maxColumnSize; ++j )
					{
						if( j < anyTypeMatrix[i].size() )
						{
							thisRow[j] = etrading::fromAnyTypeToString( anyTypeMatrix[i][j] );
						}
					}
					result.push_back( thisRow );
				}
			}
			return result;

		#endif
	}


    /* @param [in]		variantMatrix	a VariantMatrix object
	*  @output			output a matrix of strings
	*/
	SWIG_STRINGMATRIX fromVariantMatrixToMatrixOfString( const etrading::VariantMatrix& variantMatrix )
    {
		// Access Violation Guards
		AQ_REQUIRE(variantMatrix.size() > 0, "Results Matrix is empty")
		AQ_REQUIRE(variantMatrix[0].size() > 0, "Results Matrix is empty")

		// 1) Get Results Dimensions and Max Sizes, since the vectors of variantMatrix may not have the same
		// dimensions. In Step 2 below we pad any spaces with blank strings.
		const bool checkForEmptyMatrix = true;
		const size_t maxColumnSize = getMatrixMaxColumnDimension( variantMatrix, checkForEmptyMatrix );

		// String Matrices Require Special Treatment for R API
		// ---------------------------------------------------
		#if defined(SWIG_R) || defined(SWIGR)
			
			// R_API treats Matrices as Vectors
			// ----------------------------------		
			SWIG_STRINGMATRIX result;

			// Store the matrix values into the R vector, column by column

			// 2) Populate data, padding spaces with blank strings
			for (size_t j = 0; j < maxColumnSize; ++j)
			{
				for (size_t i = 0; i < variantMatrix.size(); ++i)
				{
					if (j < variantMatrix[i].size())
					{
						result.push_back( variantMatrix[i][j].toString() );
					}
					else
					{
						result.push_back( "" );
					}
				}
			}
			return result;

		#else

			SWIG_STRINGMATRIX result( variantMatrix.size() );
			for (size_t i=0; i<variantMatrix.size(); ++i)
			{
				std::vector<std::string> tempVec( maxColumnSize );
				for (size_t j=0; j< maxColumnSize; ++j)
				{
					if (j < variantMatrix[i].size())
					{
						tempVec[j] = variantMatrix[i][j].toString();
					}
				}
				result[i] = tempVec;
			}
			return result;

		#endif
    }


	/* @brief	Build a LabelValueBlock from a vector of string vectors
	*  @param [in]		input	a vector of string vectors
    *  @return		One LabelValueBlock object
    */
	LabelValueBlock buildSingleLabelValueBlock(const std::vector<std::vector <std::string> >& input)
	{

		LAStringMatrix temp;
		buildStringMatrix(temp, input);
		if (temp.size() == 0)
		{
			throw LACoreInvalidData("#Error: an empty Label Value Block object is given", __FILE__, __LINE__ );
		}

		LabelValueBlock a(temp);
		return a;
	}


     /* @brief			build Variant Matrix from a vector of string vectors
	*  @param [out]		toMatrix			the variant matrix object being built
	*  @param [in]		fromMatrix  	    the string matrix input
	*/
	void buildVariantMatrix( etrading::VariantMatrix & toMatrix, const std::vector<std::vector<std::string> >& fromMatrix )
    {
        toMatrix.clear();

		for( size_t i = 0; i < fromMatrix.size(); ++i )
		{
			etrading::VariantVector tempVariantVector;
            std::vector<std::string> tempStringVector = fromMatrix[i];

			for( size_t j = 0; j < tempStringVector.size(); ++j )
			{
				tempVariantVector.push_back( etrading::Variant( tempStringVector[j].c_str() ) );
			}

			toMatrix.push_back( tempVariantVector );
		}
    }
	/* @brief			build Variant Matrix from an LAStringMatrix
	*  @param [out]		toMatrix			the variant matrix object being built
	*  @param [in]		fromMatrix  	    the string matrix input
	*/
	void buildVariantMatrix( etrading::VariantMatrix & toMatrix, const LAStringMatrix& fromMatrix )
	{
	    toMatrix.clear();

		for( size_t i = 0; i < fromMatrix.size(); ++i )
		{
			etrading::VariantVector tempVariantVector;
            LAStringVector tempStringVector = fromMatrix[i];

			for( size_t j = 0; j < tempStringVector.size(); ++j )
			{
				tempVariantVector.push_back( etrading::Variant( tempStringVector[j].c_str() ) );
			}

			toMatrix.push_back( tempVariantVector );
		}
	}

    /* @brief			build Variant Matrix from a vector of string vectors
	*  @param [out]		toVector			the variant vector object being built
	*  @param [in]		fromVector  		the string vector input
	*/
	void buildVariantVector( etrading::VariantVector & toVector, const std::vector<std::string>& fromVector )
    {
        toVector.clear();
		for( size_t i = 0; i < fromVector.size(); ++i )
		{
			toVector.push_back( etrading::Variant( fromVector[i].c_str() ) );
		}
    }


}
