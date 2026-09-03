/*
 * @brief			validation helper methods used within validation_api
 * @Created:		17 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include <boost/lexical_cast.hpp>
#include "LabelValueBlock.h"
#include "Variant.h"
#include <boost/date_time/gregorian/gregorian.hpp>

using etrading::LabelValueBlock;

namespace etrading
{

    /* @brief			Validate if the keys from the input are valid
    *  @param [in]		expectedKeys	A list of expected keys
    *  @param [in]		keys			A list of keys from user input
    *  @param [in]		validateKeys	True to do the verification
    *  @param [in]		LVBname     	Optional name of the LVB
    */
    void validateKeysForLVB( const std::vector<std::string>& expectedKeys, const LAStringVector& keys, bool validateKeys, const std::string& LVBname = "" );

    /* @brief		Validate if the keys from the input are valid
    *  @param [in]		expectedKeys	A list of expected keys
    *  @param [in]		keys			A list of keys from user input
    *  @param [in]		validateKeys	True to do the verification
    *  @param [in]		LVBname     	Optional name of the LVB
    */
    void validateKeysForLVB( const std::vector<std::string>& expectedKeys, const std::vector<std::string>& keys, bool validateKeys, const std::string& LVBname = "" );

    /* @brief			Return a LabelValueBlock representing the spec
    *  @param [in]		keys			A list of keys from user input
    *  @param [in]		values			A list of values from user input
    *  @param [in]		keyPrefix		Prefix to the keys
    *  @return			LabelValueBlock
    */
    LabelValueBlock populateLabelValueBlock( const LAStringVector& keys, const LAStringVector& values, const LAString& keyPrefix = LAString() );

    /* @brief			return a set of expected keys for swap common label value block
    *  @return			expected keys
    */
    std::vector<std::string>  getSwapCommonLVBKeys();

    /* @brief			build a vector of strings from LAStringVector object
    *  @param [out]		sVector			A LAStringVector object
    *  @output			output a vector of strings
    */
    std::vector<std::string> fromStringVectorToStdVector( const LAStringVector& sVector );

    /* @brief			build a vector of strings from LAStringVector object
    *  @param [out]		inVal			A LAStringVector object
    *  @output			output a vector of strings
    */
    LAStringVector fromStdVectorToStringVector( const std::vector<std::string>& inVal );


    /* @brief			build a matrix of strings from a LAStringMatrix object
    *  @param [in]		inVal	a LAStringMatrix object
    *  @output			output a matrix of strings
    */
    std::vector<std::vector<std::string>> fromStringMatrixToStdMatrix( const LAStringMatrix& inVal );

    /* @brief			build a matrix of strings from a LAStringMatrix object
    *  @param [in]		inVal	a LAStringMatrix object
    *  @output			output a matrix of strings
    */
    LAStringMatrix fromStdMatrixToStringMatrix( const std::vector<std::vector<std::string>>& inVal );

    /* @brief			build a LAStringMatrix object from LabelValueBlock object
    *  @param [in]		lvb	a LabelValueBlock object, where the first columns are keys and the second columns are values
    *  @output			LAStringMatrix object
    */
    LAStringMatrix fromLabelValueBlockToStringMatrix( const LabelValueBlock& lvb );

    /* @brief			Appends the rhs string matrix to the lhs Matrix
    *  @param [in]		rhs	a LAStringMatrix object
    *  @param [out]		lhs	a LAStringMatrix object to be assigned values from rhs
    */
    void appendToMatrix( LAStringMatrix& lhs, const LAStringMatrix& rhs );

    /* @brief			check if a LAStringMatrix a label value block
    *  @param [in]		lvb	a LAStringMatrix object
    */
    void validateLVBStringMatrix( const LAStringMatrix& lvb );

    /* @brief			check if the date is the last business day of the month
    *  @param [in]		date Given date
    *  @param [in]		cal Calendar
    */
    bool isLastDayOfMonth( const LADate& date, const LAString& cal );

	/* @brief			remove prefix of the keys of label value block
    *  @param [in]		lvb				Label value block
    *  @param [in]		keyPrefix		Prefix to remove
    *  @param [in]		unchangedKeys	Keys to keep unchanged
    *  @param [out]		label value block with keys without prefix
    */
	LabelValueBlock removeKeyPrefix( const LabelValueBlock& inLvb, const std::string& keyPrefix, const std::vector<std::string> unchangedKeys = std::vector<std::string>());

    /* @brief			build a standard string vector from a Variant vector
    *  @param [in]		inVal	a Variant vector
    *  @output			output a standard string vector
    */
    std::vector<std::string> fromVariantToStdStringVector( const std::vector<Variant>& inVal );

    LabelValueBlock fromStringToLVB(const std::string& value);

    /* @brief			Merge the header matrix and body matrix pair into one matrix
    *  @param [in]		headerBodyPair	the header matrix and body matrix pair
    *  @output			merged matrix
    */
    AnyTypeMatrix mergeHeaderAndBodyPair(const std::pair<AnyTypeMatrix, AnyTypeMatrix>& headerBodyPair);

    /* @brief			Merge a vector of header and body pairs into a vector of merged matrices
    *  @param [in]		headerBodyPairs	the header and body matrix pairs
    *  @output			a vector of merged matrices
    */
    std::vector<AnyTypeMatrix> mergeHeaderAndBodyPairs(const std::vector<std::pair<AnyTypeMatrix, AnyTypeMatrix>>& headerBodyPairs);

 
}