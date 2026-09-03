#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LABlackScholesBaseFunc.h"



// Funciton ID of LABlackScholesBaseMethod
#define FN_IR_CAPLETOPTIONFUNC	1261
// Function Name of LABlackScholesBaseMethod
#define FN_IR_CAPLETOPTIONFUNC_STR	"fn_ir_capletoptionfunc"



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent linear function.

	This class derives from LABlackScholesBaseMethod

*/
class LABlackScholesCapletOption : public LABlackScholesBaseMethod
{
public:
//  LIFECYCLE
	// constructor
	LABlackScholesCapletOption();
	// destructor
	virtual ~LABlackScholesCapletOption();

	//20061017--David--Remove warning:C4512
	LABlackScholesCapletOption & operator=( const LABlackScholesCapletOption & ) { return *this; }


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
	virtual double				operator()(const DoubleArray& x, size_t calcIndex) const;
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

	virtual void setOptionTerm(double Te);
	
	virtual void setOptionActualTerm(double actT);
	
	virtual void setOptionNumeraire(void);

	virtual void setOptionStrike(void);

	virtual void setIsAfterMaturity(bool isaftermaturity);


//  OPERATION

private:

protected:
	virtual double calcConvexityAdjustment(AnalyticParam& /*param*/, double /*premium*/) const { return 0.0; }

};

