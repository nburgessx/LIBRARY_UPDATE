#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathPathEntity.h"



// ID for AQLPriceSZDDIntegralMelstein
#define FN_SZDDINTEGRAL2	1807
// Function name for AQLPriceSZDDIntegralMelstein
#define FN_SZDDINTEGRAL2_STR	"fn_szddintegralmelstein"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of displaced diffusion type of fx sde integral

*/
class AQLPriceSZDDIntegralMelstein : public AQLRatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	//AQLPriceSZDDIntegralMelstein();
	//furuya
	// Default constructor
	AQLPriceSZDDIntegralMelstein(const AQLString &sdeAttrNameVol, const AQLString &sdeAttrNameCor);

	//	Copy constructor
	AQLPriceSZDDIntegralMelstein(const AQLPriceSZDDIntegralMelstein& v);
	// Destructor
	virtual ~AQLPriceSZDDIntegralMelstein();

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
																
	////							// return VolSDE function
	////AQLRatesSDEBase*				getVolSDE() {return mpVolSDE;}

								// set up this class
	virtual void			    setUp(AQLMathPathEntity& path);
	
private:
	mutable DoubleArray			mVar;// tempolary variable for calculation
	//furuya
	AQLRatesSDEBase* mpVolSDE;
	mutable AQLString mSDEAttrNameVol;
	mutable unsigned int mPos_old;
	double	mCorr; 

protected:


};

