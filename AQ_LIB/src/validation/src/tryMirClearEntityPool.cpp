#include "tryMirClearEntityPool.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "AQLDefinitions.h"
#include "AQLCoreDataService.h"
#include "LAUpdateStaticDataManager.h"

namespace validation
{
    /* @brief			validation interface for the mirClearEntityPool function, to clear the object pool
    *  @return			A notification string
    */
    AQLString tryMirClearEntityPool()
    {
        VALID_EXCEPTION_START

        AQLDataInstance* dataInstance = etrading::getDataInstance();
        dataInstance->getObjectPool().clear();
        AQLCoreDataService::finalize();
        etrading::LAUpdateStaticDataManager::setUpForIRServer();
        AQLCoreDataService::setContext( CONTEXT_KEY_ISSETCURVEID, "TRUE" );
        etrading::LAUpdateStaticDataManager::setUpDefaultIRStaticData( *dataInstance );

        AQLString ret( "AllEntityPoolCleared" );
        return ret;

        VALID_EXCEPTION_END
    }

}