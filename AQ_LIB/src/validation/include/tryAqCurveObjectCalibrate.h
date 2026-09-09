#pragma once

#include "Variant.h"

#include <string>
#include <vector>


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
    std::string tryAqCurveObjectCalibrate(	const std::string& objectName,
										const std::string& aqObjCurveGeneratorName,
										const std::string& aqObjCurveMarketDataName,
										const std::string& domesticCurveCollection,
										const std::string& foreignCurveCollection );
}
