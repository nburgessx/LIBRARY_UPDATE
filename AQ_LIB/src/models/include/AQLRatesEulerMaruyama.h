#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// ID for AQLRatesEulerMaruyama
#define FN_EULERMARUYMA	1802
// Function name for AQLRatesEulerMaruyama
#define FN_EULERMARUYMA_STR	"fn_euler-maruyama"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of Euler-Maruyama sde integral

*/
class AQLRatesEulerMaruyama : public AQLRatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesEulerMaruyama(SDEINTEGRAL_TYPE type);
	//	Copy constructor
	AQLRatesEulerMaruyama(const AQLRatesEulerMaruyama& v);
	// Destructor
	virtual ~AQLRatesEulerMaruyama();

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
