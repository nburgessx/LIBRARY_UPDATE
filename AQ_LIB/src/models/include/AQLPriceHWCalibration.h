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
#include "AQLDateCalculations.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLMathJamshidianSwaption.h"


// Function ID for AQLPriceHWCalibration
#define FN_IR_HWCALIBRATION		10040
// Function name for AQLPriceHWCalibration
#define FN_IR_HWCALIBRATION_STR	"fn_ir_hwcalibration"


class AQLObject;
class AQLDataProcedure;
class AQLPriceDataManager;
class AQLDataMultiReference;
class AQLRatesPathElementCurve;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Generate Cashlets and trigger/call schedule

*/

class AQLPriceHWCalibration : public AQLCoreProcedure
{
public:
//  LIFECYCLE
    // constructor
	AQLPriceHWCalibration();
    // destructor	
	virtual ~AQLPriceHWCalibration();
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

protected:
    // copy constructor
	AQLPriceHWCalibration(const AQLPriceHWCalibration& p);
	
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
			ldate = AQLDateCalculations::getDate(asof, lterm, true);
			rdate = AQLDateCalculations::getDate(asof, rterm, true);
			return ldate < rdate;
			
		};
	};

	class Comp_swaptenor
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
			AQLString lterm = dynamic_cast<const AQLDataString&> ((_Left->getData(IR_CALIBRATION_DATA_SWAPTENOR, ISNOTNULL)).get()).get();
			AQLString rterm = dynamic_cast<const AQLDataString&> ((_Right->getData(IR_CALIBRATION_DATA_SWAPTENOR, ISNOTNULL)).get()).get();
			asof.setSystemDate();
			ldate = AQLDateCalculations::getDate(asof, lterm, true);
			rdate = AQLDateCalculations::getDate(asof, rterm, true);
			return ldate < rdate;
		};
	};
	
};
