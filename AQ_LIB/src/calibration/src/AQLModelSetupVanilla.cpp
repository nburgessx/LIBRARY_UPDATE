/*! @file
    @brief Class to regist Vanilla master data
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLModelSetupVanilla.cpp
//
//  DESCRIPTION :       Vanilla Master data regist class
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


#include "AQLModelSetupVanilla.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLFunctionManager.h"
#include "AQLDataBasics.h"
#include "AQLSobol.h"
#include "AQLDefinitions.h"
#include "AQLCoreDataService.h"
#include "AQLStaticDataManager.h"
#include "AQLStaticData.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLParabolicInterpolation.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"
#include "AQLMonotoneConvexInterpolation.h"
#include "AQLMonotoneSplineInterpolation.h"
#include "AQLConstrainedSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLPricePortfolioValue.h"
#include "AQLPriceYieldGenerator.h"
#include "AQLConstant.h"
#include "AQLLinearFunc.h"
#include "AQLQuadraticFunc.h"
#include "AQLDivideFunc.h"
#include "AQLMaxFunc.h"
#include "AQLMinFunc.h"
#include "AQLSumFunc.h"
#include "AQLFractionFunc.h"
#include "AQLLinearRatesOptionValue.h"
#include "AQLPriceFXOptionValue.h"
#include "AQLMathPlainVanillaEntity.h"
#include "AQLPriceFXDigitalOptionValue.h"
#include "AQLPriceFXDigitalCallSpreadOptionValue.h"
#include "AQLPriceFXKnockoutRebateValue.h"
#include "AQLPriceFXSingleBarrierOptionValue.h"
#include "AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue.h"
#include "AQLLinearRatesOptionValueDataProvider.h"
#include "AQLPriceCashFlowGenerator.h"
#include "AQLPriceCouponRainbow.h"
#include "AQLPricePayOff.h"
#include "AQLPriceConvergenceValue.h"
#include "AQLPriceCashValue.h"
#include "AQLLinearRatesSwapTradeValue.h"
#include "AQLRatesAggregateCouponCapFloor.h"
#include "AQLRatesCpnCapFloorFuncForTARN.h"
#include "AQLRatesInterSectionFunc.h"
#include "AQLRatesUnionFunc.h"
#include "AQLSumFunc2.h"
#include "AQLPriceCouponForDigital.h"
#include "AQLPriceCouponForDigital2.h"
#include "AQLPriceCouponMax.h"
#include "AQLPriceCouponMin.h"
#include "AQLPriceCouponRainbowMin.h"
#include "AQLPriceAccruedInterest.h"
#include "AQLBlackScholesBaseFunc.h"
#include "AQLPriceIRCapFloorOptionValue.h"
#include "AQLBlackScholesCapletOptionPayoff.h"
#include "AQLBlackScholesFloorletOptionPayoff.h"
#include "AQLBlackScholesCollarOptionPayoff.h"
#include "AQLBlackScholesStrangleOptionPayoff.h"
#include "AQLBlackScholesStraddleOptionPayoff.h"
#include "AQLBlackScholesCapletSpreadOptionPayoff.h"
#include "AQLBlackScholesFloorletSpreadOptionPayoff.h"
#include "AQLBlackScholesDelayedCapletOptionPayoff.h"
#include "AQLBlackScholesDelayedFloorletOptionPayoff.h"
#include "AQLBlackScholesDigitalCapletOptionPayoff.h"
#include "AQLBlackScholesDigitalFloorletOptionPayoff.h"
#include "AQLPriceIRSwaptionValue.h"
#include "AQLPriceIRSwaptionValueFromCashFlow.h"
#include "AQLPriceArbFreeGenerator.h"
#include "AQLCompoundingFunc.h"
#include "AQLPriceOptionGenerator.h"
//#include "LAPriceNDSSwaptionValue.h"
#include "AQLPolynomialPS.h"

//====================AQLModelSetupVanilla ==============================

// constructor
/*!

*/
AQLModelSetupVanilla::AQLModelSetupVanilla(void) : AQLModelSetupBase()
{
}

// destructor
/*!

*/
AQLModelSetupVanilla::~AQLModelSetupVanilla(void)
{
}

// 
/*!
    @brief regist function data to functionmaster object

	@param[in,out] dataInstance AQLDataInstance &
	@return void 
*/
void
AQLModelSetupVanilla::registFunctionMaster(AQLDataInstance &dataInstance)
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
	
	// Interpolation Methods
	fm.setFunction(new AQLLinearInterpolation(LINEAR_EXTRAPOLATION_TYPE),	FN_LINEARINTERPOLATION_STR);
	fm.setFunction(new AQLSplineInterpolation(),								FN_SPLINEINTERPOLATION_STR);
    fm.setFunction(new AQLSplineInterpolation(true),							FN_NATURALSPLINEINTERPOLATION_STR);     // UseNaturalSpline = true ( default )
    fm.setFunction(new AQLSplineInterpolation(false),						FN_CLAMPEDSPLINEINTERPOLATION_STR);    // UseNaturalSpline = false => Clamped Spline
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
    fm.setFunction(new AQLSplineInterpolation(false),						CLAMPED_SPLINE_INTERP);    // UseNaturalSpline = false => Clamped Spline
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
	// -------------------------------------------------
	const AQLStaticData &staticData			= AQLCoreDataService::getStaticDataManager().getStaticData();
	const AQLString strLambda				= staticData.getStaticData(KEY_MONOTONECONVEX_LAMBDA);
	const AQLString strIsAllowedNegative		= staticData.getStaticData(KEY_MONOTONECONVEX_ISALLOWEDNEGATIVE);

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
	// -------------------------------------------------

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

	//cf generator
	AQLPriceCashFlowGenerator *pcf = new AQLPriceCashFlowGenerator();
	pcf->registerData(dm);
	fm.setFunction(pcf, FN_IR_CASHFLOWGENERATOR_STR);

	//pay off
	AQLPricePayOff *ppayoff = new AQLPricePayOff();
	ppayoff->registerData(dm);
	fm.setFunction(ppayoff, FN_IR_PAYOFF_STR);
	//cpnsltoperatorrainbow
	AQLPriceCouponRainbow *pslt = new AQLPriceCouponRainbow();
	pslt->registerData(dm);
	fm.setFunction(pslt, FN_CPNSLTOPERATORRAINBOW_STR);
	
	AQLPriceYieldGenerator *ylg = new AQLPriceYieldGenerator();
	fm.setFunction(ylg, FN_IRYIELDGENERATOR_STR);

	AQLPriceArbFreeGenerator *ylg_arb = new AQLPriceArbFreeGenerator();
	fm.setFunction(ylg_arb, FN_IRARBFREEGENERATOR_STR);
	
	// portfolio
	AQLPricePortfolioValue *port = new AQLPricePortfolioValue();
	fm.setFunction(port, FN_IR_PORTFOLIOVALUE_STR);		
	port->registerData(dm);

	//fxoption
	AQLPriceFXOptionValue* pfxop = new AQLPriceFXOptionValue();
	fm.setFunction(pfxop, FN_FXOPTIONVALUE_STR);
	pfxop->registerData(dm);

	//fxdigitaloption
	AQLPriceFXDigitalOptionValue* pfxdig = new AQLPriceFXDigitalOptionValue();
	fm.setFunction(pfxdig, FN_FXDIGITALOPTIONVALUE_STR);
	pfxdig->registerData(dm);
	
	//fxdigitalcallspread
	AQLPriceFXDigitalCallSpreadOptionValue* pfxdigcallspread = new AQLPriceFXDigitalCallSpreadOptionValue();
	fm.setFunction(pfxdigcallspread, FN_FXDIGITALCALLSPREADOPTIONVALUE_STR);
	pfxdigcallspread->registerData(dm);

	//fxknockout
	AQLPriceFXKnockoutRebateValue* pfxknockout = new AQLPriceFXKnockoutRebateValue();
	fm.setFunction(pfxknockout, FN_FXKNOCKOUTREBATEVALUE_STR);
	pfxknockout->registerData(dm);

	//fxsinglebarrier
	AQLPriceFXSingleBarrierOptionValue* pfxsingle = new AQLPriceFXSingleBarrierOptionValue();
	fm.setFunction(pfxsingle, FN_FXSINGLEBARRIEROPTIONVALUE_STR);
	pfxsingle->registerData(dm);

	//fxdigitalsinglebarrier
	AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue* pfxdigsingle = new AQLPriceFXDigitalCallSpreadSingleBarrierOptionValue();
	fm.setFunction(pfxdigsingle, FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE_STR);
	pfxdigsingle->registerData(dm);

	//plainvanilla swap
	//AQLLinearRatesSwapTradeValue* pvanillatrade = new AQLLinearRatesSwapTradeValue();
	AQLLinearRatesSwapTradeValue* pvanillatrade = new AQLLinearRatesSwapTradeValue( new AQLPriceAccruedInterest());
	fm.setFunction(pvanillatrade, FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);
	pvanillatrade->registerData(dm);
	AQLPriceCashValue* pcash = new AQLPriceCashValue();
	fm.setFunction(pcash, FN_IR_CASHVALUE_STR);
	pcash->registerData(dm);
	
	
	//convergence
	AQLPriceConvergenceValue* pconvergece = new AQLPriceConvergenceValue();
	fm.setFunction(pconvergece, FN_IR_CONVERGENCEVALUE_STR);
	pconvergece->registerData(dm);

	//capfloor
	AQLPriceIRCapFloorOptionValue* pcapfloor = new AQLPriceIRCapFloorOptionValue();
	fm.setFunction(pcapfloor, FN_IR_CAPFLOOROPTIONVALUE_STR);
	pcapfloor->registerData(dm);

	//swaption
	AQLPriceIRSwaptionValue* pswaption = new AQLPriceIRSwaptionValue();
	pswaption->registerData(dm);
	fm.setFunction(pswaption, FN_IR_SWAPTIONVALUE_STR);

	AQLPriceIRSwaptionValueFromCashFlow* pswaption2 = new AQLPriceIRSwaptionValueFromCashFlow();
	fm.setFunction(pswaption2, FN_IR_SWAPTIONVALUEFROMCASHFLOW_STR);
//#ifndef RH6
//	LAPriceNDSSwaptionValue* pndsswaption = new LAPriceNDSSwaptionValue();
//	fm.setFunction(pndsswaption, FN_IR_NDSSWAPTIONVALUE_STR);
//#endif
	fm.setFunction(new AQLPolynomialPS(1), FN_POLYNOMIALPS_STR);
	
	AQLBlackScholesBaseMethod* pbsbasef = new AQLBlackScholesBaseMethod();
	fm.setFunction(pbsbasef, FN_BSBASEFUNC_STR);

	AQLBlackScholesCapletOption* pbscaplet = new AQLBlackScholesCapletOption();
	fm.setFunction(pbscaplet, FN_IR_CAPLETOPTIONFUNC_STR);

	AQLBlackScholesFloorletOption* pbsfloorlet = new AQLBlackScholesFloorletOption();
	fm.setFunction(pbsfloorlet, FN_IR_FLOORLETOPTIONFUNC_STR);
	
	AQLBlackScholesCollarOption* pbscollar = new AQLBlackScholesCollarOption();
	fm.setFunction(pbscollar, FN_IR_COLLAROPTIONFUNC_STR);
	
	AQLBlackScholesStrangleOption* pbsstrangle = new AQLBlackScholesStrangleOption();
	fm.setFunction(pbsstrangle, FN_IR_STRANGLEOPTIONFUNC_STR);

	AQLBlackScholesStraddleOption* pbsstraddle = new AQLBlackScholesStraddleOption();
	fm.setFunction(pbsstraddle, FN_IR_STRADDLEOPTIONFUNC_STR);

	AQLBlackScholesCapletSpreadOption* pbscapletspread = new AQLBlackScholesCapletSpreadOption();
	fm.setFunction(pbscapletspread, FN_IR_CAPLETSPREADOPTIONFUNC_STR);

	AQLBlackScholesFloorletSpreadOption* pbsfloorletspread = new AQLBlackScholesFloorletSpreadOption();
	fm.setFunction(pbsfloorletspread, FN_IR_FLOORLETSPREADOPTIONFUNC_STR);

	AQLBlackScholesDelayedCapletOption* pbsdelayedcaplet = new AQLBlackScholesDelayedCapletOption();
	fm.setFunction(pbsdelayedcaplet, FN_IR_DELAYEDCAPLETOPTIONFUNC_STR);

	AQLBlackSholesDelayedFloorletOption* pbsdelayedfloorlet = new AQLBlackSholesDelayedFloorletOption();
	fm.setFunction(pbsdelayedfloorlet, FN_IR_DELAYEDFLOORLETOPTIONFUNC_STR);

	AQLBlackScholesDigitalCapletOption* pbsdigitalcaplet = new AQLBlackScholesDigitalCapletOption();
	fm.setFunction(pbsdigitalcaplet, FN_IR_DIGITALCAPLETOPTIONFUNC_STR);

	AQLBlackShcolesDigitalFloorletOption* pbsdigitalfloorlet = new AQLBlackShcolesDigitalFloorletOption();
	fm.setFunction(pbsdigitalfloorlet, FN_IR_DIGITALFLOORLETOPTIONFUNC_STR);

	AQLPriceOptionGenerator* popgen = new AQLPriceOptionGenerator();
	fm.setFunction(popgen, FN_IR_OPTIONGENERATOR_STR);
	popgen->registerData(dm);

}

// 
/*!
    @brief regist extra data data to attributemaster object

	@param[in,out] dm AQLPriceDataManager &	
	@return void 
*/
void
AQLModelSetupVanilla::registAttrMasterEx(AQLPriceDataManager &dm)
{
}


// 
/*!
    @brief regist extra object data to entitymaster object

	@param[in,out] dataInstance AQLDataInstance &	
	@return void 
*/
void
AQLModelSetupVanilla::registEntityMasterEx(AQLDataInstance &dataInstance)
{
	AQLObjectMaster &em = dataInstance.getObjectMaster();
	em.setEntity(new AQLMathPlainVanillaEntity(&dataInstance));
}

// 
/*!
    @brief regist extra function data to functionmaster object

	@param[in,out] dataInstance AQLDataInstance &
	@return void 
*/
void
AQLModelSetupVanilla::registFunctionMasterEx(AQLDataInstance &dataInstance)
{
	dataInstance;
}




