//  2008, AlgoQuantHub.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceFXDisplacedDiffusionCalibration3F.h"
#include "LAObject.h"
#include "LADataProcedure.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataReference.h"
#include "LADataMultiReference.h"
#include "LADataInstance.h"
#include "LAPriceDataManager.h"
#include "LAObjectPool.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataFunction.h"
#include "LABasic.h"
#include "LAAlgorithm.h"
#include "LAMathDateCalculations.h"
#include "LAPriceCFGenUtility.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LADataMatrix.h"
#include "LAMathValuableEntity.h"
#include "LAMathLeastSquareBlackDD.h"
#include "LAQuantLibEndCriteria.h"
#include "LAQuantLibProblem.h"
#include "LAQuantLibOptimizationMethod.h"
#include "LAQuantLibConjugateGradient.h"
#include "LAQuantLibSteepestDescent.h"
#include "LAQuantLibSimplex.h"
#include "LAQuantLibLevenbergMarquardt.h"
#include "LAQuantLibBoundaryConstraint.h"
#include "LAMathHybridHWLVUtil.h"
#include "LAMathLeastSquareHybridHWLVProcess.h"

#include "LAPriceTargetFunction.h"
#include "LAPriceLSTargetFunction.h"
#include "LAPriceCashFlowGenerator.h"
#include "LAOptimumBFGS.h"
#include <algorithm>
#include <float.h>

using namespace std;

///////////////////////////////////////////////////////////////////////
/*!
    @brief default constructor
*/
LAPriceFXDisplacedDiffusionCalibration3F::LAPriceFXDisplacedDiffusionCalibration3F()
: LAPriceFXDisplacedDiffusionCalibration()
{
}

/*!
    @brief copy constructor
*/
LAPriceFXDisplacedDiffusionCalibration3F::LAPriceFXDisplacedDiffusionCalibration3F(const LAPriceFXDisplacedDiffusionCalibration3F& p)
: LAPriceFXDisplacedDiffusionCalibration(p)
{

}

/*!
    @brief destructor

*/
LAPriceFXDisplacedDiffusionCalibration3F::~LAPriceFXDisplacedDiffusionCalibration3F()
{
}
/*!
    @brief  Check this class ID is the same or not

	@param[in] id function ID
	@return true or false
*/
bool
LAPriceFXDisplacedDiffusionCalibration3F::isTypeOf(function_t id) const
{
	return (id == FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION3F ? true :
						LACoreProcedure::isTypeOf(id));
}
/*!
    @brief  Copy this class

	@return pointer to copied object
*/
LACoreFunctionBase*		
LAPriceFXDisplacedDiffusionCalibration3F::clone() const
{
    try 
	{
        return new LAPriceFXDisplacedDiffusionCalibration3F();
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}

/*!
	@brief Retern class type

	@return class type
*/
function_t			
LAPriceFXDisplacedDiffusionCalibration3F::getType() const
{
	return FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION;
}

/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
LAPriceFXDisplacedDiffusionCalibration3F::registerData(LAPriceDataManager& dm) const
{
	LAPriceFXDisplacedDiffusionCalibration::registerData(dm);
	dm.setData(PRICING_DATA_SKEWVOLCALIBINFO,			DATA_REFERENCE);
	dm.setData(PRICING_DATA_SIGMABETACALIBINFO,		DATA_REFERENCE);
	dm.setData(PRICING_DATA_DOMESTICCURVETYPE,			DATA_STRING);
	dm.setData(PRICING_DATA_FOREIGNCURVETYPE,			DATA_STRING);
	dm.setData(PRICING_DATA_INITIALVALUE,				DATA_DOUBLES);
	dm.setData(CALIBRATION_DATA_OPT_METHOD_TYPE,					DATA_STRING);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MAX_VOL,				DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MAX_SKEW,				DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MAX_SIGMA,				DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MAX_BETA,				DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MIN_VOL,				DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MIN_SKEW,				DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MIN_SIGMA,				DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MIN_BETA,				DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_SMALL_STEPS,						DATA_INT);
	dm.setData(CALIBRATION_DATA_MAX_ITERATION,					DATA_INT);
	dm.setData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION,	DATA_INT);
	dm.setData(CALIBRATION_DATA_ROOT_EPSILON,					DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_FUNCTION_EPSILON,				DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON,			DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_OPT_SIGMABETA_FLAG,				DATA_BOOL);
}

/*!
	@brief Generate CashFlow

	@param[in] basedate basedate
	@param[in,out] leg evaluated Object 
	@param[in] att Data with estimation procedure class 

	@note basedate is not used in estimation
*/
void	            
LAPriceFXDisplacedDiffusionCalibration3F::calibrateModel(const LADate& basedate, 
										LAObject& object, 
										const LADataProcedure& att) const
{
	calibMarketSkewVol(basedate, object, att);
	calibHybridHWDDProcess(basedate, object, att);
	//(void)basedate; (void)att;

	//const LAObject &calibinfo_sigmabeta = dynamic_cast<LADataReference &>(object.getData(PRICING_DATA_SIGMABETACALIBINFO, ISNOTNULL).get()).get().get();
	//LAObject& vole = dynamic_cast<LADataReference &>(object.getData(PRICING_DATA_SDEINFO,ISNOTNULL).get()).get().get();
	//// calib flag
	//const bool optSBFlag = dynamic_cast<const LADataBool &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_OPT_SIGMABETA_FLAG, ISNOTNULL).get()).get();
	//if (!optSBFlag)
	//{
	//	// check param exist
	//	LADataHolder *dh = &vole.getData(PRICING_DATA_FXVOLGRIDS, NOCHECK);
	//	if (!dh->isDefined() || dh->isNull())
	//	{
	//		throw LACoreInvalidData("FXVolGrid does not exist.", __FILE__, __LINE__);
	//	}
	//	dh = &vole.getData(PRICING_DATA_FXBETAGRIDS, NOCHECK);
	//	if (!dh->isDefined() || dh->isNull())
	//	{
	//		throw LACoreInvalidData("FXBetaGrid does not exist.", __FILE__, __LINE__);
	//	}
	//	return;
	//}

	//const LAObject &calibinfo_skewvol = dynamic_cast<LADataReference &>(object.getData(PRICING_DATA_SKEWVOLCALIBINFO, ISNOTNULL).get()).get().get();
	//// set up skew vol boundary
	//const double boundaryMaxSkew = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_BOUNDARY_MAX_SKEW, ISNOTNULL).get()).get();
	//const double boundaryMinSkew = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_BOUNDARY_MIN_SKEW, ISNOTNULL).get()).get();
	//const double boundaryMaxVol = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_BOUNDARY_MAX_VOL, ISNOTNULL).get()).get();
	//const double boundaryMinVol = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_BOUNDARY_MIN_VOL, ISNOTNULL).get()).get();
	//// set up end criteria
 //   const int maxIteration_skewvol = dynamic_cast<const LADataInt &>(calibinfo_skewvol.getData(CALIBRATION_DATA_MAX_ITERATION, ISNOTNULL).get()).get();
 //   const int maxStationaryStateIteration_skewvol = dynamic_cast<const LADataInt &>(calibinfo_skewvol.getData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, ISNOTNULL).get()).get();
 //   const double rootEpsilon_skewvol = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_ROOT_EPSILON, ISNOTNULL).get()).get();
 //   const double functionEpsilon_skewvol = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_FUNCTION_EPSILON, ISNOTNULL).get()).get();
 //   const double gradientNormEpsilon_skewvol = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, ISNOTNULL).get()).get();

	//LADataInstance* pDataInstance = object.getDataInstance();
	//// create domestic curve
	//const LAObject &yldentity_d = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_DOMESTICCURVE, ISNOTNULL).get()).get().get();
	//const LAString &yldname_d = dynamic_cast<const LADataString &>(yldentity_d.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	//const LAString &curveType_d = dynamic_cast<const LADataString &>(object.getData(PRICING_DATA_DOMESTICCURVETYPE, ISNOTNULL).get()).get();
	//LAMathYieldCurve yc_d(pDataInstance);	
	//yc_d.getData(IR_CALIBRATION_DATA_YIELDDATA,ISDEFINED).convertFromString(yldname_d);
	//yc_d.setCurveType(curveType_d);
	//yc_d.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	//LAMathPathYieldCurve curve0_d(&yc_d, 0.0, ACT_365_ISDA);
	//// create foreign curve
	//const LAObject &yldentity_f = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_FOREIGNCURVE, ISNOTNULL).get()).get().get();
	//const LAString &yldname_f = dynamic_cast<const LADataString &>(yldentity_f.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	//const LAString &curveType_f = dynamic_cast<const LADataString &>(object.getData(PRICING_DATA_FOREIGNCURVETYPE, ISNOTNULL).get()).get();
	//LAMathYieldCurve yc_f(pDataInstance);
	//yc_f.getData(IR_CALIBRATION_DATA_YIELDDATA,ISDEFINED).convertFromString(yldname_f);
	//yc_f.setCurveType(curveType_f);
	//yc_f.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	//LAMathPathYieldCurve curve0_f(&yc_f, 0.0, ACT_365_ISDA);
	//// spot fx
	//const double spotFX = dynamic_cast<const LADataDouble &>(object.getData(PRICING_DATA_SPOTFX, ISNOTNULL).get()).get();
	//// opt type
	//LAString optMethodType_skewvol = dynamic_cast<const LADataString &>(calibinfo_skewvol.getData(CALIBRATION_DATA_OPT_METHOD_TYPE, ISNOTNULL).get()).get();
	//optMethodType_skewvol.toUpper();

	//LAQuantLibEndCriteria endCriteria_skewvol(maxIteration_skewvol, maxStationaryStateIteration_skewvol, rootEpsilon_skewvol, functionEpsilon_skewvol, gradientNormEpsilon_skewvol);

	//const LADataMultiReference &refData = dynamic_cast<LADataMultiReference &>(object.getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
	//unsigned int gridSize = refData.getSize();

	//DoubleVector calculatedMktVolDD(gridSize);
	//DoubleVector calculatedMktSkewDD(gridSize);
	//DoubleVector timegrid(gridSize);
	//for (unsigned int i = 0; i < gridSize; ++i)
	//{
	//	timegrid[i] = dynamic_cast<const LADataDouble &>(refData.get(i).getData(PRICING_DATA_FXTERM, ISNOTNULL).get()).get();
	//	const DoubleVector tVec(1, timegrid[i]);
	//	const DoubleMatrix premMat(1, dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_OPTIONPREMIUMS, ISNOTNULL).get()).get());
	//	const DoubleMatrix strikeMat(1, dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_STRIKES, ISNOTNULL).get()).get());
	//	const DoubleMatrix weightMat(1, dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_WEIGHTS, ISNOTNULL).get()).get());
	//	const BoolVector &cpFlag = dynamic_cast<const LADataBools &>(refData.get(i).getData(PRICING_DATA_ISCALLS, ISNOTNULL).get()).get();
	//	IntVector int_cpFlag(cpFlag.size(), -1);
	//	for (unsigned int j = 0; j < int_cpFlag.size(); ++j)
	//	{
	//		if (cpFlag[j])
	//		{
	//			int_cpFlag[j] = 1;
	//		}
	//	}
	//	const vector<IntVector> cpFlagMat(1, int_cpFlag);

	//	LAMathLeastSquareBlackDD blackDD(spotFX, curve0_d, curve0_f, tVec, strikeMat, premMat, weightMat, cpFlagMat); 
	//	LAMathBoundaryConstraintMktSkewVol bcSkewVol(boundaryMaxSkew, boundaryMinSkew, boundaryMaxVol, boundaryMinVol);
	//
	//	const DoubleArray &initialVals = dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_INITIALVALUE, ISNOTNULL).get()).get();
	//	if (initialVals.size() != 2)
	//	{
	//		LACoreInvalidData("initialVals.size must be 2!", __FILE__, __LINE__);
	//	}
	//	LAQuantLibArray x(initialVals.size());
	//	for (unsigned int j = 0; j < initialVals.size(); ++j)
	//	{
	//		x[j] = initialVals[j];
	//	}
	//	// probrem
	//	LAQuantLibProblem problem(blackDD, bcSkewVol, x);

	//	// optMethd
	//	std::shared_ptr<LAQuantLibOptimizationMethod> pOptMethod;
	//	if (optMethodType_skewvol == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
	//	{
	//		pOptMethod = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibConjugateGradient());
	//	}
	//	else if (optMethodType_skewvol == CALIB_STEEPEST_DESCENT_METHOD)
	//	{
	//		pOptMethod = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSteepestDescent());
	//	}
	//	else if (optMethodType_skewvol == CALIB_SIMPLEX_METHOD)
	//	{
	//		pOptMethod = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSimplex(0.0001));
	//	}
	//	else if (optMethodType_skewvol == CALIB_LEVENBERG_MARQUARDT_METHOD)
	//	{
 //           double levenbergMarquardtEpsfcn = 1.0e-8;     // parameters specific for Levenberg-Marquardt
 //           double levenbergMarquardtXtol   = 1.0e-8;     //
 //           double levenbergMarquardtGtol   = 1.0e-8;     //
	//		pOptMethod = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol));
	//	}
	//	else
	//	{
	//		LAString msg = "Bad optMethod , " + optMethodType_skewvol;
	//		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	//	}

	//	LAQuantLibEndCriteria::Type endCriteriaResult = pOptMethod->minimize(problem, endCriteria_skewvol);
	//	LAQuantLibArray value = problem.currentValue();
	//	double optResult = problem.value(value);

	//	calculatedMktVolDD[i] = value[0];
	//	calculatedMktSkewDD[i] = value[1];
	//}
	////getDomestic HW parameter
	//const LADataReference &hw_d = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_DOMESTICIRMODEL, ISNOTNULL).get());
	//const DoubleVector &timegrid_canonic = dynamic_cast<const LADataDoubles &>(hw_d.get().getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get()).get();
	//const DoubleVector &hw_vol_d = dynamic_cast<const LADataDoubles &>(hw_d.get().getData(PRICING_DATA_CALIBVOL_T, ISNOTNULL).get()).get();
	//const DoubleVector &hw_meanrev_d = dynamic_cast<const LADataDoubles &>(hw_d.get().getData(PRICING_DATA_CALIBMEANREV_T, ISNOTNULL).get()).get();
	//LAMathHullWhiteParams hwParams_d(timegrid_canonic, hw_meanrev_d, hw_vol_d);

	////getForeign HW parameter
	//const LADataReference &hw_f = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_FOREIGNIRMODEL, ISNOTNULL).get());
	//const DoubleVector &timegrid_canonic_f = dynamic_cast<const LADataDoubles &>(hw_f.get().getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get()).get();
	//if (timegrid_canonic != timegrid_canonic_f)
	//{
	//	throw LACoreInvalidData("Invalid hw canonical terms.", __FILE__, __LINE__);
	//}
	//const DoubleVector &hw_vol_f = dynamic_cast<const LADataDoubles &>(hw_f.get().getData(PRICING_DATA_CALIBVOL_T, ISNOTNULL).get()).get();
	//const DoubleVector &hw_meanrev_f = dynamic_cast<const LADataDoubles &>(hw_f.get().getData(PRICING_DATA_CALIBMEANREV_T, ISNOTNULL).get()).get();
	//LAMathHullWhiteParams hwParams_f(timegrid_canonic, hw_meanrev_f, hw_vol_f);

	//const DoubleVector &corVec = dynamic_cast<const LADataDoubles &>(object.getData(PRICING_DATA_SDECORRELATIONS, ISNOTNULL).get()).get();
	//if (corVec.size() != 3)
	//{
	//	throw LACoreInvalidData("SDE Correlation format is wrong.", __FILE__, __LINE__);
	//}
	//DoubleMatrix corMatrix(corVec.size());
	//corMatrix[0] = DoubleVector(3, 1.0);
	//corMatrix[1] = DoubleVector(3, 1.0);
	//corMatrix[2] = DoubleVector(3, 1.0);
	//corMatrix[0][1] = corVec[0], corMatrix[1][0] = corVec[0]; // ccy_d ccy_f
	//corMatrix[0][2] = corVec[1], corMatrix[2][0] = corVec[1]; // ccy_d fx
 //	corMatrix[1][2] = corVec[2], corMatrix[2][1] = corVec[2]; // ccy_f fx
	//// correlation param
	//vector<DoubleMatrix> corParamSet(timegrid_canonic.size(), corMatrix);
	//LAMathCorrelationParams corParams(timegrid_canonic, corParamSet); 

	//// fx0
	//DoubleVector fx0(gridSize);
	//for (unsigned int i = 0; i < fx0.size(); ++i)
	//{
	//	fx0[i] = spotFX * curve0_f.getP(timegrid[i]) / curve0_d.getP(timegrid[i]);
	//}
	//// set up sigma beta boundary
	//const double boundaryMaxSigma = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_BOUNDARY_MAX_SIGMA, ISNOTNULL).get()).get();
	//const double boundaryMinSigma = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_BOUNDARY_MIN_SIGMA, ISNOTNULL).get()).get();
	//const double boundaryMaxBeta = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_BOUNDARY_MAX_BETA, ISNOTNULL).get()).get();
	//const double boundaryMinBeta = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_BOUNDARY_MIN_BETA, ISNOTNULL).get()).get();

	//// set up end criteria
 //   const int maxIteration_sigmabeta = dynamic_cast<const LADataInt &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_MAX_ITERATION, ISNOTNULL).get()).get();
 //   const int maxStationaryStateIteration_sigmabeta = dynamic_cast<const LADataInt &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, ISNOTNULL).get()).get();
 //   const double rootEpsilon_sigmabeta = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_ROOT_EPSILON, ISNOTNULL).get()).get();
 //   const double functionEpsilon_sigmabeta = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_FUNCTION_EPSILON, ISNOTNULL).get()).get();
 //   const double gradientNormEpsilon_sigmabeta = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, ISNOTNULL).get()).get();

	//// sigma and beta
	//const DoubleVector &timegrid_fx = dynamic_cast<const LADataDoubles &>(vole.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get()).get();
	////DoubleVector sigma(timegrid_fx.size(), 0.1);
	////DoubleVector beta(timegrid_fx.size(), 1.0);
	//const DoubleVector &sigma = dynamic_cast<const LADataDoubles &>(vole.getData(PRICING_DATA_FXVOLGRIDS, ISNOTNULL).get()).get();
	//const DoubleVector &beta = dynamic_cast<const LADataDoubles &>(vole.getData(PRICING_DATA_FXBETAGRIDS, ISNOTNULL).get()).get();

	//LAMathDisplacedDiffusionParams ddParams(timegrid_fx, sigma, beta);
	//const int smallSteps = dynamic_cast<const LADataInt &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_SMALL_STEPS, ISNOTNULL).get()).get();

	//// opt type
	//LAString optMethodType_sigmabeta = dynamic_cast<const LADataString &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_OPT_METHOD_TYPE, ISNOTNULL).get()).get();
	//optMethodType_sigmabeta.toUpper();

	//LAQuantLibEndCriteria endCriteria_sigmabeta(maxIteration_sigmabeta, maxStationaryStateIteration_sigmabeta, rootEpsilon_sigmabeta, functionEpsilon_sigmabeta, gradientNormEpsilon_sigmabeta);

	//DoubleVector sigma_ave(gridSize);
	//DoubleVector skew_ave(gridSize);
	//for (unsigned int i = 0; i < gridSize; ++i)
	//{
	//	const DoubleVector tVec(1, timegrid[i]);
	//	const DoubleVector fx0Vec(1, fx0[i]);
	//	const DoubleMatrix volMat(1, DoubleVector(1, calculatedMktVolDD[i]));
	//	const DoubleMatrix betaMat(1, DoubleVector(1, calculatedMktSkewDD[i]));
	//	const DoubleMatrix strikeMat(1, fx0Vec);
	//	const DoubleMatrix weightMat(1, DoubleVector(1, 1.0));
	//	LAMathTarget2fit2 sigma_target(tVec, strikeMat, volMat, weightMat);
	//	LAMathTarget2fit2 beta_target(tVec, strikeMat, betaMat, weightMat);

	//	LAMathHybridHWDDMPProcessHelper2 processHelper(timegrid[i], fx0[i], hwParams_d, hwParams_f, ddParams, corParams, timegrid_canonic, smallSteps);
	//	vector<LAMathHybridHWDDProcessHelper *> processHelperVec(1, &processHelper);
	//	double t_ = 0.0;
	//	if (i != 0)
	//	{
	//		t_ =  timegrid[i - 1];
	//	}
	//	LAMathLeastSquareHybridHWDDProcess2 leastSquareSigma(fx0Vec, processHelperVec, sigma_target, t_);
	//	LAMathLeastSquareHybridHWDDProcess3 leastSquareBeta(fx0Vec, processHelperVec, beta_target, t_);
	//	
	//	LAQuantLibBoundaryConstraint bcSigma(boundaryMinSigma, boundaryMaxSigma);
	//	LAQuantLibBoundaryConstraint bcBeta(boundaryMinBeta, boundaryMaxBeta);

	//	LAQuantLibArray x(1, sigma[i]);
	//	LAQuantLibArray y(1, beta[i]);
	//	// probrem
	//	LAQuantLibProblem problemSigma(leastSquareSigma, bcSigma, x);
	//	LAQuantLibProblem problemBeta(leastSquareBeta, bcBeta, y);

	//	// optMethd
	//	std::shared_ptr<LAQuantLibOptimizationMethod> pOptMethodSigma;
	//	std::shared_ptr<LAQuantLibOptimizationMethod> pOptMethodBeta;
	//	if (optMethodType_sigmabeta == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
	//	{
	//		pOptMethodSigma = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibConjugateGradient());
	//		pOptMethodBeta =  std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibConjugateGradient());
	//	}
	//	else if (optMethodType_sigmabeta == CALIB_STEEPEST_DESCENT_METHOD)
	//	{
	//		pOptMethodSigma = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSteepestDescent());
	//		pOptMethodBeta = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSteepestDescent());
	//	}
	//	else if (optMethodType_sigmabeta == CALIB_SIMPLEX_METHOD)
	//	{
	//		pOptMethodSigma = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSimplex(0.0001));
	//		pOptMethodBeta = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSimplex(0.0001));
	//	}
	//	else if (optMethodType_sigmabeta == CALIB_LEVENBERG_MARQUARDT_METHOD)
	//	{
 //           double levenbergMarquardtEpsfcn = 1.0e-8;     // parameters specific for Levenberg-Marquardt
 //           double levenbergMarquardtXtol   = 1.0e-8;     //
 //           double levenbergMarquardtGtol   = 1.0e-8;     //
	//		pOptMethodSigma = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol));
	//		pOptMethodBeta = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol));
	//	}
	//	else
	//	{
	//		LAString msg = "Bad optMethod , " + optMethodType_sigmabeta;
	//		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	//	}

	//	LAQuantLibEndCriteria::Type endCriteriaResultSigma = pOptMethodSigma->minimize(problemSigma, endCriteria_sigmabeta);
	//	LAQuantLibEndCriteria::Type endCriteriaResultBeta = pOptMethodBeta->minimize(problemBeta, endCriteria_sigmabeta);

	//	sigma_ave[i] = processHelperVec[0]->calcSigma();
	//	skew_ave[i] = processHelperVec[0]->calcSkew();
	//}
	//unsigned int pos;
	//LAAlgorithm::locate<DoubleArray, double>(ddParams.mT, timegrid.back(), ddParams.mT.size(), pos);
	//if (pos < ddParams.mT.size())
	//{
	//	if (timegrid.back() < ddParams.mT[pos])
	//	{
	//		--pos;
	//	}
	//	const double l_sigma = ddParams.mSigma[pos];
	//	const double l_beta = ddParams.mBeta[pos];
	//	for (unsigned int i = pos + 1; i < ddParams.mT.size(); ++i)
	//	{
	//		ddParams.mSigma[i] = l_sigma;
	//		ddParams.mBeta[i] = l_beta;
	//	}
	//}
	//// set calibrated param
	//vole.remove(PRICING_DATA_FXVOLGRIDS);
	//vole.add(PRICING_DATA_FXVOLGRIDS, new LADataDoubles(ddParams.mSigma));
	//vole.remove(PRICING_DATA_FXBETAGRIDS);
	//vole.add(PRICING_DATA_FXBETAGRIDS, new LADataDoubles(ddParams.mBeta));
}




/*!
	@brief calibrate market skew vol

	@param[in] basedate basedate
	@param[in,out] leg evaluated Object 
	@param[in] att Data with estimation procedure class 

	@note basedate is not used in estimation
*/
void	            
LAPriceFXDisplacedDiffusionCalibration3F::calibMarketSkewVol(const LADate& basedate, 
										LAObject& object, 
										const LADataProcedure& att) const
{
	(void)basedate; (void)att;

	const LAObject &calibinfo_skewvol = dynamic_cast<LADataReference &>(object.getData(PRICING_DATA_SKEWVOLCALIBINFO, ISNOTNULL).get()).get().get();
	// set up skew vol boundary
	const double boundaryMaxSkew = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_BOUNDARY_MAX_SKEW, ISNOTNULL).get()).get();
	const double boundaryMinSkew = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_BOUNDARY_MIN_SKEW, ISNOTNULL).get()).get();
	const double boundaryMaxVol = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_BOUNDARY_MAX_VOL, ISNOTNULL).get()).get();
	const double boundaryMinVol = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_BOUNDARY_MIN_VOL, ISNOTNULL).get()).get();
	// set up end criteria
    const int maxIteration_skewvol = dynamic_cast<const LADataInt &>(calibinfo_skewvol.getData(CALIBRATION_DATA_MAX_ITERATION, ISNOTNULL).get()).get();
    const int maxStationaryStateIteration_skewvol = dynamic_cast<const LADataInt &>(calibinfo_skewvol.getData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, ISNOTNULL).get()).get();
    const double rootEpsilon_skewvol = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_ROOT_EPSILON, ISNOTNULL).get()).get();
    const double functionEpsilon_skewvol = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_FUNCTION_EPSILON, ISNOTNULL).get()).get();
    const double gradientNormEpsilon_skewvol = dynamic_cast<const LADataDouble &>(calibinfo_skewvol.getData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, ISNOTNULL).get()).get();

	LADataInstance* pDataInstance = object.getDataInstance();
	// create domestic curve
	const LAObject &yldentity_d = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_DOMESTICCURVE, ISNOTNULL).get()).get().get();
	const LAString &yldname_d = dynamic_cast<const LADataString &>(yldentity_d.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	const LAString &curveType_d = dynamic_cast<const LADataString &>(object.getData(PRICING_DATA_DOMESTICCURVETYPE, ISNOTNULL).get()).get();
	LAMathYieldCurve yc_d(pDataInstance);	
	yc_d.getData(IR_CALIBRATION_DATA_YIELDDATA,ISDEFINED).convertFromString(yldname_d);
	yc_d.setCurveType(curveType_d);
	yc_d.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	LAMathPathYieldCurve curve0_d(&yc_d, 0.0, ACT_365_ISDA);
	// create foreign curve
	const LAObject &yldentity_f = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_FOREIGNCURVE, ISNOTNULL).get()).get().get();
	const LAString &yldname_f = dynamic_cast<const LADataString &>(yldentity_f.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	const LAString &curveType_f = dynamic_cast<const LADataString &>(object.getData(PRICING_DATA_FOREIGNCURVETYPE, ISNOTNULL).get()).get();
	LAMathYieldCurve yc_f(pDataInstance);
	yc_f.getData(IR_CALIBRATION_DATA_YIELDDATA,ISDEFINED).convertFromString(yldname_f);
	yc_f.setCurveType(curveType_f);
	yc_f.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	LAMathPathYieldCurve curve0_f(&yc_f, 0.0, ACT_365_ISDA);
	// spot fx
	const double spotFX = dynamic_cast<const LADataDouble &>(object.getData(PRICING_DATA_SPOTFX, ISNOTNULL).get()).get();
	// opt type
	LAString optMethodType_skewvol = dynamic_cast<const LADataString &>(calibinfo_skewvol.getData(CALIBRATION_DATA_OPT_METHOD_TYPE, ISNOTNULL).get()).get();
	optMethodType_skewvol.toUpper();

	LAQuantLibEndCriteria endCriteria_skewvol(maxIteration_skewvol, maxStationaryStateIteration_skewvol, rootEpsilon_skewvol, functionEpsilon_skewvol, gradientNormEpsilon_skewvol);

	const LADataMultiReference &refData = dynamic_cast<LADataMultiReference &>(object.getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
	unsigned int gridSize = refData.getSize();

	DoubleVector mktVolDD(gridSize);
	DoubleVector mktSkewDD(gridSize);
	DoubleVector timegrid(gridSize);
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		timegrid[i] = dynamic_cast<const LADataDouble &>(refData.get(i).getData(PRICING_DATA_FXTERM, ISNOTNULL).get()).get();
		const DoubleVector tVec(1, timegrid[i]);
		const DoubleMatrix premMat(1, dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_OPTIONPREMIUMS, ISNOTNULL).get()).get());
		const DoubleMatrix strikeMat(1, dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_STRIKES, ISNOTNULL).get()).get());
		const DoubleMatrix weightMat(1, dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_WEIGHTS, ISNOTNULL).get()).get());
		const BoolVector &cpFlag = dynamic_cast<const LADataBools &>(refData.get(i).getData(PRICING_DATA_ISCALLS, ISNOTNULL).get()).get();
		IntVector int_cpFlag(cpFlag.size(), -1);
		for (unsigned int j = 0; j < int_cpFlag.size(); ++j)
		{
			if (cpFlag[j])
			{
				int_cpFlag[j] = 1;
			}
		}
		const vector<IntVector> cpFlagMat(1, int_cpFlag);

		LAMathLeastSquareBlackDD blackDD(spotFX, curve0_d, curve0_f, tVec, strikeMat, premMat, weightMat, cpFlagMat); 
		LAMathBoundaryConstraintMktSkewVol bcSkewVol(boundaryMaxSkew, boundaryMinSkew, boundaryMaxVol, boundaryMinVol);
	
		const DoubleVector &initialVals = dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_INITIALVALUE, ISNOTNULL).get()).get();
		if (initialVals.size() != 2)
		{
			LACoreInvalidData("initialVals.size must be 2!", __FILE__, __LINE__);
		}
		LAQuantLibArray x(initialVals.size());
		for (unsigned int j = 0; j < initialVals.size(); ++j)
		{
			x[j] = initialVals[j];
		}
		// probrem
		LAQuantLibProblem problem(blackDD, bcSkewVol, x);

		// optMethd
		std::shared_ptr<LAQuantLibOptimizationMethod> pOptMethod;
		if (optMethodType_skewvol == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
		{
			pOptMethod = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibConjugateGradient());
		}
		else if (optMethodType_skewvol == CALIB_STEEPEST_DESCENT_METHOD)
		{
			pOptMethod = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSteepestDescent());
		}
		else if (optMethodType_skewvol == CALIB_SIMPLEX_METHOD)
		{
			pOptMethod = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSimplex(0.0001));
		}
		else if (optMethodType_skewvol == CALIB_LEVENBERG_MARQUARDT_METHOD)
		{
            double levenbergMarquardtEpsfcn = 1.0e-8;     // parameters specific for Levenberg-Marquardt
            double levenbergMarquardtXtol   = 1.0e-8;     //
            double levenbergMarquardtGtol   = 1.0e-8;     //
			pOptMethod = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol));
		}
		else
		{
			LAString msg = "Bad optMethod , " + optMethodType_skewvol;
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		LAQuantLibEndCriteria::Type endCriteriaResult = pOptMethod->minimize(problem, endCriteria_skewvol);
		LAQuantLibArray value = problem.currentValue();
		double optResult = problem.value(value);

		mktVolDD[i] = value[0];
		mktSkewDD[i] = value[1];
	}
	LAObject& vole = dynamic_cast<LADataReference &>(object.getData(PRICING_DATA_SDEINFO,ISNOTNULL).get()).get().get();
	// set calibrated param
	vole.remove(PRICING_DATA_MARKETTERMS);
	vole.add(PRICING_DATA_MARKETTERMS, new LADataDoubles(timegrid));
	vole.remove(PRICING_DATA_MARKETVOLDD);
	vole.add(PRICING_DATA_MARKETVOLDD, new LADataDoubles(mktVolDD));
	vole.remove(PRICING_DATA_MARKETSKEWDD);
	vole.add(PRICING_DATA_MARKETSKEWDD, new LADataDoubles(mktSkewDD));
}

/*!
	@brief calibrate market skew vol

	@param[in] basedate basedate
	@param[in,out] leg evaluated Object 
	@param[in] att Data with estimation procedure class 

	@note basedate is not used in estimation
*/
void	            
LAPriceFXDisplacedDiffusionCalibration3F::calibHybridHWDDProcess(const LADate& basedate, 
										LAObject& object, 
										const LADataProcedure& att) const
{
	const LAObject &calibinfo_sigmabeta = dynamic_cast<LADataReference &>(object.getData(PRICING_DATA_SIGMABETACALIBINFO, ISNOTNULL).get()).get().get();
	LAObject& vole = dynamic_cast<LADataReference &>(object.getData(PRICING_DATA_SDEINFO,ISNOTNULL).get()).get().get();
	// calib flag
	const bool optSBFlag = dynamic_cast<const LADataBool &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_OPT_SIGMABETA_FLAG, ISNOTNULL).get()).get();
	//if (!optSBFlag)
	//{
	//	// check param exist
	//	LADataHolder *dh = &vole.getData(PRICING_DATA_FXVOLGRIDS, NOCHECK);
	//	if (!dh->isDefined() || dh->isNull())
	//	{
	//		throw LACoreInvalidData("FXVolGrid does not exist.", __FILE__, __LINE__);
	//	}
	//	dh = &vole.getData(PRICING_DATA_FXBETAGRIDS, NOCHECK);
	//	if (!dh->isDefined() || dh->isNull())
	//	{
	//		throw LACoreInvalidData("FXBetaGrid does not exist.", __FILE__, __LINE__);
	//	}
	//	return;
	//}
	const LADataMultiReference &refData = dynamic_cast<LADataMultiReference &>(object.getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
	unsigned int gridSize = refData.getSize();

	//get domestic hw parameter
	const LADataReference &hw_d = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_DOMESTICIRMODEL, ISNOTNULL).get());
	const DoubleVector &timegrid_canonic = dynamic_cast<const LADataDoubles &>(hw_d.get().getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get()).get();
	const DoubleVector &hw_vol_d = dynamic_cast<const LADataDoubles &>(hw_d.get().getData(PRICING_DATA_CALIBVOL_T, ISNOTNULL).get()).get();
	const DoubleVector &hw_meanrev_d = dynamic_cast<const LADataDoubles &>(hw_d.get().getData(PRICING_DATA_CALIBMEANREV_T, ISNOTNULL).get()).get();
	LAMathHullWhiteParams hwParams_d(timegrid_canonic, hw_meanrev_d, hw_vol_d);

	//get foreign hw parameter
	const LADataReference &hw_f = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_FOREIGNIRMODEL, ISNOTNULL).get());
	const DoubleVector &timegrid_canonic_f = dynamic_cast<const LADataDoubles &>(hw_f.get().getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get()).get();
	//if (timegrid_canonic != timegrid_canonic_f)
	//{
	//	throw LACoreInvalidData("Invalid hw canonical terms.", __FILE__, __LINE__);
	//}
	const DoubleVector &hw_vol_f = dynamic_cast<const LADataDoubles &>(hw_f.get().getData(PRICING_DATA_CALIBVOL_T, ISNOTNULL).get()).get();
	const DoubleVector &hw_meanrev_f = dynamic_cast<const LADataDoubles &>(hw_f.get().getData(PRICING_DATA_CALIBMEANREV_T, ISNOTNULL).get()).get();
	//LAMathHullWhiteParams hwParams_f(timegrid_canonic, hw_meanrev_f, hw_vol_f);
	LAMathHullWhiteParams hwParams_f(timegrid_canonic_f, hw_meanrev_f, hw_vol_f);

	const DoubleVector &corVec = dynamic_cast<const LADataDoubles &>(object.getData(PRICING_DATA_SDECORRELATIONS, ISNOTNULL).get()).get();
	if (corVec.size() != 3)
	{
		throw LACoreInvalidData("SDE Correlation format is wrong.", __FILE__, __LINE__);
	}
	DoubleMatrix corMatrix(corVec.size());
	corMatrix[0] = DoubleVector(3, 1.0);
	corMatrix[1] = DoubleVector(3, 1.0);
	corMatrix[2] = DoubleVector(3, 1.0);
	corMatrix[0][1] = corVec[0], corMatrix[1][0] = corVec[0]; // ccy_d ccy_f
	corMatrix[0][2] = corVec[1], corMatrix[2][0] = corVec[1]; // ccy_d fx
 	corMatrix[1][2] = corVec[2], corMatrix[2][1] = corVec[2]; // ccy_f fx
	// correlation param
	vector<DoubleMatrix> corParamSet(timegrid_canonic.size(), corMatrix);
	LAMathCorrelationParams corParams(timegrid_canonic, corParamSet); 

	LADataInstance* pDataInstance = object.getDataInstance();
	// create domestic curve
	const LAObject &yldentity_d = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_DOMESTICCURVE, ISNOTNULL).get()).get().get();
	const LAString &yldname_d = dynamic_cast<const LADataString &>(yldentity_d.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	const LAString &curveType_d = dynamic_cast<const LADataString &>(object.getData(PRICING_DATA_DOMESTICCURVETYPE, ISNOTNULL).get()).get();
	LAMathYieldCurve yc_d(pDataInstance);	
	yc_d.getData(IR_CALIBRATION_DATA_YIELDDATA,ISDEFINED).convertFromString(yldname_d);
	yc_d.setCurveType(curveType_d);
	yc_d.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	LAMathPathYieldCurve curve0_d(&yc_d, 0.0, ACT_365_ISDA);
	// create foreign curve
	const LAObject &yldentity_f = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_FOREIGNCURVE, ISNOTNULL).get()).get().get();
	const LAString &yldname_f = dynamic_cast<const LADataString &>(yldentity_f.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	const LAString &curveType_f = dynamic_cast<const LADataString &>(object.getData(PRICING_DATA_FOREIGNCURVETYPE, ISNOTNULL).get()).get();
	LAMathYieldCurve yc_f(pDataInstance);
	yc_f.getData(IR_CALIBRATION_DATA_YIELDDATA,ISDEFINED).convertFromString(yldname_f);
	yc_f.setCurveType(curveType_f);
	yc_f.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	LAMathPathYieldCurve curve0_f(&yc_f, 0.0, ACT_365_ISDA);
	// spot fx
	const double spotFX = dynamic_cast<const LADataDouble &>(object.getData(PRICING_DATA_SPOTFX, ISNOTNULL).get()).get();
	DoubleVector fx0(gridSize);
	const DoubleVector &mktTimeGrid = dynamic_cast<const LADataDoubles &>(vole.getData(PRICING_DATA_MARKETTERMS, ISNOTNULL).get()).get();
	if (mktTimeGrid.size() != gridSize)
	{
		throw LACoreInvalidData("markte grid size is wrong.", __FILE__, __LINE__);
	}
	for (unsigned int i = 0; i < fx0.size(); ++i)
	{
		fx0[i] = spotFX * curve0_f.getP(mktTimeGrid[i]) / curve0_d.getP(mktTimeGrid[i]);
	}
	// set up sigma beta boundary
	const double boundaryMaxSigma = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_BOUNDARY_MAX_SIGMA, ISNOTNULL).get()).get();
	const double boundaryMinSigma = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_BOUNDARY_MIN_SIGMA, ISNOTNULL).get()).get();
	const double boundaryMaxBeta = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_BOUNDARY_MAX_BETA, ISNOTNULL).get()).get();
	const double boundaryMinBeta = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_BOUNDARY_MIN_BETA, ISNOTNULL).get()).get();

	// set up end criteria
    const int maxIteration_sigmabeta = dynamic_cast<const LADataInt &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_MAX_ITERATION, ISNOTNULL).get()).get();
    const int maxStationaryStateIteration_sigmabeta = dynamic_cast<const LADataInt &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, ISNOTNULL).get()).get();
    const double rootEpsilon_sigmabeta = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_ROOT_EPSILON, ISNOTNULL).get()).get();
    const double functionEpsilon_sigmabeta = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_FUNCTION_EPSILON, ISNOTNULL).get()).get();
    const double gradientNormEpsilon_sigmabeta = dynamic_cast<const LADataDouble &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, ISNOTNULL).get()).get();

	// sigma and beta
	const DoubleVector &timegrid_fx = dynamic_cast<const LADataDoubles &>(vole.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get()).get();
	//DoubleVector sigma(timegrid_fx.size(), 0.1);
	//DoubleVector beta(timegrid_fx.size(), 1.0);
	const DoubleVector &sigma = dynamic_cast<const LADataDoubles &>(vole.getData(PRICING_DATA_FXVOLGRIDS, ISNOTNULL).get()).get();
	const DoubleVector &beta = dynamic_cast<const LADataDoubles &>(vole.getData(PRICING_DATA_FXBETAGRIDS, ISNOTNULL).get()).get();

	LAMathDisplacedDiffusionParams ddParams(timegrid_fx, sigma, beta);
	const int smallSteps = dynamic_cast<const LADataInt &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_SMALL_STEPS, ISNOTNULL).get()).get();

	// opt type
	LAString optMethodType_sigmabeta = dynamic_cast<const LADataString &>(calibinfo_sigmabeta.getData(CALIBRATION_DATA_OPT_METHOD_TYPE, ISNOTNULL).get()).get();
	optMethodType_sigmabeta.toUpper();

	LAQuantLibEndCriteria endCriteria_sigmabeta(maxIteration_sigmabeta, maxStationaryStateIteration_sigmabeta, rootEpsilon_sigmabeta, functionEpsilon_sigmabeta, gradientNormEpsilon_sigmabeta);

	DoubleVector sigma_ave(gridSize);
	DoubleVector skew_ave(gridSize);
	DoubleMatrix modelVolDD(gridSize);
	const DoubleVector &mktVolDD = dynamic_cast<const LADataDoubles &>(vole.getData(PRICING_DATA_MARKETVOLDD, ISNOTNULL).get()).get();
	if (mktVolDD.size() != gridSize)
	{
		throw LACoreInvalidData("markte volatility dd size is wrong.", __FILE__, __LINE__);
	}
	const DoubleVector &mktSkewDD = dynamic_cast<const LADataDoubles &>(vole.getData(PRICING_DATA_MARKETSKEWDD, ISNOTNULL).get()).get();
	if (mktSkewDD.size() != gridSize)
	{
		throw LACoreInvalidData("markte skew dd size is wrong.", __FILE__, __LINE__);
	}
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		const DoubleVector tVec(1, mktTimeGrid[i]);
		const DoubleVector fx0Vec(1, fx0[i]);
		const DoubleMatrix volMat(1, DoubleVector(1, mktVolDD[i]));
		const DoubleMatrix betaMat(1, DoubleVector(1, mktSkewDD[i]));
		const DoubleMatrix strikeMat(1, fx0Vec);
		const DoubleMatrix weightMat(1, DoubleVector(1, 1.0));
		LAMathTarget2fit2 sigma_target(tVec, strikeMat, volMat, weightMat);
		LAMathTarget2fit2 beta_target(tVec, strikeMat, betaMat, weightMat);

		LAMathHybridHWDDMPProcessHelper2 processHelper(mktTimeGrid[i], fx0[i], hwParams_d, hwParams_f, ddParams, corParams, timegrid_canonic, smallSteps);
		vector<LAMathHybridHWDDProcessHelper *> processHelperVec(1, &processHelper);
		double t_ = 0.0;
		if (i != 0)
		{
			t_ =  mktTimeGrid[i - 1];
		}
		LAMathLeastSquareHybridHWDDProcess2 leastSquareSigma(fx0Vec, processHelperVec, sigma_target, t_);
		LAMathLeastSquareHybridHWDDProcess3 leastSquareBeta(fx0Vec, processHelperVec, beta_target, t_);
		
		LAQuantLibBoundaryConstraint bcSigma(boundaryMinSigma, boundaryMaxSigma);
		LAQuantLibBoundaryConstraint bcBeta(boundaryMinBeta, boundaryMaxBeta);

		LAQuantLibArray x(1, sigma[i]);
		LAQuantLibArray y(1, beta[i]);
		// probrem
		LAQuantLibProblem problemSigma(leastSquareSigma, bcSigma, x);
		LAQuantLibProblem problemBeta(leastSquareBeta, bcBeta, y);

		// optMethd
		std::shared_ptr<LAQuantLibOptimizationMethod> pOptMethodSigma;
		std::shared_ptr<LAQuantLibOptimizationMethod> pOptMethodBeta;
		if (optMethodType_sigmabeta == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
		{
			pOptMethodSigma = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibConjugateGradient());
			pOptMethodBeta =  std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibConjugateGradient());
		}
		else if (optMethodType_sigmabeta == CALIB_STEEPEST_DESCENT_METHOD)
		{
			pOptMethodSigma = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSteepestDescent());
			pOptMethodBeta = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSteepestDescent());
		}
		else if (optMethodType_sigmabeta == CALIB_SIMPLEX_METHOD)
		{
			pOptMethodSigma = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSimplex(0.0001));
			pOptMethodBeta = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSimplex(0.0001));
		}
		else if (optMethodType_sigmabeta == CALIB_LEVENBERG_MARQUARDT_METHOD)
		{
            double levenbergMarquardtEpsfcn = 1.0e-8;     // parameters specific for Levenberg-Marquardt
            double levenbergMarquardtXtol   = 1.0e-8;     //
            double levenbergMarquardtGtol   = 1.0e-8;     //
			pOptMethodSigma = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol));
			pOptMethodBeta = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol));
		}
		else
		{
			LAString msg = "Bad optMethod , " + optMethodType_sigmabeta;
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		if (optSBFlag)
		{
			LAQuantLibEndCriteria::Type endCriteriaResultSigma = pOptMethodSigma->minimize(problemSigma, endCriteria_sigmabeta);
			LAQuantLibEndCriteria::Type endCriteriaResultBeta = pOptMethodBeta->minimize(problemBeta, endCriteria_sigmabeta);
		}
		
		sigma_ave[i] = processHelperVec[0]->calcSigma();
		skew_ave[i] = processHelperVec[0]->calcSkew();

		// get model volatility
		DoubleArray value(2); 
		value[0] = sigma_ave[i]; 
		value[1] = skew_ave[i];
		const DoubleMatrix premMat(1, dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_OPTIONPREMIUMS, ISNOTNULL).get()).get());
		const DoubleMatrix strikeMat_modelVol(1, dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_STRIKES, ISNOTNULL).get()).get());
		const DoubleMatrix weightMat_modelVol(1, dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_WEIGHTS, ISNOTNULL).get()).get());
		const BoolVector &cpFlag = dynamic_cast<const LADataBools &>(refData.get(i).getData(PRICING_DATA_ISCALLS, ISNOTNULL).get()).get();
		IntVector int_cpFlag(cpFlag.size(), -1);
		for (unsigned int j = 0; j < int_cpFlag.size(); ++j)
		{
			if (cpFlag[j])
			{
				int_cpFlag[j] = 1;
			}
		}
		const vector<IntVector> cpFlagMat(1, int_cpFlag);
		LAMathLeastSquareBlackDD blackDD(spotFX, curve0_d, curve0_f, tVec, strikeMat_modelVol, premMat, weightMat_modelVol, cpFlagMat); 
		try 
		{
			modelVolDD[i] = blackDD.getBlackVol(value);
		}
		catch (LACoreNumericalError e) 
		{
			modelVolDD[i] = DoubleArray(premMat.size()*premMat[0].size(), DBL_MAX);
			//The calculation of the model implied volatility was failed, 
			//because the premium of blackDD model is not in the domain of the black implied vol function.
		}
	}
	//unsigned int pos;
	//LAAlgorithm::locate<DoubleVector, double>(ddParams.mT, mktTimeGrid.back(), ddParams.mT.size(), pos);
	unsigned int pos = lower_bound(ddParams.mT.begin(), ddParams.mT.end(), mktTimeGrid.back()) - ddParams.mT.begin();
	if (pos > 1)
	{
		----pos;
		const double l_sigma = ddParams.mSigma[pos];
		const double l_beta = ddParams.mBeta[pos];
		for (unsigned int i = pos + 1; i < ddParams.mT.size(); ++i)
		{
			ddParams.mSigma[i] = l_sigma;
			ddParams.mBeta[i] = l_beta;
		}
	}
	// set calibrated param
	vole.remove(PRICING_DATA_AVERAGEMARKETVOLDD);
	vole.add(PRICING_DATA_AVERAGEMARKETVOLDD, new LADataDoubles(sigma_ave));
	vole.remove(PRICING_DATA_AVERAGEMARKETSKEWDD);
	vole.add(PRICING_DATA_AVERAGEMARKETSKEWDD, new LADataDoubles(skew_ave));
	vole.remove(PRICING_DATA_MODELVOLDD);
	vole.add(PRICING_DATA_MODELVOLDD, new LADataDoubleMatrix(modelVolDD));

	vole.remove(PRICING_DATA_FXVOLGRIDS);
	vole.add(PRICING_DATA_FXVOLGRIDS, new LADataDoubles(ddParams.mSigma));
	vole.remove(PRICING_DATA_FXBETAGRIDS);
	vole.add(PRICING_DATA_FXBETAGRIDS, new LADataDoubles(ddParams.mBeta));
}