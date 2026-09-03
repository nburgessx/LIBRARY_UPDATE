#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// Funciton ID of LAMathVolFuncStructureBase
#define FN_VOLFUNCSTRUCTUREBASE	10010
// Function Name of LAMathVolFuncStructureBase
#define FN_VOLFUNCSTRUCTUREBASE_STR	"fn_volfuncstructurebase"


class LAMathVolFuncStructureBase : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	explicit LAMathVolFuncStructureBase(double a, double b, double c, double d);
	// destructor
	virtual ~LAMathVolFuncStructureBase(void);
	// copy constructor
	LAMathVolFuncStructureBase(const LAMathVolFuncStructureBase &rhs);

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

	
private:
	double ma;                 // calib param a
	double mb;                 // calib param b
	double mc;                 // calib param c
	double md;                 // calib param d

};

