#include "tryAqObjCurvesDualBootstrap.h"

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
#include <boost/format.hpp>
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
    std::map<std::string, std::string> tryAqObjCurvesDualBootstrap(	const std::string& objectName,
																	const std::string& curveCollection,
																	const std::string& swapCurveGeneratorName,
																	const std::string& oisCurveGeneratorName,
																	const std::string& aqObjSwapMarketObj,
																	const std::string& aqObjOISMarketObj,
																	const AQLStringMatrix& commonParams)
    {
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback
        RECORD_INPUTS( objectName, curveCollection, swapCurveGeneratorName, oisCurveGeneratorName, aqObjSwapMarketObj, aqObjOISMarketObj, commonParams);

        // AQObj Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

		// Perform initial basic sanity checks
		if ( swapCurveGeneratorName.size() == 0 )
		{
			throw AQLCoreInvalidData(( "#Error: Missing swapCurveGeneratorName input" ), __FILE__, __LINE__ );
		}

		if ( oisCurveGeneratorName.size() == 0 )
		{
			throw AQLCoreInvalidData(( "#Error: Missing oisCurveGeneratorName input" ), __FILE__, __LINE__ );
		}

		if ( aqObjSwapMarketObj.size() == 0 )
		{
			throw AQLCoreInvalidData(( "#Error: Missing aqObjSwapMarketObj input" ), __FILE__, __LINE__ );
		}

		if ( aqObjOISMarketObj.size() == 0 )
		{
			throw AQLCoreInvalidData(( "#Error: Missing aqObjOISMarketObj input" ), __FILE__, __LINE__ );
		}
		
		// Create the Curve object and store in the cache

		etrading::DualBootstrappedCurveObject curveObject( objectName, curveCollection, swapCurveGeneratorName, oisCurveGeneratorName, aqObjSwapMarketObj, aqObjOISMarketObj);
		curveObject.setEngineParams(commonParams);
		curveObject.calibrateCurve();

        etrading::copyToCache<etrading::DualBootstrappedCurveObject>( curveObject );

		std::string curveIndex_ois = curveObject.getOISCurveIndexNames();
		std::string curveIndex_swap = curveObject.getSwapCurveIndexNames();

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryAqObjCurvesDualBootstrap_outputs", objectName.c_str()));
			file.write("output", (boost::format("OIS: \"%s\" Swap: \"%s\"") % curveIndex_ois % curveIndex_swap).str().c_str());
		}

		std::map<std::string, std::string> ret;
		ret["OIS"] = curveIndex_ois;
		ret["Swap"] = curveIndex_swap;

		return ret;

		VALID_EXCEPTION_END
    };


}