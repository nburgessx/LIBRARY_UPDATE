/*! @file
    @brief Source code of base class of quanto adjustment function class



*/
//  2012, AlgoQuantHub.

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPriceQuantAdjustmentFuncBase.cpp
//
//  SYNOPSIS    :       LAPriceQuantAdjustmentFuncBase
//  DESCRIPTION :       Source of base class of quanto adjustment class
//
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAPriceQuantAdjustmentFuncBase.h"


using namespace std;

//================ LAPriceQuantAdjustmentFuncBase ===================================
/*!
	@brief default constructor
*/
LAPriceQuantAdjustmentFuncBase::LAPriceQuantAdjustmentFuncBase()
: LAFunctionBase()
{

}
/*!
	@brief copy constructor
*/
/*LAPriceQuantAdjustmentFuncBase::LAPriceQuantAdjustmentFuncBase(const LAPriceQuantAdjustmentFuncBase& v) 
: LACoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
LAPriceQuantAdjustmentFuncBase::~LAPriceQuantAdjustmentFuncBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPriceQuantAdjustmentFuncBase::isTypeOf(function_t id) const
{
	return (id==FN_QUANTADJUSTMENTFUNCBASE ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPriceQuantAdjustmentFuncBase::getType() const
{
	return FN_QUANTADJUSTMENTFUNCBASE;
}
