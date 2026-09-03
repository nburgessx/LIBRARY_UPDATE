//  2008, AlgoQuantHub.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLPriceSZCalibration.h"
#include "AQLObject.h"
#include "AQLDataProcedure.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectPool.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataFunction.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLMathDateCalculations.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLDataMatrix.h"
#include "AQLMathValuableEntity.h"
#include "AQLMathLeastSquareBlackDD.h"
//#include "AQLQuantLibCostFunction.h"
//#include "AQLQuantLibConstraint.h"
#include "AQLQuantLibEndCriteria.h"
#include "AQLQuantLibProblem.h"
#include "AQLQuantLibOptimizationMethod.h"
#include "AQLQuantLibConjugateGradient.h"
#include "AQLQuantLibSteepestDescent.h"
#include "AQLQuantLibSimplex.h"
#include "AQLQuantLibLevenbergMarquardt.h"
#include "AQLQuantLibBoundaryConstraint.h"
#include "AQLMathLeastSquareSZ.h"

#include "AQLPriceTargetFunction.h"
#include "AQLPriceLSTargetFunction.h"
#include "AQLPriceCashFlowGenerator.h"
#include "AQLOptimumBFGS.h"
#include <algorithm>
#include <float.h>

using namespace std;

///////////////////////////////////////////////////////////////////////
/*!
@brief default constructor
*/
AQLPriceSZCalibration::AQLPriceSZCalibration()
	: AQLCoreProcedure()
{
}

/*!
@brief copy constructor
*/
AQLPriceSZCalibration::AQLPriceSZCalibration(const AQLPriceSZCalibration& p)
	: AQLCoreProcedure(p)
{

}

/*!
@brief destructor

*/
AQLPriceSZCalibration::~AQLPriceSZCalibration()
{
}
/*!
@brief  Check this class ID is the same or not

@param[in] id function ID
@return true or false
*/
bool
AQLPriceSZCalibration::isTypeOf(function_t id) const
{
	return (id == FN_IR_SZCALIBRATION ? true :
		AQLCoreProcedure::isTypeOf(id));
}
/*!
@brief  Copy this class

@return pointer to copied object
*/
AQLCoreFunctionBase*
AQLPriceSZCalibration::clone() const
{
	try
	{
		return new AQLPriceSZCalibration();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
}

/*!
@brief Retern class type

@return class type
*/
function_t
AQLPriceSZCalibration::getType() const
{
	return FN_IR_SZCALIBRATION;
}

/*!
@brief register dataValues that this class uses

@param[in, out] dm data master
*/
void
AQLPriceSZCalibration::registerData(AQLPriceDataManager& dm) const
{
	dm.setData(PRICING_DATA_SKEWVOLCALIBINFO, DATA_REFERENCE);
	dm.setData(PRICING_DATA_DIFFVOLV0, DATA_DOUBLE);
	dm.setData(PRICING_DATA_VOLV0, DATA_DOUBLE);
	dm.setData(PRICING_DATA_SZPARAMSCALIBINFO, DATA_REFERENCE);
	dm.setData(PRICING_DATA_DOMESTICCURVETYPE, DATA_STRING);
	dm.setData(PRICING_DATA_FOREIGNCURVETYPE, DATA_STRING);
	dm.setData(PRICING_DATA_CALIBRATORENGINE, DATA_PROCEDURE);
	dm.setData(CALIBRATION_DATA_OPT_METHOD_TYPE, DATA_STRING);
	dm.setData(CALIBRATION_DATA_OPT_BETA_FLAG, DATA_BOOL);
	dm.setData(CALIBRATION_DATA_OPT_THETA_FLAG, DATA_BOOL);
	dm.setData(CALIBRATION_DATA_OPT_KAPPA_FLAG, DATA_STRING);
	dm.setData(CALIBRATION_DATA_OPT_EPSILON_FLAG, DATA_BOOL);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MAX_BETA, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MIN_BETA, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MAX_THETA, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MIN_THETA, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MAX_KAPPA, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MIN_KAPPA, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MAX_EPSILON, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_BOUNDARY_MIN_EPSILON, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_SMALL_STEPS, DATA_INT);
	dm.setData(CALIBRATION_DATA_MAX_ITERATION, DATA_INT);
	dm.setData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, DATA_INT);
	dm.setData(CALIBRATION_DATA_ROOT_EPSILON, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_FUNCTION_EPSILON, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, DATA_DOUBLE);
	dm.setData(CALIBRATION_DATA_OPT_PARAMETER_FLAG, DATA_BOOL);
	dm.setData(CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD, DATA_STRING);
	dm.setData(CALIB_STEEPEST_DESCENT_METHOD, DATA_STRING);
	dm.setData(CALIB_SIMPLEX_METHOD, DATA_STRING);
	dm.setData(CALIB_LEVENBERG_MARQUARDT_METHOD, DATA_STRING);
}

/*!
@brief Generate CashFlow

@param[in] basedate basedate
@param[in,out] leg evaluated Object
@param[in] att Data with estimation procedure class

@note basedate is not used in estimation
*/
void
AQLPriceSZCalibration::calibrateModel(const AQLDate& basedate,
	AQLObject& object,
	const AQLDataProcedure& att) const
{
	calibSZ(basedate, object, att);
}

/*!
@brief calibrate market skew vol

@param[in] basedate basedate
@param[in,out] leg evaluated Object
@param[in] att Data with estimation procedure class

@note basedate is not used in estimation
*/
void
AQLPriceSZCalibration::calibSZ(const AQLDate& basedate,
	AQLObject& object,
	const AQLDataProcedure& att) const
{
	const AQLObject &calibinfo = dynamic_cast<AQLDataReference &>(object.getData(PRICING_DATA_SZPARAMSCALIBINFO, ISNOTNULL).get()).get().get();
	AQLObject& vole = dynamic_cast<AQLDataReference &>(object.getData(PRICING_DATA_SDEINFO, ISNOTNULL).get()).get().get();
	// calib flag
	const bool optFlag = dynamic_cast<const AQLDataBool &>(calibinfo.getData(CALIBRATION_DATA_OPT_PARAMETER_FLAG, ISNOTNULL).get()).get();

	const AQLDataMultiReference &refData = dynamic_cast<AQLDataMultiReference &>(object.getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
	unsigned int gridSize = refData.getSize();
	AQLDataInstance* pDataInstance = object.getDataInstance();
	// create domestic curve
	const AQLObject &yldentity_d = dynamic_cast<const AQLDataReference &>(object.getData(PRICING_DATA_DOMESTICCURVE, ISNOTNULL).get()).get().get();
	const AQLString &yldname_d = dynamic_cast<const AQLDataString &>(yldentity_d.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	const AQLString &curveType_d = dynamic_cast<const AQLDataString &>(object.getData(PRICING_DATA_DOMESTICCURVETYPE, ISNOTNULL).get()).get();
	AQLMathYieldCurve yc_d(pDataInstance);
	yc_d.getData(IR_CALIBRATION_DATA_YIELDDATA, ISDEFINED).convertFromString(yldname_d);
	yc_d.setCurveType(curveType_d);
	yc_d.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	AQLMathPathYieldCurve curve0_d(&yc_d, 0.0, ACT_365_ISDA);
	// create foreign curve
	const AQLObject &yldentity_f = dynamic_cast<const AQLDataReference &>(object.getData(PRICING_DATA_FOREIGNCURVE, ISNOTNULL).get()).get().get();
	const AQLString &yldname_f = dynamic_cast<const AQLDataString &>(yldentity_f.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	const AQLString &curveType_f = dynamic_cast<const AQLDataString &>(object.getData(PRICING_DATA_FOREIGNCURVETYPE, ISNOTNULL).get()).get();
	AQLMathYieldCurve yc_f(pDataInstance);
	yc_f.getData(IR_CALIBRATION_DATA_YIELDDATA, ISDEFINED).convertFromString(yldname_f);
	yc_f.setCurveType(curveType_f);
	yc_f.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	AQLMathPathYieldCurve curve0_f(&yc_f, 0.0, ACT_365_ISDA);
	// spot fx
	double spotFX = dynamic_cast<const AQLDataDouble &>(object.getData(PRICING_DATA_SPOTFX, ISNOTNULL).get()).get();

	// set up parameters calibration flag
	const bool calibflagBeta = dynamic_cast<const AQLDataBool&>(calibinfo.getData(CALIBRATION_DATA_OPT_BETA_FLAG, ISNOTNULL).get()).get();
	const bool calibflagTheta = dynamic_cast<const AQLDataBool&>(calibinfo.getData(CALIBRATION_DATA_OPT_THETA_FLAG, ISNOTNULL).get()).get();
	const bool calibflagKappa = dynamic_cast<const AQLDataBool&>(calibinfo.getData(CALIBRATION_DATA_OPT_KAPPA_FLAG, ISNOTNULL).get()).get();
	const bool calibflagEpsilon = dynamic_cast<const AQLDataBool&>(calibinfo.getData(CALIBRATION_DATA_OPT_EPSILON_FLAG, ISNOTNULL).get()).get();
	BoolVector calibFlag = { calibflagBeta, calibflagTheta, calibflagKappa, calibflagEpsilon };

	// set up parameters boundary
	const double boundaryMaxBeta = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_BETA, ISNOTNULL).get()).get();
	const double boundaryMinBeta = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_BETA, ISNOTNULL).get()).get();
	const double boundaryMaxTheta = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_THETA, ISNOTNULL).get()).get();
	const double boundaryMinTheta = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_THETA, ISNOTNULL).get()).get();
	const double boundaryMaxKappa = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_KAPPA, ISNOTNULL).get()).get();
	const double boundaryMinKappa = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_KAPPA, ISNOTNULL).get()).get();
	const double boundaryMaxEpsilon = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_EPSILON, ISNOTNULL).get()).get();
	const double boundaryMinEpsilon = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_EPSILON, ISNOTNULL).get()).get();
	DoubleVector boundaryMax = { boundaryMaxBeta, boundaryMaxTheta, boundaryMaxKappa, boundaryMaxEpsilon };
	DoubleVector boundaryMin = { boundaryMinBeta, boundaryMinTheta, boundaryMinKappa, boundaryMinEpsilon };

	//opt method
	AQLString optMethodType_sz = dynamic_cast<const AQLDataString &>(calibinfo.getData(CALIBRATION_DATA_OPT_METHOD_TYPE, ISNOTNULL).get()).get();

	// set up end criteria
	const int maxIteration_sz = dynamic_cast<const AQLDataInt &>(calibinfo.getData(CALIBRATION_DATA_MAX_ITERATION, ISNOTNULL).get()).get();
	const int maxStationaryStateIteration_sz = dynamic_cast<const AQLDataInt &>(calibinfo.getData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, ISNOTNULL).get()).get();
	const double rootEpsilon_sz = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_ROOT_EPSILON, ISNOTNULL).get()).get();
	const double functionEpsilon_sz = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_FUNCTION_EPSILON, ISNOTNULL).get()).get();
	const double gradientNormEpsilon_sz = dynamic_cast<const AQLDataDouble &>(calibinfo.getData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, ISNOTNULL).get()).get();

	// set up vol, premium, strike, weight
	DoubleVector timegrid_start(gridSize);
	DoubleVector timegrid_end(gridSize);
	DoubleMatrix volMat(gridSize);
	DoubleMatrix premMat(gridSize);
	DoubleMatrix strikeMat(gridSize);
	DoubleMatrix weightMat(gridSize);
	DoubleVector initialBetaVec(gridSize);
	DoubleVector initialThetaVec(gridSize);
	DoubleVector initialKappaVec(gridSize);
	DoubleVector initialEpsilonVec(gridSize);
	DoubleMatrix initialparameters(gridSize);
	timegrid_start[0] = 0;
	for (unsigned int i = 0; i < gridSize; ++i)
	{
		if (i < gridSize - 1)
		{
			timegrid_start[i+1] = dynamic_cast<const AQLDataDouble &>(refData.get(i).getData(PRICING_DATA_FXTERM, ISNOTNULL).get()).get();
		}
		timegrid_end[i] = dynamic_cast<const AQLDataDouble &>(refData.get(i).getData(PRICING_DATA_FXTERM, ISNOTNULL).get()).get();
		//vol, premium, strike, weight
		volMat[i] = dynamic_cast<const AQLDataDoubles &>(refData.get(i).getData(PRICING_DATA_VOLATILITYS, ISNOTNULL).get()).get();
		premMat[i] = dynamic_cast<const AQLDataDoubles &>(refData.get(i).getData(PRICING_DATA_OPTIONPREMIUMS, ISNOTNULL).get()).get();
		strikeMat[i] = dynamic_cast<const AQLDataDoubles &>(refData.get(i).getData(PRICING_DATA_STRIKES, ISNOTNULL).get()).get();
		weightMat[i] = dynamic_cast<const AQLDataDoubles &>(refData.get(i).getData(PRICING_DATA_WEIGHTS, ISNOTNULL).get()).get();
		//parameters
		initialBetaVec[i] = dynamic_cast<const AQLDataDouble &>(refData.get(i).getData(PRICING_DATA_INITIALBETA, ISNOTNULL).get()).get();
		initialThetaVec[i] = dynamic_cast<const AQLDataDouble &>(refData.get(i).getData(PRICING_DATA_INITIALTHETA, ISNOTNULL).get()).get();
		initialKappaVec[i] = dynamic_cast<const AQLDataDouble &>(refData.get(i).getData(PRICING_DATA_INITIALKAPPA, ISNOTNULL).get()).get();
		initialEpsilonVec[i] = dynamic_cast<const AQLDataDouble &>(refData.get(i).getData(PRICING_DATA_INITIALEPSILON, ISNOTNULL).get()).get();
		initialparameters[i] = {initialBetaVec[i], initialThetaVec[i], initialKappaVec[i], initialEpsilonVec[i] };
	}

	//Initial Vol
	double diffVolV0 = dynamic_cast<const AQLDataDouble &>(object.getData(PRICING_DATA_DIFFVOLV0, ISNOTNULL).get()).get();
	DoubleVector ATMVol_shortestterm = volMat[0];
	double vol0 = ATMVol_shortestterm[2] - diffVolV0;
	
	//Rho
	double rho = dynamic_cast<const AQLDataDouble &>(object.getData(PRICING_DATA_SDEVOLCORRELATIONS, ISNOTNULL).get()).get();

	//------set up functions
	//set up CostFuncSZ
	AQLMathLeastSquareCostFuncSZ* costfunc_ = new AQLMathLeastSquareCostFuncSZ(spotFX,
																		vol0,
																		rho,
																		curve0_d,
																		curve0_f,
																		timegrid_start,
																		initialBetaVec,
																		initialThetaVec,
																		initialKappaVec,
																		initialEpsilonVec,
																		calibFlag
																		);

	//set up ConstraintSZ
	AQLMathLeastSquareConstraintSZ* constraint_ = new AQLMathLeastSquareConstraintSZ(boundaryMaxBeta, boundaryMinBeta,
																			boundaryMaxTheta, boundaryMinTheta,
																			boundaryMaxKappa, boundaryMinKappa,
																			boundaryMaxEpsilon, boundaryMinEpsilon,
																			calibFlag
																			);
	//set upopt method
	AQLString type(optMethodType_sz.toUpper());
	std::shared_ptr<AQLQuantLibOptimizationMethod> opt_method;

	if (type == "NON_LINEAR_CONJUGATE_GRADIENT_METHOD")
	{
		opt_method = std::shared_ptr<AQLQuantLibOptimizationMethod>(new AQLQuantLibConjugateGradient());
	}
	else if (type == "STEEPEST_DESCENT_METHOD")
	{
		opt_method = std::shared_ptr<AQLQuantLibOptimizationMethod>(new AQLQuantLibSteepestDescent());
	}
	else if (type == "SIMPLEX_METHOD")
	{
		opt_method = std::shared_ptr<AQLQuantLibOptimizationMethod>(new AQLQuantLibSimplex(0.1));
	}
	else if (type == "LEVENBERG-MARQUARDT_METHOD")
	{
		double levenbergMarquardtEpsfcn = 0.001;
		double levenbergMarquardtXtol = 0.001;
		double levenbergMarquardtGtol = 0.001;
		opt_method = std::shared_ptr<AQLQuantLibOptimizationMethod>(new AQLQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol));
	}
	else
	{	
		const AQLString msg = "Invalid optimization method: " + type;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	//set up end criteria
	AQLQuantLibEndCriteria ec_ = AQLQuantLibEndCriteria(maxIteration_sz,
													maxStationaryStateIteration_sz,
													rootEpsilon_sz,
													functionEpsilon_sz,
													gradientNormEpsilon_sz
													);

	// check lack ofinput 
	if (initialBetaVec.empty() || initialThetaVec.empty() || initialKappaVec.empty() || initialEpsilonVec.empty())
	throw AQLCoreInvalidData("initial value size is not 4!", __FILE__, __LINE__);

	size_t o = std::count(calibFlag.begin(), calibFlag.end(), true);
	if (o == 0)
		throw AQLCoreInvalidData("Calibration target size is 0!", __FILE__, __LINE__);

	// optimize parameters
	AQLQuantLibArray x(o);
	DoubleMatrix param(gridSize);
	size_t counter = 0;
	for (int i = 0; i < gridSize; i++)
	{
		counter = 0;
		for (size_t j = 0; j < 4; ++j)
		{
			if (calibFlag[j])
			{
				x[counter] = initialparameters[i][j];
				if (x[counter] < boundaryMin[j] || boundaryMax[j] < x[counter])
					throw AQLCoreInvalidData("the initial value is out of boundary range", __FILE__, __LINE__);
				counter++;
			}
		}

		costfunc_->Set_t2f_LS(timegrid_end[i], strikeMat[i], weightMat[i], volMat[i]);
		AQLQuantLibProblem* opt_problem = new AQLQuantLibProblem(*costfunc_, *constraint_, x);


		if (optFlag == true)
		{
			AQLQuantLibEndCriteria::Type endCriteriaResult = opt_method->minimize(*opt_problem, ec_);
		}
		AQLQuantLibArray xMinCalculated = opt_problem->currentValue();

		delete opt_problem;

		param[i].resize(4);
		counter = 0;
		for (size_t j = 0; j < 4; ++j)
		{
			if (calibFlag[j])
			{
				param[i][j] = xMinCalculated[counter];
				counter++;
			}
			else
			{
				param[i][j] = initialparameters[i][j];
			}
		}
	}
	delete costfunc_;
	delete constraint_;
	//delete opt_method;

	//set optimized parameters
	DoubleVector optimizedBetaVec(gridSize);
	DoubleVector optimizedThetaVec(gridSize);
	DoubleVector optimizedKappaVec(gridSize);
	DoubleVector optimizedEpsilonVec(gridSize);
	for (int i = 0; i < gridSize; i++)
	{
		optimizedBetaVec[i] = param[i][0];
		optimizedThetaVec[i] = param[i][1];
		optimizedKappaVec[i] = param[i][2];
		optimizedEpsilonVec[i] = param[i][3];
	}

	//time grid
	DoubleVector timegrid_fx = dynamic_cast<AQLDataDoubles &>(vole.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get()).get();
	//step interpolation for optimized params
	DoubleVector BetaVec = stepinterpolation_SZ(optimizedBetaVec, timegrid_fx, timegrid_end);
	DoubleVector ThetaVec = stepinterpolation_SZ(optimizedThetaVec, timegrid_fx, timegrid_end);
	DoubleVector KappaVec = stepinterpolation_SZ(optimizedKappaVec, timegrid_fx, timegrid_end);
	DoubleVector EpsilonVec = stepinterpolation_SZ(optimizedEpsilonVec, timegrid_fx, timegrid_end);

	vole.remove(PRICING_DATA_OPTIMIZEDBETA);
	vole.add(PRICING_DATA_OPTIMIZEDBETA, new AQLDataDoubles(BetaVec));
	vole.remove(PRICING_DATA_OPTIMIZEDTHETA);
	vole.add(PRICING_DATA_OPTIMIZEDTHETA, new AQLDataDoubles(ThetaVec));
	vole.remove(PRICING_DATA_OPTIMIZEDKAPPA);
	vole.add(PRICING_DATA_OPTIMIZEDKAPPA, new AQLDataDoubles(KappaVec));
	vole.remove(PRICING_DATA_OPTIMIZEDEPSILON);
	vole.add(PRICING_DATA_OPTIMIZEDEPSILON, new AQLDataDoubles(EpsilonVec));

	const DoubleArray v0Vec(timegrid_fx.size(), vol0);
	vole.add(PRICING_DATA_VOLV0, new AQLDataDoubles(v0Vec));
}

/*!
@brief step interpolation for optimized param

@param[in] calibrated param
@param[in] grid term
@param[in] calibration terget term
@param[out] interpolated param
*/
DoubleVector AQLPriceSZCalibration::stepinterpolation_SZ(DoubleVector& param, DoubleVector& term_g, DoubleVector& term_calib) const
{
	unsigned int gridSize = term_g.size(), gridSize_calib = term_calib.size();
	DoubleVector interpoleted_param(gridSize);

	// check
	if (param.size() != gridSize_calib)
	{
		throw AQLCoreInvalidData("Calibrated Param size is not the same with calibration target term size  is wroing.", __FILE__, __LINE__);
	}

	for (int i = 0; i < gridSize; i++)
	{
		for (int j = 0; j < gridSize_calib; j++) 
		{
			if (term_g[i] <= term_calib[j])
			{
				interpoleted_param[i] = param[j];
				break;
			}
		}
		if (term_g[i] > term_calib[gridSize_calib-1]) interpoleted_param[i] = param[gridSize_calib-1];
	}
	
	return interpoleted_param;	
}
