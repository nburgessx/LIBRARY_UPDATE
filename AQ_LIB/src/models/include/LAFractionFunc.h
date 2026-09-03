#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"



// Funciton ID of LADigitalMethod
#define FN_FRACTION	1231
// Function Name of LADigitalMethod
#define FN_FRACTION_STR	"fn_fraction"



///////////////////////////////////////////////////////////////////////
/*!
@brief Class declaration to represent fraction function.

This class derives from LAFunctionBase

*/
class LAFractionMethod : public LAFunctionBase
{
public:
	//  LIFECYCLE
	// constructor
	LAFractionMethod();
	// destructor
	virtual ~LAFractionMethod();

	// assignment operator
	LAFractionMethod & operator=(const LAFractionMethod &) { return *this; }


	//  QUERY
	//======================================
	// Check function for this class type
	virtual bool                isTypeOf(function_t id) const;
	//======================================
	// Make copy(clone) of this class
	virtual LACoreFunctionBase*     clone() const;
	//======================================
	// Return this class type
	virtual function_t          getType() const;

	//==========================================
	// return function value
	virtual double				operator()(const DoubleArray& x) const;

	//==========================================
	// return function value
	virtual double				operator()(double x) const
	{
		return LAFunctionBase::operator()(x);
	}


	// 20060929 override virtual function of base class
	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
	{
		return LAFunctionBase::operator()(pos, x);
	};

private:

protected:


};
