/*! @file
    @brief FX Risk Scenario create class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationFX.cpp
//
//  DESCRIPTION :       FX Risk Scenario create class
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


#include "AQLDataInstance.h"
#include "AQLCoreReferencePool.h"
#include "AQLObjectPool.h"
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataProcedure.h"
#include "AQLDataMultiReference.h"
#include "LAScenarioConfigurationFX.h"
#include "LAMarketData.h"
#include "AQLBasic.h"

using namespace std;

// constructor
/*!

*/
LAScenarioConfigurationFX::LAScenarioConfigurationFX(void)
{
}

// destructor
/*!

*/
LAScenarioConfigurationFX::~LAScenarioConfigurationFX(void)
{
}


 
/*!
    @brief Risk Scenario create method

	@param[int,out] dataInstance
	@param[in] param
	@return vector<MBEnity *>
*/
vector<AQLObject *>
LAScenarioConfigurationFX::createScenario(AQLDataInstance &dataInstance, const MAScenarioParam &param) const
{
	// check
	if (!param.isParallel || param.isGrid)
	{
		throw AQLCoreInvalidData("FX Scenario creator only supoort parallel shift", __FILE__, __LINE__);
	}
	// get target FX
	AQLString tFXName = param.targetName;
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	AQLString name = tFXName + "_" + param.calcType;
	LAMathFXEntity *fx = 0;
	const LAMathFXEntity &t_fx = dynamic_cast<const LAMathFXEntity &>(objPool.getObject(tFXName, ENCHKTYPE_ISDEFINED).get());
	AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
	if (!objHolder.isDefined())
	{
		// create fx
		fx = dynamic_cast<LAMathFXEntity *>(t_fx.clone());
		fx->getName().convertFromString(name);
	}
	else
	{
		fx = &dynamic_cast<LAMathFXEntity &>(objHolder.get());
	}
	//////////////
	// set rate
	//////////////
	DoubleArray spotRates = t_fx.getSpotRates().get();
	unsigned int spotSize = spotRates.size(); 
	//set base shift value
	if (spotSize != param.extraBaseParamVec.size())
	{
		throw AQLCoreInvalidData("Spot rate size and Base shift size is not same.", __FILE__, __LINE__);
	}
	for (unsigned int i = 0; i < spotSize; ++i)
		spotRates[i] += param.extraBaseParamVec[i];

	if (spotSize != param.paraShiftVec.size())
	{
		throw AQLCoreInvalidData("Spot rate size and parallel shift size is not same.", __FILE__, __LINE__);
	}
	AQLString shiftType = param.shiftType;
	shiftType.toUpper();
	for (unsigned int i = 0; i < spotSize; ++i)
	{
		double shiftVal = param.paraShiftVec[i];
		if (shiftType == RISK_SHIFTTYPE_RATIO)
		{
			shiftVal *= spotRates[i];
		}
		spotRates[i] = AQLMath::max(spotRates[i] + shiftVal, 0.0);
		if (0.0 == spotRates[i])
			throw AQLCoreInvalidData("Spot rate must be positive.", __FILE__, __LINE__);
	}
	fx->getSpotRates() = spotRates;

	///////////////////////
	// set curve reference
	///////////////////////
	if (spotSize != param.refName.size())
	{
		throw AQLCoreInvalidData("Spot rate size and target curve size is not same.", __FILE__, __LINE__);
	}

	AQLString curveRef;
	for (unsigned int i = 0; i < spotSize; ++i)
	{
		curveRef += param.refName[i] + ":";
	}
	curveRef = curveRef.subString(0, curveRef.size() - 2);
	fx->getYieldCurves().convertFromString(curveRef);
	dataInstance.getReferencePool().completeDependency();
	
	vector<AQLObject *> ret(1, fx);
	return ret;

}

