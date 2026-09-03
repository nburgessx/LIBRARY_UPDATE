#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// ID for AQLPriceFXDDIntegral3
#define FN_FXDDINTEGRAL3	1806
// Function name for AQLPriceFXDDIntegral3
#define FN_FXDDINTEGRAL3_STR	"fn_fxddintegral3"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of displaced diffusion type of fx sde integral

*/
class AQLPriceFXDDIntegral3 : public AQLRatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLPriceFXDDIntegral3();
	//	Copy constructor
	AQLPriceFXDDIntegral3(const AQLPriceFXDDIntegral3& v);
	// Destructor
	virtual ~AQLPriceFXDDIntegral3();

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
