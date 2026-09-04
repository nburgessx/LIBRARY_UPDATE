/*!
    @brief Implements Correlation and its related classes.
*/

#include "AQLMathCorrelationLMM.h"
#include "AQLMathCorrelationFuncLMM.h"
#include "AQLEigenSystems.h"

AQLMathCorrelationLMM::AQLMathCorrelationLMM()
:
mCorr(0),
mT_fix(0),
mNoFactors(1),
mCloned( false )
{
    mUnity.resize( mNoFactors, 1 );
}

AQLMathCorrelationLMM::AQLMathCorrelationLMM( const vector<double>&	T_fix_,
										size_t					no_factors_
										)
:
mCorr( 0 ),
mT_fix( T_fix_ ),
mNoFactors( 0 < no_factors_ && no_factors_ < mT_fix.size() ? no_factors_ : mT_fix.size() ),
mFullRankCorrMat(0),
mCloned( false )
{
    mUnity.resize( mNoFactors, 1 );
}

AQLMathCorrelationLMM::AQLMathCorrelationLMM( AQLMathCorrelationFuncLMM*	corr_,
										const vector<double>&	T_fix_,
										size_t					no_factors_
										)
: mCorr( corr_ ),
mT_fix( T_fix_ ),
mNoFactors( 0 < no_factors_ && no_factors_ < mT_fix.size() ? no_factors_ : mT_fix.size() ),
mCloned( false )
{
    mUnity.resize( mNoFactors, 1 );
}

AQLMathCorrelationLMM::AQLMathCorrelationLMM( const AQLMathCorrelationLMM& rhs )
: mCorr( rhs.mCorr != 0 ? dynamic_cast<AQLMathCorrelationFuncLMM*>(rhs.mCorr->clone()) : 0 ),
mT_fix( rhs.mT_fix ),
mNoFactors( rhs.mNoFactors ),
mUnity( rhs.mUnity ),
mCloned( true )
{
}

AQLMathCorrelationLMM::~AQLMathCorrelationLMM()
{
    if ( mCloned )
    {
        delete mCorr;
        mCorr = 0;
    }
}

AQLMathCorrelationLMM& AQLMathCorrelationLMM::operator =(const AQLMathCorrelationLMM& rhs)
{
    if ( this != &rhs )
    {
        delete mCorr;
        mCorr = rhs.mCorr != 0 ? dynamic_cast<AQLMathCorrelationFuncLMM*>(rhs.mCorr->clone()) : 0;

        mT_fix = rhs.mT_fix;
        mNoFactors = rhs.mNoFactors;
        mUnity = rhs.mUnity;
        mCloned = true;
    }
    return *this;
}

DoubleMatrix AQLMathCorrelationLMM::getFullRankCorr(double t) const
{ 
    return mCorr->getCorrMat(t, mT_fix);
}

