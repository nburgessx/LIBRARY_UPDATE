#include "tryAqToolsLVB.h"
#include "AQLMathInterpolationUtilities.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"

using etrading::CreateDataFile;

namespace validation
{


    AQLString getCurrentPrefix( AQLStringVector& keyPrefixes )
    {
        AQLString prefix = AQLString();
        if ( keyPrefixes.size() != 0 )
        {
            prefix = keyPrefixes.front();
            keyPrefixes.erase( keyPrefixes.begin() );
        }
        return prefix;
    }

    /* @brief			validation interface for aqToolsLVBFromKeysValues method
    *  @param [in]		keys			A list of keys from user input
    *  @param [in]		values			A list of values from user input
    *  @param [in]		keyPrefix		Prefix added to for keys
    *  @return			a AQLStringMatrix representing LabelValueBlock, i.e. first columns are keys, second columns are values
    */
    AQLStringMatrix tryAqToolsLVBFromKeysValues( const AQLStringVector& keys, const AQLStringVector& values, const AQLString& keyPrefix )
    {
        VALID_EXCEPTION_START

        // Note: No need to record it as the output of this funciton will be the input of another function
        //// Recording of inputs for playback
        //if (CreateDataFile::recordEnabled())
        //{
        //	CreateDataFile file("tryAqToolsLVBFromKeysValues_inputs");
        //	file.write("generatorFunction", "tryAqToolsLVBFromKeysValues");
        //	file.write("keys", keys);
        //	file.write("values", values);
        //	file.write("keyPrefix", keyPrefix);
        //}

        LabelValueBlock lvb = etrading::populateLabelValueBlock( keys, values, keyPrefix );

        AQLStringMatrix ret = etrading::fromLabelValueBlockToStringMatrix( lvb );

        //if (CreateDataFile::recordEnabled())
        //{
        //	CreateDataFile file("tryAqToolsLVBFromKeysValues_outputs");
        //	file.write("output", ret);
        //}

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for aqToolsLVB method
    *  @param [in]		sMatrix			A input AQLStringMatrix with common key column/row and mutltiple value columns/rows
    *  @param [in]		keyPrefix		Prefix added to for keys
    *  @param [in]		verticalKeys	True if the keys are vertical
    *  @return			a AQLStringMatrix representing a LabelValueBlock, i.e. first columns are keys, second columns are values
    */
    AQLStringMatrix tryAqToolsLVB( const AQLStringMatrix& sMatrix, AQLStringVector& keyPrefixes, bool verticalKeys )
    {
        VALID_EXCEPTION_START

        // Note: No need to record it as the output of this funciton will be the input of another function
        //// Recording of inputs for playback
        //if (CreateDataFile::recordEnabled())
        //{
        //	CreateDataFile file("tryAqToolsLVB_inputs");
        //	file.write("generatorFunction", "tryAqToolsLVB");
        //	file.write("sMatrix", sMatrix);
        //	file.write("keyPrefixes", keyPrefixes);
        //	file.write("verticalKeys", verticalKeys);
        //}

        if ( sMatrix.size() == 0 )
        {
            throw AQLCoreInvalidData( "#Error: the Matrix size is zero.", __FILE__, __LINE__ );
        }

        if ( sMatrix[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "#Error: the Matrix needs to have more than one column.", __FILE__, __LINE__ );
        }


        // we assume all rows have the same column size
        size_t rowSize = sMatrix.size();
        size_t columnSize = sMatrix[0].size();
        std::vector<LabelValueBlock> lvbs;
        if ( verticalKeys )
        {
            // first column contains keys
            for ( size_t j = 1; j < columnSize; ++j )
            {
                AQLStringVector keys;
                AQLStringVector values;
                for ( size_t i = 0; i < rowSize; ++i )
                {
                    keys.push_back( sMatrix[i][0] );
                    values.push_back( sMatrix[i][j] );
                }

                AQLString prefix = getCurrentPrefix( keyPrefixes );
                LabelValueBlock lvb = etrading::populateLabelValueBlock( keys, values, prefix );

                lvbs.push_back( lvb );
            }
        }
        else
        {
            // first row contains keys
            for ( size_t i = 1; i < rowSize; ++i )
            {
                AQLStringVector keys;
                AQLStringVector values;
                for ( size_t j = 0; j < columnSize; ++j )
                {
                    keys.push_back( sMatrix[0][j] );
                    values.push_back( sMatrix[i][j] );
                }
                AQLString prefix = getCurrentPrefix( keyPrefixes );
                LabelValueBlock lvb = etrading::populateLabelValueBlock( keys, values, prefix );
                lvbs.push_back( lvb );
            }
        }

        //transform to AQLStringMatrix format
        AQLStringMatrix ret;
        for ( size_t i = 0; i < lvbs.size(); ++i )
        {
            AQLStringMatrix temp = etrading::fromLabelValueBlockToStringMatrix( lvbs[i] );
            etrading::appendToMatrix( ret, temp );
        }

        //if (CreateDataFile::recordEnabled())
        //{
        //	CreateDataFile file("tryAqToolsLVB_outputs");
        //	file.write("output", ret);
        //}

        return ret;

        VALID_EXCEPTION_END
    }



    /* @brief			validation interface for aqToolsLVBGroup method
    *  @param [in]		lvbs	A list of AQLStringMatrix objects representing label value blocks
    *  @return			A AQLStringMatrix representing the concatenated LabelValueBlock
    */
    AQLStringMatrix tryAqToolsLVBGroup( const std::vector<AQLStringMatrix>& lvbs )
    {
        VALID_EXCEPTION_START

        // Note: No need to record it as the output of this funciton will be the input of another function
        //// Recording of inputs for playback
        //if (CreateDataFile::recordEnabled())
        //{
        //	CreateDataFile file("tryAqToolsLVBGroup_inputs");
        //	file.write("generatorFunction", "tryAqToolsLVBGroup");
        //	for (size_t i=0; i<lvbs.size(); ++i)
        //	{
        //		file.write("lvbs"+i, lvbs[i]);
        //	}
        //}

        AQLStringMatrix ret;
        for ( size_t i = 0; i < lvbs.size(); ++i )
        {
            etrading::validateLVBStringMatrix( lvbs[i] );
            etrading::appendToMatrix( ret, lvbs[i] );
        }

        //if (CreateDataFile::recordEnabled())
        //{
        //	CreateDataFile file("tryAqToolsLVBGroup_outputs");
        //	file.write("output", ret);
        //}

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for aqToolsLVBFromMultipleKeysValues method
    *  @param [in]		keys			Multiple lists of keys from user input
    *  @param [in]		values			Multiple lists of values from user input
    *  @return			a AQLStringMatrix representing LabelValueBlock, i.e. first columns are keys, second columns are values
    */
    AQLStringMatrix tryAqToolsLVBFromMultipleKeysValues( const std::vector<AQLStringVector>& keys,
            const std::vector<AQLStringVector>& values )
    {
        VALID_EXCEPTION_START

        // Note: No need to record it as the output of this funciton will be the input of another function
        //// Recording of inputs for playback
        //if (CreateDataFile::recordEnabled())
        //{
        //	CreateDataFile file("tryAqToolsLVBFromKeysValues_inputs");
        //	file.write("generatorFunction", "tryAqToolsLVBFromKeysValues");
        //	for (size_t i=0; i<keys.size(); ++i)
        //	{
        //		file.write("keys", keys[i]);
        //	}
        //	for (size_t i=0; i<values.size(); ++i)
        //	{
        //		file.write("values", values[i]);
        //	}
        //}

        if ( keys.size() != values.size() )
        {
            throw AQLCoreInvalidData( "#Error: Keys and Values are not the same size.", __FILE__, __LINE__ );
        }

        AQLStringMatrix ret;
        for ( size_t i = 0; i < keys.size(); ++i )
        {
            LabelValueBlock lvb = etrading::populateLabelValueBlock( keys[i], values[i] );
            AQLStringMatrix temp = etrading::fromLabelValueBlockToStringMatrix( lvb );
            etrading::appendToMatrix( ret, temp );
        }


        //if (CreateDataFile::recordEnabled())
        //{
        //	CreateDataFile file("tryAqToolsLVBFromKeysValues_outputs");
        //	file.write("output", ret);
        //}

        return ret;

        VALID_EXCEPTION_END
    }

    /* @brief			validation interface for aqToolsLVBFromKeysAndMultipleValues method
    *  @param [in]		commonKeys		A lists of keys from user input
    *  @param [in]		multiValues		Multiple lists of values from user input
    *  @return			a vertial AQLStringMatrix with a key column and multiple values columns
    */
    AQLStringMatrix tryAqToolsLVBFromKeysAndMultipleValues( const AQLStringVector& commonKeys, const std::vector<AQLStringVector>& multiValues)
    {
        VALID_EXCEPTION_START

        AQLStringMatrix result;
        size_t keysSize = commonKeys.size();
        for ( size_t i = 0; i < keysSize; ++i )
        {
            AQLStringVector tempVec;
            auto key = commonKeys[i];
            tempVec.push_back(key);
            for ( size_t j = 0; j < multiValues.size(); ++j )
            {
                auto values = multiValues[j];
                if ( keysSize != values.size())
                {
                    throw AQLCoreInvalidData( "#Error: Keys and Values are not the same size.", __FILE__, __LINE__ );
                }
                auto value = values[i];
                tempVec.push_back(value);
            }
            result.push_back( tempVec );
        }

        return result;

        VALID_EXCEPTION_END
    }

    /* @brief			create a string matrix
    *  @param [in]		keys			A vector of keys from user input
    *  @param [in]		value			A vector of values from user input
    *  @return			A string matrix of keys and values as a single data block
    */
    STDStringMatrix tryAqToolsLVBCreate( const STDStringVector& keys, const STDStringVector& values )
    {
        AQ_REQUIRE( keys.size() == values.size(), "Invalid Label Value Block Dimensions: The number of keys and values must match" )
        
        STDStringMatrix results( keys.size() );
        STDStringVector keyValuePair( 2 ); // Size 2
        
        for( size_t i = 0; i < keys.size(); ++i )
        {
            // Update the Initialized KeyValuePair Vector
            keyValuePair[0] = keys[i];
            keyValuePair[1] = values[i];

            // Update the Initialized Results Matrix
            results[i] = keyValuePair;
        }

        return results;
    }

    /* @brief			create a string matrix
    *  @param [in]		keys			A vector of keys from user input
    *  @param [in]		values1			A vector of values1 from user input
    *  @param [in]		values2			A vector of values2 from user input
    *  @return			A string matrix of keys and values as a single data block
    */
    STDStringMatrix tryAqToolsLVBCreate( const STDStringVector& keys, const STDStringVector& values1, const STDStringVector& values2 )
    {
        AQ_REQUIRE( keys.size() == values1.size(), "Invalid Label Value Block Dimensions: The number of keys and values1 must match" )
        AQ_REQUIRE( values1.size() == values2.size(), "Invalid Label Value Block Dimensions: The number of values1 and values2 must match" )
        
        STDStringMatrix results( keys.size() );
        STDStringVector keyValuePair( 3 ); // Size 3
        
        for( size_t i = 0; i < keys.size(); ++i )
        {
            // Update the Initialized KeyValuePair Vector
            keyValuePair[0] = keys[i];
            keyValuePair[1] = values1[i];
            keyValuePair[2] = values2[i];

            // Update the Initialized Results Matrix
            results[i] = keyValuePair;
        }

        return results;
    }

    /* @brief			Add a key and value to an existing Label Value Block (LVB)
    *  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
    *  @param [in]		key                 The new key to add to the original string matrix
    *  @param [in]		value               The new value to add to the original string matrix
    *  @return			The input LVB is appended with the key and value
    */
    void tryAqToolsLVBAdd( STDStringMatrix& LVB, const std::string& key, const std::string& value )
    {
        // Access Violation Guard
        if ( LVB.size() > 0 )
        {
            // Ensure input LVB has exactly 2 columns
            AQ_REQUIRE( LVB[0].size() == 2, "Invalid Label Value Block: Input LVB must have exactly 2 columns" )
        }

        // Append the key value pair to the Label Value Block
        STDStringVector keyValuePair = { key, value };
        LVB.push_back( keyValuePair );

        return;
    }

    /* @brief			Add a key and value to an existing Label Value Block (LVB)
    *  @param [in]		LVB                 The original string matrix Label Value Block (LVB)
    *  @param [in]		key                 The new key to add to the original string matrix
    *  @param [in]		value1              The new value to add to the original string matrix
    *  @param [in]		value2              The new value to add to the original string matrix
    *  @return			The input LVB is appended with the key, value1 and value2
    */
    void tryAqToolsLVBAdd( STDStringMatrix& LVB, const std::string& key, const std::string& value1, const std::string& value2 )
    {
         // Access Violation Guard
        if ( LVB.size() > 0 )
        {
            // Ensure input LVB has exactly 3 columns
            AQ_REQUIRE( LVB[0].size() == 3, "Invalid Label Value Block: Input LVB must have exactly 3 columns" )
        }

        // Append the key value pair to the Label Value Block
        STDStringVector keyValuePair = { key, value1, value2 };
        LVB.push_back( keyValuePair );

        return;
    }
}
