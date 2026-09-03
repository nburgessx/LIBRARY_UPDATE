#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLModelDynamicsHW1FCurve.h"
#include "AQLRatesHWIntegral.h"


// ID for AQLRatesHWIntegral3F
#define FN_HWINTEGRAL3F	10129
// Function name for AQLRatesHWIntegral3F
#define FN_HWINTEGRAL3F_STR	"fn_hwintegral3f"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of Euler-Maruyama sde integral

*/
class AQLRatesHWIntegral3F : public AQLRatesHWIntegral
{
public:
//  LIFECYCLE
                                //======================================
	                            // Default constructor
	explicit AQLRatesHWIntegral3F(SDEINTEGRAL_TYPE type);
		                            // Default constructor
	explicit AQLRatesHWIntegral3F(SDEINTEGRAL_TYPE type, const AQLString& sdeAttrName);
                                //======================================
	                            //	Copy constructor
	AQLRatesHWIntegral3F(const AQLRatesHWIntegral3F& v);
                                //======================================
	                            // Destructor
	virtual ~AQLRatesHWIntegral3F();
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
    //virtual void                setUpInitialVal(const AQLRatesPathElementBase& curve0, const SCALARARRAY& rate0);
    virtual void                setUpInitialVal(const SCALARARRAY& rate0);
};
