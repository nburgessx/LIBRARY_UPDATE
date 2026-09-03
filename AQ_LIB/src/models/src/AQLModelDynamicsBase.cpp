/*! @file
    @brief Source code of abstract base class of path element


*/
//  2007, AlgoQuantHub..


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLModelDynamicsBase.h"



//================ AQLRatesPathElementBase ===================================
/*!
	@brief default constructor
*/
AQLRatesPathElementBase::AQLRatesPathElementBase() 
{
	;
}


/*!
	@brief destructor
*/
AQLRatesPathElementBase::~AQLRatesPathElementBase() 
{
	;
}

/*!
    @brief Check path element for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRatesPathElementBase::isTypeOf(pathelement_t id) const
{
	return id == PE_BASE;
}

/*!
    @brief Return this path element type
    @return path element type
*/
pathelement_t
AQLRatesPathElementBase::getType() const
{
	return PE_BASE;
}

								// set value
/*!
    @brief set value
	@param[in] a value to set
*/
void
AQLRatesPathElementBase::set(SCALAR a)
{
	throw AQLCoreInvalidData("this method does not support", __FILE__, __LINE__);
}

/*!
    @brief set value
	@param[in] a value to set
*/
void
AQLRatesPathElementBase::set(const SCALARARRAY& a)
{
	throw AQLCoreInvalidData("this method does not support", __FILE__, __LINE__);
}
