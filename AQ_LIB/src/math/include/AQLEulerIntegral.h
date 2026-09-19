#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQL1DIntegral.h"
#include "AQLFunctionBase.h"

#include <vector>
using namespace std;

// ID for AQLNaturalIntegral
#define FN_EULERINTEGRAL     20090420
// Function name for AQLNaturalIntegral
#define FN_EULERINTEGRAL_STR	"fn_eulerIntegral"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class for Natural integral methods

    This class itself derives from abstract class "AQLIntegralBase". This class has pure
	virtual method "get".

*/
class AQLEulerIntegral : public AQL1DIntegral
{
public:
	// Default constructor
	explicit AQLEulerIntegral(unsigned short numberOfPoints);
	// Destructor
	virtual ~AQLEulerIntegral();

								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLEulerIntegral*  clone() const { return  new AQLEulerIntegral(*this); }
								//======================================
								// Return this class type
    virtual function_t          getType() const;

	//======================================
	// method for 1-dim integration
	virtual double	integrate(const AQLFunctionBase& f,double xl,double xu) const;

	//
	virtual double	integrate(const AQLFunctionBase& f,
							  const std::vector<std::pair<double,double> >& x) const
					{
						return AQL1DIntegral::integrate(f, x);
					};
	virtual double	integrate(const AQLFunctionBase& f, double xl, double xu, const std::set<double>&/*DoubleArray&*/ grids) const
					{
						return AQL1DIntegral::integrate(f, xl, xu, grids);
					};

	//======================================
	//
	virtual void	get(DoubleArray& abscissas, 
								DoubleArray& weights,
								double a, double b) const;

protected:
	unsigned short mNumberOfPoints;		// Grid number
};

/*! 
    @brief Declaration of abstract base class for Natural integral methods

    This class itself derives from abstract class "AQLIntegralBase". This class has pure
	virtual method "get".

*/
class AQLPCIntegral : public AQL1DIntegral
{
public:
	// Default constructor
	explicit AQLPCIntegral(const vector<double>& x_);
	// Destructor
	virtual ~AQLPCIntegral();

								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLPCIntegral*  clone() const { return  new AQLPCIntegral(*this); }
								//======================================
								// Return this class type
    virtual function_t          getType() const;

	//======================================
	// method for 1-dim integration
	virtual double	integrate(const AQLFunctionBase& f,double xl,double xu) const;

	//
	virtual double	integrate(const AQLFunctionBase& f,
							  const std::vector<std::pair<double,double> >& xx) const
					{
						return AQL1DIntegral::integrate(f, xx);
					};
	virtual double	integrate(const AQLFunctionBase& f, double xl, double xu, const std::set<double>&/*DoubleArray&*/ grids) const
					{
						return AQL1DIntegral::integrate(f, xl, xu, grids);
					};

	//======================================
	//
	virtual void	get(DoubleArray& abscissas, 
								DoubleArray& weights,
								double a, double b) const;

protected:
	vector<double> x;		// Grid number
	size_t mNumberOfPoints;
};
