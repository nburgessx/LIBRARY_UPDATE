#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceReplication.h"
#include "AQLGaussLegendre.h"
#include "LAAnalyticFormula.h"
#include "LAMathOptionTools.h"
#include "AQLFunctionUtilities.h"

double MVReplication::mvReplicationThreshold = 0.00001;
//================ MVReplication ===================================
// Replication under the Swap Yield Terminal Swap Rate model. Contains the generic replication (full integration), but also the particular
// cases of LogNormal (Hagan) and Normal projections.
MVReplication::MVReplication(LAPriceSwaptionCalculator* swpnCalculator_,
                             double shift_, double annuity_, double dfPay_, double tau_, double nCashFlows_,
                             string distributionType_, bool rescale_, double confidence_ , int nPoints_)
{
    mSwpnCalculator = swpnCalculator_;
    mShift = shift_;
    mAnnuity = annuity_;
    mDfPay = dfPay_;
    mTau = tau_;
    mNCashFlows = nCashFlows_;
    mDistributionType = distributionType_;
    mRescale = rescale_;
    mConfidence = confidence_;
    mNPoints = nPoints_;
}

double MVReplication::Forward(double T, double deltaT, double S0)
{
    double basePrice = S0;
    double adjustment;
    if (abs(T) < 0.00001)
        adjustment = 0.0;
    else
    {
        double scaling = (mRescale ? G(S0, mTau, deltaT, mNCashFlows) : mDfPay / mAnnuity);
        adjustment = G1(S0, mTau, deltaT, mNCashFlows) / scaling * ForwardAdjustment(T, deltaT, S0 + mShift);
    }

    return S0 + adjustment;
}

double MVReplication::Option(double T, double deltaT, double K, bool isCall, double S0)
{
    K += mShift;
    double basePrice = mSwpnCalculator->Price(T, K, isCall);
    double adjustment;
    if (abs(T) < 0.00001)
        adjustment = 0.0;
    else
    {
        double scaling = (mRescale ? G(S0, mTau, deltaT, mNCashFlows) : mDfPay / mAnnuity);
        adjustment = G1(S0, mTau, deltaT, mNCashFlows) / scaling * OptionAdjustment(T, deltaT, K, isCall, S0 + mShift);
    }

    return basePrice + adjustment;
}

double MVReplication::ForwardAdjustment(double T, double deltaT, double S0)
{
    if (mDistributionType == "LogNormal")
        return LogNormalForwardAdjustment(T, deltaT, S0);
    else if (mDistributionType == "Normal")
        return NormalForwardAdjustment(T, deltaT, S0);
    else if (mDistributionType == "Generic")
        return ReplicationForwardAdjustment(T, deltaT, S0);
    else
        throw AQLCoreInvalidData("Unknown distribution type in replication",__FILE__,__LINE__);
}

double MVReplication::OptionAdjustment(double T, double deltaT, double K, bool isCall, double S0)
{
    if (mDistributionType == "LogNormal")
        return LogNormalOptionAdjustment(T, deltaT, K, isCall, S0);
    else if (mDistributionType == "Normal")
        return NormalOptionAdjustment(T, deltaT, K, isCall, S0);
    else if (mDistributionType == "Generic")
        return ReplicationOptionAdjustment(T, deltaT, K, isCall, S0);
    else
        throw AQLCoreInvalidData("Unknown distribution type in replication",__FILE__,__LINE__);
}

double MVReplication::NormalForwardAdjustment(double T, double deltaT, double S0)
{
    double optionPrice = mSwpnCalculator->Price(T, S0, true);
    double vol = BachelierImpliedVolatility(optionPrice, T, S0, true, S0);
    double stDev = vol * sqrt(T);
    return stDev * stDev;
    //return Theta(S0, tau, deltaT, nCashFlows) / S0 * stDev * stDev;
}

double MVReplication::LogNormalForwardAdjustment(double T, double deltaT, double S0)
{
    double optionPrice = mSwpnCalculator->Price(T, S0, true);
    double vol = BlackImpliedVolatility(optionPrice, T, S0, true, S0);
    double stDev = vol * sqrt(T);
    return S0 * S0 * (exp(stDev * stDev) - 1.0);
    //return Theta(S0, tau, deltaT, nCashFlows) * S0 * (exp(stDev * stDev) - 1.0);
}

double MVReplication::ReplicationForwardAdjustment(double T, double deltaT, double S0)
{
    //// By put-call parity at ATM ////
    //double K = S0;
    //double cap = OptionAdjustment(T, K, true, S0, tau, deltaT, nCashFlows, sabrParameters, threshold, confidence, nPoints);
    //double floor = OptionAdjustment(T, K, false, S0, tau, deltaT, nCashFlows, sabrParameters, threshold, confidence, nPoints);
    //return cap - floor;

    //// By direct integration ////
    MVReplicationIntegrand integrand(T, true, mSwpnCalculator);

    //// Integration bounds
    //double lowBound = 0.0;
    //double upBound = mSwpnCalculator->BoundStrike(T, mConfidence, true);

    double epsilon = 1e-6;
    size_t nSplits = 8;
    vector<double> strikeGrid = mSwpnCalculator->StrikeGrid(T, mConfidence, nSplits);
    vector<double> integrationGrid;
    integrationGrid.push_back(0.0);
    for (size_t i = 0; i < strikeGrid.size(); i++)
    {
        if (strikeGrid[i] >= epsilon)
            integrationGrid.push_back(strikeGrid[i]);
    }

    //// Calculate adjustment ////
    AQLGaussLegendre GQ(mNPoints);
    double integral = GQ.IntegrateBySampling(integrand, integrationGrid);
    //double integral = GQ.integrate(integrand, lowBound, upBound);
    return S0 * S0 * (2.0 * integral / (S0 * S0) - 1.0);
}

double MVReplication::NormalOptionAdjustment(double T, double deltaT, double K, bool isCall, double S0)
{
    double optionPrice = mSwpnCalculator->Price(T, K, isCall);
    double vol = BachelierImpliedVolatility(optionPrice, T, K, isCall, S0);
    double stDev = vol * sqrt(T);
    double w = (isCall ? 1.0 : -1.0);
    return w * stDev * stDev * AQLDist::normsdist(w * (S0 - K) / stDev);
    //return Theta(S0, tau, deltaT, nCashFlows) / S0 * w * stDev * stDev * AQLDist::normsdist(w * (S0 - K) / stDev);
}

double MVReplication::LogNormalOptionAdjustment(double T, double deltaT, double K, bool isCall, double S0)
{
    double optionPrice = mSwpnCalculator->Price(T, K, isCall);
    double vol = BlackImpliedVolatility(optionPrice, T, K, isCall, S0);
    double stDev = vol * sqrt(T);
    double d0 = log(S0 / K) / stDev;
    double d32 = d0 + 1.5 * stDev;
    double d12 = d0 + 0.5 * stDev;
    double dm12 = d0 - 0.5 * stDev;
    double w = (isCall ? 1.0 : -1.0);
    double t1 = S0 * exp(stDev * stDev) * AQLDist::normsdist(w * d32);
    double t2 = (S0 + K) * AQLDist::normsdist(w * d12);
    double t3 = K * AQLDist::normsdist(w * dm12);
    return w * S0 * (t1 - t2 + t3);
    //return Theta(S0, tau, deltaT, nCashFlows) * w * (t1 - t2 + t3);
}

double MVReplication::ReplicationOptionAdjustment(double T, double deltaT, double K, bool isCall, double S0)
{
    // Integrand
    MVReplicationIntegrand integrand(T, isCall, mSwpnCalculator);

    // Integration bounds
    double epsilon = 1e-6;
    //double lowBound, upBound;
    //bool calculateIntegral = true;
    //if (isCall)
    //{
    //    lowBound = K;
    //    double kp = mSwpnCalculator->BoundStrike(T, mConfidence, true);
    //    upBound = kp;
    //    calculateIntegral = !(K > upBound - epsilon);
    //}
    //else
    //{
    //    double km = mSwpnCalculator->BoundStrike(T, mConfidence, false);
    //    lowBound = km;
    //    upBound = K;
    //    calculateIntegral = !(K < lowBound + epsilon);
    //}
    size_t nSplits = 8;
    vector<double> strikeGrid = mSwpnCalculator->StrikeGrid(T, mConfidence, nSplits);
    vector<double> integrationGrid;
    if (isCall)
    {
        integrationGrid.push_back(K);
        for (size_t i = 0; i < strikeGrid.size(); i++)
        {
            if (strikeGrid[i] >= K + epsilon)
                integrationGrid.push_back(strikeGrid[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < strikeGrid.size(); i++)
        {
            if (strikeGrid[i] < K - epsilon)
                integrationGrid.push_back(strikeGrid[i]);
        }
        integrationGrid.push_back(K);
    }
    bool calculateIntegral = (integrationGrid.size() >= 2);

    //// Calculate adjustment ////
    double optionPrice = mSwpnCalculator->Price(T, K, isCall);
    double integralTerm = 0.0;
    if (calculateIntegral)
    {
        AQLGaussLegendre GQ(mNPoints);
        integralTerm = GQ.IntegrateBySampling(integrand, integrationGrid);
        //integralTerm = GQ.integrate(integrand, lowBound, upBound);
    }
    double w = (isCall ? 1.0 : -1.0);
    return (K - S0) * optionPrice + w * 2.0 * integralTerm;
}

// R function of Mercurio-Pallavicini for the Linear Swap Rate model
double MVReplication::R(double S0, double tau, double deltaT, double nCashFlows)
{
    return G1(S0, tau, deltaT, nCashFlows) / G(S0, tau, deltaT, nCashFlows);
}

// Theta function of Mercurio-Pallavicini for the Linear Swap Rate model
double MVReplication::Theta(double S0, double tau, double deltaT, double nCashFlows)
{
    return S0 * G1(S0, tau, deltaT, nCashFlows) / G(S0, tau, deltaT, nCashFlows);
}

// G function of Mercurio-Pallavicini for the Linear Swap Rate model
double MVReplication::G(double S0, double tau, double deltaT, double nCashFlows)
{
    double g, p = 1.0 / tau;
    if (abs(S0) < mvReplicationThreshold)
        g = p / nCashFlows;
    else
    {
        double d = 1.0 + S0 / p;
        g = S0 / pow(d, p * deltaT) / (1.0 - pow(d, -nCashFlows));
    }

    return g;
}

// Differential of the G function of Mercurio-Pallavicini for the Linear Swap Rate model
double MVReplication::G1(double S0, double tau, double deltaT, double nCashFlows)
{
    double dg, p = 1.0 / tau;
    if (abs(S0) < mvReplicationThreshold)
        dg = (0.5 * (nCashFlows + 1.0) - p * deltaT) / nCashFlows;
    else
    {
        double d = 1.0 + S0 / p;
        double t1 = pow(d, nCashFlows - p * deltaT - 1.0);
        double dn1 = pow(d, nCashFlows) - 1.0;
        double t2 = pow(dn1, 2.0);
        double t3 = (d - S0 * deltaT) * dn1 - S0 * nCashFlows / p;
        dg = t1 * t3 / t2;
    }

    return dg;
}

//================ MVReplicationIntegrand ===================================
MVReplicationIntegrand::MVReplicationIntegrand(double T_, bool isCall_, LAPriceSwaptionCalculator* swpnCalculator_)
{
    mT = T_;
    mIsCall = isCall_;
    mSwpnCalculator = swpnCalculator_;
}

double MVReplicationIntegrand::operator()(double x) const
{
    return mSwpnCalculator->Price(mT, x, mIsCall);
}

//================ Utilities ===================================
MVReplication* GetReplicationMethod(string type, LAPriceSwaptionCalculator* swpnCalculator, double shift,
                                     double annuity, double dfPay, double tau,
                                     double nCashFlows, double confidence, int nPoints)
{
    MVReplication* replication;
    if (type == "Hagan")
        replication = new MVReplication(swpnCalculator, shift, annuity, dfPay, tau, nCashFlows, "LogNormal", false, confidence, nPoints);
    else if (type == "Rescaled Hagan")
        replication = new MVReplication(swpnCalculator, shift, annuity, dfPay, tau, nCashFlows, "LogNormal", true, confidence, nPoints);
    else if (type == "Normal Hagan")
        replication = new MVReplication(swpnCalculator, shift, annuity, dfPay, tau, nCashFlows, "Normal", true, confidence, nPoints);
    else if (type == "Mercurio")
        replication = new MVReplication(swpnCalculator, shift, annuity, dfPay, tau, nCashFlows, "Generic", true, confidence, nPoints);
    else
        throw AQLCoreInvalidData(("Unknown replication model type: " + type).c_str(),__FILE__,__LINE__);

    return replication;
}

ReplicationConfig GetReplicationConfig(const AQLStringMatrix& calibrationConfig)
{
    string swpnModel = AQLFunctionUtilities::findElement(calibrationConfig, "SwaptionModel").getCString();
    string repModel = AQLFunctionUtilities::findElement(calibrationConfig, "ReplicationModel").getCString();
    double confidence = AQLFunctionUtilities::findElement(calibrationConfig, "Confidence").getDoubleValue();
    int nPoints = AQLFunctionUtilities::findElement(calibrationConfig, "IntegrationPoints").getIntValue();
    return ReplicationConfig{ swpnModel, repModel, confidence, nPoints };
}
