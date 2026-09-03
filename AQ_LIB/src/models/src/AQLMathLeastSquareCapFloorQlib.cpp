// Cost function for least-square problems
/*! Implements a cost function using the interface provided by
    the LeastSquareProblem class.
*/

// stl
#include <utility>

#include "AQLMathCapFloor.h"
#include "AQLMathLeastSquareCapFloorQlib.h"

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareCapFloorQlib::AQLMathLeastSquareCapFloorQlib(const vector< AQLMathCapFloor* >& CapFloor_Mkt_, bool vega_weighted, mode mode__, target target_)
:
mn(CapFloor_Mkt_.size()),
mtarget2fit(mn),
mfct2fit(mn),
mWeight(mn, 1.0),
mMode_(mode__),
mTarget_(target_),
mCapFloorMkt(CapFloor_Mkt_),
mCloned(false)
{
    double fct2normalize = 0.0;

    // Get cap/floor price
    gettarget2fit();

    // Get weight
    for(size_t i = 0; i < mn; ++i)
    {
        if(vega_weighted)
        {
            mWeight[i] *= mCapFloorMkt[i]->getVega();
            fct2normalize += mWeight[i] / mn;
        }

        switch(mMode_)
        {
        case Ratio:
            mWeight[i] /= mtarget2fit[i] * mtarget2fit[i];
            break;
        case Sqrt_Ratio:
            mWeight[i] /= mtarget2fit[i];
            break;
        default:
            break;
        }
    }

    for(size_t i = 0; i < mn; ++i)
    {
        mWeight[i] /= fct2normalize;
    }
}

AQLMathLeastSquareCapFloorQlib::AQLMathLeastSquareCapFloorQlib( const AQLMathLeastSquareCapFloorQlib& rhs )
:
mn(rhs.mn),
mtarget2fit(rhs.mtarget2fit),
mfct2fit(rhs.mfct2fit),
mWeight(rhs.mWeight),
mMode_(rhs.mMode_),
mTarget_(rhs.mTarget_)
{
    for( size_t i = 0; i < mn; ++i )
    {
        mCapFloorMkt[i] = rhs.mCapFloorMkt[i] != 0 ? rhs.mCapFloorMkt[i]->clone() : 0;
    }
    mCloned = true;
}

AQLMathLeastSquareCapFloorQlib::~AQLMathLeastSquareCapFloorQlib()
{
    if(mCloned)
    {
        for( size_t i = 0; i < mn; ++i )
        {
            delete mCapFloorMkt[i];
            mCapFloorMkt[i] = 0;
        }
    }
}

// compute the target vector and the values of the function to fit
void AQLMathLeastSquareCapFloorQlib::gettarget2fit() const
{
    for(size_t i = 0; i < mn; ++i )
    {
        mtarget2fit[i] = mCapFloorMkt[i]->getPV();
    }
}

void AQLMathLeastSquareCapFloorQlib::setWeight( const DoubleVector& weight_)
{
    if( mWeight.size() != weight_.size() )
    {
        throw AQLCoreInvalidData("mWeight.size() != weight_.size()",__FILE__,__LINE__);
    }

    mWeight = weight_;
}

AQLMathLeastSquareCapFloorQlib& AQLMathLeastSquareCapFloorQlib::operator =(const AQLMathLeastSquareCapFloorQlib& rhs)
{
    if ( this != &rhs )
    {
        mn = rhs.mn;
        mWeight = rhs.mWeight;
        mtarget2fit = rhs.mtarget2fit;
        mfct2fit = rhs.mfct2fit;
        mMode_ = rhs.mMode_;

        for( size_t i = 0; i < mn; ++i )
        {
            delete mCapFloorMkt[i];
            mCapFloorMkt[i] = rhs.mCapFloorMkt[i] != 0 ? rhs.mCapFloorMkt[i]->clone() : 0;
        }

        mCloned = true;
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareCapFloorVolDiscModelQlib::AQLMathLeastSquareCapFloorVolDiscModelQlib(const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
                                                                           vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
                                                                           bool vega_weighted,
                                                                           mode mode__,
																		   target target_
                                                                          )
:
AQLMathLeastSquareCapFloorQlib( CapFloor_Mkt_, vega_weighted, mode__, target_ ),
mParamV( DoubleVector(4, 0.0) ),
mParamF( DoubleVector(8, 0.0) ),
mCapFloorLMM(CapFloor_LMM_),
mCloned(false)
{
    if( mn != mCapFloorLMM.size() ) throw AQLCoreInvalidData( "CapFloor_LMM_ size is not correct!",__FILE__,__LINE__ );
}

AQLMathLeastSquareCapFloorVolDiscModelQlib::AQLMathLeastSquareCapFloorVolDiscModelQlib( const AQLMathLeastSquareCapFloorVolDiscModelQlib& rhs )
:
AQLMathLeastSquareCapFloorQlib( rhs ),
//paramV(rhs.paramV),
//paramF(rhs.paramF),
mCloned(true)
{
    for( size_t i = 0; i < mn; ++i )
    {
        mCapFloorLMM[i] = rhs.mCapFloorLMM[i] != 0 ? rhs.mCapFloorLMM[i]->clone() : 0;
    }
}

AQLMathLeastSquareCapFloorVolDiscModelQlib::~AQLMathLeastSquareCapFloorVolDiscModelQlib()
{
    if(mCloned)
    {
        for( size_t i = 0; i < mn; ++i )
        {
            delete mCapFloorLMM[i];
            mCapFloorLMM[i] = 0;
        }
    }
}

double AQLMathLeastSquareCapFloorVolDiscModelQlib::value(const QuantLib::Array& x) const
{
    getfct2fit(x);

    double diff_SQ = 0.0;
    double diff;
    for(size_t i = 0; i < mn; ++i)
    {
        diff = mfct2fit[i] - mtarget2fit[i];
        diff_SQ += diff * diff * mWeight[i];
    }
    return sqrt( mn * diff_SQ / ( mn - 1 ) );
}

QuantLib::Array AQLMathLeastSquareCapFloorVolDiscModelQlib::values(const QuantLib::Array& x) const
{
    getfct2fit(x);

    size_t k = 0;
	QuantLib::Array diff_SQs( mn );
    double diff;
    for(size_t i = 0; i < mn; ++i)
    {
        diff = mfct2fit[i] - mtarget2fit[i];
        diff_SQs[k++] = diff * sqrt( mWeight[i] );
    }
    return diff_SQs;
}

// compute the target vector and the values of the function to fit
void AQLMathLeastSquareCapFloorVolDiscModelQlib::getfct2fit(const QuantLib::Array& x) const
{
    // TODO: do nothing????
    DoubleVector paramV(4);
    DoubleVector paramF(8);
    //DoubleVector paramG(4);
    for(size_t i = 0; i < 4; ++i)
    {
        paramV[i] = x[i];
    }
    for(size_t i = 4; i < 12; ++i)
    {
        paramF[i - 4] = x[i];
    }
    //for(size_t i = 12; i < x.size(); ++i)
    //{
    //    G[i - 12] = x[i];
    //}

    //size_t k = 0;
    //for(size_t i = 0; i < n; ++i )
    //{
    //    CapFloor_LMM[i]->Set_Param(paramV, paramF, G);
    //    fct2fit[i] = CapFloor_LMM[i]->Get_PV();
    //}

}

AQLMathLeastSquareCapFloorVolDiscModelQlib& AQLMathLeastSquareCapFloorVolDiscModelQlib::operator =(const AQLMathLeastSquareCapFloorVolDiscModelQlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).AQLMathLeastSquareCapFloorQlib::operator =( rhs );

        for( size_t i = 0; i < mn; ++i )
        {
			// TODO: why?????
//            delete rhs.mCapFloorLMM[i];
//            mCapFloorLMM[i] = rhs.mCapFloorLMM[i]->clone() != 0 ? rhs.mCapFloorLMM[i]->clone() : 0;
            delete mCapFloorLMM[i];
            mCapFloorLMM[i] = rhs.mCapFloorLMM[i] != 0 ? rhs.mCapFloorLMM[i]->clone() : 0;
        }

        mCloned = true;

        //paramV = rhs.paramV;
        //paramF = rhs.paramF;
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareCapFloorVolDiscModelVQlib::AQLMathLeastSquareCapFloorVolDiscModelVQlib(const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
																				   vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																				   bool vega_weighted,
																				   mode mode__,
																				   target target_
		)
:
AQLMathLeastSquareCapFloorVolDiscModelQlib( CapFloor_Mkt_, CapFloor_LMM_, vega_weighted, mode__, target_ )
{
}

AQLMathLeastSquareCapFloorVolDiscModelVQlib::AQLMathLeastSquareCapFloorVolDiscModelVQlib( const AQLMathLeastSquareCapFloorVolDiscModelVQlib& rhs )
:
AQLMathLeastSquareCapFloorVolDiscModelQlib( rhs )
{
}

AQLMathLeastSquareCapFloorVolDiscModelVQlib::~AQLMathLeastSquareCapFloorVolDiscModelVQlib()
{
}

// compute the target vector and the values of the function to fit
void AQLMathLeastSquareCapFloorVolDiscModelVQlib::getfct2fit(const QuantLib::Array& x) const
{
    //
    DoubleVector paramV(4);
    for(size_t i = 0; i < 4; ++i)
    {
        paramV[i] = x[i];
    }

    for(size_t i = 0; i < mn; ++i )
    {
        //under construction;
        mCapFloorLMM[i]->setParamV(paramV);
        mfct2fit[i] = mCapFloorLMM[i]->getPV();
    }
}

AQLMathLeastSquareCapFloorVolDiscModelVQlib& AQLMathLeastSquareCapFloorVolDiscModelVQlib::operator =(const AQLMathLeastSquareCapFloorVolDiscModelVQlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).AQLMathLeastSquareCapFloorVolDiscModelQlib::operator =( rhs );
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareCapFloorVolDiscModelFQlib::AQLMathLeastSquareCapFloorVolDiscModelFQlib(const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
																				   vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																				   double T_max_,
																				   bool vega_weighted,
																				   mode mode__,
																				   target target_
		)
:
AQLMathLeastSquareCapFloorVolDiscModelQlib( CapFloor_Mkt_, CapFloor_LMM_, vega_weighted, mode__, target_ ),
mT_max(T_max_)
{
}

AQLMathLeastSquareCapFloorVolDiscModelFQlib::AQLMathLeastSquareCapFloorVolDiscModelFQlib( const AQLMathLeastSquareCapFloorVolDiscModelFQlib& rhs )
:
AQLMathLeastSquareCapFloorVolDiscModelQlib( rhs ),
mT_max( rhs.mT_max )
{
}

AQLMathLeastSquareCapFloorVolDiscModelFQlib::~AQLMathLeastSquareCapFloorVolDiscModelFQlib()
{
}

// compute the target vector and the values of the function to fit
void AQLMathLeastSquareCapFloorVolDiscModelFQlib::getfct2fit(const QuantLib::Array& x) const
{
    //
    DoubleVector paramF(9);
    for(size_t i = 0; i < 8; ++i)
    {
        paramF[i] = x[i];
    }
    paramF[8] = mT_max;

    for(size_t i = 0; i < mn; ++i )
    {
        mCapFloorLMM[i]->setParamF(paramF);
        mfct2fit[i] = mCapFloorLMM[i]->getPV();
    }
}

AQLMathLeastSquareCapFloorVolDiscModelFQlib& AQLMathLeastSquareCapFloorVolDiscModelFQlib::operator =(const AQLMathLeastSquareCapFloorVolDiscModelFQlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).AQLMathLeastSquareCapFloorVolDiscModelQlib::operator =( rhs );

        mT_max = rhs.mT_max;
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareCapFloorVolDiscModelGQlib::AQLMathLeastSquareCapFloorVolDiscModelGQlib(const vector< AQLMathCapFloor* >& CapFloor_Mkt_,
																				   vector< AQLMathCapFloorVolLMMDiscModel* >& CapFloor_LMM_,
																				   bool vega_weighted,
																				   mode mode__,
																				   target target_
		)
:
AQLMathLeastSquareCapFloorVolDiscModelQlib( CapFloor_Mkt_, CapFloor_LMM_, vega_weighted, mode__, target_ )
{
}

AQLMathLeastSquareCapFloorVolDiscModelGQlib::AQLMathLeastSquareCapFloorVolDiscModelGQlib( const AQLMathLeastSquareCapFloorVolDiscModelGQlib& rhs )
:
AQLMathLeastSquareCapFloorVolDiscModelQlib( rhs )
{
}

AQLMathLeastSquareCapFloorVolDiscModelGQlib::~AQLMathLeastSquareCapFloorVolDiscModelGQlib()
{
}

// compute the target vector and the values of the function to fit
void AQLMathLeastSquareCapFloorVolDiscModelGQlib::getfct2fit(const QuantLib::Array& x) const
{
    //
    DoubleVector paramG(x.size(), 1.0);
    for(size_t i = 0; i < x.size(); ++i)
    {
        paramG[i] = x[i];
    }

    for(size_t i = 0; i < mn; ++i )
    {
        mCapFloorLMM[i]->setParamG(paramG);
        mfct2fit[i] = mCapFloorLMM[i]->getPV();
    }
}

AQLMathLeastSquareCapFloorVolDiscModelGQlib& AQLMathLeastSquareCapFloorVolDiscModelGQlib::operator =(const AQLMathLeastSquareCapFloorVolDiscModelGQlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).AQLMathLeastSquareCapFloorVolDiscModelQlib::operator =( rhs );
    }
    return *this;
}
