// BondTypes.h

/* 
 * @brief			Collection of utility methods for Bond Struct Management
 * @Created:		14 March 2016 
 * @Author:			Nicholas Burgess
 * @Department:		MHI Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"
#include "BondTypes.h"                  // For Bond Curve Fitting - Swig Struct Definitions
#include "BondCurves.h"                 // For Bond Curve Fitting - Struct Definition: NelsonSiegelSvenssonParameters
#include "NelsonSiegelFitting.h"        // For Bond Curve Fitting - Struct Definition: NelsonSiegelSvenssonCalibrationResults
#include "PolynomialFitting.h"          // For Bond Curve Fitting - Struct Definition: PolynomialCalibrationResults

class LAString;

namespace swig
{
    //
    // Swig utility methods for the Bond Type Methods
    //
	swig::NelsonSiegelSvenssonParameters toSwigNelsonSiegelSvenssonParameters( const etrading::NelsonSiegelSvenssonParameters& p );
    etrading::NelsonSiegelSvenssonParameters fromSwigNelsonSiegelSvenssonParameters( const swig::NelsonSiegelSvenssonParameters& p );

    swig::NelsonSiegelSvenssonCalibrationResults toSwigNelsonSiegelSvenssonCalibrationResults( const etrading::NelsonSiegelSvenssonCalibrationResults& r );
    etrading::NelsonSiegelSvenssonCalibrationResults fromSwigNelsonSiegelSvenssonCalibrationResults( const swig::NelsonSiegelSvenssonCalibrationResults& r );
   
    swig::PolynomialCalibrationResults toSwigPolynomialCalibrationResults( const etrading::PolynomialCalibrationResults& r );
    etrading::PolynomialCalibrationResults fromSwigPolynomialCalibrationResults( const swig::PolynomialCalibrationResults& r );

}


