/*! @file
    @brief DataInstance setup class
*/
//  2007, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LADataInstanceConfiguration.cpp
//
//  DESCRIPTION :        DataInstance setupper
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LADataInstanceConfiguration.h"
#include "LADataInstance.h"
// constructor
/*!

*/
LADataInstanceConfiguration::LADataInstanceConfiguration()
{
}

// destructor
/*!

*/
LADataInstanceConfiguration::~LADataInstanceConfiguration(void)
{
}


// 
/*!
    @brief setup dataInstance

	@param[out] dataInstance
*/
void
LADataInstanceConfiguration::setUpDataInstance(LADataInstance &dataInstance) const
{
	// setup master data
	setUpMasters(dataInstance);
	// load entities
	loadEntities(dataInstance);
	// setup sde
	setUpSDE(dataInstance);
	// setup market data object
	//setUpMarketData(dataInstance);
	// setup entities
	setUpEntityes(dataInstance);
}
