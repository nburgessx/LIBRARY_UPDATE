/*! @file
    @brief Class declaration for Normal distribution.
*/

// Improved performance and precision.
#ifndef AQLNormalDistribution_h
#define AQLNormalDistribution_h
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "AQLUnivariateDistribution.h"

class AQLNormalDistribution : public AQLUnivariateDistribution
{
public:
    AQLNormalDistribution();
    AQLNormalDistribution(double mean_, double stDev_);

    double Density(double x);
    double Cumulative(double x);
    double CumulativeInverse(double y);

private:
    bool mStandard;
};

#endif
