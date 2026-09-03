#pragma once

//3F add
#ifndef PRICING_DATA_SKEWVOLCALIBINFO
#define PRICING_DATA_SKEWVOLCALIBINFO		"SkewVolCalibInfo"		//  data name of SkewVolCalibInfo
#endif
#ifndef PRICING_DATA_SIGMABETACALIBINFO
#define PRICING_DATA_SIGMABETACALIBINFO		"SigmaBetaCalibInfo"		//  data name of SigmaBetaCalibInfo
#endif
#ifndef PRICING_DATA_INITIALVALUE
#define PRICING_DATA_INITIALVALUE			"InitialValue"		//  data name of InitialValue
#endif
#ifndef PRICING_DATA_DOMESTICCURVETYPE
#define PRICING_DATA_DOMESTICCURVETYPE	    "DomesticCurveType"		//  data name of DomesticCurveType
#endif
#ifndef PRICING_DATA_FOREIGNCURVETYPE
#define PRICING_DATA_FOREIGNCURVETYPE		"ForeignCurveType"		//  data name of ForeignCurveType
#endif
#ifndef CALIBRATION_DATA_OPT_METHOD_TYPE
#define CALIBRATION_DATA_OPT_METHOD_TYPE					"OptMethodType"					//  data name of OptMethodType
#endif
#ifndef CALIBRATION_DATA_BOUNDARY_MAX_VOL
#define CALIBRATION_DATA_BOUNDARY_MAX_VOL					"BoundaryMax_Vol"					//  data name of BoundaryMax_Vol
#endif
#ifndef CALIBRATION_DATA_BOUNDARY_MAX_SKEW
#define CALIBRATION_DATA_BOUNDARY_MAX_SKEW				"BoundaryMax_Skew"					//  data name of BoundaryMax_Skew
#endif
#ifndef CALIBRATION_DATA_BOUNDARY_MAX_SIGMA
#define CALIBRATION_DATA_BOUNDARY_MAX_SIGMA				"BoundaryMax_Sigma"					//  data name of BoundaryMax_Sigma
#endif
#ifndef CALIBRATION_DATA_BOUNDARY_MAX_BETA
#define CALIBRATION_DATA_BOUNDARY_MAX_BETA				"BoundaryMax_Beta"					//  data name of BoundaryMax_Beta
#endif
#ifndef CALIBRATION_DATA_BOUNDARY_MIN_VOL
#define CALIBRATION_DATA_BOUNDARY_MIN_VOL					"BoundaryMin_Vol"					//  data name of BoundaryMin_Vol
#endif
#ifndef CALIBRATION_DATA_BOUNDARY_MIN_SKEW
#define CALIBRATION_DATA_BOUNDARY_MIN_SKEW				"BoundaryMin_Skew"					//  data name of BoundaryMin_Skew
#endif
#ifndef CALIBRATION_DATA_BOUNDARY_MIN_SIGMA
#define CALIBRATION_DATA_BOUNDARY_MIN_SIGMA				"BoundaryMin_Sigma"					//  data name of BoundaryMin_Sigma
#endif
#ifndef CALIBRATION_DATA_BOUNDARY_MIN_BETA
#define CALIBRATION_DATA_BOUNDARY_MIN_BETA				"BoundaryMin_Beta"					//  data name of BoundaryMin_Beta
#endif
#ifndef CALIBRATION_DATA_SMALL_STEPS
#define CALIBRATION_DATA_SMALL_STEPS				        "SmallSteps"					//  data name of BoundaryMin_Beta
#endif
#ifndef CALIBRATION_DATA_MAX_ITERATION
#define CALIBRATION_DATA_MAX_ITERATION					"MaxIteration"					//  data name of MaxIteration
#endif
#ifndef CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION
#define CALIBRATION_DATA_MAX_STATIONARY_STATE_ITERATION	"MaxStationaryStateIteration"	//  data name of MaxStationaryStateIteration
#endif
#ifndef CALIBRATION_DATA_ROOT_EPSILON
#define CALIBRATION_DATA_ROOT_EPSILON						"RootEpsilon"					//  data name of RootEpsilon
#endif
#ifndef CALIBRATION_DATA_FUNCTION_EPSILON
#define CALIBRATION_DATA_FUNCTION_EPSILON					"FunctionEpsilon"				//  data name of FunctionEpsilon
#endif
#ifndef CALIBRATION_DATA_GRADIENT_NORM_EPSILON
#define CALIBRATION_DATA_GRADIENT_NORM_EPSILON			"GradientNormEpsilon"			//  data name of GradientNormEpsilon
#endif
#ifndef CALIBRATION_DATA_OPT_SIGMABETA_FLAG
#define CALIBRATION_DATA_OPT_SIGMABETA_FLAG						"OptSigmaBetaFlag"						//  data name of OptSigmaBetaFlag
#endif
#ifndef	CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD
#define CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD		"NON_LINEAR_CONJUGATE_GRADIENT_METHOD"
#endif
#ifndef	CALIB_STEEPEST_DESCENT_METHOD
#define CALIB_STEEPEST_DESCENT_METHOD					"STEEPEST_DESCENT_METHOD"
#endif
#ifndef	CALIB_SIMPLEX_METHOD
#define CALIB_SIMPLEX_METHOD							"SIMPLEX_METHOD"
#endif
#ifndef	CALIB_LEVENBERG_MARQUARDT_METHOD
#define CALIB_LEVENBERG_MARQUARDT_METHOD				"LEVENBERG-MARQUARDT_METHOD"
#endif


#ifdef __GNUG__
#pragma interface
#endif


#include "LACoreProcedure.h"
#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAOptimumBrent.h"
#include "LAModelDynamicsBase.h"
#include "LADataBasics.h"
#include "LAObject.h"
#include "LAMathDateCalculations.h"
#include "LAMathYieldCurvePro.h"
#include "LAMathAntonovFXOption.h"
#include "LAPriceFXDisplacedDiffusionCalibration.h"


// Function ID for LAPriceFXDisplacedDiffusionCalibration3F
#define FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION3F		10143
// Function name for LAPriceFXDisplacedDiffusionCalibration3F
#define FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION3F_STR	"fn_ir_fxdisplaceddiffusioncalibration3f"


class LAObject;
class LADataProcedure;
class LAPriceDataManager;
class LADataMultiReference;
class LARatesPathElementCurve;
class LAMathYieldCurve;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Generate Cashlets and trigger/call schedule

*/

class LAPriceFXDisplacedDiffusionCalibration3F : public LAPriceFXDisplacedDiffusionCalibration
{
public:
//  LIFECYCLE
    // constructor
	LAPriceFXDisplacedDiffusionCalibration3F();
    // destructor	
	virtual ~LAPriceFXDisplacedDiffusionCalibration3F();
    // Check this class ID is the same or not	
	virtual bool                isTypeOf(function_t id) const;
    // Copy this class
	virtual LACoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
    // Return class type
	virtual function_t			getType() const;
    // Generate cashlets and trigger/call schedule
	virtual void	            calibrateModel(const LADate& basedate, 
										LAObject& object, 
										const LADataProcedure& att) const;
	// register dataValues that this class uses
	virtual void				registerData(LAPriceDataManager& dm) const;
    // calibrate market skew vol
	virtual void	            calibMarketSkewVol(const LADate& basedate, 
										LAObject& object, 
										const LADataProcedure& att) const;
    // calibrate hw dd parameter
	virtual void	            calibHybridHWDDProcess(const LADate& basedate, 
										LAObject& object, 
										const LADataProcedure& att) const;



protected:
    // copy constructor
	LAPriceFXDisplacedDiffusionCalibration3F(const LAPriceFXDisplacedDiffusionCalibration3F& p);


};
