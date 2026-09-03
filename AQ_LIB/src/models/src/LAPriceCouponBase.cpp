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

#include "LAPriceCouponBase.h"


using namespace std;

/*!
    @brief constructor
*/
LAPriceCouponBase::LAPriceCouponBase()
: LAFunctionBase()
{

}
/*!
    @brief destructor

*/
LAPriceCouponBase::~LAPriceCouponBase()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceCouponBase::isTypeOf(function_t id) const
{
	return (id == FN_CPNSLTOPERATORBASE ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
LAPriceCouponBase::getType() const
{
	return FN_CPNSLTOPERATORBASE;
}



