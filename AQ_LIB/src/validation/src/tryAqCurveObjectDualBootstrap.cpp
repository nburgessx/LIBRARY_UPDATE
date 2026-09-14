#include "tryAqCurveObjectDualBootstrap.h"

#include "RecordMacros.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "CoreEnumerations.h"
#include "AQObjUtilities.h"
#include "DualBootstrappedCurveObject.h"
#include "CurveResultsContainer.h"

#include <string>
#include <set>
#include <algorithm>
#include <sstream>
#include "ExceptionMacros.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation
{
    /* @brief Perform dual bootstrapping to build OIS and Swap curves
	 * @param [in] objectName				The name of the dual-bootstrapped object
	 * @param [in] curveCollection			Curve collection to which calibrated curves belong
	 * @param [in] swapCurveGeneratorName   The name of the Swap curve generator that defines Swap curve's conventions
	 * @param [in] oisCurveGeneratorName   The name of the OIS curve generator that defines OIS curve's conventions
	 * @param [in] aqObjSwapMarketObj			Object that encapsulates all of Swap curve's market data
	 * @param [in] aqObjOISMarketObj			Object that encapsulates all of OIS curve's market data
	 * @param [in] commonParams				A collection of parameters common across curves
	 *                                      
	 * @param [out]                         The curve indexes
	 */
    std::map<std::string, std::string> tryAqCurveObjectDualBootstrap(	const std::string& objectName,
																	const std::string& curveCollection,
																	const std::string& swapCurveGeneratorName,
																	const std::string& oisCurveGeneratorName,
																	const std::string& aqObjSwapMarketObj,
																	const std::string& aqObjOISMarketObj,
																	const AQLStringMatrix& commonParams)
    {
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( objectName, curveCollection, swapCurveGeneratorName, oisCurveGeneratorName, aqObjSwapMarketObj, aqObjOISMarketObj, commonParams);

        // AQObj Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

		// Perform initial basic sanity checks
		if ( swapCurveGeneratorName.size() == 0 )
		{
			AQ_THROW( ( "Missing swapCurveGeneratorName input" ) );
		}

		if ( oisCurveGeneratorName.size() == 0 )
		{
			AQ_THROW( ( "Missing oisCurveGeneratorName input" ) );
		}

		if ( aqObjSwapMarketObj.size() == 0 )
		{
			AQ_THROW( ( "Missing aqObjSwapMarketObj input" ) );
		}

		if ( aqObjOISMarketObj.size() == 0 )
		{
			AQ_THROW( ( "Missing aqObjOISMarketObj input" ) );
		}
		
		// Create the Curve object and store in the cache

		etrading::DualBootstrappedCurveObject curveObject( objectName, curveCollection, swapCurveGeneratorName, oisCurveGeneratorName, aqObjSwapMarketObj, aqObjOISMarketObj);
		curveObject.setEngineParams(commonParams);
		curveObject.calibrateCurve();

        etrading::copyToCache<etrading::DualBootstrappedCurveObject>( curveObject );

		std::string curveIndex_ois = curveObject.getOISCurveIndexNames();
		std::string curveIndex_swap = curveObject.getSwapCurveIndexNames();

		std::ostringstream oisSwapMsg;
		oisSwapMsg << "OIS: \"" << curveIndex_ois << "\" Swap: \"" << curveIndex_swap << "\"";
		AQ_RECORD_DECORATED_OUTPUTS( objectName.c_str(), "", oisSwapMsg.str().c_str() );

		std::map<std::string, std::string> ret;
		ret["OIS"] = curveIndex_ois;
		ret["Swap"] = curveIndex_swap;

		return ret;

		VALID_EXCEPTION_END
    };


}