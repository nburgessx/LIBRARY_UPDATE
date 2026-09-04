/*!
    @brief Implements Volatility and its related classes.
*/

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "AQLMathVolatilityLMMContModel.h"
#include <cmath>
#include <algorithm>
#include "AQLEigenSystems.h"
#include "AQLModelUtilities.h"

AQLMathVolatilityLMMContModel::AQLMathVolatilityLMMContModel(const DoubleVector& paramV_,
													   const DoubleVector& paramF_,
													   const DoubleVector& G_,
													   const DoubleVector& T_fix_special_,
													   const DoubleVector& T_fix,
													   size_t              n
		)
: AQLMathVolatilityLMMCont( T_fix, n ), mParamV( paramV_ ), mParamF( paramF_ ), mG( G_ ), mT_fix_special( T_fix_special_ )
{
}

double AQLMathVolatilityLMMContModel::get(double t, size_t i)
{
    if ( mT_fix.size() <= i ) return 0.0;
    if ( t < 0 || mT_fix[i] < t ) return 0.0;
    if ( !AQLModelUtilities::eq(mT_fix_special[0],0.0) && t < mT_fix[0] )
    {
        return AQLEigenSystems::sigma0( t * mT_fix_special[0] / mT_fix[0], mT_fix_special[i], mParamV, mParamF, mG[i]);
    }

    size_t j = upper_bound(mT_fix.begin(), mT_fix.end(), t) - mT_fix.begin();
    if ( mT_fix_special.size() <= j) return 0.0;
    return AQLEigenSystems::sigma0( t * mT_fix_special[j] / mT_fix[j], mT_fix_special[i], mParamV, mParamF, mG[i]);
}

