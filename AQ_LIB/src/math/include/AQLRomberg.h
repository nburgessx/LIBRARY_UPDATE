#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQL1DIntegral.h"

#include <limits>


// ID for AQLRomberg
#define FN_ROMBERG     1406
// Function name for AQLRomberg
#define FN_ROMBERG_STR	"fn_romberg"



class AQLFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration of integration by Romberg method

	This class derives from abstract base class "AQLIntegralBase", and 
	realize pure virtual method "integrate". In constructor, divident number of
	integral region should be specified. There are inner functions, "polint" and
	"trapzd".
	
*/
class AQLRomberg : public AQL1DIntegral
{
public:
//  LIFECYCLE
	// constructor
	AQLRomberg(unsigned short mpowertopoints, unsigned short morderofromberg=2, double romberg_eps = 1.0e-5);
	// destructor
	virtual ~AQLRomberg();

//  QUERY
								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								//======================================
								// integration by Romberg method
	virtual double				integrate(const AQLFunctionBase& f, double xl, double xu) 
								const;

	// 20060929 override a virtual function of the base class
	virtual double				integrate(const AQLFunctionBase& f,
									const std::vector<std::pair<double,double> >& x) const
								{
									return AQL1DIntegral::integrate(f, x);
								};
	virtual double				integrate(const AQLFunctionBase& f, double xl, double xu, const std::set<double>&/*DoubleArray&*/ grids) const
								{
									return AQL1DIntegral::integrate(f, xl, xu, grids);
								};

								//======================================

	// assignment operator
	AQLRomberg & operator=( const AQLRomberg & ) { return *this; }

private:
								//======================================
								// inner function for polynomial interpolation
	void						polint(DoubleArray &xa,
										DoubleArray &ya,
										const double x,
										double &y,
										double &dy) const;
								//======================================
								// inner function for trapezoidal rule
	double						trapzd(const AQLFunctionBase &f,
								const double a,
								const double b,
								const int n) const;

	unsigned short	mPowerToPoints;		// divident number is 2^{mPowerToPoints}
	unsigned short	mOrderOfRomberg;	// Romberg Order
    double          ROMBERG_EPS;        // romberg eps
protected:
};

