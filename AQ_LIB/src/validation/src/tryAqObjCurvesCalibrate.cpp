#include "tryAqObjCurvesCalibrate.h"
#include "AQLDateScheduleHelpers.h"
#include "AQLUpdateStaticDataManager.h"
#include "CurveValidation.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "CoreEnumerations.h"
#include "RecordMacros.h"
#include "AQObjUtilities.h"

#include <string>
#include <set>
#include <algorithm>
#include <boost/format.hpp>
#include "ExceptionMacros.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation
{
    /* @brief Builds a curve using a CurveGenerator object and a CurveMarketData object
	 * @param [in] aqObjCurveGeneratorName     The name of the AQObjCurveGenerator object to use
	 * @param [in] aqObjCurveMarketDataName    The name of the AQObjCurveMarketData object to use
	 * @param [in] domesticCurveCollection   The curveCollection this curve will be stored in ( The Target CurveCollection )
	 * @param [in] foreignCurveCollection    The curveCollection containing foreign curve dependencies ( The Against CurveCollection )
	 *                                       Allowed to be blank if this is a single CCY curve (OIS, STD, TenorBasis)
	 * @param [out]                          The curve build status
	 */
    std::string tryAqObjCurvesCalibrate(	const std::string& objectName,
										const std::string& aqObjCurveGeneratorName,
										const std::string& aqObjCurveMarketDataName,
										const std::string& domesticCurveCollection,
										const std::string& foreignCurveCollection )
    {
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback - Note the first argument is the decorated file prefix and the second the suffix
        AQ_RECORD_DECORATED_INPUTS( objectName, "", objectName, aqObjCurveGeneratorName, aqObjCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );

		// Perform initial basic sanity checks
		if ( aqObjCurveGeneratorName.size() == 0 )
		{
				throw AQLCoreInvalidData(	( "#Error: Missing aqObjCurveGenerator name" ), __FILE__, __LINE__ );
		}
		if ( aqObjCurveMarketDataName.size() == 0 )
		{
				throw AQLCoreInvalidData(	( "#Error: Missing aqObjCurveMarketData name" ), __FILE__, __LINE__ );
		}
		if ( domesticCurveCollection.size() == 0 )
		{
			throw AQLCoreInvalidData(	( "#Error: Missing domesticCurveCollection name" ), __FILE__, __LINE__ );
		}

		// Create the Curve object and store in the cache
		etrading::SingleCurveObject curveObject( objectName, aqObjCurveGeneratorName, aqObjCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
		curveObject.calibrateCurve();
        etrading::copyToCache<etrading::SingleCurveObject>( curveObject );

		std::string curveIndex = curveObject.getCurveIndexName();

        // Record Outputs for logs, tests and playback - Note the first argument is the decorated file prefix and second the suffix
        AQ_RECORD_DECORATED_OUTPUTS( objectName, "", curveIndex );
		
        return curveIndex;

		VALID_EXCEPTION_END
    };

}