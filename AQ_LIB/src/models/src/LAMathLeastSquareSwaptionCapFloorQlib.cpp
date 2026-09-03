
#include "LAMathLeastSquareSwaptionCapFloorQlib.h"
#include "LAMathLeastSquareSwaptionVolQlib.h"
#include "LAMathLeastSquareCapFloorQlib.h"

LAMathLeastSquareSwaptionCapFloorQlib::LAMathLeastSquareSwaptionCapFloorQlib(LAMathLeastSquareSwaptionVolQlib& swaption_vol_,
																		 LAMathLeastSquareCapFloorQlib& capfloor_,
																		 const DoubleVector& weight_
		)
:
mWeight(weight_),
mCapfloor(&capfloor_),
mSwaptionVol(&swaption_vol_),
mCloned(false)
{
    if( mWeight.size() != 2 ) throw LACoreInvalidData("mWeight.size() != 2",__FILE__,__LINE__);
}

LAMathLeastSquareSwaptionCapFloorQlib::LAMathLeastSquareSwaptionCapFloorQlib( const LAMathLeastSquareSwaptionCapFloorQlib& rhs )
:
mWeight(rhs.mWeight),
mCapfloor(rhs.mCapfloor != 0 ? rhs.mCapfloor->clone() : 0),
mSwaptionVol(rhs.mSwaptionVol != 0 ? rhs.mSwaptionVol->clone() : 0),
mCloned(true)
{
}

LAMathLeastSquareSwaptionCapFloorQlib::~LAMathLeastSquareSwaptionCapFloorQlib()
{
    if(mCloned)
    {
        delete mSwaptionVol;
        mSwaptionVol = 0;

        delete mCapfloor;
        mCapfloor = 0;
    }
}

LAMathLeastSquareSwaptionCapFloorQlib* LAMathLeastSquareSwaptionCapFloorQlib::clone() const { return new LAMathLeastSquareSwaptionCapFloorQlib(*this); }

double LAMathLeastSquareSwaptionCapFloorQlib::value(const QuantLib::Array& x) const
{
    double cap_value = mCapfloor->value(x);

    double swap_value = mSwaptionVol->value(x);

    return sqrt( mWeight[0] * cap_value * cap_value + mWeight[1] * swap_value * swap_value );
}

QuantLib::Array LAMathLeastSquareSwaptionCapFloorQlib::values(const QuantLib::Array& x) const
{   
//    size_t num_target1 = swaption_vol-
    //
	QuantLib::Array diff_SQs1 = mCapfloor->values(x);
    QuantLib::Array diff_SQs2 = mSwaptionVol->values(x);

    //
    size_t num_target1 = mCapfloor->getNumTarget2Fit();
    size_t num_target2 = mSwaptionVol->getNumTarget2Fit();

	QuantLib::Array diff_SQs( num_target1 + num_target2 );
    size_t i = 0;
    for(; i < num_target1; ++i)
    {
        diff_SQs[i] = sqrt( mWeight[0] ) * diff_SQs1[i];
    }

    for(; i < num_target1 + num_target2; ++i)
    {
        diff_SQs[i] = sqrt( mWeight[1] ) * diff_SQs2[i - num_target1];
    }

    return diff_SQs;
}

LAMathLeastSquareSwaptionCapFloorQlib& LAMathLeastSquareSwaptionCapFloorQlib::operator =(const LAMathLeastSquareSwaptionCapFloorQlib& rhs)
{
    if( this != &rhs )
    {
        delete mSwaptionVol;
        mSwaptionVol = rhs.mSwaptionVol != 0 ? rhs.mSwaptionVol->clone() : 0;
        
        delete mCapfloor; 
        mCapfloor = rhs.mCapfloor != 0 ? rhs.mCapfloor->clone() : 0;
        mWeight = rhs.mWeight;

        mCloned = true;
    }
    return *this;
}    
