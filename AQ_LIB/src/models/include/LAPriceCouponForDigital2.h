/*! @file
    @brief declaration of class of rainbow Temporary type coupon select operator.

*/
//   2007,AlgoQuantHub.

//////////////////////////////////////////////////////////
//いくかのパターンでテスト実施ずみ  
//////////////////////////////////////////////////////////

#ifndef LAPriceCouponForDigital2_h
#define LAPriceCouponForDigital2_h

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

// LAPriceCouponForDigital2's function id
#define FN_CPNSLTOPERATORFORDIGITAL2		10016
// LAPriceCouponForDigital2's function name
#define FN_CPNSLTOPERATORFORDIGITAL2_STR	"fn_cpnsltoperatorfordigital2"



class LAMathFXEntity;
class LAPriceDataManager;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of class of rainbow type coupon select operator.

*/
class LAPriceCouponForDigital2 : public LAPriceCouponBase
{
public:
//  LIFECYCLE
    // constructor	
	LAPriceCouponForDigital2();
    // destructor	
	virtual ~LAPriceCouponForDigital2();

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
								unsigned int legNo, const LAObject& cashlet);

	void						setCallSpread(bool isCallSpread){mIsCallSpread = isCallSpread;};
	void						setCallSpreadValue(double CallSpreadVal){mCallSpreadVal = CallSpreadVal;};

private:
	bool mIsCallSpread;    //! Is CallSpread
	double mCallSpreadVal; //! Call Spread Value

};
#endif 