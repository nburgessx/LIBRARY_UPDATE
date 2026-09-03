/*! @file
    @brief Source code of abstruct base class of interpolation of path element


*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesPEInterpolationBase.cpp
//
//  SYNOPSIS    :       LARatesPEInterpolationBase
//  DESCRIPTION :       Source code of abstruct base class of interpolation of path element
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


#include "LARatesPEInterpolationBase.h"


using namespace std;
//================ LARatesPEInterpolationBase ===================================
/*!
	@brief default constructor

*/
LARatesPEInterpolationBase::LARatesPEInterpolationBase()
{

}
/*!
	@brief copy constructor
*/
LARatesPEInterpolationBase::LARatesPEInterpolationBase(const LARatesPEInterpolationBase& v) 
: LACoreFunctionBase(v)
{

}

/*!
	@brief destructor
*/
LARatesPEInterpolationBase::~LARatesPEInterpolationBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPEInterpolationBase::isTypeOf(function_t id) const
{
	return (id==FN_PEINTERPOLATIONBASE ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesPEInterpolationBase::getType() const
{
	return FN_PEINTERPOLATIONBASE;
}


