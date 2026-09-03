#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// Funciton ID of LAMathCorFuncLMM
#define FN_CORFUNCLMM	10014
// Function Name of LAMathCorFuncLMM
#define FN_CORFUNCLMM_STR	"fn_colfunclmm"



class LAMathCorFuncLMM : public AQLFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit LAMathCorFuncLMM(double TMax, double x, double y);
	// destructor
	virtual ~LAMathCorFuncLMM(void);
	// copy constructor
	LAMathCorFuncLMM(const LAMathCorFuncLMM &rhs);

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

	
private :
	double mTMax;               // tenor max term
	double mx;                  // calib param x
	double my;		            // calib param y

};
