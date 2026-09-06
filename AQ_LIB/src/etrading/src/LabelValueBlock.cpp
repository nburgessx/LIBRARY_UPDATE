#include "LabelValueBlock.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "TypeHelpers.h"
#include "DataUtilities.h"              // For AQ_TO_STRING macros
#include <boost/algorithm/string.hpp>   // For boost::to_upper and boost:iequals i.e. case insensitve string comparison

namespace etrading
{
    /* @brief	Constructor
    *			Constructing LabelValueBlock from a AQLStringMatrix object
    */
    LabelValueBlock::LabelValueBlock( const AQLStringMatrix& input, const bool& makeAllKeysUppercase )
    {
        // Call Native Constructor/Initializer
        initializefromAQLStringMatrix( input, makeAllKeysUppercase );
    }

    LabelValueBlock::LabelValueBlock( const StandardStringMatrix& input, const bool& makeAllKeysUppercase )
    {
        // Call Native Constructor/Initializer
        initializefromStandardStringMatrix( input, makeAllKeysUppercase );
    }

    LabelValueBlock::LabelValueBlock( const AQLString & key, const AQLString & value, const bool& makeAllKeysUppercase )
    {
        // Build Vector Size 2
        AQLStringVector stringVector(2);
        stringVector[0] = key;
        stringVector[1] = value;

        // Build Matrix Size 1
        AQLStringMatrix stringMatrix(1);
        stringMatrix[0] = stringVector;

        // Call Native Constructor/Initializer
        initializefromAQLStringMatrix( stringMatrix, makeAllKeysUppercase );
    }
    
    LabelValueBlock::LabelValueBlock( const StandardString & key, const StandardString & value, const bool& makeAllKeysUppercase )
    {
        // Build Vector Size 2
        StandardStringVector stringVector(2);
        stringVector[0] = key;
        stringVector[1] = value;

        // Build Matrix Size 1
        StandardStringMatrix stringMatrix(1);
        stringMatrix[0] = stringVector;

        // Call Native Constructor/Initializer
        AQLStringMatrix aqStringMatrix  = convertToAQLStringMatrix( stringMatrix );
        initializefromAQLStringMatrix( aqStringMatrix, makeAllKeysUppercase );
    }

    LabelValueBlock::LabelValueBlock( const AQLStringVector & keys, const AQLStringVector & values, const bool& makeAllKeysUppercase )
    {
        AQ_REQUIRE( keys.size() == values.size(),
            "Unable to create LVB: Inconsistent number of keys and values. There are "
            + AQ_TO_STRING_FROM_SIZE_T( keys.size() ) + " keys and "
            + AQ_TO_STRING_FROM_SIZE_T( values.size() ) + " values." );

        // Build a key-value string matrix
        AQLStringMatrix stringMatrix( keys.size() );
        for ( size_t i = 0; i < keys.size(); ++i )
        {
            AQLStringVector stringVector( 2 );
            stringVector[0] = keys[i];
            stringVector[1] = values[i];
            stringMatrix[i] = stringVector;
        }

        // Call Native Constructor/Initializer
        initializefromAQLStringMatrix( stringMatrix, makeAllKeysUppercase );
    }

    LabelValueBlock::LabelValueBlock( const StandardStringVector & keys, const StandardStringVector & values, const bool& makeAllKeysUppercase )
    {
        AQ_REQUIRE( keys.size() == values.size(),
            "Unable to create LVB: Inconsistent number of keys and values. There are "
            + AQ_TO_STRING_FROM_SIZE_T( keys.size() ) + " keys and "
            + AQ_TO_STRING_FROM_SIZE_T( values.size() ) + " values." );

        // Build a key-value string matrix
        StandardStringMatrix stringMatrix( keys.size() );
        for ( size_t i = 0; i < keys.size(); ++i )
        {
            StandardStringVector stringVector( 2 );
            stringVector[0] = keys[i];
            stringVector[1] = values[i];
            stringMatrix[i] = stringVector;
        }

        // Call Native Constructor/Initializer
        initializefromStandardStringMatrix( stringMatrix, makeAllKeysUppercase );
    }

    // Constructor to concatinate two LVBs and create a new one
    LabelValueBlock::LabelValueBlock( const LabelValueBlock& originalLVB, const LabelValueBlock& appendLVB, const bool& makeAllKeysUppercase )
    {
        // Initialize Combined Results Matrix Size
        StandardStringMatrix combinedStringMatrix = combineStandardStringMatrices( originalLVB.toStandardStringMatrix(), appendLVB.toStandardStringMatrix() );

        // Call Native Constructor/Initializer
        initializefromStandardStringMatrix( combinedStringMatrix, makeAllKeysUppercase );
    }

    LabelValueBlock::LabelValueBlock( const LabelValueBlock& originalLVB, const AQLString& appendKey, const AQLString& appendValue, const bool& makeAllKeysUppercase )
    {
        // Build a key-value string vector
        AQLStringVector inputVector( 2 );
        inputVector[0] = appendKey;
        inputVector[1] = appendValue;

        // Create String Matrix from Vector
        const AQLStringMatrix appendMatrix( 1, inputVector );

        // Call Native Constructor/Initializer using Combined Matrix
        AQLStringMatrix combinedStringMatrix = combineAQLStringMatrices( originalLVB.toAQLStringMatrix(), appendMatrix );
        initializefromAQLStringMatrix( combinedStringMatrix, makeAllKeysUppercase );
    }
    
    LabelValueBlock::LabelValueBlock( const LabelValueBlock& originalLVB, const AQLStringVector& appendKeys, const AQLStringVector& appendValues, const bool& makeAllKeysUppercase )
    {
        AQ_REQUIRE( appendKeys.size() == appendValues.size(),
            "Unable to create LVB: Inconsistent number of keys and values. There are "
            + AQ_TO_STRING_FROM_SIZE_T( appendKeys.size() ) + " keys and "
            + AQ_TO_STRING_FROM_SIZE_T( appendValues.size() ) + " values." );

        // Build a key-value string matrix
        AQLStringMatrix appendMatrix( appendKeys.size() );
        for ( size_t i = 0; i < appendKeys.size(); ++i )
        {
            AQLStringVector appendVector( 2 );
            appendVector[0] = appendKeys[i];
            appendVector[1] = appendValues[i];
            appendMatrix[i] = appendVector;
        }

        // Call Native Constructor/Initializer using Combined Matrix
        AQLStringMatrix combinedStringMatrix = combineAQLStringMatrices( originalLVB.toAQLStringMatrix(), appendMatrix );
        initializefromAQLStringMatrix( combinedStringMatrix, makeAllKeysUppercase );
    }
    
    LabelValueBlock::LabelValueBlock( const LabelValueBlock& originalLVB, const StandardString& appendKey, const StandardString& appendValue, const bool& makeAllKeysUppercase )
    {
        StandardStringMatrix appendMatrix
        { 
            { appendKey, appendValue }
        };

        // Call Native Constructor/Initializer using Combined String Matrix
        StandardStringMatrix combinedStringMatrix = combineStandardStringMatrices( originalLVB.toStandardStringMatrix(), appendMatrix );
        initializefromStandardStringMatrix( combinedStringMatrix, makeAllKeysUppercase );
    }
    
    LabelValueBlock::LabelValueBlock( const LabelValueBlock& originalLVB, const StandardStringVector& appendKeys, const StandardStringVector& appendValues, const bool& makeAllKeysUppercase )
    {
         AQ_REQUIRE( appendKeys.size() == appendValues.size(),
            "Unable to create LVB: Inconsistent number of keys and values. There are "
            + AQ_TO_STRING_FROM_SIZE_T( appendKeys.size() ) + " keys and "
            + AQ_TO_STRING_FROM_SIZE_T( appendValues.size() ) + " values." );

        // Build a key-value string matrix
        StandardStringMatrix appendMatrix( appendKeys.size() );
        for ( size_t i = 0; i < appendKeys.size(); ++i )
        {
            StandardStringVector appendVector( 2 );
            appendVector[0] = appendKeys[i];
            appendVector[1] = appendValues[i];
            appendMatrix[i] = appendVector;
        }

        // Call Native Constructor/Initializer using Combined Matrix
        StandardStringMatrix combinedStringMatrix = combineStandardStringMatrices( originalLVB.toStandardStringMatrix(), appendMatrix );
        initializefromStandardStringMatrix( combinedStringMatrix, makeAllKeysUppercase );
    }

    // Static Helper: std::string Constructor to build a label value block with a single key and value
    LabelValueBlock LabelValueBlock::createLabelValueBlockUsingAQLString( const AQLString& key, const AQLString& value, const bool& makeAllKeysUppercase )
    {
        // Build a string vector with the string input specified twice, once as LVB key and once for the LVB value
        AQLStringVector inputVector( 2 );
        inputVector[0] = key;
        inputVector[1] = value;

        // Declare a String Matrix of size 1 and populate it with our string vector
        const AQLStringMatrix inputMatrix( 1, inputVector );
        
        // Call the Native Label Value Block Constuctor
        return LabelValueBlock( inputMatrix, makeAllKeysUppercase );
    }

    // Static Helper: std::string Constructor to build a label value block with a single key and value
    LabelValueBlock LabelValueBlock::createLabelValueBlock( const std::string& key, const std::string& value, const bool& makeAllKeysUppercase )
    {
        // Call the Label Value Block AQLString LVB helper above
        return createLabelValueBlockUsingAQLString( key.c_str(), value.c_str(), makeAllKeysUppercase );
    }

    /* @brief	Check if a Label Value Block is empty
    *  @return	A boolean, true = empty, false = contains data
    */
    bool LabelValueBlock::isEmpty() const
    {
        const bool isMapEmpty = ( objectMap_.size() == 0 );
        return isMapEmpty;
    }

	// Convert a LabelValueBlock to a AQLStringMatrix
    AQLStringMatrix LabelValueBlock::toAQLStringMatrix() const
    {
        return aqStringMatrix_;

        // AQLStringMatrix aqStringMatix( objectMap_.size() );
        // size_t index = 0;
        // for( auto iter = objectMap_.begin(); iter != objectMap_.end(); ++iter )
        // {
        //     // Label Value Blocks always have 2 Columns
        //     AQLStringVector aqStringVector(2);
        //     aqStringVector[0] = iter->first.c_str();
        //     aqStringVector[1] = iter->second.c_str();
        //     aqStringMatix[index] = aqStringVector;
        //     ++index;
        // }
        // return aqStringMatix;
    }


    // Convert a LabelValueBlock to a StandardStringMatrix
    StandardStringMatrix LabelValueBlock::toStandardStringMatrix() const
    {
        return standardStringMatrix_;

        // StandardStringMatrix standardStringMatix( objectMap_.size() );
        // size_t index = 0;
        // for( auto iter = objectMap_.begin(); iter != objectMap_.end(); ++iter )
        // {
        //     // Label Value Blocks always have 2 Columns
        //     StandardStringVector standardStringVector(2);
        //     standardStringVector[0] = iter->first;
        //     standardStringVector[1] = iter->second;
        //     standardStringMatix[index] = standardStringVector;
        //     ++index;
        // }
        // return standardStringMatix;
    }

	/* @brief	Convert the NamedString to a AQLStringMatrix object
    *  @return	A AQLStringMatrix obj
    */
    AnyTypeMatrix LabelValueBlock::toAnyTypeMatrix() const
    {
        AnyTypeMatrix m( objectMap_.size() );
        size_t index = 0;
        for( auto iter = objectMap_.begin(); iter != objectMap_.end(); ++iter )
        {
            // Label Value Blocks always have 2 Columns
            AnyTypeVector v(2);
            v[0] = iter->first;
            v[1] = iter->second;
            m[index] = v;
            ++index;
        }
        return m;
    }

    /* @brief	Convert the NamedString to a VariantMatrix object
    *  @return	A VariantMatrix obj
    */
    VariantMatrix LabelValueBlock::toVariantMatrix() const
    {
        VariantMatrix m( objectMap_.size() );
        size_t index = 0;
        for( auto iter = objectMap_.begin(); iter != objectMap_.end(); ++iter )
        {
            // Label Value Blocks always have 2 Columns
            VariantVector v(2);
            v[0] = iter->first.c_str();
            v[1] = iter->second.c_str();
            m[index] = v;
            ++index;
        }
        return m;
    }

    /* @brief	Retrieve value of an optional key as double
    *  @param [in]	key			Key for value look up
    *  @param [in]	defaultVal	Default value for the key
    *  @return		Value in double type
    */
    double LabelValueBlock::getOptionalValueAsDouble( const std::string& key, double defaultVal ) const
    {
        std::string value = getOptionalValue( key, std::string() );

        // Can't find the key. Return default.
        if ( value == std::string() )
        {
            return defaultVal;
        }

        double ret = StringToNumber<double>( value.c_str() );
        return ret;
    }

	int LabelValueBlock::getOptionalValueAsInt(const std::string& key, int defaultVal) const
	{
		std::string value = getOptionalValue(key, std::string());

		// Can't find the key. Return default.
		if (value == std::string())
		{
			return defaultVal;
		}

		double ret = StringToNumber<int>(value.c_str());
		return ret;
	}

		
		
	/* @brief	Retrieve value of an optional key as unsigned int
    *  @param [in]	key			Key for value look up
    *  @param [in]	defaultVal	Default value for the key
    *  @return		Value in unsigned int type
    */
    unsigned int LabelValueBlock::getOptionalValueAsUnsignedInt( const std::string& key, unsigned int defaultVal ) const
    {
        std::string value = getOptionalValue( key, std::string() );

        // Can't find the key. Return default.
        if ( value == std::string() )
        {
            return defaultVal;
        }

        int ret = StringToNumber<int>( value.c_str() );

        if (ret < 0)
        {
            throw ETradingException(  ( boost::format( "#Error: key '%i' need to have positive value" ) % key  ).str()  );
        }

        return unsigned(ret);
    }

    /* @brief	Retrieve value of a compulsory key as double
    *  @param [in]	key				Key for value look up
    *  @param [in]	labelValueBlockName	        Name of the labelValueBlock
    *  @return		Value in double type
    */
    double LabelValueBlock::getCompulsoryValueAsDouble( const std::string& key, const std::string& labelValueBlockName ) const
    {
        AQLString value = getCompulsoryValueAsAQLString( key, labelValueBlockName );
        double ret = StringToNumber<double>( value.getCString() );
        return ret;
    }

	/* @brief	Retrieve value of a compulsory key as Integer
	*  @param [in]	key				Key for value look up
	*  @param [in]	labelValueBlockName	        Name of the labelValueBlock
	*  @return		Value in double type
	*/
	int LabelValueBlock::getCompulsoryValueAsInt(const std::string& key, const std::string& labelValueBlockName) const
	{
		AQLString value = getCompulsoryValueAsAQLString(key, labelValueBlockName);
		double ret = StringToNumber<int>(value.getCString());
		return ret;
	}
	
	/* @brief	Retrieve value of an optional key as AQLDate
    *  @param [in]	key			Key for value look up
    *  @param [in]	defaultVal	Default value for the key
    *  @return		Value in double type
    */
    AQLDate LabelValueBlock::getOptionalValueAsDate( const std::string& key, const AQLDate& defaultVal ) const
    {
        std::string value = getOptionalValue( key, std::string() );

        // Can't find the key. Return default.
        if ( value == std::string() )
        {
            return defaultVal;
        }

        AQLDate ret = stringToDate( value.c_str(), "" );
        return ret;
    }

    /* @brief	Retrieve value of a compulsory key as AQLDate
    *  @param [in]	key				Key for value look up
    *  @param [in]	labelValueBlockName	        Name of the labelValueBlock
    *  @return		Value in double type
    */
    AQLDate LabelValueBlock::getCompulsoryValueAsDate( const std::string& key, const std::string& labelValueBlockName ) const
    {
        AQLString value = getCompulsoryValueAsAQLString( key, labelValueBlockName );
        AQLDate ret = stringToDate( value.getCString(), "" );
        return ret;
    }

    /* @brief	Retrieve value of an optional key as AQLDate
    *  @param [in]	key			    Key for value look up
    *  @param [in]	referenceDate	When the underlying date is quoted as a tenor this is the reference date for that tenor e.g. today + 5Y
    *  @param [in]	defaultVal	    Default value for the key
    *  @return		Value in double type
    */
    AQLDate LabelValueBlock::getOptionalDateOrTenorAsDate( const std::string& key, const AQLDate& referenceDate, const AQLDate& defaultVal ) const
    {
        std::string value = getOptionalValue( key, std::string() );

        // Can't find the key. Return default.
        if ( value == std::string() )
        {
            return defaultVal;
        }

        // Get the maturity date from a Date or from a Tenor String
        AQLDate ret = validateMaturityDate( referenceDate, AQLString( value.c_str() ) );
        return ret;
    }

    /* @brief	Retrieve value of a compulsory key as AQLDate
    *  @param [in]	key				Key for value look up
    *  @param [in]	referenceDate	When the underlying date is quoted as a tenor this is the reference date for that tenor e.g. today + 5Y
    *  @param [in]	labelValueBlockName	        Name of the labelValueBlock
    *  @return		Value in double type
    */
    AQLDate LabelValueBlock::getCompulsoryDateOrTenorAsDate( const std::string& key, const AQLDate& referenceDate, const std::string& labelValueBlockName ) const
    {
        AQLString value = getCompulsoryValueAsAQLString( key, labelValueBlockName );
        AQLDate ret = validateMaturityDate( referenceDate, value );
        return ret;
    }

    /* @brief	Retrieve value of an optional key as boolean
    *  @param [in]	key			Key for value look up
    *  @param [in]	defaultVal	Default value for the key
    *  @return		Value in boolean type
    */
    bool LabelValueBlock::getOptionalValueAsBool( const std::string& key, const bool& defaultVal ) const
    {
        std::string value = getOptionalValue( key, std::string() );

        // Can't find the key. Return default.
        if ( value == std::string() )
        {
            return defaultVal;
        }

        bool ret = false;
        if ( boost::iequals( value.c_str(), "TRUE" ) )
        {
            ret = true;
        }
        else if ( boost::iequals( value.c_str(), "FALSE" ) )
        {
            ret = false;
        }
        else
        {
            std::stringstream s;
            s << "key '" << key << "' should carry a TRUE/FALSE value in the named collection.";
            throw AQLCoreInvalidData( s.str().c_str(), __FILE__, __LINE__ );
        }

        return ret;
    }

    /* @brief	Retrieve value of a compulsory key as boolean
    *  @param [in]	key				Key for value look up
    *  @param [in]	labelValueBlockName	        Name of the labelValueBlock
    *  @return		Value in boolean type
    */
    bool LabelValueBlock::getCompulsoryValueAsBool( const std::string& key, const std::string& labelValueBlockName ) const
    {
        AQLString value = getCompulsoryValueAsAQLString( key, labelValueBlockName );

        bool ret = false;
        if ( value.toUpper() == "TRUE" )
        {
            ret = true;
        }
        else if ( value.toUpper() == "FALSE" )
        {
            ret = false;
        }
        else
        {
            std::stringstream s;
            s << "key '" << key << "' should carry a TRUE/FALSE value in the named collection.";
            throw AQLCoreInvalidData( s.str().c_str(), __FILE__, __LINE__ );
        }

        return ret;
    }

    /* @brief	Retrieve value of an optional key as AQLString
    *  @param [in]	key			Key for value look up
    *  @param [in]	defaultVal	Default value for the key
    *  @return		Value in AQLString type
    */
    AQLString LabelValueBlock::getOptionalValueAsAQLString( const std::string& key, const AQLString& defaultVal) const
    {
		std::string value = getOptionalValueAsString(key, defaultVal.getCString());
		
		return value.c_str();
    }

	/* @brief	Retrieve value of an optional key as std::string
	*  @param [in]	key			Key for value look up
	*  @param [in]	defaultVal	Default value for the key
	*  @return		Value in std::string type
	*/
	std::string LabelValueBlock::getOptionalValueAsString(const std::string& key, const std::string& defaultVal) const
	{
		std::string value = getOptionalValue(key, std::string());

		// Can't find the key. Return default.
		if (value == std::string())
		{
			return defaultVal;
		}

		return value;
	}

	/* @brief	Retrieve value of a compulsory key as AQLString
    *  @param [in]	key				Key for value look up
    *  @param [in]	labelValueBlockName	        Name of the labelValueBlock
    *  @param [in]	upperCase	    Return an upper case string
    *  @return		Value in AQLString type
    */
    AQLString LabelValueBlock::getCompulsoryValueAsAQLString( const std::string& key, const std::string& labelValueBlockName, bool makeKeyUppercase ) const
    {
		std::string value = getCompulsoryValueAsString(key, labelValueBlockName, makeKeyUppercase);

        return value.c_str();
    }

	std::string LabelValueBlock::getCompulsoryValueAsString(const std::string& key, const std::string& labelValueBlockName, bool makeKeyUppercase) const
	{
		std::string value = getCompulsoryValue(key, labelValueBlockName, makeKeyUppercase);

		if (value.size() == 0 || value == "")
		{
			std::stringstream s;
			s << "Compulsory key '" << key << "' does not exist in the Label Value Block";

			// Append the Label Value Block collection name if known
			if (labelValueBlockName != "")
			{
				s << ", '" + labelValueBlockName + "'.";
			}

			throw AQLCoreInvalidData(s.str().c_str(), __FILE__, __LINE__);
		}

		return value;
	}

	/* @brief	Retrieve value of optional keys as AQLString
    *  @param [in]	key1			Key for value look up
    *  @param [in]	key2			Key for value look up if key1 cannot be found
    *  @param [in]	defaultVal	Default value for the key
    *  @return		Value in AQLString type
    */
    AQLString LabelValueBlock::getOptionalValueAsAQLStringFromKeys( const std::string& key1, const std::string& key2, const AQLString& defaultVal) const
    {
        std::string value = getOptionalValueAsStringFromKeys( key1, key2, defaultVal.getCString());
        return value.c_str();
    }

	/* @brief	Retrieve value of optional keys as std::string
	*  @param [in]	key1			Key for value look up
	*  @param [in]	key2			Key for value look up if key1 cannot be found
	*  @param [in]	defaultVal	Default value for the key
	*  @return		Value in std::string type
	*/
	std::string LabelValueBlock::getOptionalValueAsStringFromKeys(const std::string& key1, const std::string& key2, const std::string& defaultVal) const
	{
		std::string value = getOptionalValue(key1, std::string());

		// Can't find the key. Return default.
		if (value == std::string())
		{
			value = getOptionalValue(key2, std::string());
			if (value == std::string())
			{
				return defaultVal;
			}
		}

		return value;
	}

  	/* @brief	Retrieve value of optional keys as AQLString
    *  @param [in]	keys			Keys for value look up
    *  @param [in]	defaultVal	Default value for the key
    *  @return		Value in AQLString type
    */
    AQLString LabelValueBlock::getOptionalValueAsAQLStringFromMultipleKeys( const std::vector<std::string>& keys, const AQLString& defaultVal) const
    {
        std::string value = getOptionalValueAsStringFromMultipleKeys(keys, defaultVal.getCString());

        return value.c_str();
    }

	/* @brief	Retrieve value of optional keys as std::string
	*  @param [in]	keys			Keys for value look up
	*  @param [in]	defaultVal	Default value for the key
	*  @return		Value in std::string type
	*/
	std::string LabelValueBlock::getOptionalValueAsStringFromMultipleKeys(const std::vector<std::string>& keys, const std::string& defaultVal) const
	{
		checkForDuplicateKeys(keys);

		std::string value = std::string();
		for (size_t i = 0; i < keys.size(); ++i)
		{
			value = getOptionalValue(keys[i], std::string());
			if (value != std::string())
			{
				break;
			}
		}

		// Can't find the key. Return default.
		if (value == std::string())
		{
			return defaultVal;
		}

		return value;
	}

	/* @brief	Retrieve value of optional keys as double
    *  @param [in]	key1			Key for value look up
    *  @param [in]	key2			Key for value look up if key1 cannot be found
    *  @param [in]	defaultVal	    Default value for the key
    *  @return		Value in double type
    */
    double LabelValueBlock::getOptionalValueAsDoubleFromKeys( const std::string& key1, const std::string& key2, double defaultVal) const
	{

        checkForDuplicateKeys(boost::assign::list_of(key1)(key2));

        std::string value = getOptionalValue( key1, std::string() );

        // Can't find the key. Return default.
        if ( value == std::string() )
        {
            value = getOptionalValue( key2, std::string() );
			if ( value == std::string() )
			{
				return defaultVal;
			}
        }

        double ret = StringToNumber<double>( value.c_str() );
        return ret;

	}

    /* @brief	Retrieve value of compulsory keys as AQLString
    *  @param [in]	key1			Key for value look up
    *  @param [in]	key2			Key for value look up if key1 cannot be found
    *  @param [in]	labelValueBlockName	        Name of the labelValueBlock
    *  @return		Value in AQLString type
    */
    AQLString LabelValueBlock::getCompulsoryValueAsAQLStringFromKeys(const std::string& key1, const std::string& key2, const std::string& labelValueBlockName) const
    {

        checkForDuplicateKeys(boost::assign::list_of(key1)(key2));

        std::string value = getOptionalValueAsAQLStringFromKeys(key1, key2).getCString(); 
        if ( value.size() == 0 || value == "" )
        {
            std::stringstream s;
            s << "Compulsory key '" << key1 << "' or '" << key2 << "' does not exist in the Label Value Block";
            
            // Append the Label Value Block collection name if known
            if ( labelValueBlockName != "" )
            {
                s << ", '" + labelValueBlockName + "'.";
            }
            
            throw AQLCoreInvalidData( s.str().c_str(), __FILE__, __LINE__ );
        }

        return value.c_str();
    }

  	/* @brief	Retrieve value of compulsory keys as AQLString
    *  @param [in]	keys			Keys for value look up
    *  @param [in]	labelValueBlockName	        Name of the labelValueBlock
    *  @return		Value in AQLString type
    */
    AQLString LabelValueBlock::getCompulsoryValueAsAQLStringFromMultipleKeys( const std::vector<std::string>& keys, const std::string& labelValueBlockName) const
    {

        checkForDuplicateKeys(keys);

        std::string value = getOptionalValueAsAQLStringFromMultipleKeys(keys).getCString(); 
        if ( value.size() == 0 || value == "" )
        {
            std::stringstream s;
            s << "None of the Compulsory keys '";
            std::copy(keys.begin(), keys.end(),std::ostream_iterator<std::string>(s,", "));
            s << "' does not exist in the Label Value Block";
            // Append the Label Value Block collection name if known
            if ( labelValueBlockName != "" )
            {
                s << ", '" + labelValueBlockName + "'.";
            }
            
            throw AQLCoreInvalidData( s.str().c_str(), __FILE__, __LINE__ );
        }

        return value.c_str();

    }


    /* @brief	Retrieve value of a compulsory key as double
    *  @param [in]	key1			Key for value look up
    *  @param [in]	key2			Key for value look up if key1 cannot be found
    *  @param [in]	labelValueBlockName	        Name of the labelValueBlock
    *  @return		Value in double type
    */
    double LabelValueBlock::getCompulsoryValueAsDoubleFromKeys( const std::string& key1, const std::string& key2, const std::string& labelValueBlockName ) const
    {
        checkForDuplicateKeys( boost::assign::list_of(key1)(key2) );

        double value = getOptionalValueAsDoubleFromKeys(key1, key2, std::numeric_limits<double>::quiet_NaN() ); 
        if ( boost::math::isnan( value ) )
        {
            std::stringstream s;
            s << "Compulsory key '" << key1 << "' or '" << key2 << "' does not exist in the Label Value Block";
            
            // Append the Label Value Block collection name if known
            if ( labelValueBlockName != "" )
            {
                s << ", '" + labelValueBlockName + "'.";
            }
            
            throw AQLCoreInvalidData( s.str().c_str(), __FILE__, __LINE__ );
        }

        return value;
    }

    /* @brief	Validate the LVB only contains one of the keys
    *  @param [in]	keys			Keys for value look up
    */
    void LabelValueBlock::checkForDuplicateKeys(const std::vector<std::string>& keys) const
    {
        int count = 0;
        for (size_t i = 0; i < keys.size(); ++i)
        {
            auto key = keys[i];
            auto iter = objectMap_.find( AQLString( key.c_str() ).toUpper().getCString() );
            if ( iter != objectMap_.end() )
            {
                count++;
                if (count > 1)
                {
                    std::stringstream s;
                    s << "#Error: Label Value Block cannot have multiple keys at the same time: '"; 
                    std::copy(keys.begin(), keys.end(),std::ostream_iterator<std::string>(s, ", "));
                    throw AQLCoreInvalidData( s.str().c_str(), __FILE__, __LINE__ );
                }
            }
        }
    }

    // Private Helper Method to Construct a LVB from a AQLStringMatrix
    void LabelValueBlock::initializefromAQLStringMatrix( const AQLStringMatrix & input, const bool& makeAllKeysUppercase )
    {
        // Update Matrix Member Data
        aqStringMatrix_         = input;
        standardStringMatrix_   = convertToStandardStringMatrix( input );
        
        // Build Label Value Block
        size_t keyCount = input.size();
        for ( size_t i = 0; i < keyCount; ++i )
        {
            AQLStringVector element = input[i];

			//if key is empty string, do not add to the map
			if(element.size() > 0 && element[0].size()==0)
			{
				continue;
			}

            // Uppercase the key on request
            AQLString key = makeAllKeysUppercase ? element[0].toUpper() : element[0];
            
            if ( element.size() == 2 )
            {
                // Both key and value are provided
                add( key.getCString(), element[1].getCString() );
            }
            else if ( element.size() == 1 )
            {
                // Only key is provided. Value defaults to "".
                std::string value( "" );
                add( key.getCString(), value );
            }
            else
            {
                AQ_THROW( "Invalid Input Matrix: Must be a 2 column matrix, namely a column of keys and a column of values")
            }
        }
    }

    // Private Helper Method to Construct a LVB from a StandardStringMatrix
    void LabelValueBlock::initializefromStandardStringMatrix( const StandardStringMatrix & input, const bool& makeAllKeysUppercase )
    {
        // Update Matrix Member Data
        aqStringMatrix_         = convertToAQLStringMatrix( input );
        standardStringMatrix_   = input;
        
        // Build Label Value Block
        size_t keyCount = input.size();
        for ( size_t i = 0; i < keyCount; ++i )
        {
            StandardStringVector element = input[i];

			//if key is empty string, do not add to the map
			if(element.size() > 0 && element[0].size()==0)
			{
				continue;
			}
            
            // Uppercase the key on request
            StandardString key = element[0];
            if ( makeAllKeysUppercase )
            {
                boost::to_upper( key );
            }

            if ( element.size() == 2 )
            {
                // Both key and value are provided
                add( key, element[1] );
            }
            else if ( element.size() == 1 )
            {
                // Only key is provided. Value defaults to "".
                std::string value( "" );
                add( key, value );
            }
            else
            {
                AQ_THROW( "Invalid Input Matrix: Must be a 2 column matrix, namely a column of keys and a column of values")
            }
        }
    }

    //-----------------------------------------------------------------------------
    // Building LabelValueBlock(s) from AQLStringMatrix


    /* @brief	Build a LabelValueBlock from a AQLStringMatrix
    *  @param [in]	input			A given AQLStringMatrix
    *  @return		One LabelValueBlock object
    */
    LabelValueBlock buildSingleLabelValueBlock( const AQLStringMatrix& input, const bool& makeAllKeysUppercase )
    {
        if ( input.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: an empty Label Value Block object is given", __FILE__, __LINE__ );
        }

        LabelValueBlock a( input, makeAllKeysUppercase );
        return a;
    }

    /* @brief		Build a vector of LabelValueBlocks from a AQLStringMatrix
    *  @param [in]	input			A given AQLStringMatrix
    *  @return		A vector of LabelValueBlock objects
    */
    std::vector<LabelValueBlock> buildMultiLabelValueBlock( const AQLStringMatrix& input, const bool& makeAllKeysUppercase )
    {
        size_t keyCount = input.size();
        if ( keyCount == 0 )
        {
            throw AQLCoreInvalidData( "#Error: an empty Label Value Block object is given", __FILE__, __LINE__ );
        }

        size_t componentStringMatrixCount = input[0].size() - 1;
        if ( componentStringMatrixCount == 0 )
        {
            throw AQLCoreInvalidData( "#Error: the input Label Value Block only has one column", __FILE__, __LINE__ );
        }

        // Break up the input AQLStringMatrix and convert it to a vector of smaller
        // AQLStringMatrix objects each of which will have only one column of keys and one column of values
        std::vector<AQLStringMatrix> regroupedMatrix( componentStringMatrixCount );
        for ( size_t i = 0; i < keyCount; ++i )
        {
            AQLStringVector currentRow = input[i];

            AQLStringVector newRow;
            AQLString key = currentRow[0];
            newRow.push_back( key );

            size_t rowCount = currentRow.size();
            for ( size_t j = 1; j < rowCount; ++j )
            {
                newRow.push_back( currentRow[j] );

                AQLStringMatrix& m = regroupedMatrix[j - 1];
                m.push_back( newRow );

                newRow.clear();
                newRow.push_back( key );
            }
        }

        std::vector<LabelValueBlock> vectorLVB;
        for ( size_t i = 0; i < componentStringMatrixCount; ++i )
        {
            LabelValueBlock LVB = buildSingleLabelValueBlock( regroupedMatrix[i], makeAllKeysUppercase );
            vectorLVB.push_back( LVB );
        }

        return vectorLVB;
    }

    LabelValueBlock buildSingleLabelValueBlock( const StandardStringVector& keys, const StandardStringVector& values, const bool& makeAllKeysUppercase )
    {
        LabelValueBlock lvb( keys, values, makeAllKeysUppercase );
        return lvb;
    }

    // Helper Methods - Matrix & Vector Conversion 
    // --------------------------------------------------------------------------------

    StandardStringVector convertToStandardStringVector( const AQLStringVector& aqStringVector )
    {
        StandardStringVector resultVector( aqStringVector.size());
        for ( size_t i = 0; i < aqStringVector.size(); ++i )
        {
            resultVector[i] = aqStringVector[i].c_str();
        }
        return resultVector;
    }
    
    AQLStringVector convertToAQLStringVector( const StandardStringVector& standardStringVector )
    {
        AQLStringVector resultVector( standardStringVector.size());
        for ( size_t i = 0; i < standardStringVector.size(); ++i )
        {
            resultVector[i] = standardStringVector[i].c_str();
        }
        return resultVector;
    }

    // Method to convert a AQLStringMatrix to a StandardStringMatrix 
    StandardStringMatrix convertToStandardStringMatrix( const AQLStringMatrix & aqStringMatrix )
    {
        StandardStringMatrix standardStringMatrix( aqStringMatrix.size() );
        for( size_t i = 0; i < aqStringMatrix.size(); ++i)
        {
            standardStringMatrix[i] = convertToStandardStringVector( aqStringMatrix[i] );
        }
        return standardStringMatrix;
    }
    
    // Method to convert a StandardStringMatrix to a AQLStringMatrix
    AQLStringMatrix convertToAQLStringMatrix( const StandardStringMatrix & standardStringMatrix )
    {
        AQLStringMatrix aqStringMatrix( standardStringMatrix.size() );
        for( size_t i = 0; i < standardStringMatrix.size(); ++i)
        {
            aqStringMatrix[i] = convertToAQLStringVector( standardStringMatrix[i] );
        }
        return aqStringMatrix;
    }

    // Method to concatinate two Standard String Matrices
    StandardStringMatrix combineStandardStringMatrices( const StandardStringMatrix & matrix1, const StandardStringMatrix & matrix2 )
    {
        StandardStringMatrix combinedMatrix( matrix1.size() + matrix2.size() );

        size_t index = 0;
        for ( size_t i = 0; i < matrix1.size(); ++i )
        {
            combinedMatrix[index] = matrix1[i];
            ++index;
        }

        for ( size_t j = 0; j < matrix2.size(); ++j )
        {
            combinedMatrix[index] = matrix2[j];
            ++index;
        }
        
        return combinedMatrix;
    }
    
    // Method to concatinate two LA String Matrices
    AQLStringMatrix combineAQLStringMatrices( const AQLStringMatrix & matrix1, const AQLStringMatrix & matrix2 )
    {
        AQLStringMatrix combinedMatrix( matrix1.size() + matrix2.size() );

        size_t index = 0;
        for ( size_t i = 0; i < matrix1.size(); ++i )
        {
            combinedMatrix[index] = matrix1[i];
            ++index;
        }

        for ( size_t j = 0; j < matrix2.size(); ++j )
        {
            combinedMatrix[index] = matrix2[j];
            ++index;
        }
        
        return combinedMatrix;
    }

    // Helper Methods - Getting Values from LVB String Matrices
    // --------------------------------------------------------------------------------
    
    AQLString searchAQLStringMatrix( const AQLString & searchKey, const AQLStringMatrix & searchMatrix )
    {
        AQ_REQUIRE( searchMatrix.size() >0, "Unable to find lookup value: Thesearch matrix is empty" )
        
        // Assume a Square Matrix for Performance Reasons and Only check dimensions of the first inner vector
        AQ_REQUIRE( searchMatrix[0].size() == 2, "Invalid LVB search matrix: LVB searchMatrix must have exactly 2 columns, namely a column of keys and a column of values" )
        
        for ( size_t i = 0; i < searchMatrix.size(); ++i )
        {
            // Search Column 1: Keys
            if ( boost::iequals( searchKey.c_str(), searchMatrix[i][0].c_str() ) )
            {
                // Return Column 2: Value
                return searchMatrix[i][1];
            }
        }

        AQ_THROW("Unable to find key in search matrix")
    }
    
    StandardString searchStandardStringMatrix( const StandardString & searchKey, const StandardStringMatrix & searchMatrix )
    {
        AQ_REQUIRE( searchMatrix.size() > 0, "Unable to find lookup value: Thesearch matrix is empty" )
        
        // Assume a Square Matrix for Performance Reasons and Only check dimensions of the first inner vector
        AQ_REQUIRE( searchMatrix[0].size() == 2, "Invalid LVB search matrix: LVB searchMatrix must have exactly 2 columns, namely a column of keys and a column of values" )

        for ( size_t i = 0; i < searchMatrix.size(); ++i )
        {
            // Search Column 1: Keys
            if ( boost::iequals( searchKey, searchMatrix[i][0] ) )
            {
                // Return Column 2: Value
                return searchMatrix[i][1];
            }
        }

        AQ_THROW("Unable to find key in search matrix")
    }

}
