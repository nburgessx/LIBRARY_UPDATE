/*! @file
    @brief source code of abstract base class of coupon select operator.

*/
//  2006, AlgoQuantHub..
//////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLPriceCouponBase.h"


using namespace std;

/*!
    @brief constructor
*/
AQLPriceCouponBase::AQLPriceCouponBase()
: AQLFunctionBase()
{

}
/*!
    @brief destructor

*/
AQLPriceCouponBase::~AQLPriceCouponBase()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceCouponBase::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORBASE ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
AQLPriceCouponBase::getType() const
{
	return FN_CPNSLTOPERATORBASE;
}



