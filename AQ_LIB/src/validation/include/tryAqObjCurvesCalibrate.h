#pragma once

#include "Variant.h"

#include <string>
#include <vector>


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
    std::string tryAqObjCurvesCalibrate(	const std::string& objectName,
										const std::string& aqoCurveGeneratorName,
										const std::string& aqoCurveMarketDataName,
										const std::string& domesticCurveCollection,
										const std::string& foreignCurveCollection );
}
