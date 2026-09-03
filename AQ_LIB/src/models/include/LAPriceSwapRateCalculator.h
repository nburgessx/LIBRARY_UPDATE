#ifndef LAPriceSwapRateCalculator_h
#define LAPriceSwapRateCalculator_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceCMSTools.h"
#include "LAPriceReplication.h"

class SwapRateInfo
{
public:
    SwapRateInfo() { }

    SwapRateInfo(LADataInstance* dataInstance, const LAString& ccy, const LAString& tenor_, CurveInfo discCurveInfo_, CurveInfo cmsCurveInfo_,
                 LAStringMatrix cmsScheduler, ReplicationConfig repConfig, double shift);

    // Methods
    double ForwardSwapRate(LADate settlDate);
    double Annuity(LADate settlDate);
    double CMSForward(double fwd, double annuity, double T, double dfPay, double deltaT,
                      const DoubleVector& sabrParameters, const DoubleVector& tailParameters);
    double CMSForward(LADate valDate, CashFlowTiming cf);
    void CMSDistribution(LADate valDate, CashFlowTiming cf, double& cmsFwd, double& cmsVol);

    // Properties
    double Tau() { return mTau; }
    double NCashFlows() { return mNCashFlows; }

private:
    LADataInstance* mDataInstance;
    LAString mFixedLegFreq, mIndexCalendar, mIndexDaycount, mIndexSlidingRule;
    LAPriceDataSlidingRule mPaySlidingRule;
    LAPriceDataCalendar mPayCalendar;
    double mTau, mNCashFlows;
    CurveInfo mDiscCurveInfo, mCmsCurveInfo;
    LAString mTenor;
    LAStringVector mSabrIDs, mTailIDs;
    ReplicationConfig mRepConfig;
    double mShift;
};

#endif 
