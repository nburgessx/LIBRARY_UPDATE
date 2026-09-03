#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceCouponBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// LAPriceCouponRainbow's function id
#define FN_CPNSLTOPERATORRAINBOW	2010
// LAPriceCouponRainbow's function name
#define FN_CPNSLTOPERATORRAINBOW_STR	"fn_cpnsltoperatorrainbow"


#ifndef PRICING_DATA_COUPONINFOS
#define PRICING_DATA_COUPONINFOS			"CouponInfos"			//  data name of reference to coupon information entities
#endif
#ifndef PRICING_DATA_FXRATE
#define PRICING_DATA_FXRATE					"FXRate"				//  data name of fx rate for notional 
#endif
#ifndef PRICING_DATA_CURRENCY
#define PRICING_DATA_CURRENCY				"Currency"				//  data name of notional currency
#endif
#ifndef PRICING_DATA_PAYMENTDATE
#define PRICING_DATA_PAYMENTDATE			"PaymentDate"			//  data name of paymentdate
#endif
#ifndef PRICING_DATA_PATHENTITY
#define PRICING_DATA_PATHENTITY				"PathEntity"			//  data name of path object
#endif



class LAMathFXEntity;
class LAPriceDataManager;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of class of rainbow type coupon select operator.

*/
class LAPriceCouponRainbow : public LAPriceCouponBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceCouponRainbow();
    // destructor	
	virtual ~LAPriceCouponRainbow();

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
	virtual double				operator()(const DoubleArray& x) const;
	// select coupon
	virtual double				selectCoupon(const DoubleArray& x, unsigned int& pos) const;
	// set up this class
	virtual void				setUp(const LADate& basedate, const LAObject& trade,
									unsigned int legNo, const LAObject& cashlet);
	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;

protected:
	double	mPaymentTime;// payment time
	const LAMathFXEntity*					mpFX_for_Notional;// fx rate for notional
	std::vector<const LAMathFXEntity*>	mpFX_for_Coupons;// fx rate for coupon
	LAString	mNotionalCur;// notional currency
	LAStringVector mCouponsCur;// coupon currency

};
