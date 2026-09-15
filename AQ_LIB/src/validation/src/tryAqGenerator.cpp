#include "tryAqGenerator.h"
#include "tryAqObject.h"
#include "tryAqSwapObjectCreation.h"
#include "tryAqBondObject.h"
#include "tryAqCurveGenerator.h"

#include "FolderConfig.h"
#include "CoreEnumerations.h"
#include "SerializationUtilities.h"
#include "StructuredExceptionHandler.h"
#include "Variant.h"
#include "RecordMacros.h"

#include <boost/filesystem.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace validation
{
    namespace
    {
        // Restricts typeAsString to the three generator types - Generator is a
        // cross-cutting introspection category over Swap/Bond/Curve's own
        // generator objects, not a general-purpose object-type lookup.
        etrading::CachedObjectEnum toGeneratorTypeEnum( const std::string& typeAsString )
        {
            const etrading::CachedObjectEnum objEnum = etrading::toCachedObjectEnum( etrading::trim_to_upper( typeAsString.c_str() ) );

            AQ_REQUIRE( objEnum == etrading::SWAP_GENERATOR || objEnum == etrading::BOND_GENERATOR || objEnum == etrading::CURVE_GENERATOR,
                        "aqGenerator functions only support SWAP_GENERATOR, BOND_GENERATOR or CURVE_GENERATOR, not '" + typeAsString + "'" )

            return objEnum;
        }

        // The generator folders live at $(AQ)/resources/config/<type>, where <type>
        // is the CachedObjectEnum's own string form - the same convention
        // FolderConfig::deserializeObjectsForOptionalStartup already relies on.
        std::string generatorFolder( const etrading::CachedObjectEnum objEnum )
        {
            const std::string folder = etrading::FolderConfig::toPath( "AQ", "/resources/config/" + etrading::toString( objEnum ) ).getCString();
            AQ_REQUIRE( !folder.empty(), "Unable to resolve the generator folder; the 'AQ' environment variable has not been set." )
            return folder;
        }

        std::string generatorFilePath( const etrading::CachedObjectEnum objEnum, const std::string& generatorName )
        {
            return generatorFolder( objEnum ) + "/" + etrading::appendFileExtension( generatorName, etrading::JSON );
        }
    }


    std::vector<std::string> tryAqGeneratorList( const std::string& typeAsString )
    {
        VALID_EXCEPTION_START

        const etrading::CachedObjectEnum objEnum = toGeneratorTypeEnum( typeAsString );
        const boost::filesystem::path folder( generatorFolder( objEnum ) );

        std::vector<std::string> generatorNames;

        if ( boost::filesystem::exists( folder ) && boost::filesystem::is_directory( folder ) )
        {
            for ( boost::filesystem::directory_iterator it( folder ), end; it != end; ++it )
            {
                if ( !boost::filesystem::is_regular_file( it->status() ) )
                {
                    continue;
                }

                std::string extension = it->path().extension().string();
                std::transform( extension.begin(), extension.end(), extension.begin(), ::toupper );

                if ( extension == ".JSON" )
                {
                    generatorNames.push_back( it->path().stem().string() );
                }
            }
        }

        std::sort( generatorNames.begin(), generatorNames.end() );

        return generatorNames;

        VALID_EXCEPTION_END
    }


    const etrading::VariantMatrix tryAqGeneratorDisplay( const std::string& typeAsString, const std::string& generatorName )
    {
        // No thread guard here - this composes tryAqObjectLoad and the per-type
        // Display functions below, which each own their own thread guard; a
        // second one on this frame trips the re-entrancy check (see
        // tryAqObjectLoad's own comment on tryAqObjectLoadAndReturnTupleResults).
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

        const etrading::CachedObjectEnum objEnum = toGeneratorTypeEnum( typeAsString );
        const std::string filePath = generatorFilePath( objEnum, generatorName );

        etrading::checkFileExists( filePath, etrading::JSON );

        // Load (or refresh) the generator into the AQObj cache through the same
        // path aqObjectLoad uses, then hand off to the category that already
        // owns this generator type's own Display function - Generator does not
        // duplicate that rendering logic, only exposes it uniformly.
        tryAqObjectLoad( filePath );

        switch ( objEnum )
        {
            case etrading::SWAP_GENERATOR:
                return etrading::toVariantMatrixFromAQLStringMatrix( tryAqSwapGeneratorDisplay( generatorName ) );

            case etrading::BOND_GENERATOR:
                return tryAqBondGeneratorDisplay( generatorName, "" );

            case etrading::CURVE_GENERATOR:
                return tryAqCurveGeneratorDisplay( generatorName, "" );

            default:
                AQ_THROW( "aqGeneratorDisplay only supports SWAP_GENERATOR, BOND_GENERATOR or CURVE_GENERATOR" )
        }

        VALID_EXCEPTION_END
    }


    std::string tryAqGeneratorValidate( const std::string& typeAsString, const std::string& generatorName )
    {
        // No thread guard here - composes tryAqObjectLoad, which owns its own
        // (see tryAqGeneratorDisplay above for why).
        VALID_EXCEPTION_START_WITH_NO_THREAD_GUARD

        const etrading::CachedObjectEnum objEnum = toGeneratorTypeEnum( typeAsString );
        const std::string filePath = generatorFilePath( objEnum, generatorName );

        // A successful load proves the JSON parses and satisfies the generator's
        // schema - both are already enforced by aqObjectLoad's own deserialization
        // and key validation, so re-use it rather than re-implementing a schema
        // check here. Caught locally (not left to VALID_EXCEPTION_END) because
        // an invalid generator is an expected result to report, not a failure
        // of aqGeneratorValidate itself.
        try
        {
            etrading::checkFileExists( filePath, etrading::JSON );
            tryAqObjectLoad( filePath );
        }
        catch ( const std::exception& e )
        {
            return std::string( "Invalid: " ) + e.what();
        }

        return "OK";

        VALID_EXCEPTION_END
    }
}
