#define _HAS_STD_BYTE 0

/*! @file
    @brief Class to regist master data
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLModelSetupBase.cpp
//
//  DESCRIPTION :       Master data regist class
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


#include "AQLModelSetupBase.h"
#include "AQLModelManager.h"
#include "AQLDataInstance.h"
#include "AQLObjectMaster.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"

#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLDataProcedure.h"
#include "AQLDataValuation.h"
#include "AQLDataValuation.h"
#include "AQLDataBasics.h"
#include "AQLDataMultiReference.h"
#include "AQLDataReference.h"
#include "AQLDataMatrix.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLPriceDataRand.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataConvention.h"
#include "AQLPriceDataFunction.h"
#include "AQLPriceDataFunctions.h"
#include "AQLMathAttrSDE.h"
#include "AQLPriceArbFreeGenerator.h"

#include "AQLMathValuableEntity.h"
#include "AQLMathDefine.h"

#include "AQLPriceCashFlowGenerator.h"
#include "AQLPriceYieldGenerator.h"
#include "AQLPricePayOff.h"
#include "AQLPriceCouponRainbow.h"
#include "AQLPriceTradeValue.h"
#include "AQLPriceAccruedInterest.h"

#include "AQLMathPathEntity.h"
#include "AQLMathIndexEntity.h"
#include "AQLMathFXEntity.h"
#include "AQLMathYieldCurve.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLMathCorrelation.h"
#include "AQLMathVolatility.h"

#include "AQLBasic.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLParabolicInterpolation.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"
#include "AQLMonotoneConvexInterpolation.h"
#include "AQLMonotoneSplineInterpolation.h"
#include "AQLConstrainedSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQL1DDataSet.h"
#include "AQLConstant.h"
#include "AQLGaussLegendre.h"
#include "AQLRomberg.h"
#include "AQLLinearFunc.h"
#include "AQLQuadraticFunc.h"
#include "AQLDivideFunc.h"
#include "AQLMaxFunc.h"
#include "AQLMinFunc.h"
#include "AQLSumFunc.h"
#include "AQLAverageFunc.h"
#include "AQLDigitalFunc.h"
#include "AQLFractionFunc.h"
#include "AQLCompoundingFunc.h"
#include "AQLMersenneTwister.h"
#include "AQLFTQuasiRandGS.h"	// for XLL Plus
#include "AQLPriceLSMCTradeValue.h"
#include "AQLPolyFitLS.h"
#include "AQLPolynomialPS.h"
#include "AQLPricePortfolioValue.h"

#include "AQLRatesAggregateCouponCapFloor.h"
#include "AQLPriceCouponRainbowMin.h"
#include "AQLRatesInterSectionFunc.h"
#include "AQLRatesUnionFunc.h"
#include "AQLRatesCpnCapFloorFuncForTARN.h"
#include "AQLPriceCouponMax.h"
#include "AQLPriceCouponMin.h"
#include "AQLSumFunc2.h"
#include "AQLPriceCouponForDigital.h"
#include "AQLPriceCouponForDigital2.h"

#include "AQLPriceFXDisplacedDiffusionCalibration.h"
#include "AQLPriceFXDisplacedDiffusionCalibration3F.h"
#include "AQLPriceSZCalibration.h"
#include "AQLMathAntonovFXOption.h"
#include "AQLMathAntonovFXOptionBetaFixed.h"
#include "AQLMathAntonovFXOptionVolatilityFixed.h"
#include "AQLPriceHWCalibration.h"
#include "AQLMathJamshidianSwaption.h"
#include "AQLMathJamshidianSwaptionByImplyVol.h"
#include "AQLPriceLMMCalibration.h"
#include "AQLLinearRatesSwapTradeValueForExo.h"
#include "AQLPriceCashValue.h"

#include "AQLDefinitions.h"
#include "AQLCoreDataService.h"
#include "AQLStaticDataManager.h"
#include "AQLStaticData.h"

//====================AQLModelSetupBase==============================
// constructor
/*!

*/
AQLModelSetupBase::AQLModelSetupBase()
{
}

// destructor
/*!

*/
AQLModelSetupBase::~AQLModelSetupBase(void)
{
}

// 
/*!
    @brief regist master data to dataInstance object

	@param[in,out] dataInstance AQLDataInstance &	
	@return void 
*/
void
AQLModelSetupBase::registMaster(AQLDataInstance &dataInstance)
{
	//data regist
	AQLPriceDataManager &dm = dataInstance.getDataMaster();
	registAttrMaster(dm);
	registAttrMasterEx(dm);

	//object regist
	registEntityMaster(dataInstance);
	registEntityMasterEx(dataInstance);

	//function regist
	registFunctionMaster(dataInstance);
	registFunctionMasterEx(dataInstance);
}

// 
/*!
    @brief regist data data to attributemaster object

	@param[in,out] dm AQLPriceDataManager &	
	@return void 
*/
void
AQLModelSetupBase::registAttrMaster(AQLPriceDataManager &dm)
{

	dm.setData(new AQLDataBool());
	dm.setData(new AQLDataString());
	dm.setData(new AQLDataInt());
	dm.setData(new AQLDataDouble());
	dm.setData(new AQLDataDate());
	dm.setData(new AQLDataDateTime());
	dm.setData(new AQLDataBools());
	dm.setData(new AQLDataStrings());
	dm.setData(new AQLDataInts());
	dm.setData(new AQLDataDoubles());
	dm.setData(new AQLDataDates());
	dm.setData(new AQLDataDateTimes());
	dm.setData(new AQLDataDoubleMatrix());
	dm.setData(new AQLDataReference());
	dm.setData(new AQLDataMultiReference());
	dm.setData(new AQLDataValuation());
	dm.setData(new AQLDataProcedure());
	dm.setData(new AQLPriceDataCalendar());
	dm.setData(new AQLPriceDataDayCount());
	dm.setData(new AQLPriceDataConvention());
	dm.setData(new AQLPriceDataSlidingRule());
	dm.setData(new AQLPriceDataInterpolation());
	dm.setData(new AQLPriceDataRand());
	dm.setData(new AQLPriceDataFunction());	
	dm.setData(new AQLPriceDataFunctions());	
	dm.setData(new AQLMathAttrSDE());

	// calibration result
	dm.setData("ResultVolatility", DATA_DOUBLES);
	dm.setData("ResultBeta", DATA_DOUBLES);
	dm.setData("ResultConvergenceValue", DATA_DOUBLES);
	dm.setData("ResultMaturity", DATA_STRINGS);
	// for digital
	const AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString maxLegStr = staticData.getStaticData(KEY_DEAL_LEG_MAXNUM);
	const int maxLegNum = maxLegStr.getIntValue();
	for (int i = 0; i < maxLegNum; ++i)
	{
		dm.setData("CouponInfos" + AQLString(i + 1), DATA_MULTIREFERENCE);
	}
}

// 
/*!
    @brief regist object data to entitymaster object

	@param[in,out] dataInstance AQLDataInstance &
	@return void 
*/
void
AQLModelSetupBase::registEntityMaster(AQLDataInstance &dataInstance)
{
	AQLObjectMaster &em = dataInstance.getObjectMaster();
	em.setEntity(new AQLMathObjectValue(&dataInstance));
	em.setEntity(new AQLMathPathEntity(&dataInstance));
	em.setEntity(new AQLMathIndexEntity(&dataInstance));
	em.setEntity(new AQLMathFXEntity(&dataInstance));
	em.setEntity(new AQLMathYieldCurve(&dataInstance));
	em.setEntity(new AQLMathCorrelation(&dataInstance));
	em.setEntity(new AQLMathVolatility(&dataInstance));
}

// 
/*!
    @brief regist function data to functionmaster object

	@param[in,out] dataInstance AQLDataInstance &
	@return void 
*/
void
AQLModelSetupBase::registFunctionMaster(AQLDataInstance &dataInstance)
{
	AQLPriceDataManager &dm = dataInstance.getDataMaster();
	AQLFunctionManager &fm = dataInstance.getFunctionMaster();

	//base method
	fm.setFunction(new AQLConstant(),										FN_CONSTANT_STR);
	fm.setFunction(new AQLLinearMethod(),									FN_LINEAR_STR);
	fm.setFunction(new AQLQuadraticMethod(),									FN_QUADRATIC_STR);
	fm.setFunction(new AQLMaxMethod(),										FN_MAX_STR);
	fm.setFunction(new AQLMinMethod(),										FN_MIN_STR);
	fm.setFunction(new AQLSumMethod(),										FN_SUM_STR);
	fm.setFunction(new AQLDivideMethod(),									FN_DIVIDE_STR);
	fm.setFunction(new AQLFractionMethod(),									FN_FRACTION_STR);
	fm.setFunction(new AQLAverageMethod(),									FN_AVERAGE_STR);
	fm.setFunction(new AQLDigitalMethod(),									FN_DIGITAL_STR);

	// Interpolation
	fm.setFunction(new AQLLinearInterpolation(LINEAR_EXTRAPOLATION_TYPE),	FN_LINEARINTERPOLATION_STR);
	fm.setFunction(new AQLSplineInterpolation(),								FN_SPLINEINTERPOLATION_STR);
    fm.setFunction(new AQLSplineInterpolation(true),							FN_NATURALSPLINEINTERPOLATION_STR);     // UseNaturalSpline = true ( default )
    fm.setFunction(new AQLSplineInterpolation(false),						FN_CLAMPEDSPLINEINTERPOLATION_STR);     // UseNaturalSpline = false i.e. Clamped Spline
	fm.setFunction(new AQLMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::FRITSCH_BUTLAND),		FN_MONOTONESPLINEINTERPOLATION_STR);
	fm.setFunction(new AQLMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	FN_MONOTONEPARABOLICINTERPOLATION_STR);
	fm.setFunction(new AQLMonotoneSplineInterpolation(),						FN_MONOTONESPLINEINTERPOLATION_STR);
    fm.setFunction(new AQLParabolicInterpolation(),							FN_PARABOLICINTERPOLATION_STR);
    fm.setFunction(new AQLLinearSplineInterpolation(),						FN_LINEARSPLINEINTERPOLATION_STR);
	fm.setFunction(new AQLLinearMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::FRITSCH_BUTLAND),		FN_LINEARMONOTONESPLINEINTERPOLATION_STR);
	fm.setFunction(new AQLLinearMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	FN_LINEARMONOTONEPARABOLICINTERPOLATION_STR);
	fm.setFunction(new AQLConstrainedSplineInterpolation(),					FN_CONSTRAINEDSPLINEINTERPOLATION_STR);
	fm.setFunction(new AQLStepInterpolation(),								FN_STEPINTERPOLATION_STR);
	fm.setFunction(new AQLStepInterpolation(StepType::RIGHT_CONTINUOUS),		FN_RIGHTCONTINUOUS_INTERPOLATION_STR);
	fm.setFunction(new AQLStepInterpolation(StepType::LEFT_CONTINUOUS),		FN_LEFTCONTINUOUS_INTERPOLATION_STR);

	// Interpolation Alias Names
	fm.setFunction(new AQLLinearInterpolation(LINEAR_EXTRAPOLATION_TYPE),	LINEAR_INTERP);
	fm.setFunction(new AQLSplineInterpolation(),								SPLINE_INTERP);
    fm.setFunction(new AQLSplineInterpolation(true),							NATURAL_SPLINE_INTERP);     // UseNaturalSpline = true ( default )
    fm.setFunction(new AQLSplineInterpolation(false),						CLAMPED_SPLINE_INTERP);     // UseNaturalSpline = false i.e. Clamped Spline
	fm.setFunction(new AQLMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::FRITSCH_BUTLAND),		MONOTONE_SPLINE_INTERP);
	fm.setFunction(new AQLMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	MONOTONE_PARABOLIC_INTERP);
	fm.setFunction(new AQLMonotoneSplineInterpolation(),						MONOTONE_SPLINE_INTERP);
    fm.setFunction(new AQLParabolicInterpolation(),							PARABOLIC_INTERP);
    fm.setFunction(new AQLLinearSplineInterpolation(),						LINEAR_SPLINE_INTERP);
	fm.setFunction(new AQLLinearMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::FRITSCH_BUTLAND),		LINEAR_MONOTONE_SPLINE_INTERP);
	fm.setFunction(new AQLLinearMonotoneSplineInterpolation(AQLMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	LINEAR_MONOTONE_PARABOLIC_INTERP);
	fm.setFunction(new AQLConstrainedSplineInterpolation(),					CONSTRAINED_SPLINE_INTERP);
	fm.setFunction(new AQLStepInterpolation(),								STEP_INTERP);
	fm.setFunction(new AQLStepInterpolation(StepType::RIGHT_CONTINUOUS),		RIGHT_CONTINUOUS_INTERP);
	fm.setFunction(new AQLStepInterpolation(StepType::LEFT_CONTINUOUS),		LEFT_CONTINUOUS_INTERP);
	
    // Monotone Convex Parameters
	// -------------------------------------------
	const AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	const AQLString strLambda = staticData.getStaticData(KEY_MONOTONECONVEX_LAMBDA);
	const AQLString strIsAllowedNegative = staticData.getStaticData(KEY_MONOTONECONVEX_ISALLOWEDNEGATIVE);
	if (strLambda != AQ_NO_DATA && strIsAllowedNegative != AQ_NO_DATA)
	{
		AQLDataBool isAllowedNegative;
		isAllowedNegative.convertFromString(strIsAllowedNegative);
		const AQLString strInputMode = staticData.getStaticData(KEY_MONOTONECONVEX_INPUTMODE);
		const AQLString strOutputMode = staticData.getStaticData(KEY_MONOTONECONVEX_OUTPUTMODE);
		if (strInputMode != AQ_NO_DATA && strOutputMode != AQ_NO_DATA)
		{
			fm.setFunction(new AQLMonotoneConvexInterpolation(strLambda.getDoubleValue(), isAllowedNegative.get(),
				strInputMode.getIntValue(), strOutputMode.getIntValue()), FN_MONOTONECONVEXINTERPOLATION_STR);
		}
		else
		{
			fm.setFunction(new AQLMonotoneConvexInterpolation(strLambda.getDoubleValue(), isAllowedNegative.get()),
				FN_MONOTONECONVEXINTERPOLATION_STR);
		}
	}
	else
	{
		fm.setFunction(new AQLMonotoneConvexInterpolation(), FN_MONOTONECONVEXINTERPOLATION_STR);
	}
	// -------------------------------------------

	fm.setFunction(new AQLMathBasisFunction(), FN_BASISFUNC1_STR);
	fm.setFunction(new AQLMathBasisFunction2(), FN_BASISFUNC2_STR);

	fm.setFunction(new AQLRatesAggregateCouponCapFloor(), FN_AGGREGATECOUPONCAPFLOOR_STR);
	fm.setFunction(new AQLPriceCouponRainbowMin(), FN_CPNSLTOPERATORRAINBOWMIN_STR);
	fm.setFunction(new AQLRatesInterSectionFunc(), FN_INTERSECTIONFUNC_STR);
	fm.setFunction(new AQLRatesUnionFunc(), FN_UNIONFUNC_STR);
	fm.setFunction(new AQLRatesCpnCapFloorFuncForTARN(), FN_CPNCAPFLOORFORTARN_STR);
	fm.setFunction(new AQLPriceCouponMax(), FN_CPNSLTOPERATORMAX_STR);
	fm.setFunction(new AQLPriceCouponMin(), FN_CPNSLTOPERATORMIN_STR);	
	fm.setFunction(new AQLSumMethod2(), FN_SUM2_STR);
	fm.setFunction(new AQLPriceCouponForDigital(), FN_CPNSLTOPERATORFORDIGITAL_STR);
	fm.setFunction(new AQLPriceCouponForDigital2(), FN_CPNSLTOPERATORFORDIGITAL2_STR);
	fm.setFunction(new AQLCompoundMethod(), FN_COMPOUNDING_STR);
	fm.setFunction(new AQLCompoundMethod1(), FN_COMPOUNDING1_STR);
	fm.setFunction(new AQLCompoundMethod2(), FN_COMPOUNDING2_STR);
	fm.setFunction(new AQLCompoundMethod3(), FN_COMPOUNDING3_STR);
	fm.setFunction(new AQLCompoundMethod4(), FN_COMPOUNDING4_STR);
	fm.setFunction(new AQLCompoundMethod5(), FN_COMPOUNDING5_STR);
	fm.setFunction(new AQLCompoundMethod6(), FN_COMPOUNDING6_STR);
	fm.setFunction(new AQLCompoundMethod7(), FN_COMPOUNDING7_STR);
	fm.setFunction(new AQLCompoundMethod8(), FN_COMPOUNDING8_STR);
	fm.setFunction(new AQLCompoundMethod9(), FN_COMPOUNDING9_STR);
	fm.setFunction(new AQLCompoundMethod10(), FN_COMPOUNDING10_STR);

	//trade value
	AQLPriceTradeValue *tvalue = new AQLPriceTradeValue(new AQLPriceAccruedInterest());
	fm.setFunction(tvalue, FN_IR_TRADEVALUE_STR);		
	tvalue->registerData(dm);
	//lsmc trade
	AQLPriceLSMCTradeValue* lvalue = new AQLPriceLSMCTradeValue(new AQLPolyFitLS(), new AQLPriceAccruedInterest());
	fm.setFunction(lvalue, FN_IR_LSMCTRADEVALUE_STR);		
	lvalue->registerData(dm);

	//cf generator
	AQLPriceCashFlowGenerator *pcf = new AQLPriceCashFlowGenerator();
	pcf->registerData(dm);
	fm.setFunction(pcf, FN_IR_CASHFLOWGENERATOR_STR);

	AQLPriceYieldGenerator *ylg = new AQLPriceYieldGenerator();
	fm.setFunction(ylg, FN_IRYIELDGENERATOR_STR);

	AQLPriceArbFreeGenerator *ylg_arb = new AQLPriceArbFreeGenerator();
	fm.setFunction(ylg_arb, FN_IRARBFREEGENERATOR_STR);
	
	//pay off
	AQLPricePayOff *ppayoff = new AQLPricePayOff();
	ppayoff->registerData(dm);
	fm.setFunction(ppayoff, FN_IR_PAYOFF_STR);
	//cpnsltoperatorrainbow
	AQLPriceCouponRainbow *pslt = new AQLPriceCouponRainbow();
	pslt->registerData(dm);
	fm.setFunction(pslt, FN_CPNSLTOPERATORRAINBOW_STR);
	// portfolio
	AQLPricePortfolioValue *port = new AQLPricePortfolioValue();
	fm.setFunction(port, FN_IR_PORTFOLIOVALUE_STR);		
	port->registerData(dm);
	
	const unsigned long seedVal = static_cast<unsigned long>(staticData.getStaticData(KEY_SIMULATION_SEED).getDoubleValue());
	// MT
	UlongArray seed(1, seedVal);
	// is halley modification
	AQLDataBool tmp;
	tmp.convertFromString(staticData.getStaticData(KEY_SIMULATION_RAND_GAUSSIAN_ISHALLEYMODE));
	AQLMersenneTwister *pMT = new AQLMersenneTwister(tmp.get());
	fm.setFunction(pMT, FN_RAND_MT_STR);
	pMT->setSeed(seed);

// update for XLL Plus ////////////////////////////////////////////////////////////////////////////
	AQLFTQuasiRandGS *prandGS = new AQLFTQuasiRandGS();
	fm.setFunction(prandGS,FN_RAND_QUASIGS_STR);
	const unsigned long seedPathNum = static_cast<unsigned long>(staticData.getStaticData(KEY_SIMULATION_QUASIGS_RAND_PATHNUM).getDoubleValue());
	UlongArray quasiGSSeed(2,0);
	quasiGSSeed[0] = seedVal;
	quasiGSSeed[1] = seedPathNum;
	prandGS->setSeed(quasiGSSeed);
///////////////////////////////////////////////////////////////////////////////////////////////////

	AQLString baseDim = staticData.getStaticData(KEY_SIMULATION_LSMC_BASEFUNCDIM);
	// polynomial function
	fm.setFunction(new AQLPolynomialPS(baseDim.getIntValue()), FN_POLYNOMIALPS_STR); 

	/////////////////
	// calibration
	/////////////////
	//HW calibration
	AQLPriceHWCalibration *pcalibHW = new AQLPriceHWCalibration();
	pcalibHW->registerData(dm);
	fm.setFunction(pcalibHW,FN_IR_HWCALIBRATION_STR);

	//HW calibration imply vol
	AQLMathJamshidianSwaptionByImplyVol *pcalibJSBIV = new AQLMathJamshidianSwaptionByImplyVol();
	fm.setFunction(pcalibJSBIV, FN_JAMSHIDIANSWAPTIONBYIMPLYVOL_STR);

	//HW calibration engine
	AQLMathJamshidianSwaption *pcalibJS = new AQLMathJamshidianSwaption();
	pcalibJS->registerData(dm);
	fm.setFunction(pcalibJS, FN_JAMSHIDIANSWAPTION_STR);
	
	//calibrationFX
	AQLPriceFXDisplacedDiffusionCalibration *pcalibFXDD = new AQLPriceFXDisplacedDiffusionCalibration();
	pcalibFXDD->registerData(dm);
	fm.setFunction(pcalibFXDD, FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION_STR);

	//calibrationFX3F
	AQLPriceFXDisplacedDiffusionCalibration3F *pcalibFXDD3F = new AQLPriceFXDisplacedDiffusionCalibration3F();
	pcalibFXDD3F->registerData(dm);
	fm.setFunction(pcalibFXDD3F, FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION3F_STR);

	//calibrationFX(BothType)
	AQLMathAntonovFXOption *pcalibFXOpt = new AQLMathAntonovFXOption();
	pcalibFXOpt->registerData(dm);
	fm.setFunction(pcalibFXOpt, FN_ANTONOVFXOPTIOIN_STR);
	
	//calibrationFX(BetaFixed Type)
	AQLMathAntonovFXOptionBetaFixed *pcalibFXOptBeta = new AQLMathAntonovFXOptionBetaFixed();
	pcalibFXOptBeta->registerData(dm);
	fm.setFunction(pcalibFXOptBeta, FN_ANTONOVFXOPTIOINBETAFIXED_STR);
	
	//calibrationFX(VolatilityFixed Type)
	AQLMathAntonovFXOptionVolatilityFixed* pcalibFXOptVol = new AQLMathAntonovFXOptionVolatilityFixed();
	pcalibFXOptVol->registerData(dm);
	fm.setFunction(pcalibFXOptVol, FN_ANTONOVFXOPTIOINVOLATILITYFIXED_STR);

	//calibrationSZ
	AQLPriceSZCalibration *pcalibSZ = new AQLPriceSZCalibration();
	pcalibSZ->registerData(dm);
	fm.setFunction(pcalibSZ, FN_IR_SZCALIBRATION_STR);

	//LMM calibration
	AQLPriceLMMCalibration *pcalibLMM = new AQLPriceLMMCalibration();
	pcalibLMM->registerData(dm);
	fm.setFunction(pcalibLMM,FN_IR_LMMCALIBRATION_STR);

	//plainvanilla swap
	AQLLinearRatesSwapTradeValueForExo* pvanillatrade = new AQLLinearRatesSwapTradeValueForExo( new AQLPriceAccruedInterest());
	fm.setFunction(pvanillatrade, FN_IR_PLAINVANILLASWAPTRADEVALUEFOREXO_STR);
	pvanillatrade->registerData(dm);
	AQLPriceCashValue* pcash = new AQLPriceCashValue();
	fm.setFunction(pcash, FN_IR_CASHVALUE_STR);
	pcash->registerData(dm);
}
