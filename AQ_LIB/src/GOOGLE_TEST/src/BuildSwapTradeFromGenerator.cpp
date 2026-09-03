// BuildSwaptradeFromGenerator.cpp

/*
 * @brief			Build Swap Trade From Generator
 * @Created:		8th August 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub London Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "BuildSwapTradeFromGenerator.h"
#include "ReadDataFile.h"
#include "tryMeLWOSwapCreation.h"

using etrading::ReadDataFile;

namespace google_test
{
    // Function to Build a Swap Expression Label Value Block / LAStringMatrix
    LAStringMatrix buildSwapExpression( const std::string & effectiveDate,
                                      const std::string & maturityDateOrTenor,
                                      const std::string & payReceive,
                                      const std::string & notional,
                                      const std::string & rateOrSpread1,
                                      const std::string & rateOrSpread2,
                                      const std::string & firstFixing1,
                                      const std::string & firstFixing2,
                                      const std::string & lastFixing1,
                                      const std::string & lastFixing2,
                                      const std::string & isFwdInter1,
                                      const std::string & isFwdInter2 )
    {
        LAStringVector effective_     = { LAString("EffectiveDate"),  effectiveDate.c_str() };
        LAStringVector maturity_      = { LAString("MaturityDate"),   maturityDateOrTenor.c_str() };
        LAStringVector payReceive_    = { LAString("PayReceive"),     payReceive.c_str() };
        LAStringVector notional_      = { LAString("Notional"),       notional.c_str() };
        LAStringVector rateOrSpread1_ = { LAString("RateOrSpread1"),  rateOrSpread1.c_str() };
        LAStringVector rateOrSpread2_ = { LAString("RateOrSpread2"),  rateOrSpread2.c_str() };
        LAStringVector firstFixing1_  = { LAString("FirstFixing1"),   firstFixing1.c_str() };
        LAStringVector firstFixing2_  = { LAString("FirstFixing2"),   firstFixing2.c_str() };
        LAStringVector lastFixing1_   = { LAString("LastFixing1"),    lastFixing1.c_str() };
        LAStringVector lastFixing2_   = { LAString("LastFixing2"),    lastFixing2.c_str() };
        LAStringVector isFwdInter1_   = { LAString("isFwdInter1"),    isFwdInter1.c_str() };
        LAStringVector isFwdInter2_   = { LAString("isFwdInter2"),    isFwdInter2.c_str() };


        // Build the Swap Expression LVB
        LAStringMatrix swapExpressionLVB( 12 );
        swapExpressionLVB[0]        = effective_;
        swapExpressionLVB[1]        = maturity_;
        swapExpressionLVB[2]        = payReceive_;
        swapExpressionLVB[3]        = notional_;
        swapExpressionLVB[4]        = rateOrSpread1_;
        swapExpressionLVB[5]        = rateOrSpread2_;
        swapExpressionLVB[6]        = firstFixing1_;
        swapExpressionLVB[7]        = firstFixing2_;
        swapExpressionLVB[8]        = lastFixing1_;
        swapExpressionLVB[9]        = lastFixing2_;
        swapExpressionLVB[10]       = isFwdInter1_;
        swapExpressionLVB[11]       = isFwdInter2_;
        
        return swapExpressionLVB;
    }


    // Function to Create a Swap from a Swap Generator
    std::string createSwapFromGenerator( const std::string & swapName,
                                         const std::string & swapGeneratorName,
                                         const std::string & effectiveDate,
                                         const std::string & maturityDateOrTenor,
                                         const std::string & payReceive,    
                                         const std::string & notional,      
                                         const std::string & rateOrSpread1, 
                                         const std::string & rateOrSpread2, 
                                         const std::string & firstFixing1,  
                                         const std::string & firstFixing2,  
                                         const std::string & lastFixing1,   
                                         const std::string & lastFixing2,   
                                         const std::string & isFwdInter1,   
                                         const std::string & isFwdInter2 )
    {
        // Build Swap Expression LVB
        LAStringMatrix swapExpression = buildSwapExpression( effectiveDate,
                                                           maturityDateOrTenor,
                                                           payReceive,
                                                           notional,
                                                           rateOrSpread1,
                                                           rateOrSpread2,
                                                           firstFixing1,
                                                           firstFixing2,
                                                           lastFixing1,
                                                           lastFixing2,
                                                           isFwdInter1,
                                                           isFwdInter2 );

        // Create Swap from Swap Generator
        std::string swapObjectName = validation_api::tryMeLWOSwapCreateFromGenerator( swapName,
                                                                                      swapGeneratorName,
                                                                                      etrading::LabelValueBlock( swapExpression ),
                                                                                      etrading::LabelValueBlock( LAStringMatrix() ),
                                                                                      false,    // isXccySwap
                                                                                      true );   // validateKeys
        return swapObjectName;
    }


    // Function to Create an IRS from a Swap Generator
    std::string createSwapCalibrationInstrument( const std::string & swapName, const std::string & swapGeneratorName, const std::string & effectiveDate, const std::string & maturityDateOrTenor )
    {
        // Build Swap Expression LVB - This LVB contains dynamic swap information: start date, end date, tenor, notional, fixed rate and the libor spread.
        LAStringMatrix swapExpression = buildSwapExpression( effectiveDate, maturityDateOrTenor );

        // Create Swap from Swap Generator
        std::string swapObjectName = validation_api::tryMeLWOSwapCreateFromGenerator( swapName,
                                                                                      swapGeneratorName,
                                                                                      etrading::LabelValueBlock( swapExpression ),
                                                                                      etrading::LabelValueBlock( LAStringMatrix() ),
                                                                                      false,    // isXccySwap
                                                                                      true );   // validateKeys
        return swapObjectName;
    }

	/* @brief			Build a LWO swap object from a given test file representing a LWO swap
	*  @param [in]		swapInputs			The filename specifying the swap file
	*/
	void createSwapFromDataFile(const char* swapInputs)
	{
		const ReadDataFile::Load swapInputFile(swapInputs);
		std::string swapName = swapInputFile["swapName"];
		LAStringMatrix swapLvb = swapInputFile["swapLVB"];
		LAStringMatrix swapProperties = swapInputFile["swapPropertiesLVB"];
		bool isXccySwap = swapInputFile["isXccySwap"];
		bool validateKeys = swapInputFile["validateKeys"];

		validation_api::tryMeLWOSwapCreate(swapName, swapLvb, swapProperties, isXccySwap, validateKeys);
	}

}