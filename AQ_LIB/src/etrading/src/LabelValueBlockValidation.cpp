#include "LabelValueBlockValidation.h"

#include "AQLDateSchedule.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLPriceDataCalendar.h"
#include <cctype>
#include "AQLPriceDataInterpolation.h"
#include "CommonConstants.h"
#include <utility>
#include "AQLStaticData.h"
#include <boost/algorithm/string.hpp>

namespace etrading
{

    /* @brief			Validate if the keys from the input are valid
    *  @param [in]		expectedKeys	A list of expected keys
    *  @param [in]		keys			A list of keys from user input
    *  @param [in]		validateKeys	True to do the verification
    *  @param [in]		LVBname     	Optional name of the LVB
    */
    void validateKeysForLVB( const std::vector<std::string>& expectedKeys, const AQLStringVector& keys, bool validateKeys, const std::string& LVBname )
    {
        if( validateKeys && keys.size() != 0 )
        {
            validateKeysForLVB( expectedKeys, fromStringVectorToStdVector( keys ), validateKeys, LVBname );
        }
    }


    /* @brief			Validate if the keys from the input are valid. 
	*  @param [in]		expectedKeys	A list of expected keys
    *  @param [in]		keys			A list of keys from user input
    *  @param [in]		validateKeys	True to do the verification
    *  @param [in]		LVBname     	Optional name of the LVB
    */
    void validateKeysForLVB( const std::vector<std::string>& expectedKeys, const std::vector<std::string>& keys, bool validateKeys, const std::string& LVBname )
    {
        if( validateKeys && keys.size() != 0 )
        {
            std::ostringstream  ss;
            std::string key;
            for ( size_t i = 0; i < keys.size(); ++i )
            {
                key = boost::to_upper_copy(keys[i]);
				// *** Note: Since the expectedKeys can have mandatory or optional keys, so we cannot enforce all the expectedKeys are specified by the user. 
				// The Only way we can verify is to check all the keys from user are within the expected key set
				if ( std::find( expectedKeys.begin(), expectedKeys.end(), key ) == expectedKeys.end() )
                {
                    if( LVBname == "" )
                    {
                        ss << "#Error: Key " << key << " is not an expected key.";
                    }
                    else
                    {
                        ss << "#Error: Key " << key << " is not an expected key in the " << LVBname << " Label Value Block";
                    }
                    AQ_THROW( ss.str().c_str() );
                }
            }
        }
    }

    /* @brief			Return a LabelValueBlock representing the spec
    *  @param [in]		keys			A list of keys from user input
    *  @param [in]		values			A list of values from user input
    *  @param [in]		keyPrefix		Prefix to the keys
    *  @return			LabelValueBlock
    */
    LabelValueBlock populateLabelValueBlock( const AQLStringVector& keys, const AQLStringVector& values, const AQLString& keyPrefix )
    {

        AQ_THROW_IF( keys.size() != values.size(), "Keys and Values are not the same size." );

        AQLStringVector decoratedKeys( keys.size() );

        for ( size_t i = 0; i < keys.size(); ++i )
        {
            if ( keyPrefix != AQLString() )
            {
                decoratedKeys[i] = keyPrefix + keys[i];
            }
            else
            {
                decoratedKeys[i] = keys[i];

            }
        }
        LabelValueBlock lvb( decoratedKeys, values );
        return lvb;
    }

    /* @brief			return a set of expected keys for swap common label value block
                        These keys are not mandatory, however keys not in this list will be rejected and result in error messages

    *  @return			expected keys
    */
    std::vector<std::string> getSwapCommonLVBKeys()
    {
        const std::string arr[] =
        {
            etrading::IRS_KEY::EFFECTIVE_DATE
            , etrading::IRS_KEY::MATURITY_DATE
            , etrading::IRS_KEY::FIXED_FREQUENCY
            , etrading::IRS_KEY::FIXED_DAYCOUNT
            , etrading::IRS_KEY::FIXED_BUSINESSDAYADJUSTMENT
            , etrading::IRS_KEY::FIXED_CALENDAR
            , etrading::IRS_KEY::FIXED_ACCRUALBUSINESSDAYADJUSTMENT
            , etrading::IRS_KEY::FIXED_ACCRUALCALENDAR
            , etrading::IRS_KEY::FIXED_PAYMENTBUSINESSDAYADJUSTMENT
            , etrading::IRS_KEY::FIXED_PAYMENTCALENDAR
            , etrading::IRS_KEY::FIXED_FIRSTSTUBDATE
            , etrading::IRS_KEY::FIXED_LASTSTUBDATE
            , etrading::IRS_KEY::FIXED_ROLLDAY
            , etrading::IRS_KEY::FIXED_PAYMENTLAG
            , etrading::IRS_KEY::FIXED_STUBTYPE
            , etrading::IRS_KEY::FLOAT_FREQUENCY
            , etrading::IRS_KEY::FLOAT_DAYCOUNT
            , etrading::IRS_KEY::FLOAT_BUSINESSDAYADJUSTMENT
            , etrading::IRS_KEY::FLOAT_CALENDAR
            , etrading::IRS_KEY::FLOAT_FIXINGBUSINESSDAYADJUSTMENT
            , etrading::IRS_KEY::FLOAT_FIXINGCALENDAR
            , etrading::IRS_KEY::FLOAT_ACCRUALBUSINESSDAYADJUSTMENT
            , etrading::IRS_KEY::FLOAT_ACCRUALCALENDAR
            , etrading::IRS_KEY::FLOAT_PAYMENTBUSINESSDAYADJUSTMENT
            , etrading::IRS_KEY::FLOAT_PAYMENTCALENDAR
            , etrading::IRS_KEY::FLOAT_FIRSTSTUBDATE
            , etrading::IRS_KEY::FLOAT_LASTSTUBDATE
            , etrading::IRS_KEY::FLOAT_ROLLDAY
            , etrading::IRS_KEY::FLOAT_FIXINGLAG
            , etrading::IRS_KEY::FLOAT_FIRSTFIXING
            , etrading::IRS_KEY::FLOAT_LASTFIXING
            , etrading::IRS_KEY::FLOAT_PAYMENTLAG
            , etrading::IRS_KEY::FLOAT_STUBTYPE
        };

        std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );
        return expectedKeys;
    }


    /* @brief			build a vector of strings from AQLStringVector object
    *  @param [out]		sVector			A AQLStringVector object
    *  @output			output a vector of strings
    */
    std::vector<std::string> fromStringVectorToStdVector( const AQLStringVector& sVector )
    {
        std::vector<std::string> ret;
        for ( size_t i = 0; i < sVector.size(); ++i )
        {
            ret.push_back( sVector[i].getCString() );
        }
        return ret;
    }

    /* @brief			build a vector of strings from AQLStringVector object
    *  @param [out]		inVal			A AQLStringVector object
    *  @output			output a vector of strings
    */
    AQLStringVector fromStdVectorToStringVector( const std::vector<std::string>& inVal )
    {
        AQLStringVector ret;
        for ( size_t i = 0; i < inVal.size(); ++i )
        {
            ret.push_back( AQLString( inVal[i].c_str() ) );
        }
        return ret;
    }


    /* @brief			build a matrix of strings from a AQLStringMatrix object
    *  @param [in]		inVal	a AQLStringMatrix object
    *  @output			output a matrix of strings
    */
    std::vector<std::vector<std::string>> fromStringMatrixToStdMatrix( const AQLStringMatrix& inVal )
    {
        std::vector<std::vector<std::string> > ret;
        for ( size_t i = 0; i < inVal.size(); ++i )
        {
            std::vector<std::string> tempVec;
            for ( size_t j = 0; j < inVal[i].size(); ++j )
            {
                tempVec.push_back( inVal[i][j].getCString() );
            }
            ret.push_back( tempVec );
        }
        return ret;
    }

    /* @brief			build a matrix of strings from a AQLStringMatrix object
    *  @param [in]		inVal	a AQLStringMatrix object
    *  @output			output a matrix of strings
    */
    AQLStringMatrix fromStdMatrixToStringMatrix( const std::vector<std::vector<std::string>>& inVal )
    {
        AQLStringMatrix ret;
        for ( size_t i = 0; i < inVal.size(); ++i )
        {
            AQLStringVector tempVec;
            for ( size_t j = 0; j < inVal[i].size(); ++j )
            {
                tempVec.push_back( AQLString( inVal[i][j].c_str() ) );
            }
            ret.push_back( tempVec );
        }
        return ret;
    }

    /* @brief			build a AQLStringMatrix object from LabelValueBlock object
    *  @param [in]		lvb	a LabelValueBlock object, where the first columns are keys and the second columns are values
    *  @output			AQLStringMatrix object
    */
    AQLStringMatrix fromLabelValueBlockToStringMatrix( const LabelValueBlock& lvb )
    {
        AQLStringMatrix ret;
        size_t rowSize = lvb.size();
        std::vector<std::string> keys = lvb.getKeys();
        std::vector<std::string> values = lvb.getValues();

        AQLStringVector tempVec;
        for( size_t i = 0; i < rowSize; i++ )
        {
            tempVec.clear();
            tempVec.push_back( keys[i].c_str() );
            tempVec.push_back( values[i].c_str() );
            ret.push_back( tempVec );
        }
        return ret;
    }

    /* @brief			check if a AQLStringMatrix a label value block
    *  @param [in]		lvb	a AQLStringMatrix object
    */
    void validateLVBStringMatrix( const AQLStringMatrix& lvb )
    {
        AQ_THROW_IF( lvb.size() != 0 && lvb[0].size() != 2, "the Matrix is not a valid label value block." );
    }

    /* @brief			Appends the rhs string matrix to the lhs Matrix 
    *  @param [in]		rhs	a AQLStringMatrix object
    *  @param [out]		lhs	a AQLStringMatrix object to be assigned values from rhs
    */
    void appendToMatrix( AQLStringMatrix& lhs, const AQLStringMatrix& rhs )
    {
        for ( size_t i = 0; i < rhs.size(); ++i )
        {
            AQLStringVector tempVec;
            for ( size_t j = 0; j < rhs[i].size(); ++j )
            {
                tempVec.push_back( rhs[i][j] );
            }
            lhs.push_back( tempVec );
        }
    }

	/* @brief			remove prefix of the keys of label value block
    *  @param [in]		lvb				Label value block
    *  @param [in]		keyPrefix		Prefix to remove
    *  @param [in]		unchangedKeys	Keys to keep unchanged
    *  @param [out]		label value block with keys without prefix
    */
	LabelValueBlock removeKeyPrefix( const LabelValueBlock& inLvb, const std::string& keyPrefix, const std::vector<std::string> unchangedKeys)
    {
        auto keys = inLvb.getKeys();
		auto values = inLvb.getValues();
        
        StandardStringVector newKeys( keys.size() );
        for ( size_t i = 0; i < keys.size(); ++i )
        {
			newKeys[i] = keys[i];
			bool isUnchangedKey = (std::find(unchangedKeys.begin(), unchangedKeys.end(), newKeys[i]) != unchangedKeys.end());
            if (!isUnchangedKey && keyPrefix.size() != 0 && newKeys[i].find(keyPrefix) != std::string::npos )
            {
                newKeys[i].erase(0, keyPrefix.size());
		    }
        }

        LabelValueBlock outLvb( newKeys, values );
        return outLvb;
    }

    /* @brief			build a standard string vector from a Variant vector
    *  @param [in]		inVal	a Variant vector
    *  @output			output a standard string vector
    */
    std::vector<std::string> fromVariantToStdStringVector( const std::vector<Variant>& inVal )
    {
        std::vector<std::string> ret;
        for ( size_t i = 0; i < inVal.size(); ++i )
        {
            ret.push_back( inVal[i] );
        }
        return ret;
    }

    LabelValueBlock fromStringToLVB(const std::string& value)
    {
        if (value.size() != 0)
        {
            return LabelValueBlock(value, std::string("") );
        } 

        return LabelValueBlock();
    }

    /* @brief			Merge the header matrix and body matrix pair into one matrix
    *  @param [in]		headerBodyPair	the header matrix and body matrix pair
    *  @output			merged matrix
    */
    AnyTypeMatrix mergeHeaderAndBodyPair(const std::pair<AnyTypeMatrix, AnyTypeMatrix>& headerBodyPair)
     {
        auto ret = headerBodyPair.first;
        auto body = headerBodyPair.second;
        ret.insert(ret.end(), body.begin(), body.end());

        return ret;
     }

    /* @brief			Merge a vector of header and body pairs into a vector of merged matrices
    *  @param [in]		headerBodyPairs	the header and body matrix pairs
    *  @output			a vector of merged matrices
    */
     std::vector<AnyTypeMatrix> mergeHeaderAndBodyPairs(const std::vector<std::pair<AnyTypeMatrix, AnyTypeMatrix>>& headerBodyPairs)
     {
        size_t pairSize = headerBodyPairs.size();

        std::vector<AnyTypeMatrix> ret;
        ret.reserve(pairSize);

        for (size_t i = 0; i < pairSize; ++i)
        {
            auto mergedPair =  mergeHeaderAndBodyPair(headerBodyPairs[i]);
            ret.push_back(mergedPair);
        }
		
        return ret;
     }

}