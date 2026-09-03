// tryMeLWOCurveDisplay.h

/*
 * @brief			validation interface for the meLWOCurveDisplay method
 * @Created:		13th August 2018
 * @Author:			Nicholas Burgess
 * @Department:		MHI Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "Variant.h"
#include "LabelValueBlock.h"

#include <string>
#include <vector>


namespace validation_api
{
	
    /* @brief Displays market data and conventions used for curve calibration
    * @param [in] curveObjectName      The CurveGenerator object you wish to display
    * @param [out]                     A VariantMatrix containing a LabelValue block of curve market data and conventions.
    */
	etrading::VariantMatrix tryMeLWOCurveDisplay( const std::string& curveObjectName );
}
