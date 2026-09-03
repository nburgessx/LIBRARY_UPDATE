/*! @file
    @brief Class definition of integration by Romberg method

    This class derives from abstract base class "LAIntegralBase".

*/


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARomberg.cpp
//
//  SYNOPSIS    :       LARomberg
//  DESCRIPTION :       Class definition of integration by Romberg method
//                      This class derives from abstract base class "LAIntegralBase".
//
//  SEE ALSO    :       
//  TYME        :       CLASS
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LARomberg.h"
#include "LAFunctionBase.h"
#include "LABasic.h"

using namespace std;

//#define	ROMBERG_EPS		1.0e-5	// romberg eps


//================ LARomberg ===================================
/*!
	@brief Constructor
	@param[in] mpowertopoints e^{mpowertopoints} for divident number in integral region
	@param[in] morderofromberg order for Romberg method
*/
LARomberg::LARomberg(unsigned short mpowertopoints, unsigned short morderofromberg, double romberg_eps)
: LA1DIntegral(), mPowerToPoints(mpowertopoints),mOrderOfRomberg(morderofromberg),ROMBERG_EPS(romberg_eps)
{
}

/*!
	@brief Destructor
*/
LARomberg::~LARomberg() 
{
}
/*!
    @brief Return this function type
    @return function type
*/
function_t
LARomberg::getType() const
{
	return FN_ROMBERG;
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
LACoreFunctionBase*
LARomberg::clone() const
{
    try 
	{
        return new LARomberg(*this);
    }
    catch (bad_alloc & e)
	{
        throw LACoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}
/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
LARomberg::isTypeOf(function_t id) const
{
    return (id == FN_ROMBERG ? true : LA1DIntegral::isTypeOf(id));
}


/*!
    @brief  Integration function by Romberg method.

    @param[in] f target function
	@param[in] xl lower side of integral region
	@param[in] xu upper side of integral region

	@return integral result
*/
double
LARomberg::integrate(const LAFunctionBase& f,double xl,double xu)const
{
	if (xl == POSITIVE_INFINITY || xu == POSITIVE_INFINITY ||
		xl == NEGATIVE_INFINITY || xu == NEGATIVE_INFINITY)
	throw LACoreNumericalError("Wrong integral domain in LARomberg::integrate.",__FILE__,__LINE__);

	if (xl > xu)return integrate(f, xu, xl);

    int JMAX = mPowerToPoints;
	int JMAXP = JMAX + 1;

	double ss, dss;
	DoubleArray s(JMAX), h(JMAXP),
					s_t(mOrderOfRomberg), h_t(mOrderOfRomberg);
	int i, j;
	double a = xl;
	double b = xu;

	h[0] = 1.0;
	for (j = 1; j <= JMAX; j++)
	{
		s[j-1] = trapzd(f, a, b, j);

		if (j >= mOrderOfRomberg)
		{
			for (i=0; i<mOrderOfRomberg; i++)
			{
				h_t[i] = h[j-mOrderOfRomberg+i];
				s_t[i] = s[j-mOrderOfRomberg+i];
			}
			polint(h_t, s_t, 0.0, ss, dss);
			if ((LAMath::abs(dss) <= ROMBERG_EPS*LAMath::abs(ss))) return ss;
		}
		h[j] = 0.25 * h[j-1];
	}
    return ss;
	
	if (j > JMAXP)
	{
		throw LACoreNumericalError("too many steps in LARomberg::integrage", __FILE__, __LINE__);
	}
	return 0;
}

/*!
    @brief  Polynomial interpolation function

    @param[in] xa x values
    @param[in] ya y values
    @param[in] x point to evaluate a target function
    @param[out] y function value at a target point 
	@param[out] dy error estimation at a target point
*/
void
LARomberg::polint(DoubleArray &xa,
					DoubleArray &ya,
					const double x,
					double &y,
					double &dy) const
{
	int i, m, ns = 0;
	double den, dif, dift, ho, hp, w;

	int n = xa.size();
	DoubleArray c(n), d(n);
	dif = LAMath::abs(x-xa[0]);
	for (i=0; i<n; i++)
	{
		if ((dift = LAMath::abs(x-xa[i])) < dif)
		{
			ns = i;
			dif = dift;
		}
		c[i] = ya[i];
		d[i] = ya[i];
	}
	y = ya[ns--];
	for (m = 1; m < n; m++)
	{
		for (i = 0; i < n-m; i++)
		{
			ho = xa[i]-x;
			hp = xa[i+m]-x;
			w = c[i+1]-d[i];
			if ((den = ho-hp) == 0.0)
					throw LACoreInvalidData("too close input parameter in LARomberg::polint",
											__FILE__, __LINE__);
				den = w / den;
			d[i] = hp * den;
			c[i] = ho * den;
		}
		y += (dy=(2 * (ns+1) < (n-m) ? c[ns+1] : d[ns--]));
	}
}

/*!
    @brief  method to refine integral with trapezoidal rule

    @param[in] f target function
	@param[in] a lower boundary of integral region
	@param[in] b upper boundary of integral region
	@param[in] n number to determine an interior point number
*/
double
LARomberg::trapzd(const LAFunctionBase &f,
					const double a,
					const double b,
					const int n) const
{
	double x, tnm, sum, del;
	static double s;
	int it, j;

	if (n == 1)
	{
		return ( s = 0.5 * (b-a) * (f(a) + f(b)) );
	}
	else
	{
		for (it = 1, j = 1; j < n-1; j++) it <<= 1;
		tnm = it;
		del = (b-a) / tnm;
		x = a + 0.5 * del;
		for (sum = 0.0, j = 0; j < it; j++, x += del) sum += f(x);
		s = 0.5 * (s + (b-a) * sum / tnm);
		return s;
	}
}



