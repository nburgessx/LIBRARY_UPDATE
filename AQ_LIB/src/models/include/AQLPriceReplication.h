#ifndef MVReplication_h
#define MVReplication_h

#ifdef __GNUG__
#pragma interface
#endif
//////////
#include "AQLFunction.h"
#include "AQLPriceSwaptionCalculator.h"

using namespace std;

//================ MVReplication ===================================
class MVReplication
{
public:
    MVReplication(AQLPriceSwaptionCalculator* swaptionCalculator_, double shift_, double annuity_, double dfPay_, double tau_, double nCashFlows_, string distributionType_, bool rescale_, double confidence_ = 0.0, int nPoints_ = 0);

    double Forward(double T, double deltaT, double S0);
    double Option(double T, double deltaT, double K, bool isCall, double S0);

    static double R(double S0, double tau, double deltaT, double nCashFlows);

private:
    double ForwardAdjustment(double T, double deltaT, double S0);
    double OptionAdjustment(double T, double deltaT, double K, bool isCall, double S0);

    double Theta(double S0, double tau, double deltaT, double nCashFlows);
    static double G(double S0, double tau, double deltaT, double nCashFlows);
    static double G1(double S0, double tau, double deltaT, double nCashFlows);

    double NormalForwardAdjustment(double T, double deltaT, double S0);
    double LogNormalForwardAdjustment(double T, double deltaT, double S0);
    double ReplicationForwardAdjustment(double T, double deltaT, double S0);

    double NormalOptionAdjustment(double T, double deltaT, double K, bool isCall, double S0);
    double LogNormalOptionAdjustment(double T, double deltaT, double K, bool isCall, double S0);
    double ReplicationOptionAdjustment(double T, double deltaT, double K, bool isCall, double S0);

    string mDistributionType;
    AQLPriceSwaptionCalculator* mSwpnCalculator;
    double mAnnuity, mDfPay, mTau, mNCashFlows, mShift;
    bool mRescale;
    double mConfidence; // Only used for Generic Replication
    int mNPoints; // Only used for Generic Replication
    static double mvReplicationThreshold;
};

//================ MVReplicationIntegrand ===================================
class MVReplicationIntegrand: public AQLFunction
{
public:
    MVReplicationIntegrand(double T_, bool isCall_, AQLPriceSwaptionCalculator* swpnCalculator_);
    double operator()(double x) const;

private:
    double mT;
    bool mIsCall;
    AQLPriceSwaptionCalculator* mSwpnCalculator;
};

//================ Utilities ===================================
MVReplication* GetReplicationMethod(string type, AQLPriceSwaptionCalculator* swpnCalculator_, double shift,
                                    double annuity, double dfPay, double tau,
                                    double nCashFlows, double confidence, int nPoints);

struct ReplicationConfig
{
    string swaptionModel;
    string replicationModel;
    double confidence;
    int nPoints;
};

ReplicationConfig GetReplicationConfig(const AQLStringMatrix& calibrationConfig);

#endif 
