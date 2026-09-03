#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "LAMathHWFuncMR.h"
#include "LAMathHWFuncSigma.h"
#include "AQLBasic.h"
#include <limits>
#include "AQLGaussLegendre.h"


// ID for AQLFunctionBase
#define FN_HWFUNCTIONTOOL	10023 

class AQLIntegralBase;
class AQL1DIntegral;
class AQLCombinationMethod;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of function class

    Declare abstract base class to represent function(y=f(x1,x2,...,xn)).
	This class has a pure virtual method "operator()".

*/
class LAMathHWFuncTool : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAMathHWFuncTool(LAMathHWFuncMR& funcHWMR, LAMathHWFuncSigma& funcHWSigma);
	//copy constructor
    LAMathHWFuncTool(const LAMathHWFuncTool &rhs);
	// Destructor
	virtual ~LAMathHWFuncTool();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
	                            //==========================================
	                            // Return y(=f(x))
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
	                            // Return function value
	virtual double				operator()(double x) const;

	double	ExpIntegralMR(double T) const
	{
		return AQLMath::exp( mpHWMR->integrate(0,T) );
	};
	double ExpIntegralInvMR(double T) const 
	{
		return AQLMath::exp( - mpHWMR->integrate(0,T) );
	}
	
	//AQLGaussLegendre mGL;
	LAMathHWFuncMR*		getHWMR(void) const{return mpHWMR;};
	LAMathHWFuncSigma*	getHWSigma(void)  const {return mpHWSigma;};
private:

protected:
	LAMathHWFuncMR* mpHWMR;
	LAMathHWFuncSigma* mpHWSigma;
};


// ID for AQLFunctionBase
#define FN_HWFUNCTIONTOOLFORVAR	10024 

class AQLIntegralBase;
class AQL1DIntegral;
class AQLCombinationMethod;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of function class

    Declare abstract base class to represent function(y=f(x1,x2,...,xn)).
	This class has a pure virtual method "operator()".

*/
class LAMathHWFuncToolForVar : public LAMathHWFuncTool
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAMathHWFuncToolForVar(LAMathHWFuncMR& funcHWMR, LAMathHWFuncSigma& funcHWSigma);
	//copy constructor
    LAMathHWFuncToolForVar(const LAMathHWFuncToolForVar &rhs);
//	AQLFunctionBase(const AQLFunctionBase& v);
	// Destructor
	virtual ~LAMathHWFuncToolForVar();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
	                            //==========================================
	                            // Return y(=f(x))
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
	                            // Return function value
	virtual double				operator()(double x) const;

private:

protected:
};


// ID for AQLFunctionBase
#define FN_HWFUNCTIONTOOLFORMR	10025 

class AQLIntegralBase;
class AQL1DIntegral;
class AQLCombinationMethod;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of function class

    Declare abstract base class to represent function(y=f(x1,x2,...,xn)).
	This class has a pure virtual method "operator()".

*/
class LAMathHWFuncToolForMR : public LAMathHWFuncTool
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAMathHWFuncToolForMR(LAMathHWFuncMR& funcHWMR, LAMathHWFuncSigma& funcHWSigma);
	//copy constructor
    LAMathHWFuncToolForMR(const LAMathHWFuncToolForMR &rhs);
//	AQLFunctionBase(const AQLFunctionBase& v);
	// Destructor
	virtual ~LAMathHWFuncToolForMR();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
	                            //==========================================
	                            // Return y(=f(x))
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
	                            // Return function value
	virtual double				operator()(double x) const;

private:

protected:
};


// ID for AQLFunctionBase
#define FN_HWFUNCTIONTOOLFORMRINTEGRAL	10026 

class AQLIntegralBase;
class AQL1DIntegral;
class AQLCombinationMethod;

///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of function class

    Declare abstract base class to represent function(y=f(x1,x2,...,xn)).
	This class has a pure virtual method "operator()".

*/
class LAMathHWFuncToolForMRIntegral : public LAMathHWFuncTool
{
public:
//  LIFECYCLE
	// Default constructor
	explicit LAMathHWFuncToolForMRIntegral(LAMathHWFuncMR& funcHWMR, LAMathHWFuncSigma& funcHWSigma);
	//copy constructor
    LAMathHWFuncToolForMRIntegral(const LAMathHWFuncToolForMRIntegral &rhs);
//	AQLFunctionBase(const AQLFunctionBase& v);
	// Destructor
	virtual ~LAMathHWFuncToolForMRIntegral();

//  QUERY
								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const;
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
	                            //==========================================
	                            // Return y(=f(x))
	virtual double				operator()(const DoubleArray& x) const;
	                            //==========================================
	                            // Return function value
	virtual double				operator()(double x) const;
	
	LAMathHWFuncToolForMR* mpToolMR;
	AQLGaussLegendre mGL;
private:

protected:
};

