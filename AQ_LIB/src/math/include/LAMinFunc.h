#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"



// Funciton ID of LAMinMethod
#define FN_MIN	1212
// Function Name of LAMinMethod
#define FN_MIN_STR	"fn_min"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent min function.

	This class derives from LAFunctionBase

*/
class LAMinMethod : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LAMinMethod();
	// destructor
	virtual ~LAMinMethod();

	// assignment operator
	LAMinMethod & operator=( const LAMinMethod & ) { return *this; }


//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
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


	// 20060929 override a virtual function of the base class
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
	
	// 20060929 override a virtual function of the base class
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
	virtual	bool				isInDomain(const DoubleArray& x) const { (void)x; //20061017--David--Remove warning:C4100 20070411--Nagase--åxçêçÌèúÇgccÇ…Ç‡ëŒâû
												return true;};
								//==========================================
	                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos) const;
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj) const; 

//  OPERATION

private:

protected:
	

};

