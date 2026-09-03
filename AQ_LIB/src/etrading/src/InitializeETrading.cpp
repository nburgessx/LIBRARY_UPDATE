#include "InitializeETrading.h"
#include "FolderConfig.h"
#include "LibSetUpETrading.h"
#include "LAUpdateStaticDataManager.h"
#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLStaticDataImport.h"
#include "LACurveForwardRateHelpers.h"
#include "CurveCalibrationData.h"
#include "ExceptionMacros.h"
#include <boost/thread/mutex.hpp>

namespace etrading
{
    InitializeETrading* InitializeETrading::instance_ = nullptr;

    namespace 
    {
	    boost::mutex instanceProtector;
    }

    InitializeETrading::InitializeETrading(const bool checkStaticDataLoaded, const bool checkIfCalendarLoaded) : dataInstance_(new AQLDataInstance())
    {
	    // moved from LibSetUp/initialize;
	    libSetUpETrading(dataInstance_.get(), checkIfCalendarLoaded);
	    LAUpdateStaticDataManager::setUpForIRServer();
	    AQLCoreDataService::setContext(CONTEXT_KEY_ISSETCURVEID, "TRUE");
	    LAUpdateStaticDataManager::setUpDefaultIRStaticData(*dataInstance_);
	    if(checkStaticDataLoaded) checkIfStaticDataLoaded();
		FolderConfig::setupOptionalStartupConfig();
    }

    InitializeETrading::~InitializeETrading()
    {
	    // from mirClearEntityPoolAndReadProperty
	    dataInstance_->getObjectPool().clear();
	    AQLCoreDataService::finalize();
    }

    InitializeETrading& InitializeETrading::instance(const bool checkStaticDataLoaded, const bool checkIfCalendarLoaded)
    {
        boost::mutex::scoped_lock guard(instanceProtector);
	    if (instance_ == nullptr)
        {
            // checkIfStaticDataLoaded is an inline function MAStaticDataLoaded.h
		    instance_ = new InitializeETrading(checkStaticDataLoaded, checkIfCalendarLoaded);
	    }
        else if(checkStaticDataLoaded)
        {
            checkIfStaticDataLoaded();
        }
	    return *instance_;
    }

    void InitializeETrading::destroyInstance()
    {
	    boost::mutex::scoped_lock guard(instanceProtector);
        delete instance_;
        instance_ = nullptr;
    }


    //
    // Fuction to return a pointer to the ycProperties object used in the object pool
    //
    // curveCollection:             CurveID, e.g. USDYC
    // throwIfCurveDoesNotExist:    Disallows the creation of a new ycProperties object when true. Defaults to true, which is the typical user-case
    //
    CurveCalibrationData* InitializeETrading::ycStaticDataObject( const AQLString& curveCollection, const bool throwIfCurveDoesNotExist )
    {
        // Result Place Holder
        CurveCalibrationData* ycProperties = nullptr;

        // Decorated Curve Collection ID used in the object pool
        AQLString ycPropertiesCurveID = etrading::LACurveForwardRateHelpers::YIELD_CURVE_PRO_NAME_PREFIX + curveCollection;

        // Get the curve data object holder
        AQLObjectHolder ehCurve = instance_->dataInstance()->getObjectPool().getObject( ycPropertiesCurveID );
        
        // Get the ycProperties from the object pool or create a new one if is empty / not defined
        if( !ehCurve.isDefined() )
        {
            AQ_REQUIRE( !throwIfCurveDoesNotExist, "Invalid Curve: CurveCollection '" + curveCollection + "' does not exist" );
            
            // Create and Set a new ycProperties object if it doesn't exist
            ycProperties = new CurveCalibrationData( instance_->dataInstance() );
			instance_->dataInstance()->getObjectPool().set( ycPropertiesCurveID, ycProperties );
            return ycProperties;
        }
        else
        {
            // Get ycProperties from the object pool
            ycProperties = &dynamic_cast<CurveCalibrationData&>( ehCurve.get() );
            return ycProperties;
        }

        // We should never reach here
        return ycProperties;
    }
}
