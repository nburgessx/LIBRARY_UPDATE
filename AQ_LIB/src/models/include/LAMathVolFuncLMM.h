#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <algorithm>
#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// Funciton ID of LAMathVolFuncLMM
#define FN_VOLFUNCLMM	10011
// Function Name of LAMathVolFuncLMM
#define FN_VOLFUNCLMM_STR	"fn_volfunclmm"


class LAMathVolFuncLMM : public LAFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit LAMathVolFuncLMM(const LAFunctionBase *funcTenor, const LAFunctionBase *funcTerm, 
				const DoubleArray &TFix, const DoubleArray &TFix_30_360, int suffix, double multiple, bool delFlg = false);
	// destructor
	virtual ~LAMathVolFuncLMM(void);
	// copy constructor
	LAMathVolFuncLMM(const LAMathVolFuncLMM &rhs);

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

	
protected :
	const LAFunctionBase *mpFuncTenor;		// pointer to tenor function
	const LAFunctionBase *mpFuncTerm;		// pointer to term function
	DoubleArray mTFix;					     // fixing  grid
	DoubleArray mTFix_30_360;			     // 30/360 fixing grid
	DoubleArray m_t;					    // term for calculation
	double m_T;                              // libor fixing  
	double mMultiple;                         // multiple
	bool  mDelFlg;                          // member function delete flag 

};
