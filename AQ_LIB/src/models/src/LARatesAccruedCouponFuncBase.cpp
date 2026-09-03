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

#include "LARatesAccruedCouponFuncBase.h"


using namespace std;

/*!
    @brief constructor
*/
LARatesAccruedCouponFuncBase::LARatesAccruedCouponFuncBase()
: AQLFunctionBase()
{

}
/*!
    @brief destructor

*/
LARatesAccruedCouponFuncBase::~LARatesAccruedCouponFuncBase()
{
}

/*
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesAccruedCouponFuncBase::isTypeOf(function_t id) const
{
	return (id == FN_ACCRUEDCOUPONFUNCBASE ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t			
LARatesAccruedCouponFuncBase::getType() const
{
	return FN_ACCRUEDCOUPONFUNCBASE;
}



