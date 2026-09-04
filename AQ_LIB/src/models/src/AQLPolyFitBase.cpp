/*! @file
    @brief Source code of base class of polynomial fitting class



*/
//  2007, AlgoQuantHub..

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLPolyFitBase.h"



//================ AQLPolyFitBase ===================================
/*!
	@brief default constructor
*/
AQLPolyFitBase::AQLPolyFitBase()
: AQLCoreFunctionBase()
{

}
/*!
	@brief copy constructor
*/
/*AQLPolyFitBase::AQLPolyFitBase(const AQLPolyFitBase& v) 
: AQLCoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
AQLPolyFitBase::~AQLPolyFitBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLPolyFitBase::isTypeOf(function_t id) const
{
	return (id == FN_POLYFITBASE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLPolyFitBase::getType() const
{
	return FN_POLYFITBASE;
}


