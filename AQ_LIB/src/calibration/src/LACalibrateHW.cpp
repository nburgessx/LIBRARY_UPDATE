/*! @file
    @brief HW calibration request
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateHW.cpp
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
#include "LACalibrateHW.h"
#include "LACalibrationFunc.h"
#include "LADefinitionsCalibration.h"
#include "LAMarketData.h"
#include "AQLCoreAppError.h"
#include "AQLObjectPool.h"
#include "AQLCoreReferencePool.h"
#include "AQLDataReference.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMultiReference.h"
#include "AQLDataProcedure.h"
#include "AQLStepInterpolation.h"
#include "LAScenarioConfiguration.h"
#include "LACoreDataService.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "LAMathHWFuncMR.h"
#include "LAMathHWFuncSigma.h"
#include "LAMathVolFuncHW.h"
#include "LAMathJamshidianSwaption.h"
#include "LAMathJamshidianSwaptionByImplyVol.h"
#include "LAMathDateUtilities.h"
#include "LAMathCurveFuncUtility.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "AQLMathValuableEntity.h"
#include "LAPriceHWCalibration.h"
#include "LACoreDataService.h"
#include <sstream>

using namespace std;


// constructor
/*!

*/
LACalibrateHW::LACalibrateHW()
:LACalibrate()
{
}

// destructor
/*!

*/
LACalibrateHW::~LACalibrateHW()
{
}


// 
/*!
    @brief setup
*/
void
LACalibrateHW::setUp(AQLObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos)
{
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateHW set up called" << endl;
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
	const AQLString calibInfoName = param.refName[0];
	const AQLObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();
	AQLObject *calibID = 0;
	
	unsigned int curPos = 1;
	if (gridPos >= 0)
	{
		curPos += gridPos;
		if (param.isParallel)
		{
			++curPos;
		}
	}
	const AQLString curveName = param.refName[curPos];
	if (!objPool.getObject(curveName).isDefined())
	{
		AQLString msg = curveName + " is not registered in EntityPool";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const AQLString &curveIDName = dynamic_cast<const LAMathYieldCurve &>(objPool.getObject(curveName, ENCHKTYPE_ISDEFINED).get()).getYieldData().get().getName();

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
	mCalibIDName = param.calcType + "_" + calibInfoName + "_" + curveIDName + "_" + AQLString(gridPos);
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
	// set refference
	calibID->add(PRICING_DATA_CURVEID, new AQLDataReference()).convertFromString(curveIDName);
	// set integral grid
	calibID->add(PRICING_DATA_SDEINTEGRALGRID, calibInfo.getData(PRICING_DATA_SDEINTEGRALGRID, ISNOTNULL).get().clone());
	// set cannonical T and vol, men rev
	calibID->add(PRICING_DATA_CALIBCANONICAL_T, calibInfo.getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get().clone());
	calibID->add(PRICING_DATA_CALIBVOL_T, calibInfo.getData(PRICING_DATA_CALIBVOL_T, ISNOTNULL).get().clone());
	calibID->add(PRICING_DATA_CALIBMEANREV_T, calibInfo.getData(PRICING_DATA_CALIBMEANREV_T, ISNOTNULL).get().clone());
	calibID->add(PRICING_DATA_ISTIMEDEPENDMEANREV, calibInfo.getData(PRICING_DATA_ISTIMEDEPENDMEANREV, ISNOTNULL).get().clone());
	// set calibration variable
	calibID->add(PRICING_DATA_CALIBVARIABLES, calibInfo.getData(PRICING_DATA_CALIBVARIABLES, ISNOTNULL).get().clone());

	// calib method
	AQLString calibMethod = FN_JAMSHIDIANSWAPTIONBYIMPLYVOL_STR;
	const AQLString &fitTarget = dynamic_cast<const AQLDataString &>(calibInfo.getData(PRICING_DATA_FITTINGTARGET, ISNOTNULL).get()).get();
	if (fitTarget == CALIB_TARGET_PREMIUM)
	{
		calibMethod = FN_JAMSHIDIANSWAPTION_STR;
	}

	// shift forward and strike
	const double swaptionMarketForwardShift(dynamic_cast<const AQLDataDouble &>(calibInfo.getData(IR_CALIBRATION_DATA_SWAPTIONMARKETFORWARDSHIFT, ISNOTNULL).get()).get());

	// swaption vol type
	const AQLString voltype(dynamic_cast<const AQLDataString &>(calibInfo.getData(IR_CALIBRATION_DATA_SWAPTIONVOLTYPE, ISNOTNULL).get()).get());

	// "fn_jamshidianswaptionbyimplyvol" is not supported with except to black vol
	if ((fitTarget == CALIB_TARGET_VOLATILITY && voltype != VOLATITY_BLACK) ||
		(fitTarget == CALIB_TARGET_VOLATILITY && voltype == VOLATITY_BLACK && swaptionMarketForwardShift != 0.))
	{
		AQLString msg;
		msg = AQLString("Fit to this volatility type is not supported in HW calibration.");
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// get market info
	// optionmaturity
	const AQLStringVector &optionMatVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
	// swapterm
	const AQLStringVector &sTenorVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_SWAPTENOR, ISNOTNULL).get()).get();
	// black vol
	DoubleVector blackVolVec = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(IR_CALIBRATION_DATA_BLACKVOLATILITY, ISNOTNULL).get()).get();
	
	// shift black vol
	if (gridPos < 0)
	{
		// parallel case
		if (param.isParallel && !param.paraShiftVec.empty())
		{
			AQLString shiftType = param.shiftType;
			shiftType.toUpper();
			if (param.paraShiftVec.size() == 1)
			{
				DoubleArray shiftVal(blackVolVec.size(), param.paraShiftVec[0]);
				if (shiftType == RISK_SHIFTTYPE_DIFF)
				{
					// diff case
					transform(blackVolVec.begin(), blackVolVec.end(), shiftVal.begin(), blackVolVec.begin(), plus<double>());
				}
				else
				{
					// ratio case
					DoubleArray plusVal(blackVolVec.size());
					transform(blackVolVec.begin(), blackVolVec.end(), shiftVal.begin(), plusVal.begin(), multiplies<double>());
					transform(blackVolVec.begin(), blackVolVec.end(), plusVal.begin(), blackVolVec.begin(), plus<double>());
				}
				unsigned int size = blackVolVec.size();
				for (unsigned int i = 0; i < size; ++i)
				{
					blackVolVec[i] = AQLMath::max(blackVolVec[i], 0.0);
				}
			}
			else
			{
				unsigned int paraSize = param.paraTerm.size();
				if (paraSize != param.paraShiftVec.size())
				{
					throw AQLCoreInvalidData("Parallel shift val and value is not same ", __FILE__, __LINE__);
				}

				for (unsigned int i = 0; i < paraSize; ++i)
				{
					AQLString term = param.paraTerm[i];
					term.toUpper();
					//int pos = term.findString(CALIB_MARKET_SWAPTION);
					//if (pos < 0)
					if (term.findString(CALIB_MARKET_SWAPTION) == -1)
					{
						throw AQLCoreInvalidData("HW Market data swaption only.", __FILE__, __LINE__);
					}
					//unsigned int gStartPos = 9;
					AQLStringVector tmpGridTerm = term.toToken('_');
					//AQLString grid = term.subString(gStartPos, term.size() - 1);
					AQLString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];
					unsigned int gridSize = optionMatVec.size();
					// create mat and swapterm index vex
					AQLStringVector tmpGridVec(gridSize);
					for (unsigned int j = 0; j < gridSize; ++j)
					{
						tmpGridVec[j] = optionMatVec[j] + "_" + sTenorVec[j];
					}
					// check term
					AQLStringVector::const_iterator it = find(tmpGridVec.begin(), tmpGridVec.end(), grid);
					if (it == tmpGridVec.end())
					{
						AQLString msg = "Shift grid is not in swaption market. grid = " + grid;
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int index = static_cast<unsigned int>(it - tmpGridVec.begin());
					
					double shiftVal = param.paraShiftVec[i];
					if (shiftType == RISK_SHIFTTYPE_RATIO)
					{
						shiftVal = blackVolVec[index] * shiftVal;
					}
					// add shift val
					blackVolVec[index] = AQLMath::max(blackVolVec[index] + shiftVal, 0.0);
				}
			}
		}
	}
	else
	{
		if (param.isGrid && !param.gridShiftVec.empty())
		{
			// grid shift case
			if (param.gridTerm.size() - 1 < static_cast<unsigned int>(gridPos) || 
					param.gridShiftVec.size() - 1 < static_cast<unsigned int>(gridPos))
			{
				AQLString msg = "Grid shift param is not exist, grid num = " + AQLString(gridPos);
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			// check
			AQLString term = param.gridTerm[gridPos];
			term.toUpper();
			//int pos = term.findString(CALIB_MARKET_SWAPTION);
			//if (pos != 0)
			if (term.findString(CALIB_MARKET_SWAPTION) == -1)
			{
				throw AQLCoreInvalidData("HW Market data swaption only.", __FILE__, __LINE__);
			}
			//unsigned int gStartPos = 9;
			AQLStringVector tmpGridTerm = term.toToken('_');
			//AQLString grid = term.subString(gStartPos, term.size() - 1);
			AQLString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];
			unsigned int gridSize = optionMatVec.size();
			// create mat and swapterm index vex
			AQLStringVector tmpGridVec(gridSize);
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				tmpGridVec[i] = optionMatVec[i] + "_" + sTenorVec[i];
			}
			// check term
			AQLStringVector::const_iterator it = find(tmpGridVec.begin(), tmpGridVec.end(), grid);
			if (it == tmpGridVec.end())
			{
				AQLString msg = "Shift grid is not in swaption market. grid = " + grid;
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			unsigned int index = static_cast<unsigned int>(it - tmpGridVec.begin());

			// shift val
			if(!param.isWave)
			{
				// shift val
				double shiftVal = param.gridShiftVec[gridPos];			
				AQLString shiftType = param.shiftType;
				if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
				{
					shiftVal = blackVolVec[index] * shiftVal;
				}
				// add shift val
				blackVolVec[index] = AQLMath::max(blackVolVec[index] + shiftVal, 0.0);
			}
			else
			{
				for(unsigned int k =0; k <= gridPos; k++)
				{
					AQLString term = param.gridTerm[k];
					term.toUpper();
					//int pos = term.findString(CALIB_MARKET_SWAPTION);
					//if (pos != 0)
					if (term.findString(CALIB_MARKET_SWAPTION) == -1)
					{
						throw AQLCoreInvalidData("HW Market data swaption only.", __FILE__, __LINE__);
					}
					//unsigned int gStartPos = 9;
					AQLStringVector tmpGridTerm = term.toToken('_');
					//AQLString grid = term.subString(gStartPos, term.size() - 1);
					AQLString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];
					AQLStringVector::const_iterator it = find(tmpGridVec.begin(), tmpGridVec.end(), grid);
					if (it == tmpGridVec.end())
					{
						AQLString msg = "Shift grid is not in swaption market. grid = " + grid;
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					//unsigned int index = static_cast<unsigned int>(it - tmpGridVec.begin());
					unsigned int tmpindex = static_cast<unsigned int>(it - tmpGridVec.begin());

					double shiftVal = param.gridShiftVec[k];			
					AQLString shiftType = param.shiftType;
					if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
					{
						shiftVal = blackVolVec[tmpindex] * shiftVal;
					}
					// add shift val
					blackVolVec[tmpindex] = AQLMath::max(blackVolVec[tmpindex] + shiftVal, 0.0);
						
				}
			}
		}
	}

	// daycount
	const AQLStringVector &dayCountVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_MODEL_DATA_DAYCOUNT, ISNOTNULL).get()).get();
	// frequency
	const AQLStringVector &freqVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_MODEL_DATA_FREQUENCY, ISNOTNULL).get()).get();
	// noticeperiod
	const AQLStringVector &noticeVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_CALIBRATION_DATAOTICEPERIOD, ISNOTNULL).get()).get();
	// fixingcalendar
	const AQLStringVector &fCalVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_FIXINGCALENDAR, ISNOTNULL).get()).get();
	// paymentcalendar
	const AQLStringVector &pCalVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_PAYMENTCALENDAR, ISNOTNULL).get()).get();
	// slidingrule
	const AQLStringVector &slidingVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get()).get();
	// strike
	const AQLStringVector &strikeVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_STRIKE, ISNOTNULL).get()).get();
	// optiontype
	const AQLStringVector &opTypeVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL).get()).get();
	// weight
	const AQLStringVector &weightVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_WEIGHT, ISNOTNULL).get()).get();

	AQLString optBuy(CALIB_OPT_BUY);
	AQLString interSprine(CALIB_INTER_SPLINE);
	AQLString ratePer(CALIB_RATE_PER);

	AQLString marketRef;
	AQLDate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	const unsigned int gridSize = optionMatVec.size();
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		// get each market val
		AQLString strOpMat = optionMatVec[i];
		AQLString strSTenor = sTenorVec[i];
		AQLString strDayCount = dayCountVec[i];
		AQLString strFreq = freqVec[i];
		AQLString strNotice = noticeVec[i];
		AQLString strFCal = fCalVec[i];
		AQLString strPCal = pCalVec[i];
		AQLString strSliding = slidingVec[i];
		AQLString strStrike = strikeVec[i];
		AQLString strOpType = opTypeVec[i];
		AQLString strWeight = weightVec[i];

		// calc date
		AQLDate expDate = LAMathDateUtilities::getDate(asofDate, strOpMat, strSliding, strFCal);
		//AQLDate valueDate = LAMathDateUtilities::getDate(asofDate, strNotice, strSliding, strFCal);
		AQLDate startDate = LAMathDateUtilities::getDate(expDate, strNotice, strSliding, strPCal);

		// calc strike rate
		double strike = 0.0;
		if (strStrike == CALIB_STRIKE_ATM)
		{
			strike = LAMathCurveFuncUtility::getRate(startDate, strSTenor, calibID->getDataInstance(),
													curveIDName, ratePer, strFreq, strDayCount,
													strSliding, strFCal, interSprine);

		}
		else
		{
			strike = strStrike.getDoubleValue();
		}

		// convertion to premium
		double premium(0.);
		if (voltype == VOLATITY_BLACK)
		{
			premium = LAMathIRVanillaFuncUtility::swaption(calibID->getDataInstance(), curveIDName, optBuy, 
														 strOpType, 1.0, strike + swaptionMarketForwardShift, blackVolVec[i],
														 asofDate, expDate, asofDate, strNotice, strSTenor,
														 strFreq, strSliding, strDayCount, strPCal, 
														 strFCal, strike + swaptionMarketForwardShift);
		}
		else if (voltype == VOLATITY_NORMAL)
		{
			premium = LAMathIRVanillaFuncUtility::swaption_NormalDist(calibID->getDataInstance(), curveIDName, optBuy, 
														            strOpType, 1.0, strike + swaptionMarketForwardShift, blackVolVec[i],
														            asofDate, expDate, asofDate, strNotice, strSTenor,
														            strFreq, strSliding, strDayCount, strPCal, 
														            strFCal, strike + swaptionMarketForwardShift);
		}
		else
		{
			AQLString msg;
			msg = AQLString("Swaption volatility type: ") + voltype + " is not supported in HW calibration.";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		// set up market generator
		AQLMathObjectValue *mGen = 0;
		AQLString mGenName = mCalibIDName + "_" + strOpMat + "_" + strSTenor;
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
		// optionmaturity
		mGen->AQLObject::add(IR_CALIBRATION_DATA_OPTIONMATURITY, new AQLDataString(strOpMat));
		// swaptenor
		mGen->AQLObject::add(IR_CALIBRATION_DATA_SWAPTENOR, new AQLDataString(strSTenor));
		// frequency
		mGen->AQLObject::add(IR_MODEL_DATA_FREQUENCY, new AQLDataString(strFreq));
		// optiontype
		mGen->AQLObject::add(PRICING_DATA_OPTIONTYPE, new AQLDataString(strOpType));
		// noticeperiod
		mGen->AQLObject::add(PRICING_CALIBRATION_DATAOTICEPERIOD, new AQLDataString(strNotice));
		// black vol
		mGen->AQLObject::add(IR_CALIBRATION_DATA_BLACKVOLATILITY, new AQLDataDouble(blackVolVec[i]));
		// strike
		mGen->AQLObject::add(PRICING_DATA_STRIKE, new AQLDataDouble(strike));
		// premium
		mGen->AQLObject::add(PRICING_DATA_OPTIONPREMIUM, new AQLDataDouble(premium));
		// weight
		mGen->AQLObject::add(PRICING_DATA_WEIGHT, new AQLDataDouble(strWeight.getDoubleValue()));
		// modelparam
		mGen->AQLObject::add(PRICING_DATA_MODELPARAM, new AQLDataReference()).convertFromString(mCalibIDName);
		// slidingrule
		mGen->AQLObject::add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(strSliding);
		// set daycount attr
		mGen->AQLObject::add(IR_MODEL_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(strDayCount);
		// fixingcalendar
		mGen->AQLObject::add(PRICING_DATA_FIXINGCALENDAR, new AQLPriceDataCalendar()).convertFromString(strFCal);
		// paymentcalendar
		mGen->AQLObject::add(IR_CALIBRATION_DATA_PAYMENTCALENDAR, new AQLPriceDataCalendar()).convertFromString(strPCal);
		// set calib method
		mGen->setValuationMethod(calibMethod);
		
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
	calibEngine->add(PRICING_DATA_CALIBRATORENGINE, new AQLDataProcedure()).convertFromString(FN_IR_HWCALIBRATION_STR);
	// sdeinfo
	calibEngine->add(PRICING_DATA_SDEINFO, new AQLDataReference()).convertFromString(mCalibIDName);
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
	mpCaibEngine->getDataInstance()->getReferencePool().completeDependency();
	
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateHW set up end" << endl;
}

// 
/*!
    @brief calibrate
*/
void
LACalibrateHW::doCalibrate()
{
	//calibration start
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateHW calibrate called.." << endl;
clock_t cstart = clock();

	if (!mpDataInstance)
	{
		throw AQLCoreInvalidData("DataInstance member is NULL", __FILE__, __LINE__);
	}

	AQLObjectPool &objPool = mpDataInstance->getObjectPool();

	AQLString fileNum = LACoreDataService::getContext(ARG_KEY_FILENUM);
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
		if (dataVec.size() != 5)
		{
			throw AQLCoreInvalidData("Serialize data size must be 5.", __FILE__, __LINE__);
		}

		AQLObject *e = new AQLObject();
		e->add(CALIBRATION_DATA_NAME, new AQLDataString(mCalibIDName));
		e->add(PRICING_DATA_CALIBCANONICAL_T, new AQLDataDoubles()).convertFromString(dataVec[0]);
		e->add(PRICING_DATA_CALIBMEANREV_T, new AQLDataDoubles()).convertFromString(dataVec[1]);
		e->add(PRICING_DATA_CALIBVOL_T, new AQLDataDoubles()).convertFromString(dataVec[2]);
		e->add(PRICING_DATA_CURVEID, new AQLDataReference()).convertFromString(dataVec[3]);
		e->add(PRICING_DATA_ISTIMEDEPENDMEANREV, new AQLDataBool()).convertFromString(dataVec[4]);

		objPool.set(mCalibIDName, e);
	}
	// get calib param
	AQLObjectHolder calibID = objPool.getObject(mCalibIDName, ENCHKTYPE_ISDEFINED);

	// tenor
	const DoubleArray &tenor = dynamic_cast<const AQLDataDoubles &>(calibID.getData(PRICING_DATA_CALIBCANONICAL_T, ISDEFINED).get()).get();
	// mean reversion
	const DoubleArray &meanVec = dynamic_cast<const AQLDataDoubles &>(calibID.getData(PRICING_DATA_CALIBMEANREV_T, ISDEFINED).get()).get();
	// vol
	const DoubleArray &volVec = dynamic_cast<const AQLDataDoubles &>(calibID.getData(PRICING_DATA_CALIBVOL_T, ISDEFINED).get()).get();

	// check
	if (tenor.size() != meanVec.size() || tenor.size() != volVec.size())
	{
		throw AQLCoreInvalidData("Calibrated data format is wroing.", __FILE__, __LINE__);
	}
	// mean reversion method
	bool isTMRV = dynamic_cast<const AQLDataBool &>(calibID.getData(PRICING_DATA_ISTIMEDEPENDMEANREV, ISNOTNULL).get()).get();
	LAMathHWFuncMR *funcMR = 0;
	if (isTMRV)
	{
		funcMR = new LAMathHWFuncMRTMDPT(tenor, meanVec, *(new AQLStepInterpolation()));
	}
	else
	{
		funcMR = new LAMathHWFuncMR(meanVec[0]);
	}
	// hull-white volatility parameter
	LAMathHWFuncSigma *funcSigma = new LAMathHWFuncSigmaTMDPT(tenor, volVec, *(new AQLStepInterpolation()));
	LAMathVolFuncHW *funcHW = new LAMathVolFuncHW(*funcMR, *funcSigma);

	mpFunc->setRealFunction(*funcHW);
	mpFunc->setOn();

	// set object pool as calib data
	const AQLString &yieldDataName = dynamic_cast<const AQLDataReference &>(calibID.getData(PRICING_DATA_CURVEID, ISNOTNULL).get()).get().getName();
	AQLString calibDataName = LAMarketData::getCalibDataName(mCalcType, yieldDataName, mGridPos);
	cout << static_cast<int>(LACoreThread::getThreadID()) << " CalibDataName = " << calibDataName << endl;
	if (!objPool.getObject(calibDataName, ENCHKTYPE_NOCHECK).isDefined())
	{
		AQLObject *calibData = new AQLObject();
		calibData->add(CALIBRATION_DATA_NAME, new AQLDataString(calibDataName));
		calibData->add(PRICING_DATA_CALIBCANONICAL_T, new AQLDataDoubles(tenor));
		calibData->add(PRICING_DATA_CALIBMEANREV_T, new AQLDataDoubles(meanVec));
		calibData->add(PRICING_DATA_CALIBVOL_T, new AQLDataDoubles(volVec));

		objPool.set(calibDataName, calibData);
	}
	else
	{
		AQLObject &calibData = objPool.getObject(calibDataName, ENCHKTYPE_ISDEFINED).get();
		calibData.remove(PRICING_DATA_CALIBCANONICAL_T);
		calibData.add(PRICING_DATA_CALIBCANONICAL_T, new AQLDataDoubles(tenor));
		calibData.remove(PRICING_DATA_CALIBMEANREV_T);
		calibData.add(PRICING_DATA_CALIBMEANREV_T, new AQLDataDoubles(meanVec));
		calibData.remove(PRICING_DATA_CALIBVOL_T);
		calibData.add(PRICING_DATA_CALIBVOL_T, new AQLDataDoubles(volVec));
	}

	delete funcHW;

	if (mSerializeStatus != CALIB_S_DESERIALIZE)
	{
		// if serialize dump
		if (mSerializeStatus == CALIB_S_SERIALIZE)
		{
			///// mod Precision for grid
			//AQLDataDoubles tmp;
			AQLDataDoubles tmp(22);
			// tenor
			tmp.set(tenor);
			AQLString tenorStr = tmp.convertToString();
			// meanVec
			tmp.set(meanVec);
			AQLString meanVecStr = tmp.convertToString();
			// volVec
			tmp.set(volVec);
			AQLString volVecStr = tmp.convertToString();
			// isTMRV
			AQLDataBool tmp2(isTMRV);
			AQLString isTMRVStr = tmp2.convertToString();
			// set serialize
			setmSerializeMap(mCalibIDName + "," + tenorStr + "," + meanVecStr + "," + volVecStr + "," + yieldDataName + "," + isTMRVStr);
			istringstream *dataStream = new istringstream(mSerializeMap[mSerializeFile].getCString());
			LACoreDataService::setIStringStream(mSerializeFile, dataStream);
		}

		//ouput
		const AQLDataHolder &attr = mpCaibEngine->getData(PRICING_DATA_ISOUTPUT, NOCHECK);
		if (attr.isDefined() && !attr.isNull())
		{
			if (dynamic_cast<const AQLDataBool &>(attr.get()).get())
			{
				ifstream fin;
				ofstream fout;
				const AQLString dirName = LACoreDataService::getOutputDirectory(); 
				AQLString fileName = mCalibIDName + ".csv";
				fileName.exchange("/","");	
				fileName = dirName + fileName;

				fin.open(fileName.getCString());
				if (!fin)
				{
					fout.open(fileName.getCString());
					unsigned int size = tenor.size();
					for (unsigned int i = 0; i < size; ++i)
					{
						fout << AQLString(tenor[i]) << "," << AQLString(meanVec[i]) << "," << AQLString(volVec[i]) << endl;
					}
					fout.close();
				}
				else
				{
					DoubleArray t_tenor;
					DoubleArray t_menVec;
					DoubleArray t_volVec;
					string line;
					while (getline(fin, line))
					{
						const char *c_line = line.c_str();
						AQLStringVector lineVec = AQLString(c_line).toToken(MARKET_DATA_DELIMITER);
						if (lineVec.size() != 3)
						{
							throw AQLCoreInvalidData("HW Calib format is wrong", __FILE__, __LINE__);
						}
						t_tenor.push_back(lineVec[0].trimLeft().trimRight().getDoubleValue());
						t_menVec.push_back(lineVec[1].trimLeft().trimRight().getDoubleValue());
						t_volVec.push_back(lineVec[2].trimLeft().trimRight().getDoubleValue());
					}
					AQLObjectHolder objHolder = objPool.getObject(calibDataName, ENCHKTYPE_ISDEFINED);
					dynamic_cast<AQLDataDoubles &>(objHolder.getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get()).set(t_tenor);
					dynamic_cast<AQLDataDoubles &>(objHolder.getData(PRICING_DATA_CALIBMEANREV_T, ISNOTNULL).get()).set(t_menVec);
					dynamic_cast<AQLDataDoubles &>(objHolder.getData(PRICING_DATA_CALIBVOL_T, ISNOTNULL).get()).set(t_volVec);
					// mean reversion method
					LAMathHWFuncMR *t_funcMR = 0;
					if (isTMRV)
					{
						t_funcMR = new LAMathHWFuncMRTMDPT(t_tenor, t_menVec, *(new AQLStepInterpolation()));
					}
					else
					{
						t_funcMR = new LAMathHWFuncMR(t_menVec[0]);
					}
					// hull-white volatility parameter
					LAMathHWFuncSigma *t_funcSigma = new LAMathHWFuncSigmaTMDPT(t_tenor, t_volVec, *(new AQLStepInterpolation()));
					LAMathVolFuncHW *t_funcHW = new LAMathVolFuncHW(*t_funcMR, *t_funcSigma);

					mpFunc->setRealFunction(*t_funcHW);
					mpFunc->setOn();
					delete t_funcHW;
				}
				fin.close();
				//ofstream fout;
				//const AQLString dirName = LACoreDataService::getOutputDirectory(); 
				//AQLString fileName = mCalibIDName + ".csv";
				//fileName.exchange("/","");	
				//fileName = dirName + fileName;
				//fout.open(fileName.getCString());
				//unsigned int size = tenor.size();
				//for (unsigned int i = 0; i < size; ++i)
				//{
				//	fout << AQLString(tenor[i]) << "," << AQLString(meanVec[i]) << "," << AQLString(volVec[i]) << endl;
				//}
				//fout.close();
			}
		}

		// delete data
		AQLDataMultiReference &calibDataRef = dynamic_cast<AQLDataMultiReference &>(mpCaibEngine->getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
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
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateHW calibrate end.." << endl;
cout << "-> time = " << time << endl;
}


