#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// Funciton ID of AQLMathCorrelationFuncLMM
#define FN_CORRELATIONFUNCLMM	10056
// Function Name of AQLMathCorrelationFuncLMM
#define FN_CORRELATIONFUNCLMM_STR	"fn_corelationfunclmm"


class AQLMathCorrelationFuncLMM : public AQLFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLMathCorrelationFuncLMM(void);
	// destructor
	virtual ~AQLMathCorrelationFuncLMM(void);
	// copy constructor
	AQLMathCorrelationFuncLMM(const AQLMathCorrelationFuncLMM &rhs);

								//======================================
								// check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;
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
