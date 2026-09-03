/*! @file
    @brief Source code of base class of sde integral



*/
//  2007, Mizuho International London..

////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARatesSDEIntegralBase.cpp
//
//  SYNOPSIS    :       LARatesSDEIntegralBase
//  DESCRIPTION :       Source code of base class of sde integral
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


#include "LARatesSDEIntegralBase.h"


using namespace std;
//================ LARatesSDEIntegralBase ===================================
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/
LARatesSDEIntegralBase::LARatesSDEIntegralBase(SDEINTEGRAL_TYPE type)
: mSdeType(dX), mIntegralType(type), mSDEAttrName()
{

}
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/
LARatesSDEIntegralBase::LARatesSDEIntegralBase(SDEINTEGRAL_TYPE type, const LAString& sdeAttrName)
: mSdeType(dX), mIntegralType(type), mSDEAttrName(sdeAttrName)
{

}
/*!
	@brief copy constructor
*/
LARatesSDEIntegralBase::LARatesSDEIntegralBase(const LARatesSDEIntegralBase& v) 
: LACoreFunctionBase(v), mIntegralType(v.mIntegralType), mSdeType(v.mSdeType), mSDEAttrName(v.mSDEAttrName)
{

}

/*!
	@brief destructor
*/
LARatesSDEIntegralBase::~LARatesSDEIntegralBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesSDEIntegralBase::isTypeOf(function_t id) const
{
	return (id==FN_SDEINTEGRALBASE ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
LARatesSDEIntegralBase::getType() const
{
	return FN_SDEINTEGRALBASE;
}


