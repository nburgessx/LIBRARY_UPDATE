#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunctionBase.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LABlackScholesCapletOptionPayoff.h"



// Funciton ID of LABlackScholesBaseMethod
#define FN_IR_COLLAROPTIONFUNC	1263
// Function Name of LABlackScholesBaseMethod
#define FN_IR_COLLAROPTIONFUNC_STR	"fn_ir_collaroptionfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent linear function.

	This class derives from LABlackScholesBaseMethod

*/
class LABlackScholesCollarOption : public LABlackScholesCapletOption
{
public:
//  LIFECYCLE
	// constructor
	LABlackScholesCollarOption();
	// destructor
	virtual ~LABlackScholesCollarOption();

	LABlackScholesCollarOption & operator=( const LABlackScholesCollarOption & ) { return *this; }


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
	virtual	LAFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return LAFunctionBase::operator()(pos, x);
								};

	virtual std::vector<AnalyticParam* > getAnalyticParam(void);

	virtual LAStringVector getOptionTypeVector(); 

	virtual LAStringVector getBSComponentVector(LAString risktype) const;

	virtual LAStringVector getBSPayoffComponentVector(LAString risktype) const;

	//virtual void setOptionTerm(double Te);
	
	//virtual void setOptionNumeraire(void);


//  OPERATION

private:

protected:
	

};
