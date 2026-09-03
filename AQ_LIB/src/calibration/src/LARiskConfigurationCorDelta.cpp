#include <functional>
#include <memory>

#include "LACoreTemplateType.h"
#include "LADataMatrix.h"
#include "LAObjectPool.h"

#include "LAMathPathEntity.h"
#include "LAMathVolatility.h"
#include "LAPricePortfolioValue.h"

#include "LACalibrationParameters.h"
#include "LACalibrationParametersManager.h"
#include "LADefinitionsCalibration.h"
#include "LADealUtils.h"
#include "LAMarketData.h"
#include "LAStaticData.h"
#include "LARiskConfigurationCorDelta.h"
#include "LAScenarioConfiguration.h"
#include "LAScenarioConfigurationManager.h"

namespace
{
	/*!
	    @Description: Retrieves the targets of cordelta from risk.properties
	    @param riskPropAccessor [in] A object that has an access to risk.properties
	    @return The targets of cordelta like { "JPY", "JPY/EUR" } in upper case
	*/
	LAStringVector getTargets(const LAStaticData& riskPropAccessor)
	{
		const LAString raw = riskPropAccessor.getStaticData(RISK_FRONT_COR_CORDELTA_TARGET);
		LAStringVector targets = raw.toToken('-');
		if (targets.size() != 2)
		{
			throw LACoreInvalidData(
					(raw + " is not qualified for cordelta.target; it shall have two simulation SDE currencies").getCString(),
					__FILE__, __LINE__);
		}
		std::for_each(targets.begin(), targets.end(), std::mem_fn(&LAString::toUpper));
		return targets;
	}

	/*!
	    @Description: Decide the forex which resides in the participants of cordelta
	    @param participant1 [in] The first participant of cordelta, which may be a currency or a forex
	    @param participant2 [in] The second participant of cordelta, which may be a currency or a forex
	    @return The resulted forex, which might not appear explicitly in the targets of cordelta
	            (that is, even if the parcipants are "JPY" and "USD", this function correctly deduces "jpy/usd");
	            Note that the returned string shall be empty in the following cases:
	             - both participants are FXs
	             - one participant is a currency and another is an irrelevant FX (for example, "AUD" and "JPY/USD")
	             - the resulted FX is not a simulation SDE currency
	*/
	LAString deduceFX(const LAString& participant1, const LAString& participant2)
	{
		LAStringVector ccys1 = LAString(participant1).toUpper().toToken(FX_DELIMITER);
		LAStringVector ccys2 = LAString(participant2).toUpper().toToken(FX_DELIMITER);
		if (ccys1.size() > 1) {
			// participant1 is FX
			if (ccys2.size() > 1)
			{
				// Both are FXs
				return "";
			}
			else if ((ccys2[0] != ccys1[0]) && (ccys2[0] != ccys1[1]))
			{
				// participant2 is irrelevant currency (for example, "USD" for "JPY/AUD")
				return "";
			}
			else
			{
				return LAMarketData::getFXKey(ccys1[0], ccys1[1]);
			}
		}
		else if (ccys2.size() > 1)
		{
			if ((ccys1[0] != ccys2[0]) && (ccys1[0] != ccys2[1]))
			{
				// participant1 is irrelevant currency (for example, "USD" for "JPY/AUD")
				return "";
			}
			else
			{
				return LAMarketData::getFXKey(ccys2[0], ccys2[1]);
			}
		}

		// We can deduce "JPY/EUR" or "EUR/JPY" from {"JPY", "EUR"}
		LAString fx;
		const LAStringVector simccys = MADealUtils::getSimulationSDECurrencys(true);
		LAStringVector::const_iterator i = std::find(simccys.begin(), simccys.end(), LAMarketData::getFXKey(ccys1[0], ccys2[0]).toUpper());
		if (i != simccys.end())
		{
			fx = *i;
		}
		else
		{
			LAStringVector::const_iterator j = std::find(simccys.begin(), simccys.end(), LAMarketData::getFXKey(ccys2[0], ccys1[0]).toUpper());
			if (j != simccys.end())
			{
				fx = *i;
			}
			else
			{
				// the resulted FX is not a simulation SDE currency
				return "";
			}
		}
		LAStringVector ccys = fx.toToken(FX_DELIMITER);
		return LAMarketData::getFXKey(ccys[0], ccys[1]);
	}

	/*!
	    @Description: Retrieves the raw, which means "as-is", bump direction written in risk.properties
	    @param key [in] A key which points the correlation participants like "JPY-USD" or "JPY-JPY/USD", whose cases are not significant
	    @param riskPropAccessor [in] A object that has an access to risk.properties
	    @return true if upshift, false otherwise
	*/
	bool getRawBumpDirection(const LAString &key, const LAStaticData& riskPropAccessor)
	{
		const LAString direction = riskPropAccessor.getStaticData(LAString(key).toLower() +
			COR_KEY_RISK_FRONT_COR_CORDELTA_BUMPDIRECTION).toUpper();
		if (direction == RISK_BUMPDIRECTION_UPDOWNSHIFT)
		{
			throw LACoreInvalidData("Correlation's shift must be up or down; updown is not supported",
				__FILE__, __LINE__);
		}
		else
		{
			// Defaults to upshift
			return direction != RISK_BUMPDIRECTION_DOWNSHIFT;
		}
	}

	/*!
	    @Description: Find strings (ignoring cases) in the "simulation SDE currencies" and returns the indices
	    @param ccy1 [in] The first string, which may be not a currency name but a forex name (for example, "jpy/usd")
	    @param ccy2 [in] The second string, which may be not a currency name but a forex name (for example, "jpy/usd")
	    @return A pair of the index of ccy1 and the index of ccy2 in the "simulation SDE currencies"
	*/
	std::pair<std::size_t, std::size_t> locateInSimulationSDECurrencys(const LAString& ccy1, const LAString& ccy2)
	{
		LAStringVector simccys = MADealUtils::getSimulationSDECurrencys(true);
		for (auto& s : simccys) s.toUpper();
		const std::size_t i = std::find(simccys.begin(), simccys.end(), LAString(ccy1).toUpper()) - simccys.begin();
		const std::size_t j = std::find(simccys.begin(), simccys.end(), LAString(ccy2).toUpper()) - simccys.begin();
		assert(i != simccys.size());
		assert(j != simccys.size());
		return std::make_pair(i, j);
	}

	/*!
	    @Description: Stably get a certain FX from the simulation SDE currencies
	    @return A certain FX from the simulation SDE currencies, which is stable
	            (that is, this function yields the identical value every time)
	*/
	LAString getAnySimulatedFX()
	{
		LAStringVector simccys = MADealUtils::getSimulationSDECurrencys(true);
		for (std::size_t i = 0, ie = simccys.size(); i < ie; ++i)
		{
			if (simccys[i].findString(FX_DELIMITER) != -1)
			{
				return simccys[i];
			}
		}
		throw LACoreInvalidData("No FX contained in the simulation SDE currencies", __FILE__, __LINE__);
	}

} // end unnamed namespace

LARiskConfigurationCorDelta::LARiskConfigurationCorDelta(void)
:LARiskConfiguration(true/*mFXFlg*/)
{
}

// destructor
LARiskConfigurationCorDelta::~LARiskConfigurationCorDelta(void)
{
}

std::vector<std::pair<LAString, std::vector<LAObject *> > >
LARiskConfigurationCorDelta::createRiskEntity(LAObjectPool &objPool) const
{
	const LAStringVector targets = getTargets(*mpRiskStaticData);	// { "JPY", "JPY/EUR" }

	{
		LAStringVector simccys = MADealUtils::getSimulationSDECurrencys(true);
		std::for_each(simccys.begin(), simccys.end(), std::mem_fn(&LAString::toUpper));
		for (const LAString& target : targets)
		{
			const std::size_t i = std::find(simccys.begin(), simccys.end(), target) - simccys.begin();
			if (i == simccys.size())
			{
				throw LACoreInvalidData(
					(target + ", which is a target of correlation's risk, is not a simulation SDE currency").getCString(),
					__FILE__, __LINE__);
			}
		}
	}

	LAString key = targets[0] + '-' + targets[1];

	DoubleArray shiftVals = getBaseShiftVals(key);

	std::vector<LAObject *> eVec(shiftVals.size());
	for (std::size_t k = 0, jend = shiftVals.size(); k < jend; ++k)
	{
		LAString name = key + "_" + getRiskName() + "_" + getCurveType(key) + "_Shift_" + LAString(shiftVals[k]);
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

		// These don't seem to have any points for correlations, but I leave them here
		e->add(PRICING_DATA_RISKCURVETYPENAME, new LADataString()).convertFromString(getCurveType(key));
		e->add(PRICING_DATA_RISKBASESHIFTCURVETYPENAME, new LADataString()).convertFromString(getBaseShiftCurveType(key));
		e->add(PRICING_DATA_RISKCURVETYPECURRENCY, new LADataString()).convertFromString(key);

		eVec[k] = e;
	}

	putBumpedCor(key, objPool);

	std::vector<std::pair<LAString, std::vector<LAObject *>>> ret;
	ret.emplace_back(std::move(key), std::move(eVec));
	return ret;
}

bool
LARiskConfigurationCorDelta::isRiskCurrencyMode(const LAString& key) const
{
	//if MA_NODATA return false
	LAString proprslt = mpRiskStaticData->getStaticData(LAString(key).toLower() +
		COR_KEY_RISK_FRONT_COR_CORDELTA_ISRISKCURRENCYMODE);
	if (proprslt == MLIB_NO_DATA)
	{
		return false;
	}

	return convertBoolFromStr(proprslt);
}

void
LARiskConfigurationCorDelta::setUpRiskOutputCurrency(const LAString &key, LAObject &e) const
{
	if (isRiskCurrencyMode(key))
	{
		std::set<LAString> baseCurrencies;

		LAStringVector participants = key.toToken('-');
		LAString fx = deduceFX(participants[0], participants[1]);
		if (fx == "")
		{
			if (participants[0].findString(FX_DELIMITER) != -1)
			{
				// Both are FX
				baseCurrencies.insert(LAMarketData::getBaseCurrencyOfFXPair(participants[0]));
				baseCurrencies.insert(LAMarketData::getBaseCurrencyOfFXPair(participants[1]));
			}
			else
			{
				// No FX is available
				throw LACoreInvalidData(("Cannnot decide a risk output currency for " + key).getCString(), __FILE__, __LINE__);
			}
		}
		else
		{
			baseCurrencies.insert(LAMarketData::getBaseCurrencyOfFXPair(fx));
		}
		
		if (baseCurrencies.size() > 1)
		{
			std::stringstream m;
			m << "Cannot decide a risk output currency due to ambiguity between";
			std::set<LAString>::const_iterator ie = baseCurrencies.end();
			std::set<LAString>::const_iterator j = baseCurrencies.end();
			--j;
			for (std::set<LAString>::const_iterator i = baseCurrencies.begin(); i != ie; ++i)
			{
				if (i == baseCurrencies.begin())
				{
					m << ' ' << *i;
				}
				else if (i == j)
				{
					m << ", and " << *i;
				}
				else
				{
					m << ", " << *i;
				}
			}
			throw LACoreInvalidData(m.str().c_str(), __FILE__, __LINE__);
		}

		e.remove(PRICING_DATA_RISKOUTPUTCURRENCY);
		e.add(PRICING_DATA_RISKOUTPUTCURRENCY, new LADataString(LAString(*baseCurrencies.begin()).toUpper()));
	}

	return;
}

std::vector<LAObject *>
LARiskConfigurationCorDelta::createScenario1Entity(const LAString &key, LADataInstance &dataInstance, int index) const
{
	// Scenario1 is an object which holds a bumped correlation matrix for the path object

	std::unique_ptr<LAObject> scenario(new LAObject);
	scenario->add(IR_MODEL_DATA_CORRELATIONMATRIX, new LADataDoubleMatrix(mBumpedCors[key].corMtxForPathEntity));
	scenario->add(CALIBRATION_DATA_NAME, new LADataString(LAMarketData::getPathEnitty(dataInstance.getObjectPool())->getName().get() + '_' +
		IR_MODEL_DATA_CORRELATIONMATRIX + '_' + key + '_' + getRiskName() + '_' + PRICING_DATA_SCENARIO1 + "_" +
		LAString(index) + "_" + getCurveType(key) + "_Parallel"));

	return std::vector<LAObject *>(1, scenario.release());
}

std::vector<LAObject *>
LARiskConfigurationCorDelta::createScenario2Entity(const LAString &key, LADataInstance &dataInstance, int index) const
{
	key; dataInstance; index;
	return std::vector<LAObject *>();
}

std::vector<std::vector<LAObject *> >
LARiskConfigurationCorDelta::createExtraScenario1Entity(const LAString &key, LADataInstance &dataInstance, int index) const
{
	// ExtraScenario1 is FX vol, which is re-calibrated with a bumped correlation matrix

	const LAStringVector ks = LAString(key).toUpper().toToken('-');
	const LAString fx = deduceFX(ks[0], ks[1]);		// "jpy/eur"
	if (fx == "")
	{
		// Both are FX or no fx is available; We don't need any calibration, so we can place the copy of any FX's vol
		return std::vector<std::vector<LAObject *> >(1,
			std::vector<LAObject *>(1,
				dataInstance.getObjectPool().getObject(LAMarketData::getBaseVolatilityName(getAnySimulatedFX())).get().clone()));
	}

	// Below we have at most one FX in key

	const BumpedCor &cor = mBumpedCors[key];
	const LAStringVector ccys = fx.toToken(FX_DELIMITER);
	const LAString model = LAMarketData::getModelName(fx);
	const LAString riskName = getRiskName();

	// Re-calibrating is done in the same way as on fx vega
	LAString inputType = LAMarketData::getVolInputType(model, fx, RISK_FRONT_VOL_FXVEGA).toUpper();
	if (inputType == INPUT_T_DATA_MATRIX)
	{
		throw LACoreInvalidData("Correlation delta does not support data type, now.", __FILE__, __LINE__);
	}

	LAObjectPool& objPool = dataInstance.getObjectPool();

	MAScenarioParam param;
	param.ccy = fx;
	param.model = model;
	param.isCalib = true;
	param.calcType= getExtraCalcType(LAString(fx).toUpper(), SCENARIO_1, index);
	param.shiftType = RISK_SHIFTTYPE_DIFF;
	param.bumpDirection = cor.upshift ? RISK_BUMPDIRECTION_UPSHIFT : RISK_BUMPDIRECTION_DOWNSHIFT;
	param.targetName = LAMarketData::getBaseVolatilityName(fx);
	param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fx + STATIC_DATA_FX_KEY_SDE_ISDD));
	param.inputType = inputType;
	param.isWave = false;
	param.extraBaseVolParam = getBaseVolVal(fx, index);
	param.extraBaseParam = getBaseSpotVal(fx, index);
	param.isOutPut = (LACoreDataService::getContext(ARG_KEY_DATAOUT) != MLIB_NO_DATA);

	std::shared_ptr<LACalibrationParameters> calibInfoCreator(
		LACalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model));
	param.refName.push_back(calibInfoCreator->createCalibrationInfo(objPool, fx));
	calibInfoCreator.reset();
	
	LAString dYieldName = LAMarketData::getBaseYieldName(ccys[0]);
	LAString dCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, dYieldName));
	LAString fYieldName = LAMarketData::getBaseYieldName(ccys[1]);
	LAString fCalibDataName = LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, fYieldName));

	param.isParallel = true;
	param.paraShiftVec.resize(1, 0.0);
	param.paraFile.push_back(CALIB_DMY_FILE);
	param.refName.push_back(dYieldName);
	param.refName.push_back(dCalibDataName);
	param.refName.push_back(fYieldName);
	param.refName.push_back(fCalibDataName);

	// Read the intact (that is, not shifted) correlation matrix into param.corMtx
	LAStringVector calibccys = LAString(fx).toUpper().toToken(FX_DELIMITER);
	calibccys.push_back(LAString(fx).toUpper());
	LAMarketData::getSDECorrelation(calibccys, param.corMtx);

	// The correlation matrix for the vol re-calibration is a subset of the one
	// for the path object
	// (Path object's matrix contains all simulation currencies,
	//  but the one for re-calibration contains only three (cur1/cur2/fx))
	const std::pair<std::size_t, std::size_t> pij = locateInSimulationSDECurrencys(ks[0], ks[1]);
	const std::size_t ci = std::find(calibccys.begin(), calibccys.end(), ks[0]) - calibccys.begin();
	const std::size_t cj = std::find(calibccys.begin(), calibccys.end(), ks[1]) - calibccys.begin();
	param.corMtx[ci][cj] = cor.corMtxForPathEntity[pij.first ][pij.second];
	param.corMtx[cj][ci] = cor.corMtxForPathEntity[pij.second][pij.first ];

	// Re-calibrate and create scenario entities
	std::shared_ptr<LAScenarioConfiguration> sceCreator(
			LAScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL));
	std::vector<LAObject *> entities = sceCreator->createScenario(dataInstance, param);

	// Rename scenario entities;
	// before: VOL_SDE_JPY/EUR_FX_<fx>_CORDELTA_ExtraScenario1_0_STD_Parallel
	// after : VOL_SDE_JPY/EUR_FX_<key>_CORDELTA_ExtraScenario1_0_STD_Parallel
	const LAString replacee = "_FX_" + LAString(fx).toUpper() + "_";
	const LAString replacer = "_FX_" + LAString(key).toUpper() + "_";
	for (std::size_t i = 0, iend = entities.size(); i != iend; ++i)
	{
		LAMathVolatility &e = *dynamic_cast<LAMathVolatility*>(entities[i]);
		const LAString n = e.getName().get();
		const int idx = n.findString(replacee);
		const LAString prefix  = n.subString(0                    , idx - 1 );
		const LAString postfix = n.subString(idx + replacee.size(), n.size());
		const LAString nn = prefix + replacer + postfix;
		e.getName() = LADataString(nn);
	}

	return std::vector<std::vector<LAObject *> >(1, entities);
}

// Get the name of the object overwritten by a Scenario
LAString
LARiskConfigurationCorDelta::getTargetNames(const LAString &key, LADataInstance &dataInstance) const
{
	return LAMarketData::getPathEnitty(dataInstance.getObjectPool())->getCorrelationMatrixEntityName();
}

// Get the name of the object overwritten by a ExtraScenario1
LAStringVector
LARiskConfigurationCorDelta::getExtraTargetNames1(const LAString &key, LADataInstance &/*dataInstance*/) const
{
	const LAStringVector ks = key.toToken('-');
	const LAString fx = deduceFX(ks[0], ks[1]);		// "jpy/eur"
	if (fx == "")
	{
		// Both are FX or no FX is available
		return LAStringVector(1, LAMarketData::getBaseVolatilityName(getAnySimulatedFX()));
	}
	else
	{
		return LAStringVector(1, LAMarketData::getBaseVolatilityName(fx));
	}
}

LAString
LARiskConfigurationCorDelta::getOutPutName1(const LAString &key) const
{
	return mpRiskStaticData->getStaticData(LAString(key).toLower() +
		COR_KEY_RISK_FRONT_COR_CORDELTA_OUTPUTNAME);
}

bool
LARiskConfigurationCorDelta::isGridSensitivity(const LAString &key) const
{
	return false;
}

bool
LARiskConfigurationCorDelta::isParallelShift(const LAString &key) const
{
	return true;
}

LAString
LARiskConfigurationCorDelta::getRiskName(void) const
{
	return RISK_FRONT_COR_CORDELTA;
}

std::vector<LAString>
LARiskConfigurationCorDelta::getGridTerm(const LAString &key) const
{
	return std::vector<LAString>();
}

std::vector<LAString>
LARiskConfigurationCorDelta::getBucketGridTerm(const LAString &key) const
{
	return std::vector<LAString>();
}

LAString
LARiskConfigurationCorDelta::getBumpDirection(const LAString &key) const
{
	return mBumpedCors[key].upshift ? RISK_BUMPDIRECTION_UPSHIFT : RISK_BUMPDIRECTION_DOWNSHIFT;
}

double
LARiskConfigurationCorDelta::getDivUnit(const LAString &key) const
{
	return mpRiskStaticData->getStaticData(LAString(key).toLower() +
		COR_KEY_RISK_FRONT_COR_CORDELTA_DIVUNIT).getDoubleValue();
}

double
LARiskConfigurationCorDelta::getShiftVal(const LAString &key, SCENARIONUM scenarioNum) const
{
	scenarioNum;
	const double shiftVal = mpRiskStaticData->getStaticData(LAString(key).toLower() +
		COR_KEY_RISK_FRONT_COR_CORDELTA_SHIFTVAL).getDoubleValue();
	return shiftVal;
}

void
LARiskConfigurationCorDelta::putBumpedCor(const LAString &key, LAObjectPool &objPool) const
{
	// Split "JPY-JPY/EUR" into "JPY" and "JPY/EUR"
	const LAStringVector ccys = LAString(key).toUpper().toToken('-');

	// Get the intact path object and the correlation matrix
	const LAMathPathEntity& pathEntity = *LAMarketData::getPathEnitty(objPool);
	DoubleMatrix corMtxForPathEntity = pathEntity.getCorrelationMatrix().get();	// makes a copy

	// Locate factor1 and factor2 in the matrix
	std::pair<std::size_t, std::size_t> ij = locateInSimulationSDECurrencys(ccys[0], ccys[1]);
	double& corij = corMtxForPathEntity[ij.first ][ij.second];
	double& corji = corMtxForPathEntity[ij.second][ij.first ];

	// Decide the safe bump direction
	const double shiftAmount = getShiftVal(key, SCENARIO_1);
	const double corMax = 1.0;
	const double corMin = -1.0;
	const bool upshift = getRawBumpDirection(key, *mpRiskStaticData);
	if (upshift)
	{
		if (corij + shiftAmount > corMax)
		{
			throw LACoreInvalidData(
				(key + "'s correlation will exceed 1 after shifting").getCString(),
				__FILE__, __LINE__);
		}
	}
	else if (corij - shiftAmount < corMin)
	{
		throw LACoreInvalidData(
			(key + "'s correlation will fall behind -1 after shifting").getCString(),
			__FILE__, __LINE__);
	}

	// Do the shift
	corij = std::max<double>(
				std::min<double>(
					corij + shiftAmount * (upshift ? 1.0 : -1.0),
					corMax),
				corMin);
	corji = corij;

	// Emplace into the map
	BumpedCor& emplaced = mBumpedCors[key];
	emplaced.corMtxForPathEntity = corMtxForPathEntity;
	emplaced.upshift = upshift;
}
