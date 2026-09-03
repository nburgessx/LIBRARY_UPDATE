/*! @file
    @brief Source code of base class of drift function class



*/
//  2007, AlgoQuantHub..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMathDriftFuncBase.cpp
//
//  SYNOPSIS    :       LAMathDriftFuncBase
//  DESCRIPTION :       Source of base class of drift function class
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


#include "LAMathDriftFuncBase.h"


using namespace std;

//================ LAMathDriftFuncBase ===================================
/*!
	@brief default constructor
*/
LAMathDriftFuncBase::LAMathDriftFuncBase()
: LAFunctionBase()
{

}
/*!
	@brief copy constructor
*/
/*LAMathDriftFuncBase::LAMathDriftFuncBase(const LAMathDriftFuncBase& v) 
: LACoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
LAMathDriftFuncBase::~LAMathDriftFuncBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAMathDriftFuncBase::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTFUNCBASE ? true : LAFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAMathDriftFuncBase::getType() const
{
	return FN_DRIFTFUNCBASE;
}
