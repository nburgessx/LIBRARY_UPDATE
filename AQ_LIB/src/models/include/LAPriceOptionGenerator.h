#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreProcedure.h"
#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
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





class LAObject;
class LADataProcedure;
class LAPriceDataManager;
class LADataMultiReference;
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
	LAPriceOptionGenerator(const LAPriceOptionGenerator& p);

private:
	// Create cashlets
	void						createOptions(LAObject& leg, LAObject& trade) const;  

	
	// setup leg maturity
	void						setUpLegMaturity(LAObject& trade) const;
};
