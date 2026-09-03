/*! @file
    @brief declaration of class of rainbow Max type coupon select operator.

*/
//   2007,Mizuho International London.

//////////////////////////////////////////////////////////
//いくかのパターンでテスト実施ずみ  
//////////////////////////////////////////////////////////

#ifndef LAPriceCouponRainbowMax_h
#define LAPriceCouponRainbowMax_h

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

// LAPriceCouponRainbowMax's function id
#define FN_CPNSLTOPERATORMAX	10004
// LAPriceCouponRainbowMax's function name
#define FN_CPNSLTOPERATORMAX_STR	"fn_cpnsltoperatormax"



class LAMathFXEntity;
class LAPriceDataManager;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of class of rainbow type coupon select operator.

*/
class LAPriceCouponMax : public LAPriceCouponBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceCouponMax();
    // destructor	
	virtual ~LAPriceCouponMax();

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
#endif 