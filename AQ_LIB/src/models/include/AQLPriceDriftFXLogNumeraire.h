#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLRatesSDEBase.h"
#include "AQLPriceDriftFX.h"


// ID for AQLPriceDriftFXLogNumeraire
#define FN_DRIFTFXLOGNUMERARIE	2212
// Function name for AQLPriceDriftFXLogNumeraire
#define FN_DRIFTFXLOGNUMERARIE_STR	"fn_drift_fx_lognumerarie"


class AQLMathPathEntity;
class AQLRatesNumeraireBase;
class AQLMathDriftFuncBase;
class AQLPriceDriftHWQuantAdjustment3F;
class AQLPriceQuantAdjustmentHWFXDD;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of fx sde

*/
class AQLPriceDriftFXLogNumeraire : public AQLPriceDriftFX
{
public:
//  LIFECYCLE
	// Default constructor
	AQLPriceDriftFXLogNumeraire(const double s = 0.0, SDE_TYPE type = DIVIDEdXbyX);
	// constructor
	AQLPriceDriftFXLogNumeraire(const AQLString& sdeAttrNameD, const AQLString& sdeAttrNameF, const double s = 0.0, SDE_TYPE type = DIVIDEdXbyX);
	//	Copy constructor
//	AQLPriceDriftFXLogNumeraire(const AQLPriceDriftFXLogNumeraire& v);
	// Destructor
	virtual ~AQLPriceDriftFXLogNumeraire();

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
	                            //==========================================
	                            // Return drift value
	virtual double				operator()(const DoubleArray& x) const;

//	OPERATION
								//======================================
								// set up this class
	virtual void				setUp(AQLMathPathEntity& path);
								//======================================
								// get quanto adjuster
	 const AQLPriceQuantAdjustmentHWFXDD  &getQuantAdjuster(void) const;
protected:
	const AQLPriceDriftHWQuantAdjustment3F* mpDriftForeign;     // foreign drift
	const AQLPriceQuantAdjustmentHWFXDD*	mpQuantAdjuster;	// quanto adjuster
};

