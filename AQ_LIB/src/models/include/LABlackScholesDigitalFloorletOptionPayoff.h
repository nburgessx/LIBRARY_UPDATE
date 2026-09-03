/*! @file
@brief Class declaration to represent linear function

This class derives from LABlackScholesBaseMethod
*/
//  2010, AlgoQuantHub.
#ifndef LABlackShcolesDigitalFloorletOption_h
#define LABlackShcolesDigitalFloorletOption_h
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
#include "LABlackScholesDigitalCapletOptionPayoff.h"



// Funciton ID of LABlackScholesBaseMethod
#define FN_IR_DIGITALFLOORLETOPTIONFUNC	1273
// Function Name of LABlackScholesBaseMethod
#define FN_IR_DIGITALFLOORLETOPTIONFUNC_STR	"fn_ir_digitalfloorletoptionfunc"



///////////////////////////////////////////////////////////////////////
/*!
@brief Class declaration to represent digital floorlet function.

This class derives from LABlackScholesDigitalCapletOption

*/
class LABlackShcolesDigitalFloorletOption : public LABlackScholesDigitalCapletOption
{
public:
	//  LIFECYCLE
	// constructor
	LABlackShcolesDigitalFloorletOption();
	// destructor
	virtual ~LABlackShcolesDigitalFloorletOption() = default;

	LABlackShcolesDigitalFloorletOption & operator=(const LABlackShcolesDigitalFloorletOption &)
	{
		return *this;
	}

	//======================================
	// Check function for this class type
	virtual bool                isTypeOf(function_t id) const override;
	//======================================
	// Make copy(clone) of this class
	virtual LACoreFunctionBase*     clone() const override;
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
