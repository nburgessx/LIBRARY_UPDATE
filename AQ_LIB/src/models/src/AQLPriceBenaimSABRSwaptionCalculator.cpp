#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLPriceBenaimSABRSwaptionCalculator.h"
#include "AQLDist.h"
#include "AQLCoreUtil.h"
#include "AQLMathOptionTools.h"

// Benaim SABR option model to use for CMS replication. The price tails are replaced by parametric functions with smooth junction.
AQLBenaimSABRSwaptionCalculator::AQLBenaimSABRSwaptionCalculator(double S0_, const vector<double>& sabrParameters_,
                                                               const vector<double>& extraParameters_, bool useLeft)
: AQLSABRSwaptionCalculator(S0_, sabrParameters_)
{
    if (extraParameters_.size() < 4)
        AQLCoreInvalidData("Invalid parameter size in Benaim SABR swaption calculator",__FILE__,__LINE__);

    mLeftTail = extraParameters_[0];
    mRightTail = extraParameters_[1];
    mLeftCutOff = extraParameters_[2];
    mRightCutOff = extraParameters_[3];
    mUseLeft = useLeft;

    mTThreshold = 0.00001;
    mTCache = -12.34;
}

void AQLBenaimSABRSwaptionCalculator::CheckCache(double t)
{
    if (!eq(t, mTCache, mTThreshold))
    {
        double atmVol = mSabr.getSABRVol(t, mS0, mS0);
        double stDev = atmVol * sqrt(t);

        // Left tail
        double confLeft = AQLDist::invNormdist(mLeftCutOff);
        mKm = mS0 * exp(-0.5 * stDev * stDev - confLeft * stDev);
        if (mUseLeft)
        {
            double prl = SABRPrice(t, mKm, false);
            double hl = 0.0001;
            double prmhl = SABRPrice(t, mKm - hl, false);
            double prphl = SABRPrice(t, mKm + hl, false);
            double prpl = 0.5 * (prphl - prmhl) / hl;
            double prppl = (prphl + prmhl - 2.0 * prl) / (hl * hl);

            double prpp_prl = prppl / prl;
            double prp_prl = prpl / prl;
            double mu_k = mLeftTail / mKm;
            mCl = 0.5 * (prpp_prl - prp_prl * prp_prl + mu_k / mKm);
            mBl = prp_prl - 2.0 * mCl * mKm - mu_k;
            double kmu = pow(mKm, mLeftTail);
            mAl = log(prl / kmu) - mKm * (mBl + mCl * mKm);
        }
        else
        {
            mVolLeft = SABRIV(t, mKm);
        }

        // Right tail
        double confRight = AQLDist::invNormdist(mRightCutOff);
        mKp = mS0 * exp(-0.5 * stDev * stDev + confRight * stDev);
        double prr = SABRPrice(t, mKp, true);
        double hr = 0.0001;
        double prmhr = SABRPrice(t, mKp - hr, true);
        double prphr = SABRPrice(t, mKp + hr, true);
        double prpr = 0.5 * (prphr - prmhr) / hr;
        double prppr = (prphr + prmhr - 2.0 * prr) / (hr * hr);

        double k2 = mKp * mKp;
        double prpp_prr = k2 * prppr / prr;
        double prp_prr = mKp * prpr / prr;
        double nu_k = mRightTail / mKp;
        mCr = 0.5 * k2 * (prpp_prr - prp_prr * prp_prr + 2.0 * prp_prr + mRightTail);
        mBr = -mKp * (prp_prr + 2.0 * mCr / k2 + mRightTail);
        double kmnu = pow(mKp, -mRightTail);
        mAr = log(prr / kmnu) - (mBr + mCr / mKp) / mKp;

        mTCache = t;
    }
}

double AQLBenaimSABRSwaptionCalculator::Price(double t, double strike, bool isCall)
{
    double price;
    if (abs(t) < 0.00001)
        price = SABRPrice(t, strike, isCall);
    else
    {
        CheckCache(t);
        double k = strike;
        if (mKm <= k && k <= mKp)
            price = SABRPrice(t, strike, isCall);
        else if (k < mKm)
        {
            if (mUseLeft)
            {
                double put = pow(k, mLeftTail) * exp(mAl + k * (mBl + mCl * k));
                price = (isCall ? put + mS0 - k : put);
            }
            else
            {
                return BlackPrice(t, strike, isCall, mS0, mVolLeft);
            }
        }
        else // k > kp
        {
            double call = pow(k, -mRightTail) * exp(mAr + (mBr + mCr / k) / k);
            price = (isCall ? call : call - mS0 + k);
        }
    }

    return price;
}
