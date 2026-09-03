#pragma once

#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif

#include "LACoreAppError.h"


#ifdef _MSC_VER
#undef max    // not VC++H
#undef min
#endif

//===================== LAMath =============================================
/*! 
    @brief Class declaration to provide basic functions.
            Only static functions are implemented on this class.

*/
class LAMath {
public:
    /*! A number representing the precision */
    // function that returns the minimum value in double
    static double           minValue(double& val);
    // function that returns the maximum value in double
    static double           maxValue(double& val);
    // function that returns the numerical calculation error level
    static double           epsValue(double& val); 

    /*! return a constant  */
    // function that returns a pi
    static double           pi(void);           
    // function that returns (e) the number of Napier
    static double           e(void);
    // function that returns the golden ratio
    static double           goldenValue(void);

    /*! primitive operation */
                            // function that returns the square
    static double           sqr(double a)
                                {return a == (double)0.0 ? (double)0.0 : a*a;}
                            // function that returns a value greater of the two values
    static double           max(double ab, double bb) {return ab > bb ? ab : bb;}
                            // function that returns a value less of the two values
	static double           min(double a, double b) {return a < b ? a : b;}
                            // function that returns "a" value that changes the sign on the sign of "b"
    static double           sign(double a, double b); 
                            // function that returns "b" value that changes the sign on the sign of "a"
    static int              sign(int a, double b);  
                            // function to replace the contents of data
    static void             swap(double& a, double& b)
                                {double tmp = a; a = b;b = tmp;}
                            // divided difference function with m orders(elements of x, y need to be greater than m+1)
    static double           dividedDiff(const double* x, const double* y, unsigned int m);
                            // truncated power function((x2 - x1)^m for x2 >= x1, else 0)
    static double           tpf(double x1, double x2, int m);
        
    /*! operation on the absolute value */
    // function that returns the absolute value(double)
    static double           abs(double a);
    // function that returns the absolute value(int)
    static int              abs(int a);
    // function that returns the absolute value of the larger value of the absolute value among "a" and "b"(double)
    /*!
        @param[in] a 
        @param[in] b 
        @return calculation result
    */
    static double           absMax(double a, double b)
                            {
                                double ad, bd; 
                                return (ad = abs(a)) > (bd = abs(b)) ? ad : bd;
                            }
    // function that returns the absolute value of the smaller value of the absolute value among "a" and "b"(double)
    static double           absMin(double a, double b)
                            {
                                double ad, bd; 
                                return (ad = abs(a)) < (bd = abs(b)) ? ad : bd;
                            }
    // function that returns the absolute value of the larger value of the absolute value among "a" and "b"(int)
    static int              absMax(int a, int b)
                            {
                                int ad, bd; 
                                return (ad = abs(a)) > (bd = abs(b)) ? ad : bd;
                            }
    // function that returns the absolute value of the smaller value of the absolute value among "a" and "b"(int)
    static int              absMin(int a, int b)
                            {   
                                int ad, bd; 
                                return (ad = abs(a)) < (bd = abs(b)) ? ad : bd;
                            }

    
    /*! special functions */
    // function that returns the square dataInstance
    static double           sqrt(double a);
    // exponential function
    static double           exp(double x);
    // natural logarithmic function
    static double           log(double x);
    // power function(x^y)
    static double           pow(double x, double y);
    // exponential mean function (a^2 + b^2)^(1/2)
	static double           pythag(double a, double b);
    // logarithm of gamma function(ln(Gamma(x) ), x > 0)
    static double           gammln(double x);
    // factorial function(n!)
    static double           factrl(unsigned int n);
    // logarithm of factorial function ln(n!)
    static double           factln(unsigned int n);  
    // combination(nCk)
    static double           bico(unsigned int n, unsigned int k);  
    // beta function(B(z, w))
    static double           beta(double z, double w);
    // imcomplete gamma function(P(a, x))
    static double           gammp(double a, double x);
    // imcomplete beta function(Ix(a, b))
    static double           betai(double a, double b, double x);
    // error function(erf(x))
    static double           erf(double x);
    // complementary error function
    static double           erfc(double x);
};
