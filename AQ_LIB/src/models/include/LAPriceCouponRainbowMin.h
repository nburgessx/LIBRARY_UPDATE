#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceCouponRainbow.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAObject.h"
#include "LAObjectHolder.h"
#include "LADataBasics.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LAPriceDataManager.h"
#include "LAPriceDataDayCount.h"
#include "LAMathPathEntity.h"
#include "LAMathFXEntity.h"

// LAPriceCouponRainbowMin's function id
#define FN_CPNSLTOPERATORRAINBOWMIN	10000
// LAPriceCouponRainbowMin's function name
#define FN_CPNSLTOPERATORRAINBOWMIN_STR	"fn_cpnsltoperatorrainbowmin"



class LAMathFXEntity;
class LAPriceDataManager;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of class of rainbow type coupon select operator.

*/
class LAPriceCouponRainbowMin : public LAPriceCouponRainbow
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceCouponRainbowMin();
    // destructor	
	virtual ~LAPriceCouponRainbowMin();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;
	// select coupon
	virtual	double				selectCoupon(const DoubleArray& x, unsigned int& pos) const;

};
