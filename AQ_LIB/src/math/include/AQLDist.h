#pragma once

#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif


#include "AQLBasic.h"
#include "AQLCoreAppError.h"
#include <cmath>
#include <float.h>


#define ONE_OVER_SQRT_TWO     0.7071067811865475244008443621048490392848359376887
#define ONE_OVER_SQRT_TWO_PI  0.3989422804014326779399460599343818684758586311649
#define SQRT_TWO_PI           2.506628274631000502415765284811045253006986740610

#ifndef DBL_EPSILON
#define DBL_EPSILON           2.2204460492503131e-016
#endif

#ifdef _MSC_VER
#undef max
#undef min
#endif

//===================== AQLDist =============================================
/*! 
    @brief Class declaration to provide distribution functions.
	Only static functions are implemented on this class.
*/
class AQLDist {

public:
	// 
							// Cumulative density function that represents the standard normal distribution. 
							//To calculate the probability to achieve the following values: less that x
	static double			normsdist(double x)
							{
                                //const double MM_SQRT_2 = 0.7071067811865475244008443621048490392848359376887;
                                const double MM_SQRT2 = 1.41421356237309504880168872420969807856967187537694;
                                const double MM_1_SQRTPI = 0.564189583547756286948;

                                const double a[5] = {
                                    1.161110663653770e-002, 3.951404679838207e-001, 2.846603853776254e+001,
                                    1.887426188426510e+002, 3.209377589138469e+003
                                };
                                const double b[5] = {
                                    1.767766952966369e-001, 8.344316438579620e+000, 1.725514762600375e+002,
                                    1.813893686502485e+003, 8.044716608901563e+003
                                };
                                const double c[9] = {
                                    2.15311535474403846e-8, 5.64188496988670089e-1, 8.88314979438837594e00,
                                    6.61191906371416295e01, 2.98635138197400131e02, 8.81952221241769090e02,
                                    1.71204761263407058e03, 2.05107837782607147e03, 1.23033935479799725E03
                                };
                                const double d[9] = {
                                    1.00000000000000000e00, 1.57449261107098347e01, 1.17693950891312499e02,
                                    5.37181101862009858e02, 1.62138957456669019e03, 3.29079923573345963e03,
                                    4.36261909014324716e03, 3.43936767414372164e03, 1.23033935480374942e03
                                };
                                const double p[6] = {
                                    1.63153871373020978e-2, 3.05326634961232344e-1, 3.60344899949804439e-1,
                                    1.25781726111229246e-1, 1.60837851487422766e-2, 6.58749161529837803e-4
                                };
                                const double q[6] = {
                                    1.00000000000000000e00, 2.56852019228982242e00, 1.87295284992346047e00,
                                    5.27905102951428412e-1, 6.05183413124413191e-2, 2.33520497626869185e-3
                                };

                                double y, z;
                                
                                y = AQLMath::abs(x);
                                if (y <= 0.46875 * MM_SQRT2) 
                                {
                                    // evaluate erf() for |x| <= sqrt(2)*0.46875
                                    z = y * y;
                                    y = x * ((((a[0]*z+a[1])*z+a[2])*z+a[3])*z+a[4])
                                          / ((((b[0]*z+b[1])*z+b[2])*z+b[3])*z+b[4]);
                                    return 0.5 + y;
                                }
                                z = AQLMath::exp(-y*y/2) / 2;
                                if (y <= 4.0 * MM_SQRT2) 
                                {
                                    // evaluate erfc() for sqrt(2)*0.46875 <= |x| <= sqrt(2)*4.0
                                    y = y / MM_SQRT2;
                                    y = ((((((((c[0]*y+c[1])*y+c[2])*y+c[3])*y+c[4])*y+c[5])*y+c[6])*y+c[7])*y+c[8])
                                      / ((((((((d[0]*y+d[1])*y+d[2])*y+d[3])*y+d[4])*y+d[5])*y+d[6])*y+d[7])*y+d[8]);

                                    y = z * y;
                                } 
                                else 
                                {
                                    // evaluate erfc() for |x| > sqrt(2)*4.0
                                    z = z * MM_SQRT2 / y;
                                    y = 2 / (y*y);
                                    y = y * (((((p[0]*y+p[1])*y+p[2])*y+p[3])*y+p[4])*y+p[5])
                                          / (((((q[0]*y+q[1])*y+q[2])*y+q[3])*y+q[4])*y+q[5]);
                                    y = z * (MM_1_SQRTPI-y);
                                }
                                return (x < 0.0 ? y : 1-y);
							}


							// cumulative density function of two-dimensional standard normal distribution, and return the probability that
							// the first variable is less than x, the second variable is less than y
	static double			normsdist2(double x, double y, double rho);

							// cumulative binomial probability function, and return the probability that
							// success is greater than or equal to the number of k of n trials
	static double			binomdist(unsigned int k, unsigned int n, double p);
							
							// cumulative binomial probability function, and return the probability that
							// number of occurrences is from 0 to k-1 [0,k-1) in case the expectation is x
	static double			poissondist(double x, unsigned int k);

							// chi-square probability function, and return the probability that
							// the chi-square value is less than x under n degrees of freedom
	static double			chidist(double x, unsigned int n);

							// t-distribution function, and return the probability that
							// the realized value is less than t (both sides) under n degrees of freedom
	static double			tdist(double t, unsigned int n);

							// F distribution function, and return the probability that
							// the realized value is less than t under n1 and n2 degrees of freedom
	static double			fdist(double t, unsigned int n1, unsigned int n2);
	//inverse function
	
	// inverse function of normal distribution function, and return the value that the probability is less than x where x is in (0,1)
	static double			invNormdist(double x)
							{
								if (x <= 0) throw AQLCoreNumericalError("argument must be over 0 for invNormdist", __FILE__, __LINE__);
								if (x >= 1) throw AQLCoreNumericalError("argument must be under 1 for invNormdist", __FILE__, __LINE__);
    
                                const double MM_SQRT2PI = 2.506628274631000;
                                
                                const double a[6] = {
                                    -3.969683028665376e+01,  2.209460984245205e+02,
                                    -2.759285104469687e+02,  1.383577518672690e+02,
                                    -3.066479806614716e+01,  2.506628277459239e+00
                                };
                                const double b[5] = {
                                    -5.447609879822406e+01,  1.615858368580409e+02,
                                    -1.556989798598866e+02,  6.680131188771972e+01,
                                    -1.328068155288572e+01
                                };
                                const double c[6] = {
                                    -7.784894002430293e-03, -3.223964580411365e-01,
                                    -2.400758277161838e+00, -2.549732539343734e+00,
                                    4.374664141464968e+00,  2.938163982698783e+00
                                };
                                const double d[4] = {
                                    7.784695709041462e-03,  3.224671290700398e-01,
                                    2.445134137142996e+00,  3.754408661907416e+00
                                };

                                double q, t, u;

                                q = AQLMath::min(x, 1-x);
                                if (q > 0.02425) 
                                {
                                    // Rational approximation for central region.
                                    u = q - 0.5;
                                    t = u * u;
                                    u = u * (((((a[0]*t+a[1])*t+a[2])*t+a[3])*t+a[4])*t+a[5])
                                          / (((((b[0]*t+b[1])*t+b[2])*t+b[3])*t+b[4])*t+1);
                                } 
                                else 
                                {
                                    // Rational approximation for tail region.
                                    t = sqrt(-2*AQLMath::log(q));
                                    u = (((((c[0]*t+c[1])*t+c[2])*t+c[3])*t+c[4])*t+c[5])
                                      /  ((((d[0]*t+d[1])*t+d[2])*t+d[3])*t+1);
                                }
                                // The relative error of the approximation has absolute value less
                                // than 1.15e-9.  One iteration of Halley's rational method (third
                                // order) gives full machine precision...
                                t = normsdist(u) - q;    // error
                                t = t * MM_SQRT2PI * AQLMath::exp(u*u/2);   // f(u)/df(u)
                                u = u - t / (1 + u * t/2);     // Halley's method

                                return (x > 0.5 ? -u : u);
							}


    //inverse function
	// inverse function of normal distribution function, and return the value that the probability is less than x where x is in (0,1)
	static double			invNormdistHalleyMod(double u)
							{
								if (u <= 0) throw AQLCoreNumericalError("argument must be over 0 for invNormdist", __FILE__, __LINE__);
								if (u >= 1) throw AQLCoreNumericalError("argument must be under 1 for invNormdist", __FILE__, __LINE__);

                                    
                                 // Coefficients for the rational approximation.
                                const double
                                  a1 = -3.969683028665376e+01,
                                  a2 =  2.209460984245205e+02,
                                  a3 = -2.759285104469687e+02,
                                  a4 =  1.383577518672690e+02,
                                  a5 = -3.066479806614716e+01,
                                  a6 =  2.506628277459239e+00;
                                  
                                const double
                                  b1 = -5.447609879822406e+01,
                                  b2 =  1.615858368580409e+02,
                                  b3 = -1.556989798598866e+02,
                                  b4 =  6.680131188771972e+01,
                                  b5 = -1.328068155288572e+01;
                                  
                                const double
                                  c1 = -7.784894002430293e-03,
                                  c2 = -3.223964580411365e-01,
                                  c3 = -2.400758277161838e+00,
                                  c4 = -2.549732539343734e+00,
                                  c5 =  4.374664141464968e+00,
                                  c6 =  2.938163982698783e+00;
                                  
                                const double
                                  d1 =  7.784695709041462e-03,
                                  d2 =  3.224671290700398e-01,
                                  d3 =  2.445134137142996e+00,
                                  d4 =  3.754408661907416e+00;
                                  
                                // Limits of the approximation region.
                                const double
                                  u_low   = 0.02425,
                                  u_high  = 1.0 - u_low;

                                double z, r;

                                // Rational approximation for the lower region. ( 0 < u < u_low )
                                if( u < u_low ){
                                  z = sqrt(-2.0*log(u));
                                  z = (((((c1*z+c2)*z+c3)*z+c4)*z+c5)*z+c6) / ((((d1*z+d2)*z+d3)*z+d4)*z+1.0);
                                }
                                // Rational approximation for the central region. ( u_low <= u <= u_high )
                                else if( u <= u_high ){
                                  z = u - 0.5;
                                  r = z*z;
                                  z = (((((a1*r+a2)*r+a3)*r+a4)*r+a5)*r+a6)*z / (((((b1*r+b2)*r+b3)*r+b4)*r+b5)*r+1.0);
                                }
                                // Rational approximation for the upper region. ( u_high < u < 1 )
                                else {
                                  z = sqrt(-2.0*log(1.0-u));
                                  z = -(((((c1*z+c2)*z+c3)*z+c4)*z+c5)*z+c6) /  ((((d1*z+d2)*z+d3)*z+d4)*z+1.0);
                                }

                                #define REFINE_INVERSECUMULATIVENORMAL_TO_FULL_MACHINE_PRECISION_USING_HALLEYS_METHOD
                                #ifdef REFINE_INVERSECUMULATIVENORMAL_TO_FULL_MACHINE_PRECISION_USING_HALLEYS_METHOD
                                  // The relative error of the approximation has absolute value less
                                  // than 1.15e-9.  One iteration of Halley's rational method (third
                                  // order) gives full machine precision.

                                  r = (cumulativeNormal(z) - u) * SQRT_TWO_PI * exp( 0.5 * z * z );	//	f(z)/df(z)
                                  z -= r/(1+0.5*z*r);							//	Halley's method
                                #endif

                                  return z;
							}

    // Function to return the value of the standard normal probability density function (pdf)
    static double			normsPDF( const double & z )
    {
        const double standardNormalPDF = normal ( z );
        return standardNormalPDF;
    }


private:

	// normal method
	static double normal(double x){ return ONE_OVER_SQRT_TWO_PI * exp( -0.5 * (x*x) ); }

	// error method
	static double errorFunction(double x)
	{
		static const double
		tiny	    = 1e-300,
		one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
		/* c = (float)0.84506291151 */
		erx =  8.45062911510467529297e-01, /* 0x3FEB0AC1, 0x60000000 */
		//
		// Coefficients for approximation to  erf on [0,0.84375]
		//
		efx =  1.28379167095512586316e-01, /* 0x3FC06EBA, 0x8214DB69 */
		efx8=  1.02703333676410069053e+00, /* 0x3FF06EBA, 0x8214DB69 */
		pp0  =  1.28379167095512558561e-01, /* 0x3FC06EBA, 0x8214DB68 */
		pp1  = -3.25042107247001499370e-01, /* 0xBFD4CD7D, 0x691CB913 */
		pp2  = -2.84817495755985104766e-02, /* 0xBF9D2A51, 0xDBD7194F */
		pp3  = -5.77027029648944159157e-03, /* 0xBF77A291, 0x236668E4 */
		pp4  = -2.37630166566501626084e-05, /* 0xBEF8EAD6, 0x120016AC */
		qq1  =  3.97917223959155352819e-01, /* 0x3FD97779, 0xCDDADC09 */
		qq2  =  6.50222499887672944485e-02, /* 0x3FB0A54C, 0x5536CEBA */
		qq3  =  5.08130628187576562776e-03, /* 0x3F74D022, 0xC4D36B0F */
		qq4  =  1.32494738004321644526e-04, /* 0x3F215DC9, 0x221C1A10 */
		qq5  = -3.96022827877536812320e-06, /* 0xBED09C43, 0x42A26120 */
		//
		// Coefficients for approximation to  erf  in [0.84375,1.25]
		//
		pa0  = -2.36211856075265944077e-03, /* 0xBF6359B8, 0xBEF77538 */
		pa1  =  4.14856118683748331666e-01, /* 0x3FDA8D00, 0xAD92B34D */
		pa2  = -3.72207876035701323847e-01, /* 0xBFD7D240, 0xFBB8C3F1 */
		pa3  =  3.18346619901161753674e-01, /* 0x3FD45FCA, 0x805120E4 */
		pa4  = -1.10894694282396677476e-01, /* 0xBFBC6398, 0x3D3E28EC */
		pa5  =  3.54783043256182359371e-02, /* 0x3FA22A36, 0x599795EB */
		pa6  = -2.16637559486879084300e-03, /* 0xBF61BF38, 0x0A96073F */
		qa1  =  1.06420880400844228286e-01, /* 0x3FBB3E66, 0x18EEE323 */
		qa2  =  5.40397917702171048937e-01, /* 0x3FE14AF0, 0x92EB6F33 */
		qa3  =  7.18286544141962662868e-02, /* 0x3FB2635C, 0xD99FE9A7 */
		qa4  =  1.26171219808761642112e-01, /* 0x3FC02660, 0xE763351F */
		qa5  =  1.36370839120290507362e-02, /* 0x3F8BEDC2, 0x6B51DD1C */
		qa6  =  1.19844998467991074170e-02, /* 0x3F888B54, 0x5735151D */
		//
		// Coefficients for approximation to  erfc in [1.25,1/0.35]
		//
		ra0  = -9.86494403484714822705e-03, /* 0xBF843412, 0x600D6435 */
		ra1  = -6.93858572707181764372e-01, /* 0xBFE63416, 0xE4BA7360 */
		ra2  = -1.05586262253232909814e+01, /* 0xC0251E04, 0x41B0E726 */
		ra3  = -6.23753324503260060396e+01, /* 0xC04F300A, 0xE4CBA38D */
		ra4  = -1.62396669462573470355e+02, /* 0xC0644CB1, 0x84282266 */
		ra5  = -1.84605092906711035994e+02, /* 0xC067135C, 0xEBCCABB2 */
		ra6  = -8.12874355063065934246e+01, /* 0xC0545265, 0x57E4D2F2 */
		ra7  = -9.81432934416914548592e+00, /* 0xC023A0EF, 0xC69AC25C */
		sa1  =  1.96512716674392571292e+01, /* 0x4033A6B9, 0xBD707687 */
		sa2  =  1.37657754143519042600e+02, /* 0x4061350C, 0x526AE721 */
		sa3  =  4.34565877475229228821e+02, /* 0x407B290D, 0xD58A1A71 */
		sa4  =  6.45387271733267880336e+02, /* 0x40842B19, 0x21EC2868 */
		sa5  =  4.29008140027567833386e+02, /* 0x407AD021, 0x57700314 */
		sa6  =  1.08635005541779435134e+02, /* 0x405B28A3, 0xEE48AE2C */
		sa7  =  6.57024977031928170135e+00, /* 0x401A47EF, 0x8E484A93 */
		sa8  = -6.04244152148580987438e-02, /* 0xBFAEEFF2, 0xEE749A62 */
		//
		// Coefficients for approximation to  erfc in [1/.35,28]
		//
		rb0  = -9.86494292470009928597e-03, /* 0xBF843412, 0x39E86F4A */
		rb1  = -7.99283237680523006574e-01, /* 0xBFE993BA, 0x70C285DE */
		rb2  = -1.77579549177547519889e+01, /* 0xC031C209, 0x555F995A */
		rb3  = -1.60636384855821916062e+02, /* 0xC064145D, 0x43C5ED98 */
		rb4  = -6.37566443368389627722e+02, /* 0xC083EC88, 0x1375F228 */
		rb5  = -1.02509513161107724954e+03, /* 0xC0900461, 0x6A2E5992 */
		rb6  = -4.83519191608651397019e+02, /* 0xC07E384E, 0x9BDC383F */
		sb1  =  3.03380607434824582924e+01, /* 0x403E568B, 0x261D5190 */
		sb2  =  3.25792512996573918826e+02, /* 0x40745CAE, 0x221B9F0A */
		sb3  =  1.53672958608443695994e+03, /* 0x409802EB, 0x189D5118 */
		sb4  =  3.19985821950859553908e+03, /* 0x40A8FFB7, 0x688C246A */
		sb5  =  2.55305040643316442583e+03, /* 0x40A3F219, 0xCEDF3BE6 */
		sb6  =  4.74528541206955367215e+02, /* 0x407DA874, 0xE79FE763 */
		sb7  = -2.24409524465858183362e+01; /* 0xC03670E2, 0x42712D62 */

		double R,S,P,Q,s,y,z,r, ax;

		ax = fabs(x);

		if(ax < 0.84375) {		/* |x|<0.84375 */
			if(ax < 3.7252902984e-09) { /* |x|<2**-28 */
				if (ax < DBL_MIN*16)
					return 0.125*(8.0*x+efx8*x);  /*avoid underflow */
				return x + efx*x;
			}
			z = x*x;
			r = pp0+z*(pp1+z*(pp2+z*(pp3+z*pp4)));
			s = one+z*(qq1+z*(qq2+z*(qq3+z*(qq4+z*qq5))));
			y = r/s;
			return x + x*y;
		}
		if(ax <1.25) {		/* 0.84375 <= |x| < 1.25 */
			s = ax-one;
			P = pa0+s*(pa1+s*(pa2+s*(pa3+s*(pa4+s*(pa5+s*pa6)))));
			Q = one+s*(qa1+s*(qa2+s*(qa3+s*(qa4+s*(qa5+s*qa6)))));
			if(x>=0) return erx + P/Q; else return -erx - P/Q;
		}
		if (ax >= 6) {		/* inf>|x|>=6 */
			if(x>=0) return one-tiny; else return tiny-one;
		}

		/* Starts to lose accuracy when ax~5 */
		s = one/(ax*ax);

		if(ax < 2.85714285714285) {	/* |x| < 1/0.35 */
			R=ra0+s*(ra1+s*(ra2+s*(ra3+s*(ra4+s*(
								ra5+s*(ra6+s*ra7))))));
			S=one+s*(sa1+s*(sa2+s*(sa3+s*(sa4+s*(
								sa5+s*(sa6+s*(sa7+s*sa8)))))));
		} 
		else {	/* |x| >= 1/0.35 */
			R=rb0+s*(rb1+s*(rb2+s*(rb3+s*(rb4+s*(
								rb5+s*rb6)))));
			S=one+s*(sb1+s*(sb2+s*(sb3+s*(sb4+s*(
								sb5+s*(sb6+s*sb7))))));
		}
		r = exp( -ax*ax-0.5625 +R/S);
		if(x>=0) return one-r/ax; else return  r/ax-one;
	}


	// cumulative normal method
	static double cumulativeNormal(double z)
	{
		double result;
		result = 0.5 * ( 1 + errorFunction( z*ONE_OVER_SQRT_TWO ) ); 
		if (result<=DBL_EPSILON){
		//
		// Asymptotic expansion for very negative z following (26.2.12) on page 408 in
		// M. Abramowitz and A. Stegun, Pocketbook of Mathematical Functions, ISBN 3-87144818-4.
		//
			double sum=1, zsqr=z*z, i=1, g=1, x, y, a=DBL_MAX, lasta;
			do {
				lasta=a;
				x = (4*i-3)/zsqr;
				y = x*((4*i-1)/zsqr);
				a = g*(x-y);
				sum -= a;
				g *= y;
				++i;
				a = fabs(a);
			} while (lasta>a && a>=fabs(sum*DBL_EPSILON));
			result = -normal(z)/z*sum;
		}
		return result;
	}


};

