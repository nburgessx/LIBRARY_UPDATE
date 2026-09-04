/*! @file
    @brief Class definition of integration by Romberg method

    This class derives from abstract base class "AQLIntegralBase".
*/


#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRomberg.h"
#include "AQLFunctionBase.h"
#include "AQLBasic.h"

using namespace std;

//#define	ROMBERG_EPS		1.0e-5	// romberg eps


//================ AQLRomberg ===================================
/*!
	@brief Constructor
	@param[in] mpowertopoints e^{mpowertopoints} for divident number in integral region
	@param[in] morderofromberg order for Romberg method
*/
AQLRomberg::AQLRomberg(unsigned short mpowertopoints, unsigned short morderofromberg, double romberg_eps)
: AQL1DIntegral(), mPowerToPoints(mpowertopoints),mOrderOfRomberg(morderofromberg),ROMBERG_EPS(romberg_eps)
{
}

/*!
	@brief Destructor
*/
AQLRomberg::~AQLRomberg() 
{
}
/*!
    @brief Return this function type
    @return function type
*/
function_t
AQLRomberg::getType() const
{
	return FN_ROMBERG;
}
/*!
    @brief Make copy(clone) of this class
    @return Deep copy of this class
*/
AQLCoreFunctionBase*
AQLRomberg::clone() const
{
    try 
	{
        return new AQLRomberg(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }	
}
/*!
    @brief Check function for this class ID
    @param[in] id ID to check
    @return True or False
*/
bool
AQLRomberg::isTypeOf(function_t id) const
{
    return (id == FN_ROMBERG ? true : AQL1DIntegral::isTypeOf(id));
}


/*!
    @brief  Integration function by Romberg method.

    @param[in] f target function
	@param[in] xl lower side of integral region
	@param[in] xu upper side of integral region

	@return integral result
*/
double
AQLRomberg::integrate(const AQLFunctionBase& f,double xl,double xu)const
{
	if (xl == POSITIVE_INFINITY || xu == POSITIVE_INFINITY ||
		xl == NEGATIVE_INFINITY || xu == NEGATIVE_INFINITY)
	throw AQLCoreNumericalError("Wrong integral domain in AQLRomberg::integrate.",__FILE__,__LINE__);

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
			if ((AQLMath::abs(dss) <= ROMBERG_EPS*AQLMath::abs(ss))) return ss;
		}
		h[j] = 0.25 * h[j-1];
	}
    return ss;
	
	if (j > JMAXP)
	{
		throw AQLCoreNumericalError("too many steps in AQLRomberg::integrage", __FILE__, __LINE__);
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
AQLRomberg::polint(DoubleArray &xa,
					DoubleArray &ya,
					const double x,
					double &y,
					double &dy) const
{
	int i, m, ns = 0;
	double den, dif, dift, ho, hp, w;

	int n = xa.size();
	DoubleArray c(n), d(n);
	dif = AQLMath::abs(x-xa[0]);
	for (i=0; i<n; i++)
	{
		if ((dift = AQLMath::abs(x-xa[i])) < dif)
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
					throw AQLCoreInvalidData("too close input parameter in AQLRomberg::polint",
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
AQLRomberg::trapzd(const AQLFunctionBase &f,
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



