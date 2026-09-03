#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"



// Funciton ID of LAExpMethod
#define FN_EXPFUNC	1208
// Function Name of LAExpMethod
#define FN_EXPFUNC_STR	"fn_expfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent exponential function

    This class derives from LAFunctionBase

*/
class LAExpMethod : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LAExpMethod(double a, const LAFunctionBase& method);
	// destructor
	virtual ~LAExpMethod();
	// copy constructor
	LAExpMethod(const LAExpMethod& method);

//  QUERY
								//======================================
								// Check function for this class ID
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

	LAExpMethod & operator=( const LAExpMethod & ) { return *this; }

	
	// 20060929 override a virtual function of the base class
	virtual double				operator()(double x) const
								{
									return LAFunctionBase::operator()(x);
								};
	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return LAFunctionBase::operator()(pos, x);
								};

	                            //==========================================
	                            // return partial derivative result
	virtual double				partialDerivative(const DoubleArray& x, unsigned int pos,
										CALC_TYPE calctype = DEFAULT, DIFF_TYPE difftype = BOTHSIDE,double delta = 0.0001) const;

	                            //==========================================
	                            // return second partial derivative result
	virtual double				partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype = DEFAULT, double delta = 0.0001) const;


	                            //==========================================
								// check x is in domain of this function or not.
	virtual	bool				isInDomain(const DoubleArray& x) const {return mpFunc->isInDomain(x);};
								//==========================================
	                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos) const
								{return mpFunc->isDifferentiable(x, pos);};
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj) const
								{return mpFunc->isDifferentiable(x, posi, posj);};


//  OPERATION
								//==========================================
	                            // set check flag
	virtual void				setCheckFlag(bool checkflag = true);

private:
	LAFunctionBase*	mpFunc;		// pointer to fuction as power
protected:
	

};

