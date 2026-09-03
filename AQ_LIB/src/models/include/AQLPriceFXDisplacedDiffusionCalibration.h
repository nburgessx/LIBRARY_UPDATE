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

#include "AQLCoreProcedure.h"
#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLOptimumBrent.h"
#include "AQLModelDynamicsBase.h"
#include "AQLDataBasics.h"
#include "AQLObject.h"
#include "AQLMathDateCalculations.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLMathAntonovFXOption.h"


// Function ID for AQLPriceFXDisplacedDiffusionCalibration
#define FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION		10043
// Function name for AQLPriceFXDisplacedDiffusionCalibration
#define FN_IR_FXDISPLACEDDIFFUSIONCALIBRATION_STR	"fn_ir_fxdisplaceddiffusioncalibration"


class AQLObject;
class AQLDataProcedure;
class AQLPriceDataManager;
class AQLDataMultiReference;
class AQLRatesPathElementCurve;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Generate Cashlets and trigger/call schedule

*/

class AQLPriceFXDisplacedDiffusionCalibration : public AQLCoreProcedure
{
public:
//  LIFECYCLE
    // constructor
	AQLPriceFXDisplacedDiffusionCalibration();
    // destructor	
	virtual ~AQLPriceFXDisplacedDiffusionCalibration();
    // Check this class ID is the same or not	
	virtual bool                isTypeOf(function_t id) const;
    // Copy this class
	virtual AQLCoreFunctionBase*		clone() const;// %%% COVARIANT RETURN %%%
    // Return class type
	virtual function_t			getType() const;
    // Generate cashlets and trigger/call schedule
	virtual void	            calibrateModel(const AQLDate& basedate, 
										AQLObject& object, 
										const AQLDataProcedure& att) const;
	// register dataValues that this class uses
	virtual void				registerData(AQLPriceDataManager& dm) const;
	// make object for grid calibration data
			void				makeAttributeForGridEntity(const AQLDate& basedate,
															AQLObject& object,
															const AQLDataProcedure& att) const ;

protected:
    // copy constructor
	AQLPriceFXDisplacedDiffusionCalibration(const AQLPriceFXDisplacedDiffusionCalibration& p);
	
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
		bool operator()(const AQLObject* _Left, const AQLObject* _Right) const
		{
			AQLDate asof, ldate, rdate;
			AQLString lterm = dynamic_cast<const AQLDataString&> ((_Left->getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL)).get()).get();
			AQLString rterm = dynamic_cast<const AQLDataString&> ((_Right->getData(IR_CALIBRATION_DATA_OPTIONMATURITY, ISNOTNULL)).get()).get();
			asof.setSystemDate();
			ldate = AQLMathDateCalculations::getDate(asof, lterm, true);
			rdate = AQLMathDateCalculations::getDate(asof, rterm, true);
			return ldate < rdate;
			
		};
	};

};
