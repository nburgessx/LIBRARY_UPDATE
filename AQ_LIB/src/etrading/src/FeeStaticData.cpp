/*
 * @brief			Class the defines the leg market data
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


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
        legName_ = marketDataLVB.getCompulsoryValueAsLAString(IRS_KEY::LEG_TYPE,  inputLVB, true);

        discountCurve_ = marketDataLVB.getCompulsoryValueAsLAString( MARKET_KEY::DISCOUNT_CURVE, inputLVB );
        currency_ = toCCYEnum( marketDataLVB.getOptionalValueAsLAString( IRS_KEY::CURRENCY ).getCString());
        valuationCurrency_ = toCCYEnum( marketDataLVB.getOptionalValueAsLAString( IRS_KEY::VALUATION_CURRENCY).getCString());
    }

    LegStaticDataPtr FeeStaticData::clone()
    {
        LegStaticDataPtr data = LegStaticDataPtr(new FeeStaticData(*this));
        return data;
    }

    FeeStaticData::FeeStaticData(const FeeStaticData& rhs) : LegStaticData(rhs)
    {}

}

