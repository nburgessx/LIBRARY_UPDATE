#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// ID for AQLCompositeMethod
#define FN_COMPOSITEFUNC	1215 
// Function name for AQLCompositeMethod
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

    This class derives from AQLFunctionBase

*/
class AQLCompositeMethod : public AQLFunctionBase
{
public:
	// constructor
	AQLCompositeMethod(const AQLStringVector& var,
					const AQLFunctionBase& func1, const AQLStringVector& var1,
					const AQLFunctionBase& func2, const AQLStringVector& var2,
					OPERATION_TYPE type,					
					bool checkflag = false);
	// copy constructor
	AQLCompositeMethod(const AQLCompositeMethod& v);
	// destructor
	virtual ~AQLCompositeMethod();

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
	                            //==========================================
	// 20060929 override a virtual function of the base class
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
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


								//==========================================
	                            // set check flag
	virtual void				setCheckFlag(bool checkflag = true);

	AQLCompositeMethod & operator=( const AQLCompositeMethod & ) { return *this; }


private:
protected:
	OPERATION_TYPE	mType;// operation type
	AQLFunctionBase* mpF1;// function1
	AQLFunctionBase* mpF2;// function2
	UintArray		mVarPos1;
	UintArray		mVarPos2;
	mutable DoubleArray	mVar1;// variable for function1
	mutable DoubleArray	mVar2;// variable for function2

};

