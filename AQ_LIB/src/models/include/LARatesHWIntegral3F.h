#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LAModelDynamicsHW1FCurve.h"
#include "LARatesHWIntegral.h"


// ID for LARatesHWIntegral3F
#define FN_HWINTEGRAL3F	10129
// Function name for LARatesHWIntegral3F
#define FN_HWINTEGRAL3F_STR	"fn_hwintegral3f"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of Euler-Maruyama sde integral

*/
class LARatesHWIntegral3F : public LARatesHWIntegral
{
public:
//  LIFECYCLE
                                //======================================
	                            // Default constructor
	explicit LARatesHWIntegral3F(SDEINTEGRAL_TYPE type);
		                            // Default constructor
	explicit LARatesHWIntegral3F(SDEINTEGRAL_TYPE type, const AQLString& sdeAttrName);
                                //======================================
	                            //	Copy constructor
	LARatesHWIntegral3F(const LARatesHWIntegral3F& v);
                                //======================================
	                            // Destructor
	virtual ~LARatesHWIntegral3F();
                                //======================================

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// excecute integral
	inline 	virtual void		integral(double ts, double te, 
										std::vector<AQLFunctionBase*>::const_iterator drift,										
										std::vector<std::vector<AQLFunctionBase*> >::const_iterator vol,
										DoubleArray::const_iterator	bm,
										SCALARARRAY::iterator	x_in_out,	
										unsigned int varnum
								) const;
								//======================================
								//set up initial value 
    //virtual void                setUpInitialVal(const LARatesPathElementBase& curve0, const SCALARARRAY& rate0);
    virtual void                setUpInitialVal(const SCALARARRAY& rate0);
};
