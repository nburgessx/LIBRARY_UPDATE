#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
 


// Funciton ID of AQLRatesInterSectionFunc
#define FN_INTERSECTIONFUNC	10001
// Function Name of AQLRatesInterSectionFunc
#define FN_INTERSECTIONFUNC_STR	"fn_intersectionfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent RangeCount function.

	This class derives from AQLFunctionBase

*/
class AQLRatesInterSectionFunc : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	AQLRatesInterSectionFunc();
	// destructor
	virtual ~AQLRatesInterSectionFunc();

	// suppress warning C4512
	AQLRatesInterSectionFunc & operator=( const AQLRatesInterSectionFunc & ) { return *this; }


//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
    virtual AQLCoreFunctionBase*     clone() const;// %%% COVARIANT RETURN %%%
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


	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};

	
//  OPERATION

private:

protected:
	

};

