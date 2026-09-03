#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LAPriceDriftHW.h"
#include "LAPriceDriftHWQuantAdjustment.h"


// ID for LAPriceDriftHWQuantAdjustment3F
#define FN_DRIFTHWQUANTADJ3F	10136
// Function name for LAPriceDriftHWQuantAdjustment3F
#define FN_DRIFTHWQUANTADJ3F_STR	"fn_drift_hw_quantoadj3f"


class LAMathPathEntity;
class LARatesNumeraireBase;
class LARatesSpotSDE;
class LAMathVolFuncFXDD;
class LAPriceQuantAdjustmentHWFXDD;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of quanto adjustment

*/
class LAPriceDriftHWQuantAdjustment3F : public LAPriceDriftHWQuantAdjustment
{
public:
//  LIFECYCLE
	// Default constructor
	LAPriceDriftHWQuantAdjustment3F(const LARatesNumeraireBase& domestic_nu, const LAMathVolFuncFXDD& fxvol, LARatesSpotSDE& fx_sde, 
									LAPriceDriftHW* pDriftIR, const DoubleArray& cor,
									const double fx_criteria = 0.01);
	// constructor
	LAPriceDriftHWQuantAdjustment3F(const AQLString& sdeAttrNameIR_D, const AQLString& sdeAttrNameIR_F, const AQLString& sdeAttrNameFX, LAPriceDriftHW* driftIR, const double fx_criteria = 0.01);	
	//	Copy constructor
	LAPriceDriftHWQuantAdjustment3F(const LAPriceDriftHWQuantAdjustment3F& v);
	// Destructor
	virtual ~LAPriceDriftHWQuantAdjustment3F();

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
								//======================================
								// set up this class
	 virtual void				setUp(LAMathPathEntity& path);
								//======================================
								// get quanto adjuster
	 const LAPriceQuantAdjustmentHWFXDD  &getQuantAdjuster(void) const;
protected:
	LAPriceQuantAdjustmentHWFXDD*  mpQuantAdjuster;
};
