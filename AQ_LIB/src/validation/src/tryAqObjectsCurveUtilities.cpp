
#include <boost/format.hpp>

#include "tryAqObjectsCurveUtilities.h"
#include "AQOUtilities.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "ContainerUtilities.h"
#include "EnvironmentUtilities.h"
#include "FileUtilities.h"
#include "Environment.h"
#include "ObjectUtilities.h"
#include "AQLString.h"
#include "EntityPoolUtilities.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;


namespace validation
{
    const std::vector<std::string> tryAqObjectsCurveList()
    {
        auto& curveStore = etrading::getObjectStore<etrading::AQOCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        auto keys = curveStore.keys();
        if( keys.size() <= 0 )
        {
            AQ_THROW( "No AQOCurves are registered" );
        }
        return curveStore.keys();
    };

    const bool tryAqObjectsCurveDelete( const std::string& curveName )
    {
        if ( !etrading::doesAQOExist( curveName, "CURVE" ) )
        {
            AQ_THROW( ( boost::format( "Object %s does not exist." ) % curveName.c_str() ).str().c_str() );
        }
        return etrading::Environment::defaultEnv().deleteObject<etrading::AQOCurve>(curveName);
    };

    const int tryAqObjectsCurveDeleteAll()
    {
        return etrading::Environment::defaultEnv().deleteAllObjects<etrading::AQOCurve>();
    };

    std::string tryAqObjectsCurveSave(	const std::string& aqoCurveName,
                                    const std::string& fileNameToWriteTo,
                                    const etrading::FileTypeEnum fileType )
    {

        auto& env = etrading::Environment::defaultEnv();
        auto aqoCurve = env.accessObject<etrading::AQOCurve>( aqoCurveName ); // 			etrading::getAQOCurve(aqoCurveName);

        if( !aqoCurve )
        {
            AQ_THROW( ( boost::format( "AQOCurve %s does not exist" ) % aqoCurveName.c_str() ).str().c_str() );
        }
        
        // Append the file extension if missing
        std::string filenameWithExtension = etrading::appendFileExtension( fileNameToWriteTo, fileType );
	
        // Serialize
        aqoCurve->serialize( etrading::serialize::JSON, etrading::serialize::FILE, filenameWithExtension );
        
        // Check the Serialization file was created
        etrading::checkFileExists( filenameWithExtension, fileType );

        // Return Result
        return ( boost::format( "Curve %s was written to file %s" ) % aqoCurveName.c_str() % filenameWithExtension.c_str() ).str();
        
    };

    std::pair<const bool, std::string> tryAqObjectsCurveLoad( const std::string& fileName,
                                                          const etrading::FileTypeEnum fileType )
    {
        // Append the file extension if missing
        std::string filenameWithExtension = etrading::appendFileExtension( fileName, fileType );

        // Check the Serialization file was created
        etrading::checkFileExists( filenameWithExtension, fileType );

        // Deserialize and Cache the Object
        auto cacheInfoOnDeserialization = etrading::deSerializeFromJSON( etrading::serialize::FILE, filenameWithExtension );
        std::string objectName = cacheInfoOnDeserialization.first;

        auto& curveStore = etrading::getObjectStore<etrading::AQOCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        if( !curveStore.has( objectName ) )
        {
            throw AQLCoreAppError( ( boost::format( "File (%s) was loaded and read but was not a Curve, found object of type %s" )
                                % fileName.c_str()
                                % toString( cacheInfoOnDeserialization.second ) ).str().c_str(), __FILE__, __LINE__ );
        }
        return std::make_pair( true, objectName );
    };



}