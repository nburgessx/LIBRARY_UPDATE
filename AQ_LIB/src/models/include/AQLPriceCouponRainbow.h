#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceCouponBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


// AQLPriceCouponRainbow's function id
#define FN_CPNSLTOPERATORRAINBOW	2010
// AQLPriceCouponRainbow's function name
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



class AQLMathFXEntity;
class AQLPriceDataManager;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of class of rainbow type coupon select operator.

*/
class AQLPriceCouponRainbow : public AQLPriceCouponBase
{
public:
//  LIFECYCLE
    // constructor	
	AQLPriceCouponRainbow();
    // destructor	
	virtual ~AQLPriceCouponRainbow();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;
	// select coupon
	virtual double				operator()(const DoubleArray& x) const;
	// select coupon
	virtual double				selectCoupon(const DoubleArray& x, unsigned int& pos) const;
	// set up this class
	virtual void				setUp(const AQLDate& basedate, const AQLObject& trade,
									unsigned int legNo, const AQLObject& cashlet);
	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;

protected:
	double	mPaymentTime;// payment time
	const AQLMathFXEntity*					mpFX_for_Notional;// fx rate for notional
	std::vector<const AQLMathFXEntity*>	mpFX_for_Coupons;// fx rate for coupon
	AQLString	mNotionalCur;// notional currency
	AQLStringVector mCouponsCur;// coupon currency

};
