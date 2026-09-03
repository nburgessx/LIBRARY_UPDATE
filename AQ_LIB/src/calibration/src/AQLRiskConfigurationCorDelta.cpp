#include <functional>
#include <memory>

#include "AQLCoreTemplateType.h"
#include "AQLDataMatrix.h"
#include "AQLObjectPool.h"

#include "AQLMathPathEntity.h"
#include "AQLMathVolatility.h"
#include "AQLPricePortfolioValue.h"

#include "AQLCalibrationParameters.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLDealUtils.h"
#include "AQLMarketData.h"
#include "AQLStaticData.h"
#include "AQLRiskConfigurationCorDelta.h"
#include "AQLScenarioConfiguration.h"
#include "AQLScenarioConfigurationManager.h"

namespace
{
	/*!
	    @Description: Retrieves the targets of cordelta from risk.properties
	    @param riskPropAccessor [in] A object that has an access to risk.properties
	    @return The targets of cordelta like { "JPY", "JPY/EUR" } in upper case
	*/
	AQLStringVector getTargets(const AQLStaticData& riskPropAccessor)
	{
		const AQLString raw = riskPropAccessor.getStaticData(RISK_FRONT_COR_CORDELTA_TARGET);
		AQLStringVector targets = raw.toToken('-');
		if (targets.size() != 2)
		{
			throw AQLCoreInvalidData(
					(raw + " is not qualified for cordelta.target; it shall have two simulation SDE currencies").getCString(),
					__FILE__, __LINE__);
		}
		std::for_each(targets.begin(), targets.end(), std::mem_fn(&AQLString::toUpper));
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
	AQLString deduceFX(const AQLString& participant1, const AQLString& participant2)
	{
		AQLStringVector ccys1 = AQLString(participant1).toUpper().toToken(FX_DELIMITER);
		AQLStringVector ccys2 = AQLString(participant2).toUpper().toToken(FX_DELIMITER);
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
				return AQLMarketData::getFXKey(ccys1[0], ccys1[1]);
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
				return AQLMarketData::getFXKey(ccys2[0], ccys2[1]);
			}
		}

		// We can deduce "JPY/EUR" or "EUR/JPY" from {"JPY", "EUR"}
		AQLString fx;
		const AQLStringVector simccys = AQLDealUtils::getSimulationSDECurrencys(true);
		AQLStringVector::const_iterator i = std::find(simccys.begin(), simccys.end(), AQLMarketData::getFXKey(ccys1[0], ccys2[0]).toUpper());
		if (i != simccys.end())
		{
			fx = *i;
		}
		else
		{
			AQLStringVector::const_iterator j = std::find(simccys.begin(), simccys.end(), AQLMarketData::getFXKey(ccys2[0], ccys1[0]).toUpper());
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
		AQLStringVector ccys = fx.toToken(FX_DELIMITER);
		return AQLMarketData::getFXKey(ccys[0], ccys[1]);
	}

	/*!
	    @Description: Retrieves the raw, which means "as-is", bump direction written in risk.properties
	    @param key [in] A key which points the correlation participants like "JPY-USD" or "JPY-JPY/USD", whose cases are not significant
	    @param riskPropAccessor [in] A object that has an access to risk.properties
	    @return true if upshift, false otherwise
	*/
	bool getRawBumpDirection(const AQLString &key, const AQLStaticData& riskPropAccessor)
	{
		const AQLString direction = riskPropAccessor.getStaticData(AQLString(key).toLower() +
			COR_KEY_RISK_FRONT_COR_CORDELTA_BUMPDIRECTION).toUpper();
		if (direction == RISK_BUMPDIRECTION_UPDOWNSHIFT)
		{
			throw AQLCoreInvalidData("Correlation's shift must be up or down; updown is not supported",
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
	std::pair<std::size_t, std::size_t> locateInSimulationSDECurrencys(const AQLString& ccy1, const AQLString& ccy2)
	{
		AQLStringVector simccys = AQLDealUtils::getSimulationSDECurrencys(true);
		for (auto& s : simccys) s.toUpper();
		const std::size_t i = std::find(simccys.begin(), simccys.end(), AQLString(ccy1).toUpper()) - simccys.begin();
		const std::size_t j = std::find(simccys.begin(), simccys.end(), AQLString(ccy2).toUpper()) - simccys.begin();
		assert(i != simccys.size());
		assert(j != simccys.size());
		return std::make_pair(i, j);
	}

	/*!
	    @Description: Stably get a certain FX from the simulation SDE currencies
	    @return A certain FX from the simulation SDE currencies, which is stable
	            (that is, this function yields the identical value every time)
	*/
	AQLString getAnySimulatedFX()
	{
		AQLStringVector simccys = AQLDealUtils::getSimulationSDECurrencys(true);
		for (std::size_t i = 0, ie = simccys.size(); i < ie; ++i)
		{
			if (simccys[i].findString(FX_DELIMITER) != -1)
			{
				return simccys[i];
			}
		}
		throw AQLCoreInvalidData("No FX contained in the simulation SDE currencies", __FILE__, __LINE__);
	}

} // end unnamed namespace

AQLRiskConfigurationCorDelta::AQLRiskConfigurationCorDelta(void)
:AQLRiskConfiguration(true/*mFXFlg*/)
{
}

// destructor
AQLRiskConfigurationCorDelta::~AQLRiskConfigurationCorDelta(void)
{
}

std::vector<std::pair<AQLString, std::vector<AQLObject *> > >
AQLRiskConfigurationCorDelta::createRiskEntity(AQLObjectPool &objPool) const
{
	const AQLStringVector targets = getTargets(*mpRiskStaticData);	// { "JPY", "JPY/EUR" }

	{
		AQLStringVector simccys = AQLDealUtils::getSimulationSDECurrencys(true);
		std::for_each(simccys.begin(), simccys.end(), std::mem_fn(&AQLString::toUpper));
		for (const AQLString& target : targets)
		{
			const std::size_t i = std::find(simccys.begin(), simccys.end(), target) - simccys.begin();
			if (i == simccys.size())
			{
				throw AQLCoreInvalidData(
					(target + ", which is a target of correlation's risk, is not a simulation SDE currency").getCString(),
					__FILE__, __LINE__);
			}
		}
	}

	AQLString key = targets[0] + '-' + targets[1];

	DoubleArray shiftVals = getBaseShiftVals(key);

	std::vector<AQLObject *> eVec(shiftVals.size());
	for (std::size_t k = 0, jend = shiftVals.size(); k < jend; ++k)
	{
		AQLString name = key + "_" + getRiskName() + "_" + getCurveType(key) + "_Shift_" + AQLString(shiftVals[k]);
		AQLObjectHolder objHolder = objPool.getObject(name, ENCHKTYPE_NOCHECK);
		AQLObject *e = 0;
		if (!objHolder.isDefined())
		{
			// create risk object
			e = new AQLObject();
		}
		else
		{
			e = &objHolder.get();
			e->reset();
		}
		e->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(name);

		// These don't seem to have any points for correlations, but I leave them here
		e->add(PRICING_DATA_RISKCURVETYPENAME, new AQLDataString()).convertFromString(getCurveType(key));
		e->add(PRICING_DATA_RISKBASESHIFTCURVETYPENAME, new AQLDataString()).convertFromString(getBaseShiftCurveType(key));
		e->add(PRICING_DATA_RISKCURVETYPECURRENCY, new AQLDataString()).convertFromString(key);

		eVec[k] = e;
	}

	putBumpedCor(key, objPool);

	std::vector<std::pair<AQLString, std::vector<AQLObject *>>> ret;
	ret.emplace_back(std::move(key), std::move(eVec));
	return ret;
}

bool
AQLRiskConfigurationCorDelta::isRiskCurrencyMode(const AQLString& key) const
{
	//if MA_NODATA return false
	AQLString proprslt = mpRiskStaticData->getStaticData(AQLString(key).toLower() +
		COR_KEY_RISK_FRONT_COR_CORDELTA_ISRISKCURRENCYMODE);
	if (proprslt == AQ_NO_DATA)
	{
		return false;
	}

	return convertBoolFromStr(proprslt);
}

void
AQLRiskConfigurationCorDelta::setUpRiskOutputCurrency(const AQLString &key, AQLObject &e) const
{
	if (isRiskCurrencyMode(key))
	{
		std::set<AQLString> baseCurrencies;

		AQLStringVector participants = key.toToken('-');
		AQLString fx = deduceFX(participants[0], participants[1]);
		if (fx == "")
		{
			if (participants[0].findString(FX_DELIMITER) != -1)
			{
				// Both are FX
				baseCurrencies.insert(AQLMarketData::getBaseCurrencyOfFXPair(participants[0]));
				baseCurrencies.insert(AQLMarketData::getBaseCurrencyOfFXPair(participants[1]));
			}
			else
			{
				// No FX is available
				throw AQLCoreInvalidData(("Cannnot decide a risk output currency for " + key).getCString(), __FILE__, __LINE__);
			}
		}
		else
		{
			baseCurrencies.insert(AQLMarketData::getBaseCurrencyOfFXPair(fx));
		}
		
		if (baseCurrencies.size() > 1)
		{
			std::stringstream m;
			m << "Cannot decide a risk output currency due to ambiguity between";
			std::set<AQLString>::const_iterator ie = baseCurrencies.end();
			std::set<AQLString>::const_iterator j = baseCurrencies.end();
			--j;
			for (std::set<AQLString>::const_iterator i = baseCurrencies.begin(); i != ie; ++i)
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
			throw AQLCoreInvalidData(m.str().c_str(), __FILE__, __LINE__);
		}

		e.remove(PRICING_DATA_RISKOUTPUTCURRENCY);
		e.add(PRICING_DATA_RISKOUTPUTCURRENCY, new AQLDataString(AQLString(*baseCurrencies.begin()).toUpper()));
	}

	return;
}

std::vector<AQLObject *>
AQLRiskConfigurationCorDelta::createScenario1Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const
{
	// Scenario1 is an object which holds a bumped correlation matrix for the path object

	std::unique_ptr<AQLObject> scenario(new AQLObject);
	scenario->add(IR_MODEL_DATA_CORRELATIONMATRIX, new AQLDataDoubleMatrix(mBumpedCors[key].corMtxForPathEntity));
	scenario->add(CALIBRATION_DATA_NAME, new AQLDataString(AQLMarketData::getPathEnitty(dataInstance.getObjectPool())->getName().get() + '_' +
		IR_MODEL_DATA_CORRELATIONMATRIX + '_' + key + '_' + getRiskName() + '_' + PRICING_DATA_SCENARIO1 + "_" +
		AQLString(index) + "_" + getCurveType(key) + "_Parallel"));

	return std::vector<AQLObject *>(1, scenario.release());
}

std::vector<AQLObject *>
AQLRiskConfigurationCorDelta::createScenario2Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const
{
	key; dataInstance; index;
	return std::vector<AQLObject *>();
}

std::vector<std::vector<AQLObject *> >
AQLRiskConfigurationCorDelta::createExtraScenario1Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const
{
	// ExtraScenario1 is FX vol, which is re-calibrated with a bumped correlation matrix

	const AQLStringVector ks = AQLString(key).toUpper().toToken('-');
	const AQLString fx = deduceFX(ks[0], ks[1]);		// "jpy/eur"
	if (fx == "")
	{
		// Both are FX or no fx is available; We don't need any calibration, so we can place the copy of any FX's vol
		return std::vector<std::vector<AQLObject *> >(1,
			std::vector<AQLObject *>(1,
				dataInstance.getObjectPool().getObject(AQLMarketData::getBaseVolatilityName(getAnySimulatedFX())).get().clone()));
	}

	// Below we have at most one FX in key

	const BumpedCor &cor = mBumpedCors[key];
	const AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	const AQLString model = AQLMarketData::getModelName(fx);
	const AQLString riskName = getRiskName();

	// Re-calibrating is done in the same way as on fx vega
	AQLString inputType = AQLMarketData::getVolInputType(model, fx, RISK_FRONT_VOL_FXVEGA).toUpper();
	if (inputType == INPUT_T_DATA_MATRIX)
	{
		throw AQLCoreInvalidData("Correlation delta does not support data type, now.", __FILE__, __LINE__);
	}

	AQLObjectPool& objPool = dataInstance.getObjectPool();

	AQLScenarioParam param;
	param.ccy = fx;
	param.model = model;
	param.isCalib = true;
	param.calcType= getExtraCalcType(AQLString(fx).toUpper(), SCENARIO_1, index);
	param.shiftType = RISK_SHIFTTYPE_DIFF;
	param.bumpDirection = cor.upshift ? RISK_BUMPDIRECTION_UPSHIFT : RISK_BUMPDIRECTION_DOWNSHIFT;
	param.targetName = AQLMarketData::getBaseVolatilityName(fx);
	param.isDDL = convertBoolFromStr(mpStaticData->getStaticData(fx + STATIC_DATA_FX_KEY_SDE_ISDD));
	param.inputType = inputType;
	param.isWave = false;
	param.extraBaseVolParam = getBaseVolVal(fx, index);
	param.extraBaseParam = getBaseSpotVal(fx, index);
	param.isOutPut = (AQLCoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA);

	std::shared_ptr<AQLCalibrationParameters> calibInfoCreator(
		AQLCalibrationParametersManager::getInstance()->createCalibInfoCreator(param.model));
	param.refName.push_back(calibInfoCreator->createCalibrationInfo(objPool, fx));
	calibInfoCreator.reset();
	
	AQLString dYieldName = AQLMarketData::getBaseYieldName(ccys[0]);
	AQLString dCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, dYieldName));
	AQLString fYieldName = AQLMarketData::getBaseYieldName(ccys[1]);
	AQLString fCalibDataName = AQLMarketData::getCalibDataName(KEY_PV, AQLMarketData::getYieldDataName(objPool, fYieldName));

	param.isParallel = true;
	param.paraShiftVec.resize(1, 0.0);
	param.paraFile.push_back(CALIB_DMY_FILE);
	param.refName.push_back(dYieldName);
	param.refName.push_back(dCalibDataName);
	param.refName.push_back(fYieldName);
	param.refName.push_back(fCalibDataName);

	// Read the intact (that is, not shifted) correlation matrix into param.corMtx
	AQLStringVector calibccys = AQLString(fx).toUpper().toToken(FX_DELIMITER);
	calibccys.push_back(AQLString(fx).toUpper());
	AQLMarketData::getSDECorrelation(calibccys, param.corMtx);

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
	std::shared_ptr<AQLScenarioConfiguration> sceCreator(
			AQLScenarioConfigurationManager::getInstance()->createScenarioCreator(RISK_SCENARIO_VOL));
	std::vector<AQLObject *> entities = sceCreator->createScenario(dataInstance, param);

	// Rename scenario entities;
	// before: VOL_SDE_JPY/EUR_FX_<fx>_CORDELTA_ExtraScenario1_0_STD_Parallel
	// after : VOL_SDE_JPY/EUR_FX_<key>_CORDELTA_ExtraScenario1_0_STD_Parallel
	const AQLString replacee = "_FX_" + AQLString(fx).toUpper() + "_";
	const AQLString replacer = "_FX_" + AQLString(key).toUpper() + "_";
	for (std::size_t i = 0, iend = entities.size(); i != iend; ++i)
	{
		AQLMathVolatility &e = *dynamic_cast<AQLMathVolatility*>(entities[i]);
		const AQLString n = e.getName().get();
		const int idx = n.findString(replacee);
		const AQLString prefix  = n.subString(0                    , idx - 1 );
		const AQLString postfix = n.subString(idx + replacee.size(), n.size());
		const AQLString nn = prefix + replacer + postfix;
		e.getName() = AQLDataString(nn);
	}

	return std::vector<std::vector<AQLObject *> >(1, entities);
}

// Get the name of the object overwritten by a Scenario
AQLString
AQLRiskConfigurationCorDelta::getTargetNames(const AQLString &key, AQLDataInstance &dataInstance) const
{
	return AQLMarketData::getPathEnitty(dataInstance.getObjectPool())->getCorrelationMatrixEntityName();
}

// Get the name of the object overwritten by a ExtraScenario1
AQLStringVector
AQLRiskConfigurationCorDelta::getExtraTargetNames1(const AQLString &key, AQLDataInstance &/*dataInstance*/) const
{
	const AQLStringVector ks = key.toToken('-');
	const AQLString fx = deduceFX(ks[0], ks[1]);		// "jpy/eur"
	if (fx == "")
	{
		// Both are FX or no FX is available
		return AQLStringVector(1, AQLMarketData::getBaseVolatilityName(getAnySimulatedFX()));
	}
	else
	{
		return AQLStringVector(1, AQLMarketData::getBaseVolatilityName(fx));
	}
}

AQLString
AQLRiskConfigurationCorDelta::getOutPutName1(const AQLString &key) const
{
	return mpRiskStaticData->getStaticData(AQLString(key).toLower() +
		COR_KEY_RISK_FRONT_COR_CORDELTA_OUTPUTNAME);
}

bool
AQLRiskConfigurationCorDelta::isGridSensitivity(const AQLString &key) const
{
	return false;
}

bool
AQLRiskConfigurationCorDelta::isParallelShift(const AQLString &key) const
{
	return true;
}

AQLString
AQLRiskConfigurationCorDelta::getRiskName(void) const
{
	return RISK_FRONT_COR_CORDELTA;
}

std::vector<AQLString>
AQLRiskConfigurationCorDelta::getGridTerm(const AQLString &key) const
{
	return std::vector<AQLString>();
}

std::vector<AQLString>
AQLRiskConfigurationCorDelta::getBucketGridTerm(const AQLString &key) const
{
	return std::vector<AQLString>();
}

AQLString
AQLRiskConfigurationCorDelta::getBumpDirection(const AQLString &key) const
{
	return mBumpedCors[key].upshift ? RISK_BUMPDIRECTION_UPSHIFT : RISK_BUMPDIRECTION_DOWNSHIFT;
}

double
AQLRiskConfigurationCorDelta::getDivUnit(const AQLString &key) const
{
	return mpRiskStaticData->getStaticData(AQLString(key).toLower() +
		COR_KEY_RISK_FRONT_COR_CORDELTA_DIVUNIT).getDoubleValue();
}

double
AQLRiskConfigurationCorDelta::getShiftVal(const AQLString &key, SCENARIONUM scenarioNum) const
{
	scenarioNum;
	const double shiftVal = mpRiskStaticData->getStaticData(AQLString(key).toLower() +
		COR_KEY_RISK_FRONT_COR_CORDELTA_SHIFTVAL).getDoubleValue();
	return shiftVal;
}

void
AQLRiskConfigurationCorDelta::putBumpedCor(const AQLString &key, AQLObjectPool &objPool) const
{
	// Split "JPY-JPY/EUR" into "JPY" and "JPY/EUR"
	const AQLStringVector ccys = AQLString(key).toUpper().toToken('-');

	// Get the intact path object and the correlation matrix
	const AQLMathPathEntity& pathEntity = *AQLMarketData::getPathEnitty(objPool);
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
			throw AQLCoreInvalidData(
				(key + "'s correlation will exceed 1 after shifting").getCString(),
				__FILE__, __LINE__);
		}
	}
	else if (corij - shiftAmount < corMin)
	{
		throw AQLCoreInvalidData(
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
