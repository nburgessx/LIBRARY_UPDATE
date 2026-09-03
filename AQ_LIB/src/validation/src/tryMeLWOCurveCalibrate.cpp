/*
 * @brief			validation interface for the meLWOCurveCalibrate method
 * @Created:		5 May 2017
 * @Author:			Ian Castleton
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "tryMeLWOCurveCalibrate.h"
#include "LADateScheduleHelpers.h"
#include "LAUpdateStaticDataManager.h"
#include "CurveValidation.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "CoreEnumerations.h"
#include "RecordMacros.h"
#include "LWOUtilities.h"

#include <string>
#include <set>
#include <algorithm>
#include <boost/format.hpp>
#include "ExceptionMacros.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation_api
{
    /* @brief Builds a curve using a CurveGenerator object and a CurveMarketData object
	 * @param [in] lwoCurveGeneratorName     The name of the LWOCurveGenerator object to use
	 * @param [in] lwoCurveMarketDataName    The name of the LWOCurveMarketData object to use
	 * @param [in] domesticCurveCollection   The curveCollection this curve will be stored in ( The Target CurveCollection )
	 * @param [in] foreignCurveCollection    The curveCollection containing foreign curve dependencies ( The Against CurveCollection )
	 *                                       Allowed to be blank if this is a single CCY curve (OIS, STD, TenorBasis)
	 * @param [out]                          The curve build status
	 */
    std::string tryMeLWOCurveCalibrate(	const std::string& objectName,
										const std::string& lwoCurveGeneratorName,
										const std::string& lwoCurveMarketDataName,
										const std::string& domesticCurveCollection,
										const std::string& foreignCurveCollection )
    {
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback - Note the first argument is the decorated file prefix and the second the suffix
        RECORD_DECORATED_INPUTS( objectName, "", objectName, lwoCurveGeneratorName, lwoCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );

		// Perform initial basic sanity checks
		if ( lwoCurveGeneratorName.size() == 0 )
		{
				throw LACoreInvalidData(	( "#Error: Missing lwoCurveGenerator name" ), __FILE__, __LINE__ );
		}
		if ( lwoCurveMarketDataName.size() == 0 )
		{
				throw LACoreInvalidData(	( "#Error: Missing lwoCurveMarketData name" ), __FILE__, __LINE__ );
		}
		if ( domesticCurveCollection.size() == 0 )
		{
			throw LACoreInvalidData(	( "#Error: Missing domesticCurveCollection name" ), __FILE__, __LINE__ );
		}

		// Create the Curve object and store in the cache
		etrading::SingleCurveObject curveObject( objectName, lwoCurveGeneratorName, lwoCurveMarketDataName, domesticCurveCollection, foreignCurveCollection );
		curveObject.calibrateCurve();
        etrading::copyToCache<etrading::SingleCurveObject>( curveObject );

		std::string curveIndex = curveObject.getCurveIndexName();

        // Record Outputs for logs, tests and playback - Note the first argument is the decorated file prefix and second the suffix
        RECORD_DECORATED_OUTPUTS( objectName, "", curveIndex );
		
        return curveIndex;

		VALID_EXCEPTION_END
    };

}