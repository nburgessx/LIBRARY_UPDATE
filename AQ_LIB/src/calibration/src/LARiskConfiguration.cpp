/*! @file
    @brief Risk setup class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfiguration.cpp
//
//  DESCRIPTION :        Risk setup class
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
#include <functional>
#include "LARiskConfiguration.h"
#include "LADataInstance.h"
#include "LADataMatrix.h"
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LAPriceDataFunction.h"
#include "LAPricePortfolioValue.h"
#include "LACoreDataService.h"
#include "LADealUtils.h"
#include "LADefinitionsRisk.h"
#include "LAStaticDataManager.h"
#include "LAMarketData.h"
#include "LALinearFunc.h"
#include "LAStaticData.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"
#include "LALinearRatesOptionValue.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LADataReference.h"
#include "LAAlgorithm.h"
#include "LAPricePayOff.h"
#include "LAPricePortfolioValue.h"
#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "LADefinitionsCalibration.h"

using namespace std;

// constructor
/*!

*/
LARiskConfiguration::LARiskConfiguration(bool fxFlg)
: mpStaticData(&LACoreDataService::getStaticDataManager().getStaticData()), 
  mpCalibStaticData(&LACoreDataService::getStaticDataManager().getCalibStaticData()), 
  mpRiskStaticData(&LACoreDataService::getStaticDataManager().getRiskStaticData()), mFXFlg(fxFlg)
{
	LAString analytictype = mpRiskStaticData->getStaticData(RISK_KEY_ANALYTIC_CALCTYPE);
	mAnalyticCalcType = analytictype.toUpper();
}

// destructor
/*!

*/
LARiskConfiguration::~LARiskConfiguration(void)
{
}

/*!
    @brief set curve Type

	@param[in] curveType
*/
void
LARiskConfiguration::setCurveType(const LAString &ccy, const LAString &curveType)
{
	LAString tmpCurrency = ccy;
	tmpCurrency.toUpper();
	mCurveTypeMap[tmpCurrency] = curveType;
}
 
/*!
    @brief set base scenario target

	@param[in] ccy
	@param[in] target
*/
void
LARiskConfiguration::setBaseScenarioTarget(const LAString &ccy, const LAString &target)
{
	LAString tmpCurrency = ccy;
	tmpCurrency.toUpper();
	mBaseTargetMap[tmpCurrency] = target;
}

/*!
    @brief isbasescenario target

	@return bool
*/
bool
LARiskConfiguration::isBaseScenarioTarget(const LAString &ccy, int index) const
{
	LAString tmpCurrency = ccy;
	tmpCurrency.toUpper();
	map<LAString, LAString>::const_iterator it = mBaseTargetMap.find(tmpCurrency);
	if (it != mBaseTargetMap.end())
	{
		++index;
		LAStringVector targetNums = it->second.toToken('/');
		for (unsigned int i = 0; i < targetNums.size(); ++i)
		{
			if (index == targetNums[i].getIntValue())
			{
				return true;
			}
		}
		return false;
	}
	else
	{
		return true;
	}
}

/*!
    @brief set curve Type

	@param[in] curveType
*/
LAString
LARiskConfiguration::getCurveSuffix(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	tmpCurrency.toUpper();

	map<LAString, LAString>::const_iterator it = mCurveTypeMap.find(tmpCurrency);
	if (it != mCurveTypeMap.end())
	{
		if (it->second != STD)
		{
			LAString tmp = it->second;
			return "." + tmp.toLower();
		}
		else
		{
			return "";
		}
	}
	else
	{
		return "";
	}
}

/*!
    @brief get base shift curve suffix

	@param[in] curveType
*/
LAString
LARiskConfiguration::getBaseShiftCurveSuffix(const LAString &ccy) const
{
	return getCurveSuffix(ccy);
}

/*!
    @brief set curve Type

	@param[in] curveType
*/
LAString
LARiskConfiguration::getCurveType(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	tmpCurrency.toUpper();

	map<LAString, LAString>::const_iterator it = mCurveTypeMap.find(tmpCurrency);
	if (it != mCurveTypeMap.end())
	{
		return it->second;
	}
	else
	{
		return STD;
	}
}

/*!
    @brief get base shift curve Type

	@param[in] curveType
*/
LAString
LARiskConfiguration::getBaseShiftCurveType(const LAString &ccy) const
{
	return getCurveType(ccy);
}

/*!
    @brief setup risk object

	@param[out] dataInstance
	@param[in] isRefresh
*/
void
LARiskConfiguration::setUpRiskEntity(LADataInstance &dataInstance, bool isReflesh) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	vector<pair<LAString, vector<LAObject *> > > eMapVec = createRiskEntity(objPool);
	const unsigned int size = eMapVec.size();
	if (size <= 0)
	{
		return ;
	}

	LAString refStr;
	for (unsigned int i = 0; i < size; ++i)
	{
		pair<const LAString, vector<LAObject *> > p= eMapVec[i];
		const LAString ccy = p.first;
		vector<LAObject *> eVec = p.second;
		const unsigned int vecSize = eVec.size();
		for (unsigned int j = 0; j < vecSize; ++j)
		{
			if (!eVec[j])
			{
				continue;
			}
			// setup target names 
			setUpTargetNames(ccy, *eVec[j], dataInstance);
			// setup basescenario
			setUpBaseScenario(ccy, *eVec[j], dataInstance, j);
			// setup scenario
			setUpScenario(ccy, *eVec[j], dataInstance, j);
			// setup operator
			setUpOperator(ccy, *eVec[j]);
			// setup coefficient
			setUpCoefficient(ccy, *eVec[j], dataInstance);
			// setup output name
			setUpOutputName(ccy, *eVec[j], j);
			// setup isgridsensitivity
			setUpIsGridSensitivity(ccy, *eVec[j]);
			// setup iswave
			setUpIsWave(ccy, *eVec[j]);
			//// setup valuable object
			//setUpValuableEntity(ccy, objPool, j);
			// setup riskoutputcurrency
			setUpRiskOutputCurrency(ccy,*eVec[j]);
			//is analytic
			setUpIsAnalytic(ccy, *eVec[j]);
			//setup fxvega deltatype
			setUpDeltaType(ccy, *eVec[j]);
			//setup irvegatype
			setUpVegaType(ccy, *eVec[j]);
			// setup valuable object info
			setUpValuableEntityInfo(ccy, objPool, *eVec[j], j);

			const LAString &name = dynamic_cast<const LADataString &>
							(eVec[j]->getData(CALIBRATION_DATA_NAME, ISDEFINED).get()).get();
			LAObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined())
			{
				// set object pool
				objPool.set(name, eVec[j]);
			}
			refStr += name + ":";
		}

	}
	// set risk object to portfolio
	if (refStr.size() < 2)
	{
		throw LACoreInvalidData("Risk info can not set !!", __FILE__, __LINE__);
	}
	//const LAString portName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	//LAObjectHolder port = objPool.getObject(portName, ENCHKTYPE_ISDEFINED);

	refStr = refStr.subString(0, refStr.size() - 2);
	if (!isReflesh)
	{
		LAStringVector riskENames = LACoreDataService::getContext(CONTEXT_KEY_RISKENTITY_NAMES).toToken(':');
		for (int i = riskENames.size() - 1; i >= 0 ; --i)
		{
			if (riskENames[i] != AQ_NO_DATA)
			{
				if (riskENames[i].findString(refStr) < 0)
				{
					refStr = riskENames[i] + ":" + refStr;
				}
			}
		}		
	}
	LACoreDataService::setContext(CONTEXT_KEY_RISKENTITY_NAMES, refStr);
	//if (!isReflesh)
	//{
	//	LADataHolder &dh = port.getData(PRICING_DATA_RISKCALCINFOS, NOCHECK);
	//	if (dh.isDefined() && !dh.isNull())
	//	{
	//		LAString befRefStr = dh.convertToString();
	//		befRefStr.exchange(LAString("\""), LAString(""));
	//		if (befRefStr != "" )
	//		{
	//			if (befRefStr.findString(refStr) < 0)
	//			{
	//				refStr = befRefStr + ":" + refStr;
	//			}
	//			else
	//			{
	//				refStr = befRefStr;
	//			}
	//		}
	//	}
	//}
	//port.remove(PRICING_DATA_RISKCALCINFOS);
	//port.add(PRICING_DATA_RISKCALCINFOS, new LADataMultiReference()).convertFromString(refStr);

	//dataInstance.getReferencePool().completeDependency();
}

/*!
    @brief create risk object

	@param [in] objPool
	@return vector<pair<LAString, vector<LAObject *> > >
*/
vector<pair<LAString, vector<LAObject *> > >
LARiskConfiguration::createRiskEntity(LAObjectPool &objPool) const
{
	LAStringVector ccys = getRiskCurrencys(objPool);
	vector<pair<LAString, vector<LAObject *> > > ret;
	const unsigned int ccyNum = ccys.size();

	// for ir risk
	for (unsigned int i = 0; i < ccyNum; ++i)
	{
		if (!mFXFlg)
		{
			if (!isTarget(ccys[i]))
			{
				continue;
			}
			if (ccys[i].findString(FX_DELIMITER) >= 0)
			{
				break;
			}
		}
		else
		{
			if (!isTarget(ccys[i]))
			{
				continue;
			}
			if (ccys[i].findString(FX_DELIMITER) < 0)
			{
				continue;
			}
		}

		LAString curveSuffix = getCurveSuffix(ccys[i]);

		// IMM delta/gamma conversion
		const int immFwdRiskMode = getIMMFwdRiskMode(ccys[i]);
		LADataInts immTerm;
		LADataStrings riskYieldCurveName;
		LADataString riskFloorTerm;
		LADataDoubles actualGridShiftVal;
		if (immFwdRiskMode != 0)
		{
			immTerm.set(getIMMTerm(ccys[i]));
		}
		if (immFwdRiskMode == 1)
		{
			riskYieldCurveName.set(getIMMRiskYieldCurveName(ccys[i]));
			LAString riskFloorTermStr = getIMMRiskFloorTerm(ccys[i], objPool);
			if (riskFloorTermStr != "")
			{
				riskFloorTerm.set(riskFloorTermStr);
			}
			DoubleArray shift = getScenario1GridShift(ccys[i]);
			if (getBumpDirection(ccys[i]).toUpper() == RISK_BUMPDIRECTION_DOWNSHIFT)
			{
				std::transform(shift.begin(), shift.end(), shift.begin(), std::negate<double>());
			}
			actualGridShiftVal.set(shift);
		}

		vector<LAObject *> eVec;
		// non scenario case
		DoubleArray shiftVals = getBaseShiftVals(ccys[i]);
		if(shiftVals.empty())
		{
			//when no base shifts
			LAString name = ccys[i].toUpper() + "_" + getRiskName() + "_" + getCurveType(ccys[i]);
			LAObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
			LAObject *e = 0;
			if (!objHolder.isDefined())
			{
				// create risk object
				e = new LAObject();
			}
			else
			{
				e = &objHolder.get();
				e->reset();
			}
			e->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(name);
			e->add(PRICING_DATA_RISKCURVETYPENAME, new LADataString()).convertFromString(getCurveType(ccys[i]));
			e->add(PRICING_DATA_RISKCURVETYPECURRENCY, new LADataString()).convertFromString(ccys[i]);

			// IMM delta/gamma conversion
			if (immFwdRiskMode != 0)
			{
				e->add(PRICING_DATA_IMMFWDRATETERM, new LADataInts(immTerm));
			}
			if (immFwdRiskMode == 1)
			{
				e->add(PRICING_DATA_IMMRISKYIELDCURVENAME, new LADataStrings(riskYieldCurveName));
				if (!riskFloorTerm.isNull())
				{
					e->add(PRICING_DATA_IMMRISKFLOORTERM, new LADataString(riskFloorTerm));
				}
				e->add(PRICING_DATA_IMMACTUALGRIDSHIFTVAL, new LADataDoubles(actualGridShiftVal));
			}

			vector<LAObject *> eVec(1, e);
			ret.push_back(make_pair(ccys[i].toUpper(), eVec));
		
		}
		else
		{
			const int shiftSize = shiftVals.size();
			//when cross risk
			eVec.resize(shiftSize);
			for (int j = 0; j < shiftSize; ++j)
			{
				if (!isBaseScenarioTarget(ccys[i], j))
				{
					eVec[j] = 0;
					continue;
				}
				//when no base shifts
				LAString name = ccys[i].toUpper() + "_" + getRiskName() + "_" + getCurveType(ccys[i]) + "_Shift_" + LAString(shiftVals[j]);
				LAString crosscur = getCrossBaseCurrency(ccys[i]);
				if(crosscur != AQ_NO_DATA)
					name += "_BASED_" + crosscur;
				
				LAObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
				LAObject *e = 0;
				if (!objHolder.isDefined())
				{
					// create risk object
					e = new LAObject();
				}
				else
				{
					e = &objHolder.get();
					e->reset();
				}
				e->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(name);
				e->add(PRICING_DATA_RISKCURVETYPENAME, new LADataString()).convertFromString(getCurveType(ccys[i]));
				e->add(PRICING_DATA_RISKBASESHIFTCURVETYPENAME, new LADataString()).convertFromString(getBaseShiftCurveType(ccys[i]));
				e->add(PRICING_DATA_RISKCURVETYPECURRENCY, new LADataString()).convertFromString(ccys[i]);

				// IMM delta/gamma conversion
				if (immFwdRiskMode != 0)
				{
					e->add(PRICING_DATA_IMMFWDRATETERM, new LADataInts(immTerm));
				}
				if (immFwdRiskMode == 1)
				{
					e->add(PRICING_DATA_IMMRISKYIELDCURVENAME, new LADataStrings(riskYieldCurveName));
					if (!riskFloorTerm.isNull())
					{
						e->add(PRICING_DATA_IMMRISKFLOORTERM, new LADataString(riskFloorTerm));
					}
					e->add(PRICING_DATA_IMMACTUALGRIDSHIFTVAL, new LADataDoubles(actualGridShiftVal));
				}

				eVec[j] = e;
			}	
			ret.push_back(make_pair(ccys[i].toUpper(), eVec));
		}
		
	}
	return ret;
}


/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfiguration::setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const
{
	const LAString targetNames = getTargetNames(ccy, dataInstance);
	e.remove(PRICING_DATA_TARGETNAMES);
	e.add(PRICING_DATA_TARGETNAMES, new LADataStrings()).convertFromString(targetNames);
	
	// set extra target
	LAStringVector exTarget1= getExtraTargetNames1(ccy, dataInstance);
	const unsigned int size1 = exTarget1.size();
	for (unsigned int i = 0; i < size1; ++i)
	{
		const LAString attrName1 = PRICING_DATA_EXTRATARGETNAMES1 + LAString("_") + LAString(static_cast<int>(i + 1));
		e.remove(attrName1);
		e.add(attrName1, new LADataStrings()).convertFromString(exTarget1[i]);
	}
	LAStringVector exTarget2= getExtraTargetNames2(ccy, dataInstance);
	const unsigned int size2 = exTarget2.size();
	for (unsigned int i = 0; i < size2; ++i)
	{
		const LAString attrName2 = PRICING_DATA_EXTRATARGETNAMES2 + LAString("_") + LAString(static_cast<int>(i + 1));
		e.remove(attrName2);
		e.add(attrName2, new LADataStrings()).convertFromString(exTarget2[i]);
	}

	//set base target
	const LAString bTargetNames = getBaseTargetNames(ccy, dataInstance);
	e.remove(PRICING_DATA_BASETARGETNAMES);
	e.add(PRICING_DATA_BASETARGETNAMES, new LADataStrings()).convertFromString(bTargetNames);

	// set base extra target
	LAStringVector bExTarget= getBaseExtraTargetNames(ccy, dataInstance);
	e.remove(PRICING_DATA_BASEEXTRATARGETNAMES);
	e.add(PRICING_DATA_BASEEXTRATARGETNAMES, new LADataStrings(bExTarget));

}



/*!
    @brief setup basescenario

	@param[in] ccy
	@param[out] e
	@param[in,out] dataInstance
	@param[in] index
*/
void
LARiskConfiguration::setUpBaseScenario(const LAString &ccy, LAObject &e, LADataInstance &dataInstance, int index) const
{
cout << "[ Currency = " << ccy << ", BaseScenarioNum = " << index << " ]" << endl;
cout << "LARiskConfiguration create base scenario called.." << endl;
clock_t cstart = clock();
	vector<LAObject *> bSce= createBaseScenarioEntity(ccy, dataInstance, index);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LARiskConfiguration create base scenario end.." << endl;
cout << "-> time = " << time << endl;
	LAObjectPool &objPool = dataInstance.getObjectPool();

	mBaseSceNames.clear();
	const unsigned int bSize = bSce.size();
	if (bSize > 0)
	{
		LADataStrings *attrBSce = new LADataStrings();
		const LAString tNames = getTargetNames(ccy, dataInstance);
		LADataStrings tmp;
		tmp.convertFromString(tNames);
		const unsigned int tSize = tmp.getSize();
		// if size == 1, set correspond to target
		if (bSize == 1)
		{
			const LAString &name = dynamic_cast<const LADataString &>
								(bSce[0]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

			bSce[0]->remove(AP_CALIBRATION_DATA_ISRISKENTITY);
			bSce[0]->add(AP_CALIBRATION_DATA_ISRISKENTITY, new LADataBool(true));
			LAObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined())
			{
				objPool.set(name, bSce[0]);
			}
			attrBSce->push_back(name);
			mBaseSceNames.push_back(name);
		}
		else
		{
			if (bSize != tSize)
			{
				throw LACoreInvalidData("Base scenario size equals one or equals target size.", __FILE__, __LINE__);
			}
			
			if (isGridSensitivity(ccy))
			{
				throw LACoreInvalidData("Gridsensitivity does not support multi basescenario.", __FILE__, __LINE__);
			}
			for (unsigned int i = 0; i < tSize; ++i)
			{
				const LAString &name = dynamic_cast<const LADataString &>
									(bSce[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

				bSce[i]->remove(AP_CALIBRATION_DATA_ISRISKENTITY);
				bSce[i]->add(AP_CALIBRATION_DATA_ISRISKENTITY, new LADataBool(true));
				LAObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					objPool.set(name, bSce[i]);
				}
				attrBSce->push_back(name);
				mBaseSceNames.push_back(name);
			}
		}
		e.remove(PRICING_DATA_BASESCENARIO);
		e.add(PRICING_DATA_BASESCENARIO, attrBSce);
		// extra scneario
cout << "LARiskConfiguration create base extra scenario called.." << endl;
clock_t cstart = clock();
		vector<LAObject *>  exSce = createBaseExtraScenarioEntity(ccy, dataInstance, index);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LARiskConfiguration create base extra scenario end.." << endl;
cout << "-> time = " << time << endl;
		unsigned int ex_sceSize = exSce.size();
		LAString ex_sceStr;
		mExtraBaseSceNames.clear();
		for (unsigned int i = 0; i < ex_sceSize; ++i)
		{
			const LAString &name = dynamic_cast<const LADataString &>
								(exSce[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			exSce[i]->remove(AP_CALIBRATION_DATA_ISRISKENTITY);
			exSce[i]->add(AP_CALIBRATION_DATA_ISRISKENTITY, new LADataBool(true));
			ex_sceStr +=  name + ":";
			LAObjectHolder objHolder = objPool.getObject(name,  ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined())
			{
				objPool.set(name, exSce[i]);
			}
			mExtraBaseSceNames.push_back(name);
		}
		if (ex_sceStr.size() > 1)
		{
			// set scenario object
			e.remove(PRICING_DATA_BASEEXTRASCENARIO);
			e.add(PRICING_DATA_BASEEXTRASCENARIO, new LADataMultiReference()).
				convertFromString(ex_sceStr.subString(0, ex_sceStr.size() - 2));
		}

	}
	
}

/*!
    @brief setup scenario

	@param[in] ccy
	@param[out] e
	@param[out] dataInstance
	@param[in] index
*/
void
LARiskConfiguration::setUpScenario(const LAString &ccy, LAObject &e, LADataInstance &dataInstance, int index) const
{
	mSce1Names.clear();
	mSce2Names.clear();
	LAObjectPool &objPool = dataInstance.getObjectPool();
	//////////////////
	// scenario1
	//////////////////
cout << "LARiskConfiguration create scenario 1 called.." << endl;
clock_t cstart = clock();
	vector<LAObject *> sce1 = createScenario1Entity(ccy, dataInstance, index);
clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LARiskConfiguration create scenario 1 end.." << endl;
cout << "-> time = " << time << endl;
	int unsigned sce1Size = sce1.size();
	if (sce1Size == 0) return;
	mSce1Names.resize(sce1Size);
	LAString sce1Str;
	for (int unsigned i = 0; i < sce1Size; ++i)
	{
		const LAString &name = dynamic_cast<const LADataString &>
							(sce1[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
		sce1[i]->remove(AP_CALIBRATION_DATA_ISRISKENTITY);
		sce1[i]->add(AP_CALIBRATION_DATA_ISRISKENTITY, new LADataBool(true));
		mSce1Names[i] = name;
		sce1Str +=  name + ":";
		LAObjectHolder objHolder = objPool.getObject(name,  ENCHKTYPE_NOCHECK);
		if (!objHolder.isDefined())
		{
			objPool.set(name, sce1[i]);
		}
	}
	
	if (sce1Str.size() < 2)
	{
		throw LACoreInvalidData("Scenario1 can not set !!", __FILE__, __LINE__);
	}

	// set scenario1 object
	e.remove(PRICING_DATA_SCENARIO1);
	e.add(PRICING_DATA_SCENARIO1, new LADataMultiReference()).
		convertFromString(sce1Str.subString(0, sce1Str.size() - 2));
	
	// extra scneario1
cout << "LARiskConfiguration create extra scenario 1 called.." << endl;
cstart = clock();
	vector<vector<LAObject *> > exSce1 = createExtraScenario1Entity(ccy, dataInstance, index);
cend = clock();
time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LARiskConfiguration create extra scenario 1 end.." << endl;
cout << "-> time = " << time << endl;
	unsigned int ex_sce1Num = exSce1.size();
	for (unsigned int i = 0; i < ex_sce1Num; ++i)
	{
		unsigned int ex_sce1Size = exSce1[i].size();
		if (ex_sce1Size != sce1Size)
		{
			throw LACoreInvalidData("Scenario1 size and extra scenario1 size is not same.", __FILE__, __LINE__);
		}

		LAString ex_sce1Str;
		for (unsigned int j = 0; j < ex_sce1Size; ++j)
		{
			const LAString &name = dynamic_cast<const LADataString &>
								(exSce1[i][j]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			exSce1[i][j]->remove(AP_CALIBRATION_DATA_ISRISKENTITY);
			exSce1[i][j]->add(AP_CALIBRATION_DATA_ISRISKENTITY, new LADataBool(true));
			ex_sce1Str +=  name + ":";
			LAObjectHolder objHolder = objPool.getObject(name,  ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined())
			{
				objPool.set(name, exSce1[i][j]);
			}
		}
		if (ex_sce1Str.size() < 2)
		{
			throw LACoreInvalidData("Extra scenario1 can not set !!", __FILE__, __LINE__);
		}
		// set scenario1 object
		LAString attrNameEx1 = PRICING_DATA_EXTRASCENARIO1 + LAString("_") + LAString(static_cast<int>(i + 1));
		e.remove(attrNameEx1);
		e.add(attrNameEx1, new LADataMultiReference()).
			convertFromString(ex_sce1Str.subString(0, ex_sce1Str.size() - 2));
	}

	//////////////////
	// scenario2
	//////////////////
cout << "LARiskConfiguration create scenario 2 called.." << endl;
cstart = clock();
	vector<LAObject *> sce2 = createScenario2Entity(ccy, dataInstance, index);
cend = clock();
time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LARiskConfiguration create scenario 2 end.." << endl;
cout << "-> time = " << time << endl;
	if (!sce2.empty())
	{
		const unsigned int sce2Size = sce2.size();
		mSce2Names.resize(sce2Size);
		LAString sce2Str;
		for (int unsigned i = 0; i < sce2Size; ++i)
		{
			const LAString &name = dynamic_cast<const LADataString &>
							(sce2[i]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			sce2[i]->remove(AP_CALIBRATION_DATA_ISRISKENTITY);
			sce2[i]->add(AP_CALIBRATION_DATA_ISRISKENTITY, new LADataBool(true));
			mSce2Names[i] = name;
			sce2Str +=  name + ":";

			LAObjectHolder objHolder = objPool.getObject(name,  ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined())
			{
				objPool.set(name, sce2[i]);
			}
			
		}
		if (sce2Str.size() < 2)
		{
			throw LACoreInvalidData("Scenario2 can not set !!", __FILE__, __LINE__);
		}
		// set scenario2 object
		e.remove(PRICING_DATA_SCENARIO2);
		e.add(PRICING_DATA_SCENARIO2, new LADataMultiReference()).
			convertFromString(sce2Str.subString(0, sce2Str.size() - 2));

		// extra scneario2
cout << "LARiskConfiguration create extra scenario 2 called.." << endl;
cstart = clock();
		vector<vector<LAObject *> > exSce2 = createExtraScenario2Entity(ccy, dataInstance, index);
cend = clock();
time = (double)(cend - cstart) / CLOCKS_PER_SEC;
cout << "LARiskConfiguration create extra scenario 2 end.." << endl;
cout << "-> time = " << time << endl;
		unsigned int ex_sce2Num = exSce2.size();
		for (unsigned int i = 0; i < ex_sce2Num; ++i)
		{
			unsigned int ex_sce2Size = exSce2[i].size();
			if (ex_sce2Size != sce2Size)
			{
				throw LACoreInvalidData("Scenario2 size and extra scenario2 size is not same.", __FILE__, __LINE__);
			}

			LAString ex_sce2Str;
			for (unsigned int j = 0; j < ex_sce2Size; ++j)
			{
				const LAString &name = dynamic_cast<const LADataString &>
									(exSce2[i][j]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
				exSce2[i][j]->remove(AP_CALIBRATION_DATA_ISRISKENTITY);
				exSce2[i][j]->add(AP_CALIBRATION_DATA_ISRISKENTITY, new LADataBool(true));
				ex_sce2Str +=  name + ":";
				LAObjectHolder objHolder = objPool.getObject(name,  ENCHKTYPE_NOCHECK);
				if (!objHolder.isDefined())
				{
					objPool.set(name, exSce2[i][j]);
				}
			}
			if (ex_sce2Str.size() < 2)
			{
				throw LACoreInvalidData("Extra scenario2 can not set !!", __FILE__, __LINE__);
			}
			// set scenario2 object
			LAString attrNameEx2 = PRICING_DATA_EXTRASCENARIO2 + LAString("_") + LAString(static_cast<int>(i + 1));
			e.remove(attrNameEx2);
			e.add(attrNameEx2, new LADataMultiReference()).
				convertFromString(ex_sce2Str.subString(0, ex_sce2Str.size() - 2));
		}
	}
}

/*!
    @brief setup operator

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfiguration::setUpOperator(const LAString &ccy, LAObject &e) const
{
	ccy;
	e.remove(PRICING_DATA_OPERATOR);
	e.add(PRICING_DATA_OPERATOR, new LAPriceDataFunction()).
							convertFromString(getOperator1());
	LAString ope2 = getOperator2();
	if (ope2 != AQ_NO_DATA)
	{
		e.remove(PRICING_DATA_OPERATOR2);
		e.add(PRICING_DATA_OPERATOR2, new LAPriceDataFunction()).convertFromString(ope2);
	}

	
	if (isWave(ccy))
	{
		LAString ope_w = getWaveOperator();
		if (ope_w != AQ_NO_DATA)
		{
			e.remove(PRICING_DATA_WAVEOPERATOR);
			e.add(PRICING_DATA_WAVEOPERATOR, new LAPriceDataFunction()).convertFromString(ope_w);
		}
	}

	LAString ope_b = getBaseOperator();
	if (ope_b != AQ_NO_DATA)
	{
		e.remove(PRICING_DATA_BASEOPERATOR);
		e.add(PRICING_DATA_BASEOPERATOR, new LAPriceDataFunction()).convertFromString(ope_b);
	}
}

/*!
    @brief setup coefficient

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfiguration::setUpCoefficient(const LAString &ccy, LAObject &e, LADataInstance& dataInstance) const
{
	e.remove(PRICING_DATA_COEFFICIENT);
	LAString coef1 = getCoefficient1(ccy);
	if (coef1 != AQ_NO_DATA)
	{
		LAStringVector bfadjvec1 = coef1.toToken(':');
		double coef1ratio = getCoefficientRatio1(ccy,e,dataInstance);
		for (unsigned int i = 0; i < bfadjvec1.size(); i++)
		{
			double adjval1 = bfadjvec1[i].getDoubleValue() * coef1ratio;
			bfadjvec1[i] = LAString(adjval1);
		}
		coef1 = LADataStrings(bfadjvec1).convertToString();
	}
	e.add(PRICING_DATA_COEFFICIENT, new LADataDoubles()).convertFromString(coef1);

	LAString coef2 = getCoefficient2(ccy);	
	if (coef2 != AQ_NO_DATA)
	{
		e.remove(PRICING_DATA_COEFFICIENT2);
		LAStringVector bfadjvec2 = coef2.toToken(':');
		double coef2ratio = getCoefficientRatio2(ccy,e,dataInstance);
		for (unsigned int i = 0; i < bfadjvec2.size(); i++)
		{
			double adjval2 = bfadjvec2[i].getDoubleValue() * coef2ratio;
			bfadjvec2[i] = LAString(adjval2);
		}
		coef2 = LADataStrings(bfadjvec2).convertToString();
		e.add(PRICING_DATA_COEFFICIENT2, new LADataDoubles()).convertFromString(coef2);
	}

	if (isWave(ccy))
	{
		LAString coef_w = getWaveCoefficient(ccy);
		if (coef_w != AQ_NO_DATA)
		{
			e.remove(PRICING_DATA_WAVECOEFFICIENT);
			e.add(PRICING_DATA_WAVECOEFFICIENT, new LADataDoubleMatrix()).convertFromString(coef_w);
		}
	}

	LAString coef_b = getBaseCoefficient(ccy);
	if (coef_b != AQ_NO_DATA)
	{
		e.remove(PRICING_DATA_BASECOEFFICIENT);
		e.add(PRICING_DATA_BASECOEFFICIENT, new LADataDoubles()).convertFromString(coef_b);
	}
}


/*!
    @brief return operator1

	@return LAString
*/
LAString
LARiskConfiguration::getOperator1(void) const
{
	return FN_LINEAR_STR;
}


/*!
    @brief return coefficient1

	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfiguration::getCoefficient1(const LAString &ccy) const
{
	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();
	const double divUnit = getDivUnit(ccy);
	if (divUnit == 0.0)
	{
		throw LACoreInvalidData("Divid unit = 0 !!", __FILE__, __LINE__);
	}
	if (bumpDirection == RISK_BUMPDIRECTION_UPSHIFT)
	{
		const double val = 1.0 / divUnit;
		return LAString(-val) + LAString(":")
				+  LAString(val) + LAString(":0.0");
	}
	else if (bumpDirection == RISK_BUMPDIRECTION_DOWNSHIFT)
	{
		const double val = 1.0 / divUnit;
		return LAString(val) + LAString(":")
				+  LAString(-val) + LAString(":0.0");
	}
	else
	{
		const double val = 0.5 / divUnit;
		return LAString("0.0:") + LAString(val) + LAString(":")
						+  LAString(-val) + LAString(":0.0");
	}
}

/*!
    @Description: Tells IMM forward risk mode
    @param ccy [in] The currency
    @return 0 if no IMM-forward-risk-like care is applied,
            1 if market rate bump risk -> IMM forward risk conversion is applied,
            2 if market zero rate bump risk -> IMM forward risk conversion is applied;
            This class simply returns 0
*/
int
LARiskConfiguration::getIMMFwdRiskMode(const LAString &ccy) const
{
	return 0;
}

/*!
    @Description: Returns (possibly term-wise) risk yield curve name(s) for IMM forward risks
    @param ccy [in] The currency
    @return An empty vector if all IMM forward risks shall be zero,
            a vector with one risk curve name if all risk curves are the same,
            a vector with risk curve names whose number is the same as the risk grids,
            where a risk curve name is the curve type which is suitable to be passed to
            LAMathYieldCurvePro::setCurveType (for example, "JPYOIS") to deduce the forward rate changes
            against the market rate bumps;
            This class simply returns a vector which contains one empty LAString object
*/
LAStringVector
LARiskConfiguration::getIMMRiskYieldCurveName(const LAString &) const
{
	std::vector<LAString> v;
	v.push_back("");
	return v;
}

/*!
    @Description: Returns the risk floor term, the date from which the first market rate bump risk is attributed to,
                  that is, if this is "3Y" and the first market bump term is "4Y", then the first risk is attributed to
                  the period [3Y, 4Y) instead of [baseDate, 4Y)
    @param ccy [in] The currency
    @arapm objPool [in] the object pool
    @return An empty string if there is no such risk floor term, a appropriate term string otherwise
            This class simply returns an empty LAString object
*/
LAString
LARiskConfiguration::getIMMRiskFloorTerm(const LAString &ccy, LAObjectPool &objPool) const
{
	return "";
}

/*!
    @brief return wave operator

	@return LAString
*/
LAString
LARiskConfiguration::getWaveOperator(void) const
{
	return FN_LINEAR_STR;
}

/*!
    @brief return wave coefficient
	
	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfiguration::getWaveCoefficient(const LAString &ccy) const
{
	LAString baseCoef = getBaseWaveCoefficient(ccy);
	if (isGridSensitivity(ccy) && isParallelShift(ccy))
	{
		// grid and parallel
		LAString ret;
		LAString bumpDirection = getBumpDirection(ccy);
		bumpDirection.toUpper();
		
		// extra coefficient for first grid and second grid
		const double val = 1.0;
		ret = LAString("0.0:") + LAString(val) + LAString(":0.0;");
		ret += LAString("0.0:") + LAString(val) + LAString(":0.0;");
		
		unsigned int gridIndex = getMaxGridIndex(ccy);
		LAStringVector BucketGridTerm = getBucketGridTerm(ccy);		
		if(BucketGridTerm[0] != AQ_NO_DATA)
		{
			gridIndex = BucketGridTerm.size() - 1;
		}
		
		for (unsigned int i = 1; i <= gridIndex; ++i)
		{
			ret += baseCoef + ";";
		}
		return ret.subString(0, ret.size() - 2);
	}
	else if(isGridSensitivity(ccy))
	{
		// grid only
		LAString ret;
		LAString bumpDirection = getBumpDirection(ccy);
		bumpDirection.toUpper();

		// extra coefficient for first grid
		const double val = 1.0;
		ret = LAString("0.0:") + LAString(val) + LAString(":0.0;");
		unsigned int gridIndex = getMaxGridIndex(ccy);
		LAString tmpccy = ccy;
		LAStringVector BucketGridTerm = getBucketGridTerm(ccy);		
		if(BucketGridTerm[0] != AQ_NO_DATA)
		{
			gridIndex = BucketGridTerm.size() - 1;
		}
		for (unsigned int i = 1; i <= gridIndex; ++i)
		{
			ret += baseCoef + ";";
		}
		return ret.subString(0, ret.size() - 2);
	}
	else
	{
		// parallel only
		return baseCoef;
	}
}

/*!
    @brief return base wave coefficient
	
	@param[in] ccy
	@return LAString
*/
LAString
LARiskConfiguration::getBaseWaveCoefficient(const LAString &ccy) const
{
	LAString bumpDirection = getBumpDirection(ccy);
	bumpDirection.toUpper();

	const double val = 1.0;
	return LAString(-val) + LAString(":")
			+  LAString(val) + LAString(":0.0");
}

/*!
    @brief setup ouputname

	@param[in] ccy
	@param[out] e
	@param[in] index
*/
void
LARiskConfiguration::setUpOutputName(const LAString &ccy, LAObject &e, int index) const
{
	e.remove(PRICING_DATA_OUTPUTNAME);
	e.add(PRICING_DATA_OUTPUTNAME, new LADataString()).
						convertFromString(getOutPutName1(ccy, index));
	LAString out2 = getOutPutName2(ccy, index);
	if (out2 != AQ_NO_DATA)
	{
		e.remove(PRICING_DATA_OUTPUTNAME2);
		e.add(PRICING_DATA_OUTPUTNAME2, new LADataString()).convertFromString(out2);
	}

	LAString out_b = getBaseOutPutName(ccy, index);
	if (out_b != AQ_NO_DATA)
	{
		e.remove(PRICING_DATA_BASEOUTPUTNAME);
		e.add(PRICING_DATA_BASEOUTPUTNAME, new LADataString()).convertFromString(out_b);
	}
}

/*!
    @brief setup isgridsensitivity

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfiguration::setUpIsGridSensitivity(const LAString &ccy, LAObject &e) const
{
	e.remove(PRICING_DATA_ISGRIDSENSITIVITY);
	e.add(PRICING_DATA_ISGRIDSENSITIVITY, new LADataBool(isGridSensitivity(ccy)));
}

/*!
    @brief setup iswave

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfiguration::setUpIsWave(const LAString &ccy, LAObject &e) const
{
	e.remove(PRICING_DATA_ISWAVE);
	e.add(PRICING_DATA_ISWAVE, new LADataBool(isWave(ccy)));
}

/*!
    @brief setup valuable object

	@param[in] ccy
	@param[out] object pool
	@param[in] index
*/
void
LARiskConfiguration::setUpValuableEntityInfo(const LAString &ccy, LAObjectPool &objPool, LAObject &e, int index) const
{
	//LAString mTradeName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	//LAObjectHolder objHolder = objPool.getObject(mTradeName, ENCHKTYPE_ISDEFINED);
	
	LAString outputName1 = getOutPutName1(ccy, index);
	LAString outputName2 = getOutPutName2(ccy, index);
	// parallel data
	const LAString attrIsP1 = outputName1.toUpper() + AP_CALIBRATION_DATA_ISPARALLELSHIFT;
	const LAString attrIsP2 = outputName2.toUpper() + AP_CALIBRATION_DATA_ISPARALLELSHIFT;

	const LAString attrGrid1 = outputName1.toUpper() + AP_CALIBRATION_DATA_RISK_GRID;
	const LAString attrGrid2 = outputName2.toUpper() + AP_CALIBRATION_DATA_RISK_GRID;

	const LAString attrGridActVals1 = outputName1.toUpper() + AP_CALIBRATION_DATA_RISK_GRIDACTVALS;

	LAStringVector tradeSetAttr;

	if (isGridSensitivity(ccy))
	{
		const bool isParallel = isParallelShift(ccy);
		LAStringVector gridTerm;
		LAStringVector BucketGridTerm = getBucketGridTerm(ccy);
		if(BucketGridTerm[0] == AQ_NO_DATA)
		{
			gridTerm = getGridTerm(ccy, index);
		}
		else
		{
			LAStringVector tmpgridTerm = getGridTerm(ccy);
			LAStringVector changeGridTerm;
			for (unsigned int i = 0;i < tmpgridTerm.size();++i)
			{
				changeGridTerm.push_back(LAMarketData::convertToMLibTerm(tmpgridTerm[i]));
			}

			for (unsigned int i = 0;i < BucketGridTerm.size();++i)
			{
				unsigned int posf = 0;
				unsigned int posl = 0;
				if (i == 0)
				{
					LAString tmpbucketGrid = BucketGridTerm[i];
					LAStringVector bucketgrid = tmpbucketGrid.toToken('_');
					const LAStringVector gridVec = tmpgridTerm[0].toToken('_');
					const LAString &grid = gridVec[gridVec.size() - 1];
					const LAString &grid_b = bucketgrid[bucketgrid.size() - 1];
					if (grid != grid_b)
					{
						gridTerm.push_back(LAString(tmpgridTerm[0]) + "/" + grid_b);
					}
					else
					{
						gridTerm.push_back(LAString(tmpgridTerm[0]));
					}
				}
				else
				{
					for (unsigned int j = 0; j < changeGridTerm.size();++j)
					{
						const LAString& tmpGrid = changeGridTerm[j];
						const LAStringVector& gridVec = tmpGrid.toToken('_');
						const LAString& grid = gridVec[gridVec.size() - 1];
						if (grid == BucketGridTerm[i - 1] || tmpGrid == BucketGridTerm[i - 1])
						{
							posf = j;
						}
						if (grid == BucketGridTerm[i] || tmpGrid == BucketGridTerm[i])
						{
							posl = j;
							break;
						}
					}
					int gridMax = getMaxGridIndex(ccy);
					LAString tmpccy = ccy;
					LAString strBucketGrid = getPropertyBucketGridTerm(ccy);
					LAStringVector tmpGridTermVec = changeGridTerm[gridMax].toToken('_');
					if (strBucketGrid.findString(tmpGridTermVec[tmpGridTermVec.size() - 1]) == -1 && posl == gridMax)
					{
						LAStringVector maxShiftTermVec = getGridTerm(ccy);
						LAStringVector maxTermVec = maxShiftTermVec[maxShiftTermVec.size() - 1].toToken('_');
						LAString tmpbucketGrid = BucketGridTerm[i];
						LAStringVector bucketgrid = tmpbucketGrid.toToken('_');
						const LAStringVector gridVec = changeGridTerm[posf + 1].toToken('_');
						const LAString &grid = gridVec[gridVec.size() - 1];
						if (grid != maxTermVec[maxTermVec.size() - 1])
						{
							gridTerm.push_back(changeGridTerm[posf + 1] + LAString("/") + maxTermVec[maxTermVec.size() - 1]);
						}
						else
						{
							gridTerm.push_back(changeGridTerm[posf + 1]);
						}
					}
					else
					{
						LAString tmpbucketGrid = BucketGridTerm[i];
						LAStringVector bucketgrid = tmpbucketGrid.toToken('_');
						const LAStringVector gridVec = changeGridTerm[posf + 1].toToken('_');
						const LAString &grid = gridVec[gridVec.size() - 1];
						if (grid != bucketgrid[bucketgrid.size() - 1])
						{
							gridTerm.push_back(changeGridTerm[posf + 1] + LAString("/") + bucketgrid[bucketgrid.size() - 1]);
						}
						else
						{
							gridTerm.push_back(changeGridTerm[posf + 1]);
						}
					}

				}
			}
		}
		//else
		//{
		//	LAStringVector tmpgridTerm = getGridTerm(ccy);
		//	LAStringVector changeGridTerm;
		//	for (unsigned int i = 0;i < tmpgridTerm.size();++i)
		//	{
		//		changeGridTerm.push_back(LAMarketData::convertToMLibTerm(tmpgridTerm[i]));
		//	}

		//	for (unsigned int i = 0;i < BucketGridTerm.size();++i)
		//	{
		//		unsigned int posf = 0;
		//		unsigned int posl = 0;
		//		if (i == 0)
		//		{
		//			LAString tmpbucketGrid = BucketGridTerm[i];
		//			LAStringVector bucketgrid = tmpbucketGrid.toToken('_');
		//			gridTerm.push_back(LAString(tmpgridTerm[0]) + "/" + bucketgrid[bucketgrid.size() - 1]);
		//		}
		//		else
		//		{
		//			for (unsigned int j = 0; j < changeGridTerm.size();++j)
		//			{
		//				const LAString& tmpGrid = changeGridTerm[j];
		//				const LAStringVector& gridVec = tmpGrid.toToken('_');
		//				const LAString& grid = gridVec[gridVec.size() - 1];
		//				if (grid == BucketGridTerm[i - 1] || tmpGrid == BucketGridTerm[i - 1])
		//				{
		//					posf = j;
		//				}
		//				if (grid == BucketGridTerm[i] || tmpGrid == BucketGridTerm[i])
		//				{
		//					posl = j;
		//					break;
		//				}
		//			}
		//			int gridMax = getMaxGridIndex(ccy);
		//			LAString tmpccy = ccy;
		//			LAString strBucketGrid = getPropertyBucketGridTerm(ccy);
		//			LAStringVector tmpGridTermVec = changeGridTerm[gridMax].toToken('_');
		//			if (strBucketGrid.findString(tmpGridTermVec[tmpGridTermVec.size() - 1]) == -1 && posl == gridMax)
		//			{
		//				LAStringVector maxShiftTermVec = getGridTerm(ccy);
		//				LAStringVector maxTermVec = maxShiftTermVec[maxShiftTermVec.size() - 1].toToken('_');
		//				LAString tmpbucketGrid = BucketGridTerm[i];
		//				LAStringVector bucketgrid = tmpbucketGrid.toToken('_');
		//				LAString str = changeGridTerm[posf + 1] + LAString("/") + maxTermVec[maxTermVec.size() - 1];
		//				gridTerm.push_back(str);
		//			}
		//			else
		//			{
		//				LAString tmpbucketGrid = BucketGridTerm[i];
		//				LAStringVector bucketgrid = tmpbucketGrid.toToken('_');
		//				LAString str = changeGridTerm[posf + 1] + LAString("/") + bucketgrid[bucketgrid.size() - 1];
		//				gridTerm.push_back(str);
		//			}
		//		}
		//	}
		//}
		//if (dynamic_cast<const LADataValuation &>
		//	(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)

		//{
			//const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
			//									(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());

			//const unsigned int size = unders.getSize();

		LADate asOfDate = LAMarketData::getAsofDate(objPool);
		DoubleVector termvals(gridTerm.size());
		for (unsigned int i = 0; i < gridTerm.size(); i++)
		{
			termvals[i] = LAMarketData::getCalendarTime(asOfDate,gridTerm[i]);
		}

		e.remove(attrIsP1);
		e.add(attrIsP1, new LADataBool(isParallel));
		tradeSetAttr.push_back(attrIsP1);
		// set risk grid term
		e.remove(attrGrid1);
		e.add(attrGrid1, new LADataStrings(gridTerm));
		tradeSetAttr.push_back(attrGrid1);

		e.remove(attrGridActVals1);
		e.add(attrGridActVals1, new LADataDoubles(termvals));
		tradeSetAttr.push_back(attrGridActVals1);

		e.remove(attrIsP2);
		e.remove(attrGrid2);
		if (outputName2 != AQ_NO_DATA)
		{
			e.add(attrIsP2, new LADataBool(isParallel));
			tradeSetAttr.push_back(attrIsP2);
			e.add(attrGrid2, new LADataStrings(gridTerm));
			tradeSetAttr.push_back(attrGrid2);
		}
		e.remove(PRICING_DATA_ISSAVEPASTFIXING);
		e.add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(isSavePastFixing()));
		tradeSetAttr.push_back(PRICING_DATA_ISSAVEPASTFIXING);


			//for (int unsigned i = 0; i < size; ++i)
			//{
			//	LAObjectHolder &trade = unders.get(i);
			//	// set isparallel
			//	trade.remove(attrIsP1);
			//	trade.add(attrIsP1, new LADataBool(isParallel));
			//	// set risk grid term
			//	trade.remove(attrGrid1);
			//	trade.add(attrGrid1, new LADataStrings(gridTerm));
			//	trade.remove(attrGridActVals1);
			//	trade.add(attrGridActVals1, new LADataDoubles(termvals));

			//	trade.remove(attrIsP2);
			//	trade.remove(attrGrid2);
			//	if (outputName2 != AQ_NO_DATA)
			//	{
			//		trade.add(attrIsP2, new LADataBool(isParallel));
			//		trade.add(attrGrid2, new LADataStrings(gridTerm));
			//	}
			//	trade.remove(PRICING_DATA_ISSAVEPASTFIXING);
			//	trade.add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(isSavePastFixing()));
			//}
		//}
		//else
		//{
		//	throw LACoreInvalidData("If risk calc, value attr must be fn_ir_portfoliovalue", __FILE__, __LINE__);
		//}
	}
	else
	{
		//if (dynamic_cast<const LADataValuation &>
		//	(objHolder.getData(CALIBRATION_DATA_VALUE, ISNOTNULL).get()).getType() == FN_IR_PORTFOLIOVALUE)

		//{
		//	const LADataMultiReference &unders = dynamic_cast<const LADataMultiReference &>
		//										(objHolder.getData(CALIBRATION_DATA_UNDERLYINGS, ISNOTNULL).get());

		bool isanalyticfxoption  = (getRiskName().findString(RISK_FRONT_OPTIONANALYTIC) > 0);
		LADate asOf;
		DoubleVector termvec;
		LAStringVector gridTerm;
		LAStringVector BucketGridTerm;
		bool isgridchg = false;
		if (isanalyticfxoption)
		{
			asOf = LAMarketData::getAsofDate(objPool);
			gridTerm = getGridTerm(ccy);
			LAStringVector tmpchangeGridTerm;
			for (unsigned int i = 0;i < gridTerm.size();++i)
			{

				LAStringVector chk = gridTerm[i].toToken('_');
				if (chk.size()>1)
					gridTerm[i] = chk[1]; //AMT_10Y then 10Y

				tmpchangeGridTerm.push_back(LAMarketData::convertToMLibTerm(gridTerm[i]));
			}
			if (gridTerm.size() != 0)
			{

				DateVector tmpdatevec(gridTerm.size());
				termvec.resize(gridTerm.size());
				LAPriceDataDayCount dc(ACT_365_ISDA);
				for (unsigned int i = 0;i < gridTerm.size();++i)
				{
					if (tmpchangeGridTerm[i] == "ON")
					{
						tmpdatevec[i] = asOf;
						tmpdatevec[i].addDays(1);
					}
					else if (tmpchangeGridTerm[i] == "TN")
					{
						tmpdatevec[i] = asOf;
						tmpdatevec[i].addDays(2);
					}
					else
					{
						tmpdatevec[i] = LAMathDateCalculations::getDate(asOf,tmpchangeGridTerm[i],true);
					}
					termvec[i] = dc.getTerm(asOf,tmpdatevec[i],false);
				}
			
				isgridchg = true;
				BucketGridTerm = getBucketGridTerm(ccy);
			}

		}
		// set isparallel
		e.remove(attrIsP1);
		e.add(attrIsP1, new LADataBool(true));
		tradeSetAttr.push_back(attrIsP1);

		e.remove(attrIsP2);
		if (outputName2 != AQ_NO_DATA)
		{
			e.add(attrIsP2, new LADataBool(true));
			tradeSetAttr.push_back(attrIsP2);
		}
		// remove grid attr
		e.remove(attrGrid1);
		e.remove(attrGrid2);
		e.remove(attrGridActVals1);

		e.remove(PRICING_DATA_ISSAVEPASTFIXING);
		e.add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(isSavePastFixing()));
		tradeSetAttr.push_back(PRICING_DATA_ISSAVEPASTFIXING);

	

	//	for (int unsigned i = 0; i < size; ++i)
	//	{
	//		LAObjectHolder &trade = unders.get(i);
	//		
	//		if (isanalyticfxoption)
	//		{
	//			if(isgridchg)
	//			{
	//				LAString matuterm;
	//				double term = LAMarketData::getMaturityTermFromPlainVanillaEntity(trade.get(), asOf); 
	//				unsigned int pos = 0;
	//				LAAlgorithm::locate<DoubleVector, double>(termvec,term,termvec.size(),pos);
	//				if (pos == termvec.size())
	//					throw LACoreInvalidData("GridSize Error",__FILE__,__LINE__);
	//				
	//				if(BucketGridTerm[0] == AQ_NO_DATA)
	//				{
	//					matuterm = gridTerm[pos];
	//				}
	//				else
	//				{
	//					throw LACoreInvalidData("Now Bucket Term does not support for AnalyticRisk",__FILE__,__LINE__);
	//				}

	//				trade.remove(termforriskkey);	
	//				trade.add(termforriskkey,new LADataString(matuterm));
	//			}
	//		}
	//		// set isparallel
	//		trade.remove(attrIsP1);
	//		trade.add(attrIsP1, new LADataBool(true));
	//		trade.remove(attrIsP2);
	//		if (outputName2 != AQ_NO_DATA)
	//		{
	//			trade.add(attrIsP2, new LADataBool(true));
	//		}
	//		// remove grid attr
	//		trade.remove(attrGrid1);
	//		trade.remove(attrGrid2);
	//		trade.remove(attrGridActVals1);

	//		trade.remove(PRICING_DATA_ISSAVEPASTFIXING);
	//		trade.add(PRICING_DATA_ISSAVEPASTFIXING, new LADataBool(isSavePastFixing()));

	//	}
	//}
	//else
	//{
	//	throw LACoreInvalidData("If risk calc, value attr must be fn_ir_portfoliovalue", __FILE__, __LINE__);
	//}
	}
	e.remove(AP_CALIBRATION_DATA_RISK_TRADESETDATA);
	e.add(AP_CALIBRATION_DATA_RISK_TRADESETDATA, new LADataStrings(tradeSetAttr));

	const bool omitNotionalExp = omitNotionalExposure(ccy);
	e.remove(PRICING_DATA_OMITNOTIONALEXPOSURE);
	e.add(PRICING_DATA_OMITNOTIONALEXPOSURE, new LADataBool(omitNotionalExp));

}

/*!
    @brief return istargetccy

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfiguration::isTarget(const LAString &ccy) const
{

	LAString targetccys = getTargetCurrencies();
	targetccys.toUpper();

	if (targetccys == "ALL")
	{
		return true;
	}
	else
	{
		LAString tmpCurrency = ccy;
		LAStringVector targetVec = targetccys.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (targetVec.end() != find(targetVec.begin(), targetVec.end(), tmpCurrency))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

/*!
    @brief return is calibration target

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfiguration::isCalibTarget(const LAString &ccy) const
{
	if (isRealCalib() && !LAMarketData::isCalibrateModel(LAMarketData::getModelName(ccy)))
	{
		return false;
	}

	LAString tmpCurrency = ccy;
	tmpCurrency.toUpper();
	
	LAStringVector simsde = MADealUtils::getSimulationSDECurrencys();
	unsigned int sdesize = simsde.size();
	if (sdesize != 0 && simsde.end() == std::find(simsde.begin(),simsde.end(),tmpCurrency))
	{
		return false;
	}
	
	LAString targetccys = getCalibTargetCurrencies();
	targetccys.toUpper();

	if (targetccys == "ALL")
	{
		return true;
	}
	else
	{
		LAStringVector targetVec = targetccys.toToken(MULTI_STATIC_DATA_DELIMITER);
		if (targetVec.end() != find(targetVec.begin(), targetVec.end(), tmpCurrency))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

/*!
    @brief convert string to bool val

	@param[in] str
	@return bool
*/
bool
LARiskConfiguration::convertBoolFromStr(const LAString &str) const
{
	if (str == "") 
	{
		return false;
	}
	else
	{
		LADataBool tmp;
		tmp.convertFromString(str);
		return tmp.get();
	}
}


/*!
    @brief get calib target fx

	@param ccy
	get calibration target fx.
	if ccy is not calibtarget, fx is not calibrated.	

*/
LAStringVector
LARiskConfiguration::getCalibTargetFX(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAStringVector fCurveCcys;
	if (ccy.findString(FX_DELIMITER) < 0 )
	{
		LAObjectPool &objPool = dataInstance.getObjectPool();
		LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ccy), ENCHKTYPE_ISDEFINED).get());
		fCurveCcys = ycPro.getAffectingCcy();
	}

	LAStringVector ret;
	//for nocalib
	LAString tmpCurrency = ccy;
	tmpCurrency.toUpper();
	LAStringVector simccys = MADealUtils::getSimulationSDECurrencys();
	unsigned int simccySize = simccys.size();
	bool isForeCcyInSimccy = false;
	if (simccySize != 0)
	{
		for (unsigned int i = 0; i < simccySize; ++i)
		{
			simccys[i].toUpper();
			for (unsigned int j = 0; j < fCurveCcys.size(); j++)
			{
				if (simccys[i].findString(fCurveCcys[j]) >= 0) isForeCcyInSimccy = true;
			}
			if (simccys[i].findString(FX_DELIMITER) >= 0 &&
				isCalibTarget(simccys[i]) && (simccys[i].findString(tmpCurrency) >= 0 || isForeCcyInSimccy))
			{
				ret.push_back(simccys[i]);
			}
		}
		return ret;
	}
	else
	{
		LAStringVector ccys = MADealUtils::getSDECurrencys();
		unsigned int ccySize = ccys.size();
		for (unsigned int i = 0; i < ccySize; ++i)
		{
			ccys[i].toUpper();
			for (unsigned int j = 0; j < fCurveCcys.size(); j++)
			{
				if (ccys[i].findString(fCurveCcys[j]) >= 0) isForeCcyInSimccy = true;
			}
			if (ccys[i].findString(FX_DELIMITER) >= 0 &&
				isCalibTarget(ccys[i]) && (ccys[i].findString(tmpCurrency) >= 0 || isForeCcyInSimccy))
			{
				ret.push_back(ccys[i]);
			}
		}
		return ret;
	}	
}


/*!
    @brief return is calibration target

	@param[in] ccy
	@return bool 
*/
bool
LARiskConfiguration::isRealCalib() const
{
	return convertBoolFromStr(mpRiskStaticData->getStaticData(RISK_KEY_CALIB_ISREALCALIB));
}

/*!
    @brief is adjst df
	@param[in] ccy
	@return bool
*/
bool 
LARiskConfiguration::isAdjustDf(const LAString &ccy) const
{
	LAString tmpCurrency = ccy;
	return convertBoolFromStr(mpStaticData->getStaticData(tmpCurrency.toLower() + 
													STATIC_DATA_KEY_YIELD_GENERATOR_ISADJUSTDF));
}

/*!
    @brief convert string vector to double rate vector 

	This method suppose argument string vector value is BP value.
	This method convert vector from string to double, 
	and shift from BP to rate value

	@param[in] vec  string vector
	@return DoubleArray
*/
DoubleArray
LARiskConfiguration::convertToRateValues(const LAStringVector &vec) const
{
	const unsigned int size = vec.size();
	DoubleArray ret(size);
	for (unsigned int i = 0; i < size; ++i)
	{
		ret[i] = vec[i].getDoubleValue();
		ret[i] /= 10000.0;
	}
	return ret;
}

/*!
    @brief set fx object parameters 

	@param[in] fx	target fx
	@param[in,out] dataInstance
	@param[in] scenarioNum
	@param[in] index
	@param[out] param
*/
void
LARiskConfiguration::setFXEntityParams(const LAString& ccy, const LAString& fx, LADataInstance& dataInstance, const SCENARIONUM scenarioNum, int index, MAScenarioParam& param) const
{
	param.ccy = /*fx*/ccy;
	LAString shiftType = getShiftType(/*fx*/ccy); shiftType.toUpper();
	param.shiftType = shiftType;
	if (scenarioNum == SCENARIO_BASE)
		param.calcType = getBaseCalcType(/*fx*/ccy, index);
	else
		param.calcType= getCalcType(/*fx*/ccy, scenarioNum, index);
	param.model = LAMarketData::getModelName(fx);
	param.isParallel = true;
	param.isGrid = false;
	// set target name
	LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	param.targetName = targetFX->getName();

	// search shift target currency(foreign currency)
	LAStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);
	const LAStringVector &fx_ccys = targetFX->getCurrencys().get();
	LAStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
	if (it == fx_ccys.end())
	{
		LAString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int pos = static_cast<unsigned int>(it - fx_ccys.begin());
	it = find(fx_ccys.begin(), fx_ccys.end(), ccys[0]);
	if (it == fx_ccys.end())
	{
		LAString msg = "FX object ccy is not registed  ccy = " + ccys[0].toUpper();
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int ccy0pos = static_cast<unsigned int>(it - fx_ccys.begin());
	const DoubleArray& spotrates = targetFX->getSpotRates().get();
	double ccy0ratio = spotrates[ccy0pos];

	// set shift value
	param.paraShiftVec.resize(fx_ccys.size(), 0.0);
	param.paraShiftVec[pos] = getShiftVal(fx, scenarioNum);
	if (shiftType == RISK_SHIFTTYPE_DIFF)
		param.paraShiftVec[pos] *= ccy0ratio;
	else if (shiftType == RISK_SHIFTTYPE_RATIO)
		param.paraShiftVec[pos] /= 100.0;

	//set base shift value
	param.extraBaseParamVec.resize(fx_ccys.size(), 0.0);
	param.extraBaseParamVec[pos] = getBaseSpotVal(/*fx*/ccy,index);
	param.extraBaseParamVec[pos] *= ccy0ratio;

	// set reference curves
	LAString refNameStr = targetFX->getYieldCurves().convertToString().exchange("\"", "");
	param.refName = refNameStr.toToken(MULTI_STATIC_DATA_DELIMITER);
}

/*!
    @brief set fx volatility object parameters 

	@param[in] ccy	yield ccy
	@param[in] fx	calib-target fx
	@param[in,out] dataInstance
	@param[in] scenarioNum
	@param[in] index
	@param[out] param
*/
void
LARiskConfiguration::setFXVolEntityParams(const LAString &ccy, const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index, MAScenarioParam& param, bool isYield) const
{
	LAStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);
	const LAString model = LAMarketData::getModelName(fx);
	const LAString riskName = getRiskName();

	LAString targetCcy = getTargetCurrencies(); //get risk-target fx
	LAString t_targetCcy = targetCcy;
	t_targetCcy.toUpper();
	if (t_targetCcy == "ALL")
	{
		if (isYield)
		{
			targetCcy = ccy;
		}
		else
		{
			targetCcy = fx;
		}
		
	}
	else 
	{
		LAString t_ccy;
		if (isYield)
		{
			t_ccy = ccy;
		}
		else
		{
			t_ccy = fx;
		}
		t_ccy.toUpper();
		if (t_targetCcy.findString(t_ccy) >= 0)
		{
			targetCcy = t_ccy;
		}
	}

	// scenario param
	param.ccy = fx;
	param.isCalib = true;
	if (scenarioNum == SCENARIO_BASE)
		param.calcType = getBaseCalcType(targetCcy, index);
	else if (ccy == AQ_NO_DATA)
		param.calcType = getCalcType(targetCcy, scenarioNum, index);
	else
	{
		param.calcType = getExtraCalcType(targetCcy, scenarioNum, index);
		param.targetCurveType = getCurveType(ccy);
	}
	param.model = model;
	param.targetName = LAMarketData::getBaseVolatilityName(fx);
	param.inputType = LAMarketData::getVolInputType(model, fx, riskName);
	param.isParallel = isParallelShift(targetCcy);
	param.isGrid = isGridSensitivity(targetCcy);

	LAObjectPool &objPool = dataInstance.getObjectPool();
	// set reference
	LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(model);
	LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, fx);
	delete calibInfoCreator;
	param.refName.push_back(infoName);

	// set yield and calibdata
	if (ccy == AQ_NO_DATA)
	{
		for(unsigned int i = 0; i < ccys.size(); ++i)
		{
			LAString ccy_tmp = ccys[i];
			LAString FwdFXGrids = mpStaticData->getStaticData(ccy_tmp.toLower() + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID);
			if(FwdFXGrids.isDefined() && FwdFXGrids != AQ_NO_DATA)
			{
				// extra curve is only available for fxdelta and fxshiftdelta as of Nov 2016
				if(riskName == RISK_FRONT_FX_SHIFTDELTA ||
				   riskName == RISK_FRONT_FX_DELTA || 
				   riskName == RISK_OFFICIAL_FX_DELTA)
				{
					LAString ExtraCalcType = getExtraCalcType(targetCcy, scenarioNum, index);
					param.refName.push_back(LAMarketData::getBaseYieldName(ccys[i]) + "_" + ExtraCalcType);
					param.refName.push_back(LAMarketData::getCalibDataName(ExtraCalcType, LAMarketData::getYieldDataName(objPool, param.refName.back())));
				}
				else
				{
					param.refName.push_back(LAMarketData::getBaseYieldName(ccys[i]));
					param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, param.refName.back())));
				}
			}
			else
			{
				param.refName.push_back(LAMarketData::getBaseYieldName(ccys[i]));
				param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, param.refName.back())));
			}
		}
	}

	// dataout
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		param.isOutPut = true;
	else 
		param.isOutPut = false;

	// set DDL
	LAString fxKey =  LAMarketData::getFXKey(/*targetCcys[0]*/ccys[0], /*targetCcys[1]*/ccys[1]);
	param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
	LAString shiftType = getShiftType(targetCcy); shiftType.toUpper();
	param.shiftType = shiftType;
	if(fx == targetCcy)
	{
		param.extraParam.resize(1, getShiftVal(targetCcy, scenarioNum)); //set bumpval of calib-target fx
	}
	else
	{
		param.extraParam.resize(1, 0);
	}
	if (shiftType == RISK_SHIFTTYPE_RATIO)
		param.extraParam[0] /= 100.0;
	//set base shift value
	if (ccy == AQ_NO_DATA)
	{
		if(fx == targetCcy)
		{
			param.extraBaseParam = getBaseSpotVal(targetCcy,index);  //set base-shiftval of calib-target fx
		}
		else
		{
			param.extraBaseParam = 0;
		}
	}
	else
	{
		LAString baseCcy = getCrossBaseCurrency(ccy); baseCcy.toLower();
		if (baseCcy == fxKey)
			param.extraBaseParam = getBaseSpotVal(ccy,index);
	}
}

/*!
    @brief create yield object 

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfiguration::createOptionAnalyticEntity(const LAString &ccy, LADataInstance &dataInstance,int index)  const
{
	index;
	vector<LAObject *> ret;
	
	LAObjectPool& objPool = dataInstance.getObjectPool();
	const LAString portName = LACoreDataService::getContext(ARG_KEY_MAINTRADE);
	LAObject& eport = objPool.getObject(portName,ENCHKTYPE_ISDEFINED).get();
	bool isfirst = true;
	LADataHolder* dh;
	dh = &(eport.getData(CALIBRATION_DATA_UNDERLYINGS,ISNOTNULL));
	LADataMultiReference& traderef = dynamic_cast<LADataMultiReference &>(dh->get());
	unsigned int tradeSize = traderef.getSize();

	LAString risktypename, premname;
	for (unsigned int i = 0; i < tradeSize; i++)
	{
		LAObject& etrade = traderef.get(i).get();

		dh = &(etrade.getData(CALIBRATION_DATA_VALUE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LADataValuation& value = dynamic_cast<const LADataValuation &>(dh->get());
			
			function_t valuetype = value.getType();
			if (valuetype == FN_FXOPTIONVALUE || 
				valuetype == FN_FXDIGITALOPTIONVALUE ||
				valuetype == FN_FXDIGITALCALLSPREADOPTIONVALUE ||
				valuetype == FN_FXSINGLEBARRIEROPTIONVALUE)
			{
				if (isfirst)
				{
					premname = "PREM";
					premname.toUpper();

					LAObjectHolder objHolder = objPool.getObject(premname, ENCHKTYPE_NOCHECK);
					if (!objHolder.isDefined())
					{
						LAObject* e = new LAObject();
						e->add(CALIBRATION_DATA_NAME, new LADataString(premname));
						objPool.set(premname, e);
					}

					risktypename = getAnalyticalRiskType(ccy,etrade);
					risktypename += "_" + ccy;
					risktypename.toUpper();
					objHolder = objPool.getObject(risktypename, ENCHKTYPE_NOCHECK);
					if (!objHolder.isDefined())
					{
						LAObject* e = new LAObject();
						e->add(CALIBRATION_DATA_NAME, new LADataString(risktypename));
						objPool.set(risktypename, e);
						ret.push_back(e);
					}
					else
					{
						ret.push_back(&objHolder.get());
					}

					//set diff or ratio flag
					
					LAString tmpcur = ccy;
					LAString shiftstr = getShiftType(ccy);
					shiftstr.toUpper();
					if (shiftstr != RISK_SHIFTTYPE_RATIO && shiftstr != RISK_SHIFTTYPE_DIFF)
						throw LACoreInvalidData("RiskShiftType Error",__FILE__,__LINE__);

					bool isdiff = (shiftstr != RISK_SHIFTTYPE_RATIO);
					ret[0]->remove(PRICING_DATA_ISDIFFFORRISK);
					ret[0]->add(PRICING_DATA_ISDIFFFORRISK, new LADataBool(isdiff));
					
					//set shift val
					double shiftval = getShiftValForOptionAnalytic(tmpcur);
					ret[0]->remove(PRICING_DATA_SHIFTVALFORRISK);
					ret[0]->add(PRICING_DATA_SHIFTVALFORRISK, new LADataDouble(shiftval));
				
					isfirst = false;
				}
				
				etrade.remove(PRICING_DATA_ANALYTICRISKTYPE);
				etrade.add(PRICING_DATA_ANALYTICRISKTYPE, new LADataReference()).convertFromString(premname);

				//input the flag
				etrade.remove(risktypename);
				etrade.add(risktypename, new LADataBool(IsSucceedAnalyticalRiskType(ccy,etrade)));
				
			}
		}
	}

	
	return ret;
}

/*!
    @brief create risk object

	@param [in] objPool
	@return vector<pair<LAString, vector<LAObject *> > >
*/
vector<pair<LAString, vector<LAObject *> > >
LARiskConfiguration::createOptionAnalyticRiskEntity(LAObjectPool &objPool) const
{
	vector<pair<LAString, vector<LAObject *> > > ret = LARiskConfiguration::createRiskEntity(objPool);
	unsigned int vecSize = ret.size();
	for (unsigned int i = 0; i < vecSize; i++)
	{
		vector<LAObject *> evec = ret[i].second;
		unsigned int evecSize = evec.size();
		for (unsigned int j = 0; j < evecSize; j++)
		{
			LAObject* e = evec[j];
			if (!e)
			{
				continue;
			}
			LAString name = dynamic_cast<LADataString &>(e->getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
			//for analyticalrisk we must set befor SetUpTargetNames
			objPool.set(name,e);
		}
	}
	return ret;
}

/*!
    @brief setup targetNames

	@param[in] ccy
	@param[out] e
*/
void
LARiskConfiguration::setUpOptionAnalyticTargetNames(const LAString &ccy, LAObject &e) const
{
	(void)ccy;
	LAString ret(PREM);
	e.remove(PRICING_DATA_TARGETNAMES);
	e.add(PRICING_DATA_TARGETNAMES, new LADataStrings()).convertFromString(ret);
	
}


/*!
    @brief get Analytic RiskType

	@param[in] ccy
	@param[out] e
*/
LAString
LARiskConfiguration::getAnalyticalRiskType(const LAString& fx, LAObject& e) const
{
	(void)fx;(void)e;
	return mAnalyticRiskType;
}

// IsSucceedAnalyticalRiskType
bool 
LARiskConfiguration::IsSucceedAnalyticalRiskType(const LAString& fx, LAObject& e) const
{
	LAString ret;
	LAString fxcur = fx;
	fxcur.toUpper();

	LADataHolder* dh = &(e.getData(PRICING_DATA_DOMESTICCURRENCY, ISNOTNULL));
	LAString domcur = dynamic_cast<LADataString &>(dh->get()).get();
	domcur.toUpper();

	dh = &(e.getData(PRICING_DATA_FOREIGNCURRENCY, ISNOTNULL));
	LAString forcur = dynamic_cast<LADataString &>(dh->get()).get();
	forcur.toUpper();

	if (fxcur.findString(domcur) >=0 && fxcur.findString(forcur) >= 0)
		return true;
	else
	{
		return false;
	}
	
}

LAStringVector
LARiskConfiguration::getRiskCurrencys(LAObjectPool& objPool) const
{
	objPool;
	return MADealUtils::getSDECurrencys();
}

void 
LARiskConfiguration::setUpRiskOutputCurrency(const LAString &ccy, LAObject &e) const
{
	bool isriskcurrencymode = isRiskCurrencyMode(ccy);
	if (isriskcurrencymode)
	{
		LAString ret;
		if (-1 != ccy.findString(FX_DELIMITER))
		{
			ret = LAMarketData::getBaseCurrencyOfFXPair(ccy);
		}
		else
		{
			ret = ccy;
		}
		
		e.remove(PRICING_DATA_RISKOUTPUTCURRENCY);
		e.add(PRICING_DATA_RISKOUTPUTCURRENCY, new LADataString(ret.toUpper()));
	}

	return;
}

void 
LARiskConfiguration::setUpIsAnalytic(const LAString &ccy, LAObject &e) const
{
	e.remove(PRICING_DATA_ANALYTICCALCTYPE);

	bool isanalyticalrisk = isAnalyticMode(ccy);
	if (isanalyticalrisk)
	{
		e.add(PRICING_DATA_ANALYTICCALCTYPE, new LADataString(mAnalyticCalcType));

		//set analytic info bumpdirection, shifttype, isparallel, isgridsensitivity, divunit, 
		DoubleArray gridshiftvals = getScenario1GridShift(ccy);
		if (gridshiftvals.size() < 1)
			throw LACoreInvalidData("Analytic Set Error",__FILE__,__LINE__);

		e.add(PRICING_DATA_SHIFTVALS_ANALYTIC, new LADataDoubles(gridshiftvals));

		LAString shifttype = getShiftType(ccy).toUpper();
		e.add(PRICING_DATA_SHIFTTYPE_ANALYTIC, new LADataString(shifttype));

		double divunit = getDivUnit(ccy);
		e.add(PRICING_DATA_DIVUNIT_ANALYTIC, new LADataDouble(divunit));

		bool isgrid = isGridSensitivity(ccy);
		e.add(PRICING_DATA_ISGRIDSENSITIVITY_ANALYTIC, new LADataBool(isgrid));

	}
	else 
	{
		//in case of semianalytic we must modify here
		e.add(PRICING_DATA_ANALYTICCALCTYPE, new LADataString("NONE"));
	}

	return;
}

void 
LARiskConfiguration::setUpDeltaType(const LAString &ccy, LAObject &e) const
{
	e.remove(AP_CALIBRATION_DATA_DELTATYPE);

	LAString dtype = getDeltaType(ccy).toUpper();
	if (dtype != AQ_NO_DATA)
	{
		e.add(AP_CALIBRATION_DATA_DELTATYPE, new LADataString(dtype));
	}

	return;
}

void
LARiskConfiguration::setUpVegaType(const LAString &ccy, LAObject &e) const
{
	e.remove(AP_CALIBRATION_DATA_VEGATYPE);

	LAString vtype = getVegaType(ccy).toUpper();
	if (vtype != AQ_NO_DATA)
	{
		e.add(AP_CALIBRATION_DATA_VEGATYPE, new LADataString(vtype));
	}

	return;
}

// getScenarioBaseYieldName
LAString 
LARiskConfiguration::getScenarioBaseYieldName(const LAString& ccy) const
{
	return LAMarketData::getBaseYieldName(ccy);
}


// create foreign yield object in base scenario 
/*!
    @brief create foreign yield object 

	@param[in] ccy
	@param[in] fCcy foreign currency
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfiguration::createBaseForeignYieldEntity(const LAString &ccy, const LAString &fCcy, LADataInstance &dataInstance, int index, const bool isFirst)  const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	const LAString model = LAMarketData::getModelName(fCcy);
	const LAString riskName = getRiskName();

	// set up param
	MAScenarioParam param;
	param.ccy = fCcy;
	param.model = model;
	param.calcType = getBaseExtraCalcType(ccy, index);
	param.targetName = LAMarketData::getBaseYieldName(fCcy);
	param.targetCurveType = getBaseShiftCurveType(ccy);
	param.isAdjustDF = isAdjustDf(fCcy);
	param.isForeignCcy = true;
	param.isFirst = isFirst;

	//get domestic yield data name and calcType
	LAMathYieldCurvePro &ycPro_fCcy = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(fCcy), ENCHKTYPE_ISDEFINED).get());
	const LAString& affectedCcy = ycPro_fCcy.getAffectedCcy();
	const LAString& baseYieldDataName_baseccy = LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(affectedCcy));

	LAString calcType = param.calcType;
	if (affectedCcy == ccy)
		calcType = getBaseCalcType(ccy, index);
	
	param.isParallel = true;
	LAString yieldDataName_baseccy =  baseYieldDataName_baseccy + "_" + calcType + "_" + param.targetCurveType + "_Parallel";
	param.refName.push_back(yieldDataName_baseccy);

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

// create collateral yield object in base scenario 
/*!
    @brief create collateral yield object 

	@param[in] ccy
	@param[in] colCcy collateral currency
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfiguration::createBaseCollateralYieldEntity(const LAString &ccy, const LAString &colCcy, LADataInstance &dataInstance, int index)  const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	const LAString model = LAMarketData::getModelName(colCcy);
	const LAString riskName = getRiskName();

	// set up param
	MAScenarioParam param;
	param.ccy = colCcy;
	param.model = model;
	if (ccy == colCcy)
		param.calcType = getBaseCalcType(ccy, index);
	else
		param.calcType = getBaseExtraCalcType(ccy, index);
	param.targetName = LAMarketData::getBaseYieldName(colCcy);
	param.targetCurveType = getBaseShiftCurveType(ccy);
	param.isAdjustDF = isAdjustDf(colCcy);
	param.isCollateralCcy = true;

	//get domestic yield data name and calcType
	LAMathYieldCurvePro &ycPro_colCcy = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(colCcy), ENCHKTYPE_ISDEFINED).get());
	const LAString& affectedCcy = ycPro_colCcy.getColAffectedCcy();
	const LAString& baseYieldDataName_baseccy = LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(affectedCcy));

	LAString calcType;
	if (affectedCcy == ccy) 
		calcType = getBaseCalcType(ccy, index);
	else
		calcType = getBaseExtraCalcType(ccy, index);
	
	param.isParallel = true;
	LAString yieldDataName_baseccy =  baseYieldDataName_baseccy + "_" + calcType + "_" + param.targetCurveType + "_Parallel";
	param.refName.push_back(yieldDataName_baseccy);

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief create foreign ir vol scenarilo object

	@param[in] ccy
	@param[in] fCcy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *>
*/
vector<LAObject *> 
LARiskConfiguration::createBaseForeignIRVolEntity(const LAString& ccy, const LAString& fCcy, LADataInstance &dataInstance, int index) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();
	const LAString model = LAMarketData::getModelName(fCcy);
	const LAString riskName = getRiskName();

	// set up param for volatility shift
	MAScenarioParam param;
	param.isCalib = true;
	param.ccy = fCcy;
	param.calcType = getBaseExtraCalcType(ccy, index) ;
	param.model = model;
	param.inputType = LAMarketData::getVolInputType(model, fCcy, riskName);
	param.targetName = LAMarketData::getBaseVolatilityName(fCcy);
	param.targetCurveType = getBaseShiftCurveType(ccy);

	// dataout
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		param.isOutPut = true;
	}
	else 
	{
		param.isOutPut = false;
	}

	LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
	LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, fCcy);
	delete calibInfoCreator;

	param.refName.push_back(infoName);
	param.isParallel = true;
	const LAString& yieldName = LAMarketData::getBaseYieldName(fCcy);
	param.refName.push_back(yieldName + "_" + param.calcType + "_" + param.targetCurveType + "_Parallel");

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);
	vector<LAObject *> sce = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	return sce;
}

/*!
    @brief return ir shift extra target names

	@param[in] ccy
	@return LAStringVector
*/
LAStringVector 
LARiskConfiguration::getIRBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAStringVector ret(0);

	const LAString ircur = getIRBaseCurrency(ccy);

	LAObjectPool &objPool = dataInstance.getObjectPool();
	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(ircur), ENCHKTYPE_ISDEFINED).get());

	// get affecting ccys
	const LAStringVector &fCurveCcys = ycPro.getAffectingCcy();
	// get col affecting ccys
	StringSet cCurveCcys;
	LAStringVector tmpCcys = ycPro.getColAffectingCcy();
	cCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	for (int i = 0; i < fCurveCcys.size(); ++i)
	{
		const LAMathYieldCurvePro &fYcPro = dynamic_cast<LAMathYieldCurvePro &>
							(objPool.getObject(LAMarketData::getBaseYieldProName(fCurveCcys[i]), ENCHKTYPE_ISDEFINED).get());
		tmpCcys = fYcPro.getColAffectingCcy();
		cCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	}
	// get affecting ccys of col affecting ccys
	StringSet cfCurveCcys;
	for (StringSet::const_iterator it = cCurveCcys.begin(); it != cCurveCcys.end(); ++it)
	{
		const LAMathYieldCurvePro &cYcPro = dynamic_cast<LAMathYieldCurvePro &>
									(objPool.getObject(LAMarketData::getBaseYieldProName(*it), ENCHKTYPE_ISDEFINED).get());
		tmpCcys = cYcPro.getAffectingCcy();
		cfCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	}
	cfCurveCcys.erase(ircur);
	// get all affecting ccys
	StringSet curveCcys;
	curveCcys.insert(fCurveCcys.begin(), fCurveCcys.end());
	curveCcys.insert(cCurveCcys.begin(), cCurveCcys.end());
	curveCcys.insert(cfCurveCcys.begin(), cfCurveCcys.end());
	curveCcys.erase(ircur);

	for (StringSet::const_iterator it = curveCcys.begin(); it != curveCcys.end(); ++it)
	{
		ret.push_back(LAMarketData::getBaseYieldName(*it));
	}
	for (StringSet::const_iterator it = curveCcys.begin(); it != curveCcys.end(); ++it)
	{
		// check calibration target
		if (isCalibTarget(*it))
		{
			ret.push_back(LAMarketData::getBaseVolatilityName(*it));
		}
	}
	// check calibration target
	if (isCalibTarget(ircur))
	{
		ret.push_back(LAMarketData::getBaseVolatilityName(ircur));
	}
	LAStringVector targetFXVec = getCalibTargetFX(ircur, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		// if cross check fx is target
		LAStringVector ccys = targetFXVec[i].toToken(FX_DELIMITER);
		LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
		ret.push_back(LAMarketData::getBaseVolatilityName(key_fx));
	}
	return ret;
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return LAStringVector
*/
vector<LAObject *>
LARiskConfiguration::createIRBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	LAObjectPool &objPool = dataInstance.getObjectPool();

	const LAString ircur = getIRBaseCurrency(ccy);

	vector<LAObject *> ret;
	LAString bShiftStr = getBaseShiftStr(ccy, index); // base shirt value(string)

	LAMathYieldCurvePro &ycPro = dynamic_cast<LAMathYieldCurvePro &>
					(objPool.getObject(LAMarketData::getBaseYieldProName(ircur), ENCHKTYPE_ISDEFINED).get());

	map<LAString, vector<LAObject *> > scemap;
	vector<LAObject *> sce;

	// get affecting ccys
	const LAStringVector& fCurveCcys = ycPro.getAffectingCcy();
	// get col affecting ccys
	StringSet cCurveCcys;
	StringSet cfCurveCcys;
	LAStringVector tmpCcys = ycPro.getColAffectingCcy();
	cCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	for (int unsigned i = 0; i < fCurveCcys.size(); ++i)
	{
		// calc affecting curves
		sce = createBaseForeignYieldEntity(ircur, fCurveCcys[i], dataInstance, index);
		if (sce.size() > 0)
			scemap[fCurveCcys[i]] = sce;
		else
			cfCurveCcys.insert(fCurveCcys[i]);
		// get col affecting ccys
		const LAMathYieldCurvePro &fYcPro = dynamic_cast<LAMathYieldCurvePro &>
						(objPool.getObject(LAMarketData::getBaseYieldProName(fCurveCcys[i]), ENCHKTYPE_ISDEFINED).get());
		tmpCcys = fYcPro.getColAffectingCcy();
		cCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	}
	for (StringSet::const_iterator it = cCurveCcys.begin(); it != cCurveCcys.end(); ++it)
	{
		// calc affecting fwdfx const curves
		sce = createBaseCollateralYieldEntity(ircur, *it, dataInstance, index);
		if (sce.size() > 0)
			scemap[*it] = sce;
		// get affecting ccys of col affecting ccys
		const LAMathYieldCurvePro &cYcPro = dynamic_cast<LAMathYieldCurvePro &>
									(objPool.getObject(LAMarketData::getBaseYieldProName(*it), ENCHKTYPE_ISDEFINED).get());
		tmpCcys = cYcPro.getAffectingCcy();
		cfCurveCcys.insert(tmpCcys.begin(), tmpCcys.end());
	}
	cfCurveCcys.erase(ircur);
	const LAString usd = "USD";
	if (cfCurveCcys.find(usd) != cfCurveCcys.end())
	{
		// calc usd curves first
		sce = createBaseForeignYieldEntity(ircur, usd, dataInstance, index, false);
		if (sce.size() > 0)
			scemap[usd] = sce;
	}
	for (StringSet::const_iterator it = cfCurveCcys.begin(); it != cfCurveCcys.end(); ++it)
	{
		if (*it == usd)
			continue;
		// calc affecting curves of fwdfx const curves
		sce = createBaseForeignYieldEntity(ircur, *it, dataInstance, index, false);
		if (sce.size() > 0)
			scemap[*it] = sce;
	}
	// get all affecting ccys
	StringSet curveCcys;
	curveCcys.insert(fCurveCcys.begin(), fCurveCcys.end());
	curveCcys.insert(cCurveCcys.begin(), cCurveCcys.end());
	curveCcys.insert(cfCurveCcys.begin(), cfCurveCcys.end());
	curveCcys.erase(ircur);

	// set in object pool
	for (map<LAString, vector<LAObject *> >::const_iterator it = scemap.begin(); it != scemap.end(); ++it)
	{
		sce = it->second;
		ret.insert(ret.end(), sce.begin(), sce.end());
		for (int unsigned j = 0; j < sce.size(); ++j)
		{
			const LAString &name = dynamic_cast<const LADataString &>(sce[j]->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
			sce[j]->remove(AP_CALIBRATION_DATA_ISRISKENTITY);
			sce[j]->add(AP_CALIBRATION_DATA_ISRISKENTITY, new LADataBool(true));
			LAObjectHolder objHolder = objPool.getObject(name,  ENCHKTYPE_NOCHECK);
			if (!objHolder.isDefined())
				objPool.set(name, sce[j]);
		}
	}
	scemap.clear();

	for (set<LAString>::const_iterator it = curveCcys.begin(); it != curveCcys.end(); ++it)
	{
		if (isCalibTarget(*it))
		{
			sce = createBaseForeignIRVolEntity(ircur, *it, dataInstance, index);
			ret.insert(ret.end(), sce.begin(), sce.end());
		}
	}

	const LAString model = LAMarketData::getModelName(ircur);
	const LAString riskName = getRiskName();

	if (isCalibTarget(ircur))
	{
		// set up param for volatility shift
		MAScenarioParam param;
		param.ccy = ircur;
		param.calcType = getBaseExtraCalcType(ircur, index);
		param.model = model;
		param.inputType = LAMarketData::getVolInputType(model, ircur, riskName);
		param.targetName = LAMarketData::getBaseVolatilityName(ircur);
		param.targetCurveType = getCurveType(ircur);

		param.isParallel = true;
		if (isRealCalib())
		{
			param.isCalib = true;
			// dataout
			if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
			{
				param.isOutPut = true;
			}
			else 
			{
				param.isOutPut = false;
			}
			LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
			LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, ircur);
			delete calibInfoCreator;

			param.refName.push_back(infoName);

		}
		else
		{
			param.isCalib = false;
			LAMarketData::getVolFuncFilePath(model, ircur, ircur, riskName, SCENARIO_1, param.paraFile, false, 0, &bShiftStr, true);
		}

		if (mBaseSceNames.size() < 1)
		{
			throw LACoreInvalidData("Base scenario is not set.", __FILE__, __LINE__);
		}
		param.refName.push_back(mBaseSceNames[0]);
		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<LAObject *> sce = sceCreator->createScenario(dataInstance, param);
		delete sceCreator;

		ret.insert(ret.end(), sce.begin(), sce.end());
	}

	LAStringVector targetFXVec = getCalibTargetFX(ircur, dataInstance);
	unsigned int fxSize = targetFXVec.size();
	for (unsigned int i = 0; i < fxSize; ++i)
	{
		LAObjectPool &objPool = dataInstance.getObjectPool();

		LAStringVector ccys = targetFXVec[i].toToken(FX_DELIMITER);
		LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
		const LAString model_fx = LAMarketData::getModelName(key_fx);
		// set up param
		MAScenarioParam param;
		param.ccy = key_fx;
		param.calcType = getBaseExtraCalcType(ircur, index);
		param.targetCurveType = getCurveType(ircur);
		param.model = model_fx;
		param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(key_fx + STATIC_DATA_FX_KEY_SDE_ISDD));
		// set file path dmy
		param.paraFile.push_back(CALIB_DMY_FILE);
		// set shift value and type
		LAString shiftType = getShiftType(key_fx);
		shiftType.toUpper();
		param.shiftType = shiftType;
		param.inputType = LAMarketData::getVolInputType(model_fx, key_fx, riskName);
		param.targetName = LAMarketData::getBaseVolatilityName(key_fx);
		param.isParallel = true;
		ccys[0].toUpper();
		LAString tmpCurrency = ircur;
		tmpCurrency.toUpper();

		if (isRealCalib())
		{
			param.isCalib = true;
			// dataout
			if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
			{
				param.isOutPut = true;
			}
			else 
			{
				param.isOutPut = false;
			}
			LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model);
			LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, key_fx);
			delete calibInfoCreator;
			param.refName.push_back(infoName);
			// set dmy
			param.paraFile.push_back(CALIB_DMY_FILE);
			// set reference
			const LAString suffix = "_" + param.calcType + "_" + param.targetCurveType + "_Parallel";
			setScenarioParamReferences(tmpCurrency, ccys, fCurveCcys, objPool, param.refName, mBaseSceNames[0], param.calcType, /*&param.targetCurveType, */suffix, -1);
		}
		else
		{
			param.isCalib = false;
			LAMarketData::getVolFuncFilePath(model_fx, ircur, key_fx, riskName, SCENARIO_1, param.paraFile, false, 0, &bShiftStr, true);
			// set reference
			if (ccys[0] == tmpCurrency)
			{
				// set domestic curve name
				param.refName.push_back(mBaseSceNames[0]);
				// set foreign curve name
				param.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
			}
			else
			{
				// set domestic curve name
				param.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
				// set foreign curve name
				param.refName.push_back(mBaseSceNames[0]);
			}
		}

		// create scenario
		LAScenarioConfiguration *sceCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<LAObject *> sce_fx = sceCreator->createScenario(dataInstance, param);
		delete sceCreator;

		ret.insert(ret.end(), sce_fx.begin(), sce_fx.end());

	}

	return ret;
}

/*!
    @brief set up base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfiguration::createIRBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	const LAString ircur = getIRBaseCurrency(ccy);
	const LAString model = LAMarketData::getModelName(ircur);
	const LAString riskName = getRiskName();
	// get base shift and grid
	LAString tmpCurrency = ccy;
	tmpCurrency.toLower();
	DoubleArray baseShifts = getBaseShifts(ccy, index);
	LAStringVector grid = getShiftGridTerm(tmpCurrency);
	
	LAString tmpIRCurrency = ircur;
	tmpIRCurrency.toLower();
	LAString oismethod = mpStaticData->getStaticData(tmpIRCurrency + STATIC_DATA_KEY_YIELD_OIS_GENERATEMETHOD + getBaseShiftCurveSuffix(ircur)).toUpper();
	bool isoismode  = (oismethod != AQ_NO_DATA);
	if (!isoismode)
	{
		// ON and TN check
		double firstVal = baseShifts[0];
		if (grid.end() == find(grid.begin(), grid.end(), "ON"))
		{
			baseShifts.insert(baseShifts.begin(), firstVal);
			grid.insert(grid.begin(), "ON");
		}
		if (grid.end() == find(grid.begin(), grid.end(), "TN"))
		{
			baseShifts.insert(++baseShifts.begin(), firstVal);
			grid.insert(++grid.begin(), "TN");
		}
	}
	const unsigned int gridSize = grid.size();
	if (gridSize != baseShifts.size())
	{
		throw LACoreInvalidData("Base shift grid is different from curve grid", __FILE__, __LINE__);
	}
	// convert grid for M-Lib
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		grid[i] = LAMarketData::convertToMLibTerm(grid[i]);
	}

	// set param
	MAScenarioParam param;
	param.ccy = ircur;
	param.calcType= getBaseCalcType(ircur, index);
	param.model = model;
	param.shiftType = RISK_SHIFTTYPE_DIFF;
	param.bumpDirection = RISK_BUMPDIRECTION_UPSHIFT;
	param.targetName = LAMarketData::getBaseYieldName(ircur);
	param.targetCurveType = getBaseShiftCurveType(ircur);
	// set parallel
	param.isParallel = true;
	param.paraTerm = grid;
	param.paraShiftVec = baseShifts;
	param.isAdjustDF = isAdjustDf(ircur);

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELD);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief return grid term

	@param[in] ccy
	@param[in] index
	@return DoubleVector
*/
DoubleArray 
LARiskConfiguration::getBaseShifts(const LAString &ccy, int index) const
{
	unsigned int numGrid = getGridTerm(ccy).size(); 
	return DoubleVector(numGrid, 0.);
}

/*!
    @brief return baseshiftvals

	@param[in] ccy
	@param[in] index
	@return DoubleArray
*/
DoubleArray
LARiskConfiguration::getIRBaseShifts(const LAString &ccy, int index) const
{
	DoubleArray shiftVals = getIRShiftVals(ccy);
	const unsigned int shiftSize = shiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("getBaseShifts index is less than zero or over shift grid.", __FILE__, __LINE__);
	}

	return DoubleArray(getShiftGridTerm(ccy).size(), shiftVals[index]);	
}

/*!
    @brief return baseshiftstr

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString
LARiskConfiguration::getIRBaseShiftStr(const LAString &ccy , int index) const
{
	DoubleArray irShiftVals = getIRShiftVals(ccy);
	const unsigned int shiftSize = irShiftVals.size();
	if (index < 0 || index >= static_cast<int>(shiftSize))
	{
		throw LACoreInvalidData("getOutPutName1 index is less than zero or over shift grid.", __FILE__, __LINE__);
	}
	return LAString(irShiftVals[index] * 10000.0, 0);
}

/*!
    @brief set up basis base scenario

	@param[in] ccy
	@param[in] e
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfiguration::createBasisBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	const LAString ircur = getIRBaseCurrency(ccy);
	const LAString model = LAMarketData::getModelName(ircur);
	const LAString riskName = getRiskName();
	// get base shift and grid
	LAString tmpCurrency = ccy;
	tmpCurrency.toLower();
	DoubleArray baseShifts = getBaseShifts(ccy, index);
	LAStringVector grid = getShiftGridTerm(tmpCurrency);
	
	const unsigned int gridSize = grid.size();
	if (gridSize != baseShifts.size())
	{
		throw LACoreInvalidData("Base shift grid is different from curve grid", __FILE__, __LINE__);
	}
	// convert grid for M-Lib
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		grid[i] = LAMarketData::convertToMLibTerm(grid[i]);
	}

	// set param
	MAScenarioParam param;
	param.ccy = ircur;
	param.calcType= getBaseCalcType(ircur, index);
	param.model = model;
	param.shiftType = RISK_SHIFTTYPE_DIFF;
	param.bumpDirection = RISK_BUMPDIRECTION_UPSHIFT;
	param.targetName = LAMarketData::getBaseYieldName(ircur);
	param.targetCurveType = getBaseShiftCurveType(ircur);
	// set parallel
	param.isParallel = true;
	param.paraTerm = grid;
	param.paraShiftVec = baseShifts;
	param.isAdjustDF = isAdjustDf(ircur);

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_YIELDBASIS);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief return base calc type

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString 
LARiskConfiguration::getCalcType(const LAString &ccy, SCENARIONUM scenarioNum, int index) const
{
	const LAString riskName = getRiskName();
	const LAString riskNameSuffix = scenarioNum == SCENARIO_1 ? PRICING_DATA_SCENARIO1 : PRICING_DATA_SCENARIO2 ;
	return ccy + "_" + riskName + "_" + riskNameSuffix + "_" + LAString(index);
}

/*!
    @brief return base extra calc type

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString 
LARiskConfiguration::getExtraCalcType(const LAString &ccy, SCENARIONUM scenarioNum, int index) const
{
	const LAString riskName = getRiskName();
	const LAString riskNameSuffix = scenarioNum == SCENARIO_1 ? PRICING_DATA_EXTRASCENARIO1 : PRICING_DATA_EXTRASCENARIO2 ;
	return ccy + "_" + riskName + "_" + riskNameSuffix + "_" + LAString(index);
}

/*!
    @brief get baseoperator

	@param[in] ccy
*/
LAString
LARiskConfiguration::getBaseOperator(void) const
{ 
	return FN_LINEAR_STR;
}	

/*!
    @brief get basecoefficient

	@param[in] ccy
*/
LAString  
LARiskConfiguration::getBaseCoefficient(const LAString &ccy) const
{
	(void)ccy;
	return LAString("0.0:1.0:0.0");
}


/*!
    @brief return base calc type

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString 
LARiskConfiguration::getBaseCalcType(const LAString &ccy, int index) const
{
	const LAString riskName = getRiskName();
	return ccy + "_" + riskName + "_" + PRICING_DATA_BASESCENARIO + "_" + LAString(index);
}

/*!
    @brief return base extra calc type

	@param[in] ccy
	@param[in] index
	@return LAString
*/
LAString 
LARiskConfiguration::getBaseExtraCalcType(const LAString &ccy, int index) const
{
	const LAString riskName = getRiskName();
	return ccy + "_" + riskName + "_" + PRICING_DATA_BASEEXTRASCENARIO + "_" + LAString(index);
}

/*!
    @brief create fx object

	@param[in] ccy
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfiguration::createBaseFXEntity(const LAString &ccy, LADataInstance &dataInstance, int index)  const
{
	LAString fx = getCrossBaseCurrency(ccy);
	if (fx == AQ_NO_DATA) 
		fx = ccy;

	// scenario param
	MAScenarioParam param;
	setFXEntityParams(ccy, fx, dataInstance, SCENARIO_BASE, index, param);
	//param.ccy = fx;
	//param.shiftType = RISK_BUMPDIRECTION_UPSHIFT;
	//param.calcType= getBaseCalcType(fx, index);
	//param.model = LAMarketData::getModelName(fx);
	//param.isParallel = true;
	//param.isGrid = false;
	//// set target name
	//LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	//param.targetName = targetFX->getName();

	//// search shift target currency(foreign currency)
	//LAStringVector ccys;
	//LAMarketData::convertToCurrency(fx, ccys);
	//const LAStringVector &fx_ccys = targetFX->getCurrencys().get();
	//LAStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
	//if (it == fx_ccys.end())
	//{
	//	LAString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
	//	throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	//}
	//unsigned int pos = static_cast<unsigned int>(it - fx_ccys.begin());
	//it = find(fx_ccys.begin(), fx_ccys.end(), ccys[0]);
	//if (it == fx_ccys.end())
	//{
	//	LAString msg = "FX object ccy is not registed  ccy = " + ccys[0].toUpper();
	//	throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	//}
	//unsigned int ccy0pos = static_cast<unsigned int>(it - fx_ccys.begin());
	//const DoubleArray& spotrates = targetFX->getSpotRates().get();
	//double ccy0ratio = spotrates[ccy0pos];

	//// set shift value
	//param.paraShiftVec.resize(fx_ccys.size(), 0.0);
	//
	////set base shift value
	//param.extraBaseParamVec.resize(fx_ccys.size(), 0.0);
	//param.extraBaseParamVec[pos] = getBaseSpotVal(ccy, index);
	//param.extraBaseParamVec[pos] *= ccy0ratio;
	//
	//// set reference
	//LAString refNameStr = targetFX->getData(IR_MODEL_DATA_YIELDCURVES, ISNOTNULL).convertToString().exchange("\"", "");
	//param.refName = refNameStr.toToken(MULTI_STATIC_DATA_DELIMITER);

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_FX);

	vector<LAObject *> ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;
	return ret;
}

/*!
    @brief create fx vol object

	@param[in] fx
	@param[in,out] dataInstance
	@param[in] index
	@return vector<LAObject *> 
*/
vector<LAObject *> 
LARiskConfiguration::createBaseFXVolEntity(const LAString &fx, LADataInstance &dataInstance, int index)  const
{
	//create base volatility
	LAStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);
	const LAString model = LAMarketData::getModelName(fx);
	const LAString riskName = getRiskName();
	const LAString targetCcy = getTargetCurrencies();
	// scenario param
	MAScenarioParam paramvol;
	paramvol.ccy = fx;
	paramvol.isCalib = true;
	paramvol.calcType = getBaseCalcType(fx, index);
	paramvol.model = model;
	paramvol.targetName = LAMarketData::getBaseVolatilityName(fx);
	paramvol.inputType = LAMarketData::getVolInputType(model, fx, riskName);
	paramvol.isParallel = true;
	paramvol.isGrid = false;

	LAObjectPool &objPool = dataInstance.getObjectPool();
	// set reference
	LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(paramvol.model);
	LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, fx);
	delete calibInfoCreator;
	paramvol.refName.push_back(infoName);

	// set yield and calibdata
	for(unsigned int i = 0; i < ccys.size(); ++i)
	{
		LAString ccy_tmp = ccys[i];
		LAString FwdFXGrids = mpStaticData->getStaticData(ccy_tmp.toLower() + STATIC_DATA_KEY_YIELD_BASIS_FWDFX_USEGRID);
		if(FwdFXGrids.isDefined() && FwdFXGrids != AQ_NO_DATA)
		{
			// extra curve is only available for fxshiftdelta as of Nov 2016
			if(riskName == RISK_FRONT_FX_SHIFTDELTA)
			{
				const LAString BaseExtraCalcType = getBaseExtraCalcType(targetCcy, index);
				paramvol.refName.push_back(LAMarketData::getBaseYieldName(ccys[i]) + "_" + BaseExtraCalcType);
				paramvol.refName.push_back(LAMarketData::getCalibDataName(BaseExtraCalcType, LAMarketData::getYieldDataName(objPool, paramvol.refName.back())));
			}
			else
			{
				paramvol.refName.push_back(LAMarketData::getBaseYieldName(ccys[i]));
				paramvol.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, paramvol.refName.back())));
			}
		}
		else
		{
			paramvol.refName.push_back(LAMarketData::getBaseYieldName(ccys[i]));
			paramvol.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, paramvol.refName.back())));
		}
	}

	// dataout
	if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
	{
		paramvol.isOutPut = true;
	}
	else 
	{
		paramvol.isOutPut = false;
	}

	// set DDL
	LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
	paramvol.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
	// set file path dmy
	paramvol.paraFile.push_back(CALIB_DMY_FILE);
	// set shift value and type
	paramvol.shiftType = RISK_BUMPDIRECTION_UPSHIFT;
	if(fx == targetCcy)
	{
		paramvol.extraBaseParam = getBaseSpotVal(targetCcy, index);
	}
	else
	{
		paramvol.extraBaseParam = 0;
	}
	// create scenario
	LAScenarioConfiguration *sceVolCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

	vector<LAObject *> ret = sceVolCreator->createScenario(dataInstance, paramvol);
	delete sceVolCreator;
	return ret;
}

/*!
    @brief set up fx base scenario

	@param[in] ccy
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfiguration::createFXBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	vector<LAObject *> ret;

	// create fx object
	ret = LARiskConfiguration::createBaseFXEntity(ccy, dataInstance, index);

	LAString fx = getCrossBaseCurrency(ccy);
	if (fx == AQ_NO_DATA) 
		fx = ccy;

	// nocalib flag
	bool isCalib = false;
	if (ccy.findString(FX_DELIMITER) > 0 && isCalibTarget(ccy)) 
		isCalib = true;
	else
	{
		LAStringVector calibfxvec = getCalibTargetFX(ccy, dataInstance);
		unsigned int size = calibfxvec.size();
		if (size != 0 && std::find(calibfxvec.begin(), calibfxvec.end(), fx) != calibfxvec.end())
			isCalib = true;
	}
	// fx vol calibration has been replaced into extra scenario
	//if (isCalib)
	//{
	//	vector<LAObject *> base_vol = LARiskConfiguration::createBaseFXVolEntity(fx, dataInstance, index);
	//	ret.insert(ret.end(), base_vol.begin(), base_vol.end());
	//}

	return ret;
}

/*!
    @brief set up fx base extra scenario

	@param[in] ccy
	@param[in] dataInstance
	@param[in] index
*/
vector<LAObject *>
LARiskConfiguration::createFXBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const
{
	vector<LAObject *> ret;

	const LAString tmpfxcur = getCrossBaseCurrency(ccy);

	const LAString model = LAMarketData::getModelName(tmpfxcur);
	const LAString riskName = getRiskName();
	// scenario param
	MAScenarioParam param;
	param.ccy = tmpfxcur;
	param.shiftType = RISK_BUMPDIRECTION_UPSHIFT;
	param.calcType= getBaseExtraCalcType(tmpfxcur, index);
	param.basisType = getBasisType(ccy);
	param.model = model;
	param.isParallel = true;
	param.isGrid = false;
	// set target name
	LAMathFXEntity *targetFX = LAMarketData::getFXEntity(dataInstance.getObjectPool(), "FORWARDRATE");
	param.targetName = targetFX->getName();

	// search shift target currency(foreign currency)
	LAStringVector ccys;
	LAMarketData::convertToCurrency(tmpfxcur, ccys);
	const LAStringVector &fx_ccys = targetFX->getCurrencys().get();
	LAStringVector::const_iterator it = find(fx_ccys.begin(), fx_ccys.end(), ccys[1]);
	if (it == fx_ccys.end())
	{
		LAString msg = "FX object ccy is not registed  ccy = " + ccys[1].toUpper();
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int pos = static_cast<unsigned int>(it - fx_ccys.begin());
	it = find(fx_ccys.begin(), fx_ccys.end(), ccys[0]);
	if (it == fx_ccys.end())
	{
		LAString msg = "FX object ccy is not registed  ccy = " + ccys[0].toUpper();
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int ccy0pos = static_cast<unsigned int>(it - fx_ccys.begin());
	const DoubleArray& spotrates = targetFX->getSpotRates().get();
	double ccy0ratio = spotrates[ccy0pos];

	// set shift value
	param.paraShiftVec.resize(fx_ccys.size(), 0.0);

	//set base shift value
	double baseShifVal = getBaseSpotVal(ccy, index);
	param.extraBaseParamVec.resize(fx_ccys.size(), 0.0);
	param.extraBaseParamVec[pos] = baseShifVal;
	param.extraBaseParamVec[pos] *= ccy0ratio;

	// set reference
	LAString refNameStr = targetFX->getData(IR_MODEL_DATA_YIELDCURVES, ISNOTNULL).convertToString().exchange("\"", "");
	param.refName = refNameStr.toToken(MULTI_STATIC_DATA_DELIMITER);

	// create scenario
	LAScenarioConfiguration *sceCreator = 
		LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_FX);

	ret = sceCreator->createScenario(dataInstance, param);
	delete sceCreator;

	// nocalib flag
	LAStringVector calibfxvec = getCalibTargetFX(ccy, dataInstance);
	unsigned int size = calibfxvec.size();
	if (size != 0 && std::find(calibfxvec.begin(),calibfxvec.end(),tmpfxcur) != calibfxvec.end())
	{
		//fxvol 
		MAScenarioParam paramvol;
		paramvol.ccy = tmpfxcur;
		paramvol.isCalib = true;
		paramvol.calcType= getBaseExtraCalcType(tmpfxcur, index);
		paramvol.model = model;
		paramvol.targetName = LAMarketData::getBaseVolatilityName(tmpfxcur);
		paramvol.inputType = LAMarketData::getVolInputType(model, tmpfxcur, riskName);
		paramvol.isParallel = true;
		paramvol.isGrid = false;

		LAObjectPool &objPool = dataInstance.getObjectPool();
		// set reference
		LACalibrationParameters *calibInfoCreator = LACalibrationParametersManager::getInstance()->createCalibInfoCreator(paramvol.model);
		LAString infoName = calibInfoCreator->createCalibrationInfo(objPool, tmpfxcur);
		delete calibInfoCreator;
		paramvol.refName.push_back(infoName);

		// set yield and calibdata
		paramvol.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
		paramvol.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, paramvol.refName.back())));
		paramvol.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
		paramvol.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, paramvol.refName.back())));

		// dataout
		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			paramvol.isOutPut = true;
		}
		else 
		{
			paramvol.isOutPut = false;
		}

		// set DDL
		LAString fxKey =  LAMarketData::getFXKey(ccys[0], ccys[1]);
		paramvol.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fxKey + STATIC_DATA_FX_KEY_SDE_ISDD));
		// set file path dmy
		paramvol.paraFile.push_back(CALIB_DMY_FILE);
		// set shift value and type
		paramvol.shiftType = RISK_BUMPDIRECTION_UPSHIFT;
		paramvol.extraBaseParam = getBaseSpotVal(ccy, index);
		// create scenario
		LAScenarioConfiguration *sceVolCreator = 
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL);

		vector<LAObject *> base_vol = sceVolCreator->createScenario(dataInstance, paramvol);
		ret.insert(ret.end(), base_vol.begin(), base_vol.end());
		delete sceVolCreator;
	}
	return ret;
}

/*!
    @brief return Base target names

	@param[in] ccy
	@return LAStringVector
*/
LAString
LARiskConfiguration::getFXBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAString ret =  LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	LAString fx = getCrossBaseCurrency(ccy);
	if (fx == AQ_NO_DATA) 
		fx = ccy;
	// nocalib flag
	LAStringVector calibfxvec = getCalibTargetFX(ccy, dataInstance);
	unsigned int size = calibfxvec.size();
	if (size != 0 && std::find(calibfxvec.begin(), calibfxvec.end(), fx) != calibfxvec.end())
	{
		ret += (":" + LAMarketData::getBaseVolatilityName(fx));
	}
	return ret;
}

/*!
    @brief return Base extra target names

	@param[in] ccy
	@return LAStringVector
*/
LAStringVector
LARiskConfiguration::getFXBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const
{
	LAStringVector ret;
	const LAString targetName = LACoreDataService::getContext(CONTEXT_KEY_FXENTIY_NAME_FORWARD);
	ret.push_back(targetName);

	// nocalib flag
	LAStringVector calibfxvec = getCalibTargetFX(ccy, dataInstance);
	unsigned int size = calibfxvec.size();
	if (size != 0 && std::find(calibfxvec.begin(),calibfxvec.end(), getCrossBaseCurrency(ccy).toUpper()) != calibfxvec.end())
	{
		LAString fxcur = getCrossBaseCurrency(ccy);
		const LAString targetFXVolname = LAMarketData::getBaseVolatilityName(fxcur);
		ret.push_back(targetFXVolname);
	}

	return ret;
}

/*!
    @brief set reference names of param

	@param[in] targetCcy
	@param[in] ccys
	@param[in] fCurveCcys
	@param[in] objPool
	@param[out] refName
	@param[in] sceName
	@param[in] calctype
	@param[in] targetCurveType
	@param[in] suffix
	@param[in] idx
	@return LAString
*/
void 
LARiskConfiguration::setScenarioParamReferences(const LAString& targetCcy, const LAStringVector& ccys, const LAStringVector& fCurveCcys, 
								LAObjectPool& objPool, LAStringVector& refName, const LAString& sceName, const LAString& calcType, 
								const LAString& suffix, const int idx) const
{
	if (ccys[0] == targetCcy)
	{
		// set domestic curve name
		refName.push_back(sceName);
		if(isCalibTarget(targetCcy))
		{
			refName.push_back(LAMarketData::getCalibDataName(calcType, LAMarketData::getYieldDataName(objPool, refName.back()), idx));
		}
		else
		{
			refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[0]))));
		}

		// set foreign curve name
		if (fCurveCcys.size() != 0 && find(fCurveCcys.begin(), fCurveCcys.end(), ccys[1]) != fCurveCcys.end())
		{
			const LAString& forBaseYieldName = LAMarketData::getBaseYieldName(ccys[1]);
			LAString forYieldName = forBaseYieldName /*+ "_" + prefix*/ + suffix;
			refName.push_back(forYieldName);
			if (isCalibTarget(ccys[1]))
			{
				refName.push_back(LAMarketData::getCalibDataName(calcType, LAMarketData::getYieldDataName(objPool, refName.back()), idx));
			}
			else
			{
				refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, forBaseYieldName)));
			}
		}
		else
		{
			refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
			refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, refName.back())));
		}
	}
	else
	{
		// set domestic curve name
		if (fCurveCcys.size() != 0 && find(fCurveCcys.begin(), fCurveCcys.end(), ccys[0]) != fCurveCcys.end())
		{
			const LAString& forBaseYieldName = LAMarketData::getBaseYieldName(ccys[0]);
			LAString forYieldName = forBaseYieldName /*+ "_" + prefix*/ + suffix;
			refName.push_back(forYieldName);
			if (isCalibTarget(ccys[0]))
			{
				refName.push_back(LAMarketData::getCalibDataName(calcType, LAMarketData::getYieldDataName(objPool, refName.back()), idx));
			}
			else
			{
				refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, forBaseYieldName)));
			}
		}
		else
		{
			refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
			refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, refName.back())));
		}

		if (ccys[1] == targetCcy)
		{
			// set foreign curve name
			refName.push_back(sceName);
			if(isCalibTarget(targetCcy))
			{
				refName.push_back(LAMarketData::getCalibDataName(calcType, LAMarketData::getYieldDataName(objPool, refName.back()), idx));
			}
			else
			{
				refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, LAMarketData::getBaseYieldName(ccys[1]))));
			}
		}
		else
		{
			if (fCurveCcys.size() != 0 && find(fCurveCcys.begin(), fCurveCcys.end(), ccys[1]) != fCurveCcys.end())
			{
				const LAString& forBaseYieldName = LAMarketData::getBaseYieldName(ccys[1]);
				LAString forYieldName = forBaseYieldName /*+ "_" + prefix*/ + suffix;
				refName.push_back(forYieldName);
				if (isCalibTarget(ccys[1]))
				{
					refName.push_back(LAMarketData::getCalibDataName(calcType, LAMarketData::getYieldDataName(objPool, refName.back()), idx));
				}
				else
				{
					refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, forBaseYieldName)));
				}
			}
			else
			{
				refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
				refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, refName.back())));
			}
		}
	}
}

/*!
    @brief return range grid term

	@param[in] ccy
	@param[in] index
	@return LAString
*/
vector<LAString> 
LARiskConfiguration::getGridRangeTerm(const LAString &ccy) const
{
	LAStringVector ret;
	LAString gridRange = LACoreDataService::getContext(ARG_KEY_GRIDRANGE).toUpper();
	if (gridRange == AQ_NO_DATA)
	{
		return ret;
	}
	LAStringVector gridRangeVec = gridRange.toToken(':');
	if (gridRangeVec.size() != 2)
	{
		throw LACoreInvalidData("gridrange format is wrong. -gridrange [startgrid]:[endgrid]", __FILE__, __LINE__);
	}
	LAStringVector gridTerm = getShiftGridTerm(ccy);

	if (gridRangeVec[0].getIntValue() < 1)
	{
		LAString msg = "Start grid in gridrange must be larger than 1. Start grid = " +  gridRangeVec[0];
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	if (gridRangeVec[1].getIntValue() < 1)
	{
		LAString msg = "End grid in gridrange must be larger than 1. Emd grid = " +  gridRangeVec[1];
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	unsigned int pos_s = static_cast<unsigned int>(gridRangeVec[0].getIntValue());
	unsigned int pos_e = static_cast<unsigned int>(gridRangeVec[1].getIntValue());
	
	if (pos_s > pos_e)
	{
		LAString msg = "Start grid in gridrange > End grid in gridrange. Start grid = " +  gridRangeVec[0] + ". End grid = " +  gridRangeVec[1];
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	if (pos_s > gridTerm.size())
	{
		ret.push_back(gridTerm.back());
		LACoreDataService::setContext(CONTEXT_KEY_RISKGRID_ISOMITSTART, "TRUE");
		LACoreDataService::setContext(CONTEXT_KEY_RISKGRID_ISOMITEND, "TRUE");
		return ret;
	}
	--pos_s;
	--pos_e;

	if (pos_s != 0)
	{
		--pos_s;
		LACoreDataService::setContext(CONTEXT_KEY_RISKGRID_ISOMITSTART, "TRUE");
	}
	for (unsigned int i = pos_s; i <= pos_e; ++i)
	{
		if (i < gridTerm.size())
		{
			ret.push_back(gridTerm[i]);
		}
		else
		{
			break;
		}
	}

	if (pos_e < gridTerm.size() - 1)
	{
		ret.push_back(gridTerm.back());
		LACoreDataService::setContext(CONTEXT_KEY_RISKGRID_ISOMITEND, "TRUE");
	}
	return ret;
	// start pos
	//unsigned int pos_s;
	//LAStringVector::const_iterator it = find(gridTerm.begin(), gridTerm.end(), gridRangeVec[0]);
	//if (it == gridTerm.end())
	//{
	//	LAString msg = "Start grid in gridrange does not exist in grid shift term. Start grid = " +  gridRangeVec[0];
	//	throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	//}
	//pos_s = it - gridTerm.begin();
	//// end pos
	//unsigned int pos_e;
	//it = find(gridTerm.begin(), gridTerm.end(), gridRangeVec[1]);
	//if (it == gridTerm.end())
	//{
	//	LAString msg = "End grid in gridrange does not exist in grid shift term. End grid = " +  gridRangeVec[1];
	//	throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	//}
	//pos_e = it - gridTerm.begin();

	//if (pos_s > pos_e)
	//{
	//	LAString msg = "Start grid in gridrange > End grid in gridrange. Start grid = " +  gridRangeVec[0] + ". End grid = " +  gridRangeVec[1];
	//	throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	//}

	//if (pos_s != 0)
	//{
	//	--pos_s;
	//}
	//for (unsigned int i = pos_s; i <= pos_e; ++i)
	//{
	//	ret.push_back(gridTerm[i]);
	//}
	//if (pos_e != gridTerm.size() - 1)
	//{
	//	ret.push_back(gridTerm.back());
	//}
	//return ret;
}


