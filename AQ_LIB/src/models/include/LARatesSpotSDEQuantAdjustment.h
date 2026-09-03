#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LARatesSDEBase.h"
#include "LARatesSpotSDE.h"
#include "LAModelDynamicsBase.h"


// ID for LARatesSpotSDEQuantAdjustment
#define FN_SPOTSDEQUANTADJUSTMENT	1803 
// Function name for LARatesSpotSDEQuantAdjustment
#define FN_SPOTSDEQUANTADJUSTMENT_STR	"fn_spot_sde_quantadjustment"


class LAPriceQuantAdjustmentFuncBase;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of spot sde class

*/
class LARatesSpotSDEQuantAdjustment : public LARatesSpotSDE
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LARatesSpotSDEQuantAdjustment(SDE_TYPE type, LAPriceQuantAdjustmentFuncBase* pQuantAduster = 0, AQLFunctionBase* pTransformer = 0, AQLFunctionBase* pInvTransformer = 0,
										AQLFunctionBase* pAdjuster = 0);
	//	Copy constructor
	LARatesSpotSDEQuantAdjustment(const LARatesSpotSDEQuantAdjustment& v);
	// Destructor
	virtual ~LARatesSpotSDEQuantAdjustment();
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
	void						setQuantAdjuster(LAPriceQuantAdjustmentFuncBase* p){mpQuantAduster = p;}
								//==========================================
								// get quanto adjuster
	LAPriceQuantAdjustmentFuncBase	 &getQuantAdjuster(void);

private:

protected:
								//==========================================
	                            // calculate path
	virtual void				calcPath(unsigned int pos);
								//==========================================
	                            // check whether preparation of calcuation is finished or not
	virtual bool				check(void) const;

	LAPriceQuantAdjustmentFuncBase *mpQuantAduster;

};

