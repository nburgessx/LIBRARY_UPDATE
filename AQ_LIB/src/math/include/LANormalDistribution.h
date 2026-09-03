/*! @file
    @brief Class declaration for Normal distribution.
*/

// Improved performance & precision by Mizuho International London.
#ifndef LANormalDistribution_h
#define LANormalDistribution_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LANormalDistribution.h
//
//  DESCRIPTION :       Normal distribution.
//  SEE ALSO    :
//  VIRSION		:
//  STATUS      :       FIX
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "LAUnivariateDistribution.h"

class LANormalDistribution : public LAUnivariateDistribution
{
public:
    LANormalDistribution();
    LANormalDistribution(double mean_, double stDev_);

    double Density(double x);
    double Cumulative(double x);
    double CumulativeInverse(double y);

private:
    bool mStandard;
};

#endif
