/*! @file
    @brief declaration of class of rainbow Temporary type coupon select operator.

*/
//   2007,AlgoQuantHub.

//////////////////////////////////////////////////////////
//いくかのパターンでテスト実施ずみ  
//////////////////////////////////////////////////////////

#ifndef LAPriceCouponForDigital_h
#define LAPriceCouponForDigital_h

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

// LAPriceCouponForDigital's function id
#define FN_CPNSLTOPERATORFORDIGITAL		10008
// LAPriceCouponForDigital's function name
#define FN_CPNSLTOPERATORFORDIGITAL_STR	"fn_cpnsltoperatorfordigital"



class LAMathFXEntity;
class LAPriceDataManager;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of class of rainbow type coupon select operator.

*/
class LAPriceCouponForDigital : public LAPriceCouponBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceCouponForDigital();
    // destructor	
	virtual ~LAPriceCouponForDigital();

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