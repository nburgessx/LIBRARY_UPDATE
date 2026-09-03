#pragma once

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBlackScholesFloorletOptionPayoff.h"


// Funciton ID of AQLBlackSholesDelayedFloorletOption
#define FN_IR_DELAYEDFLOORLETOPTIONFUNC	1269
// Function Name of AQLBlackSholesDelayedFloorletOption
#define FN_IR_DELAYEDFLOORLETOPTIONFUNC_STR	"fn_ir_delayedfloorletoptionfunc"

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent delayed floorlet function.

	This class derives from AQLBlackScholesFloorletOption

*/
class AQLBlackSholesDelayedFloorletOption : public AQLBlackScholesFloorletOption
{
public:
//  LIFECYCLE
	// constructor
	AQLBlackSholesDelayedFloorletOption();
	// destructor
	virtual ~AQLBlackSholesDelayedFloorletOption();

	AQLBlackSholesDelayedFloorletOption & operator=( const AQLBlackSholesDelayedFloorletOption & ) { return *this; }


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

