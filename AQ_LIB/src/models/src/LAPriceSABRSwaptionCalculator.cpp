#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceSABRSwaptionCalculator.h"
#include "LAMathOptionTools.h"

// SABR option model to use for CMS replication. This is the original SABR model.
MVSABRSwaptionCalculator::MVSABRSwaptionCalculator(double S0_, const vector<double>& sabrParameters_)
: LAPriceSwaptionCalculator(S0_)
{
    mSabrParameters = sabrParameters_;
    CheckParameters();
    mSabr = LAMathSABR_Hagan(mSabrParameters);
}

double MVSABRSwaptionCalculator::Price(double t, double strike, bool isCall)
{
    return SABRPrice(t, strike, isCall);
}

double MVSABRSwaptionCalculator::BoundStrike(double t, double confidence, bool isUp)
{
    double vol = mSabr.getSABRVol(t, mS0, mS0);
    double stDev = vol * sqrt(t);
    double w = (isUp ? 1.0 : -1.0);
    return mS0 * exp(-0.5 * stDev * stDev + w* stDev * confidence);
}

vector<double> MVSABRSwaptionCalculator::StrikeGrid(double t, double confidence, size_t nPoints)
{
    double vol = mSabr.getSABRVol(t, mS0, mS0);
    // Create log-normal grid
    double v = vol * sqrt(t);
    double v2 = pow(v, 2);
    double sMin = -0.5 * v2 - v * confidence;
    double sMax = -0.5 * v2 + v * confidence;
    vector<double> sGrid(nPoints + 1);
    sGrid[0] = sMin;
    double kStep = (sMax - sGrid[0]) / nPoints;
    for (size_t i = 1; i < nPoints + 1; i++)
        sGrid[i] = sGrid[i - 1] + kStep;
    for (size_t i = 0; i < nPoints + 1; i++)
        sGrid[i] = mS0 * exp(sGrid[i]);

    return sGrid;
}

double MVSABRSwaptionCalculator::SABRPrice(double t, double strike, bool isCall)
{
    double vol = mSabr.getSABRVol(t, mS0, strike);
    return BlackPrice(t, strike, isCall, mS0, vol);
}

double MVSABRSwaptionCalculator::SABRIV(double t, double strike)
{
    return mSabr.getSABRVol(t, mS0, strike);
}

// Check parameter consistency
void MVSABRSwaptionCalculator::CheckParameters()
{
    if(mSabrParameters.size() != 4) throw AQLCoreInvalidData("SABR params.size() != 4",__FILE__,__LINE__);
    if(mSabrParameters[0] < 0) throw AQLCoreInvalidData("Alpha must be positive",__FILE__,__LINE__);
    if(mSabrParameters[1] < 0 || 1 < mSabrParameters[1]) throw AQLCoreInvalidData("Beta must be in (0,1)",__FILE__,__LINE__);
    if(mSabrParameters[2] < 0 ) throw AQLCoreInvalidData("Nu must be possitive.",__FILE__,__LINE__);
    if(fabs(mSabrParameters[3]) > 1 ) throw AQLCoreInvalidData("Rho must be in (-1,1).",__FILE__,__LINE__);
}
