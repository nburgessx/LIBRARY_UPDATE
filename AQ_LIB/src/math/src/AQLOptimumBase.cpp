/*! @file
    @brief Definition of abstract base class for optimization(minimizatioin).
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLOptimumBase.h"
//================ AQLOptimumOption ===================================
/*!
	@brief Default constructor
*/
AQLOptimumOption::AQLOptimumOption() 
:mPrecision(OPTIMUM_PRECISION),mIterMax(OPTIMUM_ITMAX),
mGTol(GTOL), mStepMax(STPMX), mAlf(ALF), mZeps(ZEPS), mWidth(WIDTH), mException(true)
,mBoundary(false),mBoundHigh(),mBoundLow()
{
}

/*!
	@brief Destructor
*/
AQLOptimumOption::~AQLOptimumOption() 
{
}

//================ AQLOptimumBase ===================================
/*!
	@brief Default constructor
*/
AQLOptimumBase::AQLOptimumBase() 
{
}

/*!
	@brief Destructor
*/
AQLOptimumBase::~AQLOptimumBase() 
{
}

/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLOptimumBase::isTypeOf(function_t id) const
{
    return (id == FN_OPTIMUMBASE ? true : AQLCoreFunctionBase::isTypeOf(id));
}

/*!
    @brief Return this class type
    @return function type
*/
function_t
AQLOptimumBase::getType() const
{
    return FN_OPTIMUMBASE;  
}

