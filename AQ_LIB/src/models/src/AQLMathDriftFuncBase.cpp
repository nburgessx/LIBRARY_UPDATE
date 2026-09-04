/*! @file
    @brief Source code of base class of drift function class
*/

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLMathDriftFuncBase.h"


using namespace std;

//================ AQLMathDriftFuncBase ===================================
/*!
	@brief default constructor
*/
AQLMathDriftFuncBase::AQLMathDriftFuncBase()
: AQLFunctionBase()
{

}
/*!
	@brief copy constructor
*/
/*AQLMathDriftFuncBase::AQLMathDriftFuncBase(const AQLMathDriftFuncBase& v) 
: AQLCoreFunctionBase(v)
{

}*/

/*!
	@brief destructor
*/
AQLMathDriftFuncBase::~AQLMathDriftFuncBase() 
{

}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLMathDriftFuncBase::isTypeOf(function_t id) const
{
	return (id==FN_DRIFTFUNCBASE ? true : AQLFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLMathDriftFuncBase::getType() const
{
	return FN_DRIFTFUNCBASE;
}
