#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"



// Funciton ID of LAShiftMethod
#define FN_SHIFTFUNC	1209
// Function Name of LAShiftMethod
#define FN_SHIFTFUNC_STR	"fn_shiftfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent shift function f(x+a)

    This class derives from LAFunctionBase

*/
class LAShiftMethod : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LAShiftMethod(const DoubleArray& a, const LAFunctionBase& method);
	// destructor
	virtual ~LAShiftMethod();
	// copy constructor
	LAShiftMethod(const LAShiftMethod& method);

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
	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return LAFunctionBase::operator()(pos, x);
								};
	virtual double				operator()(double x) const;

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
	virtual	bool				isInDomain(const DoubleArray& x) const
								{
									DoubleArray xx = x;
									for(unsigned int i = 0; i < xx.size(); i++)
										xx[i] += mParam.at(i);
									return mpFunc->isInDomain(xx);
								}
								//==========================================
	                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos)const
								{
									DoubleArray xx = x;
									for(unsigned int i = 0; i < xx.size(); i++)
										xx[i] += mParam.at(i);
									return mpFunc->isDifferentiable(xx, pos);
								}
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj)const
								{
									DoubleArray xx = x;
									for(unsigned int i = 0; i < xx.size(); i++)
										xx[i] += mParam.at(i);
									return mpFunc->isDifferentiable(xx, posi, posj);
								}


//  OPERATION
								//==========================================
	                            // set check flag
	virtual void				setCheckFlag(bool checkflag = true);

	// assignment operator
	LAShiftMethod & operator=( const LAShiftMethod & ) { return *this; }

private:
	LAFunctionBase*	mpFunc;		// pointer to fuction
protected:
	

};

