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
#include "LAFunctionBase.h"
#include "LAFunctionManager.h"
#include "LAMathVolFuncBase.h"
#include "LAPriceDataInterpolation.h"
#include "LAPriceDriftLMMSpot.h"
#include "LAPriceDriftFX.h"
#include "LARatesSpotSDE.h"
#include "LARatesLJSpotSDE.h"
#include "LARatesEulerMaruyama.h"
#include "LACalibrateModelFX.h"
#include "LADefinitionsPtberg.h"
#include "LAMarketData.h"
#include "LAMathVolFuncFX.h"
#include "LAMathFXAdjuster.h"
#include "LAMathVolatility.h"
#include "LAScenarioConfiguration.h"
#include "LACalibrateVolatilityFXStrangleSolver.h"
#include "LADealUtils.h"
#include "LAStaticData.h"
#include "LAConstant.h"
#include "LAPriceFXVolatility.h"
#include "LAPriceFXDDIntegral.h"
#include "LAPriceFXDDIntegralMelstein.h"
#include "LACalibrationParametersFXStrangleSolver.h"
#include "LAMathVolFuncFXStrangleSolver.h"
#include "LACalibrateFXStrangleSolver.h"
#include "LACalibrationFunc.h"
#include "LADataMatrix.h"

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
LARatesSDEBase *
LACalibrateModelFXStrangleSolver::createSDEInstance(const LAString &fx, LADataInstance &dataInstance) const
{
	return 0;
}


/*!
	@brief return ptberg sde type

	@param[in] fx

*/
SDE_TYPE
LACalibrateModelFXStrangleSolver::getSDEType(const LAString &fx) const
{
	SDE_TYPE a;
	return a;
}

/*!
	@brief check is Long Jump

	@param[in] fx

*/
bool 
LACalibrateModelFXStrangleSolver::isLJ(const LAString &fx) const
{
	return false;
}

/*!
	@brief set volatility to sde

	@param[in] fx
	@param[out] sde
*/
void
LACalibrateModelFXStrangleSolver::setVolatility(const LAString &fx, LARatesSDEBase &sde) const
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
LACalibrateModelFXStrangleSolver::setDrift(const LAString &fx, LARatesSDEBase &sde) const
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
LACalibrateModelFXStrangleSolver::setIntegralFunction(const LAString &fx, LARatesSDEBase &sde) const
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
LACalibrateModelFXStrangleSolver::setOutputTemplate(const LAString &fx, LARatesSDEBase &sde) const
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
LACalibrateModelFXStrangleSolver::setInterpolationMethod(const LAString &fx, LARatesSDEBase &sde) const
{
	fx;sde;
	return;
}



/*!
	@brief get function master regist name

	@param[in] fx
*/
LAString 
LACalibrateModelFXStrangleSolver::getFunctionMasterResistName(const LAString &fx) const
{
	LAString tmpFX = fx;
	return fx;
	
}

// 
/*!
    @brief get volatility input type

	@param[in]  fx 
*/
LAString
LACalibrateModelFXStrangleSolver::getVolType(const LAString &fx) const
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
LACalibrateModelFXStrangleSolver::setUpVolFunc(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const
{
	setUpVolEntity(fx,vol);
	LAStringVector ccys;
	LAMarketData::convertToCurrency(fx, ccys);

	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAStringVector fileVec(1);
	
	MAScenarioParam param;
	param.calcType = KEY_PV;
	// set fx
	param.ccy = fx;
	
	LAObjectPool &objPool = dataInstance.getObjectPool();
	
	param.isCalib = isCalibTarget(fx);
	if (param.isCalib)
	{
		// create calib info
		LACalibrationParametersFXStrangleSolver cInfo;
		LAString cInfoName = cInfo.createCalibrationInfo(dataInstance.getObjectPool(), fx);
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
		throw LACoreInvalidData("Only Calibration supports",__FILE__,__LINE__);
	}
	
	// create function
	LACalibrateVolatilityFXStrangleSolver volCreator;
	LAFunctionBase *method = volCreator.createVolatility(fileVec, &param, &objPool);
	vol.setVolatility(method);
	
	//// add strangle solver data to dataValues
	LAMathVolFuncFXStrangleSolver* func_fx = dynamic_cast<LAMathVolFuncFXStrangleSolver*>(method);
	vol.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONMETHOD, new LADataInt(func_fx->getInterpolationMethod()));
	vol.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONTARGET, new LADataInt(func_fx->getInterpolationTarget()));
	vol.LAObject::add(IR_CALIBRATION_DATA_INTERPOLATIONVARIABLE, new LADataInt(func_fx->getInterpolationVariable()));
	vol.LAObject::add(IR_CALIBRATION_DATA_ATMINTERPOLATIONMETHOD, new LADataInt(func_fx->getATMInterpolationMethod()));
	vol.LAObject::add(IR_CALIBRATION_DATA_ISWINGFLAG, new LADataBool(func_fx->getIsWing()));
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
	vol.LAObject::add(IR_CALIBRATION_DATA_SMILEDATA_DELTAPUT, new LADataDoubleMatrix(deltaPutMat));
	vol.LAObject::add(IR_CALIBRATION_DATA_SMILEDATA_LOGSTRIKE, new LADataDoubleMatrix(logStrikeMat));
	vol.LAObject::add(IR_CALIBRATION_DATA_SMILEDATA_STRIKE, new LADataDoubleMatrix(strikeMat));
	vol.LAObject::add(IR_CALIBRATION_DATA_SMILEDATA_VOL, new LADataDoubleMatrix(volMat));
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
	vol.LAObject::add(IR_CALIBRATION_DATA_FXOPTDATA_T, new LADataDoubles(T_fxOptData));
	vol.LAObject::add(IR_CALIBRATION_DATA_FXOPTDATA_PD, new LADataDoubles(Pd_fxOptData));
	vol.LAObject::add(IR_CALIBRATION_DATA_FXOPTDATA_PF, new LADataDoubles(Pf_fxOptData));
	vol.LAObject::add(IR_CALIBRATION_DATA_FXOPTDATA_F, new LADataDoubles(F_fxOptData));
	vol.LAObject::add(IR_CALIBRATION_DATA_FXOPTDATA_SPOTFX, new LADataDoubles(spotFX_fxOptData));
	vol.LAObject::add(IR_CALIBRATION_DATA_FXOPTDATA_DAYS, new LADataDoubles(Days_fxOptData));
	vol.LAObject::add(IR_CALIBRATION_DATA_FXOPTDATA_DELTATYPE, new LADataInts(deltaType_fxOptData));
	vol.LAObject::add(IR_CALIBRATION_DATA_FXOPTDATA_ATMTYPE, new LADataInts(atmType_fxOptData));
}


// 
/*!
    @brief setup volatility data

	@param[in] fx
	@param[out] vol
	@param[out] dataInstance
*/
void
LACalibrateModelFXStrangleSolver::setUpVolData(const LAString &fx, LAMathVolatility &vol, LADataInstance &dataInstance) const
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
LACalibrateModelFXStrangleSolver::setUpVolEntity(const LAString &fx, LAMathVolatility &vol) const
{
	// set interpolation
	LAStringVector ccys = fx.toToken(FX_DELIMITER);
	LAString key_fx = LAMarketData::getFXKey(ccys[0], ccys[1]);
	LAString interp = "fn_linearinterpolation";
	vol.getInterpolation().convertFromString(interp);
}

