/*! @file
    @brief source code of abstract base class of coupon select operator.

*/
//  2007,AlgoQuantHub.
//////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////
#ifdef __GNUG__
#pragma implementation
#endif
#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "AQLRatesAccruedCouponFuncBase.h"


using namespace std;

/*!
    @brief constructor
*/
AQLRatesAccruedCouponFuncBase::AQLRatesAccruedCouponFuncBase()
: AQLFunctionBase()
{

}
/*!
    @brief destructor

*/
AQLRatesAccruedCouponFuncBase::~AQLRatesAccruedCouponFuncBase()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesAccruedCouponFuncBase::isTypeOf(function_t id) const
{
	return (id == FN_ACCRUEDCOUPONFUNCBASE ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
AQLRatesAccruedCouponFuncBase::getType() const
{
	return FN_ACCRUEDCOUPONFUNCBASE;
}



