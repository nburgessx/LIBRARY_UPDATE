#include "LARiskConfigurationYieldIRDeltaSimple.h"
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
LARiskConfigurationYieldIRDeltaSimple::createMarketBumpYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const
{
	AQLString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	if (scenarioNum == SCENARIO_2 && bumpDirection != RISK_BUMPDIRECTION_UPDOWNSHIFT)
	{
		return vector<AQLObject*>(0);
	}
    if(!isParallelShift(ccy)){
        return vector<AQLObject*>(0);
    }

    AQLString ccy_lower = ccy; ccy_lower.toLower();
    double shift_val = mpRiskStaticData->getStaticData(ccy_lower + STATIC_DATA_KEY_RISK_FRONT_YIELD_IRDELTA_PARALLEL_SHIFTVAL).getDoubleValue() / 10000.0; // The unit of shift val is basis point.
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

//  getMarketTerms(ccy, param.paraTerm);
	AQLString attrSuffix = "";
	if (param.targetCurveType != STD)
	{
		attrSuffix = "_" + param.targetCurveType;
	}
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLMathYieldCurvePro &bYieldPro = dynamic_cast<AQLMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(param.ccy), ENCHKTYPE_ISDEFINED).get());
	AQLDataMultiReference &refMarketDatas = dynamic_cast<AQLDataMultiReference &>
		                                  (bYieldPro.getData(CALIBRATION_DATA_MARKETDATA + attrSuffix, ISNOTNULL).get());
	param.paraTerm.resize(refMarketDatas.getSize());
	for (unsigned int i = 0; i < refMarketDatas.getSize(); i++)
	{
		const AQLObjectHolder objHolder = refMarketDatas.get(i);
		const AQLString &dataType = dynamic_cast<const AQLDataString &>
			(objHolder.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();
		if (dataType == YIELD_TYPE_O_N)
		{
			param.paraTerm[i] = "ON";
		}
		else if (dataType == YIELD_TYPE_T_N)
		{
			param.paraTerm[i] = "TN";
		}
		else
		{
			param.paraTerm[i] = dynamic_cast<const AQLDataString &>(objHolder.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();
		}
	}

	param.baseShiftVal.resize(param.paraTerm.size(), 0);
    param.paraShiftVec.resize(param.paraTerm.size());
    fill(param.paraShiftVec.begin(), param.paraShiftVec.end(), shift_val);
	


    std::shared_ptr<LAScenarioConfiguration> creator(LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD));
    return creator->createScenario(dataInstance, param);
}
