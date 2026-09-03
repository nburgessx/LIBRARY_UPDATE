#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreProcedure.h"
#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LAPriceCashFlowGenerator.h"


// Function ID for LAPriceOptionGenerator
#define FN_IR_OPTIONGENERATOR		2016
// Function name for LAPriceOptionGenerator
#define FN_IR_OPTIONGENERATOR_STR	"fn_ir_optiongenerator"


#ifndef CALIBRATION_DATA_OPTIONLEGS
#define CALIBRATION_DATA_OPTIONLEGS				"OptionLegs"				//  data name of OptionLegs
#endif
#ifndef PRICING_DATA_OPTIONINFORS
#define PRICING_DATA_OPTIONINFORS		"OptionInfos"				//  data name of OptionInfos
#endif





class AQLObject;
class AQLDataProcedure;
class AQLPriceDataManager;
class AQLDataMultiReference;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Generate Cashlets and trigger/call schedule

*/
class LAPriceOptionGenerator : public LAPriceCashFlowGenerator
{
public:
//  LIFECYCLE
    // constructor
	LAPriceOptionGenerator();
    // destructor	
	virtual ~LAPriceOptionGenerator();
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
	LAPriceOptionGenerator(const LAPriceOptionGenerator& p);

private:
	// Create cashlets
	void						createOptions(AQLObject& leg, AQLObject& trade) const;  

	
	// setup leg maturity
	void						setUpLegMaturity(AQLObject& trade) const;
};
