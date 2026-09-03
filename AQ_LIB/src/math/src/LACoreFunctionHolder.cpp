/*! @file
    @brief Implementation to hold virtual base class for valuation, intepolation, procedure etc.

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LACoreFunctionHolder.h"
#include "LACoreFunctionBase.h"
#include <cstdio>


/*!
    @brief default constructor

    @param[in] e Function object to be set
    @param[in] deleteFlag delete flat to be set
*/
LACoreFunctionHolder::LACoreFunctionHolder(const LACoreFunctionBase* e, bool deleteFlag)
    : mpFunction(e),mDeleteFlag(deleteFlag)
{

}

/*!
    @brief copy constructor

    @param[in] objectHolder original object
*/
LACoreFunctionHolder::LACoreFunctionHolder(const LACoreFunctionHolder & objectHolder)
	: mpFunction(NULL), mDeleteFlag(false)
{
	copy(objectHolder);
}

/*!
    @brief destructor
*/
LACoreFunctionHolder::~LACoreFunctionHolder(void)
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
LACoreFunctionHolder::set(const LACoreFunctionBase* base, bool df)
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
LACoreFunctionHolder&   
LACoreFunctionHolder::copy(const LACoreFunctionHolder& e)
{
	if (this == &e) return *this;

	set(e.mpFunction, false);
	return *this;
}
