
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathCorrelation.h"
#include "LAMathCorrelationLMMDisc.h"
#include "LAPriceLMMCalibration.h"
#include "LAMathVolatilityLMMDiscModel.h"
#include "LAMathSwaptionVolLMM.h"
#include "LAMathCapFloor.h"
#include "LAMathLeastSquareCapFloor.h"
#include "LAMathLeastSquareSwaptionVol.h"
#include "LAMathLeastSquareSwaptionCapFloor.h"
#include "LAMathBoundaryConstraintVolLMM.h"
#include "AQLObject.h"
#include "AQLDataProcedure.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
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
#include "LAMathDateCalculations.h"
#include "LAPriceCFGenUtility.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLMathValuableEntity.h"

#include "LAQuantLibEndCriteria.h"
#include "LAQuantLibProblem.h"
#include "LAQuantLibConjugateGradient.h"
#include "LAQuantLibSteepestDescent.h"
#include "LAQuantLibSimplex.h"
#include "LAQuantLibLevenbergMarquardt.h"
#include "LAQuantLibProblem.h"
#include "LAModelUtilities.h"

#include "AQLPriceTargetFunction.h"
#include "AQLPriceLSTargetFunction.h"
#include "LAPriceCashFlowGenerator.h"
#include <algorithm>

#include "LAMathInterpolationUtilities.h"

using namespace std;

class IRCalibLSTool
{
public:
	// constructor
	explicit IRCalibLSTool(AQLDataDoubles* pAttr) : mpAttr(pAttr) {;}
	/*!
		@brief set up parameter for function pointed by a member variable
	*/
	void operator = (const DoubleArray& param) const
	{
		mpAttr->set(param);
	};
    void operator = (const double param) const
    {
        mpAttr->set(param, 0);
    };
	AQLDataDoubles* mpAttr; // pointer to AQLDataDoubles
};



///////////////////////////////////////////////////////////////////////
/*!
    @brief default constructor
*/
LAPriceLMMCalibration::LAPriceLMMCalibration()
: AQLCoreProcedure()
{
}
/*!
    @brief destructor

*/
LAPriceLMMCalibration::~LAPriceLMMCalibration()
{
}
/*!
    @brief  Check this class ID is the same or not

	@param[in] id function ID
	@return true or false
*/
bool
LAPriceLMMCalibration::isTypeOf(function_t id) const
{
	return (id == FN_IR_LMMCALIBRATION ? true :
						AQLCoreProcedure::isTypeOf(id));
}
/*!
    @brief  Copy this class

	@return pointer to copied object
*/
AQLCoreFunctionBase*
LAPriceLMMCalibration::clone() const
{
    try
	{
        return new LAPriceLMMCalibration();
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
LAPriceLMMCalibration::getType() const
{
	return FN_IR_LMMCALIBRATION;
}

/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master
*/
void
LAPriceLMMCalibration::registerData(AQLPriceDataManager& dm) const
{
	dm.setData(PRICING_DATA_CALIBRATORENGINE,		DATA_PROCEDURE);
	dm.setData(CALIBRATION_DATA_CALIBRATIONDATA,				DATA_MULTIREFERENCE);
	dm.setData(PRICING_DATA_SDEINFO,				DATA_REFERENCE);

}

/*!
	@brief calibration

	@param[in] basedate basedate
	@param[in,out] leg evaluated Object
	@param[in] att Data with estimation procedure class

	@note basedate is not used in estimation
*/
void
LAPriceLMMCalibration::calibrateModel(const AQLDate& basedate,
							 AQLObject& object,
							 const AQLDataProcedure& att) const
{

	(void)basedate; (void)att;

	AQLDataHolder* dh;
	dh = &(object.getData("CalibInfoName", ISNOTNULL));
	const AQLString& calibInfoName = dynamic_cast<AQLDataString &>(dh->get());
	const AQLObject &calibInfo = object.getDataInstance()->getObjectPool().getObject(calibInfoName, ENCHKTYPE_ISDEFINED).get();

	dh = &(object.getData(PRICING_DATA_SDEINFO,ISNOTNULL));
	AQLObject& models = dynamic_cast<AQLDataReference&>(dh->get()).get().get();
	const AQLString& calibIDName = dynamic_cast<AQLDataString &>(models.getData(CALIBRATION_DATA_NAME, ISNOTNULL).get()).get();

    DoubleVector paramV = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(CALIBRATION_DATA_PARAM_V, ISNOTNULL).get()).get();
    DoubleVector paramF = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(CALIBRATION_DATA_PARAM_F, ISNOTNULL).get()).get();
    DoubleVector tenorG = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(CALIBRATION_DATA_TENOR_G, ISNOTNULL).get()).get();
    DoubleVector paramG = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(CALIBRATION_DATA_PARAM_G, ISNOTNULL).get()).get();
	const AQLString interGStr = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_INTERPOLATION_G, ISNOTNULL).get()).get();
	const AQLString frequencyG = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_FREQUENCY_G, ISNOTNULL).get()).get();

	bool doCalibration = true;
	const AQLDataHolder* temp_ah = &calibInfo.getData(CALIBRATION_DATA_DO_OPT_FLAG);
	if(temp_ah->isDefined() && !temp_ah->isNull()){
		doCalibration = dynamic_cast<const AQLDataString&>(temp_ah->get()).get() == "TRUE";
	}

	if(!doCalibration){
		// paramF.size() - 1 : Because we don't need T_max;
		paramF.resize(8);
		setResult(models, paramV, paramF, paramG, tenorG);
		return;
	}

	const bool is_extratenor_calib = dynamic_cast<const AQLDataBool &>(calibInfo.getData(CALIBRATION_DATA_IS_EXTRATENOR_CALIB, ISNOTNULL).get()).get();

    // set cannonical T
	const AQLString canonicalFreq = dynamic_cast<const AQLDataString &>(calibInfo.getData(PRICING_DATA_CALIBCANONICAL_FREQ, ISNOTNULL).get()).get();
    DoubleVector tenor = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_CALIBCANONICAL_T, ISNOTNULL).get()).get();
    DoubleVector tenor_30_360 = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(PRICING_DATA_CALIBCANONICAL_T_30_360, ISNOTNULL).get()).get();
    BoolVector tenor_extraflag = dynamic_cast<const AQLDataBools &>(calibInfo.getData(PRICING_DATA_CALIBCANONICAL_T_EXTRAFLAG, ISNOTNULL).get()).get();
	DoubleVector tenor_vollmm, tenor_30_360_vollmm;

	if(is_extratenor_calib)
	{
		DoubleVector::iterator tenor_copy_begin = tenor.begin();
		if (tenor[0] == 0.0)
		{
			tenor_copy_begin++;
		}
		DoubleVector::iterator tenor_30_360_copy_begin = tenor_30_360.begin();
		if (tenor_30_360[0] == 0.0)
		{
			tenor_30_360_copy_begin++;
		}
		std::copy(tenor_copy_begin, tenor.end(), std::back_inserter(tenor_vollmm));
		std::copy(tenor_30_360_copy_begin, tenor_30_360.end(), std::back_inserter(tenor_30_360_vollmm));
	}
	else
	{
		unsigned int tenor_copy_start = 0;
		if (tenor[0] == 0.0)
		{
			tenor_copy_start++;
		}
		unsigned int tenor_30_360_copy_start = 0;
		if (tenor_30_360[0] == 0.0)
		{
			tenor_30_360_copy_start++;
		}
		for(unsigned int i = tenor_copy_start; i < tenor.size(); i++)
		{
			if (!tenor_extraflag[i])
			{
				tenor_vollmm.push_back(tenor[i]);
			}
		}
		for(unsigned int i = tenor_30_360_copy_start; i < tenor_30_360.size(); i++)
		{
			if (!tenor_extraflag[i])
			{
				tenor_30_360_vollmm.push_back(tenor_30_360[i]);
			}
		}
	}



	int numSmallSteps = dynamic_cast<const AQLDataInt &>(calibInfo.getData(CALIBRATION_DATA_NUM_SMALL_STEPS, ISNOTNULL).get()).get();

	// LAMathVolatilityLMMDiscModel
	std::shared_ptr<AQLInterpolationBase> interG;
	// If method G input frequency is not same as canonical frequency, method G will be interpolated.
	if (canonicalFreq != frequencyG)
	{
		interG = LAMathInterpolationUtilities::createInterpolation(interGStr);
	}
	LAMathVolatilityLMMDiscModel *volLMM = new LAMathVolatilityLMMDiscModel(paramV, paramF, tenorG, paramG, interG, tenor_30_360_vollmm, tenor_vollmm, numSmallSteps);

	//get init curve
	dh = &(models.getData(PRICING_DATA_CURVEID,ISNOTNULL));
	AQLObject& yldentity = dynamic_cast<AQLDataReference &>(dh->get()).get().get();
	dh = &(yldentity.getData(CALIBRATION_DATA_NAME,ISNOTNULL));
	AQLString yldname = dynamic_cast<AQLDataString &>(dh->get()).get();

	AQLDataInstance* pDataInstance = object.getDataInstance();
	//forecast curve
	const AQLString forecastCurve = dynamic_cast<const AQLDataString &>(calibInfo.getData(PRICING_DATA_FORECASTCURVE, ISNOTNULL).get()).get();
	LAMathYieldCurve *tmpCurveF = new LAMathYieldCurve(pDataInstance, &forecastCurve);
	tmpCurveF->getData(IR_CALIBRATION_DATA_YIELDDATA,ISDEFINED).convertFromString(yldname);
	tmpCurveF->setInterpolation(FN_SPLINEINTERPOLATION_STR);
	LAMathPathYieldCurve* curve0_F = new LAMathPathYieldCurve(tmpCurveF,0.0,ACT_365_ISDA);
	//discount curve
	const AQLString discountCurve = dynamic_cast<const AQLDataString &>(calibInfo.getData(PRICING_DATA_DISCOUNTCURVE, ISNOTNULL).get()).get();
	LAMathYieldCurve *tmpCurveD = new LAMathYieldCurve(pDataInstance, &discountCurve);
	tmpCurveD->getData(IR_CALIBRATION_DATA_YIELDDATA,ISDEFINED).convertFromString(yldname);
	tmpCurveD->setInterpolation(FN_SPLINEINTERPOLATION_STR);
	LAMathPathYieldCurve* curve0_D = new LAMathPathYieldCurve(tmpCurveD,0.0,ACT_365_ISDA);

	dh = &(object.getData(CALIBRATION_DATA_CALIBRATIONDATA, ISNOTNULL));
	const AQLDataMultiReference& attrdata = dynamic_cast<AQLDataMultiReference&>(dh->get());
	AQLObject& capEntity = attrdata.get(0).get();
	AQLObject& swaptionEntity = attrdata.get(1).get();

	const AQLStringVector &capTermVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_CAP_TERM, ISNOTNULL).get()).get();
	const AQLStringVector &capTenorVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_CAP_TENOR, ISNOTNULL).get()).get();
	const DoubleVector &capBlackVolVec = dynamic_cast<const AQLDataDoubles &>(capEntity.getData(AQLString("CAP_") + IR_CALIBRATION_DATA_BLACKVOLATILITY, ISNOTNULL).get()).get();
	const AQLStringVector &optionMatVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL).get()).get();
	const AQLStringVector &swapTenorVec = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(IR_CALIBRATION_DATA_SWAPTENOR, ISNOTNULL).get()).get();
	const DoubleMatrix &swaptionVolWeightMat = dynamic_cast<const AQLDataDoubleMatrix &>(calibInfo.getData(IR_CALIBRATION_DATA_SWAPTIONVOLWEIGHT, ISNOTNULL).get()).get();
	AQLString swaptionVolType = dynamic_cast<const AQLDataString &>(calibInfo.getData( IR_CALIBRATION_DATA_SWAPTIONVOLTYPE, ISNOTNULL ).get()).get();
	double swaptionVolForwardShift = dynamic_cast<const AQLDataDouble &>(calibInfo.getData( IR_CALIBRATION_DATA_SWAPTIONVOL_MARKET_FORWARDSHIFT, ISNOTNULL ).get()).get();
	const DoubleMatrix &swaptionVolMat = dynamic_cast<const AQLDataDoubleMatrix &>(swaptionEntity.getData( AQLString("SWAPTION_") + IR_CALIBRATION_DATA_BLACKVOLATILITY, ISNOTNULL ).get()).get();	
	
	// set vol type
	swaptionVolType.toUpper();
	LAMathSwaptionVolLMM::volType volType;
	if( swaptionVolType == "NORMAL" )
	{
		volType = LAMathSwaptionVolLMM::Normal;
	}
	else if( swaptionVolType == "BLACK" )
	{
		volType = LAMathSwaptionVolLMM::Black;
	}
	else
	{
		AQLString msg = "Wrong swaption vol type! : " + swaptionVolType + "?";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	// set up target
	AQLString costFuncTarget = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_OPT_COST_FUNC_TARGET, ISNOTNULL).get()).get();
	LAMathLeastSquareSwaptionVol::target target1;
	LAMathLeastSquareCapFloor::target target2;

	if( costFuncTarget == "VOLATILITY" )
	{
		target1 = LAMathLeastSquareSwaptionVol::Volatility;
		target2 = LAMathLeastSquareCapFloor::Volatility;
	}
	else if( costFuncTarget == "PREMIUM" )
	{
		target1 = LAMathLeastSquareSwaptionVol::Premium;
		target2 = LAMathLeastSquareCapFloor::Premium;
	}
	else
	{
		AQLString msg = "Wrong costFuncTarget! : " + costFuncTarget + "?";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	// set up cap pricing class
	vector<LAMathCapFloorVolLMMDiscModel*> capLMMs;
	vector<LAMathCapFloor*> capBlackVols;
	unsigned int capGridSize = capTermVec.size();
	for (unsigned int i = 0; i < capGridSize; ++i)
	{
		AQLString capTerm = capTermVec[i];
		AQLString capTenor = capTenorVec[i];

		AQLString prefix = "CAP_";
		AQLString postfix = "_" + capTerm + "_" + capTenor;
		DoubleVector T_pay_cap = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(prefix + IR_CALIBRATION_DATA_T_PAY + postfix, ISNOTNULL).get()).get();
		DoubleVector T_fix_cap = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(prefix + IR_CALIBRATION_DATA_T_FIX + postfix, ISNOTNULL).get()).get();
		DoubleVector tau_L_cap = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(prefix + IR_CALIBRATION_DATA_TAU_L + postfix, ISNOTNULL).get()).get();
		DoubleVector tau_cap = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(prefix + IR_CALIBRATION_DATA_TAU + postfix, ISNOTNULL).get()).get();

		double capBlackVol = capBlackVolVec[i];

		LAMathCapFloorVolLMMDiscModel *cap_vol = new LAMathCapFloorVolLMMDiscModel(
				*curve0_F, *volLMM,
				T_fix_cap, T_pay_cap, tau_L_cap, tau_cap
				);
		LAMathCapFloorBlackVol *cap_blackvol = new LAMathCapFloorBlackVol(
				*curve0_F, capBlackVol,
				T_fix_cap, T_pay_cap, tau_L_cap, tau_cap
				);
		capLMMs.push_back(cap_vol);
		capBlackVols.push_back(cap_blackvol);
	}

	// set up correlation class for votatility calibration
	const AQLObject &optCorEntity = dynamic_cast<const AQLDataReference &>(calibInfo.getData(CALIBRATION_DATA_LMM_CALIB_CORRELATION, ISNOTNULL).get()).get().get();
	const DoubleMatrix &corFactor = dynamic_cast<const LAMathCorrelation &>(optCorEntity).getCorFactors().get();
	// data of t = 0 will be inserted
	DoubleVector tenor_corlmm;
	if (*tenor_vollmm.begin() != 0.)
	{
		tenor_corlmm.push_back(0.);
		for(unsigned int i = 0; i < tenor_vollmm.size() - 1; i++)
		{
			tenor_corlmm.push_back(tenor_vollmm[i]);
		}
	}
	else
	{
		tenor_corlmm = tenor_vollmm;
	}
		

	// data of t = 0 will be inserted
	DoubleMatrix corFactorCalib(1, corFactor[0]);
	// corFactor strats with minimum grid but tenor_exflag is not (start at t = 0.)
	unsigned int cor_gridNum = std::min(corFactor.size() + 1, tenor_extraflag.size() - 1);
	for(unsigned int i = 1; i < cor_gridNum; i++)
	{
		if (!tenor_extraflag[i])
		{
			corFactorCalib.push_back(corFactor[i - 1]);
		}
	}
	const LAMathCorrelationLMM *optCor = new LAMathCorrelationLMMDisc(corFactorCalib, tenor_corlmm);

	// set up swaption pricing class
	vector<LAMathSwaptionVolLMMDiscModel*> swaptionLMMs;
	unsigned int optionMatSize = optionMatVec.size();
	unsigned int swapTenorSize = swapTenorVec.size();

	double Q = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_VOL_SKEW, ISNOTNULL).get()).get();
	double constShift = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_VOL_CONSTSHIFT, ISNOTNULL).get()).get();
	AQLString swaptionApproxMethod = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_SWAPTION_APPROX_MEHOD, ISNOTNULL).get()).get();

	DoubleMatrix swaptionTargetValueMat(optionMatSize, DoubleArray(swapTenorSize, 0.));
	for(size_t i = 0; i < optionMatSize; ++i)
	{
		AQLString optionMat = optionMatVec[i];
		for(size_t j = 0; j < swapTenorSize; ++j)
		{
			AQLString swapTenor = swapTenorVec[j];

			AQLString prefix = "SWAPTION_";
			AQLString postfix = "_" + optionMat + "_" + swapTenor;
			DoubleVector T_fix_L_swaption = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(prefix + IR_CALIBRATION_DATA_T_FIX_L + postfix, ISNOTNULL).get()).get();
			DoubleVector T_pay_L_swaption = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(prefix + IR_CALIBRATION_DATA_T_PAY_L + postfix, ISNOTNULL).get()).get();
			DoubleVector tau_L_swaption = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(prefix + IR_CALIBRATION_DATA_TAU_L + postfix, ISNOTNULL).get()).get();
			DoubleVector T_pay_swaption = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(prefix + IR_CALIBRATION_DATA_T_PAY + postfix, ISNOTNULL).get()).get();
			DoubleVector tau_swaption = dynamic_cast<const AQLDataDoubles &>(calibInfo.getData(prefix + IR_CALIBRATION_DATA_TAU + postfix, ISNOTNULL).get()).get();

			LAMathSwaptionVolLMMDiscModel *swaption;
			if(swaptionApproxMethod == SWAPTION_REBONATO_DD_APPROXIMATION)
			{
				swaption = new LAMathSwaptionPremLMMDiscModel_ShiftedDD( *curve0_F,
																	   *curve0_D,
																	   const_cast<LAMathCorrelationLMM &>(*optCor),
																	   *volLMM, // TODO: const_cast is a dangerous operation...
																	   T_fix_L_swaption,
																	   T_pay_L_swaption,
																	   tau_L_swaption,
																	   T_pay_swaption,
																	   tau_swaption,
																	   T_fix_L_swaption.front(),
																	   T_pay_L_swaption.back(),
																	   volType,
																	   Q,
																	   constShift,
																	   swaptionVolForwardShift
																	 );
			}
			else if(swaptionApproxMethod == SWAPTION_WIENER_ITO_CHAOS_EXPANTION_1D)
			{
				swaption = new LAMathSwaptionVolLMMDiscModel_DD( *curve0_F,
														       *curve0_D,
															   const_cast<LAMathCorrelationLMM &>(*optCor),
														       *volLMM, // TODO: const_cast is a dangerous operation...
															   T_fix_L_swaption,
															   T_pay_L_swaption,
															   tau_L_swaption,
															   T_pay_swaption,
															   tau_swaption,
															   T_fix_L_swaption.front(),
															   T_pay_L_swaption.back(),
															   volType,
															   Q
															 );
			}
			else if(swaptionApproxMethod == SWAPTION_REBONATO_APPROXIMATION)
			{
				swaption = new LAMathSwaptionVolLMMDiscModel( *curve0_F,
														    *curve0_D,
															const_cast<LAMathCorrelationLMM &>(*optCor),
															*volLMM, // TODO: const_cast is a dangerous operation...
															T_fix_L_swaption,
															T_pay_L_swaption,
															tau_L_swaption,
															T_pay_swaption,
															tau_swaption,
															T_fix_L_swaption.front(),
															T_pay_L_swaption.back(),
															volType
														  );
			}
			else
			{
				AQLString msg = "SwaptionApproxMethod : " + swaptionApproxMethod + " is not supported.";
				throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
			}

			if(target1 == LAMathLeastSquareSwaptionVol::Volatility)
			{
				swaptionTargetValueMat[i][j] = swaptionVolMat[i][j];
			}
			else if(target1 == LAMathLeastSquareSwaptionVol::Premium)
			{
				if(volType == LAMathSwaptionVolLMM::Black)
				{
					swaptionTargetValueMat[i][j] = swaption->getSwaptionBlackPrem(swaptionVolMat[i][j], swaptionVolForwardShift);
				}
				else if(volType == LAMathSwaptionVolLMM::Normal)
				{
					swaptionTargetValueMat[i][j] = swaption->getSwaptionNormalPrem(swaptionVolMat[i][j]);
				}
			}

			swaptionLMMs.push_back(swaption);
		}
	}

	// set up CostFunction
	AQLString costFuncMode = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_OPT_COST_FUNC_MODE, ISNOTNULL).get()).get();
	LAMathLeastSquareSwaptionVol::mode mode1;
	LAMathLeastSquareCapFloor::mode mode2;

	if( costFuncMode == "DIFF" )
	{
		mode1 = LAMathLeastSquareSwaptionVol::Diff;
		mode2 = LAMathLeastSquareCapFloor::Diff;
	}
	else if( costFuncMode == "RATIO" )
	{
		mode1 = LAMathLeastSquareSwaptionVol::Ratio;
		mode2 = LAMathLeastSquareCapFloor::Ratio;
	}
	else if( costFuncMode == "SQRT_RATIO" )
	{
		mode1 = LAMathLeastSquareSwaptionVol::Sqrt_Ratio;
		mode2 = LAMathLeastSquareCapFloor::Sqrt_Ratio;
	}
	else
	{
		AQLString msg = "Wrong costFuncMode! : " + costFuncMode + "?";
		throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
	}

	double T_max = paramF.back();

	vector<LAQuantLibCostFunction*> costFunc(9);
	AQLString vegaWeightStr = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_VEGA_WEIGHT, ISNOTNULL).get()).get();
	vegaWeightStr.toUpper();
	bool vegaWeight = (vegaWeightStr == "TRUE" ? true : false);
	costFunc[SWAPTION_PARAM_V] = new LAMathLeastSquareSwaptionVolDiscModelV(swaptionTargetValueMat, swaptionLMMs, swaptionVolWeightMat, mode1, target1);
	costFunc[SWAPTION_PARAM_F] = new LAMathLeastSquareSwaptionVolDiscModelF(swaptionTargetValueMat, swaptionLMMs, T_max, swaptionVolWeightMat, mode1, target1);
	costFunc[SWAPTION_PARAM_G] = new LAMathLeastSquareSwaptionVolDiscModelG(swaptionTargetValueMat, swaptionLMMs, swaptionVolWeightMat, mode1, target1);
	costFunc[CAPFLOOR_PARAM_V] = new LAMathLeastSquareCapFloorVolDiscModelV(capBlackVols, capLMMs, vegaWeight, mode2, target2);
	costFunc[CAPFLOOR_PARAM_F] = new LAMathLeastSquareCapFloorVolDiscModelF(capBlackVols, capLMMs, T_max, vegaWeight, mode2, target2);
	costFunc[CAPFLOOR_PARAM_G] = new LAMathLeastSquareCapFloorVolDiscModelG(capBlackVols, capLMMs, vegaWeight, mode2, target2);
	costFunc[SWAPTIONCAPFLOOR_PARAM_V] = new LAMathLeastSquareSwaptionCapFloor(
		dynamic_cast<LAMathLeastSquareSwaptionVol&>(*costFunc[SWAPTION_PARAM_V]), dynamic_cast<LAMathLeastSquareCapFloor&>(*costFunc[CAPFLOOR_PARAM_V]));
	costFunc[SWAPTIONCAPFLOOR_PARAM_F] = new LAMathLeastSquareSwaptionCapFloor(
		dynamic_cast<LAMathLeastSquareSwaptionVol&>(*costFunc[SWAPTION_PARAM_F]), dynamic_cast<LAMathLeastSquareCapFloor&>(*costFunc[CAPFLOOR_PARAM_F]));
	costFunc[SWAPTIONCAPFLOOR_PARAM_G] = new LAMathLeastSquareSwaptionCapFloor(
		dynamic_cast<LAMathLeastSquareSwaptionVol&>(*costFunc[SWAPTION_PARAM_G]), dynamic_cast<LAMathLeastSquareCapFloor&>(*costFunc[CAPFLOOR_PARAM_G]));

	// set constraint for optimizers: unconstrained problem
	double boundaryMaxV = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_V, ISNOTNULL).get()).get();
	double boundaryMinV = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_V, ISNOTNULL).get()).get();
	double boundaryMaxF = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_F, ISNOTNULL).get()).get();
	double boundaryMinF = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_F, ISNOTNULL).get()).get();
	double boundaryMaxG = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MAX_G, ISNOTNULL).get()).get();
	double boundaryMinG = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_BOUNDARY_MIN_G, ISNOTNULL).get()).get();
	double smoothBoundaryG = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_SMOOTH_BOUNDARY_G, ISNOTNULL).get()).get();
	vector<LAQuantLibConstraint*> constraints(3);
	constraints[PARAM_V] = new LAMathBoundaryConstraintLMMVolV(boundaryMinV, boundaryMaxV);
	constraints[PARAM_F] = new LAMathBoundaryConstraintLMMVolF(boundaryMinF, boundaryMaxF);
	constraints[PARAM_G] = new LAMathBoundaryConstraintLMMVolG(boundaryMinG, boundaryMaxG,  paramG.size(), smoothBoundaryG);

	// set initial guess for optimizer
	LAQuantLibArray x_V(paramV.size());
	for( size_t i = 0; i < paramV.size(); ++i )
	{
		x_V[i] = paramV[i];
	}

	// paramF.size() - 1 : Because we don't want to Optimize T_max;
	LAQuantLibArray x_F(paramF.size() - 1);
	for( size_t i = 0; i < paramF.size() - 1; ++i )
	{
		x_F[i] = paramF[i];
	}

	LAQuantLibArray x_G(paramG.size());
	for( size_t i = 0; i < paramG.size(); ++i )
	{
		x_G[i] = paramG[i];
	}

//----------------------------------------------------------------
//Here we correct initial parameters for levenberg calibration in Qlib,
//since the calibration method shows poor performance when initial parameters are too close to constraints.
//Note that this adjust is an emergency treatment!!
	AQLString optMethodType = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_OPT_METHOD_TYPE, ISNOTNULL).get()).get();
	double levenbergAdjustTor = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_LEVENBERG_ADJUST_Tor, ISNOTNULL).get()).get();
	double levenbergAdjustV = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_LEVENBERG_ADJUST_V, ISNOTNULL).get()).get();
	double levenbergAdjustF = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_LEVENBERG_ADJUST_F, ISNOTNULL).get()).get();
	if( optMethodType == CALIB_LEVENBERG_MARQUARDT_METHOD )
	{
	// V ajust
		ajust_V(x_V, boundaryMinV, boundaryMaxV, levenbergAdjustV, levenbergAdjustTor);

	// F ajust
		ajust_F(x_F, boundaryMinF, boundaryMaxF, levenbergAdjustF, levenbergAdjustTor);

	// G ajust
		ajust_G(x_G, boundaryMinG, boundaryMaxG);
	}
//----------------------------------------------------------------


	// set vol method F & G = 1. Because we want to fit vol method V first.
/* Fukui adjust!! */
//	DoubleVector paramF0(paramF.size() - 1);
//	DoubleVector paramF0(paramF.size());
//	paramF0[0] = 0.0;
//	paramF0[1] = 0.0;
//	paramF0[2] = 0.0;
//	paramF0[3] = 0.0;
//	paramF0[4] = 0.0;
//	paramF0[5] = 0.0;
//	paramF0[6] = 0.0;
//	paramF0[7] = 1.0;
//	paramF0[8] = paramF[8];
//	paramF.resize(8);

//	DoubleVector paramG0(paramG.size());
//	for( size_t i = 0; i < paramG.size(); ++i )
//	{
//		paramG0[i] = 1.0;
//	}
//	volLMM->setParamF(paramF0);
//	volLMM->setParamG(paramG0);
/* Fukui adjust!! */


	// Set up EndCriteria
    int maxIteration = dynamic_cast<const AQLDataInt &>(calibInfo.getData(CALIBRATION_DATA_MAX_ITERATION, ISNOTNULL).get()).get();
    int maxStationaryStateIteration = dynamic_cast<const AQLDataInt &>(calibInfo.getData(CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION, ISNOTNULL).get()).get();
    double rootEpsilon = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_ROOT_EPSILON, ISNOTNULL).get()).get();
    double functionEpsilon = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_FUNCTION_EPSILON, ISNOTNULL).get()).get();
    double gradientNormEpsilon = dynamic_cast<const AQLDataDouble &>(calibInfo.getData(CALIBRATION_DATA_GRADIENT_NORM_EPSILON, ISNOTNULL).get()).get();

	LAQuantLibEndCriteria *endCriteria = new LAQuantLibEndCriteria(maxIteration, maxStationaryStateIteration,
				rootEpsilon, functionEpsilon, gradientNormEpsilon);

	// Set Problem
	vector<LAQuantLibProblem*> problem(9);
	problem[SWAPTION_PARAM_V] = new LAQuantLibProblem(*costFunc[SWAPTION_PARAM_V], *constraints[PARAM_V], x_V);
	problem[SWAPTION_PARAM_F] = new LAQuantLibProblem(*costFunc[SWAPTION_PARAM_F], *constraints[PARAM_F], x_F);
	problem[SWAPTION_PARAM_G] = new LAQuantLibProblem(*costFunc[SWAPTION_PARAM_G], *constraints[PARAM_G], x_G);
	problem[CAPFLOOR_PARAM_V] = new LAQuantLibProblem(*costFunc[CAPFLOOR_PARAM_V], *constraints[PARAM_V], x_V);
	problem[CAPFLOOR_PARAM_F] = new LAQuantLibProblem(*costFunc[CAPFLOOR_PARAM_F], *constraints[PARAM_F], x_F);
	problem[CAPFLOOR_PARAM_G] = new LAQuantLibProblem(*costFunc[CAPFLOOR_PARAM_G], *constraints[PARAM_G], x_G);
	problem[SWAPTIONCAPFLOOR_PARAM_V] = new LAQuantLibProblem(*costFunc[SWAPTIONCAPFLOOR_PARAM_V], *constraints[PARAM_V], x_V);
	problem[SWAPTIONCAPFLOOR_PARAM_F] = new LAQuantLibProblem(*costFunc[SWAPTIONCAPFLOOR_PARAM_F], *constraints[PARAM_F], x_F);
	problem[SWAPTIONCAPFLOOR_PARAM_G] = new LAQuantLibProblem(*costFunc[SWAPTIONCAPFLOOR_PARAM_G], *constraints[PARAM_G], x_G);


	// Set Optimization Method
//	AQLString optMethodType = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_OPT_METHOD_TYPE, ISNOTNULL).get()).get();
	optMethodType.toUpper();
	vector<LAQuantLibOptimizationMethod*> optMethod(9);
	if( optMethodType == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD )
	{
		optMethod[SWAPTION_PARAM_V] = new LAQuantLibConjugateGradient();
		optMethod[SWAPTION_PARAM_F] = new LAQuantLibConjugateGradient();
		optMethod[SWAPTION_PARAM_G] = new LAQuantLibConjugateGradient();
		optMethod[CAPFLOOR_PARAM_V] = new LAQuantLibConjugateGradient();
		optMethod[CAPFLOOR_PARAM_F] = new LAQuantLibConjugateGradient();
		optMethod[CAPFLOOR_PARAM_G] = new LAQuantLibConjugateGradient();
		optMethod[SWAPTIONCAPFLOOR_PARAM_V] = new LAQuantLibConjugateGradient();
		optMethod[SWAPTIONCAPFLOOR_PARAM_F] = new LAQuantLibConjugateGradient();
		optMethod[SWAPTIONCAPFLOOR_PARAM_G] = new LAQuantLibConjugateGradient();
	}
	else if( optMethodType == CALIB_STEEPEST_DESCENT_METHOD )
	{
		optMethod[SWAPTION_PARAM_V] = new LAQuantLibSteepestDescent();
		optMethod[SWAPTION_PARAM_F] = new LAQuantLibSteepestDescent();
		optMethod[SWAPTION_PARAM_G] = new LAQuantLibSteepestDescent();
		optMethod[CAPFLOOR_PARAM_V] = new LAQuantLibSteepestDescent();
		optMethod[CAPFLOOR_PARAM_F] = new LAQuantLibSteepestDescent();
		optMethod[CAPFLOOR_PARAM_G] = new LAQuantLibSteepestDescent();
		optMethod[SWAPTIONCAPFLOOR_PARAM_V] = new LAQuantLibSteepestDescent();
		optMethod[SWAPTIONCAPFLOOR_PARAM_F] = new LAQuantLibSteepestDescent();
		optMethod[SWAPTIONCAPFLOOR_PARAM_G] = new LAQuantLibSteepestDescent();
	}
	else if( optMethodType == CALIB_SIMPLEX_METHOD )
	{
		optMethod[SWAPTION_PARAM_V] = new LAQuantLibSimplex(0.0001);
		optMethod[SWAPTION_PARAM_F] = new LAQuantLibSimplex(0.0001);
		optMethod[SWAPTION_PARAM_G] = new LAQuantLibSimplex(0.0001);
		optMethod[CAPFLOOR_PARAM_V] = new LAQuantLibSimplex(0.0001);
		optMethod[CAPFLOOR_PARAM_F] = new LAQuantLibSimplex(0.0001);
		optMethod[CAPFLOOR_PARAM_G] = new LAQuantLibSimplex(0.0001);
		optMethod[SWAPTIONCAPFLOOR_PARAM_V] = new LAQuantLibSimplex(0.0001);
		optMethod[SWAPTIONCAPFLOOR_PARAM_F] = new LAQuantLibSimplex(0.0001);
		optMethod[SWAPTIONCAPFLOOR_PARAM_G] = new LAQuantLibSimplex(0.0001);
	}
	else if( optMethodType == CALIB_LEVENBERG_MARQUARDT_METHOD )
	{
		double levenbergMarquardtEpsfcn = 1.0e-8;     // parameters specific for Levenberg-Marquardt
		double levenbergMarquardtXtol   = 1.0e-8;     //
		double levenbergMarquardtGtol   = 1.0e-8;     //

		optMethod[SWAPTION_PARAM_V] = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,levenbergMarquardtXtol, levenbergMarquardtGtol);
		optMethod[SWAPTION_PARAM_F] = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,levenbergMarquardtXtol, levenbergMarquardtGtol);
		optMethod[SWAPTION_PARAM_G] = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,levenbergMarquardtXtol, levenbergMarquardtGtol);
		optMethod[CAPFLOOR_PARAM_V] = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,levenbergMarquardtXtol, levenbergMarquardtGtol);
		optMethod[CAPFLOOR_PARAM_F] = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,levenbergMarquardtXtol, levenbergMarquardtGtol);
		optMethod[CAPFLOOR_PARAM_G] = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,levenbergMarquardtXtol, levenbergMarquardtGtol);
		optMethod[SWAPTIONCAPFLOOR_PARAM_V] = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,levenbergMarquardtXtol, levenbergMarquardtGtol);
		optMethod[SWAPTIONCAPFLOOR_PARAM_F] = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,levenbergMarquardtXtol, levenbergMarquardtGtol);
		optMethod[SWAPTIONCAPFLOOR_PARAM_G] = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,levenbergMarquardtXtol, levenbergMarquardtGtol);
	}

    int vfgLoopNum = dynamic_cast<const AQLDataInt &>(calibInfo.getData(CALIBRATION_DATA_VFG_LOOP_NUM, ISNOTNULL).get()).get();

	double convLimit = 1E-7;
    double convValue = 0.0;
    double formerConvValue = 0.0;

    AQLStringVector volWeight = dynamic_cast<const AQLDataStrings &>(calibInfo.getData(CALIBRATION_DATA_VOL_WEIGHT, ISNOTNULL).get()).get();
	double weight_CapFloor = volWeight[0].getDoubleValue();
	double weight_Swaption = volWeight[1].getDoubleValue();

	AQLString optVFlagStr = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_OPT_V_FLAG, ISNOTNULL).get()).get();
	AQLString optFFlagStr = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_OPT_F_FLAG, ISNOTNULL).get()).get();
	AQLString optGFlagStr = dynamic_cast<const AQLDataString &>(calibInfo.getData(CALIBRATION_DATA_OPT_G_FLAG, ISNOTNULL).get()).get();
	optVFlagStr.toUpper();
	optFFlagStr.toUpper();
	optGFlagStr.toUpper();
	bool optVFlag = (optVFlagStr == "TRUE");
	bool optFFlag = (optFFlagStr == "TRUE");
	bool optGFlag = (optGFlagStr == "TRUE");

	bool isParamVOpt = false;
	bool isParamFOpt = false;
	bool isParamGOpt = false;

	double optResult = 0.0;
    for(int s = 0; s < vfgLoopNum; ++s) 
	{
		//vol_optimized
		if( !LAModelUtilities::eq(weight_CapFloor + weight_Swaption, 1.0, 0.0001) ) throw AQLCoreInvalidData("weight_CapFloor + weight_Swaption must be 1.0",__FILE__,__LINE__);

		int target_V, target_F, target_G;
		string mode;
		if( weight_Swaption == 0.0 && LAModelUtilities::eq(weight_CapFloor, 1.0) )
		{
			target_V = CAPFLOOR_PARAM_V;
			target_F = CAPFLOOR_PARAM_F;
			target_G = CAPFLOOR_PARAM_G;
		}
		else if( LAModelUtilities::eq(weight_Swaption, 1.0) && weight_CapFloor == 0.0 )
		{
			target_V = SWAPTION_PARAM_V;
			target_F = SWAPTION_PARAM_F;
			target_G = SWAPTION_PARAM_G;
		}
		else
		{
			target_V = SWAPTIONCAPFLOOR_PARAM_V;
			target_F = SWAPTIONCAPFLOOR_PARAM_F;
			target_G = SWAPTIONCAPFLOOR_PARAM_G;

			vector<double> weight_;
			weight_.push_back(weight_CapFloor);
			weight_.push_back(weight_Swaption);

			dynamic_cast<LAMathLeastSquareSwaptionCapFloor*>(costFunc[SWAPTIONCAPFLOOR_PARAM_V])->setWeight(weight_);
			dynamic_cast<LAMathLeastSquareSwaptionCapFloor*>(costFunc[SWAPTIONCAPFLOOR_PARAM_F])->setWeight(weight_);
			dynamic_cast<LAMathLeastSquareSwaptionCapFloor*>(costFunc[SWAPTIONCAPFLOOR_PARAM_G])->setWeight(weight_);
		}

		if( optVFlag )
		{
			LAQuantLibEndCriteria::Type endCriteriaResult_V = optMethod[target_V]->minimize(*problem[target_V], *endCriteria);

			LAQuantLibArray xMinCalculated_V = problem[target_V]->currentValue();
			optResult = problem[target_V]->value(xMinCalculated_V);

			paramV.resize(4);
			for(size_t i = 0; i < 4; ++i)
			{
				paramV[i] = xMinCalculated_V[i];
			}
		}
		isParamVOpt = true;

		if( optFFlag )
		{
			LAQuantLibEndCriteria::Type endCriteriaResult_F = optMethod[target_F]->minimize(*problem[target_F], *endCriteria);

			LAQuantLibArray xMinCalculated_F = problem[target_F]->currentValue();
			optResult = problem[target_F]->value(xMinCalculated_F);

			//paramF.resize(8);
			for(size_t i = 0; i < 8; ++i)
			{
				paramF[i] = problem[target_F]->currentValue()[i];
			}
		}
//		else
//		{
//			//paramF.resize(8);
//			for(size_t i = 0; i < 7; ++i)
//			{
//				paramF[i] = 0.0;
//			}
//			paramF[7] = 1.0;
//		}
		isParamFOpt = true;


		if( optGFlag )
		{
			if (optionMatSize * swapTenorSize < paramG.size() )
			{
				AQLString msg = "SwaptionVolMat size must be greater than paramG size";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			LAQuantLibEndCriteria::Type endCriteriaResult_G = optMethod[target_G]->minimize(*problem[target_G], *endCriteria);


			LAQuantLibArray mixMinCalculated_G = problem[target_G]->currentValue();
			optResult = problem[target_G]->value(mixMinCalculated_G);

			size_t n = mixMinCalculated_G.size();
			paramG.resize(n);
			LAQuantLibArray curValue = problem[target_G]->currentValue();
			for(size_t i = 0; i < n; ++i)
			{
				paramG[i] = curValue[i];
			}
		}
//		else
//		{
//			paramG.resize(tenor_vollmm.size());
//			for(size_t i = 0; i < tenor_vollmm.size(); ++i)
//			{
//				paramG[i] = 1.0;
//			}
//		}
		isParamGOpt = true;

		convValue = optResult / sqrt( static_cast<double>(optionMatSize * swapTenorSize));

		if( AQLMath::abs(convValue - formerConvValue) < convLimit)
			break;
		formerConvValue = convValue;
	}

	setResult(models, paramV, vector<double>(paramF.begin(), paramF.begin() + 8), paramG, tenorG);
	
	//set V, F and G in volLMM for swaptionLMMVol and capLMMVol
	bool isResultoutLMMSwaption = true;			// added this flag to prevent swaption vol calculation error occuring in the case of quite small target swaption vol. ex) NZD swaptionvol (1E-12)
	isResultoutLMMSwaption = dynamic_cast<const AQLDataBool &>(calibInfo.getData(CALIBRATION_DATA_IS_RESULTOUT_LMM_SWAPTION, NOCHECK).get()).get();
	if (isResultoutLMMSwaption)
	{
		volLMM->setParamV(paramV);
		DoubleArray tmpParamF = paramF;
		tmpParamF.push_back(T_max);
		volLMM->setParamF(tmpParamF);
		volLMM->setParamG(paramG);

		DoubleMatrix swaptionLMMVols(optionMatSize, DoubleVector(swapTenorSize));
		for (size_t i = 0; i < optionMatSize; ++i)
		{
			for (size_t j = 0; j < swapTenorSize; ++j)
			{
				swaptionLMMVols[i][j] = swaptionLMMs[i * swapTenorSize + j]->getSwaptionVol();
			}
		}

		DoubleVector capLMMVols(capGridSize);
		for (size_t i = 0; i < capGridSize; ++i)
		{
			capLMMVols[i] = capLMMs[i]->getStdDev();
		}
		//
		//#ifdef _DEBUG	
		//	ofstream OUT_S0(std::string("W:\\S0.csv")); OUT_S0 << std::setprecision(20);
		//	ofstream OUT_Annuity(std::string("W:\\Annuity.csv")); OUT_Annuity << std::setprecision(20);
		//	ofstream OUT_T_OptMat(std::string("W:\\T_OptMat.csv")); OUT_T_OptMat << std::setprecision(20);
		//	
		//	for(size_t i = 0; i < optionMatSize; ++i)
		//	{
		//		for(size_t j = 0; j < swapTenorSize; ++j)
		//		{
		//			OUT_S0 << swaptionLMMs[i * swapTenorSize + j]->getS0() << ",";
		//			OUT_Annuity << swaptionLMMs[i * swapTenorSize + j]->getAnnuity() << ",";
		//			OUT_T_OptMat << swaptionLMMs[i * swapTenorSize + j]->getT_OptMat() << ",";
		//		}
		//		OUT_S0 << endl; OUT_Annuity << endl; OUT_T_OptMat << endl; 
		//	}
		//
		//	OUT_S0.close(); OUT_Annuity.close(); OUT_T_OptMat.close();
		//#endif
		DoubleMatrix swaptionS0s(optionMatSize, DoubleVector(swapTenorSize));
		DoubleMatrix swaptionAnnuities(optionMatSize, DoubleVector(swapTenorSize));
		DoubleMatrix swaptionOptMats(optionMatSize, DoubleVector(swapTenorSize));
		for (size_t i = 0; i < optionMatSize; ++i)
		{
			for (size_t j = 0; j < swapTenorSize; ++j)
			{
				swaptionS0s[i][j] = swaptionLMMs[i * swapTenorSize + j]->getS0();
				swaptionAnnuities[i][j] = swaptionLMMs[i * swapTenorSize + j]->getAnnuity();
				swaptionOptMats[i][j] = swaptionLMMs[i * swapTenorSize + j]->getT_OptMat();
			}
		}

		models.remove(AQLString("Result") + CALIBRATION_DATA_MODELSWAPTIONVOLS);
		models.add(AQLString("Result") + CALIBRATION_DATA_MODELSWAPTIONVOLS, new AQLDataDoubleMatrix(swaptionLMMVols));
		models.remove(AQLString("Result") + CALIBRATION_DATA_MODELSWAPTION_S0S);
		models.add(AQLString("Result") + CALIBRATION_DATA_MODELSWAPTION_S0S, new AQLDataDoubleMatrix(swaptionS0s));
		models.remove(AQLString("Result") + CALIBRATION_DATA_MODELSWAPTION_ANNUITIES);
		models.add(AQLString("Result") + CALIBRATION_DATA_MODELSWAPTION_ANNUITIES, new AQLDataDoubleMatrix(swaptionAnnuities));
		models.remove(AQLString("Result") + CALIBRATION_DATA_MODELSWAPTION_OPTMATS);
		models.add(AQLString("Result") + CALIBRATION_DATA_MODELSWAPTION_OPTMATS, new AQLDataDoubleMatrix(swaptionOptMats));
		models.remove(AQLString("Result") + CALIBRATION_DATA_MODELCAPFLOORVOLS);
		models.add(AQLString("Result") + CALIBRATION_DATA_MODELCAPFLOORVOLS, new AQLDataDoubles(capLMMVols));
	}

	delete volLMM;
	delete optCor;
	for(unsigned int i = 0; i < capGridSize; i++)
	{
		delete capLMMs[i];
		delete capBlackVols[i];
	}
	for(unsigned int i = 0; i < swaptionLMMs.size(); i++)
	{
		delete swaptionLMMs[i];
	}

	delete endCriteria;
	delete costFunc[SWAPTION_PARAM_V];
	delete costFunc[SWAPTION_PARAM_F];
	delete costFunc[SWAPTION_PARAM_G];
	delete costFunc[CAPFLOOR_PARAM_V];
	delete costFunc[CAPFLOOR_PARAM_F];
	delete costFunc[CAPFLOOR_PARAM_G];
	delete costFunc[SWAPTIONCAPFLOOR_PARAM_V];
	delete costFunc[SWAPTIONCAPFLOOR_PARAM_F];
	delete costFunc[SWAPTIONCAPFLOOR_PARAM_G];
	delete constraints[PARAM_V];
	delete constraints[PARAM_F];
	delete constraints[PARAM_G];
	delete problem[SWAPTION_PARAM_V];
	delete problem[SWAPTION_PARAM_F];
	delete problem[SWAPTION_PARAM_G];
	delete problem[CAPFLOOR_PARAM_V];
	delete problem[CAPFLOOR_PARAM_F];
	delete problem[CAPFLOOR_PARAM_G];
	delete problem[SWAPTIONCAPFLOOR_PARAM_V];
	delete problem[SWAPTIONCAPFLOOR_PARAM_F];
	delete problem[SWAPTIONCAPFLOOR_PARAM_G];
	delete optMethod[SWAPTION_PARAM_V];
	delete optMethod[SWAPTION_PARAM_F];
	delete optMethod[SWAPTION_PARAM_G];
	delete optMethod[CAPFLOOR_PARAM_V];
	delete optMethod[CAPFLOOR_PARAM_F];
	delete optMethod[CAPFLOOR_PARAM_G];
	delete optMethod[SWAPTIONCAPFLOOR_PARAM_V];
	delete optMethod[SWAPTIONCAPFLOOR_PARAM_F];
	delete optMethod[SWAPTIONCAPFLOOR_PARAM_G];

	// delete tmp curve, path object curve
	delete tmpCurveF;
	delete curve0_F;
	delete tmpCurveD;
	delete curve0_D;
}

void LAPriceLMMCalibration::setResult(AQLObject& output, 
	                               const DoubleVector& v, 
								   const DoubleVector& f, 
								   const DoubleVector& g, 
								   const DoubleVector& tenor_g) const
{
	output.remove(AQLString("Result") + CALIBRATION_DATA_PARAM_V);
	output.add(AQLString("Result") + CALIBRATION_DATA_PARAM_V, new AQLDataDoubles(v));

	output.remove(AQLString("Result") + CALIBRATION_DATA_PARAM_F);
	output.add(AQLString("Result") + CALIBRATION_DATA_PARAM_F, new AQLDataDoubles(f));

	output.remove(AQLString("Result") + CALIBRATION_DATA_PARAM_G);
	output.add(AQLString("Result") + CALIBRATION_DATA_PARAM_G, new AQLDataDoubles(g));
	output.remove(CALIBRATION_DATA_TENOR_G);
	output.add(CALIBRATION_DATA_TENOR_G, new AQLDataDoubles(tenor_g));
}

void ajust_V( LAQuantLibArray& params, double low, double high, double ajustment, double eps )
{
	if( params.size() != 4) throw AQLCoreInvalidData("size out of range",__FILE__,__LINE__);

//----------------
// lower bound
	if( LAModelUtilities::eq(params[0] + params[3], low, eps) )
	{
		params[0] += ajustment;
		params[3] += ajustment;
	}
	if( LAModelUtilities::eq(params[3], low, eps) ) params[3] += ajustment;
	if( LAModelUtilities::eq(params[2], low, eps) ) params[2] += ajustment;

//----------------
// upper bound

	for(size_t i = 0; i < 4; ++i)
	{
		if( LAModelUtilities::eq(params[i], high, eps) ) params[i] -= ajustment;
	}
}

void ajust_F( LAQuantLibArray& params, double low, double high, double ajustment, double eps )
{
	if( params.size() != 8) throw AQLCoreInvalidData("size out of range : Object::get_ID",__FILE__,__LINE__);

//----------------
// lower bound
	if( LAModelUtilities::eq(params[0], low, eps) ) params[0] += ajustment;
	if( LAModelUtilities::eq(params[1], low, eps) ) params[1] += ajustment;
	if( LAModelUtilities::eq(params[3], low, eps) ) params[3] += ajustment;
	if( LAModelUtilities::eq(params[5], low, eps) ) params[5] += ajustment;
	if( LAModelUtilities::eq(params[7], low, eps) ) params[7] += ajustment;

//----------------
// upper bound
	if( LAModelUtilities::eq(params[0], high, eps) ) params[0] -= ajustment;
	if( LAModelUtilities::eq(params[1], high, eps) ) params[1] -= ajustment;
	if( LAModelUtilities::eq(params[3], high, eps) ) params[3] -= ajustment;
	if( LAModelUtilities::eq(params[5], high, eps) ) params[5] -= ajustment;
	if( LAModelUtilities::eq(params[7], high, eps) ) params[7] -= ajustment;

//----------------
// else
	if( LAModelUtilities::eq(params[0], 0., eps) ) params[0] += ajustment;
	if( LAModelUtilities::eq(params[2], 0., eps) ) params[2] += ajustment;
	if( LAModelUtilities::eq(params[4], 0., eps) ) params[4] += ajustment;
	if( LAModelUtilities::eq(params[6], 0., eps) ) params[6] += ajustment;
	if( LAModelUtilities::eq(params[7], 0., eps) ) params[7] += ajustment;

	if( LAModelUtilities::eq(params[2], 3.141593, eps) ) params[2] -= ajustment;
	if( LAModelUtilities::eq(params[4], 3.141593, eps) ) params[4] -= ajustment;
	if( LAModelUtilities::eq(params[6], 3.141593, eps) ) params[6] -= ajustment;
}

void ajust_G( LAQuantLibArray& params, double low, double high, double ajustment, double eps )
{
	params; low; high; ajustment; eps;
 //	 if( params.size() != G_size ) throw AQLCoreInvalidData("size is not correct! : Object::get_ID",__FILE__,__LINE__);

 //	 if( params[0] < low_ ) return false;
 //   if( params[0] > high_ ) return false;
 //   if( params[G_size - 1] < low_ ) return false;
 //   if( params[G_size - 1] > high_ ) return false;

 //   for(size_t i = 1; i < G_size - 2; ++i)
 //   {
 //       if( params[i] < low_ ) return false;
 //       if( params[i] > high_ ) return false;
 //       
 //       double jump = fabs( params[i - 1] - 2.0 * params[i] + params[i + 1] );
 //       if( jump * jump > smooth_bound  )
 //       {
 //           return false;
 //       }
 //   }
}
