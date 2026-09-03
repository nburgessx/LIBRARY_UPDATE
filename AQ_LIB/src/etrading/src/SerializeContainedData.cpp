
//
// @File: SerializeContainedData.cpp
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.



#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

#include <boost/format.hpp>
#include <boost/range/irange.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/fusion/algorithm/transformation/push_back.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <rapidjson/prettywriter.h>

#include "StaticStructureStore.h"
#include "SerializeContainedData.h"
#include "FileUtilities.h"
#include "ETradingException.h"
#include "StaticStructureStore.h"
#include "TableDateDouble.h"
#include "FreeObject.h"
#include "SchemaObject.h"
#include "CurveBuildProperties.h"
#include "InterpolationParameters.h"
#include "CoreEnumerations.h"
#include "ObjectUtilities.h"


using rapidjson::Document;
using rapidjson::Value;
using rapidjson::SizeType;
using rapidjson::StringRef;

#include <iostream>


// #define SK_NAME toString(TABLE)

namespace
{
    const char*  SK_NAME = "SK_NAME";
    const char*  SK_NUMBER_OF_COLUMNS = "SK_NUMBER_OF_COLUMNS";
    const char*  SK_COLUMN_TYPES = "SK_COLUMN_TYPES";
}

namespace etrading
{

    CurveBuildProperties createCurveBuildPropertiesFromJsonArray( const Value& skVariableValuesArray,  const rapidjson::Value* interpolationParameters, const std::string& cbp_name )
    {
        // TODO: assumed order of  ( "CurveType" )( "CurveCollectionName" )( "CurveIndexName" )
        // ( "CCY" )( "AsOfDate" )( "InterpolationMethod" )("FloatingRateCompoundingFrequency")
        // ("CurveTenor")("OISCompoundingMethod")("BusinessDayAdjustment")("Calendar") - perhaps don't do this?
        const CurveTypeEnum curveType = toCurveTypeEnum( skVariableValuesArray[0].GetString() ) ;
        const std::string curveCollectionName = skVariableValuesArray[1].GetString();
        const std::string curveIndexName = skVariableValuesArray[2].GetString();
        const CCY ccy = toCCYEnum( skVariableValuesArray[3].GetString() );
        const boost::gregorian::date& asOfDate = boost::gregorian::from_undelimited_string( std::string( skVariableValuesArray[4].GetString() ).c_str() );
        const InterpolationEnum interpMethod = toInterpolationEnum( skVariableValuesArray[5].GetString() );
        const CompoundingFrequencyEnum floatRateCompoundingFreq = toCompoundingFrequencyEnum( skVariableValuesArray[6].GetString() );
        const CurveTenorEnum floatRateTenor = toCurveTenorEnum( skVariableValuesArray[7].GetString() );
        const CompoundingMethodEnum oisCompoundMethod = toCompoundingMethodEnum( skVariableValuesArray[8].GetString() );
        const BusinessDayAdjustmentEnum busDayAdjustment = toBusinessDayAdjustmentEnum( skVariableValuesArray[9].GetString() );
        const std::string calendarString = skVariableValuesArray[10].GetString() ;
        // std::cout << skVariableValuesArray[11].GetString()  << std::endl;
        const bool onlyAllowLookup = skVariableValuesArray[15].GetBool() ;  // the calendar is duplicated 3 times...

        
        return CurveBuildProperties( curveType,  cbp_name, curveCollectionName, curveIndexName, ccy, asOfDate, interpMethod,
			floatRateCompoundingFreq, floatRateTenor, oisCompoundMethod, busDayAdjustment, calendarString, {} , onlyAllowLookup );
    };

    std::vector<double> populateDoubleVector( const rapidjson::Value& skDoubleArray )
    {
        std::vector<double> discountFactors;
        for ( SizeType skCounter = 0; skCounter < skDoubleArray.Size(); skCounter++  )
        {
            const double value = skDoubleArray[skCounter].GetDouble();
            discountFactors.push_back( value );
        }
        return discountFactors;
    };

    std::vector<boost::gregorian::date> populateDateVector( const rapidjson::Value& skDateArray )
    {
        std::vector<boost::gregorian::date> dates;
        for ( SizeType skCounter = 0; skCounter < skDateArray.Size(); skCounter++ )
        {
            const std::string dateStr = skDateArray[skCounter].GetString();
            dates.emplace_back( boost::gregorian::from_undelimited_string( dateStr.c_str() ) );
        }
        return dates;
    };

    std::vector<std::string> populateStringVector( const rapidjson::Value& skStringArray )
    {
        std::vector<std::string> stringVec;
        for ( SizeType skCounter = 0; skCounter < skStringArray.Size(); skCounter++ )
        {
            const std::string str = skStringArray[skCounter].GetString();
            stringVec.emplace_back( str );
        }
        return stringVec;
    };

    std::string createJSONStringFromDataSchemas( const SchemaObject& co )
    {
        std::string startingString = "[";
        const std::vector<DataSchema> skSnapshot = co.snapshotOfSchemas();
        if( !skSnapshot.empty() )
        {
            for( unsigned int skCounter = 0u; skCounter < skSnapshot.size(); skCounter++ )
            {
                const DataSchema& sk = skSnapshot[ skCounter ];
                std::string jsonString = serializeDataSchemaToJSONString( sk, co.viewAllData(), co.columnsOfSchema( sk.getName() ) );
                if(  ( skCounter + 1 ) < skSnapshot.size() )
                {
                    jsonString += ",";
                }
                startingString += jsonString;
            }
        }
        startingString += " ] ";
        return startingString;
    };

    std::string createJSONStringFromSchemaObject(	const SchemaObject& co, bool addClosure, bool allowRecursive )
    {
        const std::string enumTypeAsString = toString( co.getEnumType() );
        std::string startingString = ( boost::format( "{\"CACHED_OBJECT_TYPE\" : \"%s\" , \"STRUCTURED_KEYS\" : " ) % enumTypeAsString ).str();
        startingString += allowRecursive ? co.serialize( serialize::JSON, serialize::STRING ) : createJSONStringFromDataSchemas( co );
        if( addClosure )
        {
            startingString += "}";
        }
        return startingString;
    }

    rapidjson::Document createJSONFromString( const std::string& inputData )
    {
        // the smallest valid JSON object is "{}"
        if(inputData.length()<=2)
        {
            throw ETradingException( std::string( "#Error: Unable to interpret file as JSON : " ) + inputData );
        }
        rapidjson::Document document;
        if( document.Parse<0>( inputData.c_str() ).HasParseError() )
        {
            throw ETradingException( std::string( "#Error: Unable to interpret file as JSON : " ) + inputData );
        }
        if( !document.IsObject() ) // the document instance here is the dataInstance of the DOM model
        {
            throw ETradingException( std::string( "#Error: Incomplete or Invalid JSON format in string: " ) + inputData );
        }
        return document;
    }

    rapidjson::Document createJSONFromFile( const std::string& fileName )
    {
        if( !fileExists( fileName ) )
        {
            throw ETradingException( std::string( "#Error: Unable to read JSON data - File not found " ) + fileName );
        }
        std::ifstream inputFileStream( fileName.c_str() );
        std::stringstream stringBuffer;
        stringBuffer << inputFileStream.rdbuf();
        std::string inputData = stringBuffer.str();
        return createJSONFromString( inputData );
    };


    rapidjson::Value VariantToJSONValue( const Variant& cv, rapidjson::Document::AllocatorType& allocator )
    {
        ContainedTypeEnum enumType = cv.getType();
        if( cv.getType() == DATE_VALUE )
        {
            return Value( StringRef( toYYYYMMDDFromGregorianDate( cv.getValue<boost::gregorian::date>() ).c_str() ), allocator );
        }
        else if( cv.getType() == BOOL_VALUE )
        {
            return Value( cv.getValue<bool>() );
        }
        else if( cv.getType() == STRING_VALUE )
        {
            return Value( StringRef( cv.getValue<std::string>().c_str() ), allocator );
        }
        else if( cv.getType() == DOUBLE_VALUE )
        {
            return Value( cv.getValue<double>() );
        }
        else if( cv.getType() == INTEGER_VALUE )
        {
            return Value( cv.getValue<int>() );
        }
        else if( cv.getType() == EMPTY_VALUE )
        {
            // Variant treats EMPTY as an empty string, see etrading::Variant.cpp
            return Value( StringRef( cv.getValue<std::string>().c_str() ), allocator );
        }
        else
        {
            // If unknown type treat as a string type
            return Value( StringRef( cv.getValue<std::string>().c_str() ), allocator );
        }
    }

    // TODO: add horizontal (by row) processing instead of by column?
    std::string serializeDataSchemaToJSONString( const DataSchema& sk, const ContainedDataType& cd, const std::vector<int>& relevantIdxs )
    {
        // check relevant index match with type and index
        if( sk.getNumberOfColumns() != relevantIdxs.size() )
        {
            throw ETradingException( "#Error: Number of columns does not match serialization DataSchema" );
        }

        Document skDoc;
        skDoc.SetObject();
        Document::AllocatorType& allocator = skDoc.GetAllocator();

        // go through structuredKey
        Value skName;
        skName.SetString( StringRef( sk.getName().c_str() ) );
        skDoc.AddMember( StringRef(SK_NAME) , skName, allocator );

        Value skNumberOfColumns;
        skNumberOfColumns.SetInt( sk.getNumberOfColumns() );
        skDoc.AddMember( StringRef(SK_NUMBER_OF_COLUMNS), skNumberOfColumns, allocator );

        Value skColumnTypes( rapidjson::kArrayType );
        for( int colCounter = 0; colCounter < sk.getNumberOfColumns(); colCounter++ )
        {
            skColumnTypes.PushBack( Value( StringRef( toString( sk.getColumnType( colCounter ) ).c_str() ), allocator ), allocator );
        }
        skDoc.AddMember( StringRef(SK_COLUMN_TYPES), skColumnTypes, allocator );

        std::vector<std::string> columnNames;
        if( sk.hasColumnNames() )
        {
            columnNames.assign( sk.getColumnNamesRef().cbegin(), sk.getColumnNamesRef().cend() );
        }
        else
        {
            std::vector<std::string> cNames( sk.getNumberOfColumns(), "COLUMN_" );
            std::vector<int> nColCounters;
            boost::push_back( nColCounters, boost::irange( 0, sk.getNumberOfColumns() ) );
            columnNames = zip_paste<std::vector<std::string>, std::vector<int>, std::string>( cNames, nColCounters );
        }

        for( unsigned int colCounter = 0u; colCounter < columnNames.size(); colCounter++ )
        {
            ContainedTypeEnum columnType = sk.getColumnType( colCounter );
            auto heldData = cd[ relevantIdxs[ colCounter ] ];
            // add array values
            Value columnAsArray( rapidjson::kArrayType );
            for( auto dataIterator = heldData.cbegin(); dataIterator != heldData.cend(); dataIterator++ )
            {
                columnAsArray.PushBack( VariantToJSONValue( *dataIterator, allocator ), allocator );
            }
            skDoc.AddMember( StringRef( columnNames[ colCounter ].c_str() ), columnAsArray, allocator );
        }

        return createStringFromJSON( skDoc );
    };

    std::vector<Variant> createVariantVector( const rapidjson::Value& inputValue, const etrading::ContainedTypeEnum targetType )
    {
        if( !inputValue.IsArray() )
        {
            throw ETradingException( "#Error: Unable to serialize a scalar value into a vector" );
        }

        std::vector<Variant> outVector( 0 );
        for ( SizeType valCounter = 0; valCounter < inputValue.Size(); valCounter++ ) // Uses SizeType instead of size_t
        {
            if( targetType == INTEGER_VALUE )
            {
                outVector.push_back( inputValue[valCounter].GetInt() );
            }
            if( targetType == DOUBLE_VALUE )
            {
                outVector.push_back( inputValue[valCounter].GetDouble() );
            }
            if( targetType == STRING_VALUE )
            {
                outVector.emplace_back( std::string( StringRef( inputValue[valCounter].GetString() ) ) );
            }
            if( targetType == EMPTY_VALUE )
            {
                outVector.emplace_back( std::string( StringRef( inputValue[valCounter].GetString() ) ) );
            }
            if( targetType == DATE_VALUE )
            {
                outVector.emplace_back( boost::gregorian::from_undelimited_string( inputValue[valCounter].GetString() ) );
            }
            if( targetType == VARIANT_VALUE )
            {
                if( inputValue[valCounter].IsString() )
                {
                    outVector.push_back( std::string( StringRef( inputValue[valCounter].GetString() ) ) );
                }
                else if( inputValue[valCounter].IsNumber() )
                {
                    if( inputValue[valCounter].IsDouble() )
                    {
                        outVector.push_back( inputValue[valCounter].GetDouble() );
                    }
                    else
                    {
                        outVector.push_back( inputValue[valCounter].GetInt() );
                    }
                }
            }
        }
        return outVector;
    }

    std::pair<DataSchema, std::vector<std::vector<Variant>>> createDataSchemaWithDataFromJSON( const Value& inputValue )
    {
        try
        {
            checkValidDataSchema(inputValue);

            const std::string keyName =  std::string( inputValue[SK_NAME].GetString() );
            const int numberOfColumns =  inputValue[SK_NUMBER_OF_COLUMNS].GetInt();
            std::vector<etrading::ContainedTypeEnum> columnTypes;
            std::vector<std::string> columnNames;

            std::vector<std::vector<Variant>> values;

            if( numberOfColumns <= 0 )
            {
                throw ETradingException( ( boost::format( "#Error: No data provided to create DataSchema '%s'" ) % keyName ).str() );
            }
            else
            {
                Value::ConstMemberIterator itr = inputValue.FindMember( SK_COLUMN_TYPES );
                if ( itr == inputValue.MemberEnd() )
                {
                    throw ETradingException( std::string( "#Error: Unable to determine the column type(s) for DataSchema" ) + keyName );
                }

                const Value& columnTypesInJSON = inputValue[SK_COLUMN_TYPES];

                std::vector<int> colCounters;
                boost::push_back( colCounters, boost::irange( 0, numberOfColumns ) );
                std::for_each( colCounters.cbegin(), colCounters.cend(),
                               [&itr, &columnNames, &columnTypes, &columnTypesInJSON, &inputValue, &values]
                               ( const int columnIdx )
                {
                    ++itr;
                    
                    // Column Names
                    std::string columnNameStr = itr->name.GetString();
                    columnNames.emplace_back( columnNameStr );
                    std::string columnTypeStr = columnTypesInJSON[columnIdx].GetString();
                    // boost::trim(columnTypeStr); // causes VS2010 compiler crash... TODO: move to C++11
                    
                    // Column Types
                    ContainedTypeEnum colType = toContainedTypeEnum( columnTypeStr );
                    columnTypes.emplace_back( colType );
                    
                    // Column Data
                    std::vector<Variant> valuesInThisColumn = createVariantVector( inputValue[columnNameStr.c_str()], colType );
                    values.emplace_back( valuesInThisColumn );
                }
                             );

                DataSchema skToReturn( keyName, numberOfColumns, columnTypes, columnNames );
                return std::make_pair( skToReturn, values );
            }
        }
        catch ( ETradingException& etx )
        {
            throw etx;
        }
        catch ( std::exception& ex )
        {
            throw ex;
        }
        catch ( ... )
        {
            throw ETradingException( "#Error: Invalid Data Schema within rapidjson file" );
        }
        throw ETradingException( "#Error: Invalid Data Schema within rapidjson file" );

    };

    // TODO: really? template this ASAP
	std::string createStringFromJSON( const rapidjson::Document& doc )
    {
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );
        doc.Accept( writer );
        return std::string( buffer.GetString(), buffer.GetSize() );
    };

    std::string createStringFromJSONValue( const rapidjson::Value& val )
    {
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer( buffer );
        val.Accept( writer );
        return std::string( buffer.GetString(), buffer.GetSize() );
    }



}