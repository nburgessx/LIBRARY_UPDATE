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
#include "AQLCalibrateFXVannaVolga.h"
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
#include "AQLPriceFXVolatility.h"
#include "AQLMathDateCalculations.h"
#include "AQLMathFXVanillaFuncUtility.h"
#include "AQLCoreDataService.h"
#include <sstream>
#include "AQLCalibratePool.h"
#include "AQLBasic.h"

#include "AQLCalibrationParametersFXVannaVolga.h"
#include "AQLDataMatrix.h"
#include "ConstantDeclarations.h"
#include "AQLMathVolFuncFXVannaVolga.h"
#include "AQLFindRootBrent.h"

using namespace std;
// constructor
/*!

*/
AQLCalibrateFXVannaVolga::AQLCalibrateFXVannaVolga()
:AQLCalibrate(), mSpotRate(0.0)
{
}

// destructor
/*!

*/
AQLCalibrateFXVannaVolga::~AQLCalibrateFXVannaVolga()
{
}


// 
/*!
    @brief setup
*/
void
AQLCalibrateFXVannaVolga::setUp(AQLObjectPool &objPool,  const AQLScenarioParam &param, AQLCalibrationFunc *method, int gridPos)
{
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateFXVannaVolga set up called" << endl;
	if (!method)
	{
		throw AQLCoreInvalidData("Argument method is NULL", __FILE__, __LINE__);
	}
	if (param.refName.size() < 2)
	{
		throw AQLCoreInvalidData("Param refName is more than be two, reference curve and calibinfo needed.", __FILE__, __LINE__);
	}
	const AQLString &calibInfoName = param.refName[0];
	const AQLObject &calibInfo = objPool.getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();

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
	const AQLString dYieldName = param.refName[curPos];
	if (!objPool.getObject(dYieldName).isDefined())
	{
		AQLString msg = dYieldName + " is not registered in EntityPool";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const AQLMathYieldCurve &dYield = dynamic_cast<const AQLMathYieldCurve &>(objPool.getObject(dYieldName, ENCHKTYPE_ISDEFINED).get());
	const AQLString &dYieldDataName = dYield.getYieldData().get().getName();
	mdYieldDataName =  dYield.getYieldData().get().getName();

	// get foreign curve
	const AQLString fYieldName = param.refName[curPos + 2];
	if (!objPool.getObject(fYieldName).isDefined())
	{
		AQLString msg = fYieldName + " is not registered in EntityPool";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const AQLMathYieldCurve &fYield = dynamic_cast<const AQLMathYieldCurve &>(objPool.getObject(fYieldName, ENCHKTYPE_ISDEFINED).get());
	const AQLString &fYieldDataName = fYield.getYieldData().get().getName();
	mfYieldDataName =  fYield.getYieldData().get().getName();

	
	// spot fx
	AQLMathFXEntity fx_tmp = *AQLMarketData::getFXEntity(objPool, "FORWARDRATE");
	fx_tmp.getFXType() = "FIXEDRATE";
	AQLStringVector ccys;
	AQLMarketData::convertToCurrency(param.ccy, ccys);
	// set spot rate
	mSpotRate = fx_tmp.getRate(ccys[1], ccys[0], 0.0);
	//add extra base param
	mSpotRate += param.extraBaseParam;
	// asofdate
	AQLDate asofDate(AQLCoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
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
	AQLDate spotDate = fx_tmp.getSpotDate(ccys[0], ccys[1], asofDate);
	mSpotRate *= dYield.getBasisDF(asofDate, spotDate) / fYield.getBasisDF(asofDate, spotDate);

	// optionmaturity
	const AQLStringVector &optionMatVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();

	DoubleMatrix volMatrix = dynamic_cast<const AQLDataDoubleMatrix &>(calibInfo.getData(PRICING_DATA_FXVOLATILITYMATRIX, ISNOTNULL).get()).get();

	const AQLStringVector &baseStrikes =  dynamic_cast<const AQLDataStrings &>(calibInfo.getData(PRICING_DATA_BASESTRIKES, ISNOTNULL).get()).get();

	unsigned int termSize = optionMatVec.size();
	DoubleVector vol10DLVec(termSize, 0.0);
	DoubleVector vol25DLVec(termSize, 0.0);
	DoubleVector volATMVec(termSize, 0.0);
	DoubleVector vol25DHVec(termSize, 0.0);
	DoubleVector vol10DHVec(termSize, 0.0);
	
	AQLStringVector::const_iterator itconst = baseStrikes.begin();
	unsigned int volpos = 0;
	//ATM
	itconst = std::find(baseStrikes.begin(),baseStrikes.end(), "ATM");
	if (itconst == baseStrikes.end())
		throw AQLCoreInvalidData("VannaVolga BaseStrikes Error",__FILE__,__LINE__);
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
	
	//// stike fwd
	
	unsigned int gridSize = optionMatVec.size();
	mFxParams.resize(gridSize);
	mSmileData.resize(gridSize);
	
	//set FXCurrency
	mFXCurrency = param.ccy;

	//vanna volga
	AQLString matumethod = dynamic_cast<const AQLDataString &>(calibInfo.getData("MaturityMethod", ISNOTNULL).get()).get();
	AQLMathFXVolatilitySurfaceGenerate::SetATMInterpolationMethod( matumethod, mAtmMethod );

	//same as fxstrglslv 
	unsigned int vecSize = optionMatVec.size();
	const DateVector &optionMatDates = dynamic_cast<const AQLDataDates &>(calibInfo.getData("MaturityDates", ISNOTNULL).get()).get();
	const DateVector &optionDelDates = dynamic_cast<const AQLDataDates &>(calibInfo.getData("DeliveryDates", ISNOTNULL).get()).get();

	const AQLStringVector& deltaTypes = dynamic_cast<const AQLDataStrings &>(calibInfo.getData("DeltaTypes", ISNOTNULL).get()).get();
	const AQLStringVector& atmTypes = dynamic_cast<const AQLDataStrings &>(calibInfo.getData("ATMTypes", ISNOTNULL).get()).get();
	
	mFxParams.resize(vecSize);
	mMatuTerms365.resize(vecSize);
	for (unsigned int i = 0; i < vecSize; i++)
	{
		
		//AQLString cal;
		AQLPriceDataCalendar cal;
		//correnct
		
		//test
		mFxParams[i] = AQLMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*mpDataInstance,mdYieldDataName,mfYieldDataName,optionMatDates[i],
			optionDelDates[i],mSpotRate,deltaTypes[i],atmTypes[i],cal);
		mMatuTerms365[i] = mFxParams[i].T;

		mSmileData[i].vols.resize(baseStrikes.size());
		mSmileData[i].strikes.resize(baseStrikes.size());

		for (unsigned int j = 0; j < baseStrikes.size(); j++)
		{
			AQLString keystr = baseStrikes[j];
			if ("ATM" == keystr)
			{
				mSmileData[i].vols[j] = volATMVec[i];
				mSmileData[i].strikes[j] =  AQLMathFXVolatilitySurfaceGenerate::GetATMStrike(volATMVec[i],mFxParams[i]);
			}
			else if ("25DHIGH" == keystr)
			{
				mSmileData[i].vols[j] = vol25DHVec[i];
				mSmileData[i].strikes[j] = AQLMath::exp(AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(0.25, vol25DHVec[i], 1, mFxParams[i] ) ) * mFxParams[i].F;
			}
			else if ("10DHIGH" == keystr)
			{
				mSmileData[i].vols[j] = vol10DHVec[i];
				mSmileData[i].strikes[j] = AQLMath::exp(AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(0.10, vol10DHVec[i], 1, mFxParams[i] ) ) * mFxParams[i].F;
			}
			else if ("25DLOW" == keystr)
			{
				mSmileData[i].vols[j] = vol25DLVec[i];
				mSmileData[i].strikes[j] = AQLMath::exp(AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(-0.25, vol25DLVec[i], -1, mFxParams[i] ) ) * mFxParams[i].F;
			}
			else if ("10DLOW" == keystr)
			{
				mSmileData[i].vols[j] = vol10DLVec[i];
				mSmileData[i].strikes[j] = AQLMath::exp(AQLMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(-0.10, vol10DLVec[i], -1, mFxParams[i] ) ) * mFxParams[i].F;
			}
			else 
				throw AQLCoreInvalidData("VannaVola BaseStrikes Error",__FILE__,__LINE__);
		}

		// vanna-volga
			//this is complicated case. In case of FORWARDATM, or SPOTATM, we must adjust atmvolatility
			//because vannavolgamethods always needs DeltaNeutral ATK strikes
		if (mFxParams[i].atmType == FORWARDATM || mFxParams[i].atmType == SPOTATM)
		{
			
			AQLFindRootBrent brent;
			double atmvol = volATMVec[i];

			std::vector<pair<double, double> > Bound;
			Bound.resize(1);
			Bound[0].first  = /*0.05*/EPS_Vol1;
			Bound[0].second = AQLMath::max(mSmileData[i].vols[0],mSmileData[i].vols[2]);

			AQLATM atm(mFxParams[i], mSmileData[i]);
			DoubleVector volvec(1,atmvol);
			brent.findRoot(atm,Bound,volvec);

			double finalvol = volvec[0];

			mSmileData[i].vols[1] = finalvol;
			mSmileData[i].strikes[1] = (mFxParams[i].deltaType == FWD_PRE || mFxParams[i].deltaType == SPOT_PRE)
										? mFxParams[i].F * AQLMath::exp(-0.5 * finalvol * finalvol * mFxParams[i].T)
										: mFxParams[i].F * AQLMath::exp(0.5 * finalvol * finalvol * mFxParams[i].T);


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
AQLCalibrateFXVannaVolga::doCalibrate()
{
		//calibration start
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateFXVannaVolga calibrate called.." << endl;
	
	if (!mpDataInstance)
	{
		throw AQLCoreInvalidData("DataInstance member is NULL", __FILE__, __LINE__);
	}

	AQLMathVolFuncFXVannaVolga* method = new AQLMathVolFuncFXVannaVolga(mpDataInstance,mAtmMethod,mFxParams,mSmileData,mMatuTerms365,
												mdYieldDataName,mfYieldDataName,mSpotRate);
	mpFunc->setRealFunction(*method);
	mpFunc->setOn();

	delete method;
	cout << static_cast<int>(AQLCoreThread::getThreadID()) << " AQLCalibrateFXVannaVolga calibrate end.." << endl;
}
