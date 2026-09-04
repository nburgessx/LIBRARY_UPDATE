/*! @file
    @brief  IR Delta setup class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include <algorithm>
#include "AQLRiskConfigurationYieldBasisDeltaAnalytic.h"
#include "AQLDataInstance.h"
#include "AQLBasic.h"
#include "AQLObjectPool.h"
#include "AQLDataReference.h"
#include "AQLDataProcedure.h"
#include "AQLDefinitionsRisk.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLFileAccessor.h"
#include "AQLMarketData.h"
#include "AQLLinearFunc.h"
#include "AQLStaticData.h"
#include "AQLDealUtils.h"

using namespace std;

// constructor
/*!

*/
AQLRiskConfigurationYieldBasisDeltaAnalytic::AQLRiskConfigurationYieldBasisDeltaAnalytic(void)
:AQLRiskConfigurationYieldBasisDelta()
{
}

// destructor
/*!

*/
AQLRiskConfigurationYieldBasisDeltaAnalytic::~AQLRiskConfigurationYieldBasisDeltaAnalytic(void)
{
}

