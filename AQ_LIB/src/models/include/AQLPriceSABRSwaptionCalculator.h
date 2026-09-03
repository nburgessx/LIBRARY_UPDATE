#ifndef AQLSABRSwaptionCalculator_h
#define AQLSABRSwaptionCalculator_h

#ifdef __GNUG__
#pragma interface
#endif
//////////
#include "AQLPriceSwaptionCalculator.h"
#include "AQLMathSABR.h"

using namespace std;

class AQLSABRSwaptionCalculator : public AQLPriceSwaptionCalculator
{
public:
    AQLSABRSwaptionCalculator(double S0_, const vector<double>& sabrParameters_);
    virtual double Price(double t, double strike, bool isCall);
    double BoundStrike(double t, double confidence, bool isUp);
    vector<double> StrikeGrid(double t, double confidence, size_t nPoints);

protected:
    double SABRPrice(double t, double strike, bool isCall);
    double SABRIV(double t, double strike);

    vector<double> mSabrParameters;
    AQLMathSABR_Hagan mSabr;

private:
    void CheckParameters();
};

#endif 
