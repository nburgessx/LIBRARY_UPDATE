#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// ID for LAPriceFXDDIntegral
#define FN_FXDDINTEGRAL	10038
// Function name for LAPriceFXDDIntegral
#define FN_FXDDINTEGRAL_STR	"fn_fxddintegral"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of displaced diffusion type of fx sde integral

*/
class LAPriceFXDDIntegral : public LARatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceFXDDIntegral();
	//	Copy constructor
	LAPriceFXDDIntegral(const LAPriceFXDDIntegral& v);
	// Destructor
	virtual ~LAPriceFXDDIntegral();

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
