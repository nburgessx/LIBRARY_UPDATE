// tryAqObjCurvesDisplay.h

/*
 * @brief			validation interface for the aqObjCurvesDisplay method
 */

#pragma once

#include "Variant.h"
#include "LabelValueBlock.h"

#include <string>
#include <vector>


namespace validation
{
	
    /* @brief Displays market data and conventions used for curve calibration
    * @param [in] curveObjectName      The CurveGenerator object you wish to display
    * @param [out]                     A VariantMatrix containing a LabelValue block of curve market data and conventions.
    */
	etrading::VariantMatrix tryAqObjCurvesDisplay( const std::string& curveObjectName );
}
