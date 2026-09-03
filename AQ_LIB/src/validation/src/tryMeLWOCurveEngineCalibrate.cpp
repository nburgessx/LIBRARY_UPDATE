/*
* @brief			validation interface for the meLWOCurveEngineCalibrate method
* @Created:			10 May 2018
* @Author:			Joseph Ye
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/

#include "tryMeLWOCurveEngineCalibrate.h"

#include "RecordMacros.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "CoreEnumerations.h"
#include "LWOUtilities.h"
#include "MultiCurveObject.h"

#include <string>
#include <set>
#include <algorithm>
#include <boost/format.hpp>
#include "ExceptionMacros.h"
#include "CurveResultsContainer.h"

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation_api
{
	/* @brief Calibrate global yield curve engine to generate multiple synchronous yield curves
	* @param [in] engineObjectName			The name of the yield curve engine object
	* @param [in] engineSettings			A collection of parameters to do with the curve engine's operations
	* @param [in] curveCollection			Curve collection to which calibrated curves belong
	* @param [in] curveGeneratorNames		The list of curve generator names
	* @param [in] marketDataObjects			The list of market data objects that correponds curve generators
	* @param [out]    The curve indexes
	*/
	LAStringVector tryMeLWOCurveEngineCalibrate(const std::string& engineObjectName,
												const std::string& curveCollection,
												const LAStringMatrix& engineSettings,
												const std::vector<std::string>& curveGeneratorNames,
												const std::vector<std::string>& marketDataObjects)
    {
		VALID_EXCEPTION_START
	
        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        MLIB_CLEAR_CURVE_RESULTS_CACHE

		if (curveGeneratorNames.size() != marketDataObjects.size())
		{
			std::string error = (boost::format("#Error: Number of curve generators (%i) does not match number of market data objects (%i).")
				% curveGeneratorNames.size()
				% marketDataObjects.size()).str();
			MLIB_THROW(error);
		}

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file(decorateFilename("tryMeLWOCurveEngineCalibrate_inputs", engineObjectName.c_str()));
			file.write("generatorFunction", "tryMeLWOCurveEngineCalibrate");
			file.write("objectName", engineObjectName);

			// Write out each marketDataType and correspondiong block of market data
			for (unsigned int i = 0; i < curveGeneratorNames.size(); i++)
			{
				file.write(curveGeneratorNames[i].c_str(), marketDataObjects[i]);
			}
		}
		
		if (curveGeneratorNames.size() == 0 || marketDataObjects.size() == 0)
		{
			MLIB_THROW("Please provide at least one set of curve generator and market data to the yield curve engine");
		}

		// Create the Curve object and store in the cache
		etrading::MultiCurveObject curveObject(engineObjectName, curveGeneratorNames, marketDataObjects, engineSettings, curveCollection);
		curveObject.calibrateCurve();

        etrading::copyToCache<etrading::MultiCurveObject>( curveObject );

		LAStringVector curveIndexes = curveObject.getCurveIndexNames();

		return curveIndexes;

		VALID_EXCEPTION_END
    };


}