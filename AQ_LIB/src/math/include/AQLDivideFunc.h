#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"

// Funciton ID of AQLDivideMethod
#define FN_DIVIDE	1219
// Function Name of AQLDivideMethod
#define FN_DIVIDE_STR	"fn_divide"

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent divide function.

	This class derives from AQLFunctionBase

*/
class AQLDivideMethod : public AQLFunctionBase
{
public:
	// constructor
	AQLDivideMethod();
	// destructor
	virtual ~AQLDivideMethod();

	AQLDivideMethod & operator=( const AQLDivideMethod & ) { return *this; }


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


private:

protected:
	

};

