#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LA1DIntegral.h"
#include "LAFunctionBase.h"

#include <vector>
using namespace std;

// ID for MMNaturalIntegral
#define FN_EULERINTEGRAL     20090420
// Function name for MMNaturalIntegral
#define FN_EULERINTEGRAL_STR	"fn_eulerIntegral"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class for Natural integral methods

    This class itself derives from abstract class "LAIntegralBase". This class has pure
	virtual method "get".

*/
class LAEulerIntegral : public LA1DIntegral
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAEulerIntegral(unsigned short numberOfPoints);
	// Destructor
	virtual ~LAEulerIntegral();

//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LAEulerIntegral*  clone() const { return  new LAEulerIntegral(*this); }
								//======================================
								// Return this class type
    virtual function_t          getType() const;

	//======================================
	// method for 1-dim integration
	virtual double	integrate(const LAFunctionBase& f,double xl,double xu) const;

	//
	virtual double	integrate(const LAFunctionBase& f,
							  const std::vector<std::pair<double,double> >& x) const
					{
						return LA1DIntegral::integrate(f, x);
					};
	virtual double	integrate(const LAFunctionBase& f, double xl, double xu, const std::set<double>&/*DoubleArray&*/ grids) const
					{
						return LA1DIntegral::integrate(f, xl, xu, grids);
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

    This class itself derives from abstract class "LAIntegralBase". This class has pure
	virtual method "get".

*/
class LAPCIntegral : public LA1DIntegral
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAPCIntegral(const vector<double>& x_);
	// Destructor
	virtual ~LAPCIntegral();

//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LAPCIntegral*  clone() const { return  new LAPCIntegral(*this); }
								//======================================
								// Return this class type
    virtual function_t          getType() const;

	//======================================
	// method for 1-dim integration
	virtual double	integrate(const LAFunctionBase& f,double xl,double xu) const;

	//
	virtual double	integrate(const LAFunctionBase& f,
							  const std::vector<std::pair<double,double> >& xx) const
					{
						return LA1DIntegral::integrate(f, xx);
					};
	virtual double	integrate(const LAFunctionBase& f, double xl, double xu, const std::set<double>&/*DoubleArray&*/ grids) const
					{
						return LA1DIntegral::integrate(f, xl, xu, grids);
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
