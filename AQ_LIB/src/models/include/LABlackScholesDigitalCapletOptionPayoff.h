/*! @file
    @brief Class declaration to represent linear function

	This class derives from LABlackScholesBaseMethod
*/
//  2010, AlgoQuantHub.
#ifndef LABlackScholesDigitalCapletOption_h
#define LABlackScholesDigitalCapletOption_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LABlackScholesBaseMethod.h
//
//  SYNOPSIS    :       LABlackScholesBaseMethod
//  DESCRIPTION :       Class declaration to represent linear function
//						This class derives from LAFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LABlackScholesCapletOptionPayoff.h"



// Funciton ID of LABlackScholesBaseMethod
#define FN_IR_DIGITALCAPLETOPTIONFUNC	1272
// Function Name of LABlackScholesBaseMethod
#define FN_IR_DIGITALCAPLETOPTIONFUNC_STR	"fn_ir_digitalcapletoptionfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent digital caplet function.

	This class derives from LABlackScholesCapletOption

*/
class LABlackScholesDigitalCapletOption : public LABlackScholesCapletOption
{
public:
//  LIFECYCLE
	// constructor
	LABlackScholesDigitalCapletOption();
	// destructor
	virtual ~LABlackScholesDigitalCapletOption() = default;

	LABlackScholesDigitalCapletOption & operator=( const LABlackScholesDigitalCapletOption & )  
	{ 
		return *this; 
	}

//  overridden functions
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const override;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*     clone() const override;
								//======================================
								// Return this class type
    virtual function_t          getType() const override ;
								
	                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const override;

	virtual LAStringVector getOptionTypeVector() override;

	virtual LAStringVector getBSComponentVector(LAString risktype) const override;

	virtual LAStringVector getBSPayoffComponentVector(LAString risktype) const override;

	virtual std::vector<AnalyticParam* > getAnalyticParam(void) override;

	virtual void setOptionStrike(void) override;

//  newly defined functions

	virtual  void setCallSpreadValue(double callSpreadValue);

	virtual double	getCallSpreadValue() const;

	virtual void setDigitalCoupon(double digitalCoupon);

	virtual double	getDigitalCoupon() const;

private:
	double mCallSpreadValue;
	double mDigitalCoupon;

protected:


};

#endif 
