#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// Funciton ID of LAMathCorrelationFuncLMM
#define FN_CORRELATIONFUNCLMM	10056
// Function Name of LAMathCorrelationFuncLMM
#define FN_CORRELATIONFUNCLMM_STR	"fn_corelationfunclmm"


class LAMathCorrelationFuncLMM : public LAFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit LAMathCorrelationFuncLMM(void);
	// destructor
	virtual ~LAMathCorrelationFuncLMM(void);
	// copy constructor
	LAMathCorrelationFuncLMM(const LAMathCorrelationFuncLMM &rhs);

								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual LACoreFunctionBase*     clone() const;
								//======================================
								// return this class type
    virtual function_t          getType() const;

	                            //==========================================
	                            // return function value
	virtual double				operator()(const DoubleArray& x) const;	
	
		                        //==========================================
	                            // return function value
	virtual double				operator()(double t) const;	

		                        //==========================================
	                            // return 
    virtual double				get(double t, double T1, double T2) const { (void)t; (void)T1; (void)T2; return 1; }

		                        //==========================================
	                            // return correlation matrix
	DoubleMatrix				getCorrMat(double t, const DoubleArray& T_fix);

};
