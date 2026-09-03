/*! @file
    @brief Class declaration for bivariate copulas.
*/

// Improved performance & precision by AlgoQuantHub.
#ifndef LABivariateCopula_h
#define LABivariateCopula_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABivariateCopula.h
//
//  DESCRIPTION :       Base class for bivariate copulas.
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       FIX
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif

enum DiffDimension { First, Second };

class LABivariateCopula
{
public:
    virtual ~LABivariateCopula() {}
    virtual double Value(double x, double y) = 0;
    virtual double Diff(double x, double y, DiffDimension d) = 0;
};

#endif
