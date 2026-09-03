/*
 * @brief			Class the defines the Constant Maturity Swap configuration parameters
 *					The purpose of this class is to store the CMS underlying SwapIndex parameters.
 * @Created:		14 March 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "CMSStaticData.h"

namespace etrading
{
	CMSStaticData::CMSStaticData() : LegStaticData(), swapGeneratorName1_(""), swapIndexMaturity1_("")
													, swapGeneratorName2_(""), swapIndexMaturity2_("")
	{}

    CMSStaticData::CMSStaticData( const LabelValueBlock& marketDataLVB ) : LegStaticData(marketDataLVB)
    {
		const std::string inputLVB = "legPropertiesLVB";

		swapGeneratorName1_   = marketDataLVB.getCompulsoryValueAsLAString( CMS_KEY::CMS_GENERATOR_NAME1, inputLVB );
		swapIndexMaturity1_   = marketDataLVB.getCompulsoryValueAsLAString( CMS_KEY::CMS_INDEX_MATURITY1, inputLVB );
		swapIndexMultiplier1_ = marketDataLVB.getCompulsoryValueAsDouble( CMS_KEY::CMS_INDEX_MULTIPLIER1, inputLVB );

		// The second CMS SwapIndex is optional
		// If present, we are pricing a CMS Spread. if missing, we are pricing a regular CMS>
		swapGeneratorName2_   = marketDataLVB.getOptionalValueAsLAString( CMS_KEY::CMS_GENERATOR_NAME2, "" );
		swapIndexMaturity2_   = marketDataLVB.getOptionalValueAsLAString( CMS_KEY::CMS_INDEX_MATURITY2, "" );
		swapIndexMultiplier2_ = marketDataLVB.getOptionalValueAsDouble( CMS_KEY::CMS_INDEX_MULTIPLIER2, 0. );
	}

    LegStaticDataPtr CMSStaticData::clone()
    {
        LegStaticDataPtr data = LegStaticDataPtr( new CMSStaticData( *this ));
        return data;
    }

    CMSStaticData::CMSStaticData( const CMSStaticData& rhs ) : LegStaticData( rhs )
																, swapGeneratorName1_( rhs.swapGeneratorName1_ ), swapIndexMaturity1_( rhs.swapIndexMaturity1_ )
																, swapGeneratorName2_( rhs.swapGeneratorName2_ ), swapIndexMaturity2_( rhs.swapIndexMaturity2_ )
    {}

	LAString CMSStaticData::getSwapGeneratorName1() const
    {
        return swapGeneratorName1_;
    }
	LAString CMSStaticData::getSwapIndexMaturity1() const
    {
        return swapIndexMaturity1_;
    }
	double CMSStaticData::getSwapIndexMultiplier1() const
	{
		return swapIndexMultiplier1_;
	}

	LAString CMSStaticData::getSwapGeneratorName2() const
    {
        return swapGeneratorName2_;
    }
	LAString CMSStaticData::getSwapIndexMaturity2() const
    {
        return swapIndexMaturity2_;
    }
	double CMSStaticData::getSwapIndexMultiplier2() const
	{
		return swapIndexMultiplier2_;
	}

}

