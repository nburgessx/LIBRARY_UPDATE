#include "FeeStaticData.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "SwapValidation.h"

namespace etrading
{
    FeeStaticData::FeeStaticData( const LabelValueBlock& marketDataLVB ) 
    {
  		inputParameters_ = marketDataLVB;

        const std::string inputLVB = "legPropertiesLVB";

		//cast it to upper case so that legName lookup is not case-sensitive (i.e Leg3:Fee, leg3:fee, LEG3:FEE are the same)
        legName_ = marketDataLVB.getCompulsoryValueAsAQLString(IRS_KEY::LEG_TYPE,  inputLVB, true);

        discountCurve_ = marketDataLVB.getCompulsoryValueAsAQLString( MARKET_KEY::DISCOUNT_CURVE, inputLVB );
        currency_ = toCCYEnum( marketDataLVB.getOptionalValueAsAQLString( IRS_KEY::CURRENCY ).getCString());
        valuationCurrency_ = toCCYEnum( marketDataLVB.getOptionalValueAsAQLString( IRS_KEY::VALUATION_CURRENCY).getCString());
    }

    LegStaticDataPtr FeeStaticData::clone()
    {
        LegStaticDataPtr data = LegStaticDataPtr(new FeeStaticData(*this));
        return data;
    }

    FeeStaticData::FeeStaticData(const FeeStaticData& rhs) : LegStaticData(rhs)
    {}

}

