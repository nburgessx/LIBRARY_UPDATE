// ExtractCurveCalibrationData.h

/*
 * @brief			Helper methods to extract yield curve calibration inputs
 * @Created:		8th August 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "tryMeLWOCurveMarketData.h"

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
