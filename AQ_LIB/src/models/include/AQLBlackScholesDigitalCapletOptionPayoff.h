/*! @file
    @brief Class declaration to represent linear function

	This class derives from AQLBlackScholesBaseMethod
*/
//  2010, AlgoQuantHub.
#ifndef AQLBlackScholesDigitalCapletOption_h
#define AQLBlackScholesDigitalCapletOption_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLBlackScholesBaseMethod.h
//
//  SYNOPSIS    :       AQLBlackScholesBaseMethod
//  DESCRIPTION :       Class declaration to represent linear function
//						This class derives from AQLFunctionBase
//                      
//  VERSION		:
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBlackScholesCapletOptionPayoff.h"



// Funciton ID of AQLBlackScholesBaseMethod
#define FN_IR_DIGITALCAPLETOPTIONFUNC	1272
// Function Name of AQLBlackScholesBaseMethod
#define FN_IR_DIGITALCAPLETOPTIONFUNC_STR	"fn_ir_digitalcapletoptionfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent digital caplet function.

	This class derives from AQLBlackScholesCapletOption

*/
class AQLBlackScholesDigitalCapletOption : public AQLBlackScholesCapletOption
{
public:
//  LIFECYCLE
	// constructor
	AQLBlackScholesDigitalCapletOption();
	// destructor
	virtual ~AQLBlackScholesDigitalCapletOption() = default;

	AQLBlackScholesDigitalCapletOption & operator=( const AQLBlackScholesDigitalCapletOption & )  
	{ 
		return *this; 
	}

//  overridden functions
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const override;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*     clone() const override;
								//======================================
								// Return this class type
    virtual function_t          getType() const override ;
								
	                            //==========================================
								// return function value
	virtual double				operator()(const DoubleArray& x) const override;

	virtual AQLStringVector getOptionTypeVector() override;

	virtual AQLStringVector getBSComponentVector(AQLString risktype) const override;

	virtual AQLStringVector getBSPayoffComponentVector(AQLString risktype) const override;

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
