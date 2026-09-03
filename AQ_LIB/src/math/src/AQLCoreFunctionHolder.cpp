/*! @file
    @brief Implementation to hold virtual base class for valuation, intepolation, procedure etc.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLCoreFunctionHolder.h"
#include "AQLCoreFunctionBase.h"
#include <cstdio>


/*!
    @brief default constructor

    @param[in] e Function object to be set
    @param[in] deleteFlag delete flat to be set
*/
AQLCoreFunctionHolder::AQLCoreFunctionHolder(const AQLCoreFunctionBase* e, bool deleteFlag)
    : mpFunction(e),mDeleteFlag(deleteFlag)
{

}

/*!
    @brief copy constructor

    @param[in] objectHolder original object
*/
AQLCoreFunctionHolder::AQLCoreFunctionHolder(const AQLCoreFunctionHolder & objectHolder)
	: mpFunction(NULL), mDeleteFlag(false)
{
	copy(objectHolder);
}

/*!
    @brief destructor
*/
AQLCoreFunctionHolder::~AQLCoreFunctionHolder(void)
{
	if (mDeleteFlag)
	{
		delete mpFunction;
	}
}

/*!
    @brief set Function object(copy the pointer to Function object)

    @param[in] base Function object to be set
    @param[in] df delete flag
*/
void                
AQLCoreFunctionHolder::set(const AQLCoreFunctionBase* base, bool df)
{
	if (base != mpFunction)
	{ // Copy
		if (mDeleteFlag)
			delete mpFunction;
		mpFunction = base;
	}
	mDeleteFlag = df;
}

/*!
    @brief shallow copy of the specified Function Holder object

    @param[in] e original Function Holder

    @return copied Function Holder
*/
AQLCoreFunctionHolder&   
AQLCoreFunctionHolder::copy(const AQLCoreFunctionHolder& e)
{
	if (this == &e) return *this;

	set(e.mpFunction, false);
	return *this;
}
