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
#include "AQLMathVolFuncIRSABR.h"
#include "AQLMathDateUtilities.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLMathIRVanillaFuncUtility.h"
#include "AQLMathValuableEntity.h"
#include "LACoreDataService.h"
#include "AQLMathSwaptionVolUtility.h"
#include "AQLDataMatrix.h"
#include <sstream>
#include "AQLBasic.h"
#include "AQLPriceCashFlowGenerator.h"
#include "LADefinitionsIRSABR.h"
#include "AQLMathJamshidianSwaption.h"
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
LACalibrateIRSABR::setUp(AQLObjectPool &objPool, const MAScenarioParam &param, MACalibrationFunc *method, int gridPos)
{
	
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateIRSABR set up called" << endl;
	if (!method)
	{
		throw AQLCoreInvalidData("Argument method is NULL", __FILE__, __LINE__);
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
	const AQLString curveName = param.refName[curPos];
	if (!objPool.getObject(curveName).isDefined())
	{
		AQLString msg = curveName + " is not registered in EntityPool";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
	const AQLString &curveIDName = dynamic_cast<const AQLMathYieldCurve &>(objPool.getObject(curveName, ENCHKTYPE_ISDEFINED).get()).getYieldData().get().getName();
	
	const AQLString calibInfoName = param.refName[0];
	AQLStringVector calibInfoNames = calibInfoName.toToken(':');
	
	for (unsigned int i_under = 0; i_under < calibInfoNames.size(); ++i_under)
	{
		const AQLDataHolder* dh;

		AQLObject &calibInfo = objPool.getObject(calibInfoNames[i_under], ENCHKTYPE_ISDEFINED).get();
		mpDataInstance = calibInfo.getDataInstance();

		AQLString underlying = dynamic_cast<const AQLDataString &>(calibInfo.getData(PRICING_DATA_UNDERLYING,ISNOTNULL).get()).get();
		mUnderlyings.push_back(underlying);

		AQLString ccy = dynamic_cast<const AQLDataString &>(calibInfo.getData(PRICING_DATA_CURRENCY,ISNOTNULL).get()).get();
		ccy.toLower();

		mcurvesetid[underlying] = calibInfoNames[i_under] + AQLString("_") + PRICING_DATA_CURVESETID;
		
		//set target
		dh = &(calibInfo.getData(PRICING_DATA_FITTINGTARGET,ISNOTNULL));
		mtarget[underlying] = dynamic_cast<const AQLDataString &>(dh->get()).get();
		//set swapvol id
		dh = &(calibInfo.getData(PRICING_DATA_SWAPTIONDATA,NOCHECK));
		if (dh->isDefined() && !dh->isNull())
			mswapvolid[underlying] = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
		//set weight
		dh = &(calibInfo.getData(PRICING_DATA_WEIGHT, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			const AQLStringVector &strWeight = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
			//mweight.resize(strWeight.size());
			for (unsigned int i  = 0; i < strWeight.size(); ++i)
			{
				mweight[underlying].push_back(strWeight[i].getDoubleValue());
			}
		}
		
		//set string matrix
		
		AQLStringMatrix sabrlimmat(4,AQLStringVector(2,""));
		sabrlimmat.resize(4,AQLStringVector(2,""));
		mcalibflag[underlying].resize(4, false);
		
		dh = &(calibInfo.getData(PRICING_DATA_ISCALIBRATEALPHA, ISNOTNULL));
		bool iscalibrate = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		sabrlimmat[0][0] = "ALPHA";
		sabrlimmat[0][1] = (iscalibrate) ? "YES" : "NO";
		mcalibflag[underlying][0] =  (iscalibrate) ? true : false;

		dh = &(calibInfo.getData(PRICING_DATA_ISCALIBRATEBETA, ISNOTNULL));
		iscalibrate = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		sabrlimmat[1][0] = "BETA";
		sabrlimmat[1][1] =  (iscalibrate) ? "YES" : "NO";
		mcalibflag[underlying][1] =  (iscalibrate) ? true : false;
		
		dh = &(calibInfo.getData(PRICING_DATA_ISCALIBRATENU, ISNOTNULL));
		iscalibrate = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		sabrlimmat[2][0] = "NU";
		sabrlimmat[2][1] =  (iscalibrate) ? "YES" : "NO";
		mcalibflag[underlying][2] =  (iscalibrate) ? true : false;
		
		dh = &(calibInfo.getData(PRICING_DATA_ISCALIBRATERHO, ISNOTNULL));
		iscalibrate = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		sabrlimmat[3][0] = "RHO";
		sabrlimmat[3][1] =  (iscalibrate) ? "YES" : "NO";
		mcalibflag[underlying][3] =  (iscalibrate) ? true : false;

		msabrLimiter[underlying] = sabrlimmat;
		

		// asofdate
		const AQLDate asofDate(LACoreDataService::getContext(CONTEXT_KEY_ASOFDATE).getCString());
		//get swaption conv id
		AQLObject& swapconv = dynamic_cast<AQLDataReference &>(calibInfo.getData(PRICING_DATA_SWAPTIONCONVID, ISNOTNULL).get()).get().get();
		///////////////////////////////getyielddatanameandset1mcurve3mcurve6mcurveandcurveid
		AQLString tmpcurveidname = curveIDName;
		//set swaption convid
		mswapconvid[underlying] = dynamic_cast<const AQLDataString &>(swapconv.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

		//set cap convid
		AQLObject& capconv = dynamic_cast<AQLDataReference &>(calibInfo.getData(PRICING_DATA_CAPCONVID, ISNOTNULL).get()).get().get();
		mcapconvid[underlying] = dynamic_cast<const AQLDataString &>(capconv.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

		//get convcal
		dh = &(swapconv.getData(PRICING_DATA_FIXINGCALENDAR, ISNOTNULL));
		const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar &>(dh->get());
		//get convslidingrurl
		dh = &(swapconv.getData(CALIBRATION_DATA_SLIDINGRULE, ISNOTNULL));
		const AQLPriceDataSlidingRule& sr = dynamic_cast<const AQLPriceDataSlidingRule &>(dh->get());
		//set daycount
		AQLPriceDataDayCount dc_act;
		dc_act.setDayCount(ACT_365_ISDA);

		//expiry string and expiry vector
		dh = &(calibInfo.getData(PRICING_DATA_EXPIRYSTRING, ISNOTNULL));
		AQLStringVector strexpiryvec = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
		DoubleVector expiryvec(strexpiryvec.size());
		DateVector expirydatevec(strexpiryvec.size());
		
		for (unsigned int i = 0; i < expiryvec.size(); i++)
		{
			AQLDate toDate = AQLMathDateCalculations::getDate(asofDate,strexpiryvec[i],sr,&cal,true);
			expirydatevec[i] = toDate;
			expiryvec[i] = dc_act.getTerm(asofDate,toDate,true);
		}
		// tenor string and tenor vector
		dh = &(calibInfo.getData(PRICING_DATA_TENORSTRING, ISNOTNULL));
		AQLStringVector strtenorvec = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
		DoubleVector tenorvec(strtenorvec.size());
		for (unsigned int i = 0; i < tenorvec.size(); i++)
		{
			int y,m,d,w;
			AQLMathDateCalculations::termStrtoYMDW(strtenorvec[i], y, m, d, w);
			tenorvec[i] = static_cast<double > (y) + static_cast<double > (m) / 12;
		}

		// curve names
		dh = &(calibInfo.getData(PRICING_DATA_CURVETYPEKEYS, ISNOTNULL));
		const AQLStringVector& curvetypekeys = dynamic_cast<const AQLDataStrings &>(dh->get()).get();

		dh = &(calibInfo.getData(PRICING_DATA_CURVETYPES, ISNOTNULL));
		const AQLStringVector& curvetypes = dynamic_cast<const AQLDataStrings &>(dh->get()).get();

		unsigned int curvematsize = 1 + curvetypekeys.size();
		
		AQLStringMatrix sizemat(curvematsize,AQLStringVector(2));
		mcurvesetmat[underlying] = sizemat;
		mcurvesetmat[underlying][0][0] = "CurveID";
		mcurvesetmat[underlying][0][1] = curveIDName;
		for (unsigned int i = 1; i < curvematsize; i++)
		{
			mcurvesetmat[underlying][i][0] = curvetypekeys[i-1];
			mcurvesetmat[underlying][i][1] = curvetypes[i-1];
		}
		AQLMathSwaptionVolUtility::setCurveID2(mpDataInstance,mcurvesetid[underlying],strtenorvec,mcurvesetmat[underlying]);
		// approximation method
		dh = &(calibInfo.getData(PRICING_DATA_APPROXMETHOD, ISNOTNULL));
		mapproxmethod[underlying] = dynamic_cast<const AQLDataString &>(dh->get()).get();
		// calibration method
		dh = &(calibInfo.getData(PRICING_DATA_CALIBMETHOD, ISNOTNULL));
		mcalibmethod[underlying] = dynamic_cast<const AQLDataString &>(dh->get()).get();
		// option type
		dh = &(calibInfo.getData(PRICING_DATA_OPTIONTYPE, ISNOTNULL));
		const AQLStringVector &optypeVec = dynamic_cast<const AQLDataStrings &>(dh->get()).get();
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
				AQLString msg = "Invalid option type : " + optypeVec[j];
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}
			moptionsign[underlying][j] = flg;
		}
		//ishaganatmvolfixed
		mishaganatmvolfixed[underlying] = true;
		dh =  &(calibInfo.getData(PRICING_DATA_ISHAGANATMVOLFIXED, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			mishaganatmvolfixed[underlying] = dynamic_cast<const AQLDataBool &>(dh->get()).get();
		}
		//forwardslidevalue
		mforwardshiftvalue[underlying] = 0.;
		dh =  &(calibInfo.getData(PRICING_DATA_FORWARDSHIFTVALUE, NOCHECK));
		if (dh->isDefined() && !dh->isNull())
		{
			mforwardshiftvalue[underlying] = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
		}
		unsigned int forSize = 5;
		bool is_alpha_entity_exist = false, is_beta_entity_exist = false, is_nu_entity_exist = false, is_rho_entity_exist = false;
		for (unsigned int i = 0; i < forSize; i++)
		{
			AQLString addstr;
			if (0 == i )
			{
				addstr =  AQLString("_") + PRICING_DATA_VOLID;
			}
			else if (1 == i )
			{
				addstr =  AQLString("_") + PRICING_DATA_ALPHAID;
			}
			else if (2 == i )
			{
				addstr =  AQLString("_") + PRICING_DATA_BETAID;
			}
			else if (3 == i )
			{
				addstr =   AQLString("_") + PRICING_DATA_RHOID;
			}
			else if (4 == i )
			{
				addstr =  AQLString("_") + PRICING_CALIBRATION_DATAUID;
			}

			const AQLDataHolder* dh = &(calibInfo.getData("InitialIRSABR" + addstr, ISNOTNULL));
			const AQLDataReference& ref = dynamic_cast<const AQLDataReference& >(dh->get());
			const AQLObject& einit = ref.get().get();

			AQLString name = dynamic_cast<const AQLDataString &>(einit.getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
			if (gridPos >= 0 && !param.gridTerm.empty())
			{
				name +=  param.calcType + "_" + curveIDName + "_" + param.gridTerm[gridPos];
			}
			else
			{
				name +=  param.calcType + "_" + curveIDName + "_" + AQLString(gridPos);
			}
			bool is_param_entity_exist = false;
			AQLObject* newide = NULL;
			AQLObjectHolder objHolder = objPool.getObject( name, ENCHKTYPE_NOCHECK );
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
			newide->add(PRICING_DATA_EXPIRYSTRING, new AQLDataStrings(strexpiryvec));
			//expiryvec
			newide->remove(PRICING_DATA_EXPIRYVECTOR);
			newide->add(PRICING_DATA_EXPIRYVECTOR, new AQLDataDoubles(expiryvec));
			//tenorstring
			newide->remove(PRICING_DATA_TENORSTRING);
			newide->add(PRICING_DATA_TENORSTRING, new AQLDataStrings(strtenorvec));
			//tenorvec
			newide->remove(PRICING_DATA_TENORVECTOR);
			newide->add(PRICING_DATA_TENORVECTOR, new AQLDataDoubles(tenorvec));
			//expirydatevec
			newide->remove(PRICING_DATA_EXPIRYDATEVECTOR);
			newide->add(PRICING_DATA_EXPIRYDATEVECTOR, new AQLDataDates(expirydatevec));
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
		dh = &(calibInfo.getData(AQLString("InitialIRSABR") + AQLString("_") + PRICING_DATA_VOLID, ISNOTNULL));
		const AQLDataReference& ref = dynamic_cast<const AQLDataReference& >(dh->get());
		AQLString volname = dynamic_cast<const AQLDataString &>(ref.get().getData(CALIBRATION_DATA_NAME,ISNOTNULL).get()).get();
		AQLString volnameSuffix;
		if (gridPos >= 0 && !param.gridTerm.empty())
		{
			volnameSuffix =  param.calcType + "_" + curveIDName + "_" + param.gridTerm[gridPos];
		}
		else
		{
			volnameSuffix =  param.calcType + "_" + curveIDName + "_" + AQLString(gridPos);
		}
		volname += volnameSuffix;
		
		AQLObject& vole = objPool.getObject(volname, ENCHKTYPE_ISDEFINED).get();

		matmvolid[underlying] = volname;
		mswapvolid[underlying].clear();
		mswapvolid[underlying].resize(1, volname);
		//set calibration target flag
		AQLString calibTarget_str = LACoreDataService::getContext(ccy + "." CONTEXT_KEY_DEAL_IRVOL + "." + underlying);
		mcalibflgmat[underlying].clear();
		if (calibTarget_str != AQ_NO_DATA)
		{
			AQLDataBoolMatrix tmp;
			tmp.convertFromString(calibTarget_str);
			BoolMatrix calibTarget = tmp.get();
			vole.remove(PRICING_DATA_CALIBRATIONFLAG);
			vole.add(PRICING_DATA_CALIBRATIONFLAG, new AQLDataBoolMatrix(calibTarget));
			mcalibflgmat[underlying] = calibTarget;
		}
		if (mishaganatmvolfixed[underlying] && (param.targetCurveType != underlying)) continue;

		AQLString scenarioTargetID = PRICING_DATA_VOLID;
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

		dh = &(calibInfo.getData(AQLString("InitialIRSABR") + AQLString("_") + scenarioTargetID, ISNOTNULL));
		const AQLDataReference& refScenarioTarget = dynamic_cast<const AQLDataReference& >(dh->get());
		AQLString scenarioTargetName = dynamic_cast<const AQLDataString &>(refScenarioTarget.get().getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
		scenarioTargetName += volnameSuffix;
		AQLObject& scenarioTargetEntity = objPool.getObject(scenarioTargetName, ENCHKTYPE_ISDEFINED).get();
		AQLDataHolder* ahScenarioTarget = &(scenarioTargetEntity.getData(PRICING_DATA_SWAPTIONMATRIX, ISNOTNULL));
		DoubleMatrix& scenarioTargetMat = dynamic_cast<AQLDataDoubleMatrix &>(ahScenarioTarget->get()).get();
		
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
							scenarioTargetMat[i][j] = AQLMath::max(scenarioTargetMat[i][j], scenarioTargetMin);
							scenarioTargetMat[i][j] = AQLMath::min(scenarioTargetMat[i][j], scenarioTargetMax);
						}
					}
				}
				else
				{
					throw AQLCoreInvalidData("Not supported now in SABR model",__FILE__,__LINE__); 
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
				const int gridStart = param.isWave ? 0 : gridPos;

				for (int iGrid = gridStart; iGrid <= gridPos; ++iGrid) {

					// check
					AQLString term = param.gridTerm[iGrid];
					term.toUpper();
					//int pos = term.findString(CALIB_MARKET_SWAPTION);
					//if (pos != 0)
					if (term.findString(CALIB_MARKET_SWAPTION) == -1)
					{
						throw AQLCoreInvalidData("IRSABR Market data swaption only.", __FILE__, __LINE__);
					}
					//unsigned int gStartPos = 9;
					AQLStringVector tmpGridTerm = term.toToken('_');
					//AQLString grid = term.subString(gStartPos, term.size() - 1);
					AQLString grid = tmpGridTerm[tmpGridTerm.size() - 2] + "_" + tmpGridTerm[tmpGridTerm.size() - 1];

					// create mat and swapterm index vex
					for (unsigned int i = 0; i < strexpiryvec.size(); i++)
					{
						unsigned int gridSize = strtenorvec.size();
						AQLStringVector tmpGridVec(gridSize);
						
						for (unsigned int j = 0; j < gridSize; j++)
						{
							tmpGridVec[j] = strexpiryvec[i] + "_" + strtenorvec[j];
							tmpGridVec[j].toUpper();
						}
						// check term
						AQLStringVector::const_iterator it = find(tmpGridVec.begin(), tmpGridVec.end(), grid);
						if (it == tmpGridVec.end())
						{
							continue;
							/*AQLString msg = "Shift grid is not in swaption market. grid = " + grid;
							throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);*/
						}
						unsigned int index = static_cast<unsigned int>(it - tmpGridVec.begin());

						// shift val
						double shiftVal = param.gridShiftVec[iGrid];
						AQLString shiftType = param.shiftType;
						if (shiftType.toUpper() == RISK_SHIFTTYPE_RATIO)
						{
							shiftVal = scenarioTargetMat[i][index] * shiftVal;
						}
						// add shift val
						scenarioTargetMat[i][index] = scenarioTargetMat[i][index] + shiftVal;
						scenarioTargetMat[i][index] = AQLMath::max(scenarioTargetMat[i][index], scenarioTargetMin);
						scenarioTargetMat[i][index] = AQLMath::min(scenarioTargetMat[i][index], scenarioTargetMax);
						break;
					}
				}
			}
		}
		if (!mishaganatmvolfixed[underlying])
		{
			// calc ATM strike(forward rate)
			const AQLObject &curveEntity = objPool.getObject(mcurvesetid[underlying], ENCHKTYPE_ISDEFINED ).get();
			const AQLString &curveID = dynamic_cast<const AQLDataString& >(curveEntity.getData(PRICING_DATA_CURVEID, ISDEFINED).get()).get();
			const AQLString &dfName = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME, ISDEFINED).get()).get();
			DoubleMatrix strikemat(expirydatevec.size());
			if (!mcalibflgmat[underlying].empty() && mcalibflgmat[underlying].size() != strikemat.size())
			{
				throw AQLCoreInvalidData("Calib flag format is wrong!", __FILE__, __LINE__);
			}
			for (unsigned int j = 0; j < expirydatevec.size(); ++j)
			{
				strikemat[j].resize(strtenorvec.size());
				if (!mcalibflgmat[underlying].empty() && mcalibflgmat[underlying][j].size() != strikemat[j].size())
				{
					throw AQLCoreInvalidData("Calib flag format is wrong!", __FILE__, __LINE__);
				}
				for (unsigned int k = 0; k < strtenorvec.size(); ++k)
				{
					if (!mcalibflgmat[underlying].empty() && !mcalibflgmat[underlying][j][k])
					{
						continue;
					}
					const AQLString *forName = 0;
					const AQLString *convID = 0;
					if (strtenorvec[k] == "1M")
					{
						forName =  &(dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_1MLCURVENAME, ISDEFINED).get()).get());
						convID = &mcapconvid[underlying];
					}
					else if (strtenorvec[k] == "3M")
					{
						forName =  &(dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_3MLCURVENAME, ISDEFINED).get()).get());
						convID = &mcapconvid[underlying];
					}
					else if  (strtenorvec[k] == "6M")
					{
						forName =  &(dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_6MLCURVENAME, ISDEFINED).get()).get());
						convID = &mcapconvid[underlying];
					}
					else
					{
						forName =  &(dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_SWAPRATELCURVENAME, ISDEFINED).get()).get());
						convID = &mswapconvid[underlying];
					}
					strikemat[j][k] = AQLMath::max(AQLMathSwaptionVolUtility::getForward(mpDataInstance, expirydatevec[j], strtenorvec[k], curveID, *convID, *forName, dfName), eps_SABR);
				}
			}
			vole.remove(PRICING_DATA_SWAPTIONSTRIKEMATRIX);
			vole.add(PRICING_DATA_SWAPTIONSTRIKEMATRIX, new AQLDataDoubleMatrix(strikemat));
			// if target is premium setup premium instead volatility
			AQLString tmp_target = mtarget[underlying];
			tmp_target.toUpper();
			if (tmp_target ==  CALIB_TARGET_PREMIUM)
			{
				DoubleMatrix premiummat;
				dh = &(vole.getData(PRICING_DATA_PREMIUMMATRIX, NOCHECK));
				if (dh->isDefined() && !dh->isNull())
				{
					premiummat = dynamic_cast<const AQLDataDoubleMatrix &>(dh->get()).get();
				}
				else
				{
					AQLString optBuy(CALIB_OPT_BUY);
					AQLString optionType(PAYERS);
					if (moptionsign[underlying][0] == -1)
					{
						optionType = RECEIVERS;
					}
					premiummat.resize(expirydatevec.size());
					const double forwardShift = mforwardshiftvalue[underlying];

					AQLDataHolder* volah = &(vole.getData(PRICING_DATA_SWAPTIONMATRIX, ISNOTNULL));
					DoubleMatrix& volmat = dynamic_cast<AQLDataDoubleMatrix &>(volah->get()).get();

					for (unsigned int j= 0; j < expirydatevec.size(); ++j)
					{
						premiummat[j].resize(strtenorvec.size());
						for (unsigned int k = 0; k < strtenorvec.size(); ++k)
						{
							if (!mcalibflgmat[underlying].empty() && !mcalibflgmat[underlying][j][k])
							{
								continue;
							}
							const AQLString *forName = 0;
							const AQLString *convID = 0;
							AQLString freq = "";
							if (strtenorvec[k] == "1M")
							{
								forName =  &(dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_1MLCURVENAME, ISDEFINED).get()).get());
								convID = &mcapconvid[underlying];
								freq = FREQ_MONTHLY;
							}
							else if (strtenorvec[k] == "3M")
							{
								forName =  &(dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_3MLCURVENAME, ISDEFINED).get()).get());
								convID = &mcapconvid[underlying];
								freq = FREQ_QUARTERLY;
							}
							else if  (strtenorvec[k] == "6M")
							{
								forName =  &(dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_6MLCURVENAME, ISDEFINED).get()).get());
								convID = &mcapconvid[underlying];
								freq = FREQ_SEMI_ANNUAL;
							}
							else
							{
								forName =  &(dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_SWAPRATELCURVENAME, ISDEFINED).get()).get());
								convID = &mswapconvid[underlying];
								//freq
								const AQLObject& convEntity = objPool.getObject(*convID, ENCHKTYPE_ISDEFINED).get();
								dh = &convEntity.getData(IR_CALIBRATION_DATA_FREQUENCY + AQLString("_") + strtenorvec[k], NOCHECK);
								if(!dh->isDefined() || dh->isNull())
								{
									dh = &convEntity.getData(IR_CALIBRATION_DATA_FREQUENCY, ISDEFINED);
								}
								freq = dh->convertToString().exchange("\"","");

							}
							const AQLObject& convEntity = objPool.getObject(*convID, ENCHKTYPE_ISDEFINED).get();
							// spotLag
							AQLString spotLag = convEntity.getData(CURVEINPUT_SPOTLAG, ISDEFINED).convertToString().exchange("\"","");
							// daycount
							AQLString daycount = convEntity.getData(IR_CALIBRATION_DATA_DAYCOUNT, ISDEFINED).convertToString().exchange("\"","");
							// sliding rule
							AQLString sldrule = convEntity.getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).convertToString().exchange("\"","");
							// payment calendar
							AQLString payCal = convEntity.getData(CALIBRATION_DATA_CALENDAR, ISDEFINED).convertToString().exchange("\"","");
							// fixing  calendar
							AQLString fixCal = convEntity.getData(PRICING_DATA_FIXINGCALENDAR, ISDEFINED).convertToString().exchange("\"","");

							premiummat[j][k] = AQLMathIRVanillaFuncUtility::swaption(mpDataInstance, curveID, optBuy, 
																				optionType, 1.0, strikemat[j][k], volmat[j][k],
																				asofDate, expirydatevec[j], asofDate, spotLag, strtenorvec[k],
																				freq, sldrule, daycount, payCal, fixCal, -10.0, forwardShift, *forName, dfName);
						}
					}
				}
				vole.remove(PRICING_DATA_SWAPTIONMATRIX);
				vole.add(PRICING_DATA_SWAPTIONMATRIX, new AQLDataDoubleMatrix(premiummat));
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
		throw AQLCoreInvalidData("DataInstance member is NULL", __FILE__, __LINE__);
	}

	AQLObjectPool &objPool = mpDataInstance->getObjectPool();

	AQLMathVolFuncIRSABR* method = new AQLMathVolFuncIRSABR(mpDataInstance);

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
				AQLString msg;
				AQLMathSwaptionVolUtility::calibrateSABRMatrix(mpDataInstance,
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
				AQLMathSwaptionVolUtility::calibrateSABRATMFix(mpDataInstance,
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
			AQLStringVector sabrParamsID(4);
			sabrParamsID[0] = malphaid[mUnderlyings[i_under]]; 
			sabrParamsID[1] = mbetaid[mUnderlyings[i_under]]; 
			sabrParamsID[2] = mnuid[mUnderlyings[i_under]]; 
			sabrParamsID[3] = mrhoid[mUnderlyings[i_under]]; 

			for (size_t i=0; i<sabrParamsID.size(); i++)
			{
				const AQLString fileSuffix = LACoreDataService::getContext(ARG_KEY_FILENUM);
				const AQLString dirName = LACoreDataService::getOutputDirectory(); 
				AQLString fileName = sabrParamsID[i].subString(26, sabrParamsID[i].size() - 1);
				fileName = dirName + fileName + fileSuffix + ".csv";
				AQLObject& sabrParamEntity = objPool.getObject(sabrParamsID[i],ENCHKTYPE_ISDEFINED).get();

				ifstream fin;
				ofstream fout;
				fin.open(fileName.getCString());

				if (!fin)
				{
					DoubleMatrix& mat = dynamic_cast<AQLDataDoubleMatrix &> (sabrParamEntity.getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

					fout.open(fileName.getCString());

					for (size_t i = 0; i < mat.size(); ++i)
					{
						AQLString output;
						for (size_t j = 0; j < mat[0].size(); ++j)
						{
							output += AQLString(mat[i][j]);
							output += AQLString(",");
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
						AQLStringVector lineVec = AQLString(c_line).toToken(MARKET_DATA_DELIMITER);
						DoubleVector lineVec_d;
						for (size_t i=0; i<lineVec.size(); i++)
						{
							lineVec_d.push_back(lineVec[i].trimLeft().trimRight().getDoubleValue());
						}
						mat.push_back(lineVec_d);
					}
					fin.close();
					dynamic_cast<AQLDataDoubleMatrix &>(sabrParamEntity.getData(PRICING_DATA_SWAPTIONMATRIX, ISNOTNULL).get()).set(mat);
				}
			}
		}
	}

	mpFunc->setRealFunction(*method);
	mpFunc->setOn();

	delete method;
	cout << static_cast<int>(LACoreThread::getThreadID()) << " LACalibrateIRSABR calibrate end.." << endl;
}


