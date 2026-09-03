// JSONInfoBlock.cpp

/*
 * @brief			Class to Manage the JSON InfoBlock Container for LWO Serialization
 * @Created:		24th September 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub, Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "JSONInfoBlock.h"
#include "DataUtilities.h"
#include "ExceptionMacros.h"

namespace etrading
{
    // Alternative Constructor
    JSONInfoBlock::JSONInfoBlock(const RawInputData & rawInputData )
    {
        // Store the Raw Input Data
        rawInputData_           = rawInputData;

        // Create a Cleansed JSON Info Block Data Container
        jsonInfoBlockTuple_     = JSONInfoBlock::createInfoBlock( rawInputData );

        // Store Tuple Components so client can access them safely, without needing to know the tuple order / magic number info
        // The underlying tuple is ugly and I hate the magic number references to data container components
        columnNames_            = std::get<0>( jsonInfoBlockTuple_ );
        columnDataTypes_        = std::get<1>( jsonInfoBlockTuple_ );
        jsonFormattedData_      = std::get<2>( jsonInfoBlockTuple_ );
    }

    // Copy Constructor
    JSONInfoBlock::JSONInfoBlock( const JSONInfoBlock & rhs ) 
        : columnNames_( rhs.columnNames_ ), columnDataTypes_( rhs.columnDataTypes_ ), rawInputData_( rhs.rawInputData_ ), jsonFormattedData_( rhs.jsonFormattedData_ ), jsonInfoBlockTuple_( rhs.jsonInfoBlockTuple_ )
    {
    }
    
    // Assignment Operator
    JSONInfoBlock & JSONInfoBlock::operator=( const JSONInfoBlock & rhs )
    {
        // For Exception Safety
            
        // 1. Make a temp copy
        JSONInfoBlock temp( rhs );

        // 2. Swap Data Members with the temp copy

        // 2A. Input Parameters
        std::swap( columnNames_,            temp.columnNames_      );
        std::swap( columnDataTypes_,        temp.columnDataTypes_     );
        std::swap( rawInputData_,           temp.rawInputData_       );
        std::swap( jsonFormattedData_,      temp.jsonFormattedData_  );
        std::swap( jsonInfoBlockTuple_,     temp.jsonInfoBlockTuple_ );

        return *this;
    }
    

    // ------------------------------------------------------------------------------------------------------------
    //
    // Static Helper Methods
    //
    // ------------------------------------------------------------------------------------------------------------


    /* @brief			Function to trim and remove blank rows from LWO Object Information Blocks
    *  @param [in]		infoBlockNames		    A vector of info block names
    *  @param [in]		infoBlocks		        A vector of info blocks
    *  @param [in]		checkForErrors		    Parameter to control throw if an error is found, default is true
    *  @return			Returns a LWO Object information blocks with empty data rows removed
    */ 
    JSONInfoBlockTuples JSONInfoBlock::trimInfoBlocks( const std::vector<std::string>& infoBlockNames,
                                                       const JSONInfoBlockTuples& infoBlocks,
                                                       const bool checkForErrors,
                                                       const bool enforceVariantDataType )
    {
        // Access Violation Guard
        MLIB_REQUIRE( infoBlockNames.size() == infoBlocks.size(), "Invalid JSON Data InfoBlock - Inconsistent number of data block names and data blocks." )
        
        // Clean Market Data - Remove Blanks
        JSONInfoBlockTuples cleansedInfoBlocks = infoBlocks;
        for( unsigned int i = 0; i < infoBlockNames.size(); i++ )
		{
            cleansedInfoBlocks[ i ] = trimInfoBlock( infoBlocks[i], infoBlockNames[i], checkForErrors, enforceVariantDataType );
		}

        return cleansedInfoBlocks;
    }


    /* @brief			Function to trim and remove blank rows from a single LWO Object Information Block
    *  @param [in]		infoBlocks		        A vector of info blocks
    *  @param [in]		checkForErrors		    Parameter to control throw if an error is found, default is true
    *  @param [in]		enforceVariantDataType  Force infoBlock to have Varaint type, default is true
    *  @return			Returns a LWO Object information blocks with empty data rows removed
    */ 
    JSONInfoBlockTuple JSONInfoBlock::trimInfoBlock( const JSONInfoBlockTuple& infoBlock,
                                                     const bool checkForErrors,
                                                     const bool enforceVariantDataType )
    {
        return trimInfoBlock( infoBlock, "InfoBlockData", checkForErrors, enforceVariantDataType );
    }

    /* @brief			Function to trim and remove blank rows from a single LWO Object Information Block
    *  @param [in]		infoBlock		        A vector of info blocks
    *  @param [in]		infoBlockName	        An optional descriptive name for the info block
    *  @param [in]		checkForErrors		    Parameter to control throw if an error is found, default is true
    *  @param [in]		enforceVariantDataType  Force infoBlock to have Varaint type, default is true
    *  @return			Returns a LWO Object information blocks with empty data rows removed
    */ 
    JSONInfoBlockTuple JSONInfoBlock::trimInfoBlock( const JSONInfoBlockTuple & infoBlock,
                                                     const std::string & infoBlockName,
                                                     const bool checkForErrors, 
                                                     const bool enforceVariantDataType )
    {
        JSONInfoBlockTuple cleansedInfoBlock;

        // *** IMPORTANT NOTE: Removing Blanks can change the column type enum ***
        // -----------------------------------------------------------------------
		// Raw data containing blanks and doubles considered as Variant Type, when removing blanks the serialization type will become double
        // Therefore when cleansing / triming blanks we must reset the column types
            
        // 1. Get the raw market data block
        const VariantMatrix& rawMarketData = std::get<2>( infoBlock );
			
        // 2. Remove Blanks and Update the Types
        const VariantMatrix& cleansedMarketData = etrading::removeEmptyColumns( rawMarketData ); 
        
        // Update types but enforce Variant type if requested
        ColumnDataTypes cleansedColumnEnumTypes( cleansedMarketData.size(), etrading::VARIANT_VALUE );
        if ( !enforceVariantDataType )
        {
            cleansedColumnEnumTypes = etrading::Variant::getContainedTypeInfo( cleansedMarketData );
        }

        // 3. Throw if an error is found in the data block
        if ( checkForErrors )
        {
            etrading::checkDataMatrixForErrors( cleansedMarketData, infoBlockName );
        }

        // 3. Create Cleansed InfoBlock Tuple
        cleansedInfoBlock = std::make_tuple( std::get<0>( infoBlock ), cleansedColumnEnumTypes, cleansedMarketData );
        return cleansedInfoBlock;
    }


    
    JSONInfoBlockTuple JSONInfoBlock::createInfoBlock( const VariantMatrix & dataMatrix, const bool enforceVariantDataType )
    {
        MLIB_REQUIRE( dataMatrix.size() > 0,    "Invalid Data: InfoBlock Data Matrix is Empty" )
        MLIB_REQUIRE( dataMatrix[0].size() > 0, "Invalid Data: InfoBlock Data Matrix is Empty" )
        
        const size_t nRows = dataMatrix.size();
        const size_t nColumns = dataMatrix[0].size();
        
        // Set the Column Names as COL1, COL2, COL3 etc ...
        ColumnNames columnNames( nColumns );
        for( size_t i = 0; i < nColumns; ++i )
        {
            // Columns have base 1, so we use index i+1
            columnNames[i] = "COL_" + std::to_string( static_cast<long long>(i+1) );
        }
        
        // Set the Column Data Types as Variant or Inspect Columns and Derive Types for Backwards compatibility )
        // Note: We can always set the type as variant, specific types only needed for backwards compatibility
        ColumnDataTypes columnDataTypes( columnNames.size(), etrading::VARIANT_VALUE );
        if ( !enforceVariantDataType )
        {
            // Inspect and Derive Column Data Type Enum(s) if Required
            columnDataTypes = etrading::Variant::getContainedTypeInfo( dataMatrix );
        }

        // *** Important - Transpose Column Data to Row Data ***
        // Data must be transposed from Columns to Rows in LWO JSON Format
        // ----------------------------------------------------------------------
        VariantMatrix dataMatrixTransposedByRow;
        for( size_t j = 0; j < nColumns; ++j )
        {
            VariantVector dataInColumn;
            for( size_t i = 0; i < nRows; ++i )
            {
                dataInColumn.push_back( etrading::Variant( dataMatrix[i][j] ).getValueAsString() ); // force conversion to std::string
            }
            dataMatrixTransposedByRow.push_back( dataInColumn );
        }
        // ----------------------------------------------------------------------

        // Create and Return a Cleansed InfoBlock
        const JSONInfoBlockTuple rawInfoBlock        = std::make_tuple( columnNames, columnDataTypes, dataMatrixTransposedByRow );
        const JSONInfoBlockTuple cleansedInfoBlock   = trimInfoBlock( rawInfoBlock, true, enforceVariantDataType ); // CheckForErrors = true
        
        return cleansedInfoBlock;
    }
}