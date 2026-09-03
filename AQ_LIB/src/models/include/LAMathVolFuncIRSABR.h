#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAInterpolationBase.h"
#include "LAAlgorithm.h"
#include "LADataInstance.h"



// Funciton ID of LAMathVolFuncIRSABR
#define FN_VOLFUNCIRSABR	10059
// Function Name of LAMathVolFuncIRSABR
#define FN_VOLFUNCIRSABR_STR	"fn_volfuncirsabr"


#ifndef SABR_ALPHA
#define SABR_ALPHA	"ALPHA"
#endif
#ifndef SABR_BETA
#define SABR_BETA	"BETA"
#endif
#ifndef SABR_NU
#define SABR_NU	"NU"
#endif
#ifndef SABR_RHO
#define SABR_RHO	"RHO"
#endif

class LAMathVolFuncIRSABR : public LAFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit LAMathVolFuncIRSABR(LADataInstance* dataInstance);
	// destructor
	virtual ~LAMathVolFuncIRSABR(void);
	// copy constructor
	LAMathVolFuncIRSABR(const LAMathVolFuncIRSABR &rhs);

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
	                            // return swapconv id
	LAString					getSwapConvID(void) const;
								//==========================================
	                            // return capconv id
	LAString					getCapConvID(void) const;
								 //==========================================
		                        // return parameter ID
	LAString					getParamID(const LAString& paramName) const;
								 //==========================================
	                            // set swapconv id
	LAString					setSwapConvID(const LAString& ID) {return mSwapConvID[mTargetUnderlying] = ID;};
								//==========================================
	                            // set capconv id
	LAString					setCapConvID(const LAString& ID) {return mCapConvID[mTargetUnderlying] = ID;};
								//==========================================
								// set sabr parmeter ID
	void						setParamID(const LAString& ID, const LAString& paramName);
								//==========================================
								// set target underlying
	void						setUnderlying(const LAString& underlying) const;
								//==========================================
		                        // return sabr parameter
	double						getSABRParam(const LAString& paramName, double expPoint, double tenorPoint) const;
								//==========================================
								// set approxmethod
	void						setApproxmethod(const LAString& approxmethod);
								//==========================================
								// get approxmethod
    LAString				    getApproxmethod() const {return mApproxmethod;}
								//==========================================
								// set forwardshiftvalue
	void						setForwardShiftValue(const double forwardShiftValue);
								//==========================================
								// get forwardshiftvalue
    double						getForwardShiftValue() const {return mForwardShiftValue;}

protected:
	LADataInstance* mpDataInstance;
	std::map<LAString, LAString> mAlphaID;
	std::map<LAString, LAString> mBetaID;
	std::map<LAString, LAString> mNuID;
	std::map<LAString, LAString> mRhoID;
	std::map<LAString, LAString> mSwapConvID;
	std::map<LAString, LAString> mCapConvID;
	mutable LAString mTargetUnderlying;
	LAString mApproxmethod;
	double mForwardShiftValue;
};

