#include "LARiskConfigurationYieldIRDeltaMultiParallel.h"
#include "LAFileAccessor.h"
#include "LAMarketData.h"
#include "LADealUtils.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LAStaticData.h"


#include <algorithm>
#include <vector>
#include <memory>



using namespace std;


std::vector<AQLObject *> 
LARiskConfigurationYieldIRDeltaMultiParallel::createMarketBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject*>(0);
	}

    double shift_val = getScenario1ParallelShiftStr(ccy).getDoubleValue() / 10000.0; // The unit of shift val is basis point.
	if (bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT || (scenarioNum == SCENARIO_2 && bumpDirection == RISK_BUMPDIRECTION_UPDOWNSHIFT))
	{
		shift_val *= -1.0;
	}

    // set up param
	MAScenarioParam param;
	param.ccy = ccy;
	param.calcType= getCalcType(ccy, scenarioNum, index);
	param.model = LAMarketData::getModelName(ccy);
	param.shiftType = getShiftType(ccy);
	param.bumpDirection = getBumpDirection(ccy);
	param.targetName = LAMarketData::getBaseYieldName(ccy);
    param.isAdjustDF = isAdjustDf(ccy);
	param.targetCurveType = getCurveType(ccy);
    param.isParallel = true;
	param.isGrid = false;

    param.paraShiftVec.resize(1, shift_val);
	

	std::shared_ptr<LAScenarioConfiguration> creator(LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELDMULTIPARALLEL));
    return creator->createScenario(dataInstance, param);
}
