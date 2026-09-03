#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// Funciton ID of LAMathVolFuncWave
#define FN_VOLFUNCWAVE	10009
// Function Name of LAMathVolFuncWave
#define FN_VOLFUNCWAVE_STR	"fn_volfuncwave"




class LAMathVolFuncWave : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	explicit LAMathVolFuncWave(double Tmax, double ep0, double ep1,
					double ep2, double ep3, double ep4, double ep5, double ep6, double ep7);
	// destructor
	virtual ~LAMathVolFuncWave(void);
	// copy constructor
	LAMathVolFuncWave(const LAMathVolFuncWave &rhs);

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

	
private :
	double mAlpha;    // calib param alpha
	double mDecay;     // calib param decay
	double mAmp1;      // calib param amp1
	double mPhase1;    // calib param phase1
	double mAmp2;      // calib param amp2
	double mPhase2;    // calib param phase2
	double mAmp3;      // calib param amp3
	double mPhase3;    // calib param phase3
	double mShift;     // calib param shift

};
