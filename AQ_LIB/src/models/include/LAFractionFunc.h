#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"



// Funciton ID of LADigitalMethod
#define FN_FRACTION	1231
// Function Name of LADigitalMethod
#define FN_FRACTION_STR	"fn_fraction"



///////////////////////////////////////////////////////////////////////
/*!
@brief Class declaration to represent fraction function.

This class derives from AQLFunctionBase

*/
class LAFractionMethod : public AQLFunctionBase
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
