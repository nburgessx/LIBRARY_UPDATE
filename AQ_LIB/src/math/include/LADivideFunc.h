#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"

// Funciton ID of LADivideMethod
#define FN_DIVIDE	1219
// Function Name of LADivideMethod
#define FN_DIVIDE_STR	"fn_divide"

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent divide function.

	This class derives from LAFunctionBase

*/
class LADivideMethod : public LAFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	LADivideMethod();
	// destructor
	virtual ~LADivideMethod();

	LADivideMethod & operator=( const LADivideMethod & ) { return *this; }


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

//  OPERATION

private:

protected:
	

};

