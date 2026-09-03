#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBlackScholesCapletOptionPayoff.h"



// Funciton ID of AQLBlackScholesBaseMethod
#define FN_IR_COLLAROPTIONFUNC	1263
// Function Name of AQLBlackScholesBaseMethod
#define FN_IR_COLLAROPTIONFUNC_STR	"fn_ir_collaroptionfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent linear function.

	This class derives from AQLBlackScholesBaseMethod

*/
class AQLBlackScholesCollarOption : public AQLBlackScholesCapletOption
{
public:
//  LIFECYCLE
	// constructor
	AQLBlackScholesCollarOption();
	// destructor
	virtual ~AQLBlackScholesCollarOption();

	AQLBlackScholesCollarOption & operator=( const AQLBlackScholesCollarOption & ) { return *this; }


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
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x)
								{
									return AQLFunctionBase::operator()(pos, x);
								};

	virtual std::vector<AnalyticParam* > getAnalyticParam(void);

	virtual AQLStringVector getOptionTypeVector(); 

	virtual AQLStringVector getBSComponentVector(AQLString risktype) const;

	virtual AQLStringVector getBSPayoffComponentVector(AQLString risktype) const;

	//virtual void setOptionTerm(double Te);
	
	//virtual void setOptionNumeraire(void);


//  OPERATION

private:

protected:
	

};
