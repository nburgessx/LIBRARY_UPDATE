#include "tryAqObjectsCurveCalibrate.h"
#include "AQLDateScheduleHelpers.h"
#include "AQLUpdateStaticDataManager.h"
#include "CurveValidation.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "CoreEnumerations.h"
#include "RecordMacros.h"
#include "AQOUtilities.h"

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
	 * @param [in] aqoCurveGeneratorName     The name of the AQOCurveGenerator object to use
	 * @param [in] aqoCurveMarketDataName    The name of the AQOCurveMarketData object to use
	 * @param [in] domesticCurveCollection   The curveCollection this curve will be stored in ( The Target CurveCollection )
	 * @param [in] foreignCurveCollection    The curveCollection containing foreign curve dependencies ( The Against CurveCollection )
	 *                                       Allowed to be blank if this is a single CCY curve (OIS, STD, TenorBasis)
	 * @param [out]                          The curve build status
	 */
    std::string tryAqObjectsCurveCalibrate(	const std::string& objectName,
										const std::string& aqoCurveGeneratorName,
										const std::string& aqoCurveMarketDataName,
										const std::string& domesticCurveCollection,
										const std::string& foreignCurveCollection )
    {
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback - Note the first argument is the decorated file prefix and the second the suffix
        RECORD_DECORATED_INPUTS( objectName, "", objectName, aqoCurveGeneratorName, aqoCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );

		// Perform initial basic sanity checks
		if ( aqoCurveGeneratorName.size() == 0 )
		{
				throw AQLCoreInvalidData(	( "#Error: Missing aqoCurveGenerator name" ), __FILE__, __LINE__ );
		}
		if ( aqoCurveMarketDataName.size() == 0 )
		{
				throw AQLCoreInvalidData(	( "#Error: Missing aqoCurveMarketData name" ), __FILE__, __LINE__ );
		}
		if ( domesticCurveCollection.size() == 0 )
		{
			throw AQLCoreInvalidData(	( "#Error: Missing domesticCurveCollection name" ), __FILE__, __LINE__ );
		}

		// Create the Curve object and store in the cache
		etrading::SingleCurveObject curveObject( objectName, aqoCurveGeneratorName, aqoCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
		curveObject.calibrateCurve();
        etrading::copyToCache<etrading::SingleCurveObject>( curveObject );

		std::string curveIndex = curveObject.getCurveIndexName();

        // Record Outputs for logs, tests and playback - Note the first argument is the decorated file prefix and second the suffix
        RECORD_DECORATED_OUTPUTS( objectName, "", curveIndex );
		
        return curveIndex;

		VALID_EXCEPTION_END
    };

}