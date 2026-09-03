/*! @file
    @brief Class to regist Vanilla master data
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAModelSetupVanilla.cpp
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


#include "LAModelSetupVanilla.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAFunctionManager.h"
#include "LADataBasics.h"
#include "LASobol.h"
#include "LADefinitions.h"
#include "LACoreDataService.h"
#include "LAStaticDataManager.h"
#include "LAStaticData.h"
#include "LAMathYieldCurvePro.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAParabolicInterpolation.h"
#include "LALinearSplineInterpolation.h"
#include "LALinearMonotoneSplineInterpolation.h"
#include "LAMonotoneConvexInterpolation.h"
#include "LAMonotoneSplineInterpolation.h"
#include "LAConstrainedSplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAPricePortfolioValue.h"
#include "LAPriceYieldGenerator.h"
#include "LAConstant.h"
#include "LALinearFunc.h"
#include "LAQuadraticFunc.h"
#include "LADivideFunc.h"
#include "LAMaxFunc.h"
#include "LAMinFunc.h"
#include "LASumFunc.h"
#include "LAFractionFunc.h"
#include "LALinearRatesOptionValue.h"
#include "LAPriceFXOptionValue.h"
#include "LAMathPlainVanillaEntity.h"
#include "LAPriceFXDigitalOptionValue.h"
#include "LAPriceFXDigitalCallSpreadOptionValue.h"
#include "LAPriceFXKnockoutRebateValue.h"
#include "LAPriceFXSingleBarrierOptionValue.h"
#include "LAPriceFXDigitalCallSpreadSingleBarrierOptionValue.h"
#include "LALinearRatesOptionValueDataProvider.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAPriceCouponRainbow.h"
#include "LAPricePayOff.h"
#include "LAPriceConvergenceValue.h"
#include "LAPriceCashValue.h"
#include "LALinearRatesSwapTradeValue.h"
#include "LARatesAggregateCouponCapFloor.h"
#include "LARatesCpnCapFloorFuncForTARN.h"
#include "LARatesInterSectionFunc.h"
#include "LARatesUnionFunc.h"
#include "LASumFunc2.h"
#include "LAPriceCouponForDigital.h"
#include "LAPriceCouponForDigital2.h"
#include "LAPriceCouponMax.h"
#include "LAPriceCouponMin.h"
#include "LAPriceCouponRainbowMin.h"
#include "LAPriceAccruedInterest.h"
#include "LABlackScholesBaseFunc.h"
#include "LAPriceIRCapFloorOptionValue.h"
#include "LABlackScholesCapletOptionPayoff.h"
#include "LABlackScholesFloorletOptionPayoff.h"
#include "LABlackScholesCollarOptionPayoff.h"
#include "LABlackScholesStrangleOptionPayoff.h"
#include "LABlackScholesStraddleOptionPayoff.h"
#include "LABlackScholesCapletSpreadOptionPayoff.h"
#include "LABlackScholesFloorletSpreadOptionPayoff.h"
#include "LABlackScholesDelayedCapletOptionPayoff.h"
#include "LABlackScholesDelayedFloorletOptionPayoff.h"
#include "LABlackScholesDigitalCapletOptionPayoff.h"
#include "LABlackScholesDigitalFloorletOptionPayoff.h"
#include "LAPriceIRSwaptionValue.h"
#include "LAPriceIRSwaptionValueFromCashFlow.h"
#include "LAPriceArbFreeGenerator.h"
#include "LACompoundingFunc.h"
#include "LAPriceOptionGenerator.h"
//#include "LAPriceNDSSwaptionValue.h"
#include "LAPolynomialPS.h"

//====================LAModelSetupVanilla ==============================

// constructor
/*!

*/
LAModelSetupVanilla::LAModelSetupVanilla(void) : LAModelSetupBase()
{
}

// destructor
/*!

*/
LAModelSetupVanilla::~LAModelSetupVanilla(void)
{
}

// 
/*!
    @brief regist function data to functionmaster object

	@param[in,out] dataInstance LADataInstance &
	@return void 
*/
void
LAModelSetupVanilla::registFunctionMaster(LADataInstance &dataInstance)
{
	LAPriceDataManager &dm = dataInstance.getDataMaster();
	LAFunctionManager &fm = dataInstance.getFunctionMaster();

	//base method
	fm.setFunction(new LAConstant(),										FN_CONSTANT_STR);
	fm.setFunction(new LALinearMethod(),									FN_LINEAR_STR);
	fm.setFunction(new LAQuadraticMethod(),									FN_QUADRATIC_STR);
	fm.setFunction(new LAMaxMethod(),										FN_MAX_STR);
	fm.setFunction(new LAMinMethod(),										FN_MIN_STR);
	fm.setFunction(new LASumMethod(),										FN_SUM_STR);
	fm.setFunction(new LADivideMethod(),									FN_DIVIDE_STR);
	fm.setFunction(new LAFractionMethod(),									FN_FRACTION_STR);
	
	// Interpolation Methods
	fm.setFunction(new LALinearInterpolation(LINEAR_EXTRAPOLATION_TYPE),	FN_LINEARINTERPOLATION_STR);
	fm.setFunction(new LASplineInterpolation(),								FN_SPLINEINTERPOLATION_STR);
    fm.setFunction(new LASplineInterpolation(true),							FN_NATURALSPLINEINTERPOLATION_STR);     // UseNaturalSpline = true ( default )
    fm.setFunction(new LASplineInterpolation(false),						FN_CLAMPEDSPLINEINTERPOLATION_STR);    // UseNaturalSpline = false => Clamped Spline
	fm.setFunction(new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND),		FN_MONOTONESPLINEINTERPOLATION_STR);
	fm.setFunction(new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	FN_MONOTONEPARABOLICINTERPOLATION_STR);
    fm.setFunction(new LAMonotoneSplineInterpolation(),						FN_MONOTONESPLINEINTERPOLATION_STR);
	fm.setFunction(new LAParabolicInterpolation(),							FN_PARABOLICINTERPOLATION_STR);
    fm.setFunction(new LALinearSplineInterpolation(),						FN_LINEARSPLINEINTERPOLATION_STR);
	fm.setFunction(new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND),		FN_LINEARMONOTONESPLINEINTERPOLATION_STR);
	fm.setFunction(new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	FN_LINEARMONOTONEPARABOLICINTERPOLATION_STR);
	fm.setFunction(new LAConstrainedSplineInterpolation(),					FN_CONSTRAINEDSPLINEINTERPOLATION_STR);
	fm.setFunction(new LAStepInterpolation(),								FN_STEPINTERPOLATION_STR);
	fm.setFunction(new LAStepInterpolation(StepType::RIGHT_CONTINUOUS),		FN_RIGHTCONTINUOUS_INTERPOLATION_STR);
	fm.setFunction(new LAStepInterpolation(StepType::LEFT_CONTINUOUS),		FN_LEFTCONTINUOUS_INTERPOLATION_STR);

	// Interpolation Alias Names
	fm.setFunction(new LALinearInterpolation(LINEAR_EXTRAPOLATION_TYPE),	LINEAR_INTERP);
	fm.setFunction(new LASplineInterpolation(),								SPLINE_INTERP);
    fm.setFunction(new LASplineInterpolation(true),							NATURAL_SPLINE_INTERP);     // UseNaturalSpline = true ( default )
    fm.setFunction(new LASplineInterpolation(false),						CLAMPED_SPLINE_INTERP);    // UseNaturalSpline = false => Clamped Spline
	fm.setFunction(new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND),		MONOTONE_SPLINE_INTERP);
	fm.setFunction(new LAMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	MONOTONE_PARABOLIC_INTERP);
    fm.setFunction(new LAMonotoneSplineInterpolation(),						MONOTONE_SPLINE_INTERP);
	fm.setFunction(new LAParabolicInterpolation(),							PARABOLIC_INTERP);
    fm.setFunction(new LALinearSplineInterpolation(),						LINEAR_SPLINE_INTERP);
	fm.setFunction(new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::FRITSCH_BUTLAND),		LINEAR_MONOTONE_SPLINE_INTERP);
	fm.setFunction(new LALinearMonotoneSplineInterpolation(LAMonotoneSplineInterpolation::MONOTONE_PARABOLIC),	LINEAR_MONOTONE_PARABOLIC_INTERP);
	fm.setFunction(new LAConstrainedSplineInterpolation(),					CONSTRAINED_SPLINE_INTERP);
	fm.setFunction(new LAStepInterpolation(),								STEP_INTERP);
	fm.setFunction(new LAStepInterpolation(StepType::RIGHT_CONTINUOUS),		RIGHT_CONTINUOUS_INTERP);
	fm.setFunction(new LAStepInterpolation(StepType::LEFT_CONTINUOUS),		LEFT_CONTINUOUS_INTERP);

	// Monotone Convex Parameters
	// -------------------------------------------------
	const LAStaticData &staticData			= LACoreDataService::getStaticDataManager().getStaticData();
	const LAString strLambda				= staticData.getStaticData(KEY_MONOTONECONVEX_LAMBDA);
	const LAString strIsAllowedNegative		= staticData.getStaticData(KEY_MONOTONECONVEX_ISALLOWEDNEGATIVE);

	if (strLambda != AQ_NO_DATA && strIsAllowedNegative != AQ_NO_DATA)
	{
		LADataBool isAllowedNegative;
		isAllowedNegative.convertFromString(strIsAllowedNegative);
		const LAString strInputMode = staticData.getStaticData(KEY_MONOTONECONVEX_INPUTMODE);
		const LAString strOutputMode = staticData.getStaticData(KEY_MONOTONECONVEX_OUTPUTMODE);
		if (strInputMode != AQ_NO_DATA && strOutputMode != AQ_NO_DATA)
		{
			fm.setFunction(new LAMonotoneConvexInterpolation(strLambda.getDoubleValue(), isAllowedNegative.get(),
				strInputMode.getIntValue(), strOutputMode.getIntValue()), FN_MONOTONECONVEXINTERPOLATION_STR);
		}
		else
		{
			fm.setFunction(new LAMonotoneConvexInterpolation(strLambda.getDoubleValue(), isAllowedNegative.get()),
				FN_MONOTONECONVEXINTERPOLATION_STR);
		}
	}
	else
	{
		fm.setFunction(new LAMonotoneConvexInterpolation(), FN_MONOTONECONVEXINTERPOLATION_STR);
	}
	// -------------------------------------------------

	fm.setFunction(new LAMathBasisFunction(), FN_BASISFUNC1_STR);
	fm.setFunction(new LAMathBasisFunction2(), FN_BASISFUNC2_STR);
	fm.setFunction(new LARatesAggregateCouponCapFloor(), FN_AGGREGATECOUPONCAPFLOOR_STR);
	fm.setFunction(new LAPriceCouponRainbowMin(), FN_CPNSLTOPERATORRAINBOWMIN_STR);
	fm.setFunction(new LARatesInterSectionFunc(), FN_INTERSECTIONFUNC_STR);
	fm.setFunction(new LARatesUnionFunc(), FN_UNIONFUNC_STR);
	fm.setFunction(new LARatesCpnCapFloorFuncForTARN(), FN_CPNCAPFLOORFORTARN_STR);
	fm.setFunction(new LAPriceCouponMax(), FN_CPNSLTOPERATORMAX_STR);
	fm.setFunction(new LAPriceCouponMin(), FN_CPNSLTOPERATORMIN_STR);	
	fm.setFunction(new LASumMethod2(), FN_SUM2_STR);
	fm.setFunction(new LAPriceCouponForDigital(), FN_CPNSLTOPERATORFORDIGITAL_STR);
	fm.setFunction(new LAPriceCouponForDigital2(), FN_CPNSLTOPERATORFORDIGITAL2_STR);
	fm.setFunction(new LACompoundMethod(), FN_COMPOUNDING_STR);
	fm.setFunction(new LACompoundMethod1(), FN_COMPOUNDING1_STR);
	fm.setFunction(new LACompoundMethod2(), FN_COMPOUNDING2_STR);
	fm.setFunction(new LACompoundMethod3(), FN_COMPOUNDING3_STR);
	fm.setFunction(new LACompoundMethod4(), FN_COMPOUNDING4_STR);
	fm.setFunction(new LACompoundMethod5(), FN_COMPOUNDING5_STR);
	fm.setFunction(new LACompoundMethod6(), FN_COMPOUNDING6_STR);
	fm.setFunction(new LACompoundMethod7(), FN_COMPOUNDING7_STR);
	fm.setFunction(new LACompoundMethod8(), FN_COMPOUNDING8_STR);
	fm.setFunction(new LACompoundMethod9(), FN_COMPOUNDING9_STR);
	fm.setFunction(new LACompoundMethod10(), FN_COMPOUNDING10_STR);

	//cf generator
	LAPriceCashFlowGenerator *pcf = new LAPriceCashFlowGenerator();
	pcf->registerData(dm);
	fm.setFunction(pcf, FN_IR_CASHFLOWGENERATOR_STR);

	//pay off
	LAPricePayOff *ppayoff = new LAPricePayOff();
	ppayoff->registerData(dm);
	fm.setFunction(ppayoff, FN_IR_PAYOFF_STR);
	//cpnsltoperatorrainbow
	LAPriceCouponRainbow *pslt = new LAPriceCouponRainbow();
	pslt->registerData(dm);
	fm.setFunction(pslt, FN_CPNSLTOPERATORRAINBOW_STR);
	
	LAPriceYieldGenerator *ylg = new LAPriceYieldGenerator();
	fm.setFunction(ylg, FN_IRYIELDGENERATOR_STR);

	LAPriceArbFreeGenerator *ylg_arb = new LAPriceArbFreeGenerator();
	fm.setFunction(ylg_arb, FN_IRARBFREEGENERATOR_STR);
	
	// portfolio
	LAPricePortfolioValue *port = new LAPricePortfolioValue();
	fm.setFunction(port, FN_IR_PORTFOLIOVALUE_STR);		
	port->registerData(dm);

	//fxoption
	LAPriceFXOptionValue* pfxop = new LAPriceFXOptionValue();
	fm.setFunction(pfxop, FN_FXOPTIONVALUE_STR);
	pfxop->registerData(dm);

	//fxdigitaloption
	LAPriceFXDigitalOptionValue* pfxdig = new LAPriceFXDigitalOptionValue();
	fm.setFunction(pfxdig, FN_FXDIGITALOPTIONVALUE_STR);
	pfxdig->registerData(dm);
	
	//fxdigitalcallspread
	LAPriceFXDigitalCallSpreadOptionValue* pfxdigcallspread = new LAPriceFXDigitalCallSpreadOptionValue();
	fm.setFunction(pfxdigcallspread, FN_FXDIGITALCALLSPREADOPTIONVALUE_STR);
	pfxdigcallspread->registerData(dm);

	//fxknockout
	LAPriceFXKnockoutRebateValue* pfxknockout = new LAPriceFXKnockoutRebateValue();
	fm.setFunction(pfxknockout, FN_FXKNOCKOUTREBATEVALUE_STR);
	pfxknockout->registerData(dm);

	//fxsinglebarrier
	LAPriceFXSingleBarrierOptionValue* pfxsingle = new LAPriceFXSingleBarrierOptionValue();
	fm.setFunction(pfxsingle, FN_FXSINGLEBARRIEROPTIONVALUE_STR);
	pfxsingle->registerData(dm);

	//fxdigitalsinglebarrier
	LAPriceFXDigitalCallSpreadSingleBarrierOptionValue* pfxdigsingle = new LAPriceFXDigitalCallSpreadSingleBarrierOptionValue();
	fm.setFunction(pfxdigsingle, FN_FXDIGITALCALLSPREADSINGLEBARRIEROPTIONVALUE_STR);
	pfxdigsingle->registerData(dm);

	//plainvanilla swap
	//LALinearRatesSwapTradeValue* pvanillatrade = new LALinearRatesSwapTradeValue();
	LALinearRatesSwapTradeValue* pvanillatrade = new LALinearRatesSwapTradeValue( new LAPriceAccruedInterest());
	fm.setFunction(pvanillatrade, FN_IR_PLAINVANILLASWAPTRADEVALUE_STR);
	pvanillatrade->registerData(dm);
	LAPriceCashValue* pcash = new LAPriceCashValue();
	fm.setFunction(pcash, FN_IR_CASHVALUE_STR);
	pcash->registerData(dm);
	
	
	//convergence
	LAPriceConvergenceValue* pconvergece = new LAPriceConvergenceValue();
	fm.setFunction(pconvergece, FN_IR_CONVERGENCEVALUE_STR);
	pconvergece->registerData(dm);

	//capfloor
	LAPriceIRCapFloorOptionValue* pcapfloor = new LAPriceIRCapFloorOptionValue();
	fm.setFunction(pcapfloor, FN_IR_CAPFLOOROPTIONVALUE_STR);
	pcapfloor->registerData(dm);

	//swaption
	LAPriceIRSwaptionValue* pswaption = new LAPriceIRSwaptionValue();
	pswaption->registerData(dm);
	fm.setFunction(pswaption, FN_IR_SWAPTIONVALUE_STR);

	LAPriceIRSwaptionValueFromCashFlow* pswaption2 = new LAPriceIRSwaptionValueFromCashFlow();
	fm.setFunction(pswaption2, FN_IR_SWAPTIONVALUEFROMCASHFLOW_STR);
//#ifndef RH6
//	LAPriceNDSSwaptionValue* pndsswaption = new LAPriceNDSSwaptionValue();
//	fm.setFunction(pndsswaption, FN_IR_NDSSWAPTIONVALUE_STR);
//#endif
	fm.setFunction(new LAPolynomialPS(1), FN_POLYNOMIALPS_STR);
	
	LABlackScholesBaseMethod* pbsbasef = new LABlackScholesBaseMethod();
	fm.setFunction(pbsbasef, FN_BSBASEFUNC_STR);

	LABlackScholesCapletOption* pbscaplet = new LABlackScholesCapletOption();
	fm.setFunction(pbscaplet, FN_IR_CAPLETOPTIONFUNC_STR);

	LABlackScholesFloorletOption* pbsfloorlet = new LABlackScholesFloorletOption();
	fm.setFunction(pbsfloorlet, FN_IR_FLOORLETOPTIONFUNC_STR);
	
	LABlackScholesCollarOption* pbscollar = new LABlackScholesCollarOption();
	fm.setFunction(pbscollar, FN_IR_COLLAROPTIONFUNC_STR);
	
	LABlackScholesStrangleOption* pbsstrangle = new LABlackScholesStrangleOption();
	fm.setFunction(pbsstrangle, FN_IR_STRANGLEOPTIONFUNC_STR);

	LABlackScholesStraddleOption* pbsstraddle = new LABlackScholesStraddleOption();
	fm.setFunction(pbsstraddle, FN_IR_STRADDLEOPTIONFUNC_STR);

	LABlackScholesCapletSpreadOption* pbscapletspread = new LABlackScholesCapletSpreadOption();
	fm.setFunction(pbscapletspread, FN_IR_CAPLETSPREADOPTIONFUNC_STR);

	LABlackScholesFloorletSpreadOption* pbsfloorletspread = new LABlackScholesFloorletSpreadOption();
	fm.setFunction(pbsfloorletspread, FN_IR_FLOORLETSPREADOPTIONFUNC_STR);

	LABlackScholesDelayedCapletOption* pbsdelayedcaplet = new LABlackScholesDelayedCapletOption();
	fm.setFunction(pbsdelayedcaplet, FN_IR_DELAYEDCAPLETOPTIONFUNC_STR);

	LABlackSholesDelayedFloorletOption* pbsdelayedfloorlet = new LABlackSholesDelayedFloorletOption();
	fm.setFunction(pbsdelayedfloorlet, FN_IR_DELAYEDFLOORLETOPTIONFUNC_STR);

	LABlackScholesDigitalCapletOption* pbsdigitalcaplet = new LABlackScholesDigitalCapletOption();
	fm.setFunction(pbsdigitalcaplet, FN_IR_DIGITALCAPLETOPTIONFUNC_STR);

	LABlackShcolesDigitalFloorletOption* pbsdigitalfloorlet = new LABlackShcolesDigitalFloorletOption();
	fm.setFunction(pbsdigitalfloorlet, FN_IR_DIGITALFLOORLETOPTIONFUNC_STR);

	LAPriceOptionGenerator* popgen = new LAPriceOptionGenerator();
	fm.setFunction(popgen, FN_IR_OPTIONGENERATOR_STR);
	popgen->registerData(dm);

}

// 
/*!
    @brief regist extra data data to attributemaster object

	@param[in,out] dm LAPriceDataManager &	
	@return void 
*/
void
LAModelSetupVanilla::registAttrMasterEx(LAPriceDataManager &dm)
{
}


// 
/*!
    @brief regist extra object data to entitymaster object

	@param[in,out] dataInstance LADataInstance &	
	@return void 
*/
void
LAModelSetupVanilla::registEntityMasterEx(LADataInstance &dataInstance)
{
	LAObjectMaster &em = dataInstance.getObjectMaster();
	em.setEntity(new LAMathPlainVanillaEntity(&dataInstance));
}

// 
/*!
    @brief regist extra function data to functionmaster object

	@param[in,out] dataInstance LADataInstance &
	@return void 
*/
void
LAModelSetupVanilla::registFunctionMasterEx(LADataInstance &dataInstance)
{
	dataInstance;
}




