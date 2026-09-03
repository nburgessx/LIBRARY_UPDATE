#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreTemplateType.h"
#include "LACoreFunctionBase.h"


// ID for LAIntegralBase
#define FN_INTEGRALBASE     1401
// Function name for LAIntegralBase
#define FN_INTEGRALBASE_STR	"fn_integralbase"


class LAFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of integration method

    This class has pure virtual method "integrate".

*/
class LAIntegralBase : public LACoreFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	LAIntegralBase();
	// Destructor
	virtual ~LAIntegralBase();

//  QUERY
								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual LACoreFunctionBase*		clone() const =0;
								//======================================
								// Return this class type
    virtual function_t          getType() const;



	// pure virtual method for integration
	virtual double integrate(const LAFunctionBase& f,
								const std::vector<std::pair<double,double> >& x
								) const = 0;

private:

protected:

};

