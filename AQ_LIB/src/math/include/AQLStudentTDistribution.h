/*! @file
    @brief Class declaration for Student t-distribution.
*/

// Improved performance and precision.
#ifndef AQLStudentTDistribution_h
#define AQLStudentTDistribution_h
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "AQLUnivariateDistribution.h"

class AQLStudentTDistribution : public AQLUnivariateDistribution
{
public:
    AQLStudentTDistribution(double nu_);
    AQLStudentTDistribution(double location_, double scale_, double nu_);

    double Density(double x);
    double Cumulative(double x);

private:
    void Set();

    double mNu, mThreshold, mP, mC, mHalfNu;
    bool mStandard;
};

#endif
