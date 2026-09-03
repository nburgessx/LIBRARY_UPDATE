/* 
 * @brief			Collection of utility methods to do with data types
 * @Created:		14 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "SwigTypes.h"
#include "LACoreTemplateType.h"
#include "LabelValueBlock.h"
#include "Variant.h"
#include "SwigTypes.h"	// For SWIG_STRINGMATRIX which manages the LAStringMatrix idiosyncrasies type for R API
#include "CoreEnumerations.h"

using etrading::LabelValueBlock;

class LAString;

namespace swig
{

	/* @brief		build LAStringMatrix from a vector of string vectors
	*  @param [out]		sMatrix					the LAStringMatrix object being built
	*  @param [in]		inputData				A vector of string vectors
	*  @param [in]		curveTypeEnum			The curveType which thw data block "rhs" comes from.
	*  @param [in]		curveMarketDataEnum		The name of the curve data block contained in inputData.
	*/
	void buildStringMatrix( LAStringMatrix& sMatrix, const std::vector<std::vector<std::string> >& inputData,	
							const etrading::CurveTypeEnum& curveTypeEnum = etrading::NONE_CURVETYPE,
							const etrading::CurveMarketDataEnum& curveMarketDataEnum = etrading::NONE_MARKETDATA );

    /* @brief			Build LAStringMatrix from input type SWIG_STRINGMATRIX.
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
	void buildStringMatrix( LAStringMatrix& sMatrix, const SWIG_STRINGMATRIX & inputData,
							const etrading::CurveTypeEnum& curveTypeEnum = etrading::NONE_CURVETYPE,
							const etrading::CurveMarketDataEnum& curveMarketDataEnum = etrading::NONE_MARKETDATA );
#endif

	/* @brief			build DateVector from a vector of strings 
	*  @param [out]		toDateVector			Result Vector of Dates
	*  @param [in]		fromStringVector		A vector of strings
	*/
	void buildDateVector(DateVector& toDateVector, const std::vector<std::string>& fromStringVector );

    /* @brief			build boost::gregorian::date Vector from a vector of strings 
	*  @param [out]		toDateVector			Result Vector of Dates
	*  @param [in]		fromStringVector		A vector of strings
	*/
	void buildGregorianDateVector( std::vector< boost::gregorian::date > & toDateVector, const std::vector<std::string>& fromStringVector );

	/* @brief			build LAStringVector from a vector of strings 
	*  @param [out]		sVector			A LAStringVector object
	*  @param [in]		rhs				a vector of strings
	*/
	void buildStringVector(LAStringVector& sVector, const std::vector<std::string>& rhs);

    /* @brief			Build a LADate from a std::string type
	*  @param [in]		dateString		String Date
    *  @param [in]		dateFormat		Date Format, typically "YYYYMMDD"
	*  @output			Date in LADate format
	*/
    LADate fromStringToLADate(const std::string& dateString, const std::string& dateFormat = "YYYYMMDD" );

	/* @brief			build a vector of strings from a DoubleVector object
	*  @param [in]		dVector			A DoubleVector object
	*  @output			date with expected format
	*/
	LAString fromStringToLAString(const std::string& inVal);

	/* @brief			Check each row of the input matrix and keep a record of the maximum column size. This is designed to work with ragged matrices.
	*  @param [in]		inputMatrix	The matrix of type T to check.
	*  @param [in]		checkForEmptyMatrix	Specify whether to throw a message if the number of rows or number of columns is zero.
	*  @output			The maximum column size
	*/
	template<typename T>
	size_t getMatrixMaxColumnDimension(const std::vector< std::vector< T > >& inputMatrix, const bool checkForEmptyMatrix);

	/* @brief			build a matrix of strings from a LAStringMatrix object
	*  @param [in]		LAStringMatrix	a LAStringMatrix object
	*  @output			output a matrix of strings
	*/
	SWIG_STRINGMATRIX fromStringMatrixToMatrixOfString( const LAStringMatrix& LAStringMatrix );

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
	LAString NumberToString ( T number )
	{
		std::stringstream ss;
		ss << number;
		return LAString(ss.str().c_str());
	}

	/* @brief	Template method that converts a string to a number
	*  @param [in]	text	String to be converted
	*  @return		Output of a certain numeric data type
	*/
	template <typename T>
	T StringToNumber ( const LAString &text )
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

    /* @brief			build Variant Matrix from a vector of string vectors
	*  @param [out]		toMatrix			the variant matrix object being built
	*  @param [in]		fromMatrix  	    the string matrix input
	*/
	void buildVariantMatrix( etrading::VariantMatrix & toMatrix, const std::vector<std::vector<std::string> >& fromMatrix );

	/* @brief			build Variant Matrix from an LAStringMatrix
	*  @param [out]		toMatrix			the variant matrix object being built
	*  @param [in]		fromMatrix  	    the string matrix input
	*/
	void buildVariantMatrix( etrading::VariantMatrix & toMatrix, const LAStringMatrix& fromMatrix );

    /* @brief			build Variant Vector from a string vector
	*  @param [out]		toVector			the variant vector object being built
	*  @param [in]		fromVector  		the string vector input
	*/
	void buildVariantVector( etrading::VariantVector & toVector, const std::vector<std::string>& fromVector );
}
