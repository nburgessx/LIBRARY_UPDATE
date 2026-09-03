
#include "AQLMathLeastSquareSwaptionCapFloor.h"
#include "AQLMathLeastSquareSwaptionCapFloorQlib.h"
#include "AQLMathLeastSquareSwaptionVol.h"
#include "AQLMathLeastSquareCapFloor.h"

AQLMathLeastSquareSwaptionCapFloor::AQLMathLeastSquareSwaptionCapFloor(AQLMathLeastSquareSwaptionVol& swaption_vol_,
																 AQLMathLeastSquareCapFloor& capfloor_,
																 const DoubleVector& weight_
		)
{
	mCostFunction = std::make_shared<AQLMathLeastSquareSwaptionCapFloorQlib>(
			*dynamic_pointer_cast<AQLMathLeastSquareSwaptionVolQlib>(swaption_vol_.getCostFunction()),
		   	*dynamic_pointer_cast<AQLMathLeastSquareCapFloorQlib>(capfloor_.getCostFunction()),
		   	weight_);
}

AQLMathLeastSquareSwaptionCapFloor::AQLMathLeastSquareSwaptionCapFloor( const AQLMathLeastSquareSwaptionCapFloor& rhs )
{
	mCostFunction = (rhs.mCostFunction != nullptr ? std::shared_ptr<QuantLib::CostFunction>(dynamic_pointer_cast<AQLMathLeastSquareSwaptionCapFloorQlib>(rhs.mCostFunction)->clone()) : std::shared_ptr<QuantLib::CostFunction>() );
}

AQLMathLeastSquareSwaptionCapFloor::~AQLMathLeastSquareSwaptionCapFloor()
{
}

AQLMathLeastSquareSwaptionCapFloor* AQLMathLeastSquareSwaptionCapFloor::clone() const { return new AQLMathLeastSquareSwaptionCapFloor(*this); }


AQLMathLeastSquareSwaptionCapFloor& AQLMathLeastSquareSwaptionCapFloor::operator =(const AQLMathLeastSquareSwaptionCapFloor& rhs)
{
    if( this != &rhs )
    {
		*mCostFunction = *rhs.mCostFunction;
    }
    return *this;
}

void AQLMathLeastSquareSwaptionCapFloor::setWeight( const DoubleVector& weight_ )
{
   	dynamic_pointer_cast<AQLMathLeastSquareSwaptionCapFloorQlib>(mCostFunction)->setWeight(weight_);
}
