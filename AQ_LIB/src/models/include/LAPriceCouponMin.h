#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAPriceCouponBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLObject.h"
#include "AQLObjectHolder.h"
#include "AQLDataBasics.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataManager.h"
#include "AQLPriceDataDayCount.h"
#include "LAMathPathEntity.h"
#include "LAMathFXEntity.h"

// LAPriceCouponMin's function id
#define FN_CPNSLTOPERATORMIN	10005
// LAPriceCouponMin's function name
#define FN_CPNSLTOPERATORMIN_STR	"fn_cpnsltoperatormin"



class LAMathFXEntity;
class AQLPriceDataManager;
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
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
	virtual double				operator()(const DoubleArray& x) const ;
    virtual function_t          getType() const;
	// select coupon
	virtual	double				selectCoupon(const DoubleArray& x, unsigned int& pos) const;

	virtual void				setUp(const AQLDate& basedate, const AQLObject& trade,
										unsigned int legNo, const AQLObject& cashlet){};


};
