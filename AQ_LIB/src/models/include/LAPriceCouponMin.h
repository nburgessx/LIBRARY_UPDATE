#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceCouponBase.h"
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

// LAPriceCouponMin's function id
#define FN_CPNSLTOPERATORMIN	10005
// LAPriceCouponMin's function name
#define FN_CPNSLTOPERATORMIN_STR	"fn_cpnsltoperatormin"



class LAMathFXEntity;
class LAPriceDataManager;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of class of rainbow type coupon select operator.

*/
class LAPriceCouponMin : public LAPriceCouponBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceCouponMin();
    // destructor	
	virtual ~LAPriceCouponMin();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
	virtual double				operator()(const DoubleArray& x) const ;
    virtual function_t          getType() const;
	// select coupon
	virtual	double				selectCoupon(const DoubleArray& x, unsigned int& pos) const;

	virtual void				setUp(const LADate& basedate, const LAObject& trade,
										unsigned int legNo, const LAObject& cashlet){};


};
