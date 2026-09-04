#pragma once

#include "Variant.h"

#include <string>
#include <vector>
#include "AQLCoreTemplateType.h"

namespace validation
{
    /* @brief Perform dual bootstrapping to build OIS and Swap curves
	 * @param [in] objectName				The name of the dual-bootstrapped object
	 * @param [in] curveCollection			Curve collection to which calibrated curves belong
	 * @param [in] swapCurveGeneratorName   The name of the Swap curve generator that defines Swap curve's conventions
	 * @param [in] oisCurveGeneratorName    The name of the OIS curve generator that defines OIS curve's conventions
	 * @param [in] lwoSwapMarketObj			Object that encapsulates all of Swap curve's market data
	 * @param [in] lwoOISMarketObj			Object that encapsulates all of OIS curve's market data
	 * @param [in] commonParams				A collection of parameters common across curves
	 *                                      
	 * @param [out]                         The curve indexes
	 */
	std::map<std::string, std::string> tryAqObjectsCurveDualBootstrap(	const std::string& objectName,
																	const std::string& curveCollection,
																	const std::string& swapCurveGeneratorName,
																	const std::string& oisCurveGeneratorName,
																	const std::string& lwoSwapMarketObj,
																	const std::string& lwoOISMarketObj,
																	const AQLStringMatrix& commonParams = AQLStringMatrix());
}
