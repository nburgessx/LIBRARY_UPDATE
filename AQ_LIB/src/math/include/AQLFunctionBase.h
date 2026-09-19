#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreFunctionBase.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include <limits>


// ID for AQLFunctionBase
#define FN_FUNCTION	1201 

class AQLIntegralBase;
class AQL1DIntegral;
class AQLCombinationMethod;

// calcutype default ,analytical or numerical
enum CALC_TYPE {
	DEFAULT,
	ANALYTICAL,
	NUMERICAL
};

// difftype bothside or oneside
enum DIFF_TYPE {
	BOTHSIDE,
	ONESIDE
};

#ifndef __SUNPRO_CC
const double POSITIVE_INFINITY	= std::numeric_limits<double>::infinity();	// positive infinity
const double NEGATIVE_INFINITY	= -std::numeric_limits<double>::infinity(); // negative infinity
#else
#define POSITIVE_INFINITY	std::numeric_limits<double>::infinity()	// positive infinity
#define NEGATIVE_INFINITY	(-std::numeric_limits<double>::infinity()) // negative infinity
#endif


///////////////////////////////////////////////////////////////////////
/*! 
    @brief Declaration of abstract base class of function class

    Declare abstract base class to represent function(y=f(x1,x2,...,xn)).
	This class has a pure virtual method "operator()".

*/
class AQLFunctionBase : public AQLCoreFunctionBase
{
public:
	// Default constructor
	explicit AQLFunctionBase(bool checkflag = false);
	//	Copy constructor
//	AQLFunctionBase(const AQLFunctionBase& v);
	// Destructor
	virtual ~AQLFunctionBase();

								//======================================
								// Check function for this class ID
	virtual bool                isTypeOf(function_t id) const;
								//======================================
								// Make copy(clone) of this class
	virtual AQLCoreFunctionBase*		clone() const =0;
								//======================================
								// Return this class ID
	virtual function_t			getType() const;
	                            //==========================================
	                            // Return y(=f(x))
	virtual double				operator()(const DoubleArray& x) const=0;
	                            //==========================================
	                            // Return function value
	virtual double				operator()(double x) const;
	
	                            //==========================================
	                            // regard this function as 1-dim fucntion
	virtual	AQLFunctionBase&		operator()(unsigned int pos, const DoubleArray& x);
	
	                            //==========================================
	                            // operator definition of "+"
	virtual AQLCombinationMethod	operator+(const AQLFunctionBase& method) const;
	
	                            //==========================================
	                            // operator definition of "+"
	virtual AQLCombinationMethod	operator+(double a) const;
	
	                            //==========================================
	                            // operator definition of "*"
	virtual AQLCombinationMethod	operator*(const AQLFunctionBase& method) const;
	
	                            //==========================================
	                            // operator definition of "*"
	virtual AQLCombinationMethod	operator*(double a) const;


	                            //==========================================
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x,
										const AQLIntegralBase* pIntegral) const;
	                            //==========================================
	                            // Return integral result
	virtual double				integral(const std::vector<std::pair<double,double> >& x) const;
	                            //==========================================
	                            // Return integral result
	virtual double				integral(double xl, double xu,
										const AQL1DIntegral* pIntegral) const;
	                            //==========================================
	                            // Return integral result
//	virtual double				integral(double xl, double xu,
//										const AQL1DIntegral* pIntegral, const DoubleArray& grids) const;
	                            //==========================================
	                            // Return integral result
	virtual double				integral(double xl, double xu) const;
	                            //==========================================
	                            // Return partial derivative
	virtual double				partialDerivative(const DoubleArray& x, unsigned int pos,
										CALC_TYPE calctype = DEFAULT, 
										DIFF_TYPE difftype = BOTHSIDE, 
										double delta=0.0001) const;

	                            //==========================================
	                            // Return second partial derivative
	virtual double				partialDerivative2(const DoubleArray& x, unsigned int posi, unsigned int posj,
												CALC_TYPE calctype = DEFAULT, 
												double delta = 0.0001) const;

	                            //==========================================
	                            // calculate function gradient
	virtual void				gradient(const DoubleArray& x, DoubleArray& out,
												CALC_TYPE calctype = DEFAULT, 
												DIFF_TYPE difftype = BOTHSIDE, 
												double delta = 0.0001) const;

	                            //==========================================
	                            // calculate function hessian
	virtual void				hessian(const DoubleArray& x, DoubleMatrix& out,
												CALC_TYPE calctype=DEFAULT, 
												double delta=0.0001) const;

// CheckMethod
	                            //==========================================
								// check x is in domain of this function or not.
	virtual	bool				isInDomain(const DoubleArray& x) const;
								//==========================================
	                            // check derivable or not.
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int pos) const;
								//==========================================
	                            // check double derivable or not.	
	virtual	bool				isDifferentiable(const DoubleArray& x, unsigned int posi, unsigned int posj) const;

								//==========================================
	                            // get function parameter
	virtual	const DoubleArray&
								getParam() const {return mParam;} 
								//==========================================
	                            // get discontinuaous points for 1DIntegral
	virtual	const std::set<double>&
								getDiscontPoints() const {return mDiscontPoints;}
								//==========================================
	                            // get function paremeter
//	virtual	DoubleArray&		getParam();
								//==========================================
	                            // return check flag
	virtual	bool				isCheck() const {return mCheckFlag;};
								//==========================================
								// return string representaion
    virtual AQLString			convertToString(void) const;

								//==========================================
								// transform from string representaion
     virtual void				convertFromString(const AQLString& str);//added by matsumura 20061204
								//==========================================
	                            // set check flag
	virtual void				setCheckFlag(bool checkflag = true);
								//==========================================
	                            // set function parameter
	virtual void				setParam(const DoubleArray& param);
								//==========================================
	                            // set discontinuaous points for 1DIntegral
	virtual void				setDiscontPoints(const std::set<double>& points);
								// equal operator
	AQLFunctionBase&				operator = (const AQLFunctionBase& v);

private:
	unsigned int mPos;				// specific dimention

protected:
								//==========================================
	                            // get specific dimention
	unsigned int 				getPos() const {return mPos;}
	DoubleArray mX;					// default x values
	mutable bool mUpdateFlag;		// function parameter is updated or not
	DoubleArray mParam;				// function parameter
	bool mCheckFlag;				// check flag
	std::set<double> mDiscontPoints;// discontinuaous information for 1DIntegral 
};

