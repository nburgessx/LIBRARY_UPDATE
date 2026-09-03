/*! @file
    @brief IRSABR calibration request
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateIRSABR.cpp
//
//  DESCRIPTION :        IRSABR calibration request class
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
#include <limits>
#include "LACalibrateIRSABR.h"
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
#include "LAMathVolFuncIRSABR.h"
#include "LAMathDateUtilities.h"
#include "LAMathCurveFuncUtility.h"
#include "LAMathIRVanillaFuncUtility.h"
#include "LAMathValuableEntity.h"
#include "LACoreDataService.h"
#include "LAMathSwaptionVolUtility.h"
#include "LADataMatrix.h"
#include <sstream>
#include "LABasic.h"
#include "LAPriceCashFlowGenerator.h"
#include "LADefinitionsIRSABR.h"
#include "LAMathJamshidianSwaption.h"
#include "LADealUtils.h"

using namespace std;


// constructor
/*!

*/
LACalibrateIRSABR::LACalibrateIRSABR()
:LACalibrate()
{
}

// destructor
/*!

*/
LACalibrateIRSABR::~LACalibrateIRSABR()
{
}


// 
/*!
    @brief setup
*/
void
LACalibrateIRSABR::setUp(LAObjectPool &objPool, const MAScenarioParam &param, MACalibrationFunc *method, int gridPos)
{
	
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateIRSABR set up called" << endl;
	if (!method)
	{
		throw LACoreInvalidData("Argument method is NULL", __FILE__, __LINE__);
	}
	
	// set method
	mpFunc = method;

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
	
	const LAString calibInfoName = param.refName[0];
	LAStringVector calibInfoNames = calibInfoName.toToken(':');
	
	for (unsigned int i_under = 0; i_under < calibInfoNames.size(); ++i_under)
	{
		const LADataHolder* dh;

		LAObject &calibInfo = objPool.getObject(calibInfoNames[i_under], ENCHKTYPE_ISDEFINED).get();
		mpDataInstance = calibInfo.getDataInstance();

		LAString underlying = dynamic_cast<const LADataString &>(calibInfo.getData(PRICING_DATA_UNDERLYING,ISNOTNULL).get()).get();
		mUnderlyings.push_back(underlying);

		LAString ccy = dynamic_cast<const LADataString &>(calibInfo.getData(PRICING_DATA_CURRENCY,ISNOTNULL).get()).get();
		ccy.toLower();

		mcurvesetid[underlying] = calibInfoNames[i_under] + LAString("_") + PRICING_DATA_CURVESETID;
		
		//set target
		dh = &(calibInfo.getData(PRICING_DATA_FITTINGTARGET,ISNOTNULL));
		mtarget[underlying] = dynamic_cast<const LADataString &>(dh->get()).get();
		//set swapvol id
		dh = &(calibInfo.getData(PRICING_DATA_SWAPTIONDATA,NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			mswapvolid[underlying] = dynamic_cast<const LADataStrings &>(dh->get()).get();
		//set weight
		dh = &(calibInfo.getData(PRICING_DATA_WEIGHT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const LAStringVector &strWeight = dynamic_cast<const LADataStrings &>(dh->get()).get();
			//mweight.resize(strWeight.size());
			for (unsigned int i  = 0; i < strWeight.size(); ++i)
			{
				mweight[underlying].push_back(strWeight[i].getDoubleValue());
			}
		}
		
		//set string matrix
		
		LAStringMatrix sabrlimmat(4,LAStringVector(2,""));
		sabrlimmat.resize(4,LAStringVector(2,""));
		mcalibflag[underlying].resize(4, false);
		
		dh = &(calibInfo.getData(PRICING_DATA_ISCALIBRATEALPHA, ISNOTNULL));
		bool iscalibrate = dynamic_cast<const LADataBool &>(dh->get()).get();
		sabrlimmat[0][0] = "ALPHA";
		sabrlimmat[0][1] = (iscalibrate) ? "YES" : "NO";
		mcalibflag[underlying][0] =  (iscalibrate) ? true : false;

		dh = &(calibInfo.getData(PRICING_DATA_ISCALIBRATEBETA, ISNOTNULL));
		iscalibrate = dynamic_cast<const LADataBool &>(dh->get()).get();
		sabrlimmat[1][0] = "BETA";
		sabrlimmat[1][1] =  (iscalibrate) ? "YES" : "NO";
		mcalibflag[underlying][1] =  (iscalibrate) ? true : false;
		
		dh = &(calibInfo.getData(PRICING_DATA_ISCALIBRATENU, ISNOTNULL));
		iscalibrate = dynamic_cast<const LADataBool &>(dh->get()).get();
		sabrlimmat[2][0] = "NU";
		sabrlimmat[2][1] =  (iscalibrate) ? "YES" : "NO";
		mcalibflag[underlying][2] =  (iscalibrate) ? true : false;
		
		dh = &(calibInfo.getData(PRICING_DATA_ISCALIBRATERHO, ISNOTNULL));
		iscalibrate = dynamic_cast<const LADataBool &>(dh->get()).get();
		sabrlimmat[3][0] = "RHO";
		sabrlimmat[3][1] =  (iscalibrate) ? "YES" : "NO";
		mcalibflag[underlying][3] =  (iscalibrate) ? true : false;

		msabrLimiter[underlying] = sabrlimmat;
		

		// asofdate
		const LADate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		//get swaption conv id
		LAObject& swapconv = dynamic_cast<LADataReference &>(calibInfo.getData(PRICING_DATA_SWAPTIONCONVID, ISNOTNULL).get()).get().get();
		///////////////////////////////getyielddatanameandset1mcurve3mcurve6mcurveandcurveid
		LAString tmpcurveidname = curveIDName;
		//set swaption convid
		mswapconvid[underlying] = dynamic_cast<const LADataString &>(swapconv.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

		//set cap convid
		LAObject& capconv = dynamic_cast<LADataReference &>(calibInfo.getData(PRICING_DATA_CAPCONVID, ISNOTNULL).get()).get().get();
		mcapconvid[underlying] = dynamic_cast<const LADataString &>(capconv.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

		//get convcal
		dh = &(swapconv.getData(PRICING_DATA_FIXINGCALENDAR, ISNOTNULL));
		const LAPriceDataCalendar& cal = dynamic_cast<const LAPriceDataCalendar &>(dh->get());
		//get convslidingrurl
		dh = &(swapconv.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
		const LAPriceDataSlidingRule& sr = dynamic_cast<const LAPriceDataSlidingRule &>(dh->get());
		//set daycount
		LAPriceDataDayCount dc_act;
		dc_act.setDayCount(ACT_365_ISDA);

		//expiry string and expiry vector
		dh = &(calibInfo.getData(PRICING_DATA_EXPIRYSTRING, ISNOTNULL));
		LAStringVector strexpiryvec = dynamic_cast<const LADataStrings &>(dh->get()).get();
		DoubleVector expiryvec(strexpiryvec.size());
		DateVector expirydatevec(strexpiryvec.size());
		
		for (unsigned int i = 0; i < expiryvec.size(); i++)
		{
			LADate toDate = LAMathDateCalculations::getDate(asofDate,strexpiryvec[i],sr,&cal,true);
			expirydatevec[i] = toDate;
			expiryvec[i] = dc_act.getTerm(asofDate,toDate,true);
		}
		// tenor string and tenor vector
		dh = &(calibInfo.getData(PRICING_DATA_TENORSTRING, ISNOTNULL));
		LAStringVector strtenorvec = dynamic_cast<const LADataStrings &>(dh->get()).get();
		DoubleVector tenorvec(strtenorvec.size());
		for (unsigned int i = 0; i < tenorvec.size(); i++)
		{
			int y,m,d,w;
			LAMathDateCalculations::termStrtoYMDW(strtenorvec[i], y, m, d, w);
			tenorvec[i] = static_cast<double > (y) + static_cast<double > (m) / 12;
		}

		// curve names
		dh = &(calibInfo.getData(PRICING_DATA_CURVETYPEKEYS, ISNOTNULL));
		const LAStringVector& curvetypekeys = dynamic_cast<const LADataStrings &>(dh->get()).get();

		dh = &(calibInfo.getData(PRICING_DATA_CURVETYPES, ISNOTNULL));
		const LAStringVector& curvetypes = dynamic_cast<const LADataStrings &>(dh->get()).get();

		unsigned int curvematsize = 1 + curvetypekeys.size();
		
		LAStringMatrix sizemat(curvematsize,LAStringVector(2));
		mcurvesetmat[underlying] = sizemat;
		mcurvesetmat[underlying][0][0] = "CurveID";
		mcurvesetmat[underlying][0][1] = curveIDName;
		for (unsigned int i = 1; i < curvematsize; i++)
		{
			mcurvesetmat[underlying][i][0] = curvetypekeys[i-1];
			mcurvesetmat[underlying][i][1] = curvetypes[i-1];
		}
		LAMathSwaptionVolUtility::setCurveID2(mpDataInstance,mcurvesetid[underlying],strtenorvec,mcurvesetmat[underlying]);
		// approximation method
		dh = &(calibInfo.getData(PRICING_DATA_APPROXMETHOD, ISNOTNULL));
		mapproxmethod[underlying] = dynamic_cast<const LADataString &>(dh->get()).get();
		// calibration method
		dh = &(calibInfo.getData(PRICING_DATA_CALIBMETHOD, ISNOTNULL));
		mcalibmethod[underlying] = dynamic_cast<const LADataString &>(dh->get()).get();
		// option type
		dh = &(calibInfo.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
		const LAStringVector &optypeVec = dynamic_cast<const LADataStrings &>(dh->get()).get();
		moptionsign[underlying].resize(optypeVec.size());
		for (unsigned int j = 0; j < optypeVec.size(); ++j)
		{
			int flg = 0;
			if (optypeVec[j] == CALL)
			{
				flg = 1;
			}
			else if (optypeVec[j] == PUT)
			{
				flg = -1;
			}
			else
			{
				LAString msg = "Invalid option type : " + optypeVec[j];
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			moptionsign[underlying][j] = flg;
		}
		//ishaganatmvolfixed
		mishaganatmvolfixed[underlying] = true;
		dh =  &(calibInfo.getData(PRICING_DATA_ISHAGANATMVOLFIXED, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			mishaganatmvolfixed[underlying] = dynamic_cast<const LADataBool &>(dh->get()).get();
		}
		//forwardslidevalue
		mforwardshiftvalue[underlying] = 0.;
		dh =  &(calibInfo.getData(PRICING_DATA_FORWARDSHIFTVALUE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			mforwardshiftvalue[underlying] = dynamic_cast<const LADataDouble &>(dh->get()).get();
		}
		unsigned int forSize = 5;
		bool is_alpha_entity_exist = false, is_beta_entity_exist = false, is_nu_entity_exist = false, is_rho_entity_exist = false;
		for (unsigned int i = 0; i < forSize; i++)
		{
			LAString addstr;
			if (0 == i )
			{
				addstr =  LAString("_") + PRICING_DATA_VOLID;
			}
			else if (1 == i )
			{
				addstr =  LAString("_") + PRICING_DATA_ALPHAID;
			}
			else if (2 == i )
			{
				addstr =  LAString("_") + PRICING_DATA_BETAID;
			}
			else if (3 == i )
			{
				addstr =   LAString("_") + PRICING_DATA_RHOID;
			}
			else if (4 == i )
			{
				addstr =  LAString("_") + PRICING_CALIBRATION_DATAUID;
			}

			const LADataHolder* dh = &(calibInfo.getData("InitialIRSABR" + addstr, ISNOTNULL));
			const LADataReference& ref = dynamic_cast<const LADataReference& >(dh->get());
			const LAObject& einit = ref.get().get();

			LAString name = dynamic_cast<const LADataString &>(einit.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
			if (gridPos >= 0 && !param.gridTerm.empty())
			{
				name +=  param.calcType + "_" + curveIDName + "_" + param.gridTerm[gridPos];
			}
			else
			{
				name +=  param.calcType + "_" + curveIDName + "_" + LAString(gridPos);
			}
			bool is_param_entity_exist = false;
			LAObject* newide = NULL;
			LAObjectHolder objHolder = objPool.getObject( name, ENCHKTYPE_NOCHECK );
			if (!objHolder.isDefined())
			{
				newide = einit.clone();
				objPool.set(name,newide);
				newide->getData(CALIBRATION_DATA_NAME,ISNOTNULL).convertFromString(name);
			}
			else
			{
				newide = &objHolder.get();
				is_param_entity_exist = true;
			}
			//expirystring
			newide->remove(PRICING_DATA_EXPIRYSTRING);
			newide->add(PRICING_DATA_EXPIRYSTRING, new LADataStrings(strexpiryvec));
			//expiryvec
			newide->remove(PRICING_DATA_EXPIRYVECTOR);
			newide->add(PRICING_DATA_EXPIRYVECTOR, new LADataDoubles(expiryvec));
			//tenorstring
			newide->remove(PRICING_DATA_TENORSTRING);
			newide->add(PRICING_DATA_TENORSTRING, new LADataStrings(strtenorvec));
			//tenorvec
			newide->remove(PRICING_DATA_TENORVECTOR);
			newide->add(PRICING_DATA_TENORVECTOR, new LADataDoubles(tenorvec));
			//expirydatevec
			newide->remove(PRICING_DATA_EXPIRYDATEVECTOR);
			newide->add(PRICING_DATA_EXPIRYDATEVECTOR, new LADataDates(expirydatevec));
			//gridapproxmethod
			newide->remove(PRICING_DATA_GRIDAPPROXMETHOD);
			newide->add(PRICING_DATA_GRIDAPPROXMETHOD, calibInfo.getData(PRICING_DATA_GRIDAPPROXMETHOD, ISNOTNULL).get().clone());	
			
			if (1 == i)
			{
				is_alpha_entity_exist = is_param_entity_exist;
				malphaid[underlying] = name;
			}
			else if (2 == i)
			{
				is_beta_entity_exist = is_param_entity_exist;
				mbetaid[underlying] = name;
			}
			else if (3 == i)
			{
				is_rho_entity_exist = is_param_entity_exist;
				mrhoid[underlying]  = name;
			}
			else if (4 == i)
			{
				is_nu_entity_exist = is_param_entity_exist;
				mnuid[underlying] = name;
			}

			if (is_alpha_entity_exist && is_beta_entity_exist && is_rho_entity_exist && is_nu_entity_exist)
			{
				mIsCalibSkip[underlying] = true;
			}
			else
			{
				mIsCalibSkip[underlying] = false;
			}
		}

		//set volatility again
		dh = &(calibInfo.getData(LAString("InitialIRSABR") + LAString("_") + PRICING_DATA_VOLID, ISNOTNULL));
		const LADataReference& ref = dynamic_cast<const LADataReference& >(dh->get());
		LAString volname = dynamic_cast<const LADataString &>(ref.get().getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
		LAString volnameSuffix;
		if (gridPos >= 0 && !param.gridTerm.empty())
		{
			volnameSuffix =  param.calcType + "_" + curveIDName + "_" + param.gridTerm[gridPos];
		}
		else
		{
			volnameSuffix =  param.calcType + "_" + curveIDName + "_" + LAString(gridPos);
		}
		volname += volnameSuffix;
		
		LAObject& vole = objPool.getObject(volname, ENCHKTYPE_ISDEFINED).get();

		matmvolid[underlying] = volname;
		mswapvolid[underlying].clear();
		mswapvolid[underlying].resize(1, volname);
		//set calibration target flag
		LAString calibTarget_str = LACoreDataService::getContext(ccy + "." CONTEXT_KEY_DEAL_IRVOL + "." + underlying);
		mcalibflgmat[underlying].clear();
		if (calibTarget_str != AQ_NO_DATA)
		{
			LADataBoolMatrix tmp;
			tmp.convertFromString(calibTarget_str);
			BoolMatrix calibTarget = tmp.get();
			vole.remove(PRICING_DATA_CALIBRATIONFLAG);
			vole.add(PRICING_DATA_CALIBRATIONFLAG, new LADataBoolMatrix(calibTarget));
			mcalibflgmat[underlying] = calibTarget;
		}
		if (mishaganatmvolfixed[underlying] && (param.targetCurveType != underlying)) continue;

		LAString scenarioTargetID = PRICING_DATA_VOLID;
		double scenarioTargetMax = std::numeric_limits<double>::max();
		double scenarioTargetMin = 0.0;
		if (!param.vegaType.isDefined() || param.vegaType == RISK_VEGATYPE_ATM) {
			scenarioTargetID = PRICING_DATA_VOLID;
			scenarioTargetMax = std::numeric_limits<double>::max();
			scenarioTargetMin = 0.0;
		}
		else if(param.vegaType == RISK_VEGATYPE_BETA){
				scenarioTargetID = PRICING_DATA_BETAID;
				scenarioTargetMax = 1.0;
				scenarioTargetMin = 0.0;
		}
		else if(param.vegaType == RISK_VEGATYPE_NU){
			scenarioTargetID = PRICING_CALIBRATION_DATAUID;
			scenarioTargetMax = std::numeric_limits<double>::max();
			scenarioTargetMin = 0.0;
		}
		else if(param.vegaType == RISK_VEGATYPE_RHO){
			scenarioTargetID = PRICING_DATA_RHOID;
			scenarioTargetMax = 1.0;
			scenarioTargetMin = -1.0;
		}

		dh = &(calibInfo.getData(LAString("InitialIRSABR") + LAString("_") + scenarioTargetID, ISNOTNULL));
		const LADataReference& refScenarioTarget = dynamic_cast<const LADataReference& >(dh->get());
		LAString scenarioTargetName = dynamic_cast<const LADataString &>(refScenarioTarget.get().getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
		scenarioTargetName += volnameSuffix;
		LAObject& scenarioTargetEntity = objPool.getObject(scenarioTargetName, ENCHKTYPE_ISDEFINED).get();
		LADataHolder* ahScenarioTarget = &(scenarioTargetEntity.getData(PRICING_DATA_SWAPTIONMATRIX, ISNOTNULL));
		DoubleMatrix& scenarioTargetMat = dynamic_cast<LADataDoubleMatrix &>(ahScenarioTarget->get()).get();
		
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
					for (unsigned int i = 0; i < scenarioTargetMat.size();i++)
					{
						DoubleVector shiftVal(scenarioTargetMat[i].size(), param.paraShiftVec[0]);
						if (shiftType == RISK_SHIFTTYPE_DIFF)
						{
							transform(scenarioTargetMat[i].begin(), scenarioTargetMat[i].end(), shiftVal.begin(), scenarioTargetMat[i].begin(), plus<double>());
						}
						else
						{
							// ratio case
							DoubleArray plusVal(scenarioTargetMat[i].size());
							transform(scenarioTargetMat[i].begin(), scenarioTargetMat[i].end(), shiftVal.begin(), plusVal.begin(), multiplies<double>());
							transform(scenarioTargetMat[i].begin(), scenarioTargetMat[i].end(), plusVal.begin(), scenarioTargetMat[i].begin(), plus<double>());
							
						}
						unsigned int size = scenarioTargetMat[i].size();
						for (unsigned int j = 0; j < size; j++)
						{
							scenarioTargetMat[i][j] = LAMath::max(scenarioTargetMat[i][j], scenarioTargetMin);
							scenarioTargetMat[i][j] = LAMath::min(scenarioTargetMat[i][j], scenarioTargetMax);
						}
					}
				}
				else
				{
					throw LACoreInvalidData("Not supported now in SABR model",__FILE__,__LINE__); 
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
				const int gridStart = param.isWave ? 0 : gridPos;

				for (int iGrid = gridStart; iGrid <= gridPos; ++iGrid) {

					// check
					LAString term = param.gridTerm[iGrid];
					term.toUpper();
					//int pos = term.findString(CALIB_MARKET_SWAPTION);
					//if (pos != 0)
					if (term.findString(CALIB_MARKET_SWAPTION) == -1)
					{
						throw LACoreInvalidData("IRSABR Market data swaption only.", __FILE__, __LINE__);
					}
					//unsigned int gStartPos = 9;
					LAStringVector tmpGridTerm = term.toToken('_');
					//LAString grid = term.subString(gStartPos, term.size() - 1);
					LAString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];

					// create mat and swapterm index vex
					for (unsigned int i = 0; i < strexpiryvec.size(); i++)
					{
						unsigned int gridSize = strtenorvec.size();
						LAStringVector tmpGridVec(gridSize);
						
						for (unsigned int j = 0; j < gridSize; j++)
						{
							tmpGridVec[j] = strexpiryvec[i] + "_" + strtenorvec[j];
							tmpGridVec[j].toUpper();
						}
						// check term
						LAStringVector::const_iterator it = find(tmpGridVec.begin(), tmpGridVec.end(), grid);
						if (it == tmpGridVec.end())
						{
							continue;
							/*LAString msg = "Shift grid is not in swaption market. grid = " + grid;
							throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);*/
						}
						unsigned int index = static_cast<unsigned int>(it - tmpGridVec.begin());

						// shift val
						double shiftVal = param.gridShiftVec[iGrid];
						LAString shiftType = param.shiftType;
						if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
						{
							shiftVal = scenarioTargetMat[i][index] * shiftVal;
						}
						// add shift val
						scenarioTargetMat[i][index] = scenarioTargetMat[i][index] + shiftVal;
						scenarioTargetMat[i][index] = LAMath::max(scenarioTargetMat[i][index], scenarioTargetMin);
						scenarioTargetMat[i][index] = LAMath::min(scenarioTargetMat[i][index], scenarioTargetMax);
						break;
					}
				}
			}
		}
		if (!mishaganatmvolfixed[underlying])
		{
			// calc ATM strike(forward rate)
			const LAObject &curveEntity = objPool.getObject(mcurvesetid[underlying], ENCHKTYPE_ISDEFINED ).get();
			const LAString &curveID = dynamic_cast<const LADataString& >(curveEntity.getData(PRICING_DATA_CURVEID, ISDEFINED).get()).get();
			const LAString &dfName = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME, ISDEFINED).get()).get();
			DoubleMatrix strikemat(expirydatevec.size());
			if (!mcalibflgmat[underlying].empty() && mcalibflgmat[underlying].size() != strikemat.size())
			{
				throw LACoreInvalidData("Calib flag format is wrong!", __FILE__, __LINE__);
			}
			for (unsigned int j = 0; j < expirydatevec.size(); ++j)
			{
				strikemat[j].resize(strtenorvec.size());
				if (!mcalibflgmat[underlying].empty() && mcalibflgmat[underlying][j].size() != strikemat[j].size())
				{
					throw LACoreInvalidData("Calib flag format is wrong!", __FILE__, __LINE__);
				}
				for (unsigned int k = 0; k < strtenorvec.size(); ++k)
				{
					if (!mcalibflgmat[underlying].empty() && !mcalibflgmat[underlying][j][k])
					{
						continue;
					}
					const LAString *forName = 0;
					const LAString *convID = 0;
					if (strtenorvec[k] == "1M")
					{
						forName =  &(dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_1MLCURVENAME, ISDEFINED).get()).get());
						convID = &mcapconvid[underlying];
					}
					else if (strtenorvec[k] == "3M")
					{
						forName =  &(dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_3MLCURVENAME, ISDEFINED).get()).get());
						convID = &mcapconvid[underlying];
					}
					else if  (strtenorvec[k] == "6M")
					{
						forName =  &(dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_6MLCURVENAME, ISDEFINED).get()).get());
						convID = &mcapconvid[underlying];
					}
					else
					{
						forName =  &(dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_SWAPRATELCURVENAME, ISDEFINED).get()).get());
						convID = &mswapconvid[underlying];
					}
					strikemat[j][k] = LAMath::max(LAMathSwaptionVolUtility::getForward(mpDataInstance, expirydatevec[j], strtenorvec[k], curveID, *convID, *forName, dfName), eps_SABR);
				}
			}
			vole.remove(PRICING_DATA_SWAPTIONSTRIKEMATRIX);
			vole.add(PRICING_DATA_SWAPTIONSTRIKEMATRIX, new LADataDoubleMatrix(strikemat));
			// if target is premium setup premium instead volatility
			LAString tmp_target = mtarget[underlying];
			tmp_target.toUpper();
			if (tmp_target ==  CALIB_TARGET_PREMIUM)
			{
				DoubleMatrix premiummat;
				dh = &(vole.getData(PRICING_DATA_PREMIUMMATRIX, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					premiummat = dynamic_cast<const LADataDoubleMatrix &>(dh->get()).get();
				}
				else
				{
					LAString optBuy(CALIB_OPT_BUY);
					LAString optionType(PAYERS);
					if (moptionsign[underlying][0] == -1)
					{
						optionType = RECEIVERS;
					}
					premiummat.resize(expirydatevec.size());
					const double forwardShift = mforwardshiftvalue[underlying];

					LADataHolder* volah = &(vole.getData(PRICING_DATA_SWAPTIONMATRIX, ISNOTNULL));
					DoubleMatrix& volmat = dynamic_cast<LADataDoubleMatrix &>(volah->get()).get();

					for (unsigned int j= 0; j < expirydatevec.size(); ++j)
					{
						premiummat[j].resize(strtenorvec.size());
						for (unsigned int k = 0; k < strtenorvec.size(); ++k)
						{
							if (!mcalibflgmat[underlying].empty() && !mcalibflgmat[underlying][j][k])
							{
								continue;
							}
							const LAString *forName = 0;
							const LAString *convID = 0;
							LAString freq = "";
							if (strtenorvec[k] == "1M")
							{
								forName =  &(dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_1MLCURVENAME, ISDEFINED).get()).get());
								convID = &mcapconvid[underlying];
								freq = FREQ_MONTHLY;
							}
							else if (strtenorvec[k] == "3M")
							{
								forName =  &(dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_3MLCURVENAME, ISDEFINED).get()).get());
								convID = &mcapconvid[underlying];
								freq = FREQ_QUARTERLY;
							}
							else if  (strtenorvec[k] == "6M")
							{
								forName =  &(dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_6MLCURVENAME, ISDEFINED).get()).get());
								convID = &mcapconvid[underlying];
								freq = FREQ_SEMI_ANNUAL;
							}
							else
							{
								forName =  &(dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_SWAPRATELCURVENAME, ISDEFINED).get()).get());
								convID = &mswapconvid[underlying];
								//freq
								const LAObject& convEntity = objPool.getObject(*convID, ENCHKTYPE_ISDEFINED).get();
								dh = &convEntity.getData(IR_CALIBRATION_DATA_FREQUENCY + LAString("_") + strtenorvec[k], NOCHECK);
								if(!dh->isDefined() || dh->isNull())
								{
									dh = &convEntity.getData(IR_CALIBRATION_DATA_FREQUENCY, ISDEFINED);
								}
								freq = dh->convertToString().exchange("\"","");

							}
							const LAObject& convEntity = objPool.getObject(*convID, ENCHKTYPE_ISDEFINED).get();
							// spotLag
							LAString spotLag = convEntity.getData(CURVEINPUT_SPOTLAG, ISDEFINED).convertToString().exchange("\"","");
							// daycount
							LAString daycount = convEntity.getData(IR_CALIBRATION_DATA_DAYCOUNT, ISDEFINED).convertToString().exchange("\"","");
							// sliding rule
							LAString sldrule = convEntity.getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).convertToString().exchange("\"","");
							// payment calendar
							LAString payCal = convEntity.getData(CALIBRATION_DATA_CALENDAR, ISDEFINED).convertToString().exchange("\"","");
							// fixing  calendar
							LAString fixCal = convEntity.getData(PRICING_DATA_FIXINGCALENDAR, ISDEFINED).convertToString().exchange("\"","");

							premiummat[j][k] = LAMathIRVanillaFuncUtility::swaption(mpDataInstance, curveID, optBuy, 
																				optionType, 1.0, strikemat[j][k], volmat[j][k],
																				asofDate, expirydatevec[j], asofDate, spotLag, strtenorvec[k],
																				freq, sldrule, daycount, payCal, fixCal, -10.0, forwardShift, *forName, dfName);
						}
					}
				}
				vole.remove(PRICING_DATA_SWAPTIONMATRIX);
				vole.add(PRICING_DATA_SWAPTIONMATRIX, new LADataDoubleMatrix(premiummat));
			}
		}
	}

	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateIRSABR set up end" << endl;
}

// 
/*!
    @brief calibrate
*/
void
LACalibrateIRSABR::doCalibrate()
{

	//calibration start
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateIRSABR calibrate called.." << endl;
	
	if (!mpDataInstance)
	{
		throw LACoreInvalidData("DataInstance member is NULL", __FILE__, __LINE__);
	}

	LAObjectPool &objPool = mpDataInstance->getObjectPool();

	LAMathVolFuncIRSABR* method = new LAMathVolFuncIRSABR(mpDataInstance);

	for (unsigned int i_under = 0; i_under < mUnderlyings.size(); ++i_under)
	{
		method->setUnderlying(mUnderlyings[i_under]);
		method->setParamID(malphaid[mUnderlyings[i_under]], SABR_ALPHA);
		method->setParamID(mbetaid[mUnderlyings[i_under]], SABR_BETA);
		method->setParamID(mnuid[mUnderlyings[i_under]], SABR_NU);
		method->setParamID(mrhoid[mUnderlyings[i_under]], SABR_RHO);
		method->setSwapConvID(mswapconvid[mUnderlyings[i_under]]);
		method->setCapConvID(mcapconvid[mUnderlyings[i_under]]);
		if (!mishaganatmvolfixed[mUnderlyings[i_under]])
		{
			method->setApproxmethod(mapproxmethod[mUnderlyings[i_under]]);
		}
		else
		{
			method->setApproxmethod(APPROXIMATION_HAGAN);
		}
		method->setForwardShiftValue(mforwardshiftvalue[mUnderlyings[i_under]]);

		if (!mIsCalibSkip[mUnderlyings[i_under]])
		{
			if (!mishaganatmvolfixed[mUnderlyings[i_under]])
			{
				const BoolMatrix *pCalibMat = 0;
				if (!mcalibflgmat[mUnderlyings[i_under]].empty())
				{
					pCalibMat = &mcalibflgmat[mUnderlyings[i_under]];
				}
				LAString msg;
				LAMathSwaptionVolUtility::calibrateSABRMatrix(mpDataInstance,
															mapproxmethod[mUnderlyings[i_under]],
															mcalibflag[mUnderlyings[i_under]],
															mcalibmethod[mUnderlyings[i_under]],
															mcurvesetid[mUnderlyings[i_under]],
															malphaid[mUnderlyings[i_under]],
															mbetaid[mUnderlyings[i_under]],
															mnuid[mUnderlyings[i_under]],
															mrhoid[mUnderlyings[i_under]],
															mswapconvid[mUnderlyings[i_under]],
															mcapconvid[mUnderlyings[i_under]],
															mswapvolid[mUnderlyings[i_under]],
															mtarget[mUnderlyings[i_under]],
															mweight[mUnderlyings[i_under]],
															moptionsign[mUnderlyings[i_under]],
															"",
															mforwardshiftvalue[mUnderlyings[i_under]],
															"",
															msg,
															pCalibMat);	

			}
			else
			{
				mswapvolid[mUnderlyings[i_under]].clear();
				mweight[mUnderlyings[i_under]].clear();
				LAMathSwaptionVolUtility::calibrateSABRATMFix(mpDataInstance,
															mcurvesetid[mUnderlyings[i_under]],
															malphaid[mUnderlyings[i_under]],
															mbetaid[mUnderlyings[i_under]],
															mnuid[mUnderlyings[i_under]],
															mrhoid[mUnderlyings[i_under]],
															mswapconvid[mUnderlyings[i_under]],
															mcapconvid[mUnderlyings[i_under]],
															matmvolid[mUnderlyings[i_under]],
															mswapvolid[mUnderlyings[i_under]],
															msabrLimiter[mUnderlyings[i_under]],
															mtarget[mUnderlyings[i_under]],
															mweight[mUnderlyings[i_under]],
															"",
															mforwardshiftvalue[mUnderlyings[i_under]]);	
			}
		}

		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			LAStringVector sabrParamsID(4);
			sabrParamsID[0] = malphaid[mUnderlyings[i_under]]; 
			sabrParamsID[1] = mbetaid[mUnderlyings[i_under]]; 
			sabrParamsID[2] = mnuid[mUnderlyings[i_under]]; 
			sabrParamsID[3] = mrhoid[mUnderlyings[i_under]]; 

			for (size_t i=0; i<sabrParamsID.size(); i++)
			{
				const LAString fileSuffix = LACoreDataService::getContext(ARG_KEY_FILENUM);
				const LAString dirName = LACoreDataService::getOutputDirectory(); 
				LAString fileName = sabrParamsID[i].subString(26, sabrParamsID[i].size() - 1);
				fileName = dirName + fileName + fileSuffix + ".csv";
				LAObject& sabrParamEntity = objPool.getObject(sabrParamsID[i],ENCHKTYPE_ISDEFINED).get();

				ifstream fin;
				ofstream fout;
				fin.open(fileName.getCString());

				if (!fin)
				{
					DoubleMatrix& mat = dynamic_cast<LADataDoubleMatrix &> (sabrParamEntity.getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

					fout.open(fileName.getCString());

					for (size_t i = 0; i < mat.size(); ++i)
					{
						LAString output;
						for (size_t j = 0; j < mat[0].size(); ++j)
						{
							output += LAString(mat[i][j]);
							output += LAString(",");
						}
						output = output.subString(0, output.size() - 2);
						fout << output.getCString() << std::endl;
					}

					fout.close();
					fin.close();
				}
				else
				{
					DoubleMatrix mat;

					string line;
					while (getline(fin, line))
					{
						const char *c_line = line.c_str();
						LAStringVector lineVec = LAString(c_line).toToken(MARKET_DATA_DELIMITER);
						DoubleVector lineVec_d;
						for (size_t i=0; i<lineVec.size(); i++)
						{
							lineVec_d.push_back(lineVec[i].trimLeft().trimRight().getDoubleValue());
						}
						mat.push_back(lineVec_d);
					}
					fin.close();
					dynamic_cast<LADataDoubleMatrix &>(sabrParamEntity.getData(PRICING_DATA_SWAPTIONMATRIX, ISNOTNULL).get()).set(mat);
				}
			}
		}
	}

	mpFunc->setRealFunction(*method);
	mpFunc->setOn();

	delete method;
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateIRSABR calibrate end.." << endl;
}


