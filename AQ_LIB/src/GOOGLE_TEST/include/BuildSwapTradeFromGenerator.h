// BuildSwaptradeFromGenerator.h

/*
 * @brief			Build Swap Trade From Generator
 * @Created:		8th August 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "LACoreTemplateType.h"

namespace google_test
{
    // Function to Build a Swap Expression Label Value Block / LAStringMatrix
    LAStringMatrix buildSwapExpression( const std::string & effectiveDate,
                                      const std::string & maturityDateOrTenor,
                                      const std::string & payReceive       =  "PAY",           // Default PAY
                                      const std::string & notional         = "1000000",        // Default 1,000,000
                                      const std::string & rateOrSpread1    = "0.0",            // Default 0.0%
                                      const std::string & rateOrSpread2    = "0.0",            // Default 0.0 bps
                                      const std::string & firstFixing1     = std::string(),    // Default Empty String
                                      const std::string & firstFixing2     = std::string(),    // Default Empty String
                                      const std::string & lastFixing1      = std::string(),    // Default Empty String
                                      const std::string & lastFixing2      = std::string(),    // Default Empty String
                                      const std::string & isFwdInter1      = std::string(),    // Default Empty String
                                      const std::string & isFwdInter2      = std::string() );  // Default Empty String

    // Function to Create a Swap from a Swap Generator
    std::string createSwapFromGenerator( const std::string & swapName,
                                         const std::string & swapGeneratorName,
                                         const std::string & effectiveDate,
                                         const std::string & maturityDateOrTenor,
                                         const std::string & payReceive       =  "PAY",           // Default PAY
                                         const std::string & notional         = "1000000",        // Default 1,000,000
                                         const std::string & rateOrSpread1    = "0.00",           // Default 0.0%
                                         const std::string & rateOrSpread2    = "0.0",            // Default 0.0 bps
                                         const std::string & firstFixing1     = std::string(),    // Default Empty String
                                         const std::string & firstFixing2     = std::string(),    // Default Empty String
                                         const std::string & lastFixing1      = std::string(),    // Default Empty String
                                         const std::string & lastFixing2      = std::string(),    // Default Empty String
                                         const std::string & isFwdInter1      = std::string(),    // Default Empty String
                                         const std::string & isFwdInter2      = std::string() );  // Default Empty String

    // Function to Create an IRS from a Swap Generator
    std::string createSwapCalibrationInstrument( const std::string & swapName, const std::string & swapGeneratorName, const std::string & effectiveDate, const std::string & maturityDateOrTenor );

	// Build a LWO swap object from a given test file representing a LWO swap
	void createSwapFromDataFile(const char* swapInputs);
}