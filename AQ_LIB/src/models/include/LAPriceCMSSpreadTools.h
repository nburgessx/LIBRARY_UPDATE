#ifndef LAPriceCMSSpreadTools_h
#define LAPriceCMSSpreadTools_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LADataInstance.h"
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
#define AQ_COP_NAMES LAStringVector { "Theta1", "Theta2", "CopRho" }
#endif

class LAPriceCMSMLATMTarget;

class LAPriceCMSSpreadUtility
{
public:

    static double CMSSpreadMLATM(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                 const LAString& expiryTerm, const LAStringMatrix& pricingConfig,
                                 const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                                 const LAString& proxySpreadID);

    static double CMSSpreadMLPrice(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                   const LAString& expiryTerm, double strike, const LAString& optionType,
                                   const LAStringMatrix& pricingConfig,
                                   const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                                   const LAString& proxySpreadID);

    static double CMSSpreadSLATM(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                 const LAString& expiryTerm, const LAStringMatrix& pricingConfig,
                                 const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler);

    static double CMSSpreadSLPrice(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                   const LAString& expiryTerm, double strike, const LAString& optionType,
                                   const LAStringMatrix& pricingConfig,
                                   const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                                   const LAString& proxySpreadID);

    static LAPriceCMSMLATMTarget WarmUpMLPrice(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                          const LAString& expiryTerm, const LAStringMatrix& pricingConfig,
                                          const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                                          const LAString& proxySpreadID);

    static void WarmUpCMSSpread(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                const LAString& expiryTerm, const LAStringMatrix& pricingConfig,
                                const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                                SwapRateInfo& rate1, SwapRateInfo& rate2);

    static void CheckInitialParameters(LADataInstance* dataInstance, const LAString& currency, LAStringVector& expiryTerms,
                                       LAStringVector& indexes);

    static void ReadSLGrid(LADataInstance* dataInstance, const LAString& currency, LAStringVector& slTerms);

    static bool CheckModes(const LAStringVector& modes);

    static void CheckStringVectors(const LAStringVector& terms1, const LAStringVector& terms2);

    static void CheckIndexes(LADataInstance* dataInstance, const LAString& ccy, bool calibrate, const LAStringVector& pairIDs,
                             const LAStringVector& initIDs);

    static DoubleVector InterpolateParameters(LADataInstance* dataInstance, const DateVector& dates, const LAString& index,
                                              const LAString& id, const LAString& currency);

    static double CMSSpreadCoupon(double strike, bool isCall, double S1, double S2, double stDev1, double stDev2,
                                  double rho);
};

class LAPriceCMSMLATMTarget : public LAFunction
{
public:
    LAPriceCMSMLATMTarget(LADataInstance* dataInstance, const LAString& ccy, LADate valDate,
                     SwapRateInfo rate1, SwapRateInfo rate2, const LAString& pairID,
                     const vector<CashFlowTiming>& schedule, const vector<size_t>& months,
                     int startLength, int endLength, CurveInfo discCurveInfo,
                     const string& copType, double confidence);

    double operator()(double x) const;

    double Price(double strike, bool isCall) const;

    double Forward();

private:
    LADataInstance* mDataInstance;
    LADate mValDate;
    vector<CashFlowTiming> mSchedule;
    double mConfidence;
    SwapRateInfo mRate1, mRate2;
    LAString mTheta1ID, mTheta2ID, mRhoID, mPairID;
    string mCopType;
    vector<size_t> mMonths;
    size_t mStartLength, mEndLength;
    CurveInfo mDiscCurveInfo;

    DoubleMatrix mCopParams;
    DoubleVector mCMS1, mVol1, mCMS2, mVol2, mDF, mT, mAcc;
};

#endif 
