/*! @file
    @brief Source code of base class of quanto adjustment function class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPriceQuantAdjustmentFuncBase.h"


using namespace std;

//================ AQLPriceQuantAdjustmentFuncBase ===================================
/*!
	@brief default constructor
*/
AQLPriceQuantAdjustmentFuncBase::AQLPriceQuantAdjustmentFuncBase()
: AQLFunctionBase()
{

}
/*!
	@brief copy constructor
*/
/*AQLPriceQuantAdjustmentFuncBase::AQLPriceQuantAdjustmentFuncBase(const AQLPriceQuantAdjustmentFuncBase& v) 
: AQLCoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
AQLPriceQuantAdjustmentFuncBase::~AQLPriceQuantAdjustmentFuncBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPriceQuantAdjustmentFuncBase::isTypeOf(function_t id) const
{
	return (id==FN_QUANTADJUSTMENTFUNCBASE ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPriceQuantAdjustmentFuncBase::getType() const
{
	return FN_QUANTADJUSTMENTFUNCBASE;
}
