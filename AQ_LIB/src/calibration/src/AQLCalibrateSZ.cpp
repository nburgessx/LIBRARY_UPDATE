/*! @file
@brief SZ calibration request
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrateSZ.cpp
//
//  DESCRIPTION :        SZ calibration request class
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
#include "AQLCalibrateSZ.h"
#include "AQLCalibrationFunc.h"
#include "AQLDefinitionsCalibration.h"
#include "AQLMarketData.h"
#include "AQLCoreAppError.h"
#include "AQLObjectPool.h"
#include "AQLCoreReferencePool.h"
#include "AQLDataReference.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLDataProcedure.h"
#include "AQLStepInterpolation.h"
#include "AQLConstant.h"
#include "AQLScenarioConfiguration.h"
#include "AQLCoreDataService.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLMathJamshidianSwaption.h"
#include "AQLMathJamshidianSwaptionByImplyVol.h"
#include "AQLMathAntonovFXOption.h"
#include "AQLPriceSZCalibration.h"
#include "AQLMathAntonovFXOptionBetaFixed.h"
#include "AQLMathAntonovFXOptionVolatilityFixed.h"
#include "AQLMathDateUtilities.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLMathIRVanillaFuncUtility.h"
#include "AQLMathValuableEntity.h"
#include "AQLPriceHWCalibration.h"
#include "AQLMathAntonovFXOption.h"
#include "AQLMathVolFuncFX.h"
#include "AQLMathVolFuncFXDD.h"
#include "AQLPriceFXVolatility.h"
#include "AQLMathDateCalculations.h"
#include "AQLMathFXVanillaFuncUtility.h"
#include "AQLCoreDataService.h"
#include <sstream>
#include "AQLCalibratePool.h"
#include "AQLMathAnalyticalFormula.h"
#include "AQLMathAntonovFXOption.h"
#include "AQLCalibrateVolatilitySZ.h"
#include "AQLCalibrateSZ.h"
#include "AQLLogManager.h"
#include "AQLLogger.h"
#include "AQLMathVolFuncSZDD.h"

using namespace std;
// constructor
/*!

*/
AQLCalibrateSZ::AQLCalibrateSZ()
{
}

// destructor
/*!

*/
AQLCalibrateSZ::~AQLCalibrateSZ()
{
}


// 
/*!
@brief setup
*/
void
AQLCalibrateSZ::setUp(AQLObjectPool &objPool, const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos)
{
	setUp2(objPool, param, method, gridPos);
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateSZ set up called" << endl;
	// deserialize mode
	if (mSerializeStatus == CALIB_S_DESERIALIZE)
	{
		return;
	}

	if (!method)
	{
		throw AQLCoreInvalidData("Argument method is NULL", __FILE__, __LINE__);
	}
	if (param.refName.size() < 2)
	{
		throw AQLCoreInvalidData("Param refName is more than be two, reference curve and calibinfo needed.", __FILE__, __LINE__);
	}
	if (!mpCaibEngine)
	{
		throw AQLCoreInvalidData("mpCaibEngine is NULL.", __FILE__, __LINE__);
	}
	// calibration engine
	mpCaibEngine->remove(PRICING_DATA_CALIBRATORENGINE);
	mpCaibEngine->add(PRICING_DATA_CALIBRATORENGINE, new AQLDataProcedure()).convertFromString(FN_IR_SZCALIBRATION_STR);

	const AQLString &calibInfoName = param.refName[0];
	const AQLObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();

	mpCaibEngine->remove(PRICING_DATA_DOMESTICCURVETYPE);
	mpCaibEngine->add(PRICING_DATA_DOMESTICCURVETYPE, calibInfo.getData(PRICING_DATA_DOMESTICCURVETYPE, ISNOTNULL).get().clone());

	mpCaibEngine->remove(PRICING_DATA_FOREIGNCURVETYPE);
	mpCaibEngine->add(PRICING_DATA_FOREIGNCURVETYPE, calibInfo.getData(PRICING_DATA_FOREIGNCURVETYPE, ISNOTNULL).get().clone());

	const AQLString &calibEngineName = dynamic_cast<const AQLDataString &>(mpCaibEngine->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

	// set up parameters info
	const AQLString SZ = "_SZ";
	const AQLString infoName = calibEngineName + SZ;
	AQLObject *pInfo = 0;
	AQLObjectHolder tmpEh = objPool.getObject(infoName, ENCHKTYPE_NOCHECK);
	if (!tmpEh.isDefined())
	{
		pInfo = new AQLObject();
		objPool.set(infoName, pInfo);
	}
	else
	{
		pInfo = &tmpEh.get();
		pInfo->reset();
	}

	// set boundary
	pInfo->remove(CALIBRATION_DATA_OPT_BETA_FLAG);
	pInfo->add(CALIBRATION_DATA_OPT_BETA_FLAG, calibInfo.getData(CALIBRATION_DATA_OPT_BETA_FLAG, ISNOTNULL).get().clone());
	
	pInfo->remove(CALIBRATION_DATA_OPT_THETA_FLAG);
	pInfo->add(CALIBRATION_DATA_OPT_THETA_FLAG, calibInfo.getData(CALIBRATION_DATA_OPT_THETA_FLAG, ISNOTNULL).get().clone());
	
	pInfo->remove(CALIBRATION_DATA_OPT_KAPPA_FLAG);
	pInfo->add(CALIBRATION_DATA_OPT_KAPPA_FLAG, calibInfo.getData(CALIBRATION_DATA_OPT_KAPPA_FLAG, ISNOTNULL).get().clone());
	
	pInfo->remove(CALIBRATION_DATA_OPT_EPSILON_FLAG);
	pInfo->add(CALIBRATION_DATA_OPT_EPSILON_FLAG, calibInfo.getData(CALIBRATION_DATA_OPT_EPSILON_FLAG, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_BOUNDARY_MAX_BETA);
	pInfo->add(CALIBRATION_DATA_BOUNDARY_MAX_BETA, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_BETA, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_BOUNDARY_MIN_BETA);
	pInfo->add(CALIBRATION_DATA_BOUNDARY_MIN_BETA, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_BETA, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_BOUNDARY_MAX_THETA);
	pInfo->add(CALIBRATION_DATA_BOUNDARY_MAX_THETA, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_THETA, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_BOUNDARY_MIN_THETA);
	pInfo->add(CALIBRATION_DATA_BOUNDARY_MIN_THETA, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_THETA, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_BOUNDARY_MAX_KAPPA);
	pInfo->add(CALIBRATION_DATA_BOUNDARY_MAX_KAPPA, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_KAPPA, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_BOUNDARY_MIN_KAPPA);
	pInfo->add(CALIBRATION_DATA_BOUNDARY_MIN_KAPPA, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_KAPPA, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_BOUNDARY_MAX_EPSILON);
	pInfo->add(CALIBRATION_DATA_BOUNDARY_MAX_EPSILON, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_EPSILON, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_BOUNDARY_MIN_EPSILON);
	pInfo->add(CALIBRATION_DATA_BOUNDARY_MIN_EPSILON, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_EPSILON, ISNOTNULL).get().clone());


	// other params
	pInfo->remove(CALIBRATION_DATA_MAX_ITERATION);
	pInfo->add(CALIBRATION_DATA_MAX_ITERATION, calibInfo.getData(CALIBRATION_DATA_MAX_ITERATION, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION);
	pInfo->add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, calibInfo.getData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_ROOT_EPSILON);
	pInfo->add(CALIBRATION_DATA_ROOT_EPSILON, calibInfo.getData(CALIBRATION_DATA_ROOT_EPSILON, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_FUNCTION_EPSILON);
	pInfo->add(CALIBRATION_DATA_FUNCTION_EPSILON, calibInfo.getData(CALIBRATION_DATA_FUNCTION_EPSILON, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_GRADIENT_NORM_EPSILON);
	pInfo->add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, calibInfo.getData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_OPT_PARAMETER_FLAG);
	pInfo->add(CALIBRATION_DATA_OPT_PARAMETER_FLAG, calibInfo.getData(CALIBRATION_DATA_OPT_PARAMETER_FLAG, ISNOTNULL).get().clone());

	pInfo->remove(CALIBRATION_DATA_OPT_METHOD_TYPE);
	pInfo->add(CALIBRATION_DATA_OPT_METHOD_TYPE, calibInfo.getData(CALIBRATION_DATA_OPT_METHOD_TYPE, ISNOTNULL).get().clone());

	pInfo->remove(PRICING_DATA_FXVOLGRIDS);
	pInfo->add(PRICING_DATA_FXVOLGRIDS, calibInfo.getData(PRICING_DATA_FXVOLGRIDS, ISNOTNULL).get().clone());

	// set reference
	mpCaibEngine->remove(PRICING_DATA_SZPARAMSCALIBINFO);
	mpCaibEngine->add(PRICING_DATA_SZPARAMSCALIBINFO, new AQLDataReference()).convertFromString(infoName);

	// asofdate
	AQLDate asofDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	const unsigned int MARKETNUM = 5;
	// check calib input type
	AQLString calibInputType = dynamic_cast<const AQLDataString &>(calibInfo.getData(PRICING_DATA_CALIBINPUTPARAMTYPE, ISNOTNULL).get()).get();
	calibInputType.toUpper();
	if (calibInputType != CALIB_INPUTTYPE_ALL)
	{
		throw AQLCoreInvalidData("Calib input param type must be ALL", __FILE__, __LINE__);
	}

	// set DIFFVOLV0 = ATM VOL - InitialValue
	mpCaibEngine->remove(PRICING_DATA_DIFFVOLV0);
	mpCaibEngine->add(PRICING_DATA_DIFFVOLV0, calibInfo.getData(PRICING_DATA_DIFFVOLV0, ISNOTNULL).get().clone());

	AQLDataMultiReference &refData = dynamic_cast<AQLDataMultiReference &>(mpCaibEngine->getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
	for (unsigned int i = 0; i < refData.getSize(); ++i)
	{
		AQLMathObjectValue &data = dynamic_cast<AQLMathObjectValue &>(refData.get(i).get());
		const double term = dynamic_cast<const AQLDataDouble &>(data.getData(PRICING_DATA_FXTERM, ISNOTNULL).get()).get();
		const double dDF = dynamic_cast<const AQLDataDouble &>(data.getData(PRICING_DATA_DOMESTICDF, ISNOTNULL).get()).get();
		const double fDF = dynamic_cast<const AQLDataDouble &>(data.getData(PRICING_DATA_FOREIGNDF, ISNOTNULL).get()).get();
		const double fwdFX = dynamic_cast<const AQLDataDouble &>(data.getData(PRICING_DATA_FORWARDFX, ISNOTNULL).get()).get();
		const DoubleVector &volVec = dynamic_cast<const AQLDataDoubles &>(data.getData(PRICING_DATA_VOLATILITYS, ISNOTNULL).get()).get();
		const DoubleVector &strikeVec = dynamic_cast<const AQLDataDoubles &>(data.getData(PRICING_DATA_STRIKES, ISNOTNULL).get()).get();
		const BoolVector &isCallVec = dynamic_cast<const AQLDataBools &>(data.getData(PRICING_DATA_ISCALLS, ISNOTNULL).get()).get();
		const DoubleVector &weightVec = dynamic_cast<const AQLDataDoubles &>(data.getData(PRICING_DATA_WEIGHTS, ISNOTNULL).get()).get();
		const double &InitialBeta = dynamic_cast<const AQLDataDouble &>(data.getData(PRICING_DATA_INITIALBETA, ISNOTNULL).get()).get();
		const double &InitialTheta = dynamic_cast<const AQLDataDouble &>(data.getData(PRICING_DATA_INITIALTHETA, ISNOTNULL).get()).get();
		const double &InitialKappa = dynamic_cast<const AQLDataDouble &>(data.getData(PRICING_DATA_INITIALKAPPA, ISNOTNULL).get()).get();
		const double &InitialEpsilon = dynamic_cast<const AQLDataDouble &>(data.getData(PRICING_DATA_INITIALEPSILON, ISNOTNULL).get()).get();

		if (volVec.size() != MARKETNUM)
		{
			AQLString msg = "volVec size is wrong. size = " + AQLString(static_cast<int>(volVec.size()));
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		if (strikeVec.size() != MARKETNUM)
		{
			AQLString msg = "strikeVec size is wrong. size = " + AQLString(static_cast<int>(strikeVec.size()));
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		if (isCallVec.size() != MARKETNUM)
		{
			AQLString msg = "isCallVec size is wrong. size = " + AQLString(static_cast<int>(isCallVec.size()));
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		// calc premium
		DoubleArray premVec(MARKETNUM);

		// ATM
		premVec[0] = AQLMathAnalyticalFormula::BlackFormula(fwdFX, volVec[0] * AQLMath::sqrt(term), strikeVec[0], isCallVec[0] ? 1 : -1) * dDF;
		// 25Delta High
		premVec[1] = AQLMathAnalyticalFormula::BlackFormula(fwdFX, volVec[1] * AQLMath::sqrt(term), strikeVec[1], isCallVec[1] ? 1 : -1) * dDF;
		// 10Delta High
		premVec[2] = AQLMathAnalyticalFormula::BlackFormula(fwdFX, volVec[2] * AQLMath::sqrt(term), strikeVec[2], isCallVec[2] ? 1 : -1) * dDF;
		// 25Delta Low
		premVec[3] = AQLMathAnalyticalFormula::BlackFormula(fwdFX, volVec[3] * AQLMath::sqrt(term), strikeVec[3], isCallVec[3] ? 1 : -1) * dDF;
		// 10Delta Low 
		premVec[4] = AQLMathAnalyticalFormula::BlackFormula(fwdFX, volVec[4] * AQLMath::sqrt(term), strikeVec[4], isCallVec[4] ? 1 : -1) * dDF;
		//set data 
		DoubleArray premVec_(MARKETNUM);
		DoubleArray strikeVec_(MARKETNUM);
		DoubleArray weightVec_(MARKETNUM);
		BoolVector isCallVec_(MARKETNUM);
		DoubleArray volVec_(MARKETNUM);
		// 10Delta Low 
		premVec_[0] = premVec[4];
		strikeVec_[0] = strikeVec[4];
		weightVec_[0] = weightVec[4];
		isCallVec_[0] = isCallVec[4];
		volVec_[0] = volVec[4];
		// 25Delta Low 
		premVec_[1] = premVec[3];
		strikeVec_[1] = strikeVec[3];
		weightVec_[1] = weightVec[3];
		isCallVec_[1] = isCallVec[3];
		volVec_[1] = volVec[3];
		// ATM
		premVec_[2] = premVec[0];
		strikeVec_[2] = strikeVec[0];
		weightVec_[2] = weightVec[0];
		isCallVec_[2] = isCallVec[0];
		volVec_[2] = volVec[0];
		// 25Delta High
		premVec_[3] = premVec[1];
		strikeVec_[3] = strikeVec[1];
		weightVec_[3] = weightVec[1];
		isCallVec_[3] = isCallVec[1];
		volVec_[3] = volVec[1];
		// 10Delta High
		premVec_[4] = premVec[2];
		strikeVec_[4] = strikeVec[2];
		weightVec_[4] = weightVec[2];
		isCallVec_[4] = isCallVec[2];
		volVec_[4] = volVec[2];
		//set as data
		data.AQLObject::remove(PRICING_DATA_OPTIONPREMIUMS);
		data.AQLObject::add(PRICING_DATA_OPTIONPREMIUMS, new AQLDataDoubles(premVec_));
		data.AQLObject::remove(PRICING_DATA_STRIKES);
		data.AQLObject::add(PRICING_DATA_STRIKES, new AQLDataDoubles(strikeVec_));
		data.AQLObject::remove(PRICING_DATA_WEIGHTS);
		data.AQLObject::add(PRICING_DATA_WEIGHTS, new AQLDataDoubles(weightVec_));
		data.AQLObject::remove(PRICING_DATA_ISCALLS);
		data.AQLObject::add(PRICING_DATA_ISCALLS, new AQLDataBools(isCallVec_));
		data.AQLObject::remove(PRICING_DATA_VOLATILITYS);
		data.AQLObject::add(PRICING_DATA_VOLATILITYS, new AQLDataDoubles(volVec_));

		//set initial parameters
		data.AQLObject::remove(PRICING_DATA_INITIALBETA);
		data.AQLObject::add(PRICING_DATA_INITIALBETA, new AQLDataDouble(InitialBeta));
		data.AQLObject::remove(PRICING_DATA_INITIALTHETA);
		data.AQLObject::add(PRICING_DATA_INITIALTHETA, new AQLDataDouble(InitialTheta));
		data.AQLObject::remove(PRICING_DATA_INITIALKAPPA);
		data.AQLObject::add(PRICING_DATA_INITIALKAPPA, new AQLDataDouble(InitialKappa));
		data.AQLObject::remove(PRICING_DATA_INITIALEPSILON);
		data.AQLObject::add(PRICING_DATA_INITIALEPSILON, new AQLDataDouble(InitialEpsilon));
	}
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " MACalibrationSZ set up end" << endl;
}

void
AQLCalibrateSZ::setUp2(AQLObjectPool &objPool, const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos)
{
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateSZ set up called" << endl;
	if (!method)
	{
		throw AQLCoreInvalidData("Argument method is NULL", __FILE__, __LINE__);
	}
	if (param.refName.size() < 2)
	{
		throw AQLCoreInvalidData("Param refName is more than be two, reference curve and calibinfo needed.", __FILE__, __LINE__);
	}
	// set method
	mpFunc = method;
	const AQLString &calibInfoName = param.refName[0];
	const AQLObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
	AQLObject *calibID = 0;

	unsigned int curPos = 1;
	if (gridPos >= 0)
	{
		curPos += 4 * gridPos;
		if (param.isParallel)
		{
			curPos = curPos + 4;
		}
	}
	// get domestic curve
	const AQLString dYieldName = param.refName[curPos];
	if (!objPool.getObject(dYieldName).isDefined())
	{
		AQLString msg = dYieldName + " is not registered in EntityPool";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const AQLMathYieldCurve &dYield = dynamic_cast<const AQLMathYieldCurve &>(objPool.getObject(dYieldName, ENCHKTYPE_ISDEFINED).get());
	const AQLString &dYieldDataName = dYield.getYieldData().get().getName();

	// get foreign curve
	const AQLString fYieldName = param.refName[curPos + 2];
	if (!objPool.getObject(fYieldName).isDefined())
	{
		AQLString msg = fYieldName + " is not registered in EntityPool";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const AQLMathYieldCurve &fYield = dynamic_cast<const AQLMathYieldCurve &>(objPool.getObject(fYieldName, ENCHKTYPE_ISDEFINED).get());
	const AQLString &fYieldDataName = fYield.getYieldData().get().getName();

	// calc df ratio
	mTimeGrid = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get()).get();
	unsigned int tSize = mTimeGrid.size();
	mDFRatios.resize(tSize);
	for (unsigned int i = 0; i < tSize; ++i)
	{
		mDFRatios[i] = fYield.getBasisDF(mTimeGrid[i]) / dYield.getBasisDF(mTimeGrid[i]);
	}

	// set calcType
	mCalcType = param.calcType;
	// set gridPos
	mGridPos = gridPos;																																																																																																																																																																							
	// set isLJ
	mIsLJ = param.isLJ;
	// set isDDL
	mIsDDL = param.isDDL;
	// serialize status
	mSerializeStatus = param.serializeStatus;
	// serialize file
	mSerializeFile = param.serializeFile;
	// set dataInstance
	mpDataInstance = calibInfo.getDataInstance();
	mCalibIDName = param.calcType + "_FXCalibInfoEntity_" + dYieldDataName + "_" + fYieldDataName + "_" + AQLString(gridPos);
	//set FXCurrency
	mFXCurrency = param.ccy;

	mSpotRate = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(PRICING_DATA_SPOTFX, NOCHECK).get()).get();
	AQLDate spotDate = dynamic_cast<const AQLDataDate &>(calibInfo.getData(IR_CALIBRATION_DATA_SPOTDATE, NOCHECK).get()).get();
	//add extra base param
	mSpotRate += param.extraBaseParam;
	if (!param.extraParam.empty())
	{
		// shiftval check
		if (param.extraParam.size() != 1)
		{
			throw AQLCoreInvalidData("Parallel shift val size must be 1 .", __FILE__, __LINE__);
		}
		double shiftVal = param.extraParam[0];
		AQLString shiftType = param.shiftType;
		if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
		{
			shiftVal = mSpotRate * shiftVal;
		}
		// add shift val
		mSpotRate = AQLMath::max(mSpotRate + shiftVal, 0.0);
	}
	// asofdate
	AQLDate asofDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	// today base
	mSpotRate *= dYield.getBasisDF(asofDate, spotDate) / fYield.getBasisDF(asofDate, spotDate);

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

	// get calibration id
	AQLObjectHolder tmpEh = objPool.getObject(mCalibIDName, ENCHKTYPE_NOCHECK);
	if (!tmpEh.isDefined())
	{
		calibID = new AQLObject();
		objPool.set(mCalibIDName, calibID);
	}
	else
	{
		calibID = &tmpEh.get();
		calibID->clear();
	}
	calibID->add(CALIBRATION_DATA_NAME, new AQLDataString(mCalibIDName));
	// add fx time grid
	calibID->add(PRICING_DATA_FXTIMEGRIDS, calibInfo.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get().clone());

	// set calib variable
	calibID->add(PRICING_DATA_CALIBVARIABLES, calibInfo.getData(PRICING_DATA_CALIBVARIABLES, ISNOTNULL).get().clone());

	AQLString calibInputType = dynamic_cast<const AQLDataString &>(calibInfo.getData(PRICING_DATA_CALIBINPUTPARAMTYPE, ISNOTNULL).get()).get();
	calibInputType.toUpper();
	// is make calib from vol
	bool isMCFVol = false;
	if (calibInputType == CALIB_INPUTTYPE_ALL)
	{
		isMCFVol = false;
	}
	else if (calibInputType == CALIB_INPUTTYPE_VOLONLY)
	{
		isMCFVol = true;
	}
	else
	{
		AQLString msg = "Input of InputParamType is wrong Input = " + calibInputType;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// optionmaturity
	const AQLStringVector &optionMatVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
	// vol 10Delta Low
	DoubleVector vol10DLVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOL10DL, ISNOTNULL).get()).get();
	// vol 25Delta Low
	DoubleVector vol25DLVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOL25DL, ISNOTNULL).get()).get();
	// vol ATM
	DoubleVector volATMVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOLATM, ISNOTNULL).get()).get();
	// vol 25Delta High
	DoubleVector vol25DHVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOL25DH, ISNOTNULL).get()).get();
	// vol 10Delta Hight
	DoubleVector vol10DHVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOL10DH, ISNOTNULL).get()).get();
	
	double basevol = param.extraBaseVolParam;
	for (unsigned int k = 0;k < vol10DLVec.size();k++)
	{
		vol10DLVec[k] += basevol;
		vol25DLVec[k] += basevol;
		volATMVec[k] += basevol;
		vol25DHVec[k] += basevol;
		vol10DHVec[k] += basevol;
	}

	// shift black vol
	if (gridPos < 0)
	{
		// parallel case
		if (param.isParallel && !param.paraShiftVec.empty())
		{
			DoubleVector vol25BFVec; // 25ButterFly
			DoubleVector vol10BFVec; // 10ButterFly
			DoubleVector vol25RRVec; // 25RiskReversal
			DoubleVector vol10RRVec; // 10RiskReversal
									 // convert to market format
			AQLMarketData::convFXVolCalib2Market(volATMVec, vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec,
				vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec);

			AQLString shiftType = param.shiftType;
			shiftType.toUpper();
			if (param.paraShiftVec.size() == 1)
			{
				DoubleArray shiftVal(vol10DLVec.size(), param.paraShiftVec[0]);
				if (shiftType == RISK_SHIFTTYPE_DIFF)
				{
					// diff case
					transform(volATMVec.begin(), volATMVec.end(), shiftVal.begin(), volATMVec.begin(), plus<double>());
				}
				else
				{
					// ratio case
					DoubleArray plusVal(volATMVec.size());
					transform(volATMVec.begin(), volATMVec.end(), shiftVal.begin(), plusVal.begin(), multiplies<double>());
					transform(volATMVec.begin(), volATMVec.end(), plusVal.begin(), volATMVec.begin(), plus<double>());
				}

				// check zero floor
				unsigned int size = vol10DLVec.size();
				for (unsigned int i = 0; i < size; ++i)
				{
					volATMVec[i] = AQLMath::max(volATMVec[i], 0.0);
				}

			}
			else
			{
				unsigned int paraSize = param.paraTerm.size();
				if (paraSize != param.paraTerm.size())
				{
					throw AQLCoreInvalidData("Parallel shift val and value is not same ", __FILE__, __LINE__);
				}

				for (unsigned int i = 0; i < paraSize; ++i)
				{
					AQLString term = param.paraTerm[i];
					term.toUpper();
					AQLStringVector termVec = term.toToken('_');
					if (termVec.size() != 2)
					{
						AQLString msg = "Term format is is wrong, term = " + term;
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					AQLStringVector::const_iterator it = find(optionMatVec.begin(), optionMatVec.end(), termVec[1]);
					if (it == optionMatVec.end())
					{
						AQLString msg = "Term is not in volatility file, term = " + termVec[1];
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int indx = static_cast<unsigned int>(it - optionMatVec.begin());
					// set shift val
					DoubleVector *pTargetVol = 0;
					if (termVec[0] == FXVOL_ATM)
					{
						pTargetVol = &volATMVec;
					}
					else if (termVec[0] == FXVOL_25BF)
					{
						pTargetVol = &vol25BFVec;
					}
					else if (termVec[0] == FXVOL_10BF)
					{
						pTargetVol = &vol10BFVec;
					}
					else if (termVec[0] == FXVOL_25RR)
					{
						pTargetVol = &vol25RRVec;
					}
					else if (termVec[0] == FXVOL_10RR)
					{
						pTargetVol = &vol10RRVec;
					}
					else
					{
						AQLString msg = "This FXVol grid is not support = " + termVec[0];
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}

					// shift val
					double shiftVal = param.paraShiftVec[i];
					if (shiftType == RISK_SHIFTTYPE_RATIO)
					{
						shiftVal = (*pTargetVol)[indx] * shiftVal;
					}
					if (termVec[0] == FXVOL_ATM)
					{
						// add shift val
						(*pTargetVol)[indx] = AQLMath::max((*pTargetVol)[indx] + shiftVal, 0.0);
					}
					else
					{
						// add shift val
						(*pTargetVol)[indx] = (*pTargetVol)[indx] + shiftVal;
					}
				}
			}
			// reconvert to calib format
			AQLMarketData::convFXVolMarket2Calib(volATMVec, vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec,
				vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec);
		}
	}
	else
	{
		if (param.isGrid && !param.gridShiftVec.empty())
		{
			// grid shift case
			// check
			if (param.gridTerm.size() - 1 < static_cast<unsigned int>(gridPos) ||
				param.gridShiftVec.size() - 1 < static_cast<unsigned int>(gridPos))
			{
				AQLString msg = "Grid shift param is not exist, grid num = " + AQLString(gridPos);
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			DoubleVector vol25BFVec; // 25ButterFly
			DoubleVector vol10BFVec; // 10ButterFly
			DoubleVector vol25RRVec; // 25RiskReversal
			DoubleVector vol10RRVec; // 10RiskReversal
			// convert to market format
			AQLMarketData::convFXVolCalib2Market(volATMVec, vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec,
				vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec);

			AQLString term = param.gridTerm[gridPos];
			term.toUpper();

			AQLStringVector termVec = term.toToken('_');
			if (termVec.size() != 2)
			{
				AQLString msg = "Term format is is wrong, term = " + term;
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			AQLStringVector::const_iterator it = find(optionMatVec.begin(), optionMatVec.end(), termVec[1]);
			if (it == optionMatVec.end())
			{
				AQLString msg = "Term is not in volatility file, term = " + termVec[1];
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			unsigned int indx = static_cast<unsigned int>(it - optionMatVec.begin());
			// set shift val
			DoubleVector *pTargetVol = 0;
			if (termVec[0] == FXVOL_ATM)
			{
				pTargetVol = &volATMVec;
			}
			else if (termVec[0] == FXVOL_25BF)
			{
				pTargetVol = &vol25BFVec;
			}
			else if (termVec[0] == FXVOL_10BF)
			{
				pTargetVol = &vol10BFVec;
			}
			else if (termVec[0] == FXVOL_25RR)
			{
				pTargetVol = &vol25RRVec;
			}
			else if (termVec[0] == FXVOL_10RR)
			{
				pTargetVol = &vol10RRVec;
			}
			else
			{
				AQLString msg = "This FXVol grid is not support = " + termVec[0];
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			// shift val
			if (!param.isWave)
			{
				IntArray pos;
				for (unsigned int k = 0;k < param.gridGroupID.size() - 1;++k)
				{
					if (param.gridGroupID[k] != param.gridGroupID[k + 1])
					{
						pos.push_back(k);
						continue;
					}
					if (k == param.gridGroupID.size() - 2 && param.gridGroupID[k + 1] != 0)
						pos.push_back(param.gridGroupID.size() - 1);
				}
				if (pos.size() != 0)
				{
					IntArray::iterator it;
					it = find(pos.begin(), pos.end(), gridPos);
					unsigned int tmppos = static_cast<unsigned int>(it - pos.begin());
					unsigned int gridPosfrom;
					if (tmppos == 0)
						gridPosfrom = 0;
					else
						gridPosfrom = pos[tmppos - 1] + 1;

					unsigned int num = gridPos - gridPosfrom + 1;
					for (unsigned int k = 0;k < num;++k)
					{
						// shift val
						double shiftVal = param.gridShiftVec[gridPosfrom + k];
						AQLString shiftType = param.shiftType;
						if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
						{
							shiftVal = (*pTargetVol)[gridPosfrom + k] * shiftVal;
						}

						if (termVec[0] == FXVOL_ATM)
						{
							// add shift val
							(*pTargetVol)[gridPosfrom + k] = AQLMath::max((*pTargetVol)[gridPosfrom + k] + shiftVal, 0.0);
						}
						else
						{
							// add shift val
							(*pTargetVol)[gridPosfrom + k] = (*pTargetVol)[gridPosfrom + k] + shiftVal;
						}
					}
				}
				else
				{
					// shift val
					double shiftVal = param.gridShiftVec[gridPos];
					AQLString shiftType = param.shiftType;
					if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
					{
						shiftVal = (*pTargetVol)[indx] * shiftVal;
					}
					// add shift val
					if (termVec[0] == FXVOL_ATM)
						(*pTargetVol)[indx] = AQLMath::max((*pTargetVol)[indx] + shiftVal, 0.0);
					else
						(*pTargetVol)[indx] = (*pTargetVol)[indx] + shiftVal;
				}
			}
			else
			{
				for (unsigned int k = 0;k <= static_cast<unsigned int>(gridPos);k++)
				{
					// shift val
					double shiftVal = param.gridShiftVec[k];
					AQLString term = param.gridTerm[k];
					term.toUpper();

					AQLStringVector termVec = term.toToken('_');
					if (termVec.size() != 2)
					{
						AQLString msg = "Term format is is wrong, term = " + term;
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					AQLStringVector::const_iterator it = find(optionMatVec.begin(), optionMatVec.end(), termVec[1]);
					if (it == optionMatVec.end())
					{
						AQLString msg = "Term is not in volatility file, term = " + termVec[1];
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int tmpindx = static_cast<unsigned int>(it - optionMatVec.begin());

					AQLString shiftType = param.shiftType;
					if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
					{
						shiftVal = (*pTargetVol)[tmpindx] * shiftVal;
					}

					if (termVec[0] == FXVOL_ATM)
					{
						// add shift val
						(*pTargetVol)[tmpindx] = AQLMath::max((*pTargetVol)[tmpindx] + shiftVal, 0.0);
					}
					else
					{
						// add shift val
						(*pTargetVol)[tmpindx] = (*pTargetVol)[tmpindx] + shiftVal;
					}

				}
			}
			// reconvert to calib format
			AQLMarketData::convFXVolMarket2Calib(volATMVec, vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec,
				vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec);
		}
	}
	// stike fwd
	const AQLStringVector &strikeFwdVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_STRIKEFWDTERM, ISNOTNULL).get()).get();

	// fixingcalendar
	const AQLStringVector &calVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_FIXINGCALENDAR, ISNOTNULL).get()).get();
	// slidingrule
	const AQLStringVector &slidingVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get()).get();
	// spot or fwd
	const AQLStringVector &soptOrFwdVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_SPOTORFWD, ISNOTNULL).get()).get();

	// optiontype ATM
	const AQLStringVector &opTypeATMVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPEATM, ISNOTNULL).get()).get();
	// optiontype 25Delta High
	const AQLStringVector &opType25DHVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPE25DH, ISNOTNULL).get()).get();
	// optiontype 10Delta Hight
	const AQLStringVector &opType10DHVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPE10DH, ISNOTNULL).get()).get();
	// optiontype 25Delta Low
	const AQLStringVector &opType25DLVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPE25DL, ISNOTNULL).get()).get();
	// optiontype 10Delta Low
	const AQLStringVector &opType10DLVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPE10DL, ISNOTNULL).get()).get();

	// weight ATM
	const DoubleVector &weightATMVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_WEIGHTATM, ISNOTNULL).get()).get();
	// weight 25Delta High
	const DoubleVector &weight25DHVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_WEIGHT25DH, ISNOTNULL).get()).get();
	// weight 10Delta Hight
	const DoubleVector &weight10DHVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_WEIGHT10DH, ISNOTNULL).get()).get();
	// weight 25Delta Low
	const DoubleVector &weight25DLVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_WEIGHT25DL, ISNOTNULL).get()).get();
	// weight 10Delta Low
	const DoubleVector &weight10DLVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_WEIGHT10DL, ISNOTNULL).get()).get();
	
	// initial params
	const DoubleVector &initialBetaVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_INITIALBETA, ISNOTNULL).get()).get();
	const DoubleVector &initialThetaVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_INITIALTHETA, ISNOTNULL).get()).get();
	const DoubleVector &initialKappaVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_INITIALKAPPA, ISNOTNULL).get()).get();
	const DoubleVector &initialEpsilonVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_INITIALEPSILON, ISNOTNULL).get()).get();
	
	// TODO merget latest version strike
	// at merge delete literal
	// strikeval 25Delta High
	const DoubleVector &strikeVal25DHVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_STRIKEVALES + AQLString("25DH"), ISNOTNULL).get()).get();
	// strikeval 10Delta Hight
	const DoubleVector &strikeVal10DHVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_STRIKEVALES + AQLString("10DH"), ISNOTNULL).get()).get();
	// strikeval 25Delta Low
	const DoubleVector &strikeVal25DLVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_STRIKEVALES + AQLString("25DL"), ISNOTNULL).get()).get();
	// strikeval 10Delta Low
	const DoubleVector &strikeVal10DLVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_STRIKEVALES + AQLString("10DL"), ISNOTNULL).get()).get();


	// is fit atm
	const BoolVector &isFitATMVec = dynamic_cast<const AQLDataBools &>(calibInfo.getData(PRICING_DATA_ISFITATM, ISNOTNULL).get()).get();
	// is delta neutral
	const BoolVector &isDeltaNeutVec = dynamic_cast<const AQLDataBools &>(calibInfo.getData(PRICING_DATA_ISDELTANEUTRAL, ISNOTNULL).get()).get();

	// strikeStr
	const unsigned int MARKETNUM = 5;
	AQLStringVector strStrikeVec(MARKETNUM);
	strStrikeVec[0] = FXVOL_ATM_STRIKE;
	strStrikeVec[1] = FXVOL_25DHIGH_STRIKE;
	strStrikeVec[2] = FXVOL_10DHIGH_STRIKE;
	strStrikeVec[3] = FXVOL_25DLOW_STRIKE;
	strStrikeVec[4] = FXVOL_10DLOW_STRIKE;

	BoolVector isCallVec(MARKETNUM, false);
	isCallVec[0] = true;
	isCallVec[1] = true;
	isCallVec[2] = true;
	// attr for calc term
	const AQLPriceDataCalendar &termCal = dynamic_cast<const AQLPriceDataCalendar &>(calibInfo.getData(PRICING_DATA_TERMCALENDAR, ISNOTNULL).get());
	const AQLString strTermCal = termCal.convertToString();
	const AQLPriceDataSlidingRule &termSliding = dynamic_cast<const AQLPriceDataSlidingRule &>(calibInfo.getData(PRICING_DATA_TERMSLIDINGRULE, ISNOTNULL).get());
	const AQLPriceDataDayCount &termDC = dynamic_cast<const AQLPriceDataDayCount &>(calibInfo.getData(PRICING_DATA_TERMDAYCOUNT, ISNOTNULL).get());
	const bool isIncludeLast = dynamic_cast<const AQLDataBool &>(calibInfo.getData(PRICING_DATA_TERMISINCLUDELAST, ISNOTNULL).get()).get();
	// spotdate for calc term
	int spotlag = dynamic_cast<const AQLDataInt &>(calibInfo.getData(PRICING_DATA_TERMSPOTLAG, ISNOTNULL).get());
	AQLDate optionSpotDate = AQLMathDateCalculations::getFXSpotDate(param.ccy, asofDate, strTermCal, spotlag, true);
	// need to calc strike and prem
	AQLString strLow("low");
	AQLString strHigh("high");
	AQLString strPrem("Prem");
	AQLString strBuy("Buy");

	AQLString marketRef;
	unsigned int gridSize = optionMatVec.size();
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		AQLString mGenName = mCalibIDName + "_" + optionMatVec[i];
		AQLMathObjectValue *mGen = 0;
		tmpEh = objPool.getObject(mGenName, ENCHKTYPE_NOCHECK);
		if (!tmpEh.isDefined())
		{
			mGen = new AQLMathObjectValue(calibID->getDataInstance());
			objPool.set(mGenName, mGen);
		}
		else
		{
			mGen = &dynamic_cast<AQLMathObjectValue &>(tmpEh.get());
			mGen->reset();
		}
		// set data
		// set name
		mGen->getName().convertFromString(mGenName);
		// set value
		//mGen->setValuationMethod(valueStr);
		// set model param
		mGen->AQLObject::add(PRICING_DATA_MODELPARAM, new AQLDataReference()).convertFromString(mCalibIDName);
		// set is fitatm
		mGen->AQLObject::add(PRICING_DATA_ISFITATM, new AQLDataBool(isFitATMVec[i]));
		// set sliding rule
		mGen->AQLObject::add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingVec[i]);
		// set spotlag
		mGen->AQLObject::add(IR_CALIBRATION_DATA_SPOTLAG, new AQLDataInt(spotlag));
		// set key FX name (ex. JPY/USD)
		mGen->AQLObject::add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString(param.ccy));
		// set fixing calendar
		mGen->AQLObject::add(PRICING_DATA_FIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(calVec[i]);
		// set fixing calendar
		mGen->AQLObject::add(PRICING_DATA_TERMCALENDAR, new AQLPriceDataCalendar(termCal));
		// set option maturity
		mGen->AQLObject::add(IR_CALIBRATION_DATA_OPTIONMATURITY, new AQLDataString(optionMatVec[i]));
		// set params
		mGen->AQLObject::add(PRICING_DATA_INITIALBETA, new AQLDataDouble(initialBetaVec[i]));
		mGen->AQLObject::add(PRICING_DATA_INITIALTHETA, new AQLDataDouble(initialThetaVec[i]));
		mGen->AQLObject::add(PRICING_DATA_INITIALKAPPA, new AQLDataDouble(initialKappaVec[i]));
		mGen->AQLObject::add(PRICING_DATA_INITIALEPSILON, new AQLDataDouble(initialEpsilonVec[i]));

		// weight
		DoubleArray weightVec(MARKETNUM);
		weightVec[0] = weightATMVec[i];
		weightVec[1] = weight25DHVec[i];
		weightVec[2] = weight10DHVec[i];
		weightVec[3] = weight25DLVec[i];
		weightVec[4] = weight10DLVec[i];
		mGen->AQLObject::add(PRICING_DATA_WEIGHTS, new AQLDataDoubles(weightVec));
		// vol
		DoubleArray volVec(MARKETNUM);
		volVec[0] = volATMVec[i];
		volVec[1] = vol25DHVec[i];
		volVec[2] = vol10DHVec[i];
		volVec[3] = vol25DLVec[i];
		volVec[4] = vol10DLVec[i];
		mGen->AQLObject::add(PRICING_DATA_VOLATILITYS, new AQLDataDoubles(volVec));
		// strike
		mGen->AQLObject::add(PRICING_DATA_STRIKESTRINGS, new AQLDataStrings(strStrikeVec));
		// isCall
		mGen->AQLObject::add(PRICING_DATA_ISCALLS, new AQLDataBools(isCallVec));

		// is make calib from vol
		mGen->AQLObject::add(PRICING_DATA_ISMAKECALIBDATAFROMVOLATILITY, new AQLDataBool(isMCFVol));
		if (isMCFVol)
		{
			// is delta neutral
			mGen->AQLObject::add(PRICING_DATA_ISDELTANEUTRAL, new AQLDataBool(isDeltaNeutVec[i]));
			// spot fwd
			mGen->AQLObject::add(PRICING_DATA_SPOTORFWD, new AQLDataString(soptOrFwdVec[i]));
		}
		else
		{
			// calc strike and optiontype
			AQLDate settledate = AQLMathDateCalculations::getDate(optionSpotDate, optionMatVec[i], termSliding, &termCal, true);
			AQLDate date = AQLMathDateCalculations::getFXSpotDate(param.ccy, settledate, strTermCal, -spotlag, true);
			const double term = termDC.getTerm(asofDate, date, isIncludeLast);
			const double dDF = dYield.getBasisDF(term);
			const double fDF = fYield.getBasisDF(term);
			const double fwdFX = mSpotRate * fDF / dDF;

			mGen->AQLObject::add(PRICING_DATA_FXTERM, new AQLDataDouble(term));
			mGen->AQLObject::add(PRICING_DATA_DOMESTICDF, new AQLDataDouble(dDF));
			mGen->AQLObject::add(PRICING_DATA_FOREIGNDF, new AQLDataDouble(fDF));
			mGen->AQLObject::add(PRICING_DATA_FORWARDFX, new AQLDataDouble(fwdFX));

			const double reversal = 0.0;
			const double strangle = 0.0;
			AQLString spotOrFwd = soptOrFwdVec[i];

			// calc strike
			DoubleArray strikeVec(MARKETNUM);
			// ATM
			AQLStringVector::const_iterator it = find(strikeFwdVec.begin(), strikeFwdVec.end(), optionMatVec[i]);
			if (it == strikeFwdVec.end())
			{
				strikeVec[0] = fwdFX * AQLMath::exp(-0.5 * AQLMath::pow(volATMVec[i], 2) * term);
			}
			else
			{
				strikeVec[0] = fwdFX;
			}

			// 25Delta High
			try
			{
				strikeVec[1] = AQLMathFXVanillaFuncUtility::calcstrikefromdelta
					(strikeVal25DHVec[i], spotOrFwd, strHigh, mSpotRate, fwdFX, vol25DHVec[i], reversal, strangle, fDF, term);
			}
			catch (AQLCoreError &err)
			{
				AQLString msg = err.getMsg();
				if (msg.findString("Not Convergence from rtsafe") == -1)
					throw AQLCoreNumericalError("Fx Option convergence does not work", __FILE__, __LINE__);

				strikeVec[1] = AQLMathFXVanillaFuncUtility::calcmaxstrike
					(spotOrFwd, mSpotRate, fwdFX, vol25DHVec[i], reversal, strangle, fDF, term);
			}
			// 10Delta High
			try
			{
				strikeVec[2] = AQLMathFXVanillaFuncUtility::calcstrikefromdelta
					(strikeVal10DHVec[i], spotOrFwd, strHigh, mSpotRate, fwdFX, vol10DHVec[i], reversal, strangle, fDF, term);
			}
			catch (AQLCoreError &err)
			{
				AQLString msg = err.getMsg();
				if (msg.findString("Not Convergence from rtsafe") == -1)
					throw AQLCoreNumericalError("Fx Option convergence does not work", __FILE__, __LINE__);

				strikeVec[2] = AQLMathFXVanillaFuncUtility::calcmaxstrike
					(spotOrFwd, mSpotRate, fwdFX, vol10DHVec[i], reversal, strangle, fDF, term);
			}
			// 25Delta Low
			strikeVec[3] = AQLMathFXVanillaFuncUtility::calcstrikefromdelta
				(strikeVal25DLVec[i], spotOrFwd, strLow, mSpotRate, fwdFX, vol25DLVec[i], reversal, strangle, fDF, term);
			// 10Delta Low
			strikeVec[4] = AQLMathFXVanillaFuncUtility::calcstrikefromdelta
				(strikeVal10DLVec[i], spotOrFwd, strLow, mSpotRate, fwdFX, vol10DLVec[i], reversal, strangle, fDF, term);
			//set as data
			mGen->AQLObject::add(PRICING_DATA_STRIKES, new AQLDataDoubles(strikeVec));

			//calc premium
			DoubleArray premVec(MARKETNUM);
			// ATM
			AQLString tmpOpType = opTypeATMVec[i];
			premVec[0] = AQLMathIRVanillaFuncUtility::bkOption(strPrem, strBuy, tmpOpType, fwdFX, strikeVec[0], volATMVec[i], dDF, asofDate, date);
			// 25Delta High
			tmpOpType = opType25DHVec[i];
			premVec[1] = AQLMathIRVanillaFuncUtility::bkOption(strPrem, strBuy, tmpOpType, fwdFX, strikeVec[1], vol25DHVec[i], dDF, asofDate, date);
			// 10Delta High
			tmpOpType = opType10DHVec[i];
			premVec[2] = AQLMathIRVanillaFuncUtility::bkOption(strPrem, strBuy, tmpOpType, fwdFX, strikeVec[2], vol10DHVec[i], dDF, asofDate, date);
			// 25Delta Low
			tmpOpType = opType25DLVec[i];
			premVec[3] = AQLMathIRVanillaFuncUtility::bkOption(strPrem, strBuy, tmpOpType, fwdFX, strikeVec[3], vol25DLVec[i], dDF, asofDate, date);
			// 10Delta Low
			tmpOpType = opType10DLVec[i];
			premVec[4] = AQLMathIRVanillaFuncUtility::bkOption(strPrem, strBuy, tmpOpType, fwdFX, strikeVec[4], vol10DLVec[i], dDF, asofDate, date);
			//set as data
			mGen->AQLObject::add(PRICING_DATA_OPTIONPREMIUMS, new AQLDataDoubles(premVec));


		}

		marketRef += mGenName + ":";
	}

	marketRef = marketRef.subString(0, marketRef.size() - 2);

	//calibrationengin
	AQLObject *calibEngine = 0;
	AQLString calibEName = "CalibEngine_" + mCalibIDName;
	tmpEh = objPool.getObject(calibEName, ENCHKTYPE_NOCHECK);
	if (!tmpEh.isDefined())
	{
		calibEngine = new AQLObject();
		objPool.set(calibEName, calibEngine);
	}
	else
	{
		calibEngine = &tmpEh.get();
		calibEngine->reset();
	}
	// name
	calibEngine->add(CALIBRATION_DATA_NAME, new AQLDataString(calibEName));
	// asofdate
	calibEngine->add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asofDate));
	// calibration data
	calibEngine->add(CALIBRATION_DATA_CALIBRATIONDATA, new AQLDataMultiReference()).convertFromString(marketRef);
	// calibration engine
	calibEngine->add(PRICING_DATA_CALIBRATORENGINE, new AQLDataProcedure()).convertFromString(FN_IR_SZCALIBRATION_STR);
	// sdeinfo
	calibEngine->add(PRICING_DATA_SDEINFO, new AQLDataReference()).convertFromString(mCalibIDName);
	// spotrate
	calibEngine->add(PRICING_DATA_SPOTFX, new AQLDataDouble(mSpotRate));

	// set domestic curve
	calibEngine->add(PRICING_DATA_DOMESTICCURVE, new AQLDataReference()).convertFromString(dYieldDataName);
	// set domestic model
	AQLString dIRModelName = param.refName[curPos + 1];
	calibEngine->add(PRICING_DATA_DOMESTICIRMODEL, new AQLDataReference()).convertFromString(dIRModelName);
	// use df2 for domestic curve
	bool isDF2forDome = false;
	AQLObjectHolder ehD = objPool.getObject(dYieldDataName, ENCHKTYPE_ISDEFINED);
	const AQLDataHolder &ahD = ehD.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK);
	if (ahD.isDefined() && !ahD.isNull())
	{
		isDF2forDome = true;
	}
	calibEngine->add(PRICING_DATA_ISUSEDF2FORDOMESTIC, new AQLDataBool(isDF2forDome));

	// set foregin curve
	calibEngine->add(PRICING_DATA_FOREIGNCURVE, new AQLDataReference()).convertFromString(fYieldDataName);
	// set foregin model
	AQLString fIRModelName = param.refName[curPos + 3];
	calibEngine->add(PRICING_DATA_FOREIGNIRMODEL, new AQLDataReference()).convertFromString(fIRModelName);
	// use df2 for foregin curve
	bool isDF2forFore = false;
	AQLObjectHolder ehF = objPool.getObject(fYieldDataName, ENCHKTYPE_ISDEFINED);
	const AQLDataHolder &ahF = ehF.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK);
	if (ahF.isDefined() && !ahF.isNull())
	{
		isDF2forFore = true;
	}
	calibEngine->add(PRICING_DATA_ISUSEDF2FORFOREIGN, new AQLDataBool(isDF2forFore));

	// set sde vol correlation
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(param.ccy, ccys);
	ccys.push_back(param.ccy);
	ccys.push_back(param.ccy + "_VOL");
	DoubleMatrix corMtx;
	AQLMarketData::getSDECorrelation(ccys, corMtx);
	double cor_vol;
	cor_vol = corMtx[2][3];
	calibEngine->add(PRICING_DATA_SDEVOLCORRELATIONS, new AQLDataDouble(cor_vol));

	// dataout
	if (param.isOutPut)
	{
		calibEngine->add(PRICING_DATA_ISOUTPUT, new AQLDataBool(true));
	}
	else
	{
		calibEngine->add(PRICING_DATA_ISOUTPUT, new AQLDataBool(false));
	}
	// set member
	mpCaibEngine = calibEngine;
	// check is calib end ir model
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " DomesticIRModelName = " << dIRModelName << " : ForeignIRModelName = " << fIRModelName << endl;
	if (!AQLMarketData::isCalibrateEnd(dIRModelName, objPool) || !AQLMarketData::isCalibrateEnd(fIRModelName, objPool))
	{
		throw AQLCoreInvalidData("IR model calibration is not end ..", __FILE__, __LINE__);
	}
	mpCaibEngine->getDataInstance()->getReferencePool().completeDependency();

	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateSZ set up end" << endl;
}

// 
/*!
@brief calibrate
*/
void
AQLCalibrateSZ::doCalibrate()
{
	//calibration start
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateSZ calibrate called.." << endl;
	clock_t cstart = clock();

	if (!mpDataInstance)
	{
		throw AQLCoreInvalidData("DataInstance member is NULL", __FILE__, __LINE__);
	}

	AQLObjectPool &objPool = mpDataInstance->getObjectPool();

	AQLString fileNum = AQLCoreDataService::getContext(ARG_KEY_FILENUM);
	if (mSerializeStatus != CALIB_S_DESERIALIZE)
	{
		if (!mpCaibEngine)
		{
			throw AQLCoreInvalidData("Calibration engine member is NULL", __FILE__, __LINE__);
		}
		// get AsOfDate
		const AQLDate &asofDate = dynamic_cast<const AQLDataDate &>(mpCaibEngine->getData(CALIBRATION_DATA_ASOFDATE, ISDEFINED).get()).get();
		AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>(mpCaibEngine->getData(PRICING_DATA_CALIBRATORENGINE, ISNOTNULL).get());

		// do calibration !!
		modelDataObj.calibrateModel(asofDate);

		// print error messages
		AQLDataHolder &dh = mpCaibEngine->getData(CALIBRATION_DATA_ERRORMESSAGES, NOCHECK);
		if (dh.isDefined() && !dh.isNull())
		{
			const AQLStringVector &msgs = dynamic_cast<AQLDataStrings &>(dh.get()).get();
			AQLLogger &logger = AQLCoreDataService::getLogManager().getLogger();
			for (unsigned int i = 0; i < msgs.size(); ++i)
			{
				logger.error(msgs[i], __FILE__, __LINE__);
			}
		}
	}
	else
	{
		//deserialize
		map<AQLString, map<AQLString, AQLString> >::const_iterator it = mDeserializedEMap.find(mSerializeFile);
		if (it == mDeserializedEMap.end())
		{
			AQLString msg = "Serialize stream is not set in mDeserializedEMap. Key = " + mSerializeFile;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		const map<AQLString, AQLString> &dataMap = it->second;
		map<AQLString, AQLString>::const_iterator it_ = dataMap.find(mCalibIDName);
		if (it_ == dataMap.end())
		{
			AQLString msg = "Object is not set in serialize stream. Name = " + mCalibIDName;
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		AQLStringVector dataVec = it_->second.toToken(',');
		if (dataVec.size() != 6)
		{
			throw AQLCoreInvalidData("Serialize data size must be 6.", __FILE__, __LINE__);
		}

		AQLObject *e = new AQLObject();
		e->add(CALIBRATION_DATA_NAME, new AQLDataString(mCalibIDName));
		e->add(PRICING_DATA_FXTIMEGRIDS, new AQLDataDoubles()).convertFromString(dataVec[0]);
		e->add(PRICING_DATA_OPTIMIZEDBETA, new AQLDataDoubles()).convertFromString(dataVec[1]);
		e->add(PRICING_DATA_OPTIMIZEDTHETA, new AQLDataDoubles()).convertFromString(dataVec[2]);
		e->add(PRICING_DATA_OPTIMIZEDKAPPA, new AQLDataDoubles()).convertFromString(dataVec[3]);
		e->add(PRICING_DATA_OPTIMIZEDEPSILON, new AQLDataDoubles()).convertFromString(dataVec[4]);
		e->add(PRICING_DATA_VOLV0, new AQLDataDoubles()).convertFromString(dataVec[5]);

		objPool.set(mCalibIDName, e);

	}

	// get calib param
	AQLObjectHolder calibID = objPool.getObject(mCalibIDName, ENCHKTYPE_ISDEFINED);

	//set FXcurrency
	AQLString fxccy = mFXCurrency;
	fxccy.toUpper();
	// tenor
	const DoubleArray &eTimeGrid = dynamic_cast<const AQLDataDoubles &>(calibID.getData(PRICING_DATA_FXTIMEGRIDS, ISDEFINED).get()).get();
	// optimized parameters
	const DoubleArray &betaVec = dynamic_cast<const AQLDataDoubles &>(calibID.getData(PRICING_DATA_OPTIMIZEDBETA, ISDEFINED).get()).get();
	const DoubleArray &thetaVec = dynamic_cast<const AQLDataDoubles &>(calibID.getData(PRICING_DATA_OPTIMIZEDTHETA, ISDEFINED).get()).get();
	const DoubleArray &kappaVec = dynamic_cast<const AQLDataDoubles &>(calibID.getData(PRICING_DATA_OPTIMIZEDKAPPA, ISDEFINED).get()).get();
	const DoubleArray &epsilonVec = dynamic_cast<const AQLDataDoubles &>(calibID.getData(PRICING_DATA_OPTIMIZEDEPSILON, ISDEFINED).get()).get();
	//initial vol
	const DoubleArray &v0Vec = dynamic_cast<const AQLDataDoubles &>(calibID.getData(PRICING_DATA_VOLV0, ISDEFINED).get()).get();
	
	unsigned int vecSize = mTimeGrid.size();
	// check
	if (vecSize != eTimeGrid.size() || vecSize != betaVec.size() || vecSize != thetaVec.size() || vecSize != kappaVec.size() || vecSize != epsilonVec.size())
	{
		throw AQLCoreInvalidData("Calibrated data format is wroing.", __FILE__, __LINE__);
	}
	const double INFINITESIMAL = 1E-7;
	for (unsigned int i = 0; i <vecSize; ++i)
	{
		if (AQLMath::abs(mTimeGrid[i] - eTimeGrid[i]) > INFINITESIMAL)
		{
			AQLString msg = "CanonicTimeGrid = " + AQLString(mTimeGrid[i]) + ", EntityTimeGrid = " + AQLString(eTimeGrid[i]);
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}

	DoubleArray fwdVec(vecSize);
	// check
	if (vecSize != mDFRatios.size())
	{
		throw AQLCoreInvalidData("DF ratio size and timegrid size is not same.", __FILE__, __LINE__);
	}
	// calc forward fx
	for (unsigned int i = 0; i < vecSize; ++i)
	{
		fwdVec[i] = mSpotRate * mDFRatios[i];
	}
	
	AQLFunctionBase *method = new AQLMathVolFuncSZDD(mTimeGrid, fwdVec, betaVec, thetaVec, kappaVec, epsilonVec, fxccy, v0Vec);

	mpFunc->setRealFunction(*method);
	mpFunc->setOn();

	delete method;

	if (mSerializeStatus != CALIB_S_DESERIALIZE)
	{
		// if serialize dump
		if (mSerializeStatus == CALIB_S_SERIALIZE)
		{
			///// mod Precision for grid
			//AQLDataDoubles tmp;
			AQLDataDoubles tmp(22);
			// time grid
			tmp.set(eTimeGrid);
			AQLString eTimeGridStr = tmp.convertToString();
			// ParametersVec
			tmp.set(betaVec);
			AQLString betaVecStr = tmp.convertToString();
			tmp.set(thetaVec);
			AQLString thetaVecStr = tmp.convertToString();
			tmp.set(kappaVec);
			AQLString kappaVecStr = tmp.convertToString();
			tmp.set(epsilonVec);
			AQLString epsilonVecStr = tmp.convertToString();
			// volVec
			tmp.set(v0Vec);
			AQLString volVecStr = tmp.convertToString();
			// set serialize
			setmSerializeMap(mCalibIDName + "," + eTimeGridStr + "," + betaVecStr + "," + thetaVecStr + "," + kappaVecStr + "," + epsilonVecStr + "," + volVecStr);
			istringstream *dataStream = new istringstream(mSerializeMap[mSerializeFile].getCString());
			AQLCoreDataService::setIStringStream(mSerializeFile, dataStream);

		}

		//ouput
		const AQLDataHolder &attr = mpCaibEngine->getData(PRICING_DATA_ISOUTPUT, NOCHECK);
		if (attr.isDefined() && !attr.isNull())
		{
			if (dynamic_cast<const AQLDataBool &>(attr.get()).get())
			{
				ifstream fin;
				ofstream fout;
				const AQLString dirName = AQLCoreDataService::getOutputDirectory();
				AQLString fileName = mCalibIDName.subString(0, 100) + ".csv";
				fileName.exchange("/", "");
				fileName = dirName + fileName;
				fin.open(fileName.getCString());
				if (!fin)
				{
					fout.open(fileName.getCString());
					unsigned int size = mTimeGrid.size();
					for (unsigned int i = 0; i < size; ++i)
					{
						fout << AQLString(mTimeGrid[i]) << "," << AQLString(betaVec[i]) << "," << AQLString(thetaVec[i]) << "," << AQLString(kappaVec[i]) << "," << AQLString(epsilonVec[i]) << "," << AQLString(v0Vec[i]) << "," << AQLString(fwdVec[i]) << endl;
					}
					fout.close();
				}
				else
				{
					DoubleArray t_timeGrid;
					DoubleArray t_betaVec;
					DoubleArray t_thetaVec;
					DoubleArray t_kappaVec;
					DoubleArray t_epsilonVec;
					DoubleArray t_volVec;
					DoubleArray t_fwdVec;
					string line;
					while (getline(fin, line))
					{
						const char *c_line = line.c_str();
						AQLStringVector lineVec = AQLString(c_line).toToken(MARKET_DATA_DELIMITER);
						if (lineVec.size() != 7)
						{
							throw AQLCoreInvalidData("SZ DD Calib format is wrong", __FILE__, __LINE__);
						}
						t_timeGrid.push_back(lineVec[0].trimLeft().trimRight().getDoubleValue());
						t_betaVec.push_back(lineVec[1].trimLeft().trimRight().getDoubleValue());
						t_thetaVec.push_back(lineVec[2].trimLeft().trimRight().getDoubleValue());
						t_kappaVec.push_back(lineVec[3].trimLeft().trimRight().getDoubleValue());
						t_epsilonVec.push_back(lineVec[4].trimLeft().trimRight().getDoubleValue());
						t_volVec.push_back(lineVec[5].trimLeft().trimRight().getDoubleValue());
						t_fwdVec.push_back(lineVec[6].trimLeft().trimRight().getDoubleValue());
					}
					// check DDL
					AQLMathVolFuncFX *t_funcFX = 0;
					t_funcFX = new AQLMathVolFuncSZDD(t_timeGrid, t_fwdVec, t_betaVec, t_thetaVec, t_kappaVec, t_epsilonVec, fxccy, t_volVec);
					mpFunc->setRealFunction(*t_funcFX);
					delete t_funcFX;
				}
				fin.close();
				
			}
		}
		// delete data
		AQLDataMultiReference &calibData = dynamic_cast<AQLDataMultiReference &>(mpCaibEngine->getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
		unsigned int dataSize = calibData.getSize();
		for (unsigned int i = 0; i < dataSize; ++i)
		{
			calibData.get(i).clear();
		}

		mpCaibEngine->clear();

	}
	// calibID.clear();

	clock_t cend = clock();
	double time = (double)(cend - cstart) / CLOCKS_PER_SEC;
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateSZ calibrate end.." << endl;
	cout << "-> time = " << time << endl;
}
