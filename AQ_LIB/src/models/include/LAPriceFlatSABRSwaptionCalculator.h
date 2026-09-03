#ifndef MVFlatSABRSwaptionCalculator_h
#define MVFlatSABRSwaptionCalculator_h

#ifdef __GNUG__
#pragma interface
#endif
//////////
#include "LAPriceSABRSwaptionCalculator.h"

class MVFlatSABRSwaptionCalculator : public MVSABRSwaptionCalculator
{
public:
    MVFlatSABRSwaptionCalculator(double S0_, const vector<double>& sabrParameters_, const vector<double>& extraParameter_);
    double Price(double t, double strike, bool isCall);

private:
    void CheckCache(double t);

    double mLeftCutOff, mRightCutOff;
    double mTThreshold, mTCache;
    double mKl, mVl;
    double mKr, mVr;
};

#endif 
