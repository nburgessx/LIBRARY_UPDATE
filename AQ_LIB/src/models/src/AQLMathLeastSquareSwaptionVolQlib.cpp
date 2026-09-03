// Cost function for least-square problems
// Implements a cost function using the interface provided by the LeastSquareProblem class.

#include <utility>

#include "AQLMathLeastSquareSwaptionVolQlib.h"
#include "AQLMathSwaptionVolLMM.h"
#include "AQLModelUtilities.h"

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareSwaptionVolQlib::AQLMathLeastSquareSwaptionVolQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
															   mode mode__,
															   target target_
//                                                    const vector< vector< pair<double, double> > >& OptMat_and_Expire_
		)
:
//x_prev(0),
mSwaptionTargetValue(Swaption_target_value_Mkt_),
mn(mSwaptionTargetValue.size()),
mm(mSwaptionTargetValue[0].size()),
mUseThisTarget( mn, vector<bool>(mm, true) ),
mfct2fit( mn, DoubleVector(mm) ),
mWeight( mn, DoubleVector(mm, 1.0) ),
mMode_(mode__),
mTarget_(target_)
{
    mNumTarget = mn * mm;

    // setup weight
    for(size_t i = 0; i < mn; ++i)
    {
        for(size_t j = 0; j < mm; ++j)
        {
            mWeight[i][j] = 1.0 / static_cast<double>( mNumTarget );

            switch(mMode_)
            {
            case Ratio:
				mWeight[i][j] /= mSwaptionTargetValue[i][j] * mSwaptionTargetValue[i][j];
                break;
            case Sqrt_Ratio:
				mWeight[i][j] /= mSwaptionTargetValue[i][j];
                break;
            default:
                // Diff
                break;
            }
        }
    }
}

AQLMathLeastSquareSwaptionVolQlib::AQLMathLeastSquareSwaptionVolQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
															   const DoubleMatrix& weight_,
															   mode mode__,
															   target target_
//                                                    const vector< vector< pair<double, double> > >& OptMat_and_Expire_
		)
:
mSwaptionTargetValue(Swaption_target_value_Mkt_),
mn(mSwaptionTargetValue.size()),
mm(mSwaptionTargetValue[0].size()),
mUseThisTarget( mn, vector<bool>(mm, false) ),
mfct2fit( mn, DoubleVector(mm) ),
mWeight( weight_ ),
mMode_(mode__),
mTarget_(target_)
{
    if( mWeight.size() != mn)
    {
        throw AQLCoreInvalidData("mWeight.size() != mSwaptionTargetValue.size()",__FILE__,__LINE__);
    }

    if( mWeight[0].size() != mm )
    {
        throw AQLCoreInvalidData("mWeight[0].size() != mSwaptionTargetValue[0].size()",__FILE__,__LINE__);
    }

    mNumTarget = 0;
    for(size_t i = 0; i < mn; ++i)
    {
        for(size_t j = 0; j < mm; ++j)
        {
            if( !AQLModelUtilities::eq(mWeight[i][j], 0.0) )
            {
                ++mNumTarget;
                mUseThisTarget[i][j] = true;
            }
        }
    }
    if( mNumTarget == 0 ) throw AQLCoreInvalidData( "mNumTarget == 0",__FILE__,__LINE__);


    // setup weight
    double fct2normalize = 0.0;
    for(size_t i = 0; i < mn; ++i)
    {
        for(size_t j = 0; j < mm; ++j)
        {
            if( mUseThisTarget[i][j] )
            {
                mWeight[i][j] /= static_cast<double>( mNumTarget );
                fct2normalize += mWeight[i][j];

                switch(mMode_)
                {
                case Ratio:
					mWeight[i][j] /= mSwaptionTargetValue[i][j] * mSwaptionTargetValue[i][j];
                    break;
                case Sqrt_Ratio:
					mWeight[i][j] /= mSwaptionTargetValue[i][j];
                    break;
                default:
                    // Diff
                    break;
                }
            }
        }
    }

    for(size_t i = 0; i < mn; ++i)
    {
        for(size_t j = 0; j < mm; ++j)
        {
            if( mUseThisTarget[i][j] ) mWeight[i][j] /= fct2normalize;
        }
    }
}


AQLMathLeastSquareSwaptionVolQlib::AQLMathLeastSquareSwaptionVolQlib( const AQLMathLeastSquareSwaptionVolQlib& rhs )
:
//x_prev(rhs.x_prev),
mSwaptionTargetValue(rhs.mSwaptionTargetValue),
//OptMat_and_Expire(rhs.OptMat_and_Expire),
mn(rhs.mn),
mm(rhs.mm),
mUseThisTarget(rhs.mUseThisTarget),
mNumTarget(rhs.mNumTarget),
mfct2fit(rhs.mfct2fit),
mWeight(rhs.mWeight),
mMode_(rhs.mMode_),
mTarget_(rhs.mTarget_)
//diff_SQ(rhs.diff_SQ),
//diff_SQs(rhs.diff_SQs)
{
}

AQLMathLeastSquareSwaptionVolQlib::~AQLMathLeastSquareSwaptionVolQlib()
{
}

void AQLMathLeastSquareSwaptionVolQlib::setMode(string mode__)
{
    if( mode__ == "Diff" )
    {
        mMode_ = Diff;
    }
    else if( mode__ == "Ratio" )
    {
        mMode_ = Ratio;
    }
    else if( mode__ == "Sqrt_Ratio" )
    {
        mMode_ = Sqrt_Ratio;
    }
    else
    {
        mMode_ = Diff;
    }
}

AQLMathLeastSquareSwaptionVolQlib& AQLMathLeastSquareSwaptionVolQlib::operator =(const AQLMathLeastSquareSwaptionVolQlib& rhs)
{
    if ( this != &rhs )
    {
        mSwaptionTargetValue = rhs.mSwaptionTargetValue;

        mfct2fit = rhs.mfct2fit;

        mn = rhs.mn;
        mm = rhs.mm;
        mWeight = rhs.mWeight;

        mUseThisTarget = rhs.mUseThisTarget;
        mNumTarget = rhs.mNumTarget;

        mMode_ = rhs.mMode_;
		mTarget_ = rhs.mTarget_;
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareSwaptionVolDiscModelQlib::AQLMathLeastSquareSwaptionVolDiscModelQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
																				  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																				  mode mode__,
																				  target target_
		)
:
AQLMathLeastSquareSwaptionVolQlib( Swaption_target_value_Mkt_, mode__, target_ ),
mParamV( DoubleVector(4, 0.0) ),
mParamF( DoubleVector(8, 0.0) ),
mSwaptionVol( Swaption_Vol_ ),
mCloned(false)
{
    if( mn * mm != Swaption_Vol_.size() )
    {
        AQLString msg = "Swaption_vol and Swaption_Vol_ size is not consistent!";
        throw AQLCoreInvalidData( msg.getCString(),__FILE__,__LINE__ );
    }
}

AQLMathLeastSquareSwaptionVolDiscModelQlib::AQLMathLeastSquareSwaptionVolDiscModelQlib( const DoubleMatrix& Swaption_target_value_Mkt_,
																				  vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																				  const DoubleMatrix& weight_,
																				  mode mode__,
																				  target target_
		)
:
AQLMathLeastSquareSwaptionVolQlib( Swaption_target_value_Mkt_, weight_, mode__, target_ ),
mParamV( DoubleVector(4, 0.0) ),
mParamF( DoubleVector(8, 0.0) ),
mSwaptionVol( Swaption_Vol_ ),
mCloned(false)
{
    if( mn * mm != Swaption_Vol_.size() )
    {
        AQLString msg = "Swaption_vol and Swaption_Vol_ size is not consistent!";
        throw AQLCoreInvalidData( msg.getCString(),__FILE__,__LINE__ );
    }
}

AQLMathLeastSquareSwaptionVolDiscModelQlib::AQLMathLeastSquareSwaptionVolDiscModelQlib( const AQLMathLeastSquareSwaptionVolDiscModelQlib& rhs )
:
AQLMathLeastSquareSwaptionVolQlib( rhs ),
mCloned(true)
{
    for( size_t i = 0; i < mSwaptionVol.size(); ++i )
    {
        mSwaptionVol[i] = rhs.mSwaptionVol[i] != 0 ? rhs.mSwaptionVol[i]->clone() : 0;
    }
}

AQLMathLeastSquareSwaptionVolDiscModelQlib::~AQLMathLeastSquareSwaptionVolDiscModelQlib()
{
    if(mCloned)
    {
        for( size_t i = 0; i < mSwaptionVol.size(); ++i )
        {
            delete mSwaptionVol[i];
            mSwaptionVol[i] = 0;
        }
    }
}

double AQLMathLeastSquareSwaptionVolDiscModelQlib::value(const QuantLib::Array& x) const
{
    getfct2fit(x);

    double diff_SQ = 0.0;
    double diff;
    for(size_t i = 0; i < mn; ++i)
    {
        for(size_t j = 0; j < mm; ++j)
        {
            if( mUseThisTarget[i][j] )
            {
				diff = mfct2fit[i][j] - mSwaptionTargetValue[i][j];
                diff_SQ += diff * diff * mWeight[i][j];
            }
        }
    }
    return sqrt( mNumTarget * diff_SQ / ( mNumTarget - 1 ) );
    //return sqrt( diff_SQ );
    //return diff_SQ;
}

QuantLib::Array AQLMathLeastSquareSwaptionVolDiscModelQlib::values(const QuantLib::Array& x) const
{
    getfct2fit(x);

    size_t k = 0;
	QuantLib::Array diff_SQs( mNumTarget );
    double diff;
    for(size_t i = 0; i < mn; ++i)
    {
        for(size_t j = 0; j < mm; ++j)
        {
            if( mUseThisTarget[i][j] )
            {
				diff = mfct2fit[i][j] - mSwaptionTargetValue[i][j];
                diff_SQs[k++] = diff * sqrt( mWeight[i][j] );
            }
        }
    }

    return diff_SQs;
}

// compute the target vector and the values of the function to fit
void AQLMathLeastSquareSwaptionVolDiscModelQlib::getfct2fit(const QuantLib::Array& x) const
{
    if( !mGetGSize )
    {
        mGetGSize = true;
        mG.resize( x.size() - 12 );
    }

    //
    for(size_t i = 0; i < 4; ++i)
    {
        mParamV[i] = x[i];
    }
    for(size_t i = 4; i < 12; ++i)
    {
        mParamF[i - 4] = x[i];
    }
    for(size_t i = 12; i < x.size(); ++i)
    {
        mG[i - 12] = x[i];
    }

    size_t k = 0;
    for(size_t i = 0; i < mn; ++i )
    {
        for(size_t j = 0; j < mm; ++j )
        {
            if( mUseThisTarget[i][j] )
            {
                mSwaptionVol[k]->setParam(mParamV, mParamF, mG);
				if(mTarget_ == Volatility)
				{
					mfct2fit[i][j] = mSwaptionVol[k]->getSwaptionVol();
				}
				else
				{
					mfct2fit[i][j] = mSwaptionVol[k]->getSwaptionPrem();
				}
            }
            else
            {
                mfct2fit[i][j] = 0.0;
            }
            ++k;
        }
    }

}

AQLMathLeastSquareSwaptionVolDiscModelQlib& AQLMathLeastSquareSwaptionVolDiscModelQlib::operator =(const AQLMathLeastSquareSwaptionVolDiscModelQlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).AQLMathLeastSquareSwaptionVolQlib::operator =( rhs );

        for( size_t i = 0; i < mSwaptionVol.size(); ++i )
        {
            delete mSwaptionVol[i];
            mSwaptionVol[i] = rhs.mSwaptionVol[i] != 0 ? rhs.mSwaptionVol[i]->clone() : 0;
        }
        mCloned = true;

        mfct2fit = rhs.mfct2fit;

        mParamV = rhs.mParamV;
        mParamF = rhs.mParamF;
        mG = rhs.mG;
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareSwaptionVolDiscModelVQlib::AQLMathLeastSquareSwaptionVolDiscModelVQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
																				   vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																				   mode mode__,
																				   target target_
		)
:
AQLMathLeastSquareSwaptionVolDiscModelQlib( Swaption_target_value_Mkt_, Swaption_Vol_, mode__, target_ )
{
}

AQLMathLeastSquareSwaptionVolDiscModelVQlib::AQLMathLeastSquareSwaptionVolDiscModelVQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
																				   vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																				   const DoubleMatrix& weight_,
																				   mode mode__,
																				   target target_
		)
:
AQLMathLeastSquareSwaptionVolDiscModelQlib( Swaption_target_value_Mkt_, Swaption_Vol_, weight_, mode__, target_ )
{
}

AQLMathLeastSquareSwaptionVolDiscModelVQlib::AQLMathLeastSquareSwaptionVolDiscModelVQlib( const AQLMathLeastSquareSwaptionVolDiscModelVQlib& rhs )
:
AQLMathLeastSquareSwaptionVolDiscModelQlib( rhs )
{
}

AQLMathLeastSquareSwaptionVolDiscModelVQlib::~AQLMathLeastSquareSwaptionVolDiscModelVQlib()
{
}

// compute the target vector and the values of the function to fit
void AQLMathLeastSquareSwaptionVolDiscModelVQlib::getfct2fit(const QuantLib::Array& x) const
{
    //
    for(size_t i = 0; i < 4; ++i)
    {
        mParamV[i] = x[i];
    }

    size_t k = 0;
    for(size_t i = 0; i < mn; ++i )
    {
        for(size_t j = 0; j < mm; ++j )
        {
            if( mUseThisTarget[i][j] )
            {
                mSwaptionVol[k]->setParamV(mParamV);
				if(mTarget_ == Volatility)
				{
					mfct2fit[i][j] = mSwaptionVol[k]->getSwaptionVol();
				}
				else
				{
					mfct2fit[i][j] = mSwaptionVol[k]->getSwaptionPrem();
				}
            }
            else
            {
                mfct2fit[i][j] = 0.0;
            }
            ++k;
		}
    }
}

AQLMathLeastSquareSwaptionVolDiscModelVQlib& AQLMathLeastSquareSwaptionVolDiscModelVQlib::operator =(const AQLMathLeastSquareSwaptionVolDiscModelVQlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).AQLMathLeastSquareSwaptionVolDiscModelQlib::operator =( rhs );
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareSwaptionVolDiscModelFQlib::AQLMathLeastSquareSwaptionVolDiscModelFQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
																				   vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																				   double T_max_,
																				   mode mode__,
																				   target target_
		)
:
AQLMathLeastSquareSwaptionVolDiscModelQlib( Swaption_target_value_Mkt_, Swaption_Vol_, mode__, target_ ),
mT_max(T_max_)
{
}

AQLMathLeastSquareSwaptionVolDiscModelFQlib::AQLMathLeastSquareSwaptionVolDiscModelFQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
																				   vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																				   double T_max_,
																				   const DoubleMatrix& weight_,
																				   mode mode__,
																				   target target_
		)
:
AQLMathLeastSquareSwaptionVolDiscModelQlib( Swaption_target_value_Mkt_, Swaption_Vol_, weight_, mode__, target_ ),
mT_max(T_max_)
{
}

AQLMathLeastSquareSwaptionVolDiscModelFQlib::AQLMathLeastSquareSwaptionVolDiscModelFQlib( const AQLMathLeastSquareSwaptionVolDiscModelFQlib& rhs )
:
AQLMathLeastSquareSwaptionVolDiscModelQlib( rhs ),
mT_max( rhs.mT_max )
{
}

AQLMathLeastSquareSwaptionVolDiscModelFQlib::~AQLMathLeastSquareSwaptionVolDiscModelFQlib()
{
}

// compute the target vector and the values of the function to fit
void AQLMathLeastSquareSwaptionVolDiscModelFQlib::getfct2fit(const QuantLib::Array& x) const
{
    //
    for(size_t i = 0; i < 8; ++i)
    {
        mParamF[i] = x[i];
    }
    mParamF.push_back(mT_max);

    size_t k = 0;
    for(size_t i = 0; i < mn; ++i )
    {
        for(size_t j = 0; j < mm; ++j )
        {
            if( mUseThisTarget[i][j] )
            {
                mSwaptionVol[k]->setParamF(mParamF);
				if(mTarget_ == Volatility)
				{
					mfct2fit[i][j] = mSwaptionVol[k]->getSwaptionVol();
				}
				else
				{
					mfct2fit[i][j] = mSwaptionVol[k]->getSwaptionPrem();
				}
            }
            else
            {
                mfct2fit[i][j] = 0.0;
            }
            ++k;
        }
    }
}

AQLMathLeastSquareSwaptionVolDiscModelFQlib& AQLMathLeastSquareSwaptionVolDiscModelFQlib::operator =(const AQLMathLeastSquareSwaptionVolDiscModelFQlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).AQLMathLeastSquareSwaptionVolDiscModelQlib::operator =( rhs );

        mT_max = rhs.mT_max;
    }
    return *this;
}

//
//--------------------------------------------------------------------------------
//

AQLMathLeastSquareSwaptionVolDiscModelGQlib::AQLMathLeastSquareSwaptionVolDiscModelGQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
																				   vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																				   mode mode__,
																				   target target_
		)
:
AQLMathLeastSquareSwaptionVolDiscModelQlib( Swaption_target_value_Mkt_, Swaption_Vol_, mode__, target_ )
{
}

AQLMathLeastSquareSwaptionVolDiscModelGQlib::AQLMathLeastSquareSwaptionVolDiscModelGQlib(const DoubleMatrix& Swaption_target_value_Mkt_,
																				   vector< AQLMathSwaptionVolLMMDiscModel* >& Swaption_Vol_,
																				   const DoubleMatrix& weight_,
																				   mode mode__,
																				   target target_
		)
:
AQLMathLeastSquareSwaptionVolDiscModelQlib( Swaption_target_value_Mkt_, Swaption_Vol_, weight_, mode__, target_ )
{
}

AQLMathLeastSquareSwaptionVolDiscModelGQlib::AQLMathLeastSquareSwaptionVolDiscModelGQlib( const AQLMathLeastSquareSwaptionVolDiscModelGQlib& rhs )
:
AQLMathLeastSquareSwaptionVolDiscModelQlib( rhs )
{
}

AQLMathLeastSquareSwaptionVolDiscModelGQlib::~AQLMathLeastSquareSwaptionVolDiscModelGQlib()
{
}

// compute the target vector and the values of the function to fit
void AQLMathLeastSquareSwaptionVolDiscModelGQlib::getfct2fit(const QuantLib::Array& x) const
{
    //
    mG.resize(x.size());
    for(size_t i = 0; i < x.size(); ++i)
    {
        mG[i] = x[i];
    }

	size_t k = 0;
    for(size_t i = 0; i < mn; ++i )
    {
        for(size_t j = 0; j < mm; ++j )
        {
            if( mUseThisTarget[i][j] )
            {
				mSwaptionVol[k]->setParamG(mG);
				if(mTarget_ == Volatility)
				{
					mfct2fit[i][j] = mSwaptionVol[k]->getSwaptionVol();
				}
				else
				{
					mfct2fit[i][j] = mSwaptionVol[k]->getSwaptionPrem();
				}
            }
            else
            {
                mfct2fit[i][j] = 0.0;
            }
            ++k;
        }
    }
}

AQLMathLeastSquareSwaptionVolDiscModelGQlib& AQLMathLeastSquareSwaptionVolDiscModelGQlib::operator =(const AQLMathLeastSquareSwaptionVolDiscModelGQlib& rhs)
{
    if ( this != &rhs )
    {
        (*this).AQLMathLeastSquareSwaptionVolDiscModelQlib::operator =( rhs );
    }
    return *this;
}
