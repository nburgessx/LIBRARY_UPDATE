
#include <sstream>

#include "tryAqCurveObjectUtilities.h"
#include "AQObjUtilities.h"
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
    const std::vector<std::string> tryAqCurveObjectList()
    {
        auto& curveStore = etrading::getObjectStore<etrading::AQObjCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        auto keys = curveStore.keys();
        if( keys.size() <= 0 )
        {
            AQ_THROW( "No AQObjCurves are registered" );
        }
        return curveStore.keys();
    };

    const bool tryAqCurveObjectDelete( const std::string& curveName )
    {
        if ( !etrading::doesAQObjExist( curveName, "CURVE" ) )
        {
            std::ostringstream msg;
            msg << "Object " << curveName << " does not exist.";
            AQ_THROW( msg.str() );
        }
        return etrading::Environment::defaultEnv().deleteObject<etrading::AQObjCurve>(curveName);
    };

    const int tryAqCurveObjectDeleteAll()
    {
        return etrading::Environment::defaultEnv().deleteAllObjects<etrading::AQObjCurve>();
    };

    std::string tryAqCurveObjectSave(	const std::string& aqObjCurveName,
                                    const std::string& fileNameToWriteTo,
                                    const etrading::FileTypeEnum fileType )
    {

        auto& env = etrading::Environment::defaultEnv();
        auto aqObjCurve = env.accessObject<etrading::AQObjCurve>( aqObjCurveName ); // 			etrading::getAQObjCurve(aqObjCurveName);

        if( !aqObjCurve )
        {
            std::ostringstream msg;
            msg << "AQObjCurve " << aqObjCurveName << " does not exist";
            AQ_THROW( msg.str() );
        }
        
        // Append the file extension if missing
        std::string filenameWithExtension = etrading::appendFileExtension( fileNameToWriteTo, fileType );
	
        // Serialize
        aqObjCurve->serialize( etrading::serialize::JSON, etrading::serialize::FILE, filenameWithExtension );
        
        // Check the Serialization file was created
        etrading::checkFileExists( filenameWithExtension, fileType );

        // Return Result
        std::ostringstream msg;
        msg << "Curve " << aqObjCurveName << " was written to file " << filenameWithExtension;
        return msg.str();
        
    };

    std::pair<const bool, std::string> tryAqCurveObjectLoad( const std::string& fileName,
                                                          const etrading::FileTypeEnum fileType )
    {
        // Append the file extension if missing
        std::string filenameWithExtension = etrading::appendFileExtension( fileName, fileType );

        // Check the Serialization file was created
        etrading::checkFileExists( filenameWithExtension, fileType );

        // Deserialize and Cache the Object
        auto cacheInfoOnDeserialization = etrading::deSerializeFromJSON( etrading::serialize::FILE, filenameWithExtension );
        std::string objectName = cacheInfoOnDeserialization.first;

        auto& curveStore = etrading::getObjectStore<etrading::AQObjCurve>( etrading::Environment::DEFAULT_ENV_NAME );
        if( !curveStore.has( objectName ) )
        {
            std::ostringstream msg;
            msg << "File (" << fileName << ") was loaded and read but was not a Curve, found object of type " << toString( cacheInfoOnDeserialization.second );
            AQ_THROW( msg.str() );
        }
        return std::make_pair( true, objectName );
    };



}