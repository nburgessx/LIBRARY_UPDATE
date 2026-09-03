#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include <algorithm>
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// Funciton ID of AQLMathVolFuncLMM
#define FN_VOLFUNCLMM	10011
// Function Name of AQLMathVolFuncLMM
#define FN_VOLFUNCLMM_STR	"fn_volfunclmm"


class AQLMathVolFuncLMM : public AQLFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLMathVolFuncLMM(const AQLFunctionBase *funcTenor, const AQLFunctionBase *funcTerm, 
				const DoubleArray &TFix, const DoubleArray &TFix_30_360, int suffix, double multiple, bool delFlg = false);
	// destructor
	virtual ~AQLMathVolFuncLMM(void);
	// copy constructor
	AQLMathVolFuncLMM(const AQLMathVolFuncLMM &rhs);

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

	
protected :
	const AQLFunctionBase *mpFuncTenor;		// pointer to tenor function
	const AQLFunctionBase *mpFuncTerm;		// pointer to term function
	DoubleArray mTFix;					     // fixing  grid
	DoubleArray mTFix_30_360;			     // 30/360 fixing grid
	DoubleArray m_t;					    // term for calculation
	double m_T;                              // libor fixing  
	double mMultiple;                         // multiple
	bool  mDelFlg;                          // member function delete flag 

};
