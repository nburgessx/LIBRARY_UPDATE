#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// ID for AQLPriceFXDDIntegralMelstein
#define FN_FXDDINTEGRAL2	1805
// Function name for AQLPriceFXDDIntegralMelstein
#define FN_FXDDINTEGRAL2_STR	"fn_fxddintegralmelstein"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of displaced diffusion type of fx sde integral

*/
class AQLPriceFXDDIntegralMelstein : public AQLRatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	AQLPriceFXDDIntegralMelstein();
	//	Copy constructor
	AQLPriceFXDDIntegralMelstein(const AQLPriceFXDDIntegralMelstein& v);
	// Destructor
	virtual ~AQLPriceFXDDIntegralMelstein();

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
