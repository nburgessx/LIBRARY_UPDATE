/*! @file
    @brief Implementation of virtual base classes to get data.
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLDataBase.h"
/*!
    @brief constructor

	@param[in] name name of the object
*/
AQLBase::AQLBase(const AQLString& name)
:mName(name)
{
}
/*!
    @brief destructor
*/
AQLBase::~AQLBase()
{
}

