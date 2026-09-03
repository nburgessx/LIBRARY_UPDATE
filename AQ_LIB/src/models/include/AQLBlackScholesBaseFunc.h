#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLBlackScholesCalc.h"
#include "AQLAnalyticFormula.h"



// Funciton ID of AQLBlackScholesBaseMethod
#define FN_BSBASEFUNC	1260
// Function Name of AQLBlackScholesBaseMethod
#define FN_BSBASEFUNC_STR	"fn_bsbasefunc"
// minimum rate
#define MIN_RATE 0.0000001 //0.001bp



///////////////////////////////////////////////////////////////////////
/*! 
    @brief Class declaration to represent linear function.

	This class derives from AQLFunctionBase

*/
class AQLBlackScholesBaseMethod : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// constructor
	AQLBlackScholesBaseMethod();
	// destructor
	virtual ~AQLBlackScholesBaseMethod();

	// assignment operator
	AQLBlackScholesBaseMethod & operator=( const AQLBlackScholesBaseMethod & ) { return *this; }


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

	virtual std::vector<AQLBlackScholesBase* > getAnalyticMethod(AQLString risktype);

	virtual std::vector<AQLBlackScholesBase* > getPayoffMethod(AQLString risktype);

	virtual std::vector<AnalyticParam* > getAnalyticParam(void) { return std::vector<AnalyticParam* >(0);};

	virtual AQLStringVector getOptionTypeVector(){return AQLStringVector();};

	virtual AQLStringVector getBSComponentVector(AQLString risktype) const {return AQLStringVector();};

	virtual AQLStringVector getBSPayoffComponentVector(AQLString risktype) const {return AQLStringVector();};

	virtual void setOptionTerm(double Te) {};
	
	virtual void setOptionActualTerm(double actT){};
	
	virtual void setOptionNumeraire(void) {};

	virtual void setOptionStrike(void) {};

	virtual void setIsAfterMaturity(bool isaftermaturity){};

	virtual void setConvexityFactors(const DoubleArray& x) {};

	void setForwardShiftValue(double forwardshiftvalue);

	std::vector<AQLString > mOptionType;

	std::vector<AQLBlackScholesBase* > mBSAnalyticMethod;
	std::vector<AQLBlackScholesBase* > mBSPayoffMethod;

	std::vector<AnalyticParam* >mBSInputParam;
	bool mIsAfterMaturity;
	double mForwardShiftValue;
	BoolVector mFixedPayOffs;

//  OPERATION

private:

protected:
	//slide forward and strike of input parameter
	void getForwardShiftParam(AnalyticBKParam* param) const;
	

};

