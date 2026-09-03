/*
 * @brief			validation interface for the meLWOCurveGenerator method
 * @Created:		24 Nov 2016
 * @Author:			Ian Castleton
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "tryMeLWOCurveGenerator.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "CoreEnumerations.h"
#include "CurveGenerator.h"
#include "LWOUtilities.h"
#include "RecordMacros.h"
#include "JSONInfoBlock.h"

#include <string>
#include <set>
#include <algorithm>
#include <boost/format.hpp>

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation_api
{
    /* @brief Creates an LWOCurveGenerator object, containing all of the curve properties.
	 * @param [in] objectName        The name of the Curve Configuration object
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
    std::string tryMeLWOCurveGeneratorCreate( const std::string& objectName,
                                              const std::vector<std::string>& propertyNames,
                                              const std::vector<TableInfo>& infoBlocks )
    {

		VALID_EXCEPTION_START
		
		size_t nColumnHeaders   = propertyNames.size();
        size_t nDataColumns     = infoBlocks.size();
		
        MLIB_REQUIRE( nDataColumns > 0, "Invalid InfoBlock Data: Empty InfoBlock - No data provided" )
        MLIB_REQUIRE( nColumnHeaders == nDataColumns, "Invalid InfoBlock Data: Number of Data Column Headers " +  std::to_string(static_cast<long long>(nColumnHeaders)) + " does not match the actual number of Data Columns " +  std::to_string(static_cast<long long>(nDataColumns)) )
        
        // TODO: JSONInfoBlocks should not be here move below the valiation_api
        // Clean Data - Remove Blanks
        const etrading::JSONInfoBlockTuples cleansedInfoBlocks = etrading::JSONInfoBlock::trimInfoBlocks( propertyNames, infoBlocks );
        
		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOCurveGeneratorCreate_inputs", objectName.c_str()));
			file.write("generatorFunction", "tryMeLWOCurveGeneratorCreate");
			file.write("objectName", objectName);

			// Write out each propertyName and corresponding block of property config data
			for( unsigned int i = 0; i < propertyNames.size(); i++ )
			{
				// TODO: JSONInfoBlocks should not be here move below the valiation_api
				const LAString propertyName( propertyNames[i].c_str() );
                const etrading::VariantMatrix& configData = std::get<2>( cleansedInfoBlocks[ i ] );
				file.write( propertyName, transpose(configData) );
			}
		}

		// Verify that the supplied propertyNames have been set
        const bool hasAnEmptyName = std::any_of( propertyNames.cbegin(),
												 propertyNames.cend(),
												 []( const std::string & propertyName ) -> bool
        {
            return ( propertyName.empty() || propertyName == "" );
        } );

        MLIB_REQUIRE( !hasAnEmptyName, "Invalid InfoBlock: Invalid Data Column Header - One of the Column Names is empty or invalid " + etrading::containerAsString( propertyNames ) )

		// Verify that the supplied propertyNames match the CurveGeneratorEnum
		std::set<etrading::CurveGeneratorEnum> enumSet;
		std::for_each(propertyNames.cbegin(),
					  propertyNames.cend(),
					  [&enumSet] (const std::string & propertyName )
		{	
			etrading::CurveGeneratorEnum curveGeneratorEnum = etrading::toCurveGeneratorEnum( propertyName );
			enumSet.insert( curveGeneratorEnum );
		});

		// Create the CurveGenerator object
		etrading::CurveGenerator curveGenerator( objectName, propertyNames, cleansedInfoBlocks );

		// ..  and store in the cache
        etrading::copyToCache<etrading::CurveGenerator>( curveGenerator );

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeLWOCurveGeneratorCreate_outputs", objectName.c_str()));
			file.write("output", objectName);
		}

        return objectName;

		VALID_EXCEPTION_END
    };

	/* @brief Displays the specified property of an LWOCurveGenerator. If propertyName is blank, all properties are returned.
	 * @param [in] objectName        The CurveGenerator object you wish to display
	 * @param [in] propertyName      The name of the property label-value block that you wish to display.
	 * @param [out]                  A VariantMatrix containing a LabelValue block of properties.
	 */
	const etrading::VariantMatrix tryMeLWOCurveGeneratorDisplay(const std::string& objectName, const std::string& propertyKey )
    {
		VALID_EXCEPTION_START

		// Attempt to retrieve LWOCurveGenerator object from the LWO Cache
		auto curveGenerator = etrading::getCurveGenerator( objectName );

		// extract just the data for the specified propertyKey
		return curveGenerator->viewInputParameters( propertyKey );

		VALID_EXCEPTION_END
    }

	/* @brief Builds a new CurveGenerator from an existing base CurveGenerator, with modified properties.
	*  @param[in] newObjectName		The name to use for the new CurveGenerator object
	*  @param[in] baseObjectName	The name of the existing CurveGenerator object on which to base the new object
	*  @param[in] modifiedValues	A LabelValueBlock containing key/value pairs to update. Each key is specified in the format: PROPERTYNAME:KEYNAME.
	*/
	std::string tryMeLWOCurveGeneratorModify( const std::string& newObjectName,
											  const std::string& baseObjectName,
                                              const etrading::LabelValueBlock& modifiedValues )
	{
		VALID_EXCEPTION_START

		RECORD_INPUTS( newObjectName, baseObjectName, modifiedValues );

		// Attempt to retrieve the existing CurveGenerator object from the LWO Cache
		auto baseCurveGenerator = etrading::getCurveGenerator( baseObjectName );

		// Construct the new CurveGenerator with modified values
		auto newCurveGenerator = etrading::CurveGenerator( newObjectName, *baseCurveGenerator, modifiedValues );

		// Store the new CurveGenerator in the LWO cache
        etrading::copyToCache<etrading::CurveGenerator>( newCurveGenerator );

		RECORD_OUTPUTS_AND_RETURN_RESULT( newObjectName );

		VALID_EXCEPTION_END
	}

    /* @brief Displays the specified property of an LWOCurveGenerator. If propertyName is blank, all properties are returned.
	 * @param [in] objectName        The CurveGenerator object you wish to display
	 * @param [in] propertyName      The name of the property label-value block that you wish to display.
	 * @param [out]                  A VariantMatrix containing a LabelValue block of properties.
	 */
	const etrading::VariantMatrix tryMeLWOCurveDisplayConventions(const std::string& objectName, const std::string& propertyKey )
    {
		VALID_EXCEPTION_START

		auto& env = etrading::Environment::defaultEnv();
        auto singleCurveObject = env.accessObject<etrading::SingleCurveObject>( objectName );
        MLIB_REQUIRE( singleCurveObject, "LWO Curve " + objectName + " does not exist" )

		// Attempt to retrieve LWOCurveConventions / generator object from the LWO Cache
		auto curveGenerator = singleCurveObject->getCurveGeneratorObj();

		// extract just the data for the specified propertyKey
		return curveGenerator->viewInputParameters( propertyKey );

		VALID_EXCEPTION_END
    }
}
