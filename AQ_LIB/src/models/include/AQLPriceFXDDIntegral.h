#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// ID for AQLPriceFXDDIntegral
#define FN_FXDDINTEGRAL	10038
// Function name for AQLPriceFXDDIntegral
#define FN_FXDDINTEGRAL_STR	"fn_fxddintegral"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of displaced diffusion type of fx sde integral

*/
class AQLPriceFXDDIntegral : public AQLRatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLPriceFXDDIntegral();
	//	Copy constructor
	AQLPriceFXDDIntegral(const AQLPriceFXDDIntegral& v);
	// Destructor
	virtual ~AQLPriceFXDDIntegral();

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
								
									
																
								
								
								
	
private:
	mutable DoubleArray			mVar;// tempolary variable for calculation
protected:
};
