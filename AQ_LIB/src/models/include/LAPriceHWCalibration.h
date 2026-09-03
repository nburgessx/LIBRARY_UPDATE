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
#include "LAMathJamshidianSwaption.h"


// Function ID for LAPriceHWCalibration
#define FN_IR_HWCALIBRATION		10040
// Function name for LAPriceHWCalibration
#define FN_IR_HWCALIBRATION_STR	"fn_ir_hwcalibration"


class LAObject;
class LADataProcedure;
class LAPriceDataManager;
class LADataMultiReference;
class LARatesPathElementCurve;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Generate Cashlets and trigger/call schedule

*/

class LAPriceHWCalibration : public LACoreProcedure
{
public:
//  LIFECYCLE
    // constructor
	LAPriceHWCalibration();
    // destructor	
	virtual ~LAPriceHWCalibration();
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

protected:
    // copy constructor
	LAPriceHWCalibration(const LAPriceHWCalibration& p);
	
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

	class Comp_swaptenor
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
			LAString lterm = dynamic_cast<const LADataString&> ((_Left->getData(IR_CALIBRATION_DATA_SWAPTENOR, ISNOTNULL)).get()).get();
			LAString rterm = dynamic_cast<const LADataString&> ((_Right->getData(IR_CALIBRATION_DATA_SWAPTENOR, ISNOTNULL)).get()).get();
			asof.setSystemDate();
			ldate = LAMathDateCalculations::getDate(asof, lterm, true);
			rdate = LAMathDateCalculations::getDate(asof, rterm, true);
			return ldate < rdate;
		};
	};
	
};
