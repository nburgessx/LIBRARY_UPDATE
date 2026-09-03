#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


// LAPriceCouponBase's function id
#define FN_CPNSLTOPERATORBASE		2004
// LAPriceCouponBase's function name
#define FN_CPNSLTOPERATORBASE_STR	"fn_cpnsltoperatorbase"



class LADate;
class LAObject;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of abstract base class of coupon select operator.

*/
class LAPriceCouponBase : public LAFunctionBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceCouponBase();
    // destructor	
	virtual ~LAPriceCouponBase();

								//======================================
								// Check function for this class ID
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const = 0;// %%% COVARIANT RETURN %%%
								//======================================
								// Return this class type
    virtual function_t          getType() const;

	/*!
		@brief select coupon
		@param[in] x coupons
		@note pure virtual method
	*/
	virtual double				operator()(const DoubleArray& x) const = 0;
	/*!
		@brief select coupon
		@param[in] x coupons
		@param[out] pos selected position of coupons
		@note pure virtual method
	*/
	virtual double				selectCoupon(const DoubleArray& x, unsigned int& pos) const = 0;
	/*!
		@brief set up this class
		@param[in] basedate basedate
		@param[in] trade trade object
		@param[in] legNo leg number(first leg number = 0)
		@param[in] cashlet cashlet object
	*/
	virtual void				setUp(const LADate& basedate, const LAObject& trade,
									unsigned int legNo, const LAObject& cashlet) = 0;

protected:
	

};

