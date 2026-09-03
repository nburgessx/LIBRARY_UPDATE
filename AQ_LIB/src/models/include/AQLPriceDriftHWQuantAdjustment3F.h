#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLMathDriftFuncBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLPriceDriftHW.h"
#include "AQLPriceDriftHWQuantAdjustment.h"


// ID for AQLPriceDriftHWQuantAdjustment3F
#define FN_DRIFTHWQUANTADJ3F	10136
// Function name for AQLPriceDriftHWQuantAdjustment3F
#define FN_DRIFTHWQUANTADJ3F_STR	"fn_drift_hw_quantoadj3f"


class AQLMathPathEntity;
class AQLRatesNumeraireBase;
class AQLRatesSpotSDE;
class AQLMathVolFuncFXDD;
class AQLPriceQuantAdjustmentHWFXDD;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of drift function of quanto adjustment

*/
class AQLPriceDriftHWQuantAdjustment3F : public AQLPriceDriftHWQuantAdjustment
{
public:
//  LIFECYCLE
	// Default constructor
	AQLPriceDriftHWQuantAdjustment3F(const AQLRatesNumeraireBase& domestic_nu, const AQLMathVolFuncFXDD& fxvol, AQLRatesSpotSDE& fx_sde, 
									AQLPriceDriftHW* pDriftIR, const DoubleArray& cor,
									const double fx_criteria = 0.01);
	// constructor
	AQLPriceDriftHWQuantAdjustment3F(const AQLString& sdeAttrNameIR_D, const AQLString& sdeAttrNameIR_F, const AQLString& sdeAttrNameFX, AQLPriceDriftHW* driftIR, const double fx_criteria = 0.01);	
	//	Copy constructor
	AQLPriceDriftHWQuantAdjustment3F(const AQLPriceDriftHWQuantAdjustment3F& v);
	// Destructor
	virtual ~AQLPriceDriftHWQuantAdjustment3F();

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
	 virtual void				setUp(AQLMathPathEntity& path);
								//======================================
								// get quanto adjuster
	 const AQLPriceQuantAdjustmentHWFXDD  &getQuantAdjuster(void) const;
protected:
	AQLPriceQuantAdjustmentHWFXDD*  mpQuantAdjuster;
};
