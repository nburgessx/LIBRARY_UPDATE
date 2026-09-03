#include "CurveMarketData.h"
#include "ParameterValidation.h"
#include "ObjectUtilities.h"
#include "DataUtilities.h"
#include "CommonConstants.h"
#include "CurveInstruments.h"	// required for isARRFuture method checks if an OIS or ARR swap instrument is actually a future from it's term string
#include "DataUtilities.h"		// required for MLIB_TO_STRING_FROM_SIZE_T macro

#include <vector>
#include <string>
#include <boost/format.hpp>		// iequals case insensitive string comparison


namespace etrading
{
	/* @brief Main Constructor
	 * @param[in] objectName      The name of this CurveMarketData instance
	 * @param[in] marketDataKeys  A vector containing the names of each marketData block
	 * @param[in] infoBlocks      A vector containing the marketData blocks
	 */
	CurveMarketData::CurveMarketData( const std::string& objectName,
									  const std::vector<std::string>& marketDataKeys,
									  const std::vector<TableInfo>& infoBlocks ) 
					   : IsLWOObject(objectName, CURVE_MARKETDATA), 
						 freeObject_(objectName),
						 variantMarketDataLocalCache_(),
						 stringMarketDataLocalCache_()
	{
		MLIB_REQUIRE( marketDataKeys.size() == infoBlocks.size(), "Invalid Market Data: Inconsistent number of market data keys and values")

		// Create a FreeObject from each property label-value block, and concatenate
		const bool allowJaggedData = false;
        for( unsigned int gridCounter = 0; gridCounter < infoBlocks.size(); gridCounter++ )
        {
			// Info Block Tuple
			// Element 0:	Column Names
			// Element 1:	Column Types
			// Element 2:	Variant Matrix Data
			const std::vector<std::string>& columnNames = std::get<0>( infoBlocks[ gridCounter ] );
            const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>( infoBlocks[ gridCounter ]) ;
            const VariantMatrix& rawMarketData = std::get<2>( infoBlocks[ gridCounter ] );

			// Remove Unused Market Data, which is determined by the "USE" market data column parameter
			const VariantMatrix cleansedMarketData = checkDimensionsAndRemoveUnusedData( marketDataKeys[gridCounter], rawMarketData );

            // Skip Empty Data Blocks
            // ----------------------
            if ( cleansedMarketData.size() == 0 )
            {
                continue;
            }
            
            freeObject_ += createFreeObjectFromGrid( objectName, columnNames, colTypes, cleansedMarketData, marketDataKeys[ gridCounter ], allowJaggedData );
        }
	}

	/* @brief Constructor used by deserialization
	 * @param[in] objectName    The name of this CurveMarketData instance
	 * @param[in] freeObject    A freeObject constructed from the serialized data
	 */
	CurveMarketData::CurveMarketData( const std::string& objectName,
									  const FreeObject& freeObject ) 
						: IsLWOObject(objectName, CURVE_MARKETDATA), 
						  freeObject_(freeObject),
						  variantMarketDataLocalCache_(),
						  stringMarketDataLocalCache_()
	{
    }

	/* @brief Copy Constructor
	 */
	CurveMarketData::CurveMarketData(const CurveMarketData& rhs) 
		  :	IsLWOObject(rhs.getRefToName(), CURVE_MARKETDATA), 
			freeObject_(rhs.freeObject_),
			variantMarketDataLocalCache_(rhs.variantMarketDataLocalCache_),
			stringMarketDataLocalCache_(rhs.stringMarketDataLocalCache_)
	{
	}

    std::shared_ptr<CurveMarketData> CurveMarketData::clone() const
    {
       auto data = std::make_shared<CurveMarketData>(*this);
       return data;
    }

	/* @brief Used to serialize an instance of this class
	 * @param[out] the populated SchemaObject
	 */
	const SchemaObject CurveMarketData::toSchemaObject() const
	{
		SchemaObject schemaObject( CURVE_MARKETDATA, getRefToName());
        
        toSchemaObject(schemaObject);

        return schemaObject;
	}

     //This is a helper method which is not in the abstract class:
    void const CurveMarketData::toSchemaObject(SchemaObject& schemaObject) const
    {
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();
		const int nSchemas = freeObject_.numberOfSchemas();
		for (int i=0; i<nSchemas; i++)
		{
			schemaObject.addDataSchema(freeObject_.viewSchema(i));

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema( propertyName );
			VariantMatrix variantMatrix;
			std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView]( const int idx )
            {
				variantMatrix.push_back( allDataView[ idx ] );
            } );

			schemaObject.setDataForSchema(propertyName, variantMatrix);
		}
	}

    // ===============================================================================================================

    // *** Helper Method to provide the Market Data Rate Column to Bump ***
    // Note: We return column number as -1 to indicate there is no rate column
    int findMarketDataRateColumnNumber_ForOutrightInstrumentsOnly( const CurveMarketDataEnum & marketDataType )
    {
        // Note: Using dimension base zero here
        switch ( marketDataType )
        {
            case OIS_MARKETDATA:                    return 1;   // i.e. Column 2
            case SWAP_MARKETDATA:                   return 1;   // i.e. Column 2
            case LIBOR_FIXING_TABLE:                return 1;   // i.e. Column 2
            case FUTURES_MARKETDATA:                return 3;   // i.e. Column 4
            case FRA_MARKETDATA:                    return 1;   // i.e. Column 2
            case FXFWD_MARKETDATA:                  return 1;   // i.e. Column 2
            case FXSPOT_MARKETDATA:                 return 2;   // i.e. Column 3 
            default:                                return -1;  // Use -1 to indicate don't bump the market data object or no Rate column to bump
        }
    }

    // *** Helper Method to provide the Market Data Rate Column to Bump ***
    // Note: We return column number as -1 to indicate there is no rate column
    int CurveMarketData::findMarketDataRateColumnNumber_ForAllInstrumentTypes( const CurveMarketDataEnum & marketDataType )
    {
        // Note: Using dimension base zero here
        switch ( marketDataType )
        {
            case OIS_MARKETDATA:                    return 1;   // i.e. Column 2
            case LIBOR_OIS_BASISSPREAD_MARKETDATA:  return 1;   // i.e. Column 2
            case SWAP_MARKETDATA:                   return 1;   // i.e. Column 2
            case LIBOR_FIXING_TABLE:                return 1;   // i.e. Column 2
            case FUTURES_MARKETDATA:                return 3;   // i.e. Column 4
            case FRA_MARKETDATA:                    return 1;   // i.e. Column 2
            case BASIS_SWAP_MARKETDATA:             return 1;   // i.e. Column 2
            case XCCY_SWAP_MARKETDATA:              return 1;   // i.e. Column 2
            case FXFWD_MARKETDATA:                  return 1;   // i.e. Column 2
            case FXSPOT_MARKETDATA:                 return 2;   // i.e. Column 3 
            default:                                return -1;  // Use -1 to indicate don't bump the market data object or no Rate column to bump
        }
    }

	// *** Helper Method to provide the Market Data Rate Column to Bump ***
    // Note: We return column number as -1 to indicate there is no rate column
    int findMarketDataTenorColumnNumber_ForOutrightInstrumentsOnly( const CurveMarketDataEnum & marketDataType )
    {
		// Note: Using dimension base zero here
		const size_t column1 = 0;

        switch ( marketDataType )
        {
            case OIS_MARKETDATA:                    return column1;
            case SWAP_MARKETDATA:                   return column1;
            case LIBOR_FIXING_TABLE:                return column1;
            case FUTURES_MARKETDATA:                return column1;
            case FRA_MARKETDATA:                    return column1;
            case FXFWD_MARKETDATA:                  return column1;
            case FXSPOT_MARKETDATA:                 return column1;
            default:                                return -1;  // Use -1 to indicate don't bump the market data object or no Rate column to bump
        }
    }

	// *** Helper Method to provide the Market Data Rate Column to Bump ***
    // Note: We return column number as -1 to indicate there is no rate column
    int findMarketDataTenorColumnNumber_ForAllInstrumentTypes( const CurveMarketDataEnum & marketDataType )
    {
		// Note: Using dimension base zero here
		const size_t column1 = 0;

        switch ( marketDataType )
        {
            case OIS_MARKETDATA:                    return column1;
            case LIBOR_OIS_BASISSPREAD_MARKETDATA:  return column1;
            case SWAP_MARKETDATA:                   return column1;
            case LIBOR_FIXING_TABLE:                return column1;
            case FUTURES_MARKETDATA:                return column1;
            case FRA_MARKETDATA:                    return column1;
            case BASIS_SWAP_MARKETDATA:             return column1;
            case XCCY_SWAP_MARKETDATA:              return column1;
            case FXFWD_MARKETDATA:                  return column1;
            case FXSPOT_MARKETDATA:                 return column1;
            default:                                return -1;  // Use -1 to indicate don't bump the market data object or no Rate column to bump
        }
    }

    // ===============================================================================================================


    /* @brief Static Helper Method to bump the marketData for the specified MarketDataStringMatrix, bumpSize and marketDataKey 
    * @param [in/out]   marketDataStringMatrix          The market data to be bumped - input the original market data to get bumped market data as output
    * @param [in]       bumpSize                        Enter the bump or shift size to apply
    * @param [in]       marketDataType                  The marketData key or type to be updated
	*/
    void CurveMarketData::bumpMarketDataFromLAStringMatrix( LAStringMatrix & marketDataStringMatrix, const double bumpSize, const CurveMarketDataEnum & marketDataType )
    {
        // Note: Use Base Zero Here for Native C++ Vector / Matrix Lookups
        // ================================================================

        // 1. Do nothing if the market data block is empty
        if ( marketDataStringMatrix.empty() )
        {
            return;
        }

        // 2. Check if Market Data Contains the Rate Column
        const int marketDataRateColumn  = findMarketDataRateColumnNumber_ForAllInstrumentTypes( marketDataType );
        
        // The bumpMarketDataRateColumnNumber method will return -1 when there is no Rate column in the market data block 
        const bool doesMarketDataContainRateColumn = marketDataRateColumn > -1 ? true : false; 

        // 3. Bump the Market Data
        if ( doesMarketDataContainRateColumn )
        {
            for (size_t i = 0; i < marketDataStringMatrix.size(); i++)
		    {
                // Check the Market Data Matrix Dimensions and Market Data Rate Data Type
                if ( i == 0 )
                {
                    MLIB_REQUIRE( marketDataStringMatrix[0].size() >= size_t(marketDataRateColumn), "Invalid Market Data: There are not enough columns in the market data block '"
                        + toString( marketDataType ) + "'. Need at least " + MLIB_TO_STRING_FROM_INT( marketDataRateColumn ) + " data columns." )
                }
                
                const double originalMarketData = marketDataStringMatrix[i][marketDataRateColumn].getDoubleValue();
                double bumpedMarketData;

                if ( marketDataType == FUTURES_MARKETDATA )
                {
                    // Futures Formulae: 
                    // RateFromFutureInPercent      = ( 100 - FuturePrice ) / 100
                    // FuturePrice                  = 100 - ( RateFromFutureInPercent * 100 )
                    double rateFromFuture = ( 100.0 - originalMarketData ) / 100.0;
                    rateFromFuture += bumpSize;
                    bumpedMarketData = 100.0 - ( rateFromFuture * 100.0 );
                }
                else
                {
                    bumpedMarketData = originalMarketData + bumpSize;
                }

                // Update Market Data Row
                // Market Data in the Entity Pool is stored as a string so use high precision when casting from double to string
                marketDataStringMatrix[i][marketDataRateColumn] = MLIB_TO_STRING_FROM_DOUBLE_WITH_PRECISION(bumpedMarketData,20).c_str();
		    }
        }
        else
        {
            // Market Data Rate Column not Found - or market data block bumping not permitted
            // =================================
            MLIB_THROW( "Invalid Market Data: Cannot apply a bump or shift to market data of type '" + toString( marketDataType ) + "'" )
            
        }
    }
    
	/* @brief Static Helper Method to bump the nth marketData instrument for the specified MarketDataStringMatrix, bumpSize and marketDataKey 
    * @param [in/out]   marketDataStringMatrix          The market data to be bumped - input the original market data to get bumped market data as output
    * @param [in]       nthInstrumentToBump             Enter the nth instrument to bump using base zero
	* @param [in]       bumpSize                        Enter the bump or shift size to apply
    * @param [in]       marketDataType                  The marketData key or type to be updated
	*/
	void CurveMarketData::perturbMarketDataFromLAStringMatrix( LAStringMatrix & marketDataStringMatrix, size_t nthInstrumentToBump, const double bumpSize, const CurveMarketDataEnum & marketDataType )
	{
		// Note: Use Base Zero Here for Native C++ Vector / Matrix Lookups
        // ================================================================

        // 1. Throw if the market data block is empty
		MLIB_REQUIRE( !marketDataStringMatrix.empty(), "Unble to Perturb '" + toString( marketDataType) + "' Market Data: The Market Data Block is Empty" )
        
        // 2. Check if Market Data Contains the Rate Column
        const int marketDataRateColumn  = findMarketDataRateColumnNumber_ForAllInstrumentTypes( marketDataType );
        
        // The bumpMarketDataRateColumnNumber method will return -1 when there is no Rate column in the market data block 
        const bool doesMarketDataContainRateColumn = marketDataRateColumn > -1 ? true : false; 

        // 3. Perturb the nth Market Data instrument
        if ( doesMarketDataContainRateColumn )
        {
			MLIB_REQUIRE( nthInstrumentToBump < marketDataStringMatrix.size(), "Unable to Perturb Instrument Number " + MLIB_TO_STRING_FROM_SIZE_T( nthInstrumentToBump + 1u )
						  + " , since Market Data Type '" + toString( marketDataType) + "' contains only " + MLIB_TO_STRING_FROM_SIZE_T( marketDataStringMatrix.size() ) + "instruments" )


			// Check the Market Data Matrix Dimensions and Market Data Rate Data Type
            MLIB_REQUIRE( marketDataStringMatrix[nthInstrumentToBump].size() >= size_t( marketDataRateColumn ), "Invalid Market Data: There are not enough columns in the market data block '"
                          + toString( marketDataType ) + "'. Need at least " + MLIB_TO_STRING_FROM_INT( marketDataRateColumn ) + " data columns." )
                
            const double originalMarketData = marketDataStringMatrix[nthInstrumentToBump][marketDataRateColumn].getDoubleValue();
            double bumpedMarketData;

            if ( marketDataType == FUTURES_MARKETDATA )
            {
                // Futures Formulae: 
                // RateFromFutureInPercent      = ( 100 - FuturePrice ) / 100
                // FuturePrice                  = 100 - ( RateFromFutureInPercent * 100 )
                double rateFromFuture = ( 100.0 - originalMarketData ) / 100.0;
                rateFromFuture += bumpSize;
                bumpedMarketData = 100.0 - ( rateFromFuture * 100.0 );
            }
            else
            {
                bumpedMarketData = originalMarketData + bumpSize;
            }

            // Update Market Data Row
            // Market Data in the Entity Pool is stored as a string so use high precision when casting from double to string
            marketDataStringMatrix[nthInstrumentToBump][marketDataRateColumn] = MLIB_TO_STRING_FROM_DOUBLE_WITH_PRECISION(bumpedMarketData,20).c_str();
        }
        else
        {
            // Market Data Rate Column not Found - or market data block bumping not permitted
            // =================================
            MLIB_THROW( "Invalid Market Data: Cannot perturb market data of type '" + toString( marketDataType ) + "'" )
            
        }
	}

	/* @brief Static Helper Method to get the Tenor for the marketData instrument for the specified MarketDataStringMatrix
    * @param [in/out]   marketDataStringMatrix          The market data to be bumped - input the original market data to get bumped market data as output
    * @param [in]       marketDataType                  The marketData key or type to be updated
	*/
	StandardStringVector CurveMarketData::getMarketDataTenorsFromLAStringMatrix( const LAStringMatrix & marketDataStringMatrix, const CurveMarketDataEnum & marketDataType )
	{
		// Note: Use Base Zero Here for Native C++ Vector / Matrix Lookups
        // ================================================================

        // 1. Return null vector if the market data block is empty
		if( marketDataStringMatrix.empty() )
		{
			return StandardStringVector();
		}
		
		// 2. Check if Market Data Contains the Tenor Column
        const int marketDataTenorColumn  = findMarketDataTenorColumnNumber_ForAllInstrumentTypes( marketDataType );
        
        // The bumpMarketDataTenorColumnNumber method will return -1 when there is no Tenor column in the market data block 
        const bool doesMarketDataContainRateColumn = marketDataTenorColumn > -1 ? true : false; 

        // 3. Extract the Tenor Information from each market data instrument
        if ( doesMarketDataContainRateColumn )
        {
			const size_t nInstruments = marketDataStringMatrix.size();
			StandardStringVector resultInstrumentTenors( nInstruments );
			
			for( size_t i = 0; i < nInstruments; ++i )
			{
				// Check the Market Data Matrix Column Dimensions
				MLIB_REQUIRE( marketDataStringMatrix[i].size() >= size_t( marketDataTenorColumn ), "Invalid Market Data: There are not enough columns in the market data block '"
							+ toString( marketDataType ) + "'. Need at least " + MLIB_TO_STRING_FROM_INT( marketDataTenorColumn ) + " data columns in market data row " + MLIB_TO_STRING_FROM_INT( i ) )

				resultInstrumentTenors[i] = marketDataStringMatrix[i][marketDataTenorColumn].c_str();
			}

			return resultInstrumentTenors;
        }
        
		// Market Data Rate Column not Found
        // =================================
        MLIB_THROW( "Invalid Market Data: Unable to determine the tenor for instrument type '" + toString( marketDataType ) + "'" )
	}

	/* @brief Method to recalculate the futures price for given bump in the underlying forward rate
	* @param [in]   futures price                   original futures price
	* @param [in]   bumpSize                        Enter the bump or shift size to apply
    */
	double CurveMarketData::bumpFuturesPrice( const double & futuresPrice, const double & bumpSize ) const
	{
		// Futures Formulae: 
		// RateFromFutureInPercent      = ( 100 - FuturePrice ) / 100
        // FuturePrice                  = 100 - ( RateFromFutureInPercent * 100 )
        double rateFromFuture			= ( 100.0 - futuresPrice ) / 100.0;
        rateFromFuture					+= bumpSize;
        const double bumpedFuturesPrice = 100.0 - ( rateFromFuture * 100.0 );

		return bumpedFuturesPrice;
	}

    /* @brief Method to bump the marketData for the specified bumpSize and marketDataKey 
	* @param [in]   bumpSize                        Enter the bump or shift size to apply
    * @param [in]   marketDataKey                   The marketData type to be updated
    * @param [in]   onlyBumpOutrightInstruments     Only bump outright instruments, defaults to true
    * @param [in]   noThrow                         No throw if market data not bumped, defaults to false i.e. will throw by default
	*/
    void CurveMarketData::bumpMarketData( const double bumpSize, const StandardString & marketDataType, const bool onlyBumpOutrightInstruments, const bool noThrow ) const
    {
        // Note: Use Base Zero Here for Native C++ Vector / Matrix Lookups
        // ================================================================

        // 1. Extract just the data for the specified marketDataType - Must take from free object to avoid circular bumps
        const CurveMarketDataEnum marketDataTypeAsEnum = toCurveMarketDataEnum( marketDataType );
        VariantMatrix marketData = transpose( getVariantMatrixFromFreeObject( freeObject_, marketDataType ) );

        // 2. Check if Market Data Contains the Rate Column
        const int marketDataRateColumn  = onlyBumpOutrightInstruments ? findMarketDataRateColumnNumber_ForOutrightInstrumentsOnly( marketDataTypeAsEnum )
                                                                      : findMarketDataRateColumnNumber_ForAllInstrumentTypes( marketDataTypeAsEnum );
        
        // The bumpMarketDataRateColumnNumber method will return -1 when there is no Rate column in the market data block 
        const bool doesMarketDataContainRateColumn = marketDataRateColumn > -1 ? true : false; 

        // 3. Bump the Market Data
        if ( doesMarketDataContainRateColumn )
        {
			// The OIS / ARR Swaps can contain futures data, so we need to check the instrument in case it is a future. This is not good.
			const int oisInstrumentColumn = 0; // i.e. Column 1

            for (size_t i = 0; i < marketData.size(); i++)
		    {
                // Check the Market Data Matrix Dimensions and Market Data Rate Data Type
                if ( i == 0 )
                {
                    MLIB_REQUIRE( marketData[0].size() >= size_t(marketDataRateColumn), "Invalid Market Data: There are not enough columns in the market data block '"
                        + marketDataType + "'. Need at least " + MLIB_TO_STRING_FROM_INT( marketDataRateColumn ) + " data columns." )
                }
                
                const double originalMarketData = marketData[i][marketDataRateColumn].getValue<double>();
                double bumpedMarketData;

				switch( marketDataTypeAsEnum )
				{
					case OIS_MARKETDATA:
					{
						// Special treatment for OIS and ARR swaps that can contain futures
						const bool isfuture = isFuture( marketData[i][oisInstrumentColumn].getValueAsString() );
						bumpedMarketData = isfuture ? bumpFuturesPrice( originalMarketData, bumpSize ) : originalMarketData + bumpSize;
						break;
					}
					case FUTURES_MARKETDATA:
					{
						bumpedMarketData = bumpFuturesPrice( originalMarketData, bumpSize );
						break;
					}
					default:
					{
						bumpedMarketData = originalMarketData + bumpSize;
						break;
					}
				}

				// Update Market Data Row
                marketData[i][marketDataRateColumn] = bumpedMarketData;
		    }
        }
        else
        {
            // Market Data Rate Column not Found ( or market data instrument intentionally excluded from bumping )
            // =================================

            // *** Important Note *** : Sometimes we want to update the local market data cache with unbumped instrument data e.g. for spread instruments
            // in this case we set noThrow = true and we update the local cache with the original data unbumped.
            if ( noThrow == false )
            {
                MLIB_THROW( "Invalid Market Data: Cannot apply a bump or shift to market data of type '" + marketDataType + "'" )
            }
        }
        
        // 4. Update the Market Data Block
        updateMarketDataLocalCache( marketData, marketDataType );
    }

    /* @brief Method to bump all marketData for the specified bumpSize
	* @param [in]   bumpSize                        Enter the bump or shift size to apply
    * @param [in]   onlyBumpOutrightInstruments     Only bump outright instruments, defaults to true
    */
    void CurveMarketData::bumpAllMarketData( const double bumpSize, const bool onlyBumpOutrightInstruments ) const
    {
        const bool DO_NOT_THROW = true;
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		for (auto it = keyNames.begin(); it != keyNames.end(); ++it)
		{
            bumpMarketData( bumpSize, *it, onlyBumpOutrightInstruments, DO_NOT_THROW );
		}
    }

    /* @brief Updates the marketData for the specified marketDataKey
	 * @param [in]   marketData      The marketData
     * @param [in]   marketDataKey   The marketData type to be updated
     */
	void CurveMarketData::updateMarketDataLocalCache( const VariantMatrix & marketData, const StandardString & marketDataType ) const
    {
        bool marketDataTypeFound = false;
        for( size_t marketDataIndex = 0; marketDataIndex < freeObject_.keyNames().size(); ++marketDataIndex )
        {
            if ( marketDataType == freeObject_.keyNames()[marketDataIndex] )
            {
                marketDataTypeFound = true;
                break;
            }
        }
        MLIB_REQUIRE( marketDataTypeFound, "Unable to update market data, since marketDataType ' " +  marketDataType + "' not found in the market data object" )
        variantMarketDataLocalCache_[marketDataType] = transpose( marketData );

        stringMarketDataLocalCache_[marketDataType] = transpose( toLAStringMatrixFromVariantMatrix( marketData ) );
    }

    /* @brief Clear the local market data cache(s) */
    void CurveMarketData::clearMarketDataLocalCache() const
    {
        variantMarketDataLocalCache_.clear();
        stringMarketDataLocalCache_.clear();
    }

	/* @brief Returns the marketData information for the specified marketDataKey
	* @param [in]   marketDataKey   The marketData type to be displayed
    * @param [in]   columnNumber    Enter the column number using Base 1 to return a particular column or use -1 to show all columns
	* @param [out]  A VariantMatrix containing a LabelValue block of properties
	*/
    const VariantMatrix CurveMarketData::viewInputParameters( const CurveMarketDataEnum marketDataType, const int columnNumber ) const
	{
        // *** Note We are using Base 1 and also use -1 as a special key to indicate return all columns ***
        // ************************************************************************************************

		if ( marketDataType == NONE_MARKETDATA )
		{
			VariantMatrix allParameters;

			bool addSeparatorRow = false;
			const std::vector<std::string> keyNames = freeObject_.keyNames();
			for (auto it = keyNames.begin(); it != keyNames.end(); ++it)
			{
				if (addSeparatorRow)
				{
					std::vector<Variant> blankRow(2, "");
					allParameters.push_back( blankRow );
				}
				addSeparatorRow = true; // Add a separator before all but the first row

				std::string propertyKey = *it;
				std::vector<Variant> row(2, "");
				row[0] = propertyKey;
				allParameters.push_back(row);

				VariantMatrix variantMatrix = transpose( toVariantMatrix( propertyKey ));
				for (unsigned int i = 0; i < variantMatrix.size(); i++)
				{
					allParameters.push_back(variantMatrix[i]);
				}
			}
			return allParameters;
		}

		// extract just the data for the specified marketDataType
        VariantMatrix variantMatrix = transpose( toVariantMatrix( toString( marketDataType ) ) );

        // Return the Entire Block or a Particular Column as Requested
        // *** Note We are using Base 1 and also use -1 as a special key to indicate return all columns ***
        if ( columnNumber  == -1 || variantMatrix.empty() || variantMatrix[0].empty() )
        {
            // Return Entire or Empty Block
            return variantMatrix;
        }
        else if ( columnNumber > 0 )
        {
            // Return a pre-specified column
            // Access Violation Guard Above
            MLIB_REQUIRE( size_t(columnNumber) <= variantMatrix[0].size(), "Invalid Column Number - Column number must not be greater than the total number of columns" )

            VariantMatrix infoColumn( variantMatrix.size(), std::vector<Variant>(1) ); // Vector of Size 1 Vector
            for( unsigned int i = 0; i < variantMatrix.size(); ++i )
            {
                // Access Violation Guard and Dimension Check Above
                infoColumn[i][0] = variantMatrix[i][ size_t(columnNumber)-1 ]; // Minus one since column index has base 1
            }
            return infoColumn;
        }
        else
        {
            MLIB_THROW( "Unable to Display Data - When specified the column number must be greater than zero and less than the total number of columns" )
        }
        
        // We should never reach here
        return variantMatrix;
	}

    /* @brief Returns a column of marketData information for the specified marketDataKey
	 * @param [in]   marketDataKey   The marketData type to be displayed
     * @param [in]   columnNumber    Enter the column number using Base 1 to return a particular column
	 * @param [out]  A VariantVector containing a LabelValue block of properties
	 */
    const VariantVector CurveMarketData::viewCurveMarketDataColumn( const CurveMarketDataEnum marketDataType, const int & columnNumber ) const
    {
        // extract just the data for the specified marketDataType
        VariantMatrix marketData = transpose( toVariantMatrix( toString( marketDataType ) ) );
		VariantVector infoColumn( marketData.size() );

        // Return the Entire Block or a Particular Column as Requested
        // *** Note We are using Base 1 and also use -1 as a special key to indicate return all columns ***
        MLIB_REQUIRE( columnNumber > 0, "Unable to Display Data - When specified the column number must be greater than zero")

        // Return a pre-specified column 
        // Access Violation Guard Above
        MLIB_REQUIRE( size_t(columnNumber) <= marketData[0].size(), "Invalid Column Number - Column number must not be greater than the total number of columns" )

        for( unsigned int i = 0; i < marketData.size(); ++i )
        {
            // Access Violation Guard and Dimension Check Above
            infoColumn[i] = marketData[i][ size_t(columnNumber)-1 ]; // Minus one since column index has base 1
        }
        return infoColumn;
    }

	/* @brief Returns a VariantMatrix containing the marketData information for the specified marketDataKey
	* @param [in]   marketDataKey   The property to be displayed
	* @param [out]  A VariantMatrix containing the LabelValue block
	*/
	VariantMatrix CurveMarketData::toVariantMatrix( const std::string& marketDataKey ) const
	{
		etrading::VariantMatrix variantMatrix;
		auto iter = variantMarketDataLocalCache_.find(marketDataKey);

		if (iter == variantMarketDataLocalCache_.end())
		{
			// extract just the data for the specified property
			variantMatrix = getVariantMatrixFromFreeObject( freeObject_, marketDataKey );
			variantMarketDataLocalCache_[marketDataKey] = variantMatrix;
		}
		else
		{
			variantMatrix = iter->second;
		}

		return variantMatrix;
	}

	/* @brief Returns a LAStringMatrix containing the marketData information for the specified marketDataKey
	* @param [in]   marketDataKey   The property to be displayed
	* @param [in]   trimBlankRows   Whether to remove blank rows from the end of the LAStringMatrix
	* @param [out]  A LAStringMatrix containing the LabelValue block
	*/
	LAStringMatrix CurveMarketData::toLAStringMatrix( const std::string& marketDataKey, const bool trimBlankRows ) const
	{		
		LAStringMatrix stringMatrix;
		auto iter = stringMarketDataLocalCache_.find(marketDataKey);
		
		if (iter == stringMarketDataLocalCache_.end())
		{
			stringMatrix = getLAStringMatrixFromFreeObject( freeObject_, marketDataKey, trimBlankRows );
			stringMarketDataLocalCache_[marketDataKey] = stringMatrix;
		}
		else
		{
			stringMatrix = iter->second;
		}
		
		return stringMatrix;
	}

    /* @brief Returns a StandardStringMatrix containing the marketData information for the specified marketDataKey
	 * @param [in]   marketDataKey   The property to be displayed
	 * @param [in]   trimBlankRows   Whether to remove blank rows from the end of the LAStringMatrix
	 * @param [out]  A StandardStringMatrix containing the LabelValue block
	 */
	StandardStringMatrix CurveMarketData::toStandardStringMatrix( const std::string& marketDataKey, const bool trimBlankRows ) const
    {
        // Reuse the above toLAStringMtrix() method
        const LAStringMatrix laStringMatrix = toLAStringMatrix( marketDataKey, trimBlankRows );
        const StandardStringMatrix standardStringMatrix( convertToStandardStringMatrix( laStringMatrix ) );
		return standardStringMatrix;
    }

    /* @brief Returns a LabelValueBlock containing the marketData information for the specified marketDataKey
	 * @param [in]   marketDataKey   The property to be displayed
	 * @param [in]   trimBlankRows   Whether to remove blank rows from the end of the LAStringMatrix
	 * @param [out]  A LAStringMatrix containing the LabelValue block
	 */
	LabelValueBlock CurveMarketData::toLabelValueBlock( const std::string& marketDataKey, const bool trimBlankRows ) const
    {
        // Reuse the above toLAStringMtrix() method
        const LAStringMatrix laStringMatrix = toLAStringMatrix( marketDataKey, trimBlankRows );
        const LabelValueBlock LVB( laStringMatrix );
		return LVB;
    }


    /* @brief Returns a boolean to confirm if an optional marketData key has been specified
	 * @param [in]   propertyKey   The property to be displayed
	 * @param [out]  A bool to confirm if the optional property key is present
	 */
	const bool CurveMarketData::doesKeyExist( const std::string& marketDataKey ) const
    {
        bool keyFound = true;

        // 1. FirstCheck for the propertyKey in the LAStringMatrix Cache
        auto iter = stringMarketDataLocalCache_.find( marketDataKey );
		if (iter == stringMarketDataLocalCache_.end())  // Not Found
        {
            // 2. Second Check for the propertyKey in the VariantMatrix Cache
		    auto iter = variantMarketDataLocalCache_.find( marketDataKey );
            if (iter == variantMarketDataLocalCache_.end())
            {
                // 3. Thirdly Check for the propertyKey in the freeobject itself
                const etrading::VariantMatrix& allDataView = freeObject_.viewAllData();
                keyFound = freeObject_.doesKeyExist( marketDataKey );
                return keyFound;
            }
            else
            {
                keyFound = false;
            }
        }

        return keyFound;
    }

	/* @brief Returns A vector of keys used in the block of data identified by the marketDataKey
	* @param [in]   marketDataKey   Key that identifies the block of data interested
	* @param [out]  A vector of keys
	*/
	std::vector<std::string> CurveMarketData::getKeysInTable( const std::string& marketDataKey )
	{
		LAStringMatrix s = toLAStringMatrix( marketDataKey );

		std::vector<std::string> keys;
		for(size_t i = 0; i < s.size(); ++i)
		{
			LAStringVector v = s[i];
			if (v.size() != 0)
			{
				keys.push_back(v[0].getCString());
			}
		}

		return keys;
	}

	/* @brief Sets the local Market Data Cache, which is a temporary cache for market data that is in memory and not persisted as part of the Market Data Object
	* @param [in]   marketDataKey   Key that identifies the block of data being set
	* @param [in]   data			 A string matrix type of market data being set to the object
	*/
	void CurveMarketData::setMarketDataLocalCache( const std::string& marketDataType, const LAStringMatrix& marketData )
	{
		stringMarketDataLocalCache_[marketDataType] = marketData;

        // No need to synch the variantMarketDataLocalCache since we would need to transpose data and find a way to preserve the data type of each column        
	}

	// Method to remove unused market data where the "USE" market data column flag is set to "FALSE"
	// Note the marketDataValues are transpose and the Use Column is optional and not always present
	VariantMatrix CurveMarketData::checkDimensionsAndRemoveUnusedData( const std::string & marketDataKey, const VariantMatrix & marketDataValues )
	{
		// Initialize Results
		VariantMatrix results = marketDataValues;

		// Return Empty Matrix if Input is also Empty
		size_t nDataRows = marketDataValues.size();
		if ( marketDataValues.empty() || nDataRows == 0 )
		{
			return results;
		}

		// *** Important Note *** JSON Data is Transposed, Columns are Rows and Vice Versa
		// Number of Market Data Rows and Columns, Access Violation Check Above
		size_t nDataColumns = marketDataValues[0].size();
		size_t useMarketDataRow = 0;

		// Validate Market Data Dimensions and Set the useFlagColumn for each curve instrument data type
		CurveMarketDataEnum curveMarketDataEnum = toCurveMarketDataEnum(marketDataKey);
		switch ( curveMarketDataEnum )
		{
			case OIS_MARKETDATA:
			{
				// Note: Data is Transposed and using Index Base 0
				MLIB_REQUIRE(nDataRows == 2 || nDataRows == 4 || nDataRows == 5 || nDataRows == 6, "Invalid Market Data: OIS data must have 2, 4, 5 or 6 columns:- Tenor, Rate, Start (OIS Futures), End(OIS Futures), Convexity (OIS Futures), Use (Optional)")
					useMarketDataRow = 5; // i.e. Transposed Column 6 with base 0
				break;
			}
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
			{
				// Note: Data is Transposed and using Index Base 0
				MLIB_REQUIRE(nDataRows == 2 || nDataRows == 3 || nDataRows == 4, "Invalid Market Data: LIBOROISBASISSPREADS data must have between 2-4 columns:- Tenor, Rate, BasisType (Optional), Use (Optional)")
					useMarketDataRow = 3; // i.e. Transposed Column 4 with base 0
				break;
			}
			case FRA_MARKETDATA:
			{
				// Note: Data is Transposed and using Index Base 0
				// Dynamic Use Market Data Column: Can be Column 3 or 5
				MLIB_REQUIRE(nDataRows == 2 || nDataRows == 3, "Invalid Market Data: FRAS data must have 2-3 columns:- Tenor, Rate, Use (Optional)")
					useMarketDataRow = 2; // i.e. Transposed Column 3 with base 0
				if ( nDataRows >= 4 )
				{
					useMarketDataRow = 4; // i.e. Transposed Column 5 with base 0
				}
				break;
			}
			case FUTURES_MARKETDATA:
			{
				// Note: Data is Transposed and using Index Base 0
				MLIB_REQUIRE(nDataRows == 5 || nDataRows == 6, "Invalid Market Data: FUTURES data must have 5-6 columns:- Tenor, Start, End, Rate, Convexity, Use (Optional)")
					useMarketDataRow = 5; // i.e. Transposed Column 6 with base 0
				break;
			}
			case SWAP_MARKETDATA:
			{
				// Note: Data is Transposed and using Index Base 0
				MLIB_REQUIRE(nDataRows == 2 || nDataRows == 3, "Invalid Market Data: SWAPS data must have 2-3 columns:- Tenor, Rate, Use (Optional)")
					useMarketDataRow = 2; // i.e. Transposed Column 3 with base 0
				break;
			}
			case BASIS_SWAP_MARKETDATA:
			{
				// Note: Data is Transposed and using Index Base 0
				MLIB_REQUIRE(nDataRows == 2 || nDataRows == 3, "Invalid Market Data: BASISSWAPS data must have 2-3 columns:- Tenor, Rate, Use (Optional)")
					useMarketDataRow = 2; // i.e. Transposed Column 3 with base 0
				break;
			}
			case XCCY_SWAP_MARKETDATA:
			{
				// Note: Data is Transposed and using Index Base 0
				MLIB_REQUIRE(nDataRows == 2 || nDataRows == 3, "Invalid Market Data: XCCYSWAPS data must have 2-3 columns:- Tenor, Rate, Use (Optional)")
					useMarketDataRow = 2; // i.e. Transposed Column 3 with base 0
				break;
			}
			case FXFWD_MARKETDATA:
			{
				// Note: Data is Transposed and using Index Base 0
				MLIB_REQUIRE(nDataRows == 2 || nDataRows == 3, "Invalid Market Data: FXFWDS data must have 2-3 columns:- Tenor, Rate, Use (Optional)")
					useMarketDataRow = 2; // i.e. Transposed Column 3 with base 0
				break;
			}
            case FXFWD_BIDASK_MARKETDATA:
			{   
				// Note: Data is Transposed and using Index Base 0
				MLIB_REQUIRE(nDataRows == 3 || nDataRows == 4, "Invalid Market Data: FXFWDS data must have 3-4 columns:- Tenor, Bid, Ask, Use (Optional)")
					useMarketDataRow = 3; // i.e. Transposed Column 4 with base 0
				break;
			}
			default:
			{
				// Return Original Data if the Market Data Type does not support the "USE" market data parameter e.g. Fixing Tables
				return results;
				break;
			}
		}

		// We Only Reach Here if the Market Data Type Supports the "USE" market data parameter

		// Check for "USE" market data Column (Note JSON data is transposed)
		// Note: nDataRows is the size with index base 1, and useMarketDataRow is an index base 0, so we add 1 to the useMarketDataRow
		const bool foundUseMarketDataColumn = ( nDataRows >= useMarketDataRow + 1 ) ? true : false;

		if ( foundUseMarketDataColumn )
		{
			// Market Data Use Value, can be TRUE, FALSE or blank (=TRUE)
			std::string useValue = "";

			// We assume a rectangular (non-jagged) matrix and note the column size is changing as we erase unused data columns
			// Here we use empty() as access violation guard in case we erase all data columns when all columns are set to USE = false
			for ( size_t i = 0; !results.empty() && i < results[0].size(); ++i )
			{
				useValue = results[useMarketDataRow][i].toString();

				// Skip if Use Market Data is empty or TRUE
				if ( useValue == "" || boost::iequals( useValue, std::string("TRUE") ) )
				{
						continue;	
				}
				// Erase Results Market Data if USE Flag is FALSE
				else if ( boost::iequals( useValue, std::string("FALSE") ) )
				{
				
					// Erase Market Data Row
					// Note JSON data is by column so deleting a market data row translates to deleting a JSON data column
					for ( size_t j = 0; j < nDataRows; ++j )
					{
						results[j].erase( results[j].begin() + i );
					}

					// Reallign the i index column counter
					--i;
				}
				else
				{
					MLIB_THROW("Invalid Curve Market Data: " + marketDataKey + " 'Use' parameter in row " + MLIB_TO_STRING_FROM_SIZE_T(i+1) +
							   " and column " + MLIB_TO_STRING_FROM_SIZE_T( useMarketDataRow + 1 ) + " is set to '" + useValue +
							   "' but should be 'TRUE', 'FALSE' or blank" )
				}
			}
		}
		return results;
	}

}
