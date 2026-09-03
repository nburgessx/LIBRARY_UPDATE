
#include "LAMathLeastSquareSwaptionCapFloor.h"
#include "LAMathLeastSquareSwaptionCapFloorQlib.h"
#include "LAMathLeastSquareSwaptionVol.h"
#include "LAMathLeastSquareCapFloor.h"

LAMathLeastSquareSwaptionCapFloor::LAMathLeastSquareSwaptionCapFloor(LAMathLeastSquareSwaptionVol& swaption_vol_,
																 LAMathLeastSquareCapFloor& capfloor_,
																 const DoubleVector& weight_
		)
{
	mCostFunction = std::make_shared<LAMathLeastSquareSwaptionCapFloorQlib>(
			*dynamic_pointer_cast<LAMathLeastSquareSwaptionVolQlib>(swaption_vol_.getCostFunction()),
		   	*dynamic_pointer_cast<LAMathLeastSquareCapFloorQlib>(capfloor_.getCostFunction()),
		   	weight_);
}

LAMathLeastSquareSwaptionCapFloor::LAMathLeastSquareSwaptionCapFloor( const LAMathLeastSquareSwaptionCapFloor& rhs )
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<LAMathLeastSquareSwaptionCapFloorQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

LAMathLeastSquareSwaptionCapFloor::~LAMathLeastSquareSwaptionCapFloor()
{
}

LAMathLeastSquareSwaptionCapFloor* LAMathLeastSquareSwaptionCapFloor::clone() const { return new LAMathLeastSquareSwaptionCapFloor(*this); }


LAMathLeastSquareSwaptionCapFloor& LAMathLeastSquareSwaptionCapFloor::operator =(const LAMathLeastSquareSwaptionCapFloor& rhs)
{
    if( this != &rhs )
    {
		*mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}

void LAMathLeastSquareSwaptionCapFloor::setWeight( const DoubleVector& weight_ )
{
   	dynamic_pointer_cast<LAMathLeastSquareSwaptionCapFloorQlib>(mCostFunction)->setWeight(weight_);
}
