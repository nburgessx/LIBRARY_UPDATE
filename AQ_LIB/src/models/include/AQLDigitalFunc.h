#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
 


// Funciton ID of AQLDigitalMethod
#define FN_DIGITAL	1230
// Function Name of AQLDigitalMethod
#define FN_DIGITAL_STR	"fn_digital"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent average function.

	This class derives from AQLFunctionBase

*/
class AQLDigitalMethod : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	AQLDigitalMethod();
	// destructor
	virtual ~AQLDigitalMethod();

	// assignment operator
	AQLDigitalMethod & operator=( const AQLDigitalMethod & ) { return *this; }


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
