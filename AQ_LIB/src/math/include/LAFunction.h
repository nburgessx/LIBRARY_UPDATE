#pragma once

#include "LAFunctionBase.h"
#include <vector>

//using namespace std;

// ID for LAFunction
#define FN_FUNC	1210 

//
class LAFunction : public LAFunctionBase
{

public:
    
	// Default constructor
    LAFunction() {};
    
	// Copy constructor
    LAFunction( const LAFunction& rhs ) : LAFunctionBase(rhs) {};
    
	// Destructor
    ~LAFunction() {};


	//======================================
	// Check function for this class ID
    bool isTypeOf(function_t id) const { return (id==FN_FUNC ? true : LACoreFunctionBase::isTypeOf(id)); }
 	
	//======================================
	// Return this class ID
	function_t getType() const { return FN_FUNC; }

	 //======================================
	// Make copy(clone) of this class
	virtual LACoreFunctionBase* clone() const { return new LAFunction(*this); }
                                
	//==========================================
	// Return y(=f(double x))
    virtual double operator()( double x )
    { 
        return 0.0;
    }

	//==========================================
	// Return y(=f(double x) const)
    virtual double operator()( double x ) const
    { 
        return 0.0;
    }

	//==========================================
	// Return y(=f(const DoubleArray&  x))
    virtual double operator()( const DoubleArray&  x )
    { 
        return 0.0;
    }

	//==========================================
	// Return y(=f(const DoubleArray&  x) const)
    virtual double operator()( const DoubleArray&  x ) const
    { 
        return 0.0;
    }

	//==========================================
    // Brent's Method
    double SolveBR( double x1,              //[in] Lower bound.
                    double x2,              //[in] Upper bound.
                    size_t num_srch,        //[in] Maximum number of iteration.
                    double tol              //[in] Tolerance.
                  );

    DoubleArray SolveBR2( const DoubleArray& low,
                          const DoubleArray& high,
                          const DoubleArray& initial,
                          size_t num_srch,
                          double tol
                        );

	double IntegrateGLegendre(double a, double b, size_t n)  const;

	//==========================================
	// Euler Integration
    double IntegrateEuler(double a, double b, size_t n) const;
	
	//==========================================
	// PC Integration
	double Integrate_GL_piecewise(double a, double b, const std::vector<double>& x, size_t n) const;
	
	//==========================================
	// PC Integration    
    double Integrate_Array(double a, double b, const std::vector<double>& x, const std::vector<double>& y ) const;
	
	//==========================================
	// Euler Integration
    //double IntegrateEuler(double a, double b, size_t n)  const;
	
	//==========================================
	// Gauss-Laguerre Integration
    double IntegrateGLaguerre(size_t n, double alpha = 0.)  const;
	
	//==========================================
    // Gauss-Lobatto Integration
	double IntegrateGLobatto(double a, double b, size_t n) const;
	
	//==========================================
    double Solve1D_Brent();
};

//
template <class C>
class LAMathFunction : public LAFunction
{

public:

    // Default constructor
    LAMathFunction() {};

	// Copy constructor
    LAMathFunction( const LAMathFunction& rhs )
	:	LAFunction(rhs),
		obj(rhs.obj),
		func1(rhs.func1),
		func2(rhs.func2),
		func3(rhs.func3),
		func4(rhs.func4)
	{}

	// Constructor
    LAMathFunction( C& obj_ ) : obj(&obj_) {};
    
	// Constructor
    LAMathFunction( C& obj_, double (C::*func_)(double) ) : obj(&obj_), func1(func_) {};
    
	// Constructor
    LAMathFunction( C& obj_, double (C::*func_)(double) const ) : obj(&obj_), func2(func_) {};
    
	// Constructor
    LAMathFunction( C& obj_, double (C::*func_)(const DoubleArray& x) ) : obj(&obj_), func3(func_) {};
    
	// Constructor
    LAMathFunction( C& obj_, double (C::*func_)(const DoubleArray& x) const ) : obj(&obj_), func4(func_) {};

	// Destructor
    ~LAMathFunction() {};

	// Check function for this class ID   
    bool isTypeOf(function_t id) const { return true; }
 								
	// Return this class ID
    function_t getType() const { return FN_FUNC; }
								
	// Make copy(clone) of this class
    LACoreFunctionBase* clone() const { return new LAMathFunction(*this); }
                                
    // SetUp func1
    void SetFunc( double (C::*func_)(double) ) { func1 = func_; }
	
	// SetUp func2
    void SetFunc( double (C::*func_)(double) const ) { func2 = func_; }
                                
	// SetUp func3
    void SetFunc( double (C::*func_)(const DoubleArray&) ) { func3 = func_; }
                                
	// SetUp func4
    void SetFunc( double (C::*func_)(const DoubleArray&) const ) { func4 = func_; }
                                
	// SetUp obj & func1
    void SetFunc( C& obj_, double (C::*func_)(double) ) { obj = &obj_; func1 = func_; }
                                
	// SetUp obj & func2
    void SetFunc( C& obj_, double (C::*func_)(double) const ) { obj = &obj_; func2 = func_; }
                                
	// SetUp obj & func3
    void SetFunc( C& obj_, double (C::*func_)(const DoubleArray&) ) { obj = &obj_; func3 = func_; }
                                
	// SetUp obj & func4
    void SetFunc( C& obj_, double (C::*func_)(const DoubleArray&) const ) { obj = &obj_; func4 = func_; }
	                            
	// Return y(=func1(x))
    virtual double operator()( double x ) { return (obj->*func1)(x); }
	                            
	// Return y(=func2(x))
    virtual double operator()( double x ) const { return (obj->*func2)(x); }
	                            
	// Return y(=func3(x))
    virtual double operator()( const DoubleArray&  x ) { return (obj->*func3)(x); }
	                            
	// Return y(=func4x))
    virtual double operator()( const DoubleArray&  x ) const { return (obj->*func4)(x); }

private :
    
    C* obj;

    double (C::*func1)( double );

    double (C::*func2)( double ) const;

    double (C::*func3)( const DoubleArray&);
    
    double (C::*func4)( const DoubleArray&) const;

};
