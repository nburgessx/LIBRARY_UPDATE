#ifndef AQLThresholdSABRSwaptionCalculator_h
#define AQLThresholdSABRSwaptionCalculator_h

#ifdef __GNUG__
#pragma interface
#endif
//////////
#include "AQLPriceSABRSwaptionCalculator.h"

class AQLThresholdSABRSwaptionCalculator : public AQLSABRSwaptionCalculator
{
public:
    AQLThresholdSABRSwaptionCalculator(double S0_, const vector<double>& sabrParameters_,
                                      const vector<double>& extraParameters_);
    double Price(double t, double strike, bool isCall);

private:
    double mThreshold;
};

#endif 
