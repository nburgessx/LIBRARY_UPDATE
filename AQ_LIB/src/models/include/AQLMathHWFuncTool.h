#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathHWFuncMR.h"
#include "AQLMathHWFuncSigma.h"
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
class AQLMathHWFuncTool : public AQLFunctionBase
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLMathHWFuncTool(AQLMathHWFuncMR& funcHWMR, AQLMathHWFuncSigma& funcHWSigma);
	//copy constructor
    AQLMathHWFuncTool(const AQLMathHWFuncTool &rhs);
	// Destructor
	virtual ~AQLMathHWFuncTool();

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
	AQLMathHWFuncMR*		getHWMR(void) const{return mpHWMR;};
	AQLMathHWFuncSigma*	getHWSigma(void)  const {return mpHWSigma;};
private:

protected:
	AQLMathHWFuncMR* mpHWMR;
	AQLMathHWFuncSigma* mpHWSigma;
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
class AQLMathHWFuncToolForVar : public AQLMathHWFuncTool
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLMathHWFuncToolForVar(AQLMathHWFuncMR& funcHWMR, AQLMathHWFuncSigma& funcHWSigma);
	//copy constructor
    AQLMathHWFuncToolForVar(const AQLMathHWFuncToolForVar &rhs);
//	AQLFunctionBase(const AQLFunctionBase& v);
	// Destructor
	virtual ~AQLMathHWFuncToolForVar();

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
class AQLMathHWFuncToolForMR : public AQLMathHWFuncTool
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLMathHWFuncToolForMR(AQLMathHWFuncMR& funcHWMR, AQLMathHWFuncSigma& funcHWSigma);
	//copy constructor
    AQLMathHWFuncToolForMR(const AQLMathHWFuncToolForMR &rhs);
//	AQLFunctionBase(const AQLFunctionBase& v);
	// Destructor
	virtual ~AQLMathHWFuncToolForMR();

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
class AQLMathHWFuncToolForMRIntegral : public AQLMathHWFuncTool
{
public:
//  LIFECYCLE
	// Default constructor
	explicit AQLMathHWFuncToolForMRIntegral(AQLMathHWFuncMR& funcHWMR, AQLMathHWFuncSigma& funcHWSigma);
	//copy constructor
    AQLMathHWFuncToolForMRIntegral(const AQLMathHWFuncToolForMRIntegral &rhs);
//	AQLFunctionBase(const AQLFunctionBase& v);
	// Destructor
	virtual ~AQLMathHWFuncToolForMRIntegral();

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
	
	AQLMathHWFuncToolForMR* mpToolMR;
	AQLGaussLegendre mGL;
private:

protected:
};

