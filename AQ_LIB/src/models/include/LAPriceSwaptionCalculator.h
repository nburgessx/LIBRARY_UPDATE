#ifndef LAPriceSwaptionCalculator_h
#define LAPriceSwaptionCalculator_h

#ifdef __GNUG__
#pragma interface
#endif
//////////
#include <vector>
#include <string>

using namespace std;

//================ LAPriceSwaptionCalculator ===================================
class LAPriceSwaptionCalculator
{
public:
    LAPriceSwaptionCalculator(double S0_);
    virtual ~LAPriceSwaptionCalculator();
    virtual double Price(double t, double strike, bool isCall) = 0;
    virtual double BoundStrike(double t, double confidence, bool isUp) = 0;
    virtual vector<double> StrikeGrid(double t, double confidence, size_t nPoints) = 0;

protected:
    double mS0;
};

//================ Utilities ===================================
LAPriceSwaptionCalculator* GetSwaptionCalculator(string type, double fwd, const vector<double>& sabrParameters,
                                            const vector<double>& extraParameters);

#endif 
