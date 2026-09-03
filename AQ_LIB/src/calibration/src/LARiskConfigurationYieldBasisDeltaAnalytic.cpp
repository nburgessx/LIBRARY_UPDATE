/*! @file
    @brief  IR Delta setup class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldBasisDeltaAnalytic.cpp
//
//  DESCRIPTION :       IR Delta setup class
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


#include <algorithm>
#include "LARiskConfigurationYieldBasisDeltaAnalytic.h"
#include "AQLDataInstance.h"
#include "AQLBasic.h"
#include "AQLObjectPool.h"
#include "AQLDataReference.h"
#include "AQLDataProcedure.h"
#include "LADefinitionsRisk.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAFileAccessor.h"
#include "LAMarketData.h"
#include "AQLLinearFunc.h"
#include "LAStaticData.h"
#include "LADealUtils.h"

using namespace std;

// constructor
/*!

*/
LARiskConfigurationYieldBasisDeltaAnalytic::LARiskConfigurationYieldBasisDeltaAnalytic(void)
:LARiskConfigurationYieldBasisDelta()
{
}

// destructor
/*!

*/
LARiskConfigurationYieldBasisDeltaAnalytic::~LARiskConfigurationYieldBasisDeltaAnalytic(void)
{
}

