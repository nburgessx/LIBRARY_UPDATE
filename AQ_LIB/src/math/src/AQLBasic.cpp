/*! @file
    @brief Implementation to provide basic functions.
            Only static functions are implemented on this class.
*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLMath.cpp
//
//  DESCRIPTION :       Define a special function for numeric operations.
//                      Only static functions are implemented on this class.
//  SEE ALSO    :       AQLMath.h
//  STATUS      :       FIX
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLBasic.h"
#include "AQLCoreAppError.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <climits>

//#include <FLOAT.H>
#include <float.h>

using namespace std;

class AQLMath;


template <class T> class AQLMath_sub {
public:
/*!
    @brief variable to calculate the minimum value (not divided any more) corresponding to the variable type
*/
    T minVal(T& val) {for (T d(1); d != T(0); val = d, d /= T(2)); 
                            return val;}
/*!
    @brief variable to calculate the minimum error value corresponding to the variable type
*/
    T epsilon(T& val) {for (T d(1); T(1) + d != T(1); 
                            val = d, d /= T(2)); return val;};
};


static const int ITMAX = 100;
static double    wk;
static const double MINIMUM = AQLMath::minValue(wk);
static const double EPSILON = AQLMath::epsValue(wk);
static const double MAXIMUM = DBL_MAX;//FLT_MAX;

//============== IMPLEMENTATION =========================
///////////////// Auxiliary function /////////////////////////////
/*!
    @brief gcf function related with Incomplete Gamma Function\n
    (See "Numerical recipes in C")

*/
static inline void 
gcf(double& gammcf, double a, double x, double& gln) 
{
    int n;
    double gold =0, g, fac=1,b1=1;
    double b0=0,anf,ana,an,a1,a0=1;

    gln=AQLMath::gammln(a);
    a1 = x;
    for (n = 1; n <= ITMAX;++n)
    {
        an = (double)n;
        ana=an-a;
        a0=(a1+a0*ana)*fac;
        b0=(b1+b0*ana)*fac;
        anf=an*fac;
        a1=x*a0+anf*a1;
        b1=x*b0+anf*b1;
        if (a1)
        {
            fac=(double)1.0/a1;
            g=b1*fac;
            if (fabs((g-gold)/g) < EPSILON)
            {
                gammcf =AQLMath::exp(-x+a*AQLMath::log(x)-gln)*g;
                return;
            }
            gold =g;
        }
    }
    { // ERROR
        char_t  msg[128];
        SPRINTF(msg,
            "Error in gcf. The argument is too large[%lf]", a);
        throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
    }
}
/*!
    @brief gser function related with Incomplete Gamma Function\n
    (See "Numerical recipes in C")

*/
static inline void 
gser(double& gamser, double a, double x, double& gln) 
{
    int n;
    double sum,del,ap;

    gln=AQLMath::gammln(a);

    if (x <= 0.0) 
    {
        if (x < 0.0) 
        {   // Error
            char_t  msg[128];
            SPRINTF(msg,
                "Error in gser. The argument is negative[%lf]", x);
            throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
        }
        gamser=0.0;
        return;
    } else {
        ap=a;
        del=sum=(double)1.0/a;
        for (n=1;n<=ITMAX;n++) {
            ++ap;
            del *= x/ap;
            sum += del;
            if (AQLMath::abs(del) < AQLMath::abs(sum)*EPSILON) {
                gamser=sum*AQLMath::exp(-x+a*AQLMath::log(x)-gln);
                return;
            }
        }
        { //error 
            char_t  msg[128];
            SPRINTF(msg,
                "Error in gser. The argument is too large[%lf]", a);
            throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
        }
    }
}
/*!
    @brief betacf function related with IncompleteBeta Function\n
    (See "Numerical recipes in C")

*/
static inline double 
betacf(double a, double b, double x) 
{
    int m,m2;
    double aa,c,d,del,h,qab,qam,qap;

    qab=a+b;
    qap=a + (double)1.0;
    qam=a - (double)1.0;
    c=(double)1.0;
    d=(double)1.0-qab*x/qap;
    if (AQLMath::abs(d) < MINIMUM) d=MINIMUM;
    d=(double)1.0/d;
    h=d;
    for (m=1;m<=ITMAX;m++) 
    {
        m2=2*m;
        aa=m*(b-m)*x/((qam+m2)*(a+m2));
        d=(double)1.0+aa*d;
        if (AQLMath::abs(d) < MINIMUM) d=MINIMUM;
        c=(double)1.0+aa/c;
        if (AQLMath::abs(c) < MINIMUM) c=MINIMUM;
        d=(double)1.0/d;
        h *= d*c;
        aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
        d=(double)1.0+aa*d;
        if (AQLMath::abs(d) < MINIMUM) d=MINIMUM;
        c=(double)1.0+aa/c;
        if (AQLMath::abs(c) < MINIMUM) c=MINIMUM;
        d=(double)1.0/d;
        del=d*c;
        h *= del;
        if (AQLMath::abs(del-(double)1.0) < EPSILON) break;
    }
    if (m > ITMAX) 
    {
        char_t  msg[128];
        SPRINTF(msg,
                "Error in betacf. The argument is too large[%lf]", a);
        throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
    }   
    return h;
}

/////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
/////////////////////////////////////////////////////////////////////////
/*! A number representing the precision */
/*!
    @brief function that returns the minimum value in double
    @param[in] val variable to be evaluated
    @return minimum value in double
*/
double 
AQLMath::minValue(double& val) 
{
    static double min;
    return (min == 0.0) ? (val = AQLMath_sub<double>().minVal(min)) 
                            : (val = min);
}
/*!
    @brief function that returns the maximum value in double
    @param[in] val variable to be evaluated
    @return the maximum value in double
*/
double 
AQLMath::maxValue(double& val) 
{
    val = MAXIMUM;
    return MAXIMUM;
}

/*!
    @brief function that returns the numerical calculation error level
    @param[in] val variable to be evaluated
    @return numerical calculation error level in double 
*/
double 
AQLMath::epsValue(double& val) 
{
    static double eps;
    return (eps == 0.0) ? (val = AQLMath_sub<double>().epsilon(eps)) 
                            : (val = eps);
}


/*!
    @brief function that returns a pi
    @return pi in double 
*/
double 
AQLMath::pi(void) 
{
    static double pi = 0.0;
    return (pi == 0.0) ? (pi = double(2.0 * ::asin(1.0))) : pi;
}

/*!
    @brief function that returns (e) the number of Napier
    @return the number of Napier in double
*/
double AQLMath::e(void) 
{
    static double e;
// ysuzuki  return (e == 0) ? (e = double(::exp(1))) : e;
    return (e == 0) ? (e = double(::exp(1.0))) : e;
}

/*!
    @brief function that returns the golden ratio
    @return the golden ratio in double
*/
double 
AQLMath::goldenValue(void) 
{
    static double gold;
    return ((gold == 0) ? gold = double((1.0 + ::sqrt(5.0)) / 2.0) 
                            : gold);
}

/*! primitive arithemetic  */

/*!
    @brief function that returns "a" value that changes the sign on the sign of "b"
    @param[in] a one of the value
    @param[in] b one of the value
    @return calculation result (double)
*/
double          
AQLMath::sign(double a, double b)
{
    return b >= 0.0 ? abs(a) : -abs(a);
}

/*!
    @brief function that returns "b" value that changes the sign on the sign of "a"
    @param[in] a one of the value
    @param[in] b one of the value
    @return calculation result (int)
*/
int     
AQLMath::sign(int a, double b)
{
    return b >= 0.0 ? abs(a) : -abs(a);
}

/*!
    @brief divided difference function with m orders(elements of x, y need to be greater than m+1)
    @param[in] x value of the x-axis
    @param[in] y value of the x-axis
    @param[in] m orders in the difference quotient
    @return calculation result (double)
*/
double
AQLMath::dividedDiff(const double* x, const double* y, unsigned int m)
{
    if (m == 0)
    {
        return y[0];
    }
    return (dividedDiff(x+1, y+1, m-1) - dividedDiff(x, y, m-1)) / 
                (x[m] - x[0]);
}

/*!
    @brief truncated power function((x2 - x1)^m for x2 >= x1, else 0)
    @param[in] x1 x1 value
    @param[in] x2 x2 value
    @param[in] m orders
    @return result in double
*/
double 
AQLMath::tpf(double x1, double x2, int m)
{
    // not include = in the following condition since x2 = x1 is 0
    return (x2 > x1) ? pow(x2 - x1, (double)m) : (double)0.0;
}


/*! Operations on the absolute value */
/*!
    @brief function that returns the absolute value(double)
    @param[in] a value in double
    @return calculation result (double)
*/
double          
AQLMath::abs(double a)
{
    return (double)::fabs(a);
}
/*!
    @brief function that returns the absolute value(int)
    @param[in] a value in integer
    @return calculation result (double)
*/
int         
AQLMath::abs(int a)
{
    return ::abs(a);
}

/*! special function */
/*!
    @brief function that returns the square dataInstance
    @param[in] a value in double
    @return calculation result (double)
*/
double          
AQLMath::sqrt(double a)
{
    if (a < 0)
    { // exception when less than 0
        char_t  msg[128];
        SPRINTF(msg,
            "Error in sqrt. The argument is negative[%lf]", a);
        throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
    }
    return double(::sqrt(a));
}

/*!
    @brief exponential function
    @param[in] x value in double
    @return calculation result (double)
*/
double          
AQLMath::exp(double x)
{
    double ret = double(::exp(x));

    if (ret > MAXIMUM)
    {
        char_t  msg[128];
        SPRINTF(msg,
			"Error: Exponential function: Input is too large [%1.3e]. Hint: Data should typically be entered as a percentage.", x);
        throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
    }
    return ret;
}

/*!
    @brief natural logarithmic function
    @param[in] x value in double
    @return calculation result (double)
*/

double          
AQLMath::log(double x)
{
    if (x < 0)
    { // exception when less than 0
        char_t  msg[128];
        SPRINTF(msg,
			"Error: Log function: Negative input to Log function is not allowed [%lf].", x);
        throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
    }
    return double(::log(x));
}

/*!
    @brief power function(x^y)
    @param[in] x value in double
    @param[in] y value in double
    @return calculation result (double)
*/
double          
AQLMath::pow(double x, double y)
{
    double ret = double(::pow(x, y));
    if (ret > MAXIMUM)  
    {
        char_t  msg[128];
        SPRINTF(msg,
            "Error: Power function: Input is too large [%lf^%lf]. Hint: Data should typically be entered as a percentage.", x, y);
        throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
    }
    return ret;
}
/*!
    @brief exponential mean function (a^2 + b^2)^(1/2)
    @param[in] a value in double
    @param[in] b value in double
    @return calculation result (double)
*/
double 
AQLMath::pythag(double a, double b)
{
    double absa, absb;
    absa=(double)::fabs(a);
    absb=(double)::fabs(b);
    double ret;

    if (absa > absb) {
        ret = absa*sqrt((double)1.0+sqr(absb/absa));
    } else {
        ret = (absb == (double)0.0 ? (double)0.0 : 
                        absb*sqrt((double)1.0+sqr(absa/absb)));
    }

    if (ret > MAXIMUM)  
    {
        char_t  msg[128];
        SPRINTF(msg,
            "Error in pythag. The argument is too large[%lf %lf]", a, b);
        throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
    }
    return ret;
}

/*!
    @brief  logarithm of gamma function(ln(Gamma(x)), x > 0)
    @param[in] x value in double
    @return calculation result (double)
*/
double 
AQLMath::gammln(double x)
/*!* ${tt xx} > 0$ then return $ln Gamma({tt x})$ **/
{   // the approximation of Lanczos(precision can be about 2E-10)
    if (x <= 0)
    { // exception when not larger than 0
        char_t  msg[128];
        SPRINTF(msg,
            "Error in gammln. The argument is negative[%lf]", x);
        throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
    }
    
    // apply the approximation of Lanczos
    double xx,y,tmp,ser;
    static double cof[6]={76.18009172947146,-86.50532032941677,
        24.01409824083091,-1.231739572450155,
        0.1208650973866179e-2,-0.5395239384953e-5};
    int j;  
    
    y=xx=x;

    tmp=xx+5.5;
    tmp -= (xx+0.5)*::log(tmp);

    ser=1.000000000190015;
    for (j=0;j<6;j++) ser += cof[j]/++y;
    return double(-tmp+::log(2.5066282746310005*ser/xx));
}



/*!
    @brief  factorial function(n!)
    @param[in] n value in integer
    @return calculation result (double)
*/
double 
AQLMath::factrl(unsigned int n) {
    static unsigned int ntop=4;
    static double a[33]={1.0,1.0,2.0,6.0,24.0};
    int j;

    if (n > 32)         
        return exp(gammln(n+(double)1.0));

    while (ntop<n) {
        j=ntop++;
        a[ntop]=a[j]*ntop;
    }
    return a[n];
}


/*!
    @brief logarithm of factorial function ln(n!)
    @param[in] n value in integer
    @return calculation result (double)
*/
double 
AQLMath::factln(unsigned int n)
{
    static double a[51];

    if (n <= 1) return 0.0;
    if (n <= 50) return a[n] ? a[n] : (a[n]=gammln(n+(double)1.0));
    else return gammln(n+(double)1.0);
}

/*!
    @brief  combination(nCk)
    @param[in] n integer
    @param[in] k integer
    @return calculation result (double)
*/
double          
AQLMath::bico(unsigned int n, unsigned int k)
{
    return double(::floor(0.5+exp(factln(n)-factln(k)-factln(n-k))));
}

/*!
    @brief  beta function(B(z, w)) \n
    B(z,w)=int^1_0t^{z-1}(1-t)^{w-1}dt=frac{Gamma(z)Gamma(w)}{gamma(z+w)} \n
    where Gamma() is gamma function (See Numerical Recipes in C)
    @param[in] z parameter
    @param[in] w parameter
    @return calculation result (double)
*/
double          
AQLMath::beta(double z, double w)
{   
    return exp(gammln(z) + gammln(w) - gammln(z + w));
}

/*!
    @brief  imcomplete gamma function(P(a, x)) \n
    P(z,w)=gamma(a,x)/Gamma(a)=1/Gamma(a) int^x_0 exp^{-t}t^{a-1}dt (a>0) \n
    where Gamma() is gamma function (See Numerical Recipes in C)
    @param[in] a parameter
    @param[in] x input value
    @return calculation result (double)
*/
double 
AQLMath::gammp(double a, double x)
{
    if (x < 0.0 || a <= 0.0) 
    {
        char_t  msg[128];
        SPRINTF(msg,
            "Error in gammp. The argument is invalid[a=%lf x=%lf]", a, x);
        throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
    }
    double gln;
    if (x < (a+1.0)) 
    {
        double gamser;
        gser(gamser,a,x,gln);
        return gamser;
    } 
    else 
    {
        double gammcf;
        gcf(gammcf,a,x,gln);
        return (double)1.0-gammcf;
    }
}
/*!
    @brief  imcomplete beta function(Ix(a, b))
    Ix(a,b)=B_x(a,b)/B(a,b)=1/B(a,b) int^x_0 t^{a-1}(1-t)^{b-1}dt (a,b > 0) \n
	where B(,) is beta function (See Numerical Recipes in C)
    @param[in] a parameter(double)
    @param[in] b parameter(double)
    @param[in] x input value(double)
    @return calculation result (double)
*/
double 
AQLMath::betai(double a, double b, double x)
{
    double bt;

    if (x < 0.0 || x > 1.0 || a <= 0.0 || b <= 0.0) 
    {
        char_t  msg[128];
        SPRINTF(msg,
            "Error in betai. The argument is invalid [a=%lf, b=%lf, x=%lf]",
                                    a, b, x);
        throw AQLCoreNumericalError(msg, __FILE__, __LINE__);
    }

    if (x == 0.0 || x == 1.0) 
    {
        bt=0.0;

    } 
    else 
    {
        bt=exp(gammln(a+b)-gammln(a)-gammln(b)+
                a*log(x)+b*log((double)1.0-x));
    }

    if (x < (a+1.0)/(a+b+2.0)) {
        return bt*betacf(a,b,x)/a;
    } else {
        return (double)1.0-bt*betacf(b,a,(double)1.0-x)/b;
    }
}

/*!
    @brief  error function(erf(x))
    @param[in] x input value (double)
    @return calculation result (double)
*/
double
AQLMath::erf(double x)
{
    return (x < 0.0) ? -gammp((double)0.5, x*x) 
                        : gammp((double)0.5, x*x);
}
/*!
    @brief  complementary error function
    @param[in] x input value (double)
    @return calculation result (double)
*/
double
AQLMath::erfc(double x)
{
    return (double)1.0 - erf(x);
}


