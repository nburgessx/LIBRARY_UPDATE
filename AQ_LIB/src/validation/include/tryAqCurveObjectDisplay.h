// tryAqCurveObjectDisplay.h

/*
 * @brief			validation interface for the aqCurveObjectDisplay method
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
	etrading::VariantMatrix tryAqCurveObjectDisplay( const std::string& curveObjectName );
}
