#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// Funciton ID of AQLShiftMethod
#define FN_SHIFTFUNC	1209
// Function Name of AQLShiftMethod
#define FN_SHIFTFUNC_STR	"fn_shiftfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent shift function f(x+a)

    This class derives from AQLFunctionBase

*/
class AQLShiftMethod : public AQLFunctionBase
{
public:
	// constructor
	AQLShiftMethod(const DoubleArray& a, const AQLFunctionBase& method);
	// destructor
	virtual ~AQLShiftMethod();
	// copy constructor
	AQLShiftMethod(const AQLShiftMethod& method);

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
    virtual function_t          getType() const;

	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const;
	
	// 20060929 override a virtual function of the base class
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
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


								//==========================================
	                            // set check flag
	virtual void				setCheckFlag(bool checkflag = true);

	// assignment operator
	AQLShiftMethod & operator=( const AQLShiftMethod & ) { return *this; }

private:
	AQLFunctionBase*	mpFunc;		// pointer to fuction
protected:
	

};

