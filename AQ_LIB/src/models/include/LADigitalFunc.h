#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
 


// Funciton ID of LADigitalMethod
#define FN_DIGITAL	1230
// Function Name of LADigitalMethod
#define FN_DIGITAL_STR	"fn_digital"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent average function.

	This class derives from LAFunctionBase

*/
class LADigitalMethod : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LADigitalMethod();
	// destructor
	virtual ~LADigitalMethod();

	// assignment operator
	LADigitalMethod & operator=( const LADigitalMethod & ) { return *this; }


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
