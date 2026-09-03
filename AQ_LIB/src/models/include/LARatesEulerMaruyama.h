#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// ID for LARatesEulerMaruyama
#define FN_EULERMARUYMA	1802
// Function name for LARatesEulerMaruyama
#define FN_EULERMARUYMA_STR	"fn_euler-maruyama"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of Euler-Maruyama sde integral

*/
class LARatesEulerMaruyama : public LARatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesEulerMaruyama(SDEINTEGRAL_TYPE type);
	//	Copy constructor
	LARatesEulerMaruyama(const LARatesEulerMaruyama& v);
	// Destructor
	virtual ~LARatesEulerMaruyama();

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
