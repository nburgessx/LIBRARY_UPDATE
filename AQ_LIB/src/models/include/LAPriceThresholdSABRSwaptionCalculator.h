#ifndef MVThresholdSABRSwaptionCalculator_h
#define MVThresholdSABRSwaptionCalculator_h

#ifdef __GNUG__
#pragma interface
#endif
//////////
#include "LAPriceSABRSwaptionCalculator.h"

class MVThresholdSABRSwaptionCalculator : public MVSABRSwaptionCalculator
{
public:
    MVThresholdSABRSwaptionCalculator(double S0_, const vector<double>& sabrParameters_,
                                      const vector<double>& extraParameters_);
    double Price(double t, double strike, bool isCall);

private:
    double mThreshold;
};

#endif 
