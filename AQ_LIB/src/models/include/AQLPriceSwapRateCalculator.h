#ifndef AQLPriceSwapRateCalculator_h
#define AQLPriceSwapRateCalculator_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceCMSTools.h"
#include "AQLPriceReplication.h"

class SwapRateInfo
{
public:
    SwapRateInfo() { }

    SwapRateInfo(AQLDataInstance* dataInstance, const AQLString& ccy, const AQLString& tenor_, CurveInfo discCurveInfo_, CurveInfo cmsCurveInfo_,
                 AQLStringMatrix cmsScheduler, ReplicationConfig repConfig, double shift);

    // Methods
    double ForwardSwapRate(AQLDate settlDate);
    double Annuity(AQLDate settlDate);
    double CMSForward(double fwd, double annuity, double T, double dfPay, double deltaT,
                      const DoubleVector& sabrParameters, const DoubleVector& tailParameters);
    double CMSForward(AQLDate valDate, CashFlowTiming cf);
    void CMSDistribution(AQLDate valDate, CashFlowTiming cf, double& cmsFwd, double& cmsVol);

    // Properties
    double Tau() { return mTau; }
    double NCashFlows() { return mNCashFlows; }

private:
    AQLDataInstance* mDataInstance;
    AQLString mFixedLegFreq, mIndexCalendar, mIndexDaycount, mIndexSlidingRule;
    AQLPriceDataSlidingRule mPaySlidingRule;
    AQLPriceDataCalendar mPayCalendar;
    double mTau, mNCashFlows;
    CurveInfo mDiscCurveInfo, mCmsCurveInfo;
    AQLString mTenor;
    AQLStringVector mSabrIDs, mTailIDs;
    ReplicationConfig mRepConfig;
    double mShift;
};

#endif 
