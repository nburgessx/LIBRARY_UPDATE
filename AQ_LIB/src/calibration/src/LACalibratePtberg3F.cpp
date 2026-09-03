/*! @file
    @brief HW calibration request
*/
//  2008, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibratePtberg3F.cpp
//
//  DESCRIPTION :        HW calibration request class
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
#include "LACalibratePtberg3F.h"
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
#include "LAPriceFXDisplacedDiffusionCalibration3F.h"
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

using namespace std;
// constructor
/*!

*/
LACalibratePtberg3F::LACalibratePtberg3F()
:LACalibratePtberg()
{
}

// destructor
/*!

*/
LACalibratePtberg3F::~LACalibratePtberg3F()
{
}


// 
/*!
    @brief setup
*/
void
LACalibratePtberg3F::setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos)
{
	LACalibratePtberg::setUp(objPool, param, method, gridPos);
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibratePtberg3F set up called" << endl;
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
	mpCaibEngine->add(PRICING_DATA_CALIBRATORENGINE, new LADataProcedure()).convertFromString(FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION3F_STR);

	const LAString &calibInfoName = param.refName[0];
	const LAObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();

	mpCaibEngine->remove(PRICING_DATA_DOMESTICCURVETYPE);
	mpCaibEngine->add(PRICING_DATA_DOMESTICCURVETYPE, calibInfo.getData(PRICING_DATA_DOMESTICCURVETYPE, ISNOTNULL).get().clone());

	mpCaibEngine->remove(PRICING_DATA_FOREIGNCURVETYPE);
	mpCaibEngine->add(PRICING_DATA_FOREIGNCURVETYPE, calibInfo.getData(PRICING_DATA_FOREIGNCURVETYPE, ISNOTNULL).get().clone());

	const LAString &calibEngineName = dynamic_cast<const LADataString &>(mpCaibEngine->getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	// set up skew & vol info
	const LAString SKEWVOL = "_SKEWVOL";
	const LAString infoName_skewvol = calibEngineName + SKEWVOL;
	LAObject *pInfo_skewvol = 0;
	LAObjectHolder tmpEh = objPool.getObject(infoName_skewvol, ENCHKTYPE_NOCHECK);
	if (!tmpEh.isDefined())
	{
		pInfo_skewvol = new LAObject();
		objPool.set(infoName_skewvol, pInfo_skewvol);
	}
	else
	{
		pInfo_skewvol = &tmpEh.get();
		pInfo_skewvol->reset();
	}

	// set boundary
	pInfo_skewvol->remove(CALIBRATION_DATA_BOUNDARY_MAX_SKEW);
	pInfo_skewvol->add(CALIBRATION_DATA_BOUNDARY_MAX_SKEW, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_SKEW, ISNOTNULL).get().clone());
	
	pInfo_skewvol->remove(CALIBRATION_DATA_BOUNDARY_MIN_SKEW);
	pInfo_skewvol->add(CALIBRATION_DATA_BOUNDARY_MIN_SKEW, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_SKEW, ISNOTNULL).get().clone());
	
	pInfo_skewvol->remove(CALIBRATION_DATA_BOUNDARY_MAX_VOL);
	pInfo_skewvol->add(CALIBRATION_DATA_BOUNDARY_MAX_VOL, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_VOL, ISNOTNULL).get().clone());
	
	pInfo_skewvol->remove(CALIBRATION_DATA_BOUNDARY_MIN_VOL);
	pInfo_skewvol->add(CALIBRATION_DATA_BOUNDARY_MIN_VOL, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_VOL, ISNOTNULL).get().clone());
	
	// other params
	pInfo_skewvol->remove(CALIBRATION_DATA_MAX_ITERATION);
	pInfo_skewvol->add(CALIBRATION_DATA_MAX_ITERATION, calibInfo.getData(CALIBRATION_DATA_MAX_ITERATION + SKEWVOL, ISNOTNULL).get().clone());
	
	pInfo_skewvol->remove(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION);
	pInfo_skewvol->add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, calibInfo.getData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION + SKEWVOL, ISNOTNULL).get().clone());
	
	pInfo_skewvol->remove(CALIBRATION_DATA_ROOT_EPSILON);
	pInfo_skewvol->add(CALIBRATION_DATA_ROOT_EPSILON, calibInfo.getData(CALIBRATION_DATA_ROOT_EPSILON + SKEWVOL, ISNOTNULL).get().clone());
	
	pInfo_skewvol->remove(CALIBRATION_DATA_FUNCTION_EPSILON);
	pInfo_skewvol->add(CALIBRATION_DATA_FUNCTION_EPSILON, calibInfo.getData(CALIBRATION_DATA_FUNCTION_EPSILON + SKEWVOL, ISNOTNULL).get().clone());
	
	pInfo_skewvol->remove(CALIBRATION_DATA_GRADIENT_NORM_EPSILON);
	pInfo_skewvol->add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, calibInfo.getData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON + SKEWVOL, ISNOTNULL).get().clone());
	
	pInfo_skewvol->remove(CALIBRATION_DATA_OPT_METHOD_TYPE);
	pInfo_skewvol->add(CALIBRATION_DATA_OPT_METHOD_TYPE, calibInfo.getData(CALIBRATION_DATA_OPT_METHOD_TYPE + SKEWVOL, ISNOTNULL).get().clone());

	// set reference
	mpCaibEngine->remove(PRICING_DATA_SKEWVOLCALIBINFO);
	mpCaibEngine->add(PRICING_DATA_SKEWVOLCALIBINFO, new LADataReference()).convertFromString(infoName_skewvol);


	// set up sigma & beta info
	const LAString SIGMABETA = "_SIGMABETA";
	const LAString infoName_sigmabeta = calibEngineName + SIGMABETA;
	LAObject *pInfo_sigmabeta = 0;
	tmpEh = objPool.getObject(infoName_sigmabeta, ENCHKTYPE_NOCHECK);
	if (!tmpEh.isDefined())
	{
		pInfo_sigmabeta = new LAObject();
		objPool.set(infoName_sigmabeta, pInfo_sigmabeta);
	}
	else
	{
		pInfo_sigmabeta = &tmpEh.get();
		pInfo_sigmabeta->reset();
	}

	// set boundary
	pInfo_sigmabeta->remove(CALIBRATION_DATA_BOUNDARY_MAX_SIGMA);
	pInfo_sigmabeta->add(CALIBRATION_DATA_BOUNDARY_MAX_SIGMA, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_SIGMA, ISNOTNULL).get().clone());
	
	pInfo_sigmabeta->remove(CALIBRATION_DATA_BOUNDARY_MIN_SIGMA);
	pInfo_sigmabeta->add(CALIBRATION_DATA_BOUNDARY_MIN_SIGMA, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_SIGMA, ISNOTNULL).get().clone());
	
	pInfo_sigmabeta->remove(CALIBRATION_DATA_BOUNDARY_MAX_BETA);
	pInfo_sigmabeta->add(CALIBRATION_DATA_BOUNDARY_MAX_BETA, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_BETA, ISNOTNULL).get().clone());
	
	pInfo_sigmabeta->remove(CALIBRATION_DATA_BOUNDARY_MIN_BETA);
	pInfo_sigmabeta->add(CALIBRATION_DATA_BOUNDARY_MIN_BETA, calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_BETA, ISNOTNULL).get().clone());
	
	// other params
	pInfo_sigmabeta->remove(CALIBRATION_DATA_MAX_ITERATION);
	pInfo_sigmabeta->add(CALIBRATION_DATA_MAX_ITERATION, calibInfo.getData(CALIBRATION_DATA_MAX_ITERATION + SIGMABETA, ISNOTNULL).get().clone());
	
	pInfo_sigmabeta->remove(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION);
	pInfo_sigmabeta->add(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, calibInfo.getData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION + SIGMABETA, ISNOTNULL).get().clone());
	
	pInfo_sigmabeta->remove(CALIBRATION_DATA_ROOT_EPSILON);
	pInfo_sigmabeta->add(CALIBRATION_DATA_ROOT_EPSILON, calibInfo.getData(CALIBRATION_DATA_ROOT_EPSILON + SIGMABETA, ISNOTNULL).get().clone());
	
	pInfo_sigmabeta->remove(CALIBRATION_DATA_FUNCTION_EPSILON);
	pInfo_sigmabeta->add(CALIBRATION_DATA_FUNCTION_EPSILON, calibInfo.getData(CALIBRATION_DATA_FUNCTION_EPSILON + SIGMABETA, ISNOTNULL).get().clone());
	
	pInfo_sigmabeta->remove(CALIBRATION_DATA_GRADIENT_NORM_EPSILON);
	pInfo_sigmabeta->add(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, calibInfo.getData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON + SIGMABETA, ISNOTNULL).get().clone());
	
	pInfo_sigmabeta->remove(CALIBRATION_DATA_SMALL_STEPS);
	pInfo_sigmabeta->add(CALIBRATION_DATA_SMALL_STEPS, calibInfo.getData(CALIBRATION_DATA_SMALL_STEPS, ISNOTNULL).get().clone());

	pInfo_sigmabeta->remove(CALIBRATION_DATA_OPT_SIGMABETA_FLAG);
	pInfo_sigmabeta->add(CALIBRATION_DATA_OPT_SIGMABETA_FLAG, calibInfo.getData(CALIBRATION_DATA_OPT_SIGMABETA_FLAG, ISNOTNULL).get().clone());

	pInfo_sigmabeta->remove(CALIBRATION_DATA_OPT_METHOD_TYPE);
	pInfo_sigmabeta->add(CALIBRATION_DATA_OPT_METHOD_TYPE, calibInfo.getData(CALIBRATION_DATA_OPT_METHOD_TYPE + SIGMABETA, ISNOTNULL).get().clone());

	pInfo_sigmabeta->remove(PRICING_DATA_FXVOLGRIDS);
	pInfo_sigmabeta->add(PRICING_DATA_FXVOLGRIDS, calibInfo.getData(PRICING_DATA_FXVOLGRIDS, ISNOTNULL).get().clone());

	pInfo_sigmabeta->remove(PRICING_DATA_FXBETAGRIDS);
	pInfo_sigmabeta->add(PRICING_DATA_FXBETAGRIDS, calibInfo.getData(PRICING_DATA_FXBETAGRIDS, ISNOTNULL).get().clone());

	// set reference
	mpCaibEngine->remove(PRICING_DATA_SIGMABETACALIBINFO);
	mpCaibEngine->add(PRICING_DATA_SIGMABETACALIBINFO, new LADataReference()).convertFromString(infoName_sigmabeta);

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

	LADataMultiReference &refData = dynamic_cast<LADataMultiReference &>(mpCaibEngine->getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
	for (unsigned int i = 0; i < refData.getSize(); ++i)
	{
		LAMathObjectValue &data = dynamic_cast<LAMathObjectValue &>(refData.get(i).get());
		const double term = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_FXTERM, ISNOTNULL).get()).get();
		// set initial val
		data.LAObject::remove(PRICING_DATA_INITIALVALUE);
		data.LAObject::add(PRICING_DATA_INITIALVALUE, calibInfo.getData(PRICING_DATA_INITIALVALUE, ISNOTNULL).get().clone());

		const double dDF = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_DOMESTICDF, ISNOTNULL).get()).get();
		const double fDF = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_FOREIGNDF, ISNOTNULL).get()).get();
		const double fwdFX = dynamic_cast<const LADataDouble &>(data.getData(PRICING_DATA_FORWARDFX, ISNOTNULL).get()).get();
		const DoubleVector &volVec = dynamic_cast<const LADataDoubles &>(data.getData(PRICING_DATA_VOLATILITYS, ISNOTNULL).get()).get();
		const DoubleVector &strikeVec = dynamic_cast<const LADataDoubles &>(data.getData(PRICING_DATA_STRIKES, ISNOTNULL).get()).get();
		const BoolVector &isCallVec = dynamic_cast<const LADataBools &>(data.getData(PRICING_DATA_ISCALLS, ISNOTNULL).get()).get();
		const DoubleVector &weightVec = dynamic_cast<const LADataDoubles &>(data.getData(PRICING_DATA_WEIGHTS, ISNOTNULL).get()).get();

		
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
		//set data for 3F
		DoubleArray premVec_(MARKETNUM);
		DoubleArray strikeVec_(MARKETNUM);
		DoubleArray weightVec_(MARKETNUM);
		BoolVector isCallVec_(MARKETNUM);
		// 10Delta Low 
		premVec_[0] = premVec[4];
		strikeVec_[0] = strikeVec[4];
		weightVec_[0] = weightVec[4];
		isCallVec_[0] = isCallVec[4];
		// 25Delta Low 
		premVec_[1] = premVec[3];
		strikeVec_[1] = strikeVec[3];
		weightVec_[1] = weightVec[3];
		isCallVec_[1] = isCallVec[3];
		// ATM
		premVec_[2] = premVec[0];
		strikeVec_[2] = strikeVec[0];
		weightVec_[2] = weightVec[0];
		isCallVec_[2] = isCallVec[0];
		// 25Delta High
		premVec_[3] = premVec[1];
		strikeVec_[3] = strikeVec[1];
		weightVec_[3] = weightVec[1];
		isCallVec_[3] = isCallVec[1];
		// 10Delta High
		premVec_[4] = premVec[2];
		strikeVec_[4] = strikeVec[2];
		weightVec_[4] = weightVec[2];
		isCallVec_[4] = isCallVec[2];
		//set as data
		data.LAObject::remove(PRICING_DATA_OPTIONPREMIUMS);
		data.LAObject::add(PRICING_DATA_OPTIONPREMIUMS, new LADataDoubles(premVec_));
		data.LAObject::remove(PRICING_DATA_STRIKES);
		data.LAObject::add(PRICING_DATA_STRIKES, new LADataDoubles(strikeVec_));
		data.LAObject::remove(PRICING_DATA_WEIGHTS);
		data.LAObject::add(PRICING_DATA_WEIGHTS, new LADataDoubles(weightVec_));
		data.LAObject::remove(PRICING_DATA_ISCALLS);
		data.LAObject::add(PRICING_DATA_ISCALLS, new LADataBools(isCallVec_));

	}
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibratePtberg3F set up end" << endl;
}

// 
/*!
    @brief calibrate
*/
void
LACalibratePtberg3F::doCalibrate()
{
	//calibration start
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibratePtberg3F calibrate called.." << endl;
	LACalibratePtberg::doCalibrate();
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibratePtberg3F calibrate end.." << endl;
}
