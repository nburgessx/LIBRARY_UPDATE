/*! @file
    @brief LMM calibration request
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateLMM.cpp
//
//  DESCRIPTION :        LMM calibration request class
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
#include <iostream>
#include <fstream>
#include "LACalibrateLMM.h"
#include "LACalibrationFunc.h"
#include "LAPriceLMMCalibration.h"
#include "LADefinitionsCalibration.h"
#include "LAMathVolFuncStructureBase.h"
#include "LAMathVolFuncWave.h"
#include "LAMathVolFuncLMM.h"
#include "LAMarketData.h"
#include "LACoreAppError.h"
#include "LAObjectPool.h"
#include "LACoreReferencePool.h"
#include "LADataReference.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LADataMultiReference.h"
#include "LADataProcedure.h"
#include "LAStepInterpolation.h"
#include "LAScenarioConfiguration.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAMathDateUtilities.h"
#include "LAMathCurveFuncUtility.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathValuableEntity.h"
#include "LABasic.h"
#include "LACoreDataService.h"
#include "LAMathInterpolationUtilities.h"
#include <sstream>

using namespace std;


// constructor
/*!

*/
LACalibrateLMM::LACalibrateLMM()
:LACalibrate()
{
}

// destructor
/*!

*/
LACalibrateLMM::~LACalibrateLMM()
{
}


// 
/*!
    @brief setup
*/
void
LACalibrateLMM::setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos)
{
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateLMM set up called" << endl;
	if (!method)
	{
		throw LACoreInvalidData("Argument method is NULL", __FILE__, __LINE__);
	}
	if (param.refName.size() < 2)
	{
		throw LACoreInvalidData("Param refName is more than be two, reference curve and calibinfo needed.", __FILE__, __LINE__);
	}
	// set method
	mpFunc = method;
	const LAString calibInfoName = param.refName[0];
	const LAObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
	LAObject *calibID = 0;

	unsigned int curPos = 1;
	if (gridPos >= 0)
	{
		curPos += gridPos;
		if (param.isParallel)
		{
			++curPos;
		}
	}
	const LAString curveName = param.refName[curPos];
	if (!objPool.getObject(curveName).isDefined())
	{
		LAString msg = curveName + " is not registered in EntityPool";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const LAString &curveIDName = dynamic_cast<const LAMathYieldCurve &>(objPool.getObject(curveName, ENCHKTYPE_ISDEFINED).get()).getYieldData().get().getName();

	// set calcType
	mCalcType = param.calcType;
	// set gridPos
	mGridPos = gridPos;
	// serialize status
	mSerializeStatus = param.serializeStatus;
	// serialize file
	mSerializeFile = param.serializeFile;
	// set dataInstance
	mpDataInstance = calibInfo.getDataInstance();
	mCalibIDName = param.calcType + "_" + calibInfoName + "_" + curveIDName + "_" + LAString(gridPos);

	// get calibration id
	LAObjectHolder tmpEh = objPool.getObject(mCalibIDName, ENCHKTYPE_NOCHECK);
	if (!tmpEh.isDefined())
	{
		calibID = new LAObject();
		objPool.set(mCalibIDName, calibID);
	}
	else
	{
		calibID = &tmpEh.get();
		calibID->clear();
	}

	calibID->add(CALIBRATION_DATA_NAME, new LADataString(mCalibIDName));

	calibID->add(CALIBRATION_DATA_IS_EXTRATENOR_CALIB, calibInfo.getData(CALIBRATION_DATA_IS_EXTRATENOR_CALIB, ISNOTNULL).get().clone());

	// set cannonical T
	calibID->add(PRICING_DATA_CALIBCANONICAL_FREQ, calibInfo.getData(PRICING_DATA_CALIBCANONICAL_FREQ, ISNOTNULL).get().clone());
	calibID->add(PRICING_DATA_CALIBCANONICAL_T, calibInfo.getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get().clone());
	calibID->add(PRICING_DATA_CALIBCANONICAL_T_DELTATENOR, calibInfo.getData(PRICING_DATA_CALIBCANONICAL_T_DELTATENOR, ISNOTNULL).get().clone());
	calibID->add(PRICING_DATA_CALIBCANONICAL_T_30_360, calibInfo.getData(PRICING_DATA_CALIBCANONICAL_T_30_360, ISNOTNULL).get().clone());
	calibID->add(PRICING_DATA_CALIBCANONICAL_T_EXTRAFLAG, calibInfo.getData(PRICING_DATA_CALIBCANONICAL_T_EXTRAFLAG, ISNOTNULL).get().clone());
	calibID->add(CALIBRATION_DATA_MAXTERM, calibInfo.getData(CALIBRATION_DATA_MAXTERM, ISNOTNULL).get().clone());

	// set method G consistent parameters
	calibID->add(CALIBRATION_DATA_INTERPOLATION_G, calibInfo.getData(CALIBRATION_DATA_INTERPOLATION_G, ISNOTNULL).get().clone());
	calibID->add(CALIBRATION_DATA_FREQUENCY_G, calibInfo.getData(CALIBRATION_DATA_FREQUENCY_G, ISNOTNULL).get().clone());

	// set skew parameter
	calibID->add(CALIBRATION_DATA_VOL_SKEW, calibInfo.getData(CALIBRATION_DATA_VOL_SKEW, ISNOTNULL).get().clone());

	// deserialize mode
	if (mSerializeStatus == CALIB_S_DESERIALIZE)
	{
		if (!mIsDeserializedMap[mSerializeFile])
		{
			deserializeStream(mSerializeFile);
			setmIsDeserializedMap(true);
		}
		return;
	}
	// set reference
	calibID->add(PRICING_DATA_CURVEID, new LADataReference()).convertFromString(curveIDName);

	// get market info
	// cap parameters
	// term
	const LAStringVector &capTermVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_CAP_TERM, ISNOTNULL).get()).get();
	// tenor
	const LAStringVector &capTenorVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_CAP_TENOR, ISNOTNULL).get()).get();
	// black vol
	DoubleVector capBlackVolVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(LAString("CAP_") + IR_CALIBRATION_DATA_BLACKVOLATILITY, ISNOTNULL).get()).get();
	// swaption parameters
	// optionmaturity
	const LAStringVector &optionMatVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
	// swapterm
	const LAStringVector &swapTenorVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_SWAPTENOR, ISNOTNULL).get()).get();
	// vol mat
	DoubleMatrix swaptionBlackVolMat = dynamic_cast<const LADataDoubleMatrix &>(calibInfo.getData( LAString("SWAPTION_") + IR_CALIBRATION_DATA_BLACKVOLATILITY, ISNOTNULL).get()).get();

	LAString marketRef;
	const LADate asOfDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());

	// shift black vol
	LAString calcType = param.calcType;
	calcType.toUpper();
	LAString bumpType = param.bumpType;
	bumpType.toUpper();

	if (calcType != KEY_PV && bumpType == RISK_MARKET_BUMP)
	{
		if (gridPos < 0)
		{
			// parallel case
			if (param.isParallel && !param.paraShiftVec.empty())
			{
				LAString shiftType = param.shiftType;
				shiftType.toUpper();
				if (param.paraShiftVec.size() == 1)
				{
					if (param.paraTerm.empty() || param.paraTerm[0] == CALIB_MARKET_CAP)
					{
					// cap
					unsigned int capSize = capBlackVolVec.size();
					DoubleArray capShiftVal(capSize, param.paraShiftVec[0]);
					if (shiftType == RISK_SHIFTTYPE_DIFF)
					{
						// diff case
						transform(capBlackVolVec.begin(), capBlackVolVec.end(), capShiftVal.begin(), capBlackVolVec.begin(), plus<double>());
					}
					else
					{
						// ratio case
						DoubleArray plusVal(capBlackVolVec.size());
						transform(capBlackVolVec.begin(), capBlackVolVec.end(), capShiftVal.begin(), plusVal.begin(), multiplies<double>());
						transform(capBlackVolVec.begin(), capBlackVolVec.end(), plusVal.begin(), capBlackVolVec.begin(), plus<double>());
					}
					for (unsigned int i = 0; i < capSize; ++i)
					{
						capBlackVolVec[i] = LAMath::max(capBlackVolVec[i], 0.0);
					}
					}

					if (param.paraTerm.empty() || param.paraTerm[0] == CALIB_MARKET_SWAPTION)
					{
					// swaption
					unsigned int swaptionSize1 = swaptionBlackVolMat.size();
					unsigned int swaptionSize2 = swaptionBlackVolMat[0].size();
					DoubleArray swaptionShiftVal(DoubleArray(swaptionSize2, param.paraShiftVec[0]));
					if (shiftType == RISK_SHIFTTYPE_DIFF)
					{
						// diff case
						for(unsigned int i = 0; i < swaptionSize1; ++i)
						{
							transform(swaptionBlackVolMat[i].begin(), swaptionBlackVolMat[i].end(), swaptionShiftVal.begin(), swaptionBlackVolMat[i].begin(), plus<double>());
						}
					}
					else
					{
						// ratio case
						DoubleArray plusVal(swaptionSize2);
						for(unsigned int i = 0; i < swaptionSize1; ++i)
						{
							transform(swaptionBlackVolMat[i].begin(), swaptionBlackVolMat[i].end(), swaptionShiftVal.begin(), plusVal.begin(), multiplies<double>());
							transform(swaptionBlackVolMat[i].begin(), swaptionBlackVolMat[i].end(), plusVal.begin(), swaptionBlackVolMat[i].begin(), plus<double>());
						}
					}
					for (unsigned int i = 0; i < swaptionSize1; ++i)
					{
						for (unsigned int j = 0; j < swaptionSize2; ++j)
						{
							swaptionBlackVolMat[i][j] = LAMath::max(swaptionBlackVolMat[i][j], 0.0);
						}
						}
					}
				}
				else
				{
					unsigned int paraSize = param.paraTerm.size();
					if (paraSize != param.paraShiftVec.size())
					{
						throw LACoreInvalidData("Parallel shift val and value is not same ", __FILE__, __LINE__);
					}

					for (unsigned int i = 0; i < paraSize; ++i)
					{
						LAString term = param.paraTerm[i];
						term.toUpper();
						int posSwaption = term.findString(CALIB_MARKET_SWAPTION);
						int posCap = term.findString(CALIB_MARKET_CAP);
						if(posSwaption == -1 && posCap == -1)
						{
							throw LACoreInvalidData("LMM Market data is swaption or cap only.", __FILE__, __LINE__);
						}

						// cap
						if(posCap != -1)
						{
							unsigned int gStartPos = strlen(CALIB_MARKET_CAP)+1;
							LAString grid = term.subString(gStartPos, term.size() - 1);
							LAStringVector gridIdx = grid.toToken('_');
							LAString capTerm = gridIdx[0];
							LAString capTenor = gridIdx[1];

							// check term
							LAStringVector::const_iterator it = find(capTenorVec.begin(), capTenorVec.end(), capTenor);
							if (it == capTenorVec.end())
							{
								LAString msg = "Shift grid is not in cap market. grid = " + grid;
								throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
							}
							unsigned int index = static_cast<unsigned int>(it - capTenorVec.begin());

							if (capTermVec[index] != capTerm)
							{
								LAString msg = "Shift grid is not in cap market. grid = " + grid;
								throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
							}

							// shift val
							double shiftVal = param.paraShiftVec[i];
							if (shiftType == RISK_SHIFTTYPE_RATIO)
							{
								shiftVal = capBlackVolVec[index] * shiftVal;
							}
							// add shift val
							capBlackVolVec[index] = LAMath::max(capBlackVolVec[index] + shiftVal, 0.0);
						}
						else if(posSwaption != -1)
						{
							// swaption
							//unsigned int gStartPos = strlen(CALIB_MARKET_SWAPTION)+1;
							LAStringVector tmpGridTerm = term.toToken('_');
							//LAString grid = term.subString(gStartPos, term.size() - 1);
							LAString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];
							LAStringVector gridIdx = grid.toToken('_');
							LAString optionMat = gridIdx[0];
							LAString swapTenor = gridIdx[1];

							// check term
							LAStringVector::const_iterator it = find(optionMatVec.begin(), optionMatVec.end(), optionMat);
							if (it == optionMatVec.end())
							{
								LAString msg = "Shift grid is not in swaption market. grid = " + grid;
								throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
							}
							unsigned int indexOptionMat = static_cast<unsigned int>(it - optionMatVec.begin());

							it = find(swapTenorVec.begin(), swapTenorVec.end(), swapTenor);
							if (it == swapTenorVec.end())
							{
								LAString msg = "Shift grid is not in swaption market. grid = " + grid;
								throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
							}
							unsigned int indexSwapTenor = static_cast<unsigned int>(it - swapTenorVec.begin());

							// shift val
							double shiftVal = param.paraShiftVec[i];
							if (shiftType == RISK_SHIFTTYPE_RATIO)
							{
								shiftVal = swaptionBlackVolMat[indexOptionMat][indexSwapTenor] * shiftVal;
							}
							// add shift val
							swaptionBlackVolMat[indexOptionMat][indexSwapTenor] =
								LAMath::max(swaptionBlackVolMat[indexOptionMat][indexSwapTenor] + shiftVal, 0.0);
						}
					}
				}
			}
		}
		else
		{
			if (param.isGrid && !param.gridShiftVec.empty())
			{
				LAMarketData::shiftGridMarketVol(param, gridPos, &capTermVec, &capTenorVec, &capBlackVolVec, &optionMatVec, &swapTenorVec, &swaptionBlackVolMat);
			}
		}
	}

	// set up market generator
	// cap
	LAMathObjectValue *capGen = 0;
	LAString capGenName = mCalibIDName + "_CAP";
	tmpEh = objPool.getObject(capGenName, ENCHKTYPE_NOCHECK);
	if (!tmpEh.isDefined())
	{
		capGen = new LAMathObjectValue(calibID->getDataInstance());
		objPool.set(capGenName, capGen);
	}
	else
	{
		capGen = &dynamic_cast<LAMathObjectValue &>(tmpEh.get());
		capGen->reset();
	}
	capGen->LAObject::add(LAString("CAP_") + IR_CALIBRATION_DATA_BLACKVOLATILITY, new LADataDoubles(capBlackVolVec));

	// swaption
	LAMathObjectValue *swaptionGen = 0;
	LAString swaptionGenName = mCalibIDName + "_SWAPTION";
	tmpEh = objPool.getObject(swaptionGenName, ENCHKTYPE_NOCHECK);
	if (!tmpEh.isDefined())
	{
		swaptionGen = new LAMathObjectValue(calibID->getDataInstance());
		objPool.set(swaptionGenName, swaptionGen);
	}
	else
	{
		swaptionGen = &dynamic_cast<LAMathObjectValue &>(tmpEh.get());
		swaptionGen->reset();
	}
	swaptionGen->LAObject::add(LAString("SWAPTION_") + IR_CALIBRATION_DATA_BLACKVOLATILITY, new LADataDoubleMatrix(swaptionBlackVolMat));

	marketRef = capGenName + ":" + swaptionGenName;

	//calibration engine
	LAObject *calibEngine = 0;
	LAString calibEName = "CalibEngine_" + mCalibIDName;
	tmpEh = objPool.getObject(calibEName, ENCHKTYPE_NOCHECK);
	if (!tmpEh.isDefined())
	{
		calibEngine = new LAObject();
		objPool.set(calibEName, calibEngine);
	}
	else
	{
		calibEngine = &tmpEh.get();
		calibEngine->reset();
	}
	// name
	calibEngine->add(CALIBRATION_DATA_NAME, new LADataString(calibEName));
	// asOfdate
	calibEngine->add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate));
	// calibration data
	calibEngine->add(CALIBRATION_DATA_CALIBRATIONDATA, new LADataMultiReference()).convertFromString(marketRef);
	// calibration engine
	calibEngine->add(PRICING_DATA_CALIBRATORENGINE, new LADataProcedure()).convertFromString(FN_IR_LMMCALIBRATION_STR);
	// sdeinfo
	calibEngine->add(PRICING_DATA_SDEINFO, new LADataReference()).convertFromString(mCalibIDName);
	// dataout
	if (param.isOutPut)
	{
		calibEngine->add(PRICING_DATA_ISOUTPUT, new LADataBool(true));
	}
	else
	{
		calibEngine->add(PRICING_DATA_ISOUTPUT, new LADataBool(false));
	}
	calibEngine->add("CalibInfoName", new LADataString(calibInfoName));

	// set member
	mpCaibEngine = calibEngine;
	mpCaibEngine->getDataInstance()->getReferencePool().completeDependency();
	
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateLMM set up end" << endl;
}

// 
/*!
    @brief calibrate
*/
void
LACalibrateLMM::doCalibrate()
{
	//calibration start
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateLMM calibrate called.." << endl;
clock_t cstart = clock();

	if (!mpDataInstance)
	{
		throw LACoreInvalidData("DataInstance member is NULL", __FILE__, __LINE__);
	}

	LAObjectPool &objPool = mpDataInstance->getObjectPool();

	LAString fileNum = LACoreDataService::getContext(ARG_KEY_FILENUM);
	if (mSerializeStatus != CALIB_S_DESERIALIZE)
	{
		if (!mpCaibEngine)
		{
			throw LACoreInvalidData("Calibration engine member is NULL", __FILE__, __LINE__);
		}
		// get AsOfDate
		const LADate &asofDate = dynamic_cast<const LADataDate &>(mpCaibEngine->getData(CALIBRATION_DATA_ASOFDATE, ISDEFINED).get()).get();
		LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>(mpCaibEngine->getData(PRICING_DATA_CALIBRATORENGINE, ISNOTNULL).get());
		
		// do calibration !!
		modelDataObj.calibrateModel(asofDate);
	}
	else
	{
		//deserialize
		map<LAString, map<LAString, LAString> >::const_iterator it = mDeserializedEMap.find(mSerializeFile);
		if (it == mDeserializedEMap.end())
		{
			LAString msg = "Serialize stream is not set in mDeserializedEMap. Key = " + mSerializeFile;
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		const map<LAString, LAString> &dataMap = it->second;
		map<LAString, LAString>::const_iterator it_ = dataMap.find(mCalibIDName);
		if (it_ == dataMap.end())
		{
			LAString msg = "Object is not set in serialize stream. Name = " + mCalibIDName;
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		LAStringVector dataVec = it_->second.toToken(',');
		if (dataVec.size() != 5)
		{
			throw LACoreInvalidData("Serialize data size must be 5.", __FILE__, __LINE__);
		}

		LAObject *e = &objPool.getObject(mCalibIDName, ENCHKTYPE_NOCHECK).get();
		// set parameters that have been calibrated already.
		e->add(LAString("Result") + CALIBRATION_DATA_PARAM_V, new LADataDoubles()).convertFromString(dataVec[0]);
		e->add(LAString("Result") + CALIBRATION_DATA_PARAM_F, new LADataDoubles()).convertFromString(dataVec[1]);
		e->add(CALIBRATION_DATA_TENOR_G, new LADataDoubles()).convertFromString(dataVec[2]);
		e->add(LAString("Result") + CALIBRATION_DATA_PARAM_G, new LADataDoubles()).convertFromString(dataVec[3]);
		e->add(PRICING_DATA_CURVEID, new LADataReference()).convertFromString(dataVec[4]);
	}
	// get calib param
	LAObjectHolder calibID = objPool.getObject(mCalibIDName, ENCHKTYPE_ISDEFINED);

	const bool is_extratenor_calib = dynamic_cast<const LADataBool &>(calibID.getData(CALIBRATION_DATA_IS_EXTRATENOR_CALIB, ISDEFINED).get()).get();

	const LADataHolder* dh;
	// calibrated paramV
	const DoubleVector &calibParamV = dynamic_cast<const LADataDoubles &>(calibID.getData(LAString("Result") + CALIBRATION_DATA_PARAM_V, ISDEFINED).get()).get();
	// calibrated paramF
	const DoubleVector &calibParamF = dynamic_cast<const LADataDoubles &>(calibID.getData(LAString("Result") + CALIBRATION_DATA_PARAM_F, ISDEFINED).get()).get();
	// calibrated paramG
	const DoubleVector &tenorG = dynamic_cast<const LADataDoubles &>(calibID.getData(CALIBRATION_DATA_TENOR_G, ISDEFINED).get()).get();
	const DoubleVector &calibParamG = dynamic_cast<const LADataDoubles &>(calibID.getData(LAString("Result") + CALIBRATION_DATA_PARAM_G, ISDEFINED).get()).get();
	// param G information
	const LAString interGStr = dynamic_cast<const LADataString &>(calibID.getData(CALIBRATION_DATA_INTERPOLATION_G, ISNOTNULL).get()).get();
	const LAString frequencyG = dynamic_cast<const LADataString &>(calibID.getData(CALIBRATION_DATA_FREQUENCY_G, ISNOTNULL).get()).get();
	// model swaption vols
	DoubleMatrix swaptionLMMVols;
	dh = &(calibID.getData(LAString("Result") + CALIBRATION_DATA_MODELSWAPTIONVOLS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		swaptionLMMVols = dynamic_cast<const LADataDoubleMatrix &>(dh->get()).get();
	}
	// model capfloor vols
	DoubleVector capLMMVols;
	dh = &(calibID.getData(LAString("Result") + CALIBRATION_DATA_MODELCAPFLOORVOLS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		capLMMVols = dynamic_cast<const LADataDoubles &>(dh->get()).get();
	}
	// model S0, Annuity, OptMaturity
	DoubleMatrix swaptionS0s;
	dh = &(calibID.getData(LAString("Result") + CALIBRATION_DATA_MODELSWAPTION_S0S, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		swaptionS0s = dynamic_cast<const LADataDoubleMatrix &>(dh->get()).get();
	}
	DoubleMatrix swaptionAnnuities;
	dh = &(calibID.getData(LAString("Result") + CALIBRATION_DATA_MODELSWAPTION_ANNUITIES, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		swaptionAnnuities = dynamic_cast<const LADataDoubleMatrix &>(dh->get()).get();
	}
	DoubleMatrix swaptionOptMats;
	dh = &(calibID.getData(LAString("Result") + CALIBRATION_DATA_MODELSWAPTION_OPTMATS, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		swaptionOptMats = dynamic_cast<const LADataDoubleMatrix &>(dh->get()).get();
	}

	// check
	if (calibParamV.size() != 4 || calibParamF.size() != 8)
	{
		throw LACoreInvalidData("Calibrated data format is wrong.", __FILE__, __LINE__);
	}

	// calib param
	const double a = calibParamV[0];
	const double b = calibParamV[1];
	const double c = calibParamV[2];
	const double d = calibParamV[3];
	const double decay  = calibParamF[0];
	const double amp1   = calibParamF[1];
	const double phase1 = calibParamF[2];
	const double amp2   = calibParamF[3];
	const double phase2 = calibParamF[4];
	const double amp3   = calibParamF[5];
	const double phase3 = calibParamF[6];
	const double shift  = calibParamF[7];

	const double tmax = dynamic_cast<const LADataDouble &>(calibID.getData(CALIBRATION_DATA_MAXTERM, ISNOTNULL).get()).get();
	const LAString canonicalFreq = dynamic_cast<const LADataString &>(calibID.getData(PRICING_DATA_CALIBCANONICAL_FREQ, ISNOTNULL).get()).get();
	const DoubleVector &tenor = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get()).get();
	const DoubleVector &delta_tenor = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_CALIBCANONICAL_T_DELTATENOR, ISNOTNULL).get()).get();
	const DoubleVector &tenor_30_360 = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_CALIBCANONICAL_T_30_360, ISNOTNULL).get()).get();
	const BoolVector &tenor_extraflag = dynamic_cast<const LADataBools &>(calibID.getData(PRICING_DATA_CALIBCANONICAL_T_EXTRAFLAG, ISNOTNULL).get()).get();

	const double skew = dynamic_cast<const LADataDouble &>(calibID.getData(CALIBRATION_DATA_VOL_SKEW, ISNOTNULL).get()).get();

	// set vol
	const double qval = 1.0 / (1.0 - LAMath::log(skew) / LAMath::log(2.0));

	unsigned int exCounter = 0;
	for ( size_t i = 0; i < tenor.size(); ++i)
	{
		if (tenor_extraflag[i]) 
		{
			++exCounter;
		}
	}
	const size_t extG_size = tenor.size() - exCounter - 1;
	DoubleVector extG(extG_size);
	if (frequencyG == canonicalFreq)
	{
		extG = calibParamG;
	}
	else
	{
		std::shared_ptr<LAInterpolationBase> interG = LAMathInterpolationUtilities::createInterpolation(interGStr);
		interG->set(tenorG, calibParamG);

		for ( size_t i = 0; i < extG_size; ++i )
		{
			extG[i] = interG->value(tenor[i + exCounter + 1]);
		}
	}

	const double f_adjParam = extG[0] * qval;

	for (unsigned int i = 0; i < tenor.size() - 2; ++i)
	{
		LAMathVolFuncLMM *vol = NULL;
		LAFunctionBase *funcTerm = new LAMathVolFuncWave(tmax, decay, amp1, phase1, 
														amp2, phase2, amp3, phase3, shift);
		LAFunctionBase *funcTenor = new LAMathVolFuncStructureBase(a, b, c, d);

		double adjParam = 0.0;
		if (is_extratenor_calib)
		{
			adjParam = extG[i] * qval;
		}
		else
		{
			if (tenor_extraflag[i + 1])
			{
				adjParam = f_adjParam;
			}
			else
			{
				adjParam = extG[i - exCounter] * qval;
			}
		}

		vol = new LAMathVolFuncLMM(funcTenor, funcTerm, tenor, tenor_30_360, i + 1, adjParam, true);

		mpFunc[i].setRealFunction(*vol);
		mpFunc[i].setOn();

		delete vol;
	}

	// set object pool as calib data
	const LAString &yieldDataName = dynamic_cast<const LADataReference &>(calibID.getData(PRICING_DATA_CURVEID, ISNOTNULL).get()).get().getName();
	LAString calibDataName = LAMarketData::getCalibDataName(mCalcType, yieldDataName, mGridPos);
	cout << static_cast<int>(LACoreThread::getThreadID()) << " CalibDataName = " << calibDataName << endl;
	if (!objPool.getObject(calibDataName, ENCHKTYPE_NOCHECK).isDefined())
	{
		LAObject *calibData = new LAObject();
		calibData->add(CALIBRATION_DATA_NAME, new LADataString(calibDataName));
		calibData->add(CALIBRATION_DATA_PARAM_V, new LADataDoubles(calibParamV));
		calibData->add(CALIBRATION_DATA_PARAM_F, new LADataDoubles(calibParamF));
		calibData->add(CALIBRATION_DATA_TENOR_G, new LADataDoubles(tenorG));
		calibData->add(CALIBRATION_DATA_PARAM_G, new LADataDoubles(calibParamG));
		calibData->add(CALIBRATION_DATA_INTERPOLATION_G, new LADataString(interGStr));
		calibData->add(PRICING_DATA_CALIBCANONICAL_T, new LADataDoubles(tenor));
		calibData->add(PRICING_DATA_CALIBCANONICAL_T_DELTATENOR, new LADataDoubles(delta_tenor));
		calibData->add(PRICING_DATA_CALIBCANONICAL_T_30_360, new LADataDoubles(tenor_30_360));
		calibData->add(CALIBRATION_DATA_MODELSWAPTIONVOLS, new LADataDoubleMatrix(swaptionLMMVols));
		calibData->add(CALIBRATION_DATA_MODELCAPFLOORVOLS, new LADataDoubles(capLMMVols));
		calibData->add(CALIBRATION_DATA_MODELSWAPTION_S0S, new LADataDoubleMatrix(swaptionS0s));
		calibData->add(CALIBRATION_DATA_MODELSWAPTION_ANNUITIES, new LADataDoubleMatrix(swaptionAnnuities));
		calibData->add(CALIBRATION_DATA_MODELSWAPTION_OPTMATS, new LADataDoubleMatrix(swaptionOptMats));

		objPool.set(calibDataName, calibData);
	}

	if (mSerializeStatus != CALIB_S_DESERIALIZE)
	{
		// if serialize dump
		if (mSerializeStatus == CALIB_S_SERIALIZE)
		{
			///// mod Precision for grid
			//LADataDoubles tmp;
			LADataDoubles tmp(22);
			// paramV
			tmp.set(calibParamV);
			LAString calibParamVStr = tmp.convertToString();
			// paramF
			tmp.set(calibParamF);
			LAString calibParamFStr = tmp.convertToString();
			// tenorg
			tmp.set(tenorG);
			LAString calibTenorGStr = tmp.convertToString();
			// paramg
			tmp.set(calibParamG);
			LAString calibParamGStr = tmp.convertToString();
			// set serialize
			setmSerializeMap(mCalibIDName + "," + calibParamVStr + "," + calibParamFStr + "," + calibTenorGStr + "," + calibParamGStr + "," + yieldDataName);
			istringstream *dataStream = new istringstream(mSerializeMap[mSerializeFile].getCString());
			LACoreDataService::setIStringStream(mSerializeFile, dataStream);
		}

		//ouput
		const LADataHolder &attr = mpCaibEngine->getData(PRICING_DATA_ISOUTPUT, NOCHECK);
		if (attr.isDefined() && !attr.isNull())
		{
			if (dynamic_cast<const LADataBool &>(attr.get()).get())
			{
				ofstream fout;
				LAString fileName = mCalibIDName + ".csv";
				fout.open(fileName.getCString());
				for (unsigned int i = 0; i < calibParamV.size(); i++)
				{
					fout << LAString(calibParamV[i]).getCString() << endl;
				}
				for (unsigned int i = 0; i < calibParamF.size(); i++)
				{
					fout << LAString(calibParamF[i]).getCString() << endl;
				}
				for (unsigned int i = 0; i < calibParamG.size(); i++)
				{
					fout << LAString(calibParamG[i]).getCString() << endl;
				}
				fout.close();
			}
		}

		// delete data
		LADataMultiReference &calibDataRef = dynamic_cast<LADataMultiReference &>(mpCaibEngine->getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
		unsigned int dataSize = calibDataRef.getSize();
		for (unsigned int i = 0; i < dataSize; ++i)
		{
			calibDataRef.get(i).clear();
		}
		mpCaibEngine->clear();
	}
	calibID.clear();

clock_t cend = clock();
double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateLMM calibrate end.." << endl;
cout << "-> time = " << time << endl;

}
