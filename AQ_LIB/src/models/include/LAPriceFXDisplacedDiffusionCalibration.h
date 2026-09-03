#pragma once

#ifndef PRICING_DATA_CALIBRATORENGINE
#define PRICING_DATA_CALIBRATORENGINE		"CalibratorEngine"				//  data name of CalibratorEngine
#endif
#ifndef CALIBRATION_DATA_CALIBRATIONDATA
#define CALIBRATION_DATA_CALIBRATIONDATA				"CalibrationData"				//  data name of CalibrationData
#endif
#ifndef PRICING_DATA_SDEINFO
#define PRICING_DATA_SDEINFO				"SDEInfo"						//  data name of SDEInfo
#endif
#ifndef PRICING_DATA_BETACAP
#define PRICING_DATA_BETACAP				"BetaCap"						//  data name of BetaCap
#endif
#ifndef PRICING_DATA_BETAFLOOR
#define PRICING_DATA_BETAFLOOR				"BetaFloor"						//  data name of BetaFloor
#endif
#ifndef PRICING_DATA_DOMESTICCURVE
#define PRICING_DATA_DOMESTICCURVE			"DomesticCurve"				//  data name of DomesticCurve
#endif
#ifndef PRICING_DATA_FOREIGNCURVE
#define PRICING_DATA_FOREIGNCURVE			"ForeignCurve"				//  data name of ForeignCurve
#endif
#ifndef PRICING_DATA_ISUSEDF2FORDOMESTIC
#define PRICING_DATA_ISUSEDF2FORDOMESTIC	"IsUseDF2ForDomestic"		//  data name of IsUseDF2ForDomestic
#endif
#ifndef PRICING_DATA_ISUSEDF2FORFOREIGN
#define PRICING_DATA_ISUSEDF2FORFOREIGN		"IsUseDF2ForForeign"		//  data name of IsUseDF2ForForeign
#endif
#ifndef PRICING_DATA_SPOTFX
#define PRICING_DATA_SPOTFX					"SpotFX"		//  data name of SpotFX
#endif
#ifndef PRICING_DATA_DOMESTICIRMODEL
#define PRICING_DATA_DOMESTICIRMODEL		"DomesticIRModel"		//  data name of DomesticIRModel
#endif
#ifndef PRICING_DATA_FOREIGNIRMODEL
#define PRICING_DATA_FOREIGNIRMODEL			"ForeignIRModel"		//  data name of ForeignIRModel
#endif
#ifndef PRICING_DATA_MARKETTERMS
#define PRICING_DATA_MARKETTERMS		      "MarketTerms"		//  data name of MarketTerms
#endif
#ifndef PRICING_DATA_MARKETVOLDD
#define PRICING_DATA_MARKETVOLDD		      "MarketVolDD"		//  data name of MarketVolDD
#endif
#ifndef PRICING_DATA_MARKETSKEWDD
#define PRICING_DATA_MARKETSKEWDD		      "MarketSkewDD"		//  data name of MarketSkewDD
#endif
#ifndef PRICING_DATA_AVERAGEMARKETVOLDD
#define PRICING_DATA_AVERAGEMARKETVOLDD		   "AverageMarketVolDD"		//  data name of AverageMarketVolDD
#endif
#ifndef PRICING_DATA_AVERAGEMARKETSKEWDD
#define PRICING_DATA_AVERAGEMARKETSKEWDD	   "AverageMarketSkewDD"		//  data name of AverageMarketSkewDD
#endif
#ifndef PRICING_DATA_MODELVOLDD
#define PRICING_DATA_MODELVOLDD		      "ModelVolDD"		//  data name of MarketVolDD
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


// Function ID for LAPriceFXDisplacedDiffusionCalibration
#define FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION		10043
// Function name for LAPriceFXDisplacedDiffusionCalibration
#define FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION_STR	"fn_ir_fxdisplaceddiffusioncalibration"


class LAObject;
class LADataProcedure;
class LAPriceDataManager;
class LADataMultiReference;
class LARatesPathElementCurve;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Generate Cashlets and trigger/call schedule

*/

class LAPriceFXDisplacedDiffusionCalibration : public LACoreProcedure
{
public:
//  LIFECYCLE
    // constructor
	LAPriceFXDisplacedDiffusionCalibration();
    // destructor	
	virtual ~LAPriceFXDisplacedDiffusionCalibration();
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
	// make object for grid calibration data
			void				makeAttributeForGridEntity(const LADate& basedate,
															LAObject& object,
															const LADataProcedure& att) const ;

protected:
    // copy constructor
	LAPriceFXDisplacedDiffusionCalibration(const LAPriceFXDisplacedDiffusionCalibration& p);
	
private:
	class Comp_term
	{
	public:
		/*!
			@brief compare term
			@param[in] _Left one object
			@param[in] _Right another object
			@return true when right argument maturity date > left argument maturity date
		*/
		bool operator()(const LAObject* _Left, const LAObject* _Right) const
		{
			LADate asof, ldate, rdate;
			LAString lterm = dynamic_cast<const LADataString&> ((_Left->getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL)).get()).get();
			LAString rterm = dynamic_cast<const LADataString&> ((_Right->getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL)).get()).get();
			asof.setSystemDate();
			ldate = LAMathDateCalculations::getDate(asof, lterm, true);
			rdate = LAMathDateCalculations::getDate(asof, rterm, true);
			return ldate < rdate;
			
		};
	};

};
