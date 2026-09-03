/*
 * @brief			validation interface for the mirClearEntityPool function
 * @Created:		27 April 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "tryMirClearEntityPool.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "LADefinitions.h"
#include "LACoreDataService.h"
#include "LAUpdateStaticDataManager.h"

namespace validation_api
{
    /* @brief			validation interface for the mirClearEntityPool function, to clear the object pool
    *  @return			A notification string
    */
    LAString tryMirClearEntityPool()
    {
        VALID_EXCEPTION_START

        LADataInstance* dataInstance = etrading::getDataInstance();
        dataInstance->getObjectPool().clear();
        LACoreDataService::finalize();
        etrading::LAUpdateStaticDataManager::setUpForIRServer();
        LACoreDataService::setContext( CONTEXT_KEY_ISSETCURVEID, "TRUE" );
        etrading::LAUpdateStaticDataManager::setUpDefaultIRStaticData( *dataInstance );

        LAString ret( "AllEntityPoolCleared" );
        return ret;

        VALID_EXCEPTION_END
    }

}