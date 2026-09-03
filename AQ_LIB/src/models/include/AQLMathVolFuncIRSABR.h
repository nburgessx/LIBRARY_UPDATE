#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include "AQLAlgorithm.h"
#include "AQLDataInstance.h"



// Funciton ID of AQLMathVolFuncIRSABR
#define FN_VOLFUNCIRSABR	10059
// Function Name of AQLMathVolFuncIRSABR
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

class AQLMathVolFuncIRSABR : public AQLFunctionBase
{
public :
//  LIFECYCLE
	// constructor
	explicit AQLMathVolFuncIRSABR(AQLDataInstance* dataInstance);
	// destructor
	virtual ~AQLMathVolFuncIRSABR(void);
	// copy constructor
	AQLMathVolFuncIRSABR(const AQLMathVolFuncIRSABR &rhs);

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
	                            // return swapconv id
	AQLString					getSwapConvID(void) const;
								//==========================================
	                            // return capconv id
	AQLString					getCapConvID(void) const;
								 //==========================================
		                        // return parameter ID
	AQLString					getParamID(const AQLString& paramName) const;
								 //==========================================
	                            // set swapconv id
	AQLString					setSwapConvID(const AQLString& ID) {return mSwapConvID[mTargetUnderlying] = ID;};
								//==========================================
	                            // set capconv id
	AQLString					setCapConvID(const AQLString& ID) {return mCapConvID[mTargetUnderlying] = ID;};
								//==========================================
								// set sabr parmeter ID
	void						setParamID(const AQLString& ID, const AQLString& paramName);
								//==========================================
								// set target underlying
	void						setUnderlying(const AQLString& underlying) const;
								//==========================================
		                        // return sabr parameter
	double						getSABRParam(const AQLString& paramName, double expPoint, double tenorPoint) const;
								//==========================================
								// set approxmethod
	void						setApproxmethod(const AQLString& approxmethod);
								//==========================================
								// get approxmethod
    AQLString				    getApproxmethod() const {return mApproxmethod;}
								//==========================================
								// set forwardshiftvalue
	void						setForwardShiftValue(const double forwardShiftValue);
								//==========================================
								// get forwardshiftvalue
    double						getForwardShiftValue() const {return mForwardShiftValue;}

protected:
	AQLDataInstance* mpDataInstance;
	std::map<AQLString, AQLString> mAlphaID;
	std::map<AQLString, AQLString> mBetaID;
	std::map<AQLString, AQLString> mNuID;
	std::map<AQLString, AQLString> mRhoID;
	std::map<AQLString, AQLString> mSwapConvID;
	std::map<AQLString, AQLString> mCapConvID;
	mutable AQLString mTargetUnderlying;
	AQLString mApproxmethod;
	double mForwardShiftValue;
};

