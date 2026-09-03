/*! @file
    @brief FXVanilla sde generator class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModelFXStrangleSolver.cpp
//
//  DESCRIPTION :        FXVanilla SDE generator 
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


#include "LACalibrateModelFXStrangleSolver.h"
#include "AQLFunctionBase.h"
#include "AQLFunctionManager.h"
#include "AQLMathVolFuncBase.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDriftLMMSpot.h"
#include "AQLPriceDriftFX.h"
#include "AQLRatesSpotSDE.h"
#include "AQLRatesLJSpotSDE.h"
#include "AQLRatesEulerMaruyama.h"
#include "LACalibrateModelFX.h"
#include "LADefinitionsPtberg.h"
#include "LAMarketData.h"
#include "AQLMathVolFuncFX.h"
#include "AQLMathFXAdjuster.h"
#include "AQLMathVolatility.h"
#include "LAScenarioConfiguration.h"
#include "LACalibrateVolatilityFXStrangleSolver.h"
#include "LADealUtils.h"
#include "LAStaticData.h"
#include "AQLConstant.h"
#include "AQLPriceFXVolatility.h"
#include "AQLPriceFXDDIntegral.h"
#include "AQLPriceFXDDIntegralMelstein.h"
#include "LACalibrationParametersFXStrangleSolver.h"
#include "AQLMathVolFuncFXStrangleSolver.h"
#include "LACalibrateFXStrangleSolver.h"
#include "LACalibrationFunc.h"
#include "AQLDataMatrix.h"

using namespace std;

// constructor
/*!

*/
LACalibrateModelFXStrangleSolver::LACalibrateModelFXStrangleSolver()
: LACalibrateModelFX()
{
}

// destructor
/*!

*/
LACalibrateModelFXStrangleSolver::~LACalibrateModelFXStrangleSolver(void)
{
}


// 
/*!
    @brief create sde instance

	@param[in] fx
	@param[in] dataInstance
*/
AQLRatesSDEBase *
LACalibrateModelFXStrangleSolver::createSDEInstance(const AQLString &fx, AQLDataInstance &dataInstance) const
{
	return 0;
}


/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
LACalibrateModelFXStrangleSolver::getSDEType(const AQLString &fx) const
{
	SDE_TYPE a;
	return a;
}

/*!
	@brief check is Long Jump

	@param[in] fx

*/
bool 
LACalibrateModelFXStrangleSolver::isLJ(const AQLString &fx) const
{
	return false;
}

/*!
	@brief set volatility to sde

	@param[in] fx
	@param[out] sde
*/
void
LACalibrateModelFXStrangleSolver::setVolatility(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	fx;sde;
	return;
}

/*!
	@brief set drift class to sde

	note: fx's first cccy (ex.ccy1/cccy2 ccy2) must be domestic ccy

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelFXStrangleSolver::setDrift(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	fx;sde;
	return;
}



/*!
	@brief set integral function

	@param[in] fx
	@param[out] sde

*/
void
LACalibrateModelFXStrangleSolver::setIntegralFunction(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	fx;sde;
	return;
}

// 
/*!
    @brief set path

	@param[in] fx
	@param[out] sde
*/
void
LACalibrateModelFXStrangleSolver::setOutputTemplate(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	fx;sde;
	return;
}

// 
/*!
    @brief set setInterpolationMethod

	@param[in] fx
	@param[out] sde
*/
void
LACalibrateModelFXStrangleSolver::setInterpolationMethod(const AQLString &fx, AQLRatesSDEBase &sde) const
{
	fx;sde;
	return;
}



/*!
	@brief get function master regist name

	@param[in] fx
*/
AQLString 
LACalibrateModelFXStrangleSolver::getFunctionMasterResistName(const AQLString &fx) const
{
	AQLString tmpFX = fx;
	return fx;
	
}

// 
/*!
    @brief get volatility input type

	@param[in]  fx 
*/
AQLString
LACalibrateModelFXStrangleSolver::getVolType(const AQLString &fx) const
{
	return INPUT_FUNC;
}

// 
/*!
    @brief set volatility function

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelFXStrangleSolver::setUpVolFunc(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	setUpVolEntity(fx,vol);
	AQLStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLStringVector fileVec(1);
	
	MAScenarioParam param;
	param.calcType = KEY_PV;
	// set fx
	param.ccy = fx;
	
	AQLObjectPool &objPool = dataInstance.getObjectPool();
	
	param.isCalib = isCalibTarget(fx);
	if (param.isCalib)
	{
		// create calib info
		LACalibrationParametersFXStrangleSolver cInfo;
		AQLString cInfoName = cInfo.createCalibrationInfo(dataInstance.getObjectPool(), fx);
		// first element set calib info
		param.refName.push_back(cInfoName);

		if (LACoreDataService::getContext(ARG_KEY_DATAOUT) != AQ_NO_DATA)
		{
			param.isOutPut = true;
		}
		else 
		{
			param.isOutPut = false;
		}
		// set yield and calibdata
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[0]));
		param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, param.refName.back())));
		param.refName.push_back(LAMarketData::getBaseYieldName(ccys[1]));
		param.refName.push_back(LAMarketData::getCalibDataName(KEY_PV, LAMarketData::getYieldDataName(objPool, param.refName.back())));
	}
	else
	{
		throw AQLCoreInvalidData("Only Calibration supports",__FILE__,__LINE__);
	}
	
	// create function
	LACalibrateVolatilityFXStrangleSolver volCreator;
	AQLFunctionBase *method = volCreator.createVolatility(fileVec, &param, &objPool);
	vol.setVolatility(method);
	
	//// add strangle solver data to dataValues
	AQLMathVolFuncFXStrangleSolver* func_fx = dynamic_cast<AQLMathVolFuncFXStrangleSolver*>(method);
	vol.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONMETHOD, new AQLDataInt(func_fx->getInterpolationMethod()));
	vol.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONTARGET, new AQLDataInt(func_fx->getInterpolationTarget()));
	vol.AQLObject::add(IR_CALIBRATION_DATA_INTERPOLATIONVARIABLE, new AQLDataInt(func_fx->getInterpolationVariable()));
	vol.AQLObject::add(IR_CALIBRATION_DATA_ATMINTERPOLATIONMETHOD, new AQLDataInt(func_fx->getATMInterpolationMethod()));
	vol.AQLObject::add(IR_CALIBRATION_DATA_ISWINGFLAG, new AQLDataBool(func_fx->getIsWing()));
	// smile data
	DoubleMatrix deltaPutMat, logStrikeMat, strikeMat, volMat;
	vector<SmileData > smileData = func_fx->getSmileData();
	for (size_t i = 0; i < smileData.size(); ++i)
	{
		deltaPutMat.push_back(smileData[i].deltaPuts);
		logStrikeMat.push_back(smileData[i].logStrikes);
		strikeMat.push_back(smileData[i].strikes);
		volMat.push_back(smileData[i].vols);
	}
	vol.AQLObject::add(IR_CALIBRATION_DATA_SMILEDATA_DELTAPUT, new AQLDataDoubleMatrix(deltaPutMat));
	vol.AQLObject::add(IR_CALIBRATION_DATA_SMILEDATA_LOGSTRIKE, new AQLDataDoubleMatrix(logStrikeMat));
	vol.AQLObject::add(IR_CALIBRATION_DATA_SMILEDATA_STRIKE, new AQLDataDoubleMatrix(strikeMat));
	vol.AQLObject::add(IR_CALIBRATION_DATA_SMILEDATA_VOL, new AQLDataDoubleMatrix(volMat));
	// fx option data
	DoubleVector T_fxOptData, Pd_fxOptData, Pf_fxOptData, F_fxOptData, spotFX_fxOptData, Days_fxOptData;
	IntVector deltaType_fxOptData, atmType_fxOptData;
	vector<FXOptionData > fxOptData = func_fx->getFXOptionData();
	for (size_t i = 0; i < fxOptData.size(); ++i)
	{
		T_fxOptData.push_back(fxOptData[i].T);
		Pd_fxOptData.push_back(fxOptData[i].Pd);
		Pf_fxOptData.push_back(fxOptData[i].Pf);
		F_fxOptData.push_back(fxOptData[i].F);
		spotFX_fxOptData.push_back(fxOptData[i].spotFX);
		Days_fxOptData.push_back(fxOptData[i].Days);
		deltaType_fxOptData.push_back(fxOptData[i].deltaType);
		atmType_fxOptData.push_back(fxOptData[i].atmType);
	}
	vol.AQLObject::add(IR_CALIBRATION_DATA_FXOPTDATA_T, new AQLDataDoubles(T_fxOptData));
	vol.AQLObject::add(IR_CALIBRATION_DATA_FXOPTDATA_PD, new AQLDataDoubles(Pd_fxOptData));
	vol.AQLObject::add(IR_CALIBRATION_DATA_FXOPTDATA_PF, new AQLDataDoubles(Pf_fxOptData));
	vol.AQLObject::add(IR_CALIBRATION_DATA_FXOPTDATA_F, new AQLDataDoubles(F_fxOptData));
	vol.AQLObject::add(IR_CALIBRATION_DATA_FXOPTDATA_SPOTFX, new AQLDataDoubles(spotFX_fxOptData));
	vol.AQLObject::add(IR_CALIBRATION_DATA_FXOPTDATA_DAYS, new AQLDataDoubles(Days_fxOptData));
	vol.AQLObject::add(IR_CALIBRATION_DATA_FXOPTDATA_DELTATYPE, new AQLDataInts(deltaType_fxOptData));
	vol.AQLObject::add(IR_CALIBRATION_DATA_FXOPTDATA_ATMTYPE, new AQLDataInts(atmType_fxOptData));
}


// 
/*!
    @brief setup volatility data

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelFXStrangleSolver::setUpVolData(const AQLString &fx, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const
{
	fx;
	vol;
	dataInstance;
}

// 
/*!
    @brief  setup volatility object

	@param[in] fx
	@param[out] vol

*/
void
LACalibrateModelFXStrangleSolver::setUpVolEntity(const AQLString &fx, AQLMathVolatility &vol) const
{
	// set interpolation
	AQLStringVector ccys = fx.toToken(FX_DELIMITER);
	AQLString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	AQLString interp = "fn_linearinterpolation";
	vol.getInterpolation().convertFromString(interp);
}

