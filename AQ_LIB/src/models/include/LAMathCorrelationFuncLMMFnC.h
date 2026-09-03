/*! @file
    @brief Class declaration to represent LMM correlation function
	This class derives from LAMathCorrelationFuncLMM
*/

#ifdef __GNUG__
#pragma interface
#endif

#pragma once

#include "AQLFunctionBase.h"
#include "LAMathCorrelationFuncLMM.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LAModelUtilities.h"

// Funciton ID of LAMathCorrelationFuncLMMFnC
#define FN_CORRELATIONFUNCLMMFNC	10058

// Function Name of LAMathCorrelationFuncLMM
#define FN_CORRELATIONFUNCLMMFNC_STR	"fn_corelationfunclmmfnc"

#include <stdio.h>


class LAMathCorrelationFuncLMMFnC : public LAMathCorrelationFuncLMM
{
public :

	// constructor
	explicit LAMathCorrelationFuncLMMFnC(double x, double y);

	// destructor
	virtual ~LAMathCorrelationFuncLMMFnC(void);

	// copy constructor
	LAMathCorrelationFuncLMMFnC(const LAMathCorrelationFuncLMMFnC &rhs);

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
        return  my + (1 - my) * exp( -mx * LAModelUtilities::abs_( sqrt( T1 - t ) - sqrt( T2 - t ) ) );
    }

protected :
	double mx;                  // calib param x
	double my;		            // calib param y

};
