#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
 


// Funciton ID of LAAverageMethod
#define FN_AVERAGE	1213
// Function Name of LAAverageMethod
#define FN_AVERAGE_STR	"fn_average"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent average function.

	This class derives from LAFunctionBase

*/
class LAAverageMethod : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LAAverageMethod();
	// destructor
	virtual ~LAAverageMethod();

	// assignment operator
	LAAverageMethod & operator=( const LAAverageMethod & ) { return *this; }


//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;
								
	                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
								// return function value
	virtual double				operator()(double x) const
								{
									return LAFunctionBase::operator()(x);								
								}


	// 20060929 override virtual function of base class
	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return LAFunctionBase::operator()(pos, x);
								};

	                            //==========================================
								// return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const
								{
									return LAFunctionBase::integral(x);
								}

	                            //==========================================
								// return integral result
	virtual double				integral(double xl, double xu) const
								{
									return LAFunctionBase::integral(xl, xu);
								}
	
	// 20060929 override virtual function of base class
	virtual double				integral(const std::vector<std::pair<double,double> >& x,
										const LAIntegralBase* pIntegral) const
								{
									return LAFunctionBase::integral(x, pIntegral);
								};
	virtual double				integral(double xl, double xu,
										const LA1DIntegral* pIntegral) const
								{
									return LAFunctionBase::integral(xl, xu, pIntegral);
								};

	                            //==========================================
								// return partial derivative value
	virtual double				partialDerivative(const DoubleArray& x, unsigned int pos,
										CALC_TYPE calctype = DEFAULT, DIFF_TYPE difftype = BOTHSIDE,double delta = 0.0001) const;

	                            //==========================================
								// return second partial deribative value
	virtual double				partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype = DEFAULT, double delta = 0.0001) const;


	                            //==========================================
								// check x is in domain of this function or not.
	virtual	bool				isInDomain(const DoubleArray& x) const {  (void)x; return true;};
								//==========================================
	                            // check derivable or not.
                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos) const { (void)x; (void)pos; return true;};
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj) const { (void)x; (void)posi; (void)posj; return true;};
//  OPERATION

private:

protected:
	

};
