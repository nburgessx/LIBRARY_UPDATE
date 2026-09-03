/*! @file
    @brief DataInstance setup class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLDataInstanceConfiguration.cpp
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


#include "AQLDataInstanceConfiguration.h"
#include "AQLDataInstance.h"
// constructor
/*!

*/
AQLDataInstanceConfiguration::AQLDataInstanceConfiguration()
{
}

// destructor
/*!

*/
AQLDataInstanceConfiguration::~AQLDataInstanceConfiguration(void)
{
}


// 
/*!
    @brief setup dataInstance

	@param[out] dataInstance
*/
void
AQLDataInstanceConfiguration::setUpDataInstance(AQLDataInstance &dataInstance) const
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
