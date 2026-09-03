#pragma once

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LABlackScholesBaseFunc.h"
#include "LABlackScholesCapletOptionPayoff.h"

// Funciton ID of LABlackScholesDelayedCapletOption
#define FN_IR_DELAYEDCAPLETOPTIONFUNC	1268
// Function Name of LABlackScholesBaseMethod
#define FN_IR_DELAYEDCAPLETOPTIONFUNC_STR	"fn_ir_delayedcapletoptionfunc"

///////////////////////////////////////////////////////////////////////
/*! 
	@brief Class declaration to represent delayed caplet function.

	This class derives from LABlackScholesCapletOption

*/
class LABlackScholesDelayedCapletOption : public LABlackScholesCapletOption
{
public:
//  LIFECYCLE
	// constructor
	LABlackScholesDelayedCapletOption();
	// destructor
	virtual ~LABlackScholesDelayedCapletOption();

	LABlackScholesDelayedCapletOption & operator=( const LABlackScholesDelayedCapletOption & ) { return *this; }

//  QUERY
								//======================================
								// Check function for this class type
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*     clone() const;
								//======================================
								// Return this class type
	virtual function_t          getType() const;

//  OPERATION
	virtual void setConvexityFactors(const DoubleArray& x);

private:
	double mTimingTerm;
	double mDFFromPayment2End;
	double mVolCutoff;

protected:
	virtual double calcConvexityAdjustment(AnalyticParam& param, double premium) const;

};

