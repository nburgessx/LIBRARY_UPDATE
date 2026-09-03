#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEIntegralBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"



// ID for LAPriceFXDDIntegral3
#define FN_FXDDINTEGRAL3	1806
// Function name for LAPriceFXDDIntegral3
#define FN_FXDDINTEGRAL3_STR	"fn_fxddintegral3"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of displaced diffusion type of fx sde integral

*/
class LAPriceFXDDIntegral3 : public LARatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceFXDDIntegral3();
	//	Copy constructor
	LAPriceFXDDIntegral3(const LAPriceFXDDIntegral3& v);
	// Destructor
	virtual ~LAPriceFXDDIntegral3();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
								//======================================
								// excecute integral
	inline 	virtual void		integral(double ts, double te, 
										std::vector<LAFunctionBase*>::const_iterator drift,										
										std::vector<std::vector<LAFunctionBase*> >::const_iterator vol,
										DoubleArray::const_iterator	bm,
										SCALARARRAY::iterator	x_in_out,	
										unsigned int varnum
								) const;
								
									
																
								
								
								
	
private:
	mutable DoubleArray			mVar;// tempolary variable for calculation
protected:
};
