/*
 * @brief			Class the defines the Constant Maturity Swap configuration parameters
 *					The purpose of this class is to store the CMS underlying SwapIndex parameters.
 * @Created:		14 March 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include "LegStaticData.h"

namespace etrading
{
    class CMSStaticData : public LegStaticData
    {
    public:

		CMSStaticData();
        CMSStaticData( const LabelValueBlock& marketDataLVB );
        CMSStaticData(const CMSStaticData& rhs);
        virtual ~CMSStaticData() {};

        LegStaticDataPtr clone();

		// Getter methods for fetching the parameters of underlying swap index 1
		LAString getSwapGeneratorName1() const;
		LAString getSwapIndexMaturity1() const;
		double getSwapIndexMultiplier1() const;

		// Getter methods for fetching the parameters of underlying swap index 2
		// Note that the CMS instrument will only contain values for swap index 2 if it is a CMS spread.
		LAString getSwapGeneratorName2() const;
		LAString getSwapIndexMaturity2() const;
		double getSwapIndexMultiplier2() const;

	private:
		LAString swapGeneratorName1_;
		LAString swapIndexMaturity1_;
		double swapIndexMultiplier1_;

		LAString swapGeneratorName2_;
		LAString swapIndexMaturity2_;
		double swapIndexMultiplier2_;

    };

}
