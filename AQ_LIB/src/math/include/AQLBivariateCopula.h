/*! @file
    @brief Class declaration for bivariate copulas.
*/

// Improved performance and precision.
#ifndef AQLBivariateCopula_h
#define AQLBivariateCopula_h
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif

enum DiffDimension { First, Second };

class AQLBivariateCopula
{
public:
    virtual ~AQLBivariateCopula() {}
    virtual double Value(double x, double y) = 0;
    virtual double Diff(double x, double y, DiffDimension d) = 0;
};

#endif
