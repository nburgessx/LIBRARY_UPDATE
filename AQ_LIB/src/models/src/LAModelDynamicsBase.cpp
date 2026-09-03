/*! @file
    @brief Source code of abstract base class of path element


*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAModelDynamicsBase.h"



//================ LARatesPathElementBase ===================================
/*!
	@brief default constructor
*/
LARatesPathElementBase::LARatesPathElementBase() 
{
	;
}


/*!
	@brief destructor
*/
LARatesPathElementBase::~LARatesPathElementBase() 
{
	;
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARatesPathElementBase::isTypeOf(pathelement_t id) const
{
	return id == PE_BASE;
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
LARatesPathElementBase::getType() const
{
	return PE_BASE;
}

								// set value
/*!
    @brief set value
	@param[in] a value to set
*/
void
LARatesPathElementBase::set(SCALAR a)
{
	throw AQLCoreInvalidData("this method does not support", __FILE__, __LINE__);
}

/*!
    @brief set value
	@param[in] a value to set
*/
void
LARatesPathElementBase::set(const SCALARARRAY& a)
{
	throw AQLCoreInvalidData("this method does not support", __FILE__, __LINE__);
}
