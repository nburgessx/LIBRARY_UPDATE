/*! @file
    @brief HW calibration request
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateFXStrangleSolver.cpp
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
#include "LACalibrateFXStrangleSolver.h"
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
#include "LAMathDateUtilities.h"
#include "LAMathCurveFuncUtility.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathValuableEntity.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#include "LAPriceFXVolatility.h"
#include "LAMathDateCalculations.h"
#include "LAMathFXVanillaFuncUtility.h"
#include "LACoreDataService.h"
#include <sstream>
#include "LABasic.h"

using namespace std;
// constructor
/*!

*/
LACalibrateFXStrangleSolver::LACalibrateFXStrangleSolver()
:LACalibrate(), mSpotRate(0.0)
{
}

// destructor
/*!

*/
LACalibrateFXStrangleSolver::~LACalibrateFXStrangleSolver()
{
}


// 
/*!
    @brief setup
*/
void
LACalibrateFXStrangleSolver::setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos)
{
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateFXStrangleSolver set up called" << endl;

	const LAString &calibInfoName = param.refName[0];
	const LAObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();

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


	
	// spot fx
    LAMathFXEntity* forward_fx = LAMarketData::getFXEntity(objPool, "FORWARDRATE");
    if(forward_fx == NULL){
        throw LACoreInvalidData("fx object is not registered.", __FILE__, __LINE__);
    }
	LAMathFXEntity fx_tmp = *forward_fx;
	fx_tmp.getFXType() = "FIXEDRATE";
	LAStringVector ccys;
	LAMarketData::convertToCurrency(param.ccy, ccys);
	// set spot rate
	mSpotRate = fx_tmp.getRate(ccys[1], ccys[0], 0.0);
	//add extra base param
	mSpotRate += param.extraBaseParam;
	//
	//LAMathPlainVanillaEntity& eplain = *(LAMarketData::getPlainVanillaEntity(objPool));
	//LAMathFXEntity& efx = dynamic_cast<LAMathFXEntity &>(eplain.getFXEntity().get().get());
	//LAStringVector ccys;
	//LAMarketData::convertToCurrency(param.ccy, ccys);
	//// set spot rate
	//mSpotRate = efx.getRate(ccys[1], ccys[0], 0.0);
	////add extra base param
	//mSpotRate += param.extraBaseParam;

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
	LADate spotDate = fx_tmp.getSpotDate(ccys[0], ccys[1], asofDate);
	mSpotRate *= dYield.getBasisDF(asofDate, spotDate) / fYield.getBasisDF(asofDate, spotDate);
	
	// optionmaturity

	const LAStringVector &optionMatVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
	unsigned int vecSize = optionMatVec.size();
	const DateVector &optionMatDates = dynamic_cast<const LADataDates &>(calibInfo.getData("MaturityDates", ISNOTNULL).get()).get();
	const DateVector &optionDelDates = dynamic_cast<const LADataDates &>(calibInfo.getData("DeliveryDates", ISNOTNULL).get()).get();
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
					// add shift val
					if (termVec[0] == FXVOL_ATM)
						(*pTargetVol)[indx] = LAMath::max((*pTargetVol)[indx] + shiftVal, 0.0);
					else
						(*pTargetVol)[indx] = (*pTargetVol)[indx] + shiftVal;
					
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
				for(unsigned int k=0;k <= static_cast<unsigned int>(gridPos) ;k++)
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
					
					// add shift val
					if (termVec[0] == FXVOL_ATM)
						(*pTargetVol)[tmpindx] = LAMath::max((*pTargetVol)[tmpindx] + shiftVal, 0.0);
					else
						(*pTargetVol)[tmpindx] = (*pTargetVol)[tmpindx] + shiftVal;
				
				}
			}
			// reconvert to calib format
			LAMarketData::convFXVolMarket2Calib(volATMVec, vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec,
												vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec);

		}
	}

	// reconvert to calib format
	LAMarketData::convFXVolCalib2Market(volATMVec, vol25DHVec, vol10DHVec, vol25DLVec, vol10DLVec,
												vol25BFVec, vol10BFVec, vol25RRVec, vol10RRVec);


	
	
	////approximation flag
	//LAString strisaproxm = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_ISAPPROXIMATION);
	//info->add("IsApptoximation", new LADataBool()).convertFromString(strisaproxm);
	//
	////interpolationmethod
	//LAString strinterp = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_INTERPOLATIONMETHOD);
	//info->add("InterpolationMethod", new LADataString()).convertFromString(strinterp);

	//LAString strdeltastrike = mpCalibStaticData->getStaticData(keyFX + FX_KEY_CALIB_STRGLSLV_DELTAORSTRIKE);
	//info->add("DeltaOrStrike", new LADataString()).convertFromString(strdeltastrike);
	//

	const LAStringVector& deltaTypes = dynamic_cast<const LADataStrings &>(calibInfo.getData("DeltaTypes", ISNOTNULL).get()).get();
	const LAStringVector& atmTypes = dynamic_cast<const LADataStrings &>(calibInfo.getData("ATMTypes", ISNOTNULL).get()).get();
	mWingFactor = dynamic_cast<const LADataDoubles &>(calibInfo.getData("WingFactors", ISNOTNULL).get()).get();

	LAString interpMethod = dynamic_cast<const LADataString &>(calibInfo.getData("InterpolationMethod", ISNOTNULL).get()).get();
	LAMathFXVolatilitySurfaceGenerate::SetInterpolationMethod(interpMethod, mMethod );

	LAString target = dynamic_cast<const LADataString &>(calibInfo.getData("DeltaOrStrike", ISNOTNULL).get()).get();
	LAMathFXVolatilitySurfaceGenerate::SetInterpolationTarget( target, mTarget );

	LAString variable = dynamic_cast<const LADataString &>(calibInfo.getData("Variable", ISNOTNULL).get()).get();
	LAMathFXVolatilitySurfaceGenerate::SetInterpolationVariable( variable, mVariable );
	if (mVariable != VariableLogStrike)
		throw LACoreInvalidData("Only LogStrike is supported", __FILE__,__LINE__);

	LAString matumethod = dynamic_cast<const LADataString &>(calibInfo.getData("MaturityMethod", ISNOTNULL).get()).get();
	LAMathFXVolatilitySurfaceGenerate::SetATMInterpolationMethod( matumethod, mAtmMethod );

	mIsApproximation  = dynamic_cast<const LADataBool &>(calibInfo.getData("IsApptoximation", ISNOTNULL).get()).get();
	//is wing
	mIsWing = dynamic_cast<const LADataBool &>(calibInfo.getData("IsWing", ISNOTNULL).get()).get();

	//Spot Calneder
	LAString calStr = dynamic_cast<const LADataString &>(calibInfo.getData("SpotCalender",ISNOTNULL).get()).get();
	LAPriceDataCalendar cal;
	cal.convertFromString(calStr);
	
	mFxParams.resize(vecSize);
	mSmileParams.resize(vecSize);
	mSmileData.resize(vecSize);
	
	for (unsigned int i = 0; i < vecSize; i++)
	{
		/*mFxParams[i] = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*mpDataInstance,dYieldDataName,fYieldDataName,optionMatDates[i],
                mSpotRate,deltaTypes[i],atmTypes[i]);*/

		//LAString cal;
//		LAPriceDataCalendar cal;
		mFxParams[i] = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*mpDataInstance,dYieldDataName,fYieldDataName,optionMatDates[i],
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
			if( LAMath::abs(mSmileParams[i].lowBF)<=EPS_Vol1 )
            {
                mSmileParams[i].lowBF = LAMath::sign(EPS_Vol1, mSmileParams[i].lowBF);
            }

            if( LAMath::abs(mSmileParams[i].highBF)<=EPS_Vol1 )
            {
                mSmileParams[i].highBF = LAMath::sign(EPS_Vol1, mSmileParams[i].highBF);
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
			if( LAMath::abs(mSmileParams[i].lowRR)<=EPS_Vol1 )
            {
                mSmileParams[i].lowRR = LAMath::sign(EPS_Vol1, mSmileParams[i].lowRR);
            }

            if( LAMath::abs(mSmileParams[i].highRR)<=EPS_Vol1 )
            {
                mSmileParams[i].highRR = LAMath::sign(EPS_Vol1, mSmileParams[i].highRR);
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
	
	mCalibIDName = param.calcType + "_FXCalibInfoEntity_"  + dYieldDataName + "_" + fYieldDataName + "_" + LAString(gridPos);
	
	
	// set method
	mpFunc = method;

	return;
}

// 
/*!
    @brief calibrate
*/
void
LACalibrateFXStrangleSolver::doCalibrate()
{
	//calibration start
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateFXStrangleSolver calibrate called.." << endl;
	
	if (!mpDataInstance)
	{
		throw LACoreInvalidData("DataInstance member is NULL", __FILE__, __LINE__);
	}

	LAObjectPool &objPool = mpDataInstance->getObjectPool();

	//calibration engine
	unsigned int vecSize = mFxParams.size();
	for (unsigned int i = 0; i < vecSize; i++)
	{
		
		if (mIsApproximation)
		{
			LAString msg;
			mSmileData[i] = LAMathFXVolatilitySurfaceGenerate::BuildSmile(mSmileParams[i], mFxParams[i], mIsWing, mWingFactor[i],msg);
		}
		else 
		{
			mSmileData[i] = LAMathFXVolatilitySurfaceGenerate::FindStrangleVol(mFxParams[i], mSmileParams[i], mMethod, mTarget, mIsWing, mWingFactor[i]);
		}
		
	}

	LAMathVolFuncFXStrangleSolver* method = new LAMathVolFuncFXStrangleSolver(mMethod,mTarget,mVariable,mAtmMethod,mFxParams,mSmileData,mIsWing);
	mpFunc->setRealFunction(*method);
	mpFunc->setOn();

	delete method;
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateFXStrangleSolver calibrate end.." << endl;
}
