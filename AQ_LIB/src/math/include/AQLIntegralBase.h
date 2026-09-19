#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCoreFunctionBase.h"


// ID for AQLIntegralBase
#define FN_INTEGRALBASE     1401
// Function name for AQLIntegralBase
#define FN_INTEGRALBASE_STR	"fn_integralbase"


class AQLFunctionBase;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of integration method

    This class has pure virtual method "integrate".

*/
class AQLIntegralBase : public AQLCoreFunctionBase
{
public:
	// Default constructor
	AQLIntegralBase();
	// Destructor
	virtual ~AQLIntegralBase();

								//======================================
								// Check function for this class type
    virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const =0;
								//======================================
								// Return this class type
    virtual function_t          getType() const;



	// pure virtual method for integration
	virtual double integrate(const AQLFunctionBase& f,
								const std::vector<std::pair<double,double> >& x
								) const = 0;

private:

protected:

};

