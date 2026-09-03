#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEIntegralBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LAMathPathEntity.h"



// ID for LAPriceSZDDIntegralMelstein
#define FN_SZDDINTEGRAL2	1807
// Function name for LAPriceSZDDIntegralMelstein
#define FN_SZDDINTEGRAL2_STR	"fn_szddintegralmelstein"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of displaced diffusion type of fx sde integral

*/
class LAPriceSZDDIntegralMelstein : public LARatesSDEIntegralBase
{
public:
//  LIFECYCLE
	// Default constructor
	//LAPriceSZDDIntegralMelstein();
	//furuya
	// Default constructor
	LAPriceSZDDIntegralMelstein(const AQLString &sdeAttrNameVol, const AQLString &sdeAttrNameCor);

	//	Copy constructor
	LAPriceSZDDIntegralMelstein(const LAPriceSZDDIntegralMelstein& v);
	// Destructor
	virtual ~LAPriceSZDDIntegralMelstein();

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
	////LARatesSDEBase*				getVolSDE() {return mpVolSDE;}

								// set up this class
	virtual void			    setUp(LAMathPathEntity& path);
	
private:
	mutable DoubleArray			mVar;// tempolary variable for calculation
	//furuya
	LARatesSDEBase* mpVolSDE;
	mutable AQLString mSDEAttrNameVol;
	mutable unsigned int mPos_old;
	double	mCorr; 

protected:


};

