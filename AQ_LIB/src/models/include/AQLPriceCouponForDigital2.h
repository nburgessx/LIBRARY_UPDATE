/*! @file
    @brief declaration of class of rainbow Temporary type coupon select operator.
*/

//////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////

#ifndef AQLPriceCouponForDigital2_h
#define AQLPriceCouponForDigital2_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLPriceCouponBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLObject.h"
#include "AQLObjectHolder.h"
#include "AQLDataBasics.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLPriceDataManager.h"
#include "AQLPriceDataDayCount.h"
#include "AQLMathPathEntity.h"
#include "AQLMathFXEntity.h"

// AQLPriceCouponForDigital2's function id
#define FN_CPNSLTOPERATORFORDIGITAL2		10016
// AQLPriceCouponForDigital2's function name
#define FN_CPNSLTOPERATORFORDIGITAL2_STR	"fn_cpnsltoperatorfordigital2"



class AQLMathFXEntity;
class AQLPriceDataManager;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief declaration of class of rainbow type coupon select operator.

*/
class AQLPriceCouponForDigital2 : public AQLPriceCouponBase
{
public:
//  LIFECYCLE
    // constructor	
	AQLPriceCouponForDigital2();
    // destructor	
	virtual ~AQLPriceCouponForDigital2();

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
								unsigned int legNo, const AQLObject& cashlet);

	void						setCallSpread(bool isCallSpread){mIsCallSpread = isCallSpread;};
	void						setCallSpreadValue(double CallSpreadVal){mCallSpreadVal = CallSpreadVal;};

private:
	bool mIsCallSpread;    //! Is CallSpread
	double mCallSpreadVal; //! Call Spread Value

};
#endif 