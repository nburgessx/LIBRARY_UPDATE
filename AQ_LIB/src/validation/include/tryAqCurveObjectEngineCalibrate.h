#pragma once

#include "Variant.h"

#include <string>
#include <vector>
#include "AQLCoreTemplateType.h"

namespace validation
{
    /* @brief Calibrate global yield curve engine to generate multiple synchronous yield curves
	 * @param [in] engineObjectName			The name of the yield curve engine object
	 * @param [in] engineSettings			A collection of parameters to do with the curve engine's operations
	 * @param [in] curveCollection			Curve collection to which calibrated curves belong
	 * @param [in] curveGeneratorNames		The list of curve generator names
	 * @param [in] marketDataObjects		The list of market data objects that correponds curve generators
	 * @param [out]                         The curve indexes
	 */
	AQLStringVector tryAqCurveObjectEngineCalibrate(const std::string& engineObjectName,
																	const std::string& curveCollection,
																	const AQLStringMatrix& engineSettings,
																	const std::vector<std::string>& curveGeneratorNames,
																	const std::vector<std::string>& marketDataObjects);
}
