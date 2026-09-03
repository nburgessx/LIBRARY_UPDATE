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
#include "AQLMathHWFuncTool.h"
#include <map>
//
//------------------------------ AQLMathVolFuncHW ------------------------------
//

//+++++ DEFINE +++++
// Funciton ID of AQLMathVolFuncHW
#define FN_VOLFUNCHW	10034
// Function Name of AQLMathVolFuncHW
#define N_VOLFUNCHW_STR	"fn_volfunchw"

//
class AQLMathHWFuncMR;
class AQLMathHWFuncSigma;

class AQLMathVolFuncHW : public AQLFunctionBase
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
    AQLMathVolFuncHW( AQLMathHWFuncMR& HW_a_, AQLMathHWFuncSigma& HW_s_);
                                //======================================
	                            //copy constructor
    AQLMathVolFuncHW(const AQLMathVolFuncHW &rhs);
                                //======================================
	                            //destructor
    virtual ~AQLMathVolFuncHW(void);
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
	                            // return function value
    virtual double              operator()(const DoubleArray& x) const;
	                            //==========================================


	const AQLMathHWFuncToolForVar*		getHWFuncTool(void) const {return mpforVar;}; 
	
protected :
	AQLGaussLegendre mGL;
	AQLMathHWFuncToolForVar* mpforVar;
    bool is_cloned;
	mutable std::map<double, double> integrate_cache;
	mutable std::map<double, bool> is_cache;
	
};

