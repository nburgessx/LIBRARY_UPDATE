#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//+++++ INCLUDE +++++
#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAInterpolationBase.h"
#include "LAAlgorithm.h"
#include "LAGaussLegendre.h"
#include "LA1DDataSet.h"
#include "LAMathHWFuncTool.h"
#include "LAMathVolFuncHW.h"
#include <map>
//
//------------------------------ LAMathVolFuncSZDDVolFactor ------------------------------
//

//+++++ DEFINE +++++
// Funciton ID of LAMathVolFuncSZDDVolFactor
#define FN_VOLFUNCSZDDVOLFACTOR	10064
// Function Name of LAMathVolFuncSZDDVolFactor
#define N_VOLFUNCSZDDVOLFACTOR_STR	"fn_volfunczsddvolfactor"

//
class LAMathHWFuncMR;
class LAMathHWFuncSigma;

class LAMathVolFuncSZDDVolFactor : public LAMathVolFuncHW
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
    LAMathVolFuncSZDDVolFactor( LAMathHWFuncMR& HW_a_, LAMathHWFuncSigma& HW_s_, LA1DDataSet& HW_theta_);
                                //======================================
	                            //copy constructor
    LAMathVolFuncSZDDVolFactor(const LAMathVolFuncSZDDVolFactor &rhs);
                                //======================================
	                            //destructor
    virtual ~LAMathVolFuncSZDDVolFactor(void);
                                //======================================
    //  QUERY
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
								// Return theta
	const LAFunctionBase*		getTheta() const {return mpThetaFunc;}
	                            //==========================================

protected:
	const LA1DDataSet*			mpThetaFunc;
	
};

