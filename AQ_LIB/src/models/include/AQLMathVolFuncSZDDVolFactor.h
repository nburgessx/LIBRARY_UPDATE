#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//+++++ INCLUDE +++++
#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLInterpolationBase.h"
#include "AQLAlgorithm.h"
#include "AQLGaussLegendre.h"
#include "AQL1DDataSet.h"
#include "AQLMathHWFuncTool.h"
#include "AQLMathVolFuncHW.h"
#include <map>
//
//------------------------------ AQLMathVolFuncSZDDVolFactor ------------------------------
//

//+++++ DEFINE +++++
// Funciton ID of AQLMathVolFuncSZDDVolFactor
#define FN_VOLFUNCSZDDVOLFACTOR	10064
// Function Name of AQLMathVolFuncSZDDVolFactor
#define N_VOLFUNCSZDDVOLFACTOR_STR	"fn_volfunczsddvolfactor"

//
class AQLMathHWFuncMR;
class AQLMathHWFuncSigma;

class AQLMathVolFuncSZDDVolFactor : public AQLMathVolFuncHW
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
    AQLMathVolFuncSZDDVolFactor( AQLMathHWFuncMR& HW_a_, AQLMathHWFuncSigma& HW_s_, AQL1DDataSet& HW_theta_);
                                //======================================
	                            //copy constructor
    AQLMathVolFuncSZDDVolFactor(const AQLMathVolFuncSZDDVolFactor &rhs);
                                //======================================
	                            //destructor
    virtual ~AQLMathVolFuncSZDDVolFactor(void);
                                //======================================
    //  QUERY
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
								// Return theta
	const AQLFunctionBase*		getTheta() const {return mpThetaFunc;}
	                            //==========================================

protected:
	const AQL1DDataSet*			mpThetaFunc;
	
};

