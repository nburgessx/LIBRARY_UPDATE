#pragma once

#include "SwigTypes.h"
#include "AQLCoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Variant.h"
#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the AQLStringMatrix idiosyncrasies type for R API
#include "CoreEnumerations.h"

using etrading::LabelValueBlock;

class AQLString;

namespace swig
{

	/* @brief		build AQLStringMatrix from a vector of string vectors
	*  @param [out]		sMatrix					the AQLStringMatrix object being built
	*  @param [in]		inputData				A vector of string vectors
	*  @param [in]		curveTypeEnum			The curveType which thw data block "rhs" comes from.
	*  @param [in]		curveMarketDataEnum		The name of the curve data block contained in inputData.
	*/
	void buildStringMatrix( AQLStringMatrix& sMatrix, const std::vector<std::vector<std::string> >& inputData,	
							const etrading::CurveTypeEnum& curveTypeEnum = etrading::NONE_CURVETYPE,
							const etrading::CurveMarketDataEnum& curveMarketDataEnum = etrading::NONE_MARKETDATA );

    /* @brief			Build AQLStringMatrix from input type SWIG_STRINGMATRIX.
	*                   In "R", the SWIG_STRINGMATRIX is defined as vector< string >.
	*					In all other languages, SWIG_STRINGMATRIX is a vector of string vectors.
	*					The curveTypeEnum and curveMarketDataEnum is used to figure out how many columns there should be in the final matrix.
	*
	*  @param [out]		sMatrix					The string matrix object being built
	*  @param [in]		inputData				An R block of data, represented as a vector of string
	*  @param [in]		curveTypeEnum			The curveType which thw data block "rhs" comes from.
	*  @param [in]		curveMarketDataEnum		The name of the curve data block contained in inputData.
	*/
#if defined(SWIG_R) || defined(SWIGR)
	void buildStringMatrix( AQLStringMatrix& sMatrix, const SWIG_STRINGMATRIX & inputData,
							const etrading::CurveTypeEnum& curveTypeEnum = etrading::NONE_CURVETYPE,
							const etrading::CurveMarketDataEnum& curveMarketDataEnum = etrading::NONE_MARKETDATA );
#endif

	/* @brief			build DateVector from a vector of strings 
	*  @param [out]		toDateVector			Result Vector of Dates
	*  @param [in]		fromStringVector		A vector of strings
	*/
	void buildDateVector(DateVector& toDateVector, const std::vector<std::string>& fromStringVector );

	/* @brief			build a vector of strings from a DateVector
	*  @param [out]		toStringVector			Result vector of date strings (YYYYMMDD)
	*  @param [in]		fromDateVector			A DateVector
	*/
	void buildStringVectorFromDateVector( StandardStringVector& toStringVector, const DateVector& fromDateVector );

    /* @brief			build boost::gregorian::date Vector from a vector of strings 
	*  @param [out]		toDateVector			Result Vector of Dates
	*  @param [in]		fromStringVector		A vector of strings
	*/
	void buildGregorianDateVector( std::vector< boost::gregorian::date > & toDateVector, const std::vector<std::string>& fromStringVector );

	/* @brief			build AQLStringVector from a vector of strings 
	*  @param [out]		sVector			A AQLStringVector object
	*  @param [in]		rhs				a vector of strings
	*/
	void buildStringVector(AQLStringVector& sVector, const std::vector<std::string>& rhs);

    /* @brief			Build a AQLDate from a std::string type
	*  @param [in]		dateString		String Date
    *  @param [in]		dateFormat		Date Format, typically "YYYYMMDD"
	*  @output			Date in AQLDate format
	*/
    AQLDate fromStringToAQLDate(const std::string& dateString, const std::string& dateFormat = "YYYYMMDD" );

	/* @brief			build a vector of strings from a DoubleVector object
	*  @param [in]		dVector			A DoubleVector object
	*  @output			date with expected format
	*/
	AQLString fromStringToAQLString(const std::string& inVal);

	/* @brief			Check each row of the input matrix and keep a record of the maximum column size. This is designed to work with ragged matrices.
	*  @param [in]		inputMatrix	The matrix of type T to check.
	*  @param [in]		checkForEmptyMatrix	Specify whether to throw a message if the number of rows or number of columns is zero.
	*  @output			The maximum column size
	*/
	template<typename T>
	size_t getMatrixMaxColumnDimension(const std::vector< std::vector< T > >& inputMatrix, const bool checkForEmptyMatrix);

	/* @brief			build a matrix of strings from a AQLStringMatrix object
	*  @param [in]		AQLStringMatrix	a AQLStringMatrix object
	*  @output			output a matrix of strings
	*/
	SWIG_STRINGMATRIX fromStringMatrixToMatrixOfString( const AQLStringMatrix& AQLStringMatrix );

    /* @brief			build a matrix of strings from a AnyTypeMatrix object
	*  @param [in]		anyMatrix	a AnyTypeMatrix object
	*  @output			output a matrix of strings
	*/
	SWIG_STRINGMATRIX fromAnyTypeMatrixToMatrixOfString( const AnyTypeMatrix& anyMatrix );

	/* @brief			build a matrix of strings from a vector of AnyTypeMatrix object
	*  @param [in]		vectorOfAnyMatrix	a vector of AnyTypeMatrix object
	*  @output			output a matrix of strings
	*/
	SWIG_STRINGMATRIX fromVectorOfAnyTypeMatrixToMatrixOfString( const std::vector<AnyTypeMatrix>& vectorOfAnyMatrix );

    /* @param [in]		variantMatrix	a VariantMatrix object
	*  @output			output a matrix of strings
	*/
	SWIG_STRINGMATRIX fromVariantMatrixToMatrixOfString( const etrading::VariantMatrix& variantMatrix );


	/* @brief	Template method that converts a number to a string
	*  @param [in]	number		Input of a certain numeric data type
	*  @return		String output
	*/
	template <typename T>
	AQLString NumberToString ( T number )
	{
		std::stringstream ss;
		ss << number;
		return AQLString(ss.str().c_str());
	}

	/* @brief	Template method that converts a string to a number
	*  @param [in]	text	String to be converted
	*  @return		Output of a certain numeric data type
	*/
	template <typename T>
	T StringToNumber ( const AQLString &text )
	{
		std::stringstream ss(text.getCString());
		T result;
		return ss >> result ? result : 0;
	}

	/* @brief	Build a LabelValueBlock from a vector of string vectors
	*  @param [in]		input	a vector of string vectors
    *  @return		One LabelValueBlock object
    */
	LabelValueBlock buildSingleLabelValueBlock(const std::vector<std::vector <std::string> >& input);

	/* @brief			Build a LabelValueBlock from R's flat SWIG_STRINGMATRIX. A LabelValueBlock is always
	*                   2 columns (label, value) by contract, so the column count needed to unflatten it
	*                   is fixed rather than looked up.
	*  @param [in]		input	R's flattened matrix representation, column by column
    *  @return		One LabelValueBlock object
    */
#if defined(SWIG_R) || defined(SWIGR)
	LabelValueBlock buildSingleLabelValueBlock(const SWIG_STRINGMATRIX& input);
#endif

    /* @brief			build Variant Matrix from a vector of string vectors
	*  @param [out]		toMatrix			the variant matrix object being built
	*  @param [in]		fromMatrix  	    the string matrix input
	*/
	void buildVariantMatrix( etrading::VariantMatrix & toMatrix, const std::vector<std::vector<std::string> >& fromMatrix );

	/* @brief			build Variant Matrix from an AQLStringMatrix
	*  @param [out]		toMatrix			the variant matrix object being built
	*  @param [in]		fromMatrix  	    the string matrix input
	*/
	void buildVariantMatrix( etrading::VariantMatrix & toMatrix, const AQLStringMatrix& fromMatrix );

    /* @brief			build Variant Vector from a string vector
	*  @param [out]		toVector			the variant vector object being built
	*  @param [in]		fromVector  		the string vector input
	*/
	void buildVariantVector( etrading::VariantVector & toVector, const std::vector<std::string>& fromVector );
}
