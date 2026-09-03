/*! @file
    @brief HW calibration request
*/
//  2011, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateFXVannaVolga.cpp
//
//  DESCRIPTION :        FXVannaVolga calibration request class
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
#include "LACalibrateFXVannaVolga.h"
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
#include "LAPriceFXVolatility.h"
#include "LAMathDateCalculations.h"
#include "LAMathFXVanillaFuncUtility.h"
#include "LACoreDataService.h"
#include <sstream>
#include "LACalibratePool.h"
#include "LABasic.h"

#include "LACalibrationParametersFXVannaVolga.h"
#include "LADataMatrix.h"
#include "ConstantDeclarations.h"
#include "LAMathVolFuncFXVannaVolga.h"
#include "LAFindRootBrent.h"

using namespace std;
// constructor
/*!

*/
LACalibrateFXVannaVolga::LACalibrateFXVannaVolga()
:LACalibrate(), mSpotRate(0.0)
{
}

// destructor
/*!

*/
LACalibrateFXVannaVolga::~LACalibrateFXVannaVolga()
{
}


// 
/*!
    @brief setup
*/
void
LACalibrateFXVannaVolga::setUp(LAObjectPool &objPool,  const MAScenarioParam &param, MACalibrationFunc *method, int gridPos)
{
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateFXVannaVolga set up called" << endl;
	if (!method)
	{
		throw LACoreInvalidData("Argument method is NULL", __FILE__, __LINE__);
	}
	if (param.refName.size() < 2)
	{
		throw LACoreInvalidData("Param refName is more than be two, reference curve and calibinfo needed.", __FILE__, __LINE__);
	}
	const LAString &calibInfoName = param.refName[0];
	const LAObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();

	// set dataInstance
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
	mdYieldDataName =  dYield.getYieldData().get().getName();

	// get foreign curve
	const LAString fYieldName = param.refName[curPos + 2];
	if (!objPool.getObject(fYieldName).isDefined())
	{
		LAString msg = fYieldName + " is not registered in EntityPool";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const LAMathYieldCurve &fYield = dynamic_cast<const LAMathYieldCurve &>(objPool.getObject(fYieldName, ENCHKTYPE_ISDEFINED).get());
	const LAString &fYieldDataName = fYield.getYieldData().get().getName();
	mfYieldDataName =  fYield.getYieldData().get().getName();

	
	// spot fx
	LAMathFXEntity fx_tmp = *LAMarketData::getFXEntity(objPool, "FORWARDRATE");
	fx_tmp.getFXType() = "FIXEDRATE";
	LAStringVector ccys;
	LAMarketData::convertToCurrency(param.ccy, ccys);
	// set spot rate
	mSpotRate = fx_tmp.getRate(ccys[1], ccys[0], 0.0);
	//add extra base param
	mSpotRate += param.extraBaseParam;
	// asofdate
	LADate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
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
	LADate spotDate = fx_tmp.getSpotDate(ccys[0], ccys[1], asofDate);
	mSpotRate *= dYield.getBasisDF(asofDate, spotDate) / fYield.getBasisDF(asofDate, spotDate);

	// optionmaturity
	const LAStringVector &optionMatVec = dynamic_cast<const LADataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();

	DoubleMatrix volMatrix = dynamic_cast<const LADataDoubleMatrix &>(calibInfo.getData(PRICING_DATA_FXVOLATILITYMATRIX, ISNOTNULL).get()).get();

	const LAStringVector &baseStrikes =  dynamic_cast<const LADataStrings &>(calibInfo.getData(PRICING_DATA_BASESTRIKES, ISNOTNULL).get()).get();

	unsigned int termSize = optionMatVec.size();
	DoubleVector vol10DLVec(termSize, 0.0);
	DoubleVector vol25DLVec(termSize, 0.0);
	DoubleVector volATMVec(termSize, 0.0);
	DoubleVector vol25DHVec(termSize, 0.0);
	DoubleVector vol10DHVec(termSize, 0.0);
	
	LAStringVector::const_iterator itconst = baseStrikes.begin();
	unsigned int volpos = 0;
	//ATM
	itconst = std::find(baseStrikes.begin(),baseStrikes.end(), "ATM");
	if (itconst == baseStrikes.end())
		throw LACoreInvalidData("VannaVolga BaseStrikes Error",__FILE__,__LINE__);
	else
	{
		volpos = static_cast<unsigned int>(itconst- baseStrikes.begin());
		volATMVec = volMatrix[volpos];
	}

	//25DLOW
	itconst = std::find(baseStrikes.begin(),baseStrikes.end(), "25DLOW");
	if (itconst !=  baseStrikes.end())
	{
		volpos = static_cast<unsigned int>(itconst- baseStrikes.begin());
		vol25DLVec = volMatrix[volpos];
	}
	
	//25DHIGH
	itconst = std::find(baseStrikes.begin(),baseStrikes.end(), "25DHIGH");
	if (itconst !=  baseStrikes.end())
	{
		volpos = static_cast<unsigned int>(itconst- baseStrikes.begin());
		vol25DHVec = volMatrix[volpos];
	}

	//10DLOW
	itconst = std::find(baseStrikes.begin(),baseStrikes.end(), "10DLOW");
	if (itconst !=  baseStrikes.end())
	{
		volpos = static_cast<unsigned int>(itconst- baseStrikes.begin());
		vol10DLVec = volMatrix[volpos];
	}

	//10DHIGH
	itconst = std::find(baseStrikes.begin(),baseStrikes.end(), "10DHIGH");
	if (itconst !=  baseStrikes.end())
	{
		volpos = static_cast<unsigned int>(itconst- baseStrikes.begin());
		vol10DHVec = volMatrix[volpos];
	}

	double basevol = param.extraBaseVolParam;
	for( unsigned int k = 0;k < vol10DLVec.size();k++)
	{
		vol10DLVec[k] +=basevol;
		vol25DLVec[k] +=basevol;
		volATMVec[k] +=basevol;
		vol25DHVec[k] +=basevol;
		vol10DHVec[k] +=basevol;
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
	
	//// stike fwd
	
	unsigned int gridSize = optionMatVec.size();
	mFxParams.resize(gridSize);
	mSmileData.resize(gridSize);
	
	//set FXCurrency
	mFXCurrency = param.ccy;

	//vanna volga
	LAString matumethod = dynamic_cast<const LADataString &>(calibInfo.getData("MaturityMethod", ISNOTNULL).get()).get();
	LAMathFXVolatilitySurfaceGenerate::SetATMInterpolationMethod( matumethod, mAtmMethod );

	//same as fxstrglslv 
	unsigned int vecSize = optionMatVec.size();
	const DateVector &optionMatDates = dynamic_cast<const LADataDates &>(calibInfo.getData("MaturityDates", ISNOTNULL).get()).get();
	const DateVector &optionDelDates = dynamic_cast<const LADataDates &>(calibInfo.getData("DeliveryDates", ISNOTNULL).get()).get();

	const LAStringVector& deltaTypes = dynamic_cast<const LADataStrings &>(calibInfo.getData("DeltaTypes", ISNOTNULL).get()).get();
	const LAStringVector& atmTypes = dynamic_cast<const LADataStrings &>(calibInfo.getData("ATMTypes", ISNOTNULL).get()).get();
	
	mFxParams.resize(vecSize);
	mMatuTerms365.resize(vecSize);
	for (unsigned int i = 0; i < vecSize; i++)
	{
		
		//LAString cal;
		LAPriceDataCalendar cal;
		//correnct
		
		//test
		mFxParams[i] = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*mpDataInstance,mdYieldDataName,mfYieldDataName,optionMatDates[i],
			optionDelDates[i],mSpotRate,deltaTypes[i],atmTypes[i],cal);
		mMatuTerms365[i] = mFxParams[i].T;

		mSmileData[i].vols.resize(baseStrikes.size());
		mSmileData[i].strikes.resize(baseStrikes.size());

		for (unsigned int j = 0; j < baseStrikes.size(); j++)
		{
			LAString keystr = baseStrikes[j];
			if ("ATM" == keystr)
			{
				mSmileData[i].vols[j] = volATMVec[i];
				mSmileData[i].strikes[j] =  LAMathFXVolatilitySurfaceGenerate::GetATMStrike(volATMVec[i],mFxParams[i]);
			}
			else if ("25DHIGH" == keystr)
			{
				mSmileData[i].vols[j] = vol25DHVec[i];
				mSmileData[i].strikes[j] = LAMath::exp(LAMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(0.25, vol25DHVec[i], 1, mFxParams[i] ) ) * mFxParams[i].F;
			}
			else if ("10DHIGH" == keystr)
			{
				mSmileData[i].vols[j] = vol10DHVec[i];
				mSmileData[i].strikes[j] = LAMath::exp(LAMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(0.10, vol10DHVec[i], 1, mFxParams[i] ) ) * mFxParams[i].F;
			}
			else if ("25DLOW" == keystr)
			{
				mSmileData[i].vols[j] = vol25DLVec[i];
				mSmileData[i].strikes[j] = LAMath::exp(LAMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(-0.25, vol25DLVec[i], -1, mFxParams[i] ) ) * mFxParams[i].F;
			}
			else if ("10DLOW" == keystr)
			{
				mSmileData[i].vols[j] = vol10DLVec[i];
				mSmileData[i].strikes[j] = LAMath::exp(LAMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(-0.10, vol10DLVec[i], -1, mFxParams[i] ) ) * mFxParams[i].F;
			}
			else 
				throw LACoreInvalidData("VannaVola BaseStrikes Error",__FILE__,__LINE__);
		}

		//hishida vannavolga 
			//this is complicated case. In case of FORWARDATM, or SPOTATM, we must adjust atmvolatility
			//because vannavolgamethods always needs DeltaNeutral ATK strikes
		if (mFxParams[i].atmType == FORWARDATM || mFxParams[i].atmType == SPOTATM)
		{
			
			LAFindRootBrent brent;
			double atmvol = volATMVec[i];

			std::vector<pair<double, double> > Bound;
			Bound.resize(1);
			Bound[0].first  = /*0.05*/EPS_Vol1;
			Bound[0].second = LAMath::max(mSmileData[i].vols[0],mSmileData[i].vols[2]);

			MMATM atm(mFxParams[i], mSmileData[i]);
			DoubleVector volvec(1,atmvol);
			brent.findRoot(atm,Bound,volvec);

			double finalvol = volvec[0];

			mSmileData[i].vols[1] = finalvol;
			mSmileData[i].strikes[1] = (mFxParams[i].deltaType == FWD_PRE || mFxParams[i].deltaType == SPOT_PRE)
										? mFxParams[i].F * LAMath::exp(-0.5 * finalvol * finalvol * mFxParams[i].T)
										: mFxParams[i].F * LAMath::exp(0.5 * finalvol * finalvol * mFxParams[i].T);


			int stop = 1;
		
		}
	}

	
	
	// set method
	mpFunc = method;
	return;

}

// 
/*!
    @brief calibrate
*/
void
LACalibrateFXVannaVolga::doCalibrate()
{
		//calibration start
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateFXVannaVolga calibrate called.." << endl;
	
	if (!mpDataInstance)
	{
		throw LACoreInvalidData("DataInstance member is NULL", __FILE__, __LINE__);
	}

	LAMathVolFuncFXVannaVolga* method = new LAMathVolFuncFXVannaVolga(mpDataInstance,mAtmMethod,mFxParams,mSmileData,mMatuTerms365,
												mdYieldDataName,mfYieldDataName,mSpotRate);
	mpFunc->setRealFunction(*method);
	mpFunc->setOn();

	delete method;
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateFXVannaVolga calibrate end.." << endl;
}
