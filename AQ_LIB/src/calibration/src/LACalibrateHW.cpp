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
#include "LACoreAppError.h"
#include "LAObjectPool.h"
#include "LACoreReferencePool.h"
#include "LADataReference.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMultiReference.h"
#include "LADataProcedure.h"
#include "LAStepInterpolation.h"
#include "LAScenarioConfiguration.h"
#include "LACoreDataService.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAMathHWFuncMR.h"
#include "LAMathHWFuncSigma.h"
#include "LAMathVolFuncHW.h"
#include "LAMathJamshidianSwaption.h"
#include "LAMathJamshidianSwaptionByImplyVol.h"
#include "LAMathDateUtilities.h"
#include "LAMathCurveFuncUtility.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathValuableEntity.h"
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
LACalibrateHW::setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos)
{
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateHW set up called" << endl;
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
	// set refference
	calibID->add(PRICING_DATA_CURVEID, new LADataReference()).convertFromString(curveIDName);
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
	LAString calibMethod = FN_JAMSHIDIANSWAPTIONBYIMPLYVOL_STR;
	const LAString &fitTarget = dynamic_cast<const LADataString &>(calibInfo.getData(PRICING_DATA_FITTINGTARGET, ISNOTNULL).get()).get();
	if (fitTarget == CALIB_TARGET_PREMIUM)
	{
		calibMethod = FN_JAMSHIDIANSWAPTION_STR;
	}

	// shift forward and strike
	const double swaptionMarketForwardShift(dynamic_cast<const LADataDouble &>(calibInfo.getData(IR_CALIBRATION_DATA_SWAPTIONMARKETFORWARDSHIFT, ISNOTNULL).get()).get());

	// swaption vol type
	const LAString voltype(dynamic_cast<const LADataString &>(calibInfo.getData(IR_CALIBRATION_DATA_SWAPTIONVOLTYPE, ISNOTNULL).get()).get());

	// "fn_jamshidianswaptionbyimplyvol" is not supported with except to black vol
	if ((fitTarget == CALIB_TARGET_VOLATILITY && voltype != VOLATITY_BLACK) ||
		(fitTarget == CALIB_TARGET_VOLATILITY && voltype == VOLATITY_BLACK && swaptionMarketForwardShift != 0.))
	{
		LAString msg;
		msg = LAString("Fit to this volatility type is not supported in HW calibration.");
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	// get market info
	// optionmaturity
	const LAStringVector &optionMatVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
	// swapterm
	const LAStringVector &sTenorVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_SWAPTENOR, ISNOTNULL).get()).get();
	// black vol
	DoubleVector blackVolVec = dynamic_cast<const LADataDoubles &>(calibInfo.getData(IR_CALIBRATION_DATA_BLACKVOLATILITY, ISNOTNULL).get()).get();
	
	// shift black vol
	if (gridPos < 0)
	{
		// parallel case
		if (param.isParallel && !param.paraShiftVec.empty())
		{
			LAString shiftType = param.shiftType;
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
					blackVolVec[i] = LAMath::max(blackVolVec[i], 0.0);
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
					//int pos = term.findString(CALIB_MARKET_SWAPTION);
					//if (pos < 0)
					if (term.findString(CALIB_MARKET_SWAPTION) == -1)
					{
						throw LACoreInvalidData("HW Market data swaption only.", __FILE__, __LINE__);
					}
					//unsigned int gStartPos = 9;
					LAStringVector tmpGridTerm = term.toToken('_');
					//LAString grid = term.subString(gStartPos, term.size() - 1);
					LAString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];
					unsigned int gridSize = optionMatVec.size();
					// create mat and swapterm index vex
					LAStringVector tmpGridVec(gridSize);
					for (unsigned int j = 0; j < gridSize; ++j)
					{
						tmpGridVec[j] = optionMatVec[j] + "_" + sTenorVec[j];
					}
					// check term
					LAStringVector::const_iterator it = find(tmpGridVec.begin(), tmpGridVec.end(), grid);
					if (it == tmpGridVec.end())
					{
						LAString msg = "Shift grid is not in swaption market. grid = " + grid;
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					unsigned int index = static_cast<unsigned int>(it - tmpGridVec.begin());
					
					double shiftVal = param.paraShiftVec[i];
					if (shiftType == RISK_SHIFTTYPE_RATIO)
					{
						shiftVal = blackVolVec[index] * shiftVal;
					}
					// add shift val
					blackVolVec[index] = LAMath::max(blackVolVec[index] + shiftVal, 0.0);
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
				LAString msg = "Grid shift param is not exist, grid num = " + LAString(gridPos);
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			// check
			LAString term = param.gridTerm[gridPos];
			term.toUpper();
			//int pos = term.findString(CALIB_MARKET_SWAPTION);
			//if (pos != 0)
			if (term.findString(CALIB_MARKET_SWAPTION) == -1)
			{
				throw LACoreInvalidData("HW Market data swaption only.", __FILE__, __LINE__);
			}
			//unsigned int gStartPos = 9;
			LAStringVector tmpGridTerm = term.toToken('_');
			//LAString grid = term.subString(gStartPos, term.size() - 1);
			LAString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];
			unsigned int gridSize = optionMatVec.size();
			// create mat and swapterm index vex
			LAStringVector tmpGridVec(gridSize);
			for (unsigned int i = 0; i < gridSize; ++i)
			{
				tmpGridVec[i] = optionMatVec[i] + "_" + sTenorVec[i];
			}
			// check term
			LAStringVector::const_iterator it = find(tmpGridVec.begin(), tmpGridVec.end(), grid);
			if (it == tmpGridVec.end())
			{
				LAString msg = "Shift grid is not in swaption market. grid = " + grid;
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			unsigned int index = static_cast<unsigned int>(it - tmpGridVec.begin());

			// shift val
			if(!param.isWave)
			{
				// shift val
				double shiftVal = param.gridShiftVec[gridPos];			
				LAString shiftType = param.shiftType;
				if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
				{
					shiftVal = blackVolVec[index] * shiftVal;
				}
				// add shift val
				blackVolVec[index] = LAMath::max(blackVolVec[index] + shiftVal, 0.0);
			}
			else
			{
				for(unsigned int k =0; k <= gridPos; k++)
				{
					LAString term = param.gridTerm[k];
					term.toUpper();
					//int pos = term.findString(CALIB_MARKET_SWAPTION);
					//if (pos != 0)
					if (term.findString(CALIB_MARKET_SWAPTION) == -1)
					{
						throw LACoreInvalidData("HW Market data swaption only.", __FILE__, __LINE__);
					}
					//unsigned int gStartPos = 9;
					LAStringVector tmpGridTerm = term.toToken('_');
					//LAString grid = term.subString(gStartPos, term.size() - 1);
					LAString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];
					LAStringVector::const_iterator it = find(tmpGridVec.begin(), tmpGridVec.end(), grid);
					if (it == tmpGridVec.end())
					{
						LAString msg = "Shift grid is not in swaption market. grid = " + grid;
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
					}
					//unsigned int index = static_cast<unsigned int>(it - tmpGridVec.begin());
					unsigned int tmpindex = static_cast<unsigned int>(it - tmpGridVec.begin());

					double shiftVal = param.gridShiftVec[k];			
					LAString shiftType = param.shiftType;
					if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
					{
						shiftVal = blackVolVec[tmpindex] * shiftVal;
					}
					// add shift val
					blackVolVec[tmpindex] = LAMath::max(blackVolVec[tmpindex] + shiftVal, 0.0);
						
				}
			}
		}
	}

	// daycount
	const LAStringVector &dayCountVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_MODEL_DATA_DAYCOUNT, ISNOTNULL).get()).get();
	// frequency
	const LAStringVector &freqVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_MODEL_DATA_FREQUENCY, ISNOTNULL).get()).get();
	// noticeperiod
	const LAStringVector &noticeVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_CALIBRATION_DATAOTICEPERIOD, ISNOTNULL).get()).get();
	// fixingcalendar
	const LAStringVector &fCalVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_FIXINGCALENDAR, ISNOTNULL).get()).get();
	// paymentcalendar
	const LAStringVector &pCalVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_PAYMENTCALENDAR, ISNOTNULL).get()).get();
	// slidingrule
	const LAStringVector &slidingVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL).get()).get();
	// strike
	const LAStringVector &strikeVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_STRIKE, ISNOTNULL).get()).get();
	// optiontype
	const LAStringVector &opTypeVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL).get()).get();
	// weight
	const LAStringVector &weightVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_WEIGHT, ISNOTNULL).get()).get();

	LAString optBuy(CALIB_OPT_BUY);
	LAString interSprine(CALIB_INTER_SPLINE);
	LAString ratePer(CALIB_RATE_PER);

	LAString marketRef;
	LADate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
	const unsigned int gridSize = optionMatVec.size();
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		// get each market val
		LAString strOpMat = optionMatVec[i];
		LAString strSTenor = sTenorVec[i];
		LAString strDayCount = dayCountVec[i];
		LAString strFreq = freqVec[i];
		LAString strNotice = noticeVec[i];
		LAString strFCal = fCalVec[i];
		LAString strPCal = pCalVec[i];
		LAString strSliding = slidingVec[i];
		LAString strStrike = strikeVec[i];
		LAString strOpType = opTypeVec[i];
		LAString strWeight = weightVec[i];

		// calc date
		LADate expDate = LAMathDateUtilities::getDate(asofDate, strOpMat, strSliding, strFCal);
		//LADate valueDate = LAMathDateUtilities::getDate(asofDate, strNotice, strSliding, strFCal);
		LADate startDate = LAMathDateUtilities::getDate(expDate, strNotice, strSliding, strPCal);

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
			LAString msg;
			msg = LAString("Swaption volatility type: ") + voltype + " is not supported in HW calibration.";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		// set up market generator
		LAMathObjectValue *mGen = 0;
		LAString mGenName = mCalibIDName + "_" + strOpMat + "_" + strSTenor;
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
		// optionmaturity
		mGen->LAObject::add(IR_CALIBRATION_DATA_OPTIONMATURITY, new LADataString(strOpMat));
		// swaptenor
		mGen->LAObject::add(IR_CALIBRATION_DATA_SWAPTENOR, new LADataString(strSTenor));
		// frequency
		mGen->LAObject::add(IR_MODEL_DATA_FREQUENCY, new LADataString(strFreq));
		// optiontype
		mGen->LAObject::add(PRICING_DATA_OPTIONTYPE, new LADataString(strOpType));
		// noticeperiod
		mGen->LAObject::add(PRICING_CALIBRATION_DATAOTICEPERIOD, new LADataString(strNotice));
		// black vol
		mGen->LAObject::add(IR_CALIBRATION_DATA_BLACKVOLATILITY, new LADataDouble(blackVolVec[i]));
		// strike
		mGen->LAObject::add(PRICING_DATA_STRIKE, new LADataDouble(strike));
		// premium
		mGen->LAObject::add(PRICING_DATA_OPTIONPREMIUM, new LADataDouble(premium));
		// weight
		mGen->LAObject::add(PRICING_DATA_WEIGHT, new LADataDouble(strWeight.getDoubleValue()));
		// modelparam
		mGen->LAObject::add(PRICING_DATA_MODELPARAM, new LADataReference()).convertFromString(mCalibIDName);
		// slidingrule
		mGen->LAObject::add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(strSliding);
		// set daycount attr
		mGen->LAObject::add(IR_MODEL_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(strDayCount);
		// fixingcalendar
		mGen->LAObject::add(PRICING_DATA_FIXINGCALENDAR, new LAPriceDataCalendar()).convertFromString(strFCal);
		// paymentcalendar
		mGen->LAObject::add(IR_CALIBRATION_DATA_PAYMENTCALENDAR, new LAPriceDataCalendar()).convertFromString(strPCal);
		// set calib method
		mGen->setValuationMethod(calibMethod);
		
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
	calibEngine->add(PRICING_DATA_CALIBRATORENGINE, new LADataProcedure()).convertFromString(FN_IR_HWCALIBRATION_STR);
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

		LAObject *e = new LAObject();
		e->add(CALIBRATION_DATA_NAME, new LADataString(mCalibIDName));
		e->add(PRICING_DATA_CALIBCANONICAL_T, new LADataDoubles()).convertFromString(dataVec[0]);
		e->add(PRICING_DATA_CALIBMEANREV_T, new LADataDoubles()).convertFromString(dataVec[1]);
		e->add(PRICING_DATA_CALIBVOL_T, new LADataDoubles()).convertFromString(dataVec[2]);
		e->add(PRICING_DATA_CURVEID, new LADataReference()).convertFromString(dataVec[3]);
		e->add(PRICING_DATA_ISTIMEDEPENDMEANREV, new LADataBool()).convertFromString(dataVec[4]);

		objPool.set(mCalibIDName, e);
	}
	// get calib param
	LAObjectHolder calibID = objPool.getObject(mCalibIDName, ENCHKTYPE_ISDEFINED);

	// tenor
	const DoubleArray &tenor = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_CALIBCANONICAL_T, ISDEFINED).get()).get();
	// mean reversion
	const DoubleArray &meanVec = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_CALIBMEANREV_T, ISDEFINED).get()).get();
	// vol
	const DoubleArray &volVec = dynamic_cast<const LADataDoubles &>(calibID.getData(PRICING_DATA_CALIBVOL_T, ISDEFINED).get()).get();

	// check
	if (tenor.size() != meanVec.size() || tenor.size() != volVec.size())
	{
		throw LACoreInvalidData("Calibrated data format is wroing.", __FILE__, __LINE__);
	}
	// mean reversion method
	bool isTMRV = dynamic_cast<const LADataBool &>(calibID.getData(PRICING_DATA_ISTIMEDEPENDMEANREV, ISNOTNULL).get()).get();
	LAMathHWFuncMR *funcMR = 0;
	if (isTMRV)
	{
		funcMR = new LAMathHWFuncMRTMDPT(tenor, meanVec, *(new LAStepInterpolation()));
	}
	else
	{
		funcMR = new LAMathHWFuncMR(meanVec[0]);
	}
	// hull-white volatility parameter
	LAMathHWFuncSigma *funcSigma = new LAMathHWFuncSigmaTMDPT(tenor, volVec, *(new LAStepInterpolation()));
	LAMathVolFuncHW *funcHW = new LAMathVolFuncHW(*funcMR, *funcSigma);

	mpFunc->setRealFunction(*funcHW);
	mpFunc->setOn();

	// set object pool as calib data
	const LAString &yieldDataName = dynamic_cast<const LADataReference &>(calibID.getData(PRICING_DATA_CURVEID, ISNOTNULL).get()).get().getName();
	LAString calibDataName = LAMarketData::getCalibDataName(mCalcType, yieldDataName, mGridPos);
	cout << static_cast<int>(LACoreThread::getThreadID()) << " CalibDataName = " << calibDataName << endl;
	if (!objPool.getObject(calibDataName, ENCHKTYPE_NOCHECK).isDefined())
	{
		LAObject *calibData = new LAObject();
		calibData->add(CALIBRATION_DATA_NAME, new LADataString(calibDataName));
		calibData->add(PRICING_DATA_CALIBCANONICAL_T, new LADataDoubles(tenor));
		calibData->add(PRICING_DATA_CALIBMEANREV_T, new LADataDoubles(meanVec));
		calibData->add(PRICING_DATA_CALIBVOL_T, new LADataDoubles(volVec));

		objPool.set(calibDataName, calibData);
	}
	else
	{
		LAObject &calibData = objPool.getObject(calibDataName, ENCHKTYPE_ISDEFINED).get();
		calibData.remove(PRICING_DATA_CALIBCANONICAL_T);
		calibData.add(PRICING_DATA_CALIBCANONICAL_T, new LADataDoubles(tenor));
		calibData.remove(PRICING_DATA_CALIBMEANREV_T);
		calibData.add(PRICING_DATA_CALIBMEANREV_T, new LADataDoubles(meanVec));
		calibData.remove(PRICING_DATA_CALIBVOL_T);
		calibData.add(PRICING_DATA_CALIBVOL_T, new LADataDoubles(volVec));
	}

	delete funcHW;

	if (mSerializeStatus != CALIB_S_DESERIALIZE)
	{
		// if serialize dump
		if (mSerializeStatus == CALIB_S_SERIALIZE)
		{
			///// mod Precision for grid
			//LADataDoubles tmp;
			LADataDoubles tmp(22);
			// tenor
			tmp.set(tenor);
			LAString tenorStr = tmp.convertToString();
			// meanVec
			tmp.set(meanVec);
			LAString meanVecStr = tmp.convertToString();
			// volVec
			tmp.set(volVec);
			LAString volVecStr = tmp.convertToString();
			// isTMRV
			LADataBool tmp2(isTMRV);
			LAString isTMRVStr = tmp2.convertToString();
			// set serialize
			setmSerializeMap(mCalibIDName + "," + tenorStr + "," + meanVecStr + "," + volVecStr + "," + yieldDataName + "," + isTMRVStr);
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
				LAString fileName = mCalibIDName + ".csv";
				fileName.exchange("/","");	
				fileName = dirName + fileName;

				fin.open(fileName.getCString());
				if (!fin)
				{
					fout.open(fileName.getCString());
					unsigned int size = tenor.size();
					for (unsigned int i = 0; i < size; ++i)
					{
						fout << LAString(tenor[i]) << "," << LAString(meanVec[i]) << "," << LAString(volVec[i]) << endl;
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
						LAStringVector lineVec = LAString(c_line).toToken(MARKET_DATA_DELIMITER);
						if (lineVec.size() != 3)
						{
							throw LACoreInvalidData("HW Calib format is wrong", __FILE__, __LINE__);
						}
						t_tenor.push_back(lineVec[0].trimLeft().trimRight().getDoubleValue());
						t_menVec.push_back(lineVec[1].trimLeft().trimRight().getDoubleValue());
						t_volVec.push_back(lineVec[2].trimLeft().trimRight().getDoubleValue());
					}
					LAObjectHolder objHolder = objPool.getObject(calibDataName, ENCHKTYPE_ISDEFINED);
					dynamic_cast<LADataDoubles &>(objHolder.getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get()).set(t_tenor);
					dynamic_cast<LADataDoubles &>(objHolder.getData(PRICING_DATA_CALIBMEANREV_T, ISNOTNULL).get()).set(t_menVec);
					dynamic_cast<LADataDoubles &>(objHolder.getData(PRICING_DATA_CALIBVOL_T, ISNOTNULL).get()).set(t_volVec);
					// mean reversion method
					LAMathHWFuncMR *t_funcMR = 0;
					if (isTMRV)
					{
						t_funcMR = new LAMathHWFuncMRTMDPT(t_tenor, t_menVec, *(new LAStepInterpolation()));
					}
					else
					{
						t_funcMR = new LAMathHWFuncMR(t_menVec[0]);
					}
					// hull-white volatility parameter
					LAMathHWFuncSigma *t_funcSigma = new LAMathHWFuncSigmaTMDPT(t_tenor, t_volVec, *(new LAStepInterpolation()));
					LAMathVolFuncHW *t_funcHW = new LAMathVolFuncHW(*t_funcMR, *t_funcSigma);

					mpFunc->setRealFunction(*t_funcHW);
					mpFunc->setOn();
					delete t_funcHW;
				}
				fin.close();
				//ofstream fout;
				//const LAString dirName = LACoreDataService::getOutputDirectory(); 
				//LAString fileName = mCalibIDName + ".csv";
				//fileName.exchange("/","");	
				//fileName = dirName + fileName;
				//fout.open(fileName.getCString());
				//unsigned int size = tenor.size();
				//for (unsigned int i = 0; i < size; ++i)
				//{
				//	fout << LAString(tenor[i]) << "," << LAString(meanVec[i]) << "," << LAString(volVec[i]) << endl;
				//}
				//fout.close();
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
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateHW calibrate end.." << endl;
cout << "-> time = " << time << endl;
}


