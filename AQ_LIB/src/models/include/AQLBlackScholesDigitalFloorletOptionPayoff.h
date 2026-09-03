/*! @file
@brief Class declaration to represent linear function

This class derives from AQLBlackScholesBaseMethod
*/
//  2010, AlgoQuantHub.
#ifndef AQLBlackShcolesDigitalFloorletOption_h
#define AQLBlackShcolesDigitalFloorletOption_h
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
#include "AQLBlackScholesDigitalCapletOptionPayoff.h"



// Funciton ID of AQLBlackScholesBaseMethod
#define FN_IR_DIGITALFLOORLETOPTIONFUNC	1273
// Function Name of AQLBlackScholesBaseMethod
#define FN_IR_DIGITALFLOORLETOPTIONFUNC_STR	"fn_ir_digitalfloorletoptionfunc"



///////////////////////////////////////////////////////////////////////
/*!
@brief Class declaration to represent digital floorlet function.

This class derives from AQLBlackScholesDigitalCapletOption

*/
class AQLBlackShcolesDigitalFloorletOption : public AQLBlackScholesDigitalCapletOption
{
public:
	//  LIFECYCLE
	// constructor
	AQLBlackShcolesDigitalFloorletOption();
	// destructor
	virtual ~AQLBlackShcolesDigitalFloorletOption() = default;

	AQLBlackShcolesDigitalFloorletOption & operator=(const AQLBlackShcolesDigitalFloorletOption &)
	{
		return *this;
	}

	//======================================
	// Check function for this class type
	virtual bool                isTypeOf(function_t id) const override;
	//======================================
	// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*     clone() const override;
	//======================================
	// Return this class type
	virtual function_t          getType() const override;

	//==========================================
	// return function value
	virtual double				operator()(const DoubleArray& x) const override;


private:

protected:


};

#endif 
