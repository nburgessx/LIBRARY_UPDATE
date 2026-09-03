/*! @file
    @brief Class declaration for Student t-distribution.
*/

// Improved performance & precision by Mizuho International London.
#ifndef LAStudentTDistribution_h
#define LAStudentTDistribution_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAStudentTDistribution.h
//
//  DESCRIPTION :       Student t-distribution.
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

class LAStudentTDistribution : public LAUnivariateDistribution
{
public:
    LAStudentTDistribution(double nu_);
    LAStudentTDistribution(double location_, double scale_, double nu_);

    double Density(double x);
    double Cumulative(double x);

private:
    void Set();

    double mNu, mThreshold, mP, mC, mHalfNu;
    bool mStandard;
};

#endif
