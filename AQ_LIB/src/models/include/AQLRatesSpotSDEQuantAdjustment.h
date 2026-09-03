#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRatesSDEBase.h"
#include "AQLRatesSpotSDE.h"
#include "AQLModelDynamicsBase.h"


// ID for AQLRatesSpotSDEQuantAdjustment
#define FN_SPOTSDEQUANTADJUSTMENT	1803 
// Function name for AQLRatesSpotSDEQuantAdjustment
#define FN_SPOTSDEQUANTADJUSTMENT_STR	"fn_spot_sde_quantadjustment"


class AQLPriceQuantAdjustmentFuncBase;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of spot sde class

*/
class AQLRatesSpotSDEQuantAdjustment : public AQLRatesSpotSDE
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLRatesSpotSDEQuantAdjustment(SDE_TYPE type, AQLPriceQuantAdjustmentFuncBase* pQuantAduster = 0, AQLFunctionBase* pTransformer = 0, AQLFunctionBase* pInvTransformer = 0,
										AQLFunctionBase* pAdjuster = 0);
	//	Copy constructor
	AQLRatesSpotSDEQuantAdjustment(const AQLRatesSpotSDEQuantAdjustment& v);
	// Destructor
	virtual ~AQLRatesSpotSDEQuantAdjustment();
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
								// set adjuster
	void						setQuantAdjuster(AQLPriceQuantAdjustmentFuncBase* p){mpQuantAduster = p;}
								//==========================================
								// get quanto adjuster
	AQLPriceQuantAdjustmentFuncBase	 &getQuantAdjuster(void);

private:

protected:
								//==========================================
	                            // calculate path
	virtual void				calcPath(unsigned int pos);
								//==========================================
	                            // check whether preparation of calcuation is finished or not
	virtual bool				check(void) const;

	AQLPriceQuantAdjustmentFuncBase *mpQuantAduster;

};

