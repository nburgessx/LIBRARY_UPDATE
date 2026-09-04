/*! @file
    @brief Source code of base class of sde integral
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRatesSDEIntegralBase.h"


using namespace std;
//================ AQLRatesSDEIntegralBase ===================================
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/
AQLRatesSDEIntegralBase::AQLRatesSDEIntegralBase(SDEINTEGRAL_TYPE type)
: mSdeType(dX), mIntegralType(type), mSDEAttrName()
{

}
/*!
	@brief default constructor
	
	@param[in] type sde integral type
*/
AQLRatesSDEIntegralBase::AQLRatesSDEIntegralBase(SDEINTEGRAL_TYPE type, const AQLString& sdeAttrName)
: mSdeType(dX), mIntegralType(type), mSDEAttrName(sdeAttrName)
{

}
/*!
	@brief copy constructor
*/
AQLRatesSDEIntegralBase::AQLRatesSDEIntegralBase(const AQLRatesSDEIntegralBase& v) 
: AQLCoreFunctionBase(v), mIntegralType(v.mIntegralType), mSdeType(v.mSdeType), mSDEAttrName(v.mSDEAttrName)
{

}

/*!
	@brief destructor
*/
AQLRatesSDEIntegralBase::~AQLRatesSDEIntegralBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesSDEIntegralBase::isTypeOf(function_t id) const
{
	return (id==FN_SDEINTEGRALBASE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRatesSDEIntegralBase::getType() const
{
	return FN_SDEINTEGRALBASE;
}


