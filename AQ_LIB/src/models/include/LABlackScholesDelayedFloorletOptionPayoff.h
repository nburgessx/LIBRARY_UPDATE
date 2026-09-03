#pragma once

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LABlackScholesFloorletOptionPayoff.h"


// Funciton ID of LABlackSholesDelayedFloorletOption
#define FN_IR_DELAYEDFLOORLETOPTIONFUNC	1269
// Function Name of LABlackSholesDelayedFloorletOption
#define FN_IR_DELAYEDFLOORLETOPTIONFUNC_STR	"fn_ir_delayedfloorletoptionfunc"

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent delayed floorlet function.

	This class derives from LABlackScholesFloorletOption

*/
class LABlackSholesDelayedFloorletOption : public LABlackScholesFloorletOption
{
public:
//  LIFECYCLE
	// constructor
	LABlackSholesDelayedFloorletOption();
	// destructor
	virtual ~LABlackSholesDelayedFloorletOption();

	LABlackSholesDelayedFloorletOption & operator=( const LABlackSholesDelayedFloorletOption & ) { return *this; }


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

