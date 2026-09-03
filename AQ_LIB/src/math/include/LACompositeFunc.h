#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// ID for LACompositeMethod
#define FN_COMPOSITEFUNC	1215 
// Function name for LACompositeMethod
#define FN_COMPOSITEFUNC_STR	"fn_compositefunc" 


// operation type
enum OPERATION_TYPE {	
	ADDITION,
	SUBTRACTION,
	MULTIPLICATION,
	DIVISION
};
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent composite function of two functions

    This class derives from LAFunctionBase

*/
class LACompositeMethod : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LACompositeMethod(const LAStringVector& var,
					const LAFunctionBase& func1, const LAStringVector& var1,
					const LAFunctionBase& func2, const LAStringVector& var2,
					OPERATION_TYPE type,					
					bool checkflag = false);
	// copy constructor
	LACompositeMethod(const LACompositeMethod& v);
	// destructor
	virtual ~LACompositeMethod();

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
	                            //==========================================
	// 20060929 override a virtual function of the base class
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
	virtual	bool				isInDomain(const DoubleArray& x)const;
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

	LACompositeMethod & operator=( const LACompositeMethod & ) { return *this; }


private:
protected:
	OPERATION_TYPE	mType;// operation type
	LAFunctionBase* mpF1;// function1
	LAFunctionBase* mpF2;// function2
	UintArray		mVarPos1;
	UintArray		mVarPos2;
	mutable DoubleArray	mVar1;// variable for function1
	mutable DoubleArray	mVar2;// variable for function2

};

