/*! @file
@brief SZ calibration request
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateSZ.cpp
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
#include "LACalibrateSZ.h"
#include "LACalibrationFunc.h"
#include "LADefinitionsCalibration.h"
#include "LAMarketData.h"
#include "LACoreAppError.h"
#include "LAObjectPool.h"
#include "LACoreReferencePool.h"
#include "LADataReference.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LADataProcedure.h"
#include "LAStepInterpolation.h"
#include "LAConstant.h"
#include "LAScenarioConfiguration.h"
#include "LACoreDataService.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAMathJamshidianSwaption.h"
#include "LAMathJamshidianSwaptionByImplyVol.h"
#include "LAMathAntonovFXOption.h"
#include "LAPriceSZCalibration.h"
#include "LAMathAntonovFXOptionBetaFixed.h"
#include "LAMathAntonovFXOptionVolatilityFixed.h"
#include "LAMathDateUtilities.h"
#include "LAMathCurveFuncUtility.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathValuableEntity.h"
#include "LAPriceHWCalibration.h"
#include "LAMathAntonovFXOption.h"
#include "LAMathVolFuncFX.h"
#include "LAMathVolFuncFXDD.h"
#include "LAPriceFXVolatility.h"
#include "LAMathDateCalculations.h"
#include "LAMathFXVanillaFuncUtility.h"
#include "LACoreDataService.h"
#include <sstream>
#include "LACalibratePool.h"
#include "LAMathAnalyticalFormula.h"
#include "LAMathAntonovFXOption.h"
#include "LACalibrateVolatilitySZ.h"
#include "LACalibrateSZ.h"
#include "LALogManager.h"
#include "LALogger.h"
#include "LAMathVolFuncSZDD.h"

using namespace std;
// constructor
/*!

*/
LACalibrateSZ::LACalibrateSZ()
{
}

// destructor
/*!

*/
LACalibrateSZ::~LACalibrateSZ()
{
}


// 
/*!
@brief setup
*/
void
LACalibrateSZ::setUp(LAObjectPool &objPool, const MAScenarioParam &param, MACalibrationFunc *method, int gridPos)
{
	setUp2(objPool, param, method, gridPos);
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateSZ set up called" << endl;
	// deserialize mode
	if (mSerializeStatus == CALIB_S_DESERIALIZE)
	{
		return;
	}

	if (!method)
	{
		throw LACoreInvalidData("Argument method is NULL", __FILE__, __LINE__);
	}
	if (param.refName.size() < 2)
	{
		throw LACoreInvalidData("Param refName is more than be two, reference curve and calibinfo needed.", __FILE__, __LINE__);
	}
	if (!mpCaibEngine)
	{
		throw LACoreInvalidData("mpCaibEngine is NULL.", __FILE__, __LINE__);
	}
	// calibration engine
	mpCaibEngine->remove(PRICING_DATA_CALIBRATORENGINE);
	mpCaibEngine->add(PRICING_DATA_CALIBRATORENGINE, new LADataProcedure()).convertFromString(FN_IR_SZCALIBRATION_STR);

	const LAString &calibInfoName = param.refName[0];
	const LAObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();

	mpCaibEngine->remove(PRICING_DATA_DOMESTICCURVETYPE);
	mpCaibEngine->add(PRICING_DATA_DOMESTICCURVETYPE, calibInfo.getData(PRICING_DATA_DOMESTICCURVETYPE, ISNOTNULL).get().clone());

	mpCaibEngine->remove(PRICING_DATA_FOREIGNCURVETYPE);
	mpCaibEngine->add(PRICING_DATA_FOREIGNCURVETYPE, calibInfo.getData(PRICING_DATA_FOREIGNCURVETYPE, ISNOTNULL).get().clone());

	const LAString &calibEngineName = dynamic_cast<const LADataString &>(mpCaibEngine->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

	// set up parameters info
	const LAString SZ = "_SZ";
	const LAString infoName = calibEngineName + SZ;
	LAObject *pInfo = 0;
	LAObjectHolder tmpEh = objPool.getObject(infoName, ENCHKTYPE_NOCHECK);
	if (!tmpEh.isDefined())
	{
		pInfo = new LAObject();
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
	mpCaibEngine->add(PRICING_DATA_SZPARAMSCALIBINFO, new LADataReference()).convertFromString(infoName);

	// asofdate
	LADate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	const unsigned int MARKETNUM = 5;
	// check calib input type
	LAString calibInputType = dynamic_cast<const LADataString &>(calibInfo.getData(PRICING_DATA_CALIBINPUTPARAMTYPE, ISNOTNULL).get()).get();
	calibInputType.toUpper();
	if (calibInputType != CALIB_INPUTTYPE_ALL)
	{
		throw LACoreInvalidData("Calib input param type must be ALL", __FILE__, __LINE__);
	}

	// set DIFFVOLV0 = ATM VOL - InitialValue
	mpCaibEngine->remove(PRICING_DATA_DIFFVOLV0);
	mpCaibEngine->add(PRICING_DATA_DIFFVOLV0, calibInfo.getData(PRICING_DATA_DIFFVOLV0, ISNOTNULL).get().clone());

	LADataMultiReference &refData = dynamic_cast<LADataMultiReference &>(mpCaibEngine->getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
	for (unsigned int i = 0; i < refData.getSize(); ++i)
	{
		LAMathObjectValue &data = dynamic_cast<LAMathObjectValue &>(refData.get(i).get());
		const double term = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_FXTERM, ISNOTNULL).get()).get();
		const double dDF = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_DOMESTICDF, ISNOTNULL).get()).get();
		const double fDF = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_FOREIGNDF, ISNOTNULL).get()).get();
		const double fwdFX = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_FORWARDFX, ISNOTNULL).get()).get();
		const DoubleVector &volVec = dynamic_cast<const LADataDoubles &>(data.getData(PRICING_DATA_VOLATILITYS, ISNOTNULL).get()).get();
		const DoubleVector &strikeVec = dynamic_cast<const LADataDoubles &>(data.getData(PRICING_DATA_STRIKES, ISNOTNULL).get()).get();
		const BoolVector &isCallVec = dynamic_cast<const LADataBools &>(data.getData(PRICING_DATA_ISCALLS, ISNOTNULL).get()).get();
		const DoubleVector &weightVec = dynamic_cast<const LADataDoubles &>(data.getData(PRICING_DATA_WEIGHTS, ISNOTNULL).get()).get();
		const double &InitialBeta = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_INITIALBETA, ISNOTNULL).get()).get();
		const double &InitialTheta = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_INITIALTHETA, ISNOTNULL).get()).get();
		const double &InitialKappa = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_INITIALKAPPA, ISNOTNULL).get()).get();
		const double &InitialEpsilon = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_INITIALEPSILON, ISNOTNULL).get()).get();

		if (volVec.size() != MARKETNUM)
		{
			LAString msg = "volVec size is wrong. size = " + LAString(static_cast<int>(volVec.size()));
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		if (strikeVec.size() != MARKETNUM)
		{
			LAString msg = "strikeVec size is wrong. size = " + LAString(static_cast<int>(strikeVec.size()));
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		if (isCallVec.size() != MARKETNUM)
		{
			LAString msg = "isCallVec size is wrong. size = " + LAString(static_cast<int>(isCallVec.size()));
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
		// calc premium
		DoubleArray premVec(MARKETNUM);

		// ATM
		premVec[0] = LAMathAnalyticalFormula::BlackFormula(fwdFX, volVec[0] * LAMath::sqrt(term), strikeVec[0], isCallVec[0] ? 1 : -1) * dDF;
		// 25Delta High
		premVec[1] = LAMathAnalyticalFormula::BlackFormula(fwdFX, volVec[1] * LAMath::sqrt(term), strikeVec[1], isCallVec[1] ? 1 : -1) * dDF;
		// 10Delta High
		premVec[2] = LAMathAnalyticalFormula::BlackFormula(fwdFX, volVec[2] * LAMath::sqrt(term), strikeVec[2], isCallVec[2] ? 1 : -1) * dDF;
		// 25Delta Low
		premVec[3] = LAMathAnalyticalFormula::BlackFormula(fwdFX, volVec[3] * LAMath::sqrt(term), strikeVec[3], isCallVec[3] ? 1 : -1) * dDF;
		// 10Delta Low 
		premVec[4] = LAMathAnalyticalFormula::BlackFormula(fwdFX, volVec[4] * LAMath::sqrt(term), strikeVec[4], isCallVec[4] ? 1 : -1) * dDF;
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
		data.LAObject::remove(PRICING_DATA_OPTIONPREMIUMS);
		data.LAObject::add(PRICING_DATA_OPTIONPREMIUMS, new LADataDoubles(premVec_));
		data.LAObject::remove(PRICING_DATA_STRIKES);
		data.LAObject::add(PRICING_DATA_STRIKES, new LADataDoubles(strikeVec_));
		data.LAObject::remove(PRICING_DATA_WEIGHTS);
		data.LAObject::add(PRICING_DATA_WEIGHTS, new LADataDoubles(weightVec_));
		data.LAObject::remove(PRICING_DATA_ISCALLS);
		data.LAObject::add(PRICING_DATA_ISCALLS, new LADataBools(isCallVec_));
		data.LAObject::remove(PRICING_DATA_VOLATILITYS);
		data.LAObject::add(PRICING_DATA_VOLATILITYS, new LADataDoubles(volVec_));

		//set initial parameters
		data.LAObject::remove(PRICING_DATA_INITIALBETA);
		data.LAObject::add(PRICING_DATA_INITIALBETA, new LADataDouble(InitialBeta));
		data.LAObject::remove(PRICING_DATA_INITIALTHETA);
		data.LAObject::add(PRICING_DATA_INITIALTHETA, new LADataDouble(InitialTheta));
		data.LAObject::remove(PRICING_DATA_INITIALKAPPA);
		data.LAObject::add(PRICING_DATA_INITIALKAPPA, new LADataDouble(InitialKappa));
		data.LAObject::remove(PRICING_DATA_INITIALEPSILON);
		data.LAObject::add(PRICING_DATA_INITIALEPSILON, new LADataDouble(InitialEpsilon));
	}
	cout << static_cast<int>(LACoreThread::getThreadID()) << " MACalibrationSZ set up end" << endl;
}

void
LACalibrateSZ::setUp2(LAObjectPool &objPool, const MAScenarioParam &param, MACalibrationFunc *method, int gridPos)
{
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateSZ set up called" << endl;
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
	const LAString &calibInfoName = param.refName[0];
	const LAObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
	LAObject *calibID = 0;

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
	const LAString dYieldName = param.refName[curPos];
	if (!objPool.getObject(dYieldName).isDefined())
	{
		LAString msg = dYieldName + " is not registered in EntityPool";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const LAMathYieldCurve &dYield = dynamic_cast<const LAMathYieldCurve &>(objPool.getObject(dYieldName, ENCHKTYPE_ISDEFINED).get());
	const LAString &dYieldDataName = dYield.getYieldData().get().getName();

	// get foreign curve
	const LAString fYieldName = param.refName[curPos + 2];
	if (!objPool.getObject(fYieldName).isDefined())
	{
		LAString msg = fYieldName + " is not registered in EntityPool";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const LAMathYieldCurve &fYield = dynamic_cast<const LAMathYieldCurve &>(objPool.getObject(fYieldName, ENCHKTYPE_ISDEFINED).get());
	const LAString &fYieldDataName = fYield.getYieldData().get().getName();

	// calc df ratio
	mTimeGrid = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get()).get();
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
	mCalibIDName = param.calcType + "_FXCalibInfoEntity_" + dYieldDataName + "_" + fYieldDataName + "_" + LAString(gridPos);
	//set FXCurrency
	mFXCurrency = param.ccy;

	mSpotRate = dynamic_cast<const LADataDouble &>(calibInfo.getData(PRICING_DATA_SPOTFX, NOCHECK).get()).get();
	LADate spotDate = dynamic_cast<const LADataDate &>(calibInfo.getData(IR_CALIBRATION_DATA_SPOTDATE, NOCHECK).get()).get();
	//add extra base param
	mSpotRate += param.extraBaseParam;
	if (!param.extraParam.empty())
	{
		// shiftval check
		if (param.extraParam.size() != 1)
		{
			throw LACoreInvalidData("Parallel shift val size must be 1 .", __FILE__, __LINE__);
		}
		double shiftVal = param.extraParam[0];
		LAString shiftType = param.shiftType;
		if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
		{
			shiftVal = mSpotRate * shiftVal;
		}
		// add shift val
		mSpotRate = LAMath::max(mSpotRate + shiftVal, 0.0);
	}
	// asofdate
	LADate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
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
	// add fx time grid
	calibID->add(PRICING_DATA_FXTIMEGRIDS, calibInfo.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get().clone());

	// set calib variable
	calibID->add(PRICING_DATA_CALIBVARIABLES, calibInfo.getData(PRICING_DATA_CALIBVARIABLES, ISNOTNULL).get().clone());

	LAString calibInputType = dynamic_cast<const LADataString &>(calibInfo.getData(PRICING_DATA_CALIBINPUTPARAMTYPE, ISNOTNULL).get()).get();
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
		LAString msg = "Input of InputParamType is wrong Input = " + calibInputType;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// optionmaturity
	const LAStringVector &optionMatVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
	// vol 10Delta Low
	DoubleVector vol10DLVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOL10DL, ISNOTNULL).get()).get();
	// vol 25Delta Low
	DoubleVector vol25DLVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOL25DL, ISNOTNULL).get()).get();
	// vol ATM
	DoubleVector volATMVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOLATM, ISNOTNULL).get()).get();
	// vol 25Delta High
	DoubleVector vol25DHVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOL25DH, ISNOTNULL).get()).get();
	// vol 10Delta Hight
	DoubleVector vol10DHVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_FXVOL10DH, ISNOTNULL).get()).get();
	
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
			LAMarketData::convFXVolCalib2Market(volATMVec, vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec,
				vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec);

			LAString shiftType = param.shiftType;
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
					volATMVec[i] = LAMath::max(volATMVec[i], 0.0);
				}

			}
			else
			{
				unsigned int paraSize = param.paraTerm.size();
				if (paraSize != param.paraTerm.size())
				{
					throw LACoreInvalidData("Parallel shift val and value is not same ", __FILE__, __LINE__);
				}

				for (unsigned int i = 0; i < paraSize; ++i)
				{
					LAString term = param.paraTerm[i];
					term.toUpper();
					LAStringVector termVec = term.toToken('_');
					if (termVec.size() != 2)
					{
						LAString msg = "Term format is is wrong, term = " + term;
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					LAStringVector::const_iterator it = find(optionMatVec.begin(), optionMatVec.end(), termVec[1]);
					if (it == optionMatVec.end())
					{
						LAString msg = "Term is not in volatility file, term = " + termVec[1];
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
						LAString msg = "This FXVol grid is not support = " + termVec[0];
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
						(*pTargetVol)[indx] = LAMath::max((*pTargetVol)[indx] + shiftVal, 0.0);
					}
					else
					{
						// add shift val
						(*pTargetVol)[indx] = (*pTargetVol)[indx] + shiftVal;
					}
				}
			}
			// reconvert to calib format
			LAMarketData::convFXVolMarket2Calib(volATMVec, vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec,
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
				LAString msg = "Grid shift param is not exist, grid num = " + LAString(gridPos);
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			DoubleVector vol25BFVec; // 25ButterFly
			DoubleVector vol10BFVec; // 10ButterFly
			DoubleVector vol25RRVec; // 25RiskReversal
			DoubleVector vol10RRVec; // 10RiskReversal
			// convert to market format
			LAMarketData::convFXVolCalib2Market(volATMVec, vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec,
				vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec);

			LAString term = param.gridTerm[gridPos];
			term.toUpper();

			LAStringVector termVec = term.toToken('_');
			if (termVec.size() != 2)
			{
				LAString msg = "Term format is is wrong, term = " + term;
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			LAStringVector::const_iterator it = find(optionMatVec.begin(), optionMatVec.end(), termVec[1]);
			if (it == optionMatVec.end())
			{
				LAString msg = "Term is not in volatility file, term = " + termVec[1];
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
				LAString msg = "This FXVol grid is not support = " + termVec[0];
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
						LAString shiftType = param.shiftType;
						if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
						{
							shiftVal = (*pTargetVol)[gridPosfrom + k] * shiftVal;
						}

						if (termVec[0] == FXVOL_ATM)
						{
							// add shift val
							(*pTargetVol)[gridPosfrom + k] = LAMath::max((*pTargetVol)[gridPosfrom + k] + shiftVal, 0.0);
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
					LAString shiftType = param.shiftType;
					if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
					{
						shiftVal = (*pTargetVol)[indx] * shiftVal;
					}
					// add shift val
					if (termVec[0] == FXVOL_ATM)
						(*pTargetVol)[indx] = LAMath::max((*pTargetVol)[indx] + shiftVal, 0.0);
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
					LAString term = param.gridTerm[k];
					term.toUpper();

					LAStringVector termVec = term.toToken('_');
					if (termVec.size() != 2)
					{
						LAString msg = "Term format is is wrong, term = " + term;
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					LAStringVector::const_iterator it = find(optionMatVec.begin(), optionMatVec.end(), termVec[1]);
					if (it == optionMatVec.end())
					{
						LAString msg = "Term is not in volatility file, term = " + termVec[1];
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int tmpindx = static_cast<unsigned int>(it - optionMatVec.begin());

					LAString shiftType = param.shiftType;
					if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
					{
						shiftVal = (*pTargetVol)[tmpindx] * shiftVal;
					}

					if (termVec[0] == FXVOL_ATM)
					{
						// add shift val
						(*pTargetVol)[tmpindx] = LAMath::max((*pTargetVol)[tmpindx] + shiftVal, 0.0);
					}
					else
					{
						// add shift val
						(*pTargetVol)[tmpindx] = (*pTargetVol)[tmpindx] + shiftVal;
					}

				}
			}
			// reconvert to calib format
			LAMarketData::convFXVolMarket2Calib(volATMVec, vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec,
				vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec);
		}
	}
	// stike fwd
	const LAStringVector &strikeFwdVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_STRIKEFWDTERM, ISNOTNULL).get()).get();

	// fixingcalendar
	const LAStringVector &calVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_FIXINGCALENDAR, ISNOTNULL).get()).get();
	// slidingrule
	const LAStringVector &slidingVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get()).get();
	// spot or fwd
	const LAStringVector &soptOrFwdVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_SPOTORFWD, ISNOTNULL).get()).get();

	// optiontype ATM
	const LAStringVector &opTypeATMVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPEATM, ISNOTNULL).get()).get();
	// optiontype 25Delta High
	const LAStringVector &opType25DHVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPE25DH, ISNOTNULL).get()).get();
	// optiontype 10Delta Hight
	const LAStringVector &opType10DHVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPE10DH, ISNOTNULL).get()).get();
	// optiontype 25Delta Low
	const LAStringVector &opType25DLVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPE25DL, ISNOTNULL).get()).get();
	// optiontype 10Delta Low
	const LAStringVector &opType10DLVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPE10DL, ISNOTNULL).get()).get();

	// weight ATM
	const DoubleVector &weightATMVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_WEIGHTATM, ISNOTNULL).get()).get();
	// weight 25Delta High
	const DoubleVector &weight25DHVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_WEIGHT25DH, ISNOTNULL).get()).get();
	// weight 10Delta Hight
	const DoubleVector &weight10DHVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_WEIGHT10DH, ISNOTNULL).get()).get();
	// weight 25Delta Low
	const DoubleVector &weight25DLVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_WEIGHT25DL, ISNOTNULL).get()).get();
	// weight 10Delta Low
	const DoubleVector &weight10DLVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_WEIGHT10DL, ISNOTNULL).get()).get();
	
	// initial params
	const DoubleVector &initialBetaVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_INITIALBETA, ISNOTNULL).get()).get();
	const DoubleVector &initialThetaVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_INITIALTHETA, ISNOTNULL).get()).get();
	const DoubleVector &initialKappaVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_INITIALKAPPA, ISNOTNULL).get()).get();
	const DoubleVector &initialEpsilonVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_INITIALEPSILON, ISNOTNULL).get()).get();
	
	// TODO merget latest version strike
	// at merge delete literal
	// strikeval 25Delta High
	const DoubleVector &strikeVal25DHVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_STRIKEVALES + LAString("25DH"), ISNOTNULL).get()).get();
	// strikeval 10Delta Hight
	const DoubleVector &strikeVal10DHVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_STRIKEVALES + LAString("10DH"), ISNOTNULL).get()).get();
	// strikeval 25Delta Low
	const DoubleVector &strikeVal25DLVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_STRIKEVALES + LAString("25DL"), ISNOTNULL).get()).get();
	// strikeval 10Delta Low
	const DoubleVector &strikeVal10DLVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(PRICING_DATA_STRIKEVALES + LAString("10DL"), ISNOTNULL).get()).get();


	// is fit atm
	const BoolVector &isFitATMVec = dynamic_cast<const LADataBools &>(calibInfo.getData(PRICING_DATA_ISFITATM, ISNOTNULL).get()).get();
	// is delta neutral
	const BoolVector &isDeltaNeutVec = dynamic_cast<const LADataBools &>(calibInfo.getData(PRICING_DATA_ISDELTANEUTRAL, ISNOTNULL).get()).get();

	// strikeStr
	const unsigned int MARKETNUM = 5;
	LAStringVector strStrikeVec(MARKETNUM);
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
	const LAPriceDataCalendar &termCal = dynamic_cast<const LAPriceDataCalendar &>(calibInfo.getData(PRICING_DATA_TERMCALENDAR, ISNOTNULL).get());
	const LAString strTermCal = termCal.convertToString();
	const LAPriceDataSlidingRule &termSliding = dynamic_cast<const LAPriceDataSlidingRule &>(calibInfo.getData(PRICING_DATA_TERMSLIDINGRULE, ISNOTNULL).get());
	const LAPriceDataDayCount &termDC = dynamic_cast<const LAPriceDataDayCount &>(calibInfo.getData(PRICING_DATA_TERMDAYCOUNT, ISNOTNULL).get());
	const bool isIncludeLast = dynamic_cast<const LADataBool &>(calibInfo.getData(PRICING_DATA_TERMISINCLUDELAST, ISNOTNULL).get()).get();
	// spotdate for calc term
	int spotlag = dynamic_cast<const LADataInt &>(calibInfo.getData(PRICING_DATA_TERMSPOTLAG, ISNOTNULL).get());
	LADate optionSpotDate = LAMathDateCalculations::getFXSpotDate(param.ccy, asofDate, strTermCal, spotlag, true);
	// need to calc strike and prem
	LAString strLow("low");
	LAString strHigh("high");
	LAString strPrem("Prem");
	LAString strBuy("Buy");

	LAString marketRef;
	unsigned int gridSize = optionMatVec.size();
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		LAString mGenName = mCalibIDName + "_" + optionMatVec[i];
		LAMathObjectValue *mGen = 0;
		tmpEh = objPool.getObject(mGenName, ENCHKTYPE_NOCHECK);
		if (!tmpEh.isDefined())
		{
			mGen = new LAMathObjectValue(calibID->getDataInstance());
			objPool.set(mGenName, mGen);
		}
		else
		{
			mGen = &dynamic_cast<LAMathObjectValue &>(tmpEh.get());
			mGen->reset();
		}
		// set data
		// set name
		mGen->getName().convertFromString(mGenName);
		// set value
		//mGen->setValuationMethod(valueStr);
		// set model param
		mGen->LAObject::add(PRICING_DATA_MODELPARAM, new LADataReference()).convertFromString(mCalibIDName);
		// set is fitatm
		mGen->LAObject::add(PRICING_DATA_ISFITATM, new LADataBool(isFitATMVec[i]));
		// set sliding rule
		mGen->LAObject::add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingVec[i]);
		// set spotlag
		mGen->LAObject::add(IR_CALIBRATION_DATA_SPOTLAG, new LADataInt(spotlag));
		// set key FX name (ex. JPY/USD)
		mGen->LAObject::add(IR_CALIBRATION_DATA_CURRENCY, new LADataString(param.ccy));
		// set fixing calendar
		mGen->LAObject::add(PRICING_DATA_FIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(calVec[i]);
		// set fixing calendar
		mGen->LAObject::add(PRICING_DATA_TERMCALENDAR, new LAPriceDataCalendar(termCal));
		// set option maturity
		mGen->LAObject::add(IR_CALIBRATION_DATA_OPTIONMATURITY, new LADataString(optionMatVec[i]));
		// set params
		mGen->LAObject::add(PRICING_DATA_INITIALBETA, new LADataDouble(initialBetaVec[i]));
		mGen->LAObject::add(PRICING_DATA_INITIALTHETA, new LADataDouble(initialThetaVec[i]));
		mGen->LAObject::add(PRICING_DATA_INITIALKAPPA, new LADataDouble(initialKappaVec[i]));
		mGen->LAObject::add(PRICING_DATA_INITIALEPSILON, new LADataDouble(initialEpsilonVec[i]));

		// weight
		DoubleArray weightVec(MARKETNUM);
		weightVec[0] = weightATMVec[i];
		weightVec[1] = weight25DHVec[i];
		weightVec[2] = weight10DHVec[i];
		weightVec[3] = weight25DLVec[i];
		weightVec[4] = weight10DLVec[i];
		mGen->LAObject::add(PRICING_DATA_WEIGHTS, new LADataDoubles(weightVec));
		// vol
		DoubleArray volVec(MARKETNUM);
		volVec[0] = volATMVec[i];
		volVec[1] = vol25DHVec[i];
		volVec[2] = vol10DHVec[i];
		volVec[3] = vol25DLVec[i];
		volVec[4] = vol10DLVec[i];
		mGen->LAObject::add(PRICING_DATA_VOLATILITYS, new LADataDoubles(volVec));
		// strike
		mGen->LAObject::add(PRICING_DATA_STRIKESTRINGS, new LADataStrings(strStrikeVec));
		// isCall
		mGen->LAObject::add(PRICING_DATA_ISCALLS, new LADataBools(isCallVec));

		// is make calib from vol
		mGen->LAObject::add(PRICING_DATA_ISMAKECALIBDATAFROMVOLATILITY, new LADataBool(isMCFVol));
		if (isMCFVol)
		{
			// is delta neutral
			mGen->LAObject::add(PRICING_DATA_ISDELTANEUTRAL, new LADataBool(isDeltaNeutVec[i]));
			// spot fwd
			mGen->LAObject::add(PRICING_DATA_SPOTORFWD, new LADataString(soptOrFwdVec[i]));
		}
		else
		{
			// calc strike and optiontype
			LADate settledate = LAMathDateCalculations::getDate(optionSpotDate, optionMatVec[i], termSliding, &termCal, true);
			LADate date = LAMathDateCalculations::getFXSpotDate(param.ccy, settledate, strTermCal, -spotlag, true);
			const double term = termDC.getTerm(asofDate, date, isIncludeLast);
			const double dDF = dYield.getBasisDF(term);
			const double fDF = fYield.getBasisDF(term);
			const double fwdFX = mSpotRate * fDF / dDF;

			mGen->LAObject::add(PRICING_DATA_FXTERM, new LADataDouble(term));
			mGen->LAObject::add(PRICING_DATA_DOMESTICDF, new LADataDouble(dDF));
			mGen->LAObject::add(PRICING_DATA_FOREIGNDF, new LADataDouble(fDF));
			mGen->LAObject::add(PRICING_DATA_FORWARDFX, new LADataDouble(fwdFX));

			const double reversal = 0.0;
			const double strangle = 0.0;
			LAString spotOrFwd = soptOrFwdVec[i];

			// calc strike
			DoubleArray strikeVec(MARKETNUM);
			// ATM
			LAStringVector::const_iterator it = find(strikeFwdVec.begin(), strikeFwdVec.end(), optionMatVec[i]);
			if (it == strikeFwdVec.end())
			{
				strikeVec[0] = fwdFX * LAMath::exp(-0.5 * LAMath::pow(volATMVec[i], 2) * term);
			}
			else
			{
				strikeVec[0] = fwdFX;
			}

			// 25Delta High
			try
			{
				strikeVec[1] = LAMathFXVanillaFuncUtility::calcstrikefromdelta
					(strikeVal25DHVec[i], spotOrFwd, strHigh, mSpotRate, fwdFX, vol25DHVec[i], reversal, strangle, fDF, term);
			}
			catch (LACoreError &err)
			{
				LAString msg = err.getMsg();
				if (msg.findString("Not Convergence from rtsafe") == -1)
					throw LACoreNumericalError("Fx Option convergence does not work", __FILE__, __LINE__);

				strikeVec[1] = LAMathFXVanillaFuncUtility::calcmaxstrike
					(spotOrFwd, mSpotRate, fwdFX, vol25DHVec[i], reversal, strangle, fDF, term);
			}
			// 10Delta High
			try
			{
				strikeVec[2] = LAMathFXVanillaFuncUtility::calcstrikefromdelta
					(strikeVal10DHVec[i], spotOrFwd, strHigh, mSpotRate, fwdFX, vol10DHVec[i], reversal, strangle, fDF, term);
			}
			catch (LACoreError &err)
			{
				LAString msg = err.getMsg();
				if (msg.findString("Not Convergence from rtsafe") == -1)
					throw LACoreNumericalError("Fx Option convergence does not work", __FILE__, __LINE__);

				strikeVec[2] = LAMathFXVanillaFuncUtility::calcmaxstrike
					(spotOrFwd, mSpotRate, fwdFX, vol10DHVec[i], reversal, strangle, fDF, term);
			}
			// 25Delta Low
			strikeVec[3] = LAMathFXVanillaFuncUtility::calcstrikefromdelta
				(strikeVal25DLVec[i], spotOrFwd, strLow, mSpotRate, fwdFX, vol25DLVec[i], reversal, strangle, fDF, term);
			// 10Delta Low
			strikeVec[4] = LAMathFXVanillaFuncUtility::calcstrikefromdelta
				(strikeVal10DLVec[i], spotOrFwd, strLow, mSpotRate, fwdFX, vol10DLVec[i], reversal, strangle, fDF, term);
			//set as data
			mGen->LAObject::add(PRICING_DATA_STRIKES, new LADataDoubles(strikeVec));

			//calc premium
			DoubleArray premVec(MARKETNUM);
			// ATM
			LAString tmpOpType = opTypeATMVec[i];
			premVec[0] = LAMathIRVanillaFuncUtility::bkOption(strPrem, strBuy, tmpOpType, fwdFX, strikeVec[0], volATMVec[i], dDF, asofDate, date);
			// 25Delta High
			tmpOpType = opType25DHVec[i];
			premVec[1] = LAMathIRVanillaFuncUtility::bkOption(strPrem, strBuy, tmpOpType, fwdFX, strikeVec[1], vol25DHVec[i], dDF, asofDate, date);
			// 10Delta High
			tmpOpType = opType10DHVec[i];
			premVec[2] = LAMathIRVanillaFuncUtility::bkOption(strPrem, strBuy, tmpOpType, fwdFX, strikeVec[2], vol10DHVec[i], dDF, asofDate, date);
			// 25Delta Low
			tmpOpType = opType25DLVec[i];
			premVec[3] = LAMathIRVanillaFuncUtility::bkOption(strPrem, strBuy, tmpOpType, fwdFX, strikeVec[3], vol25DLVec[i], dDF, asofDate, date);
			// 10Delta Low
			tmpOpType = opType10DLVec[i];
			premVec[4] = LAMathIRVanillaFuncUtility::bkOption(strPrem, strBuy, tmpOpType, fwdFX, strikeVec[4], vol10DLVec[i], dDF, asofDate, date);
			//set as data
			mGen->LAObject::add(PRICING_DATA_OPTIONPREMIUMS, new LADataDoubles(premVec));


		}

		marketRef += mGenName + ":";
	}

	marketRef = marketRef.subString(0, marketRef.size() - 2);

	//calibrationengin
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
	// asofdate
	calibEngine->add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asofDate));
	// calibration data
	calibEngine->add(CALIBRATION_DATA_CALIBRATIONDATA, new LADataMultiReference()).convertFromString(marketRef);
	// calibration engine
	calibEngine->add(PRICING_DATA_CALIBRATORENGINE, new LADataProcedure()).convertFromString(FN_IR_SZCALIBRATION_STR);
	// sdeinfo
	calibEngine->add(PRICING_DATA_SDEINFO, new LADataReference()).convertFromString(mCalibIDName);
	// spotrate
	calibEngine->add(PRICING_DATA_SPOTFX, new LADataDouble(mSpotRate));

	// set domestic curve
	calibEngine->add(PRICING_DATA_DOMESTICCURVE, new LADataReference()).convertFromString(dYieldDataName);
	// set domestic model
	LAString dIRModelName = param.refName[curPos + 1];
	calibEngine->add(PRICING_DATA_DOMESTICIRMODEL, new LADataReference()).convertFromString(dIRModelName);
	// use df2 for domestic curve
	bool isDF2forDome = false;
	LAObjectHolder ehD = objPool.getObject(dYieldDataName, ENCHKTYPE_ISDEFINED);
	const LADataHolder &ahD = ehD.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK);
	if (ahD.isDefined() && !ahD.isNull())
	{
		isDF2forDome = true;
	}
	calibEngine->add(PRICING_DATA_ISUSEDF2FORDOMESTIC, new LADataBool(isDF2forDome));

	// set foregin curve
	calibEngine->add(PRICING_DATA_FOREIGNCURVE, new LADataReference()).convertFromString(fYieldDataName);
	// set foregin model
	LAString fIRModelName = param.refName[curPos + 3];
	calibEngine->add(PRICING_DATA_FOREIGNIRMODEL, new LADataReference()).convertFromString(fIRModelName);
	// use df2 for foregin curve
	bool isDF2forFore = false;
	LAObjectHolder ehF = objPool.getObject(fYieldDataName, ENCHKTYPE_ISDEFINED);
	const LADataHolder &ahF = ehF.getData(IR_CALIBRATION_DATA_DFS2, NOCHECK);
	if (ahF.isDefined() && !ahF.isNull())
	{
		isDF2forFore = true;
	}
	calibEngine->add(PRICING_DATA_ISUSEDF2FORFOREIGN, new LADataBool(isDF2forFore));

	// set sde vol correlation
	LAStringVector ccys;
	LAMarketData::convertToCurrency(param.ccy, ccys);
	ccys.push_back(param.ccy);
	ccys.push_back(param.ccy + "_VOL");
	DoubleMatrix corMtx;
	LAMarketData::getSDECorrelation(ccys, corMtx);
	double cor_vol;
	cor_vol = corMtx[2][3];
	calibEngine->add(PRICING_DATA_SDEVOLCORRELATIONS, new LADataDouble(cor_vol));

	// dataout
	if (param.isOutPut)
	{
		calibEngine->add(PRICING_DATA_ISOUTPUT, new LADataBool(true));
	}
	else
	{
		calibEngine->add(PRICING_DATA_ISOUTPUT, new LADataBool(false));
	}
	// set member
	mpCaibEngine = calibEngine;
	// check is calib end ir model
	cout << static_cast<int>(LACoreThread::getThreadID()) << " DomesticIRModelName = " << dIRModelName << " : ForeignIRModelName = " << fIRModelName << endl;
	if (!LAMarketData::isCalibrateEnd(dIRModelName, objPool) || !LAMarketData::isCalibrateEnd(fIRModelName, objPool))
	{
		throw LACoreInvalidData("IR model calibration is not end ..", __FILE__, __LINE__);
	}
	mpCaibEngine->getDataInstance()->getReferencePool().completeDependency();

	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateSZ set up end" << endl;
}

// 
/*!
@brief calibrate
*/
void
LACalibrateSZ::doCalibrate()
{
	//calibration start
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateSZ calibrate called.." << endl;
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

		// print error messages
		LADataHolder &dh = mpCaibEngine->getData(CALIBRATION_DATA_ERRORMESSAGES, NOCHECK);
		if (dh.isDefined() && !dh.isNull())
		{
			const LAStringVector &msgs = dynamic_cast<LADataStrings &>(dh.get()).get();
			MALogger &logger = LACoreDataService::getLogManager().getLogger();
			for (unsigned int i = 0; i < msgs.size(); ++i)
			{
				logger.error(msgs[i], __FILE__, __LINE__);
			}
		}
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
		if (dataVec.size() != 6)
		{
			throw LACoreInvalidData("Serialize data size must be 6.", __FILE__, __LINE__);
		}

		LAObject *e = new LAObject();
		e->add(CALIBRATION_DATA_NAME, new LADataString(mCalibIDName));
		e->add(PRICING_DATA_FXTIMEGRIDS, new LADataDoubles()).convertFromString(dataVec[0]);
		e->add(PRICING_DATA_OPTIMIZEDBETA, new LADataDoubles()).convertFromString(dataVec[1]);
		e->add(PRICING_DATA_OPTIMIZEDTHETA, new LADataDoubles()).convertFromString(dataVec[2]);
		e->add(PRICING_DATA_OPTIMIZEDKAPPA, new LADataDoubles()).convertFromString(dataVec[3]);
		e->add(PRICING_DATA_OPTIMIZEDEPSILON, new LADataDoubles()).convertFromString(dataVec[4]);
		e->add(PRICING_DATA_VOLV0, new LADataDoubles()).convertFromString(dataVec[5]);

		objPool.set(mCalibIDName, e);

	}

	// get calib param
	LAObjectHolder calibID = objPool.getObject(mCalibIDName, ENCHKTYPE_ISDEFINED);

	//set FXcurrency
	LAString fxccy = mFXCurrency;
	fxccy.toUpper();
	// tenor
	const DoubleArray &eTimeGrid = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_FXTIMEGRIDS, ISDEFINED).get()).get();
	// optimized parameters
	const DoubleArray &betaVec = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_OPTIMIZEDBETA, ISDEFINED).get()).get();
	const DoubleArray &thetaVec = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_OPTIMIZEDTHETA, ISDEFINED).get()).get();
	const DoubleArray &kappaVec = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_OPTIMIZEDKAPPA, ISDEFINED).get()).get();
	const DoubleArray &epsilonVec = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_OPTIMIZEDEPSILON, ISDEFINED).get()).get();
	//initial vol
	const DoubleArray &v0Vec = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_VOLV0, ISDEFINED).get()).get();
	
	unsigned int vecSize = mTimeGrid.size();
	// check
	if (vecSize != eTimeGrid.size() || vecSize != betaVec.size() || vecSize != thetaVec.size() || vecSize != kappaVec.size() || vecSize != epsilonVec.size())
	{
		throw LACoreInvalidData("Calibrated data format is wroing.", __FILE__, __LINE__);
	}
	const double INFINITESIMAL = 1E-7;
	for (unsigned int i = 0; i <vecSize; ++i)
	{
		if (LAMath::abs(mTimeGrid[i] - eTimeGrid[i]) > INFINITESIMAL)
		{
			LAString msg = "CanonicTimeGrid = " + LAString(mTimeGrid[i]) + ", EntityTimeGrid = " + LAString(eTimeGrid[i]);
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}
	}

	DoubleArray fwdVec(vecSize);
	// check
	if (vecSize != mDFRatios.size())
	{
		throw LACoreInvalidData("DF ratio size and timegrid size is not same.", __FILE__, __LINE__);
	}
	// calc forward fx
	for (unsigned int i = 0; i < vecSize; ++i)
	{
		fwdVec[i] = mSpotRate * mDFRatios[i];
	}
	
	LAFunctionBase *method = new LAMathVolFuncSZDD(mTimeGrid, fwdVec, betaVec, thetaVec, kappaVec, epsilonVec, fxccy, v0Vec);

	mpFunc->setRealFunction(*method);
	mpFunc->setOn();

	delete method;

	if (mSerializeStatus != CALIB_S_DESERIALIZE)
	{
		// if serialize dump
		if (mSerializeStatus == CALIB_S_SERIALIZE)
		{
			///// mod Precision for grid
			//LADataDoubles tmp;
			LADataDoubles tmp(22);
			// time grid
			tmp.set(eTimeGrid);
			LAString eTimeGridStr = tmp.convertToString();
			// ParametersVec
			tmp.set(betaVec);
			LAString betaVecStr = tmp.convertToString();
			tmp.set(thetaVec);
			LAString thetaVecStr = tmp.convertToString();
			tmp.set(kappaVec);
			LAString kappaVecStr = tmp.convertToString();
			tmp.set(epsilonVec);
			LAString epsilonVecStr = tmp.convertToString();
			// volVec
			tmp.set(v0Vec);
			LAString volVecStr = tmp.convertToString();
			// set serialize
			setmSerializeMap(mCalibIDName + "," + eTimeGridStr + "," + betaVecStr + "," + thetaVecStr + "," + kappaVecStr + "," + epsilonVecStr + "," + volVecStr);
			istringstream *dataStream = new istringstream(mSerializeMap[mSerializeFile].getCString());
			LACoreDataService::setIStringStream(mSerializeFile, dataStream);

		}

		//ouput
		const LADataHolder &attr = mpCaibEngine->getData(PRICING_DATA_ISOUTPUT, NOCHECK);
		if (attr.isDefined() && !attr.isNull())
		{
			if (dynamic_cast<const LADataBool &>(attr.get()).get())
			{
				ifstream fin;
				ofstream fout;
				const LAString dirName = LACoreDataService::getOutputDirectory();
				LAString fileName = mCalibIDName.subString(0, 100) + ".csv";
				fileName.exchange("/", "");
				fileName = dirName + fileName;
				fin.open(fileName.getCString());
				if (!fin)
				{
					fout.open(fileName.getCString());
					unsigned int size = mTimeGrid.size();
					for (unsigned int i = 0; i < size; ++i)
					{
						fout << LAString(mTimeGrid[i]) << "," << LAString(betaVec[i]) << "," << LAString(thetaVec[i]) << "," << LAString(kappaVec[i]) << "," << LAString(epsilonVec[i]) << "," << LAString(v0Vec[i]) << "," << LAString(fwdVec[i]) << endl;
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
						LAStringVector lineVec = LAString(c_line).toToken(MARKET_DATA_DELIMITER);
						if (lineVec.size() != 7)
						{
							throw LACoreInvalidData("SZ DD Calib format is wrong", __FILE__, __LINE__);
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
					LAMathVolFuncFX *t_funcFX = 0;
					t_funcFX = new LAMathVolFuncSZDD(t_timeGrid, t_fwdVec, t_betaVec, t_thetaVec, t_kappaVec, t_epsilonVec, fxccy, t_volVec);
					mpFunc->setRealFunction(*t_funcFX);
					delete t_funcFX;
				}
				fin.close();
				
			}
		}
		// delete data
		LADataMultiReference &calibData = dynamic_cast<LADataMultiReference &>(mpCaibEngine->getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
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
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateSZ calibrate end.." << endl;
	cout << "-> time = " << time << endl;
}
