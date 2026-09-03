#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// Funciton ID of AQLDigitalMethod
#define FN_FRACTION	1231
// Function Name of AQLDigitalMethod
#define FN_FRACTION_STR	"fn_fraction"



///////////////////////////////////////////////////////////////////////
/*!
@brief Class declaration to represent fraction function.

This class derives from AQLFunctionBase

*/
class AQLFractionMethod : public AQLFunctionBase
{
public:
	//  LIFECYCLE
	// constructor
	AQLFractionMethod();
	// destructor
	virtual ~AQLFractionMethod();

	// assignment operator
	AQLFractionMethod & operator=(const AQLFractionMethod &) { return *this; }


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

	//==========================================
	// return function value
	virtual double				operator()(const DoubleArray& x) const;

	//==========================================
	// return function value
	virtual double				operator()(double x) const
	{
		return AQLFunctionBase::operator()(x);
	}


	// 20060929 override virtual function of base class
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
	{
		return AQLFunctionBase::operator()(pos, x);
	};

private:

protected:


};
