#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLPriceFlatSABRSwaptionCalculator.h"
#include "AQLDist.h"
#include "AQLCoreUtil.h"
#include "AQLMathOptionTools.h"

// Flat SABR option model to use for CMS replication. The price tails are replaced by constants.
MVFlatSABRSwaptionCalculator::MVFlatSABRSwaptionCalculator(double S0_, const vector<double>& sabrParameters_,
                                                           const vector<double>& extraParameters_)
: MVSABRSwaptionCalculator(S0_, sabrParameters_)
{
    if (extraParameters_.size() < 2)
        AQLCoreInvalidData("Invalid parameter size in Flat SABR swaption calculator",__FILE__,__LINE__);

    mLeftCutOff = extraParameters_[0];
    mRightCutOff = extraParameters_[1];

    mTThreshold = 0.00001;
    mTCache = -12.34;
}

void MVFlatSABRSwaptionCalculator::CheckCache(double t)
{
    if (!eq(t, mTCache, mTThreshold))
    {
        double atmVol = mSabr.getSABRVol(t, mS0, mS0);
        double stDev = atmVol * sqrt(t);

        // Left tail
        double confLeft = AQLDist::invNormdist(mLeftCutOff);
        mKl = mS0 * exp(-0.5 * stDev * stDev - confLeft * stDev);
        mVl = SABRIV(t, mKl);

        // Right tail
        double confRight = AQLDist::invNormdist(mRightCutOff);
        mKr = mS0 * exp(-0.5 * stDev * stDev + confRight * stDev);
        mVr = SABRIV(t, mKr);

        mTCache = t;
    }
}

double MVFlatSABRSwaptionCalculator::Price(double t, double strike, bool isCall)
{
    double price;
    if (abs(t) < 0.00001)
        price = SABRPrice(t, strike, isCall);
    else
    {
        CheckCache(t);
        double k = strike;
        if (mKl <= k && k <= mKr)
            price = SABRPrice(t, strike, isCall);
        else if (k < mKl)
            price = BlackPrice(t, k, isCall, mS0, mVl);
        else // k > kr
            price = BlackPrice(t, k, isCall, mS0, mVr);
    }

    return price;
}
