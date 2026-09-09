// BuildSwaptradeFromGenerator.cpp

/*
 * @brief			Build Swap Trade From Generator
 */

#include "BuildSwapTradeFromGenerator.h"
#include "ReadDataFile.h"
#include "tryAqSwapObjectCreation.h"

using etrading::ReadDataFile;

namespace google_test
{
    // Function to Build a Swap Expression Label Value Block / AQLStringMatrix
    AQLStringMatrix buildSwapExpression( const std::string & effectiveDate,
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
        AQLStringVector effective_     = { AQLString("EffectiveDate"),  effectiveDate.c_str() };
        AQLStringVector maturity_      = { AQLString("MaturityDate"),   maturityDateOrTenor.c_str() };
        AQLStringVector payReceive_    = { AQLString("PayReceive"),     payReceive.c_str() };
        AQLStringVector notional_      = { AQLString("Notional"),       notional.c_str() };
        AQLStringVector rateOrSpread1_ = { AQLString("RateOrSpread1"),  rateOrSpread1.c_str() };
        AQLStringVector rateOrSpread2_ = { AQLString("RateOrSpread2"),  rateOrSpread2.c_str() };
        AQLStringVector firstFixing1_  = { AQLString("FirstFixing1"),   firstFixing1.c_str() };
        AQLStringVector firstFixing2_  = { AQLString("FirstFixing2"),   firstFixing2.c_str() };
        AQLStringVector lastFixing1_   = { AQLString("LastFixing1"),    lastFixing1.c_str() };
        AQLStringVector lastFixing2_   = { AQLString("LastFixing2"),    lastFixing2.c_str() };
        AQLStringVector isFwdInter1_   = { AQLString("isFwdInter1"),    isFwdInter1.c_str() };
        AQLStringVector isFwdInter2_   = { AQLString("isFwdInter2"),    isFwdInter2.c_str() };


        // Build the Swap Expression LVB
        AQLStringMatrix swapExpressionLVB( 12 );
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
        AQLStringMatrix swapExpression = buildSwapExpression( effectiveDate,
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
        std::string swapObjectName = validation::tryAqSwapObjectCreateFromGenerator( swapName,
                                                                                      swapGeneratorName,
                                                                                      etrading::LabelValueBlock( swapExpression ),
                                                                                      etrading::LabelValueBlock( AQLStringMatrix() ),
                                                                                      false,    // isXccySwap
                                                                                      true );   // validateKeys
        return swapObjectName;
    }


    // Function to Create an IRS from a Swap Generator
    std::string createSwapCalibrationInstrument( const std::string & swapName, const std::string & swapGeneratorName, const std::string & effectiveDate, const std::string & maturityDateOrTenor )
    {
        // Build Swap Expression LVB - This LVB contains dynamic swap information: start date, end date, tenor, notional, fixed rate and the libor spread.
        AQLStringMatrix swapExpression = buildSwapExpression( effectiveDate, maturityDateOrTenor );

        // Create Swap from Swap Generator
        std::string swapObjectName = validation::tryAqSwapObjectCreateFromGenerator( swapName,
                                                                                      swapGeneratorName,
                                                                                      etrading::LabelValueBlock( swapExpression ),
                                                                                      etrading::LabelValueBlock( AQLStringMatrix() ),
                                                                                      false,    // isXccySwap
                                                                                      true );   // validateKeys
        return swapObjectName;
    }

	/* @brief			Build a AQObj swap object from a given test file representing a AQObj swap
	*  @param [in]		swapInputs			The filename specifying the swap file
	*/
	void createSwapFromDataFile(const char* swapInputs)
	{
		const ReadDataFile::Load swapInputFile(swapInputs);
		std::string swapName = swapInputFile["swapName"];
		AQLStringMatrix swapLvb = swapInputFile["swapLVB"];
		AQLStringMatrix swapProperties = swapInputFile["swapPropertiesLVB"];
		bool isXccySwap = swapInputFile["isXccySwap"];
		bool validateKeys = swapInputFile["validateKeys"];

		validation::tryAqSwapObjectCreate(swapName, swapLvb, swapProperties, isXccySwap, validateKeys);
	}

}