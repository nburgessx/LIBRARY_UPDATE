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
#include "LAMathHWFuncTool.h"
#include <map>
//
//------------------------------ LAMathVolFuncHW ------------------------------
//

//+++++ DEFINE +++++
// Funciton ID of LAMathVolFuncHW
#define FN_VOLFUNCHW	10034
// Function Name of LAMathVolFuncHW
#define N_VOLFUNCHW_STR	"fn_volfunchw"

//
class LAMathHWFuncMR;
class LAMathHWFuncSigma;

class LAMathVolFuncHW : public AQLFunctionBase
{
public :
//  LIFECYCLE
                                //======================================
	                            // constructor
    LAMathVolFuncHW( LAMathHWFuncMR& HW_a_, LAMathHWFuncSigma& HW_s_);
                                //======================================
	                            //copy constructor
    LAMathVolFuncHW(const LAMathVolFuncHW &rhs);
                                //======================================
	                            //destructor
    virtual ~LAMathVolFuncHW(void);
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


	const LAMathHWFuncToolForVar*		getHWFuncTool(void) const {return mpforVar;}; 
	
protected :
	AQLGaussLegendre mGL;
	LAMathHWFuncToolForVar* mpforVar;
    bool is_cloned;
	mutable std::map<double, double> integrate_cache;
	mutable std::map<double, bool> is_cache;
	
};

