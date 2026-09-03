#ifndef AQLBenaimSABRSwaptionCalculator_h
#define AQLBenaimSABRSwaptionCalculator_h

#ifdef __GNUG__
#pragma interface
#endif
//////////
#include "AQLPriceSABRSwaptionCalculator.h"

class AQLBenaimSABRSwaptionCalculator : public AQLSABRSwaptionCalculator
{
public:
    AQLBenaimSABRSwaptionCalculator(double S0_, const vector<double>& sabrParameters_, const vector<double>& extraParameter_,
                                   bool useLeft);
    double Price(double t, double strike, bool isCall);

private:
    void CheckCache(double t);

    double mLeftTail, mRightTail, mLeftCutOff, mRightCutOff;
    double mTThreshold, mTCache;
    double mKm, mAl, mBl, mCl;
    double mKp, mAr, mBr, mCr;

    bool mUseLeft;
    double mVolLeft;
};

#endif 
