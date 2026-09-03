/*! @file
    @brief Definition of abstract base class for optimization(minimizatioin).

	   

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAOptimumBase.cpp
//
//  SYNOPSIS    :       LAOptimumBase
//  DESCRIPTION :       Definition of abstract base class for optimization(minimization).
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


#include "LAOptimumBase.h"
//================ LAOptimumOption ===================================
/*!
	@brief Default constructor
*/
LAOptimumOption::LAOptimumOption() 
:mPrecision(OPTIMUM_PRECISION),mIterMax(OPTIMUM_ITMAX),
mGTol(GTOL), mStepMax(STPMX), mAlf(ALF), mZeps(ZEPS), mWidth(WIDTH), mException(true)
,mBoundary(false),mBoundHigh(),mBoundLow()
{
}

/*!
	@brief Destructor
*/
LAOptimumOption::~LAOptimumOption() 
{
}

//================ LAOptimumBase ===================================
/*!
	@brief Default constructor
*/
LAOptimumBase::LAOptimumBase() 
{
}

/*!
	@brief Destructor
*/
LAOptimumBase::~LAOptimumBase() 
{
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LAOptimumBase::isTypeOf(function_t id) const
{
    return (id == FN_OPTIMUMBASE ? true : LACoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t
LAOptimumBase::getType() const
{
    return FN_OPTIMUMBASE;  
}

