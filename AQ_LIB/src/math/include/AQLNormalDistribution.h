/*! @file
    @brief Class declaration for Normal distribution.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef AQLNormalDistribution_h
#define AQLNormalDistribution_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLNormalDistribution.h
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
