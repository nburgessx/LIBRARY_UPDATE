/*! @file
    @brief HW calibration request
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <algorithm>
#include <iostream>
#include <fstream>
#include "AQLCalibrateFXStrangleSolver.h"
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
#include "AQLMathDateUtilities.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLMathIRVanillaFuncUtility.h"
#include "AQLMathValuableEntity.h"
#include "AQLMathVolFuncFXStrangleSolver.h"
#include "AQLPriceFXVolatility.h"
#include "AQLMathDateCalculations.h"
#include "AQLMathFXVanillaFuncUtility.h"
#include "AQLCoreDataService.h"
#include <sstream>
#include "AQLBasic.h"

using namespace std;
// constructor
/*!

*/
AQLCalibrateFXStrangleSolver::AQLCalibrateFXStrangleSolver()
:AQLCalibrate(), mSpotRate(0.0)
{
}

// destructor
/*!

*/
AQLCalibrateFXStrangleSolver::~AQLCalibrateFXStrangleSolver()
{
}


// 
/*!
    @brief setup
*/
void
AQLCalibrateFXStrangleSolver::setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos)
{
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateFXStrangleSolver set up called" << endl;

	const AQLString &calibInfoName = param.refName[0];
	const AQLObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();

	//first set dataInstance
	mpDataInstance = calibInfo.getDataInstance();


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


	
	// spot fx
    AQLMathFXEntity* forward_fx = AQLMarketData::getFXEntity(objPool, "FORWARDRATE");
    if(forward_fx == NULL){
        throw AQLCoreInvalidData("fx object is not registered.", __FILE__, __LINE__);
    }
	AQLMathFXEntity fx_tmp = *forward_fx;
	fx_tmp.getFXType() = "FIXEDRATE";
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(param.ccy, ccys);
	// set spot rate
	mSpotRate = fx_tmp.getRate(ccys[1], ccys[0], 0.0);
	//add extra base param
	mSpotRate += param.extraBaseParam;
	//
	//AQLMathPlainVanillaEntity& eplain = *(AQLMarketData::getPlainVanillaEntity(objPool));
	//AQLMathFXEntity& efx = dynamic_cast<AQLMathFXEntity &>(eplain.getFXEntity().get().get());
	//AQLStringVector ccys;
	//AQLMarketData::convertToCurrency(param.ccy, ccys);
	//// set spot rate
	//mSpotRate = efx.getRate(ccys[1], ccys[0], 0.0);
	////add extra base param
	//mSpotRate += param.extraBaseParam;

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
	AQLDate spotDate = fx_tmp.getSpotDate(ccys[0], ccys[1], asofDate);
	mSpotRate *= dYield.getBasisDF(asofDate, spotDate) / fYield.getBasisDF(asofDate, spotDate);
	
	// optionmaturity

	const AQLStringVector &optionMatVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
	unsigned int vecSize = optionMatVec.size();
	const DateVector &optionMatDates = dynamic_cast<const AQLDataDates &>(calibInfo.getData("MaturityDates", ISNOTNULL).get()).get();
	const DateVector &optionDelDates = dynamic_cast<const AQLDataDates &>(calibInfo.getData("DeliveryDates", ISNOTNULL).get()).get();
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
	for( unsigned int k = 0;k < vol10DLVec.size();k++)
	{
		vol10DLVec[k] +=basevol;
		vol25DLVec[k] +=basevol;
		volATMVec[k] +=basevol;
		vol25DHVec[k] +=basevol;
		vol10DHVec[k] +=basevol;
	}

	DoubleVector vol25BFVec; // 25ButterFly
	DoubleVector vol10BFVec; // 10ButterFly
	DoubleVector vol25RRVec; // 25RiskReversal
	DoubleVector vol10RRVec; // 10RiskReversal
	// shift black vol
	if (gridPos < 0)
	{
		// parallel case
		if (param.isParallel && !param.paraShiftVec.empty())
		{
			
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
					// add shift val
					if (termVec[0] == FXVOL_ATM)
						(*pTargetVol)[indx] = AQLMath::max((*pTargetVol)[indx] + shiftVal, 0.0);
					else
						(*pTargetVol)[indx] = (*pTargetVol)[indx] + shiftVal;
					
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
			if(!param.isWave)
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
				for(unsigned int k=0;k <= static_cast<unsigned int>(gridPos) ;k++)
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
					
					// add shift val
					if (termVec[0] == FXVOL_ATM)
						(*pTargetVol)[tmpindx] = AQLMath::max((*pTargetVol)[tmpindx] + shiftVal, 0.0);
					else
						(*pTargetVol)[tmpindx] = (*pTargetVol)[tmpindx] + shiftVal;
				
				}
			}
			// reconvert to calib format
			AQLMarketData::convFXVolMarket2Calib(volATMVec, vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec,
												vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec);

		}
	}

	// reconvert to calib format
	AQLMarketData::convFXVolCalib2Market(volATMVec, vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec,
												vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec);


	
	
	////approximation flag
	//AQLString strisaproxm = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_ISAPPROXIMATION);
	//info->add("IsApptoximation", new AQLDataBool()).convertFromString(strisaproxm);
	//
	////interpolationmethod
	//AQLString strinterp = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_INTERPOLATIONMETHOD);
	//info->add("InterpolationMethod", new AQLDataString()).convertFromString(strinterp);

	//AQLString strdeltastrike = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_DELTAORSTRIKE);
	//info->add("DeltaOrStrike", new AQLDataString()).convertFromString(strdeltastrike);
	//

	const AQLStringVector& deltaTypes = dynamic_cast<const AQLDataStrings &>(calibInfo.getData("DeltaTypes", ISNOTNULL).get()).get();
	const AQLStringVector& atmTypes = dynamic_cast<const AQLDataStrings &>(calibInfo.getData("ATMTypes", ISNOTNULL).get()).get();
	mWingFactor = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData("WingFactors", ISNOTNULL).get()).get();

	AQLString interpMethod = dynamic_cast<const AQLDataString &>(calibInfo.getData("InterpolationMethod", ISNOTNULL).get()).get();
	AQLMathFXVolatilitySurfaceGenerate::SetInterpolationMethod(interpMethod, mMethod );

	AQLString target = dynamic_cast<const AQLDataString &>(calibInfo.getData("DeltaOrStrike", ISNOTNULL).get()).get();
	AQLMathFXVolatilitySurfaceGenerate::SetInterpolationTarget( target, mTarget );

	AQLString variable = dynamic_cast<const AQLDataString &>(calibInfo.getData("Variable", ISNOTNULL).get()).get();
	AQLMathFXVolatilitySurfaceGenerate::SetInterpolationVariable( variable, mVariable );
	if (mVariable != VariableLogStrike)
		throw AQLCoreInvalidData("Only LogStrike is supported", __FILE__,__LINE__);

	AQLString matumethod = dynamic_cast<const AQLDataString &>(calibInfo.getData("MaturityMethod", ISNOTNULL).get()).get();
	AQLMathFXVolatilitySurfaceGenerate::SetATMInterpolationMethod( matumethod, mAtmMethod );

	mIsApproximation  = dynamic_cast<const AQLDataBool &>(calibInfo.getData("IsApptoximation", ISNOTNULL).get()).get();
	//is wing
	mIsWing = dynamic_cast<const AQLDataBool &>(calibInfo.getData("IsWing", ISNOTNULL).get()).get();

	//Spot Calneder
	AQLString calStr = dynamic_cast<const AQLDataString &>(calibInfo.getData("SpotCalender",ISNOTNULL).get()).get();
	AQLPriceDataCalendar cal;
	cal.convertFromString(calStr);
	
	mFxParams.resize(vecSize);
	mSmileParams.resize(vecSize);
	mSmileData.resize(vecSize);
	
	for (unsigned int i = 0; i < vecSize; i++)
	{
		/*mFxParams[i] = AQLMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*mpDataInstance,dYieldDataName,fYieldDataName,optionMatDates[i],
                mSpotRate,deltaTypes[i],atmTypes[i]);*/

		//AQLString cal;
//		AQLPriceDataCalendar cal;
		mFxParams[i] = AQLMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*mpDataInstance,dYieldDataName,fYieldDataName,optionMatDates[i],
			optionDelDates[i],mSpotRate,deltaTypes[i],atmTypes[i],cal);
	
		mSmileParams[i].atmVol = volATMVec[i];
        mSmileParams[i].lowRR  = vol10RRVec[i];
        mSmileParams[i].lowBF  = vol10BFVec[i];
        mSmileParams[i].highRR = vol25RRVec[i];
        mSmileParams[i].highBF = vol25BFVec[i];
	}

	//adjust mSileParam
	if (mIsApproximation && mIsWing)
	{
		for(unsigned int i = 0; i < vecSize;i++)
        {
		   
            //smile param adjust
			if( AQLMath::abs(mSmileParams[i].lowBF)<=EPS_Vol1 )
            {
                mSmileParams[i].lowBF = AQLMath::sign(EPS_Vol1, mSmileParams[i].lowBF);
            }

            if( AQLMath::abs(mSmileParams[i].highBF)<=EPS_Vol1 )
            {
                mSmileParams[i].highBF = AQLMath::sign(EPS_Vol1, mSmileParams[i].highBF);
            }

            if( mSmileParams[i].lowBF>0.0 && mSmileParams[i].highBF<0.0 && mSmileParams[i].lowBF>=-mSmileParams[i].highBF )
            {
                mSmileParams[i].highBF = /*0.001*/EPS_Vol1;
            }
            else if( mSmileParams[i].lowBF>0.0 && mSmileParams[i].highBF<0.0 && mSmileParams[i].lowBF<=-mSmileParams[i].highBF )
            {
                mSmileParams[i].lowBF = -/*0.001*/EPS_Vol1;
            }
            else if( mSmileParams[i].lowBF<0.0 && mSmileParams[i].highBF>0.0 && -mSmileParams[i].lowBF>=mSmileParams[i].highBF )
            {
                mSmileParams[i].highBF = -/*0.001*/EPS_Vol1;
            }
            else if( mSmileParams[i].lowBF<0.0 && mSmileParams[i].highBF>0.0 && -mSmileParams[i].lowBF<=mSmileParams[i].highBF )
            {
                mSmileParams[i].lowBF = /*0.001*/EPS_Vol1;
            }

            //smile param adjust
			if( AQLMath::abs(mSmileParams[i].lowRR)<=EPS_Vol1 )
            {
                mSmileParams[i].lowRR = AQLMath::sign(EPS_Vol1, mSmileParams[i].lowRR);
            }

            if( AQLMath::abs(mSmileParams[i].highRR)<=EPS_Vol1 )
            {
                mSmileParams[i].highRR = AQLMath::sign(EPS_Vol1, mSmileParams[i].highRR);
            }

            if( mSmileParams[i].lowRR>0.0 && mSmileParams[i].highRR<0.0 && mSmileParams[i].lowRR>=-mSmileParams[i].highRR )
            {
                mSmileParams[i].highRR = /*0.001*/EPS_Vol1;
            }
            else if( mSmileParams[i].lowRR>0.0 && mSmileParams[i].highRR<0.0 && mSmileParams[i].lowRR<=-mSmileParams[i].highRR )
            {
                mSmileParams[i].lowRR = -/*0.001*/EPS_Vol1;
            }
            else if( mSmileParams[i].lowRR<0.0 && mSmileParams[i].highRR>0.0 && -mSmileParams[i].lowRR>=mSmileParams[i].highRR )
            {
                mSmileParams[i].highRR = -/*0.001*/EPS_Vol1;
            }
            else if( mSmileParams[i].lowRR<0.0 && mSmileParams[i].highRR>0.0 && -mSmileParams[i].lowRR<=mSmileParams[i].highRR )
            {
                mSmileParams[i].lowRR = /*0.001*/EPS_Vol1;
            }

		}
	}


	







	// set calcType
	mCalcType = param.calcType;
	// set gridPos
	mGridPos = gridPos;
	
	mCalibIDName = param.calcType + "_FXCalibInfoEntity_"  + dYieldDataName + "_" + fYieldDataName + "_" + AQLString(gridPos);
	
	
	// set method
	mpFunc = method;

	return;
}

// 
/*!
    @brief calibrate
*/
void
AQLCalibrateFXStrangleSolver::doCalibrate()
{
	//calibration start
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateFXStrangleSolver calibrate called.." << endl;
	
	if (!mpDataInstance)
	{
		throw AQLCoreInvalidData("DataInstance member is NULL", __FILE__, __LINE__);
	}

	AQLObjectPool &objPool = mpDataInstance->getObjectPool();

	//calibration engine
	unsigned int vecSize = mFxParams.size();
	for (unsigned int i = 0; i < vecSize; i++)
	{
		
		if (mIsApproximation)
		{
			AQLString msg;
			mSmileData[i] = AQLMathFXVolatilitySurfaceGenerate::BuildSmile(mSmileParams[i], mFxParams[i], mIsWing, mWingFactor[i],msg);
		}
		else 
		{
			mSmileData[i] = AQLMathFXVolatilitySurfaceGenerate::FindStrangleVol(mFxParams[i], mSmileParams[i], mMethod, mTarget, mIsWing, mWingFactor[i]);
		}
		
	}

	AQLMathVolFuncFXStrangleSolver* method = new AQLMathVolFuncFXStrangleSolver(mMethod,mTarget,mVariable,mAtmMethod,mFxParams,mSmileData,mIsWing);
	mpFunc->setRealFunction(*method);
	mpFunc->setOn();

	delete method;
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateFXStrangleSolver calibrate end.." << endl;
}
