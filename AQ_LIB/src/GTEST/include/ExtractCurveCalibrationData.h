// ExtractCurveCalibrationData.h

/*
 * @brief			Helper methods to extract yield curve calibration inputs
 */

#pragma once
#include "tryAqCurveMarketData.h"

namespace google_test
{
        // Function to get curve calibrationData
        etrading::VariantVector curveCalibrationData( const std::string & curveObject,
                                                      const etrading::CurveMarketDataEnum & marketDataEnum,
                                                      const unsigned int & columnNumber );


        // Function to get calibration data from a curve - Basis Swap Tenors
        std::vector<std::string> curveCalibrationOisSwapTenors( const std::string & curveObject );
        
        // Function to get calibration data from a curve - Basis Swap Spreads
        std::vector<double> curveCalibrationOisSwapParRates(const std::string & curveObject );
        
        // Function to get calibration data from a curve - Basis Swap Tenors
        std::vector<std::string> curveCalibrationLiborOisTenors( const std::string & curveObject );
        
        // Function to get calibration data from a curve - Basis Swap Spreads
        std::vector<double> curveCalibrationLiborOisSpreads(const std::string & curveObject );

        // Function to get calibration data from a curve - Swap Tenors
        std::vector<std::string> curveCalibrationSwapTenors( const std::string & curveObject );
        
        // Function to get calibration data from a curve - Swap Par Rates
        std::vector<double> curveCalibrationSwapParRates(const std::string & curveObject );
    
        // Function to get calibration data from a curve - Basis Swap Tenors
        std::vector<std::string> curveCalibrationBasisSwapTenors( const std::string & curveObject );
        
        // Function to get calibration data from a curve - Basis Swap Spreads
        std::vector<double> curveCalibrationBasisSwapSpreads(const std::string & curveObject );

}
