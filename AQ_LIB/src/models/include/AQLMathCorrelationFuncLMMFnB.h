#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <cmath>
#include <cstdlib>
#include "AQLFunctionBase.h"
#include "AQLMathCorrelationFuncLMM.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBasic.h"


// Funciton ID of AQLMathCorrelationFuncLMMFnB
#define FN_CORRELATIONFUNCLMMFNB	10057
// Function Name of AQLMathCorrelationFuncLMM
#define FN_CORRELATIONFUNCLMMFNB_STR	"fn_corelationfunclmmfnb"



class AQLMathCorrelationFuncLMMFnB : public AQLMathCorrelationFuncLMM
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLMathCorrelationFuncLMMFnB(double TMax, double x, double y);
	// destructor
	virtual ~AQLMathCorrelationFuncLMMFnB(void);
	// copy constructor
	AQLMathCorrelationFuncLMMFnB(const AQLMathCorrelationFuncLMMFnB &rhs);

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
    virtual double				get(double t, double T1, double T2) const
	{
		return AQLMath::exp( -AQLMath::abs(T1 - T2) * ( mx + my * ( 1.0 - (T1 - t + T2 - t) / ( 2.0 * ( mTMax - t ) ) ) ) );
	}

protected :
	double mTMax;               // tenor max term
	double mx;                  // calib param x
	double my;		            // calib param y

};
