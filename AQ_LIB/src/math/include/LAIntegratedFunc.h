#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"



// Funciton ID of LAIntegrandMethod
#define FN_INTEGRATEDFUNC	1206
// Function Name of LAIntegrandMethod
#define FN_INTEGRATEDFUNC_STR	"fn_integratedfunc"


// integrated method type
enum IntegratedFuncType {
	TYPE_A,
	TYPE_B,
	TYPE_C
};

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of class to represent function integrated with respect to one parameter

	This class derives from LAFunctionBase

*/
class LAIntegrandMethod : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LAIntegrandMethod(IntegratedFuncType type,
					unsigned int pos,
					const DoubleArray& param,
					const LAFunctionBase& method,					
                    const LA1DIntegral* pIntegral = NULL);
	// destructor
	virtual ~LAIntegrandMethod();
	// copy constructor
	LAIntegrandMethod(const LAIntegrandMethod& method);

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

	// 20060929 override a virtual function of the base class
	virtual double				operator()(double x) const
								{
									return LAFunctionBase::operator()(x);
								};
	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return LAFunctionBase::operator()(pos, x);

								};
	LAIntegrandMethod & operator=( const LAIntegrandMethod & ) { return *this; }

	                            //==========================================
	                            // return partial derivative result
	virtual double				partialDerivative(const DoubleArray& x, unsigned int pos,
										CALC_TYPE calctype = DEFAULT, DIFF_TYPE difftype = BOTHSIDE,double delta = 0.0001) const;

	                            //==========================================
	                            // return second partial derivative result
	virtual double				partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype=DEFAULT, double delta = 0.0001) const;

								//==========================================
	                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos)const;
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi,unsigned int posj)const;


//  OPERATION
								//==========================================
	                            // set check flag
	virtual void				setCheckFlag(bool checkflag = true);

private:
	LAFunctionBase*	mpFunc;			// pointer to original function
	LA1DIntegral*	mpIntegral;		// pointer to integral method
	IntegratedFuncType mType;		// type of integration
	unsigned int mPos;				// location of specific variable
protected:
	

};

