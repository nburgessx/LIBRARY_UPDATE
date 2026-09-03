#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLMathFXVolatilitySurfaceGenerate.h"
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include "AQLAlgorithm.h"



// Funciton ID of AQLMathVolFuncFXStrangleSolver
#define FN_VOLFUNCFXSTRANGLESOLV	10020
// Function Name of AQLMathVolFuncFXStrangleSolver
#define FN_VOLFUNCFXSTRANGLESOLV_STR	"fn_volfuncfxstranglesolver"



class AQLMathVolFuncFXStrangleSolver : public AQLFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLMathVolFuncFXStrangleSolver(const InterpolationMethod &method, const InterpolationTarget &target, 
									const InterpolationVariable &variable, const ATMInterpolationMethod &atmMethod, 
									const std::vector<FXOptionData> &mopdata, const std::vector<SmileData> &smiledata,
									bool iswing);
	// destructor
	virtual ~AQLMathVolFuncFXStrangleSolver(void);
	// copy constructor
	AQLMathVolFuncFXStrangleSolver(const AQLMathVolFuncFXStrangleSolver &rhs);

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

