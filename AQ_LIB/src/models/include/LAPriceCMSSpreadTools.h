#ifndef LAPriceCMSSpreadTools_h
#define LAPriceCMSSpreadTools_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDataInstance.h"
#include "LAPriceSwapRateCalculator.h"

#ifndef AQ_THETA1_IN
#define AQ_THETA1_IN "_Theta1_In_"
#endif

#ifndef AQ_THETA2_IN
#define AQ_THETA2_IN "_Theta2_In_"
#endif

#ifndef AQ_COPRHO_IN
#define AQ_COPRHO_IN "_CopRho_In_" // Named for Copula-Rho to distinguish from SABR Rho
#endif

#ifndef AQ_THETA1_OUT
#define AQ_THETA1_OUT "_Theta1_Out_" // Named for Copula-Rho to distinguish from SABR Rho
#endif

#ifndef AQ_THETA2_OUT
#define AQ_THETA2_OUT "_Theta2_Out_" // Named for Copula-Rho to distinguish from SABR Rho
#endif

#ifndef AQ_COPRHO_OUT
#define AQ_COPRHO_OUT "_CopRho_Out_" // Named for Copula-Rho to distinguish from SABR Rho
#endif

#ifndef AQ_CMSSPRD_TGT_OUT
#define AQ_CMSSPRD_TGT_OUT "_CMSSpread_Targets_Out_"
#endif

#ifndef AQ_SL_ATM
#define AQ_SL_ATM "_SL_ATM_"
#endif

#ifndef AQ_SL_SMILE
#define AQ_SL_SMILE "_SL_SMILE_"
#endif

#ifndef AQ_ML_SMILE
#define AQ_ML_SMILE "_ML_SMILE_"
#endif

#ifndef AQ_COP_NAMES
#define AQ_COP_NAMES AQLStringVector { "Theta1", "Theta2", "CopRho" }
#endif

class LAPriceCMSMLATMTarget;

class LAPriceCMSSpreadUtility
{
public:

    static double CMSSpreadMLATM(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                 const AQLString& expiryTerm, const AQLStringMatrix& pricingConfig,
                                 const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                                 const AQLString& proxySpreadID);

    static double CMSSpreadMLPrice(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                   const AQLString& expiryTerm, double strike, const AQLString& optionType,
                                   const AQLStringMatrix& pricingConfig,
                                   const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                                   const AQLString& proxySpreadID);

    static double CMSSpreadSLATM(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                 const AQLString& expiryTerm, const AQLStringMatrix& pricingConfig,
                                 const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler);

    static double CMSSpreadSLPrice(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                   const AQLString& expiryTerm, double strike, const AQLString& optionType,
                                   const AQLStringMatrix& pricingConfig,
                                   const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                                   const AQLString& proxySpreadID);

    static LAPriceCMSMLATMTarget WarmUpMLPrice(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                          const AQLString& expiryTerm, const AQLStringMatrix& pricingConfig,
                                          const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                                          const AQLString& proxySpreadID);

    static void WarmUpCMSSpread(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                const AQLString& expiryTerm, const AQLStringMatrix& pricingConfig,
                                const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                                SwapRateInfo& rate1, SwapRateInfo& rate2);

    static void CheckInitialParameters(AQLDataInstance* dataInstance, const AQLString& currency, AQLStringVector& expiryTerms,
                                       AQLStringVector& indexes);

    static void ReadSLGrid(AQLDataInstance* dataInstance, const AQLString& currency, AQLStringVector& slTerms);

    static bool CheckModes(const AQLStringVector& modes);

    static void CheckStringVectors(const AQLStringVector& terms1, const AQLStringVector& terms2);

    static void CheckIndexes(AQLDataInstance* dataInstance, const AQLString& ccy, bool calibrate, const AQLStringVector& pairIDs,
                             const AQLStringVector& initIDs);

    static DoubleVector InterpolateParameters(AQLDataInstance* dataInstance, const DateVector& dates, const AQLString& index,
                                              const AQLString& id, const AQLString& currency);

    static double CMSSpreadCoupon(double strike, bool isCall, double S1, double S2, double stDev1, double stDev2,
                                  double rho);
};

class LAPriceCMSMLATMTarget : public AQLFunction
{
public:
    LAPriceCMSMLATMTarget(AQLDataInstance* dataInstance, const AQLString& ccy, AQLDate valDate,
                     SwapRateInfo rate1, SwapRateInfo rate2, const AQLString& pairID,
                     const vector<CashFlowTiming>& schedule, const vector<size_t>& months,
                     int startLength, int endLength, CurveInfo discCurveInfo,
                     const string& copType, double confidence);

    double operator()(double x) const;

    double Price(double strike, bool isCall) const;

    double Forward();

private:
    AQLDataInstance* mDataInstance;
    AQLDate mValDate;
    vector<CashFlowTiming> mSchedule;
    double mConfidence;
    SwapRateInfo mRate1, mRate2;
    AQLString mTheta1ID, mTheta2ID, mRhoID, mPairID;
    string mCopType;
    vector<size_t> mMonths;
    size_t mStartLength, mEndLength;
    CurveInfo mDiscCurveInfo;

    DoubleMatrix mCopParams;
    DoubleVector mCMS1, mVol1, mCMS2, mVol2, mDF, mT, mAcc;
};

#endif 
