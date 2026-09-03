#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathFXVolatilitySurfaceGenerate.h"
#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAInterpolationBase.h"
#include "LAAlgorithm.h"



// Funciton ID of LAMathVolFuncFXStrangleSolver
#define FN_VOLFUNCFXSTRANGLESOLV	10020
// Function Name of LAMathVolFuncFXStrangleSolver
#define FN_VOLFUNCFXSTRANGLESOLV_STR	"fn_volfuncfxstranglesolver"



class LAMathVolFuncFXStrangleSolver : public LAFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit LAMathVolFuncFXStrangleSolver(const InterpolationMethod &method, const InterpolationTarget &target, 
									const InterpolationVariable &variable, const ATMInterpolationMethod &atmMethod, 
									const std::vector<FXOptionData> &mopdata, const std::vector<SmileData> &smiledata,
									bool iswing);
	// destructor
	virtual ~LAMathVolFuncFXStrangleSolver(void);
	// copy constructor
	LAMathVolFuncFXStrangleSolver(const LAMathVolFuncFXStrangleSolver &rhs);

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

	InterpolationMethod getInterpolationMethod(void) const {return mMethod;};

	InterpolationTarget getInterpolationTarget(void)const {return mTarget;};
	
	InterpolationVariable getInterpolationVariable(void) const {return mVariable;};

	ATMInterpolationMethod getATMInterpolationMethod(void)const {return mAtmMethod;};

	std::vector<FXOptionData> getFXOptionData(void)const {return mOpData;};

	std::vector<SmileData> getSmileData(void)const {return mSmileData;};

	bool getIsWing(void)const {return mIsWing;};	
	
protected :
	InterpolationMethod mMethod;
	InterpolationTarget mTarget;
	
	mutable InterpolationVariable mVariable;
	mutable ATMInterpolationMethod mAtmMethod;

	
	std::vector<FXOptionData> mOpData;
	mutable	std::vector<SmileData> mSmileData;

	bool mIsWing;

	
};

