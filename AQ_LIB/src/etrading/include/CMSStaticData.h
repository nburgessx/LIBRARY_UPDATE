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
		AQLString getSwapGeneratorName1() const;
		AQLString getSwapIndexMaturity1() const;
		double getSwapIndexMultiplier1() const;

		// Getter methods for fetching the parameters of underlying swap index 2
		// Note that the CMS instrument will only contain values for swap index 2 if it is a CMS spread.
		AQLString getSwapGeneratorName2() const;
		AQLString getSwapIndexMaturity2() const;
		double getSwapIndexMultiplier2() const;

	private:
		AQLString swapGeneratorName1_;
		AQLString swapIndexMaturity1_;
		double swapIndexMultiplier1_;

		AQLString swapGeneratorName2_;
		AQLString swapIndexMaturity2_;
		double swapIndexMultiplier2_;

    };

}
