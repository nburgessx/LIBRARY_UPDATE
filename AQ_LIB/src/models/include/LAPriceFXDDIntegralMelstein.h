#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// ID for LAPriceFXDDIntegralMelstein
#define FN_FXDDINTEGRAL2	1805
// Function name for LAPriceFXDDIntegralMelstein
#define FN_FXDDINTEGRAL2_STR	"fn_fxddintegralmelstein"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of displaced diffusion type of fx sde integral

*/
class LAPriceFXDDIntegralMelstein : public LARatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceFXDDIntegralMelstein();
	//	Copy constructor
	LAPriceFXDDIntegralMelstein(const LAPriceFXDDIntegralMelstein& v);
	// Destructor
	virtual ~LAPriceFXDDIntegralMelstein();

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
