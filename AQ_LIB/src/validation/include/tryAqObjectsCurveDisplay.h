// tryAqObjectsCurveDisplay.h

/*
 * @brief			validation interface for the aqObjectsCurveDisplay method
 * @Created:		13th August 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
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
	etrading::VariantMatrix tryAqObjectsCurveDisplay( const std::string& curveObjectName );
}
