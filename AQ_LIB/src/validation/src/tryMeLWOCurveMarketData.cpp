#include "tryMeLWOCurveMarketData.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "CoreEnumerations.h"
#include "CurveMarketData.h"
#include "LWOUtilities.h"
#include "JSONInfoBlock.h"
#include "ExceptionMacros.h"
#include "RecordMacros.h"

#include <string>
#include <set>
#include <algorithm>
#include <boost/format.hpp>

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation_api
{
    /* @brief Creates a LWOCurveMarketData object, containing all of the curve properties.
	 * @param [in] objectName               The name of the Market Data object
	 * @param [in] marketDataTypes          A vector of market data instrument types corresponding to each label-value block of market data
	 * @param [in] infoBlocks               A vector of containing the label-value blocks of market data
	 * @param [out]                         The objectName
	 */
    std::string tryMeLWOCurveMarketDataCreate(  const std::string& objectName,
												const std::vector<std::string>& marketDataKeys,
												const std::vector<TableInfo>& infoBlocks )
	{
		VALID_EXCEPTION_START
       
		size_t nColumnHeaders   = marketDataKeys.size();
        size_t nDataColumns     = infoBlocks.size();
		
        AQ_REQUIRE( nDataColumns > 0, "Invalid InfoBlock Data: Empty InfoBlock - No data provided" )
        AQ_REQUIRE( nColumnHeaders == nDataColumns, "Invalid InfoBlock Data: Number of Data Column Headers " +  std::to_string(static_cast<long long>(nColumnHeaders)) + " does not match the actual number of Data Columns " +  std::to_string(static_cast<long long>(nDataColumns)) )
        
        // TODO: JSONInfoBlocks should not be here move below the valiation_api
        // Clean Data - Remove Blanks
        const etrading::JSONInfoBlockTuples cleansedInfoBlocks = etrading::JSONInfoBlock::trimInfoBlocks( marketDataKeys, infoBlocks );
        
		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOCurveMarketDataCreate_inputs", objectName.c_str()));
			file.write("generatorFunction", "tryMeLWOCurveMarketDataCreate");
			file.write("objectName", objectName);

			// Write out each marketDataType and correspondiong block of market data
			for( unsigned int i = 0; i < marketDataKeys.size(); i++ )
			{
			    // TODO: JSONInfoBlocks should not be here move below the valiation_api
                const LAString marketDataType( marketDataKeys[i].c_str() );
				const etrading::VariantMatrix& marketData = std::get<2>( cleansedInfoBlocks[ i ] );
				file.write( marketDataType, transpose(marketData) );
			}
		}

		// Verify that the supplied marketDataTypes have been set
        const bool hasAnEmptyName = std::any_of( marketDataKeys.cbegin(),
												    marketDataKeys.cend(),
												    []( const std::string & marketDataKey ) -> bool
        {
            return ( marketDataKey.empty() || marketDataKey == "" );
        } );

        AQ_REQUIRE( !hasAnEmptyName, "Invalid InfoBlock: Invalid Data Column Header - One of the Column Names is empty or invalid " + etrading::containerAsString( marketDataKeys ) )

		// Verify that the supplied marketDataTypes match the CurveMarketDataEnum
		std::set<etrading::CurveMarketDataEnum> enumSet;
		std::for_each(marketDataKeys.cbegin(),
					    marketDataKeys.cend(),
					    [&enumSet] (const std::string & marketDataType )
		{	
			etrading::CurveMarketDataEnum curveMarketDataEnum = etrading::toCurveMarketDataEnum(marketDataType);
			enumSet.insert(curveMarketDataEnum);
		});

		// Create the CurveMarketData object and store in the cache
		etrading::CurveMarketData curveMarketData(objectName, marketDataKeys, cleansedInfoBlocks );
        etrading::copyToCache<etrading::CurveMarketData>( curveMarketData );

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOCurveMarketDataCreate_outputs", objectName.c_str()));
			file.write("output", objectName);
		}

        return objectName;

        VALID_EXCEPTION_END
	}

    /* @brief Clears All Market Data Bumps and Restores the Existing Curve Market Data Object
	 * @param [in] objectName                       The name of the Market Data object
     * @param [out]                                 The objectName
	 */
    std::string tryMeLWOCurveMarketDataBumpClear( const std::string& objectName )
    {
        VALID_EXCEPTION_START
       
		RECORD_INPUTS( objectName );
        
        // Attempt to retrieve LWOCurveMarketData object from the LWO Cache
		auto curveMarketData = etrading::getCurveMarketData( objectName );
        curveMarketData->clearMarketDataLocalCache();

		RECORD_OUTPUTS_AND_RETURN_RESULT( objectName )

        VALID_EXCEPTION_END
    }

    /* @brief Bumps a Single Data Block within an Existing Curve Market Data Object
	 * @param [in] objectName                       The name of the Market Data object
     * @param [in] marketDataType                   The market data block or type to bump
     * @param [in] bumpSize                         The BumpSize in Percent
     * @param [out]                                 The objectName
	 */
    std::string tryMeLWOCurveMarketDataBumpInstrument( const std::string& objectName, const std::string & marketDataType, const double bumpSize, const bool clearExistingBumps )
    {
        VALID_EXCEPTION_START
       
		RECORD_INPUTS( objectName, marketDataType, bumpSize );
        
        // Get Market Data, Clear Existing Bumps then Apply New Bump
		auto curveMarketData = etrading::getCurveMarketData( objectName );
        
        if ( clearExistingBumps )
        {
            curveMarketData->clearMarketDataLocalCache();
        }

        curveMarketData->bumpMarketData( bumpSize, marketDataType, false, true ); // false = onlyBumpOutrights, true = throwOnError

		RECORD_OUTPUTS_AND_RETURN_RESULT( objectName )

        VALID_EXCEPTION_END
    }


    /* @brief Bumps a Single Instrument within an Existing Curve Object
	 * @param [in] objectName                       The name of the Market Data object
     * @param [in] marketDataType                   The market data block or type to bump
     * @param [in] bumpSize                         The BumpSize in Percent
     * @param [out]                                 The objectName
	 */
    std::string tryMeLWOCurveBumpInstrument( const std::string& objectName, const std::string & marketDataType, const double bumpSize, const bool clearExistingBumps )
    {
        VALID_EXCEPTION_START
       
		RECORD_INPUTS( objectName );
        
        auto& env = etrading::Environment::defaultEnv();
        auto curveObject = env.accessObject<etrading::SingleCurveObject>( objectName );
        AQ_REQUIRE( curveObject, "Curve " + objectName + " does not exist" )

        // Get Market Data, Clear Existing Bumps then Apply New Bump
		auto curveMarketData = curveObject->getCurveMarketDataObj();
        
        if ( clearExistingBumps )
        {
            curveMarketData->clearMarketDataLocalCache();
        }

        curveMarketData->bumpMarketData( bumpSize, marketDataType, false, true ); // false = onlyBumpOutrights, true = throwOnError

        // ReCalibrate Curve
        curveObject->calibrateCurve();
        etrading::copyToCache<etrading::SingleCurveObject>( *curveObject );

		RECORD_OUTPUTS_AND_RETURN_RESULT( objectName )

        VALID_EXCEPTION_END
    }

    /* @brief Bumps an Existing Curve Market Data Object
	 * @param [in] objectName                       The name of the Market Data object
     * @param [in] bumpSize                         The BumpSize in Percent
     * @param [in] onlyBumpOutrightInstruments      Only bump outright instruments, defaults to true
     * @param [out]                                 The objectName
	 */
    std::string tryMeLWOCurveMarketDataBumpAll( const std::string& objectName, const double bumpSize, const bool onlyBumpOutrightInstruments )
    {
        VALID_EXCEPTION_START
       
		RECORD_INPUTS( objectName, bumpSize, onlyBumpOutrightInstruments );
        
		auto curveMarketData = etrading::getCurveMarketData( objectName );
        curveMarketData->bumpAllMarketData( bumpSize, onlyBumpOutrightInstruments );

		RECORD_OUTPUTS_AND_RETURN_RESULT( objectName )

        VALID_EXCEPTION_END
    }

    /* @brief Bumps all Instruments in an Existing Curve Object
	 * @param [in] objectName                       The name of the Market Data object
     * @param [in] bumpSize                         The BumpSize in Percent
     * @param [in] onlyBumpOutrightInstruments      Only bump outright instruments, defaults to true
     * @param [out]                                 The objectName
	 */
    std::string tryMeLWOCurveBumpAll( const std::string& objectName, const double bumpSize, const bool onlyBumpOutrightInstruments )
    {
        VALID_EXCEPTION_START
       
		RECORD_INPUTS( objectName );
        
        auto& env = etrading::Environment::defaultEnv();
        auto curveObject = env.accessObject<etrading::SingleCurveObject>( objectName );
        AQ_REQUIRE( curveObject, "Curve " + objectName + " does not exist" )

        // Bump Market Data
		auto curveMarketData = curveObject->getCurveMarketDataObj();
        curveMarketData->bumpAllMarketData( bumpSize, onlyBumpOutrightInstruments );

        // ReCalibrate Curve
        curveObject->calibrateCurve();
        etrading::copyToCache<etrading::SingleCurveObject>( *curveObject );

		RECORD_OUTPUTS_AND_RETURN_RESULT( objectName )

        VALID_EXCEPTION_END
    }

	/* @brief Displays a LWOCurveMarketData
	* @param [in] marketDataObjectName	The market data object you wish to display
	* @param [in] marketDataKey			The name of the marketData label-value block that you wish to display
    * @param [in] columnIndexToDisplay	Optional Column Number to display (starting from 1). Specify -1 to get all columns
	* @param [out]						A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantMatrix tryMeLWOCurveMarketDataDisplay( const std::string& marketDataObjectName,
														    const std::string& marketDataKey,
                                                            const int columnIndexToDisplay )
    {
		VALID_EXCEPTION_START

		// Attempt to retrieve LWOCurveMarketData object from the LWO Cache
		auto curveMarketData = etrading::getCurveMarketData( marketDataObjectName );

		 // Attempt to convert the marketData type to enum. This will throw early with a meaningful error message if
		 // an incorrect marketData Key is provided.
		 etrading::CurveMarketDataEnum enumTypeToDisplay = etrading::toCurveMarketDataEnum(marketDataKey.c_str());

		 // extract just the data for the specified marketDataKey
		return curveMarketData->viewInputParameters( enumTypeToDisplay, columnIndexToDisplay );

		VALID_EXCEPTION_END
	}


    /* @brief Displays CurveMarketData extracted from a curve object
	* @param [in] curveObjectName		The curve object whose market data you wish to display
	* @param [in] marketDataKey			The name of the marketData label-value block that you wish to display
    * @param [in] columnIndexToDisplay	Optional Column Number to display (starting from 1). Specify -1 to get all columns
	* @param [out]						A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantMatrix tryMeLWOCurveMarketDataDisplayFromCurve( const std::string& curveObjectName,
																	 const std::string& marketDataKey,
																	 const int columnIndexToDisplay )
    {
		VALID_EXCEPTION_START

        auto& env = etrading::Environment::defaultEnv();
        auto singleCurveObject = env.accessObject<etrading::SingleCurveObject>( curveObjectName );
        AQ_REQUIRE( singleCurveObject, "LWO Curve " + curveObjectName + " does not exist" )

		// Attempt to retrieve LWOCurveMarketData object from the LWO Cache
		auto curveMarketData = singleCurveObject->getCurveMarketDataObj();

		 // Attempt to convert the marketData type to enum. This will throw early with a meaningful error message if
		 // an incorrect marketData Key is provided.
		 etrading::CurveMarketDataEnum enumTypeToDisplay = etrading::toCurveMarketDataEnum( marketDataKey.c_str() );

		 // extract just the data for the specified marketDataKey
		return curveMarketData->viewInputParameters( enumTypeToDisplay, columnIndexToDisplay );

		VALID_EXCEPTION_END
	}
	

    /* @brief Displays a Column LWOCurveMarketData for use with Google Test Calibration Tests
	* @param [in] curveObjectName      The curve object whose market data you wish to display
	* @param [in] marketDataKey     The name of the marketData label-value block that you wish to display
    * @param [in] columnNumber      Optional Column Number to display using column base index 1 and -1 to display all columns
	* @param [out]                  A VariantMatrix containing a LabelValue block of properties
	*/
	etrading::VariantVector tryMeLWOCurveMarketDataColumn( const std::string& curveObjectName,
														   const std::string& marketDataKey,
                                                           const int columnNumber )
    {
		VALID_EXCEPTION_START

        auto& env = etrading::Environment::defaultEnv();
        auto singleCurveObject = env.accessObject<etrading::SingleCurveObject>( curveObjectName );
        AQ_REQUIRE( singleCurveObject, "LWO Curve " + curveObjectName + " does not exist" )

		// Attempt to retrieve LWOCurveMarketData object from the LWO Cache
		auto curveMarketData = singleCurveObject->getCurveMarketDataObj();

		 // Attempt to convert the marketData type to enum. This will throw early with a meaningful error message if
		 // an incorrect marketData Key is provided.
		 etrading::CurveMarketDataEnum enumTypeToDisplay = etrading::toCurveMarketDataEnum( marketDataKey.c_str() );

		 // extract just the data for the specified marketDataKey and column number
		return curveMarketData->viewCurveMarketDataColumn( enumTypeToDisplay, columnNumber );

		VALID_EXCEPTION_END
	}

}

