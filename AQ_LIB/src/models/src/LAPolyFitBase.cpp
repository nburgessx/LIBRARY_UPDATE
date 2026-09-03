/*! @file
    @brief Source code of base class of polynomial fitting class



*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAPolyFitBase.cpp
//
//  SYNOPSIS    :       LAPolyFitBase
//  DESCRIPTION :       Declaration of base class of polynomial fitting class
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


#include "LAPolyFitBase.h"



//================ LAPolyFitBase ===================================
/*!
	@brief default constructor
*/
LAPolyFitBase::LAPolyFitBase()
: LACoreFunctionBase()
{

}
/*!
	@brief copy constructor
*/
/*LAPolyFitBase::LAPolyFitBase(const LAPolyFitBase& v) 
: LACoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
LAPolyFitBase::~LAPolyFitBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAPolyFitBase::isTypeOf(function_t id) const
{
	return (id == FN_POLYFITBASE ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LAPolyFitBase::getType() const
{
	return FN_POLYFITBASE;
}


