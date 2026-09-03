/*! @file
    @brief Implementation of virtual base classes to get data.

*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#ifdef __MDEBUG__
#define __MDEBUG_DUMP__
#endif

#include "AQLDataBase.h"
/*!
    @brief constructor

	@param[in] name name of the object
*/
MDBase::MDBase(const AQLString& name)
:mName(name)
{
}
/*!
    @brief destructor
*/
MDBase::~MDBase()
{
}

