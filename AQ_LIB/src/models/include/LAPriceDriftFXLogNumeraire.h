#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathDriftFuncBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LARatesSDEBase.h"
#include "LAPriceDriftFX.h"


// ID for LAPriceDriftFXLogNumeraire
#define FN_DRIFTFXLOGNUMERARIE	2212
// Function name for LAPriceDriftFXLogNumeraire
#define FN_DRIFTFXLOGNUMERARIE_STR	"fn_drift_fx_lognumerarie"


class LAMathPathEntity;
class LARatesNumeraireBase;
class LAMathDriftFuncBase;
class LAPriceDriftHWQuantAdjustment3F;
class LAPriceQuantAdjustmentHWFXDD;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of fx sde

*/
class LAPriceDriftFXLogNumeraire : public LAPriceDriftFX
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceDriftFXLogNumeraire(const double s = 0.0, SDE_TYPE type = DIVIDEdXbyX);
	// constructor
	LAPriceDriftFXLogNumeraire(const LAString& sdeAttrNameD, const LAString& sdeAttrNameF, const double s = 0.0, SDE_TYPE type = DIVIDEdXbyX);
	//	Copy constructor
//	LAPriceDriftFXLogNumeraire(const LAPriceDriftFXLogNumeraire& v);
	// Destructor
	virtual ~LAPriceDriftFXLogNumeraire();

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
	                            //==========================================
	                            // Return drift value
	virtual double				operator()(const DoubleArray& x) const;

//	OPERATION
								//======================================
								// set up this class
	virtual void				setUp(LAMathPathEntity& path);
								//======================================
								// get quanto adjuster
	 const LAPriceQuantAdjustmentHWFXDD  &getQuantAdjuster(void) const;
protected:
	const LAPriceDriftHWQuantAdjustment3F* mpDriftForeign;     // foreign drift
	const LAPriceQuantAdjustmentHWFXDD*	mpQuantAdjuster;	// quanto adjuster
};

