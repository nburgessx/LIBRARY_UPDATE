/*
 * @brief			validation interface for the meLWOCurveCalibrate method
 * @Created:		5 May 2017
 * @Author:			Ian Castleton
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "Variant.h"

#include <string>
#include <vector>


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
										const std::string& foreignCurveCollection );
}
