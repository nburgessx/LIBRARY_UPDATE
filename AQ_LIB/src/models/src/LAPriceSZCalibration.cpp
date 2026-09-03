//  2008, AlgoQuantHub.
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceSZCalibration.h"
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
//#include "LAQuantLibCostFunction.h"
//#include "LAQuantLibConstraint.h"
#include "LAQuantLibEndCriteria.h"
#include "LAQuantLibProblem.h"
#include "LAQuantLibOptimizationMethod.h"
#include "LAQuantLibConjugateGradient.h"
#include "LAQuantLibSteepestDescent.h"
#include "LAQuantLibSimplex.h"
#include "LAQuantLibLevenbergMarquardt.h"
#include "LAQuantLibBoundaryConstraint.h"
#include "LAMathLeastSquareSZ.h"

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
LAPriceSZCalibration::LAPriceSZCalibration()
	: LACoreProcedure()
{
}

/*!
@brief copy constructor
*/
LAPriceSZCalibration::LAPriceSZCalibration(const LAPriceSZCalibration& p)
	: LACoreProcedure(p)
{

}

/*!
@brief destructor

*/
LAPriceSZCalibration::~LAPriceSZCalibration()
{
}
/*!
@brief  Check this class ID is the same or not

@param[in] id function ID
@return true or false
*/
bool
LAPriceSZCalibration::isTypeOf(function_t id) const
{
	return (id == FN_IR_SZCALIBRATION ? true :
		LACoreProcedure::isTypeOf(id));
}
/*!
@brief  Copy this class

@return pointer to copied object
*/
LACoreFunctionBase*
LAPriceSZCalibration::clone() const
{
	try
	{
		return new LAPriceSZCalibration();
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
LAPriceSZCalibration::getType() const
{
	return FN_IR_SZCALIBRATION;
}

/*!
@brief register dataValues that this class uses

@param[in, out] dm data master
*/
void
LAPriceSZCalibration::registerData(LAPriceDataManager& dm) const
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
LAPriceSZCalibration::calibrateModel(const LADate& basedate,
	LAObject& object,
	const LADataProcedure& att) const
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
LAPriceSZCalibration::calibSZ(const LADate& basedate,
	LAObject& object,
	const LADataProcedure& att) const
{
	const LAObject &calibinfo = dynamic_cast<LADataReference &>(object.getData(PRICING_DATA_SZPARAMSCALIBINFO, ISNOTNULL).get()).get().get();
	LAObject& vole = dynamic_cast<LADataReference &>(object.getData(PRICING_DATA_SDEINFO, ISNOTNULL).get()).get().get();
	// calib flag
	const bool optFlag = dynamic_cast<const LADataBool &>(calibinfo.getData(CALIBRATION_DATA_OPT_PARAMETER_FLAG, ISNOTNULL).get()).get();

	const LADataMultiReference &refData = dynamic_cast<LADataMultiReference &>(object.getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL).get());
	unsigned int gridSize = refData.getSize();
	LADataInstance* pDataInstance = object.getDataInstance();
	// create domestic curve
	const LAObject &yldentity_d = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_DOMESTICCURVE, ISNOTNULL).get()).get().get();
	const LAString &yldname_d = dynamic_cast<const LADataString &>(yldentity_d.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	const LAString &curveType_d = dynamic_cast<const LADataString &>(object.getData(PRICING_DATA_DOMESTICCURVETYPE, ISNOTNULL).get()).get();
	LAMathYieldCurve yc_d(pDataInstance);
	yc_d.getData(IR_CALIBRATION_DATA_YIELDDATA, ISDEFINED).convertFromString(yldname_d);
	yc_d.setCurveType(curveType_d);
	yc_d.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	LAMathPathYieldCurve curve0_d(&yc_d, 0.0, ACT_365_ISDA);
	// create foreign curve
	const LAObject &yldentity_f = dynamic_cast<const LADataReference &>(object.getData(PRICING_DATA_FOREIGNCURVE, ISNOTNULL).get()).get().get();
	const LAString &yldname_f = dynamic_cast<const LADataString &>(yldentity_f.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();
	const LAString &curveType_f = dynamic_cast<const LADataString &>(object.getData(PRICING_DATA_FOREIGNCURVETYPE, ISNOTNULL).get()).get();
	LAMathYieldCurve yc_f(pDataInstance);
	yc_f.getData(IR_CALIBRATION_DATA_YIELDDATA, ISDEFINED).convertFromString(yldname_f);
	yc_f.setCurveType(curveType_f);
	yc_f.setInterpolation(FN_SPLINEINTERPOLATION_STR);
	LAMathPathYieldCurve curve0_f(&yc_f, 0.0, ACT_365_ISDA);
	// spot fx
	double spotFX = dynamic_cast<const LADataDouble &>(object.getData(PRICING_DATA_SPOTFX, ISNOTNULL).get()).get();

	// set up parameters calibration flag
	const bool calibflagBeta = dynamic_cast<const LADataBool&>(calibinfo.getData(CALIBRATION_DATA_OPT_BETA_FLAG, ISNOTNULL).get()).get();
	const bool calibflagTheta = dynamic_cast<const LADataBool&>(calibinfo.getData(CALIBRATION_DATA_OPT_THETA_FLAG, ISNOTNULL).get()).get();
	const bool calibflagKappa = dynamic_cast<const LADataBool&>(calibinfo.getData(CALIBRATION_DATA_OPT_KAPPA_FLAG, ISNOTNULL).get()).get();
	const bool calibflagEpsilon = dynamic_cast<const LADataBool&>(calibinfo.getData(CALIBRATION_DATA_OPT_EPSILON_FLAG, ISNOTNULL).get()).get();
	BoolVector calibFlag = { calibflagBeta, calibflagTheta, calibflagKappa, calibflagEpsilon };

	// set up parameters boundary
	const double boundaryMaxBeta = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_BETA, ISNOTNULL).get()).get();
	const double boundaryMinBeta = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_BETA, ISNOTNULL).get()).get();
	const double boundaryMaxTheta = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_THETA, ISNOTNULL).get()).get();
	const double boundaryMinTheta = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_THETA, ISNOTNULL).get()).get();
	const double boundaryMaxKappa = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_KAPPA, ISNOTNULL).get()).get();
	const double boundaryMinKappa = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_KAPPA, ISNOTNULL).get()).get();
	const double boundaryMaxEpsilon = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_EPSILON, ISNOTNULL).get()).get();
	const double boundaryMinEpsilon = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_EPSILON, ISNOTNULL).get()).get();
	DoubleVector boundaryMax = { boundaryMaxBeta, boundaryMaxTheta, boundaryMaxKappa, boundaryMaxEpsilon };
	DoubleVector boundaryMin = { boundaryMinBeta, boundaryMinTheta, boundaryMinKappa, boundaryMinEpsilon };

	//opt method
	LAString optMethodType_sz = dynamic_cast<const LADataString &>(calibinfo.getData(CALIBRATION_DATA_OPT_METHOD_TYPE, ISNOTNULL).get()).get();

	// set up end criteria
	const int maxIteration_sz = dynamic_cast<const LADataInt &>(calibinfo.getData(CALIBRATION_DATA_MAX_ITERATION, ISNOTNULL).get()).get();
	const int maxStationaryStateIteration_sz = dynamic_cast<const LADataInt &>(calibinfo.getData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, ISNOTNULL).get()).get();
	const double rootEpsilon_sz = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_ROOT_EPSILON, ISNOTNULL).get()).get();
	const double functionEpsilon_sz = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_FUNCTION_EPSILON, ISNOTNULL).get()).get();
	const double gradientNormEpsilon_sz = dynamic_cast<const LADataDouble &>(calibinfo.getData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, ISNOTNULL).get()).get();

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
			timegrid_start[i+1] = dynamic_cast<const LADataDouble &>(refData.get(i).getData(PRICING_DATA_FXTERM, ISNOTNULL).get()).get();
		}
		timegrid_end[i] = dynamic_cast<const LADataDouble &>(refData.get(i).getData(PRICING_DATA_FXTERM, ISNOTNULL).get()).get();
		//vol, premium, strike, weight
		volMat[i] = dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_VOLATILITYS, ISNOTNULL).get()).get();
		premMat[i] = dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_OPTIONPREMIUMS, ISNOTNULL).get()).get();
		strikeMat[i] = dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_STRIKES, ISNOTNULL).get()).get();
		weightMat[i] = dynamic_cast<const LADataDoubles &>(refData.get(i).getData(PRICING_DATA_WEIGHTS, ISNOTNULL).get()).get();
		//parameters
		initialBetaVec[i] = dynamic_cast<const LADataDouble &>(refData.get(i).getData(PRICING_DATA_INITIALBETA, ISNOTNULL).get()).get();
		initialThetaVec[i] = dynamic_cast<const LADataDouble &>(refData.get(i).getData(PRICING_DATA_INITIALTHETA, ISNOTNULL).get()).get();
		initialKappaVec[i] = dynamic_cast<const LADataDouble &>(refData.get(i).getData(PRICING_DATA_INITIALKAPPA, ISNOTNULL).get()).get();
		initialEpsilonVec[i] = dynamic_cast<const LADataDouble &>(refData.get(i).getData(PRICING_DATA_INITIALEPSILON, ISNOTNULL).get()).get();
		initialparameters[i] = {initialBetaVec[i], initialThetaVec[i], initialKappaVec[i], initialEpsilonVec[i] };
	}

	//Initial Vol
	double diffVolV0 = dynamic_cast<const LADataDouble &>(object.getData(PRICING_DATA_DIFFVOLV0, ISNOTNULL).get()).get();
	DoubleVector ATMVol_shortestterm = volMat[0];
	double vol0 = ATMVol_shortestterm[2] - diffVolV0;
	
	//Rho
	double rho = dynamic_cast<const LADataDouble &>(object.getData(PRICING_DATA_SDEVOLCORRELATIONS, ISNOTNULL).get()).get();

	//------set up functions
	//set up CostFuncSZ
	LAMathLeastSquareCostFuncSZ* costfunc_ = new LAMathLeastSquareCostFuncSZ(spotFX,
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
	LAMathLeastSquareConstraintSZ* constraint_ = new LAMathLeastSquareConstraintSZ(boundaryMaxBeta, boundaryMinBeta,
																			boundaryMaxTheta, boundaryMinTheta,
																			boundaryMaxKappa, boundaryMinKappa,
																			boundaryMaxEpsilon, boundaryMinEpsilon,
																			calibFlag
																			);
	//set upopt method
	LAString type(optMethodType_sz.toUpper());
	std::shared_ptr<LAQuantLibOptimizationMethod> opt_method;

	if (type == "NON_LINEAR_CONJUGATE_GRADIENT_METHOD")
	{
		opt_method = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibConjugateGradient());
	}
	else if (type == "STEEPEST_DESCENT_METHOD")
	{
		opt_method = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSteepestDescent());
	}
	else if (type == "SIMPLEX_METHOD")
	{
		opt_method = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibSimplex(0.1));
	}
	else if (type == "LEVENBERGÅ|MARQUARDT_METHOD")
	{
		double levenbergMarquardtEpsfcn = 0.001;
		double levenbergMarquardtXtol = 0.001;
		double levenbergMarquardtGtol = 0.001;
		opt_method = std::shared_ptr<LAQuantLibOptimizationMethod>(new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn, levenbergMarquardtXtol, levenbergMarquardtGtol));
	}
	else
	{	
		const LAString msg = "Invalid optimization method: " + type;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	//set up end criteria
	LAQuantLibEndCriteria ec_ = LAQuantLibEndCriteria(maxIteration_sz,
													maxStationaryStateIteration_sz,
													rootEpsilon_sz,
													functionEpsilon_sz,
													gradientNormEpsilon_sz
													);

	// check lack ofinput 
	if (initialBetaVec.empty() || initialThetaVec.empty() || initialKappaVec.empty() || initialEpsilonVec.empty())
	throw LACoreInvalidData("initial value size is not 4!", __FILE__, __LINE__);

	size_t o = std::count(calibFlag.begin(), calibFlag.end(), true);
	if (o == 0)
		throw LACoreInvalidData("Calibration target size is 0!", __FILE__, __LINE__);

	// optimize parameters
	LAQuantLibArray x(o);
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
					throw LACoreInvalidData("the initial value is out of boundary range", __FILE__, __LINE__);
				counter++;
			}
		}

		costfunc_->Set_t2f_LS(timegrid_end[i], strikeMat[i], weightMat[i], volMat[i]);
		LAQuantLibProblem* opt_problem = new LAQuantLibProblem(*costfunc_, *constraint_, x);


		if (optFlag == true)
		{
			LAQuantLibEndCriteria::Type endCriteriaResult = opt_method->minimize(*opt_problem, ec_);
		}
		LAQuantLibArray xMinCalculated = opt_problem->currentValue();

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
	DoubleVector timegrid_fx = dynamic_cast<LADataDoubles &>(vole.getData(PRICING_DATA_FXTIMEGRIDS, ISNOTNULL).get()).get();
	//step interpolation for optimized params
	DoubleVector BetaVec = stepinterpolation_SZ(optimizedBetaVec, timegrid_fx, timegrid_end);
	DoubleVector ThetaVec = stepinterpolation_SZ(optimizedThetaVec, timegrid_fx, timegrid_end);
	DoubleVector KappaVec = stepinterpolation_SZ(optimizedKappaVec, timegrid_fx, timegrid_end);
	DoubleVector EpsilonVec = stepinterpolation_SZ(optimizedEpsilonVec, timegrid_fx, timegrid_end);

	vole.remove(PRICING_DATA_OPTIMIZEDBETA);
	vole.add(PRICING_DATA_OPTIMIZEDBETA, new LADataDoubles(BetaVec));
	vole.remove(PRICING_DATA_OPTIMIZEDTHETA);
	vole.add(PRICING_DATA_OPTIMIZEDTHETA, new LADataDoubles(ThetaVec));
	vole.remove(PRICING_DATA_OPTIMIZEDKAPPA);
	vole.add(PRICING_DATA_OPTIMIZEDKAPPA, new LADataDoubles(KappaVec));
	vole.remove(PRICING_DATA_OPTIMIZEDEPSILON);
	vole.add(PRICING_DATA_OPTIMIZEDEPSILON, new LADataDoubles(EpsilonVec));

	const DoubleArray v0Vec(timegrid_fx.size(), vol0);
	vole.add(PRICING_DATA_VOLV0, new LADataDoubles(v0Vec));
}

/*!
@brief step interpolation for optimized param

@param[in] calibrated param
@param[in] grid term
@param[in] calibration terget term
@param[out] interpolated param
*/
DoubleVector LAPriceSZCalibration::stepinterpolation_SZ(DoubleVector& param, DoubleVector& term_g, DoubleVector& term_calib) const
{
	unsigned int gridSize = term_g.size(), gridSize_calib = term_calib.size();
	DoubleVector interpoleted_param(gridSize);

	// check
	if (param.size() != gridSize_calib)
	{
		throw LACoreInvalidData("Calibrated Param size is not the same with calibration target term size  is wroing.", __FILE__, __LINE__);
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
