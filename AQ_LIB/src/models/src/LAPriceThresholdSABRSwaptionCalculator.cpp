#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceThresholdSABRSwaptionCalculator.h"
#include "LAMathOptionTools.h"

// Threshold SABR option model to use for CMS replication. The volatility is capped by a threshold.
MVThresholdSABRSwaptionCalculator::MVThresholdSABRSwaptionCalculator(double S0_,
                                                                     const vector<double>& sabrParameters_,
                                                                     const vector<double>& extraParameters_)
: MVSABRSwaptionCalculator(S0_, sabrParameters_)
{
    if (extraParameters_.size() < 1)
        LACoreInvalidData("Invalid parameter size in Threshold SABR swaption calculator",__FILE__,__LINE__);

    mThreshold = extraParameters_[0];
}

double MVThresholdSABRSwaptionCalculator::Price(double t, double strike, bool isCall)
{
    double vol = mSabr.getSABRVol(t, mS0, strike);
    vol = (vol > mThreshold ? mThreshold : vol);
    return BlackPrice(t, strike, isCall, mS0, vol);
}
